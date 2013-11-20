#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <loongson_hwmon.h>

#define EMC1412_SMB_ADDR	0x4c

struct i2c_client *emc1412_client = NULL;

static int __devinit emc1412_probe(struct platform_device *dev)
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

	info.addr = EMC1412_SMB_ADDR;
	info.platform_data = "EMC1412 Temprature Sensor";

	emc1412_client = i2c_new_device(adapter, &info);
	if (emc1412_client == NULL) {
		printk(KERN_ERR "failed to attach EMC1412 sensor\n");
		goto fail;
	}

	printk(KERN_INFO "Success to attach EMC1412 sensor\n");

	return 0;
fail:
	printk(KERN_ERR "Fail to fount smbus controller attach EMC1412 sensor\n");

	return 0;
}

/*
 * emc1412 provide 2 temprature data
 * Internal temprature: reg0.reg29
 * External temprature: reg1.reg10
 * reg0 & reg1 from 0 to 127
 * reg1 & reg10 between (0.125, 0.875)
 * to avoid use float, temprature will mult 1000
 */
int emc1412_internal_temp(void)
{
	u8 reg;
	int temp;

	/* not ready ??? */
	if (emc1412_client == NULL)
		return NOT_VALID_TEMP;

	temp = i2c_smbus_read_byte_data(emc1412_client, 0) * 1000;
	reg = i2c_smbus_read_byte_data(emc1412_client, 0x29);
	temp += (reg >> 5) * 125;

	return temp;
}

int emc1412_external_temp(void)
{
	u8 reg;
	int temp;

	/* not ready ??? */
	if (emc1412_client == NULL)
		return NOT_VALID_TEMP;

	temp = i2c_smbus_read_byte_data(emc1412_client, 1) * 1000;
	reg = i2c_smbus_read_byte_data(emc1412_client, 0x10);
	temp += (reg >> 5) * 125;

	return temp;
}

EXPORT_SYMBOL_GPL(emc1412_internal_temp);
EXPORT_SYMBOL_GPL(emc1412_external_temp);

static struct platform_driver emc1412_driver = {
	.probe		= emc1412_probe,
	.driver		= {
		.name	= "EMC1412",
		.owner	= THIS_MODULE,
	},
};


static int __init emc1412_init(void)
{
	return platform_driver_register(&emc1412_driver);
}

static void __exit emc1412_exit(void)
{
	platform_driver_unregister(&emc1412_driver);
}

late_initcall(emc1412_init);
module_exit(emc1412_exit);

MODULE_AUTHOR("Xiang Yu <xiangy@lemote.com>");
MODULE_DESCRIPTION("EMC1412 driver");
MODULE_LICENSE("GPL");
