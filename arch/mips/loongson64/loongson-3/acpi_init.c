// SPDX-License-Identifier: GPL-2.0
#include <linux/io.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/ioport.h>
#include <linux/export.h>
#include <linux/interrupt.h>
#include <loongson-pch.h>

static int acpi_irq;
static void *gpe0_status_reg;
static void *acpi_status_reg;
static void *acpi_enable_reg;
static void *acpi_control_reg;
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
	value = readw(acpi_status_reg);
	value |= (1 << 8 | 1 << 15);
	writew(value, acpi_status_reg);

	/* GPEStatus: Clear all generated events */
	writel(readl(gpe0_status_reg), gpe0_status_reg);
}

void acpi_sleep_prepare(void)
{
	u16 value;

	acpi_hw_clear_status();

	/* Turn ON LED blink */
	if (loongson_pch->type == RS780E) {
		value = pm_ioread(0x7c);
		value = (value & ~0xc) | 0x8;
		pm_iowrite(0x7c, value);
	}
}

void acpi_sleep_complete(void)
{
	u8 value;

	acpi_hw_clear_status();

	/* Turn OFF LED blink */
	if (loongson_pch->type == RS780E) {
		value = pm_ioread(0x7c);
		value |= 0xc;
		pm_iowrite(0x7c, value);
	}
}

static irqreturn_t acpi_int_routine(int irq, void *dev_id)
{
	u16 value;

	/* PMStatus: Check PwrBtnStatus */
	value = readw(acpi_status_reg);
	if (value & (1 << 8)) {
		writew(0x1 << 8, acpi_status_reg);
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

	if (!acpi_irq)
		return -ENODEV;

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

	if (loongson_pch->type != RS780E)
		goto enable_power_button;

	/* PM Status Base */
	pm_iowrite(0x20, SBX00_PM_EVT_BLK & 0xff);
	pm_iowrite(0x21, SBX00_PM_EVT_BLK >> 8);

	/* PM Control Base */
	pm_iowrite(0x22, SBX00_PM_CNT_BLK & 0xff);
	pm_iowrite(0x23, SBX00_PM_CNT_BLK >> 8);

	/* GPM Base */
	pm_iowrite(0x28, SBX00_GPE0_BLK & 0xff);
	pm_iowrite(0x29, SBX00_GPE0_BLK >> 8);

	/* ACPI End */
	pm_iowrite(0x2e, SBX00_PM_END & 0xff);
	pm_iowrite(0x2f, SBX00_PM_END >> 8);

	/* IO Decode: When AcpiDecodeEnable set, South-Bridge uses the contents
	 * of the PM registers at index 0x20~0x2B to decode ACPI I/O address. */
	pm_iowrite(0x0e, 1 << 3);

	/* Enable to generate SCI */
	pm_iowrite(0x10, pm_ioread(0x10) | 1);

	/* GPM3/GPM9 enable */
	value = inl(SBX00_GPE0_BLK + 4);
	outl(value | (1 << 14) | (1 << 22), SBX00_GPE0_BLK + 4);

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

enable_power_button:
	/* SCI_EN set */
	value = readw(acpi_control_reg);
	value |= 1;
	writew(value, acpi_control_reg);

	/* PMEnable: Enable PwrBtn */
	value = readw(acpi_enable_reg);
	value |= 1 << 8;
	writew(value, acpi_enable_reg);
}

int __init loongson_acpi_init(void)
{
	switch (loongson_pch->type) {
	case LS2H:
		acpi_irq = LS2H_PCH_ACPI_IRQ;
		acpi_control_reg = LS2H_PM_CNT_REG;
		acpi_status_reg  = LS2H_PM_EVT_REG;
		acpi_enable_reg  = LS2H_PM_ENA_REG;
		gpe0_status_reg  = LS2H_GPE0_STS_REG;
		break;
	case LS7A:
		acpi_irq = LS7A_PCH_ACPI_IRQ;
		acpi_control_reg = LS7A_PM1_CNT_REG;
		acpi_status_reg  = LS7A_PM1_EVT_REG;
		acpi_enable_reg  = LS7A_PM1_ENA_REG;
		gpe0_status_reg  = LS7A_GPE0_STS_REG;
		break;
	case RS780E:
		acpi_irq = RS780_PCH_ACPI_IRQ;
		acpi_control_reg = (void *)(mips_io_port_base + SBX00_PM_CNT_BLK + 0);
		acpi_status_reg  = (void *)(mips_io_port_base + SBX00_PM_EVT_BLK + 0);
		acpi_enable_reg  = (void *)(mips_io_port_base + SBX00_PM_EVT_BLK + 2);
		gpe0_status_reg  = (void *)(mips_io_port_base + SBX00_GPE0_BLK   + 0);
		register_acpi_resource();
		break;
	default:
		return 0;
	}

	acpi_registers_setup();
	acpi_hw_clear_status();

	return 0;
}
