#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/export.h>

#define SBX00_ACPI_IO_BASE 0x800
#define SBX00_ACPI_IO_SIZE 0x100

#define ACPI_PM_EVT_BLK         (SBX00_ACPI_IO_BASE + 0x00) /* 4 bytes */
#define ACPI_PM1_CNT_BLK        (SBX00_ACPI_IO_BASE + 0x04) /* 2 bytes */
#define ACPI_PMA_CNT_BLK        (SBX00_ACPI_IO_BASE + 0x0F) /* 1 byte */
#define ACPI_PM_TMR_BLK         (SBX00_ACPI_IO_BASE + 0x18) /* 4 bytes */
#define ACPI_GPE0_BLK           (SBX00_ACPI_IO_BASE + 0x10) /* 8 bytes */
#define ACPI_END                (SBX00_ACPI_IO_BASE + 0x80)

#define PM_INDEX        0xCD6
#define PM_DATA         0xCD7
#define PM2_INDEX       0xCD0
#define PM2_DATA        0xCD1

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

void sci_interrupt_setup(void)
{
	u32 temp32;

	/* pm1 base */
	pm_iowrite(0x22, ACPI_PM1_CNT_BLK & 0xff);
	pm_iowrite(0x23, ACPI_PM1_CNT_BLK >> 8);

	/* gpm base */
	pm_iowrite(0x28, ACPI_GPE0_BLK & 0xFF);
	pm_iowrite(0x29, ACPI_GPE0_BLK >> 8);

	/* gpm base */
	pm_iowrite(0x2e, ACPI_END & 0xFF);
	pm_iowrite(0x2f, ACPI_END >> 8);

	/* io decode */
	pm_iowrite(0x0E, 1<<3 | 0<<2); /* AcpiDecodeEnable, When set, SB uses
					* the contents of the PM registers at
					* index 20-2B to decode ACPI I/O address.
					* AcpiSmiEn & SmiCmdEn */

	/* SCI_EN set P225 */
	outw(1, ACPI_PM1_CNT_BLK);

	/* enable to generate SCI P180 */
	pm_iowrite(0x10, pm_ioread(0x10) | 1);

	/* gpm9 enable P227 */
	temp32 = inl(ACPI_GPE0_BLK + 4);
	outl(temp32 | (1 << 14), ACPI_GPE0_BLK + 4);

	/* set gpm9 as input P205 */
	pm_iowrite(0x8d, pm_ioread(0x8d) & (~(1 << 1)));

	/* set gpm9 not as output P207 */
	pm_iowrite(0x94, pm_ioread(0x94) | (1 << 3));

	/* gpm9 config ACPI trigger SCIOUT P191 */
	pm_iowrite(0x3d, pm_ioread(0x3d) & (~(3 << 2)));

	/* set gpm9 pull-down enable, P258 */
	temp32 = pm2_ioread(0xf8);
	temp32 |= ((1 << 5) | (1 << 1));
	pm2_iowrite(0xf8, temp32);
}

int __init sbx00_acpi_init(void)
{
	register_acpi_resource();

	sci_interrupt_setup();

	return 0;
}
