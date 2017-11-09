// SPDX-License-Identifier: GPL-2.0
#include <linux/io.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/input.h>
#include <linux/ioport.h>
#include <linux/export.h>
#include <linux/interrupt.h>

#define SBX00_ACPI_IO_BASE 0x800
#define SBX00_ACPI_IO_SIZE 0x100

#define ACPI_PM_EVT_BLK         (SBX00_ACPI_IO_BASE + 0x00) /* 4 bytes */
#define ACPI_PM_CNT_BLK         (SBX00_ACPI_IO_BASE + 0x04) /* 2 bytes */
#define ACPI_PMA_CNT_BLK        (SBX00_ACPI_IO_BASE + 0x0F) /* 1 byte */
#define ACPI_PM_TMR_BLK         (SBX00_ACPI_IO_BASE + 0x18) /* 4 bytes */
#define ACPI_GPE0_BLK           (SBX00_ACPI_IO_BASE + 0x10) /* 8 bytes */
#define ACPI_END                (SBX00_ACPI_IO_BASE + 0x80)

#define PM_INDEX        0xCD6
#define PM_DATA         0xCD7
#define PM2_INDEX       0xCD0
#define PM2_DATA        0xCD1

static int acpi_irq;
static struct input_dev *button;

/*
 * SCI interrupt need acpi space, allocate here
 */

static int __init register_acpi_resource(void)
{
	request_region(SBX00_ACPI_IO_BASE, SBX00_ACPI_IO_SIZE, "acpi");
	return 0;
}

static void pmio_write_index(u16 index, u8 reg, u8 value)
{
	outb(reg, index);
	outb(value, index + 1);
}

static u8 pmio_read_index(u16 index, u8 reg)
{
	outb(reg, index);
	return inb(index + 1);
}

void pm_iowrite(u8 reg, u8 value)
{
	pmio_write_index(PM_INDEX, reg, value);
}
EXPORT_SYMBOL(pm_iowrite);

u8 pm_ioread(u8 reg)
{
	return pmio_read_index(PM_INDEX, reg);
}
EXPORT_SYMBOL(pm_ioread);

void pm2_iowrite(u8 reg, u8 value)
{
	pmio_write_index(PM2_INDEX, reg, value);
}
EXPORT_SYMBOL(pm2_iowrite);

u8 pm2_ioread(u8 reg)
{
	return pmio_read_index(PM2_INDEX, reg);
}
EXPORT_SYMBOL(pm2_ioread);

static void acpi_hw_clear_status(void)
{
	u16 value;

	/* PMStatus: Clear WakeStatus/PwrBtnStatus */
	value = inw(ACPI_PM_EVT_BLK);
	value |= (1 << 8 | 1 << 15);
	outw(value, ACPI_PM_EVT_BLK);

	/* GPEStatus: Clear all generated events */
	outl(inl(ACPI_GPE0_BLK), ACPI_GPE0_BLK);
}

void acpi_sleep_prepare(void)
{
	u16 value;

	acpi_hw_clear_status();

	/* Turn ON LED blink */
	value = pm_ioread(0x7c);
	value = (value & ~0xc) | 0x8;
	pm_iowrite(0x7c, value);
}

void acpi_sleep_complete(void)
{
	u8 value;

	acpi_hw_clear_status();

	/* Turn OFF LED blink */
	value = pm_ioread(0x7c);
	value |= 0xc;
	pm_iowrite(0x7c, value);
}

