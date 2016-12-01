#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <boot_param.h>

#define AT24_SMB_ADDR	0x51
struct i2c_client *at24c04_client;

static struct at24_platform_data at24c04 = {
	.byte_len       = SZ_4K / 8,
	.page_size      = 16,
};

static int at24c04_probe(struct platform_device *dev)
{
	struct i2c_adapter *adapter = NULL;
	struct i2c_board_info info;
	int i = 0, found = 0;

	memset(&info, 0, sizeof(struct i2c_board_info));

	adapter = i2c_get_adapter(i++);
	while (adapter) {
		if (strncmp(adapter->name, "SMBus PIIX4", 11) == 0) {
			found = 1;
			break;
		}

		adapter = i2c_get_adapter(i++);
	}

	if (!found)
		goto fail;

	info.addr = AT24_SMB_ADDR;
	info.platform_data = &at24c04;

	/* name should match drivers/misc/eeprom/at24.c id_table */
	strncpy(info.type, "24c04", I2C_NAME_SIZE);

	at24c04_client = i2c_new_device(adapter, &info);
	if (at24c04_client == NULL) {
		pr_err("failed to attach at24c04 sensor\n");
		goto fail;
	}

	pr_info("Success to attach AT24c04 sensor\n");

	return 0;
fail:
	pr_err("Fail to found smbus controller attach AT24c04 sensor\n");

	return 0;
}

static struct platform_driver at24c04_driver = {
	.probe		= at24c04_probe,
	.driver		= {
		.name	= "24c04",
		.owner	= THIS_MODULE,
	},
};

static struct platform_device *pdev;

static int __init at24c04_init(void)
{
	int ret;

	ret = platform_driver_register(&at24c04_driver);
	if (ret)
		return ret;
	if (IS_ERR(pdev = platform_device_register_simple("24c04", -1, NULL, 0)))
		return PTR_ERR(pdev);

	return ret;
}

static void __exit at24c04_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&at24c04_driver);
}

module_init(at24c04_init);
module_exit(at24c04_exit);

MODULE_AUTHOR("Binbin Zhou <zhoubb@lemote.com>");
MODULE_DESCRIPTION("AT24c04 driver, based on the at24 driver");
MODULE_LICENSE("GPL");
