#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <loongson_hwmon.h>

struct i2c_client *emc1412_client[5] = {NULL};
unsigned short normal_list[] = {0x1C, 0x4C, 0x5C, 0x6C, I2C_CLIENT_END};

static int emc1412_detect(struct i2c_client *client,
			  struct i2c_board_info *info)
{
	u8 vendor_id, device_id;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	/* read ID */
	device_id = i2c_smbus_read_byte_data(client, 0xFD);
	if (device_id < 0) {
		pr_err("Read i2c device %s error!\n", client->name);
		return device_id;
	}
	vendor_id = i2c_smbus_read_byte_data(client, 0xFE);
	if (vendor_id < 0) {
		pr_err("Read i2c device %s error!\n", client->name);
		return vendor_id;
	}
	if (vendor_id != 0x5D || device_id != 0x20)
		return -ENODEV;

	strncpy(info->type, "emc1412", I2C_NAME_SIZE);

	return 0;
}

static int
emc1412_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	if (client == NULL)
		goto fail;

	if (client->addr == 0x4c)
		emc1412_client[0] = client;
	if (client->addr == 0x6c)
		emc1412_client[1] = client;
	if (client->addr == 0x1c)
		emc1412_client[2] = client;
	if (client->addr == 0x5c)
		emc1412_client[3] = client;

	pr_info("Success to attach EMC1412 sensor\n");

	return 0;
fail:
	pr_warn("Fail to fount smbus controller attach EMC1412 sensor\n");

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
int emc1412_internal_temp(int id)
{
	u8 reg;
	int temp;
	struct i2c_client *client;

	if (id < 2 || !(client = emc1412_client[id - 2]))
		return NOT_VALID_TEMP;

	temp = i2c_smbus_read_byte_data(client, 0) * 1000;
	reg = i2c_smbus_read_byte_data(client, 0x29);
	temp += (reg >> 5) * 125;

	return temp;
}
EXPORT_SYMBOL_GPL(emc1412_internal_temp);

int emc1412_external_temp(int id)
{
	u8 reg;
	int temp;
	struct i2c_client *client;

	/* not ready ??? */
	if (id < 2 || !(client = emc1412_client[id - 2]))
		return NOT_VALID_TEMP;

	temp = i2c_smbus_read_byte_data(client, 1) * 1000;
	reg = i2c_smbus_read_byte_data(client, 0x10);
	temp += (reg >> 5) * 125;

	return temp;
}
EXPORT_SYMBOL_GPL(emc1412_external_temp);

static const struct i2c_device_id emc1412_id[] = {
	{ "emc1412", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, emc1412_id);

static struct i2c_driver emc1412_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver		= {
		.name	= "emc1412",
		.owner	= THIS_MODULE,
	},
	.id_table	= emc1412_id,
	.probe		= emc1412_probe,
	.detect		= emc1412_detect,
	.address_list	= normal_list,
};
module_i2c_driver(emc1412_driver);

MODULE_AUTHOR("Xiang Yu <xiangy@lemote.com>");
MODULE_DESCRIPTION("EMC1412 driver");
MODULE_LICENSE("GPL");