static irqreturn_t acpi_int_routine(int irq, void *dev_id)
{
	u16 value;

	/* PMStatus: Check PwrBtnStatus */
	value = inw(ACPI_PM_EVT_BLK);
	if (value & (1 << 8)) {
		outw(1 << 8, ACPI_PM_EVT_BLK);
		pr_info("Power Button pressed...\n");
		input_report_key(button, KEY_POWER, 1);
		input_sync(button);
		input_report_key(button, KEY_POWER, 0);
		input_sync(button);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static int __init power_button_init(void)
{
	int ret;
	struct pci_dev *dev;

	dev = pci_get_bus_and_slot(0, 0);
	switch (dev->vendor) {
	case PCI_VENDOR_ID_AMD:
	case PCI_VENDOR_ID_ATI:
		acpi_irq = 7;
		break;
	default:
		return -ENODEV;
	}

	button = input_allocate_device();
	if (!button)
		return -ENOMEM;

	button->name = "ACPI Power Button";
	button->phys = "acpi/button/input0";
	button->id.bustype = BUS_HOST;
	button->dev.parent = NULL;
	input_set_capability(button, EV_KEY, KEY_POWER);

	ret = request_irq(acpi_irq, acpi_int_routine, IRQF_SHARED, "acpi", acpi_int_routine);
	if (ret) {
		pr_err("ACPI Power Button Driver: Request irq %d failed!\n", acpi_irq);
		return -EFAULT;
	}

	ret = input_register_device(button);
	if (ret) {
		input_free_device(button);
		return ret;
	}

	pr_info("ACPI Power Button Driver: Init successful!\n");

	return 0;
}
device_initcall(power_button_init);

void acpi_registers_setup(void)
{
	u32 value;

	/* PM Status Base */
	pm_iowrite(0x20, ACPI_PM_EVT_BLK & 0xff);
	pm_iowrite(0x21, ACPI_PM_EVT_BLK >> 8);

	/* PM Control Base */
	pm_iowrite(0x22, ACPI_PM_CNT_BLK & 0xff);
	pm_iowrite(0x23, ACPI_PM_CNT_BLK >> 8);

	/* GPM Base */
	pm_iowrite(0x28, ACPI_GPE0_BLK & 0xff);
	pm_iowrite(0x29, ACPI_GPE0_BLK >> 8);

	/* ACPI End */
	pm_iowrite(0x2e, ACPI_END & 0xff);
	pm_iowrite(0x2f, ACPI_END >> 8);

	/* IO Decode: When AcpiDecodeEnable set, South-Bridge uses the contents
	 * of the PM registers at index 0x20~0x2B to decode ACPI I/O address. */
	pm_iowrite(0x0e, 1 << 3);

	/* SCI_EN set */
	outw(1, ACPI_PM_CNT_BLK);

	/* Enable to generate SCI */
	pm_iowrite(0x10, pm_ioread(0x10) | 1);

	/* GPM3/GPM9 enable */
	value = inl(ACPI_GPE0_BLK + 4);
	outl(value | (1 << 14) | (1 << 22), ACPI_GPE0_BLK + 4);

	/* Set GPM9 as input */
	pm_iowrite(0x8d, pm_ioread(0x8d) & (~(1 << 1)));

	/* Set GPM9 as non-output */
	pm_iowrite(0x94, pm_ioread(0x94) | (1 << 3));

	/* GPM3 config ACPI trigger SCIOUT */
	pm_iowrite(0x33, pm_ioread(0x33) & (~(3 << 4)));

	/* GPM9 config ACPI trigger SCIOUT */
	pm_iowrite(0x3d, pm_ioread(0x3d) & (~(3 << 2)));

	/* GPM3 config falling edge trigger */
	pm_iowrite(0x37, pm_ioread(0x37) & (~(1 << 6)));

	/* No wait for STPGNT# in ACPI Sx state */
	pm_iowrite(0x7c, pm_ioread(0x7c) | (1 << 6));

	/* Set GPM3 pull-down enable */
	value = pm2_ioread(0xf6);
	value |= ((1 << 7) | (1 << 3));
	pm2_iowrite(0xf6, value);

	/* Set GPM9 pull-down enable */
	value = pm2_ioread(0xf8);
	value |= ((1 << 5) | (1 << 1));
	pm2_iowrite(0xf8, value);

	/* PMEnable: Enable PwrBtn */
	value = inw(ACPI_PM_EVT_BLK + 2);
	value |= 1 << 8;
	outw(value, ACPI_PM_EVT_BLK + 2);
}

int __init sbx00_acpi_init(void)
{
	register_acpi_resource();
	acpi_registers_setup();
	acpi_hw_clear_status();

	return 0;
}
