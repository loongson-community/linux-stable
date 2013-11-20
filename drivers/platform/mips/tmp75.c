#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <boot_param.h>
#include <loongson_hwmon.h>

#define TMP75_SMB_ADDR	0x4E

struct i2c_client *tmp75_client;

static int tmp75_probe(struct platform_device *dev)
{
	struct i2c_adapter *adapter = NULL;
	struct i2c_board_info info;
	int i = 0, found = 0;
	struct sensor_device *sdev = (struct sensor_device *)dev->dev.platform_data;

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

	info.addr = sdev->base_addr;
	info.platform_data = dev->dev.platform_data;

	/* name should match drivers/hwmon/lm75.c id_table */
	strncpy(info.type, "tmp75", I2C_NAME_SIZE);

	tmp75_client = i2c_new_device(adapter, &info);
	if (tmp75_client == NULL) {
		pr_err("failed to attach tmp75 sensor\n");
		goto fail;
	}

	pr_info("Success to attach TMP75 sensor\n");

	return 0;
fail:
	pr_err("Fail to found smbus controller attach TMP75 sensor\n");

	return 0;
}

static struct platform_driver tmp75_driver = {
	.probe		= tmp75_probe,
	.driver		= {
		.name	= "tmp75",
		.owner	= THIS_MODULE,
	},
};

static int __init tmp75_init(void)
{
	return platform_driver_register(&tmp75_driver);
}

static void __exit tmp75_exit(void)
{
	platform_driver_unregister(&tmp75_driver);
}

late_initcall(tmp75_init);
module_exit(tmp75_exit);

MODULE_AUTHOR("Hongbing Hu <huhb@lemote.com>");
MODULE_DESCRIPTION("TMP75 driver, based on the EMC1412 driver");
MODULE_LICENSE("GPL");
