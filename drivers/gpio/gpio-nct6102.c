#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <asm/types.h>
#include <linux/gpio.h>
#include <linux/io.h>

#define NCT6102_GPIO_BASE 	80

#define EFIR_ADDR		0x2E
#define EFDR_ADDR		0x2F

#define SIO_NCT6102_ID		0xc450
#define SIO_REG_DEVID		0x20	/* Device ID (2 bytes) */
#define SIO_REG_LDSEL		0x7 	/* Logical device select */
#define SIO_REG_MFS		0x1C 	/* Multi Function Selection */
#define SIO_GP2X_DISABLE	0x02	/* Disable GP2x */
#define SIO_DEV_GPIO		0x7	/* Logical device 7(GPIO) */
#define SIO_GPIO_DESC_BASE	0xE0	/* GPIO description base address */
#define SIO_GPIO_DATA_BASE	0xE1	/* GPIO data base address*/
#define SIO_GPIO_ENABLE		0x30	/* GPIO enable register */

#define REG_ADDR_OFFSET(x)	((x / 8) * 4)
#define REG_VALUE(x)		(x % 8)

static DEFINE_SPINLOCK(gpio_lock);

/*
 read/write_gbl() is used to read/write Global register.
 */
static unsigned char read_gbl(u8 gbl_reg)
{
	u8 ret = 0;

	/* Enter the Extended Function Mode */
	outb(0x87, EFIR_ADDR);
	outb(0x87, EFIR_ADDR);
	/* Read Global Reg */
	outb(gbl_reg, EFIR_ADDR);
	ret = inb(EFDR_ADDR);
	/* Exit the Extended Function Mode */
	outb(0xAA, EFIR_ADDR);

	return ret;
}

static void write_gbl(u8 val, u8 gbl_reg)
{
	outb(0x87, EFIR_ADDR);
	outb(0x87, EFIR_ADDR);
	outb(gbl_reg, EFIR_ADDR);
	outb(val, EFDR_ADDR);
	outb(0xAA, EFIR_ADDR);
}

/*
 read/write_dev() is used to read/write Logical Device register.
 */
static unsigned char read_dev(u8 dev_num, u8 dev_reg)
{
	u8 ret = 0;

	/* Enter the Extended Function Mode */
	outb(0x87, EFIR_ADDR);
	outb(0x87, EFIR_ADDR);
	/* Point to Logical Device Number Reg */
	outb(SIO_REG_LDSEL, EFIR_ADDR);
	/* Select Logical Device x */
	outb(dev_num, EFDR_ADDR);
	/* Select Logical Device Reg */
	outb(dev_reg, EFIR_ADDR);
	/* Read Logica Device Reg */
	ret = inb(EFDR_ADDR);
	/* Exit the Extended Function Mode */
	outb(0xAA, EFIR_ADDR);

	return ret;
}

static void write_dev(u8 val, u8 dev_num, u8 dev_reg)
{
	outb(0x87, EFIR_ADDR);
	outb(0x87, EFIR_ADDR);
	outb(SIO_REG_LDSEL, EFIR_ADDR);
	outb(dev_num, EFDR_ADDR);
	outb(dev_reg, EFIR_ADDR);
	outb(val, EFDR_ADDR);
	outb(0xAA, EFIR_ADDR);
}

/* Detect chip */
static u16 nct6102_chip_detect(void)
{
	u16 tmp;

	tmp = (read_gbl(SIO_REG_DEVID) << 8) | read_gbl(SIO_REG_DEVID + 1);

	return tmp & 0xFFF0;
}

static int nct6102_gpio_get_value(struct gpio_chip *chip, unsigned offset)
{
	u8 tmp;

	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DATA_BASE + REG_ADDR_OFFSET(offset));
	tmp &= 1 << REG_VALUE(offset);
	spin_unlock(&gpio_lock);

	return tmp;
}

static void nct6102_gpio_set_value(struct gpio_chip *chip, unsigned offset, int value)
{
	u8 tmp;

	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DATA_BASE + REG_ADDR_OFFSET(offset));
	if(value)
		tmp |= 1 << REG_VALUE(offset);
	else
		tmp &= ~(1 << REG_VALUE(offset));
	write_dev(tmp, SIO_DEV_GPIO, SIO_GPIO_DATA_BASE + REG_ADDR_OFFSET(offset));
	spin_unlock(&gpio_lock);
}

static int nct6102_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	u8 tmp;

	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DESC_BASE + REG_ADDR_OFFSET(offset));
	tmp |= 1 << REG_VALUE(offset);
	write_dev(tmp, SIO_DEV_GPIO, SIO_GPIO_DESC_BASE + REG_ADDR_OFFSET(offset));
	spin_unlock(&gpio_lock);

	return 0;
}

static int nct6102_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int level)
{
	u8 tmp;

	nct6102_gpio_set_value(chip, offset, level);
	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DESC_BASE + REG_ADDR_OFFSET(offset));
	tmp &= ~(1 << REG_VALUE(offset));
	write_dev(tmp, SIO_DEV_GPIO, SIO_GPIO_DESC_BASE + REG_ADDR_OFFSET(offset));
	spin_unlock(&gpio_lock);

	return 0;
}

static int nct6102_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	u8 val;

	val = read_dev(SIO_DEV_GPIO, SIO_GPIO_ENABLE);

	if (val & 1 << (offset / 8))
		return 0;

	return -EINVAL;
}

static struct gpio_chip nct6102_chip = {
	.label                  ="nct6102-gpio-chip",
	.request		= nct6102_gpio_request,
	.direction_input        = nct6102_gpio_direction_input,
	.direction_output       = nct6102_gpio_direction_output,
	.get                    = nct6102_gpio_get_value,
	.set                    = nct6102_gpio_set_value,
	.base                   = NCT6102_GPIO_BASE,
	.ngpio			= 64,
};

static int __init nct6102_gpio_setup(void)
{
	int ret;
	u16 dev_id;

	/* Detect device */
	dev_id = nct6102_chip_detect();
	if (dev_id != SIO_NCT6102_ID)
		return 0;

	return gpiochip_add(&nct6102_chip);
}
subsys_initcall(nct6102_gpio_setup);
