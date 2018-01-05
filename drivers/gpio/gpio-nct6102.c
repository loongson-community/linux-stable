#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <asm/types.h>
#include <linux/gpio.h>
#include <linux/io.h>

#define EFIR_ADDR	0x2E
#define EFDR_ADDR	0x2F

#define SIO_NCT6102_ID		0xc450
#define SIO_REG_DEVID		0x20	/* Device ID (2 bytes) */
#define SIO_REG_LDSEL		0x7 	/* Logical device select */
#define SIO_REG_MFS		0x1C 	/* Multi Function Selection */
#define SIO_GP2X_DISABLE	0x02	/* Disable GP2x */
#define SIO_DEV_GPIO		0x7	/* Logical device 7(GPIO) */
#define SIO_GPIO_DIR		0xE8	/* GP2x I/O register */
#define SIO_GPIO_DATA		0xE9	/* GP2x data register */
#define NCT6102_GPIO_BASE 	80

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

static int nct6102_gpio_get_value(struct gpio_chip *chip, unsigned gpio)
{
	u8 tmp;

	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DATA);
	tmp &= (1 << gpio);
	spin_unlock(&gpio_lock);

	return tmp;
}

static void nct6102_gpio_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	u8 tmp;

	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DATA);
	if(value)
		tmp |= (1 << gpio);
	else
		tmp &= ~(1 << gpio);
	write_dev(tmp, SIO_DEV_GPIO, SIO_GPIO_DATA);
	spin_unlock(&gpio_lock);
}

static int nct6102_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	u8 tmp;

	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DIR);
	tmp |= (1 << gpio);
	write_dev(tmp, SIO_DEV_GPIO, SIO_GPIO_DIR);
	spin_unlock(&gpio_lock);

	return 0;
}

static int nct6102_gpio_direction_output(struct gpio_chip *chip, unsigned gpio, int level)
{
	u8 tmp;

	nct6102_gpio_set_value(chip, gpio, level);
	spin_lock(&gpio_lock);
	tmp = read_dev(SIO_DEV_GPIO, SIO_GPIO_DIR);
	tmp &= ~(1 << gpio);
	write_dev(tmp, SIO_DEV_GPIO, SIO_GPIO_DIR);
	spin_unlock(&gpio_lock);

	return 0;
}

static struct gpio_chip nct6102_chip = {
	.label                  ="nct6102-gpio-chip",
	.direction_input        = nct6102_gpio_direction_input,
	.get                    = nct6102_gpio_get_value,
	.direction_output       = nct6102_gpio_direction_output,
	.set                    = nct6102_gpio_set_value,
	.base                   = NCT6102_GPIO_BASE,
	.ngpio			= 8,
};

static int __init nct6102_gpio_setup(void)
{
	u8 mfs_cfg;
	u16 dev_id;

	/* Detect device */
	dev_id = nct6102_chip_detect();
	if (dev_id != SIO_NCT6102_ID)
		return 0;

	mfs_cfg = read_gbl(SIO_REG_MFS);
	if (mfs_cfg & SIO_GP2X_DISABLE)
		return 0;

	return gpiochip_add(&nct6102_chip);
}
subsys_initcall(nct6102_gpio_setup);
