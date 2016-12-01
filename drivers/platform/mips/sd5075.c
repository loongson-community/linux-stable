#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <boot_param.h>
#include <loongson_hwmon.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>

static struct i2c_client *sd5075_client = NULL;

static struct device *sd5075_hwmon_dev;

static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t get_sd5075_label(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t get_sd5075_temp(struct device *dev,
			struct device_attribute *attr, char *buf);

static SENSOR_DEVICE_ATTR(name, S_IRUGO, get_hwmon_name, NULL, 0);

static struct attribute *sd5075_hwmon_attributes[] =
{
	&sensor_dev_attr_name.dev_attr.attr,
	NULL
};

/* Hwmon device attribute group */
static struct attribute_group sd5075_hwmon_attribute_group =
{
	.attrs = sd5075_hwmon_attributes,
};

/* Hwmon device get name */
static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "sd5075\n");
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, get_sd5075_temp, NULL, 1);
static SENSOR_DEVICE_ATTR(temp1_label, S_IRUGO, get_sd5075_label, NULL, 1);

static const struct attribute *sd5075_hwmon_temp[1][3] = {
	{
		&sensor_dev_attr_temp1_input.dev_attr.attr,
		&sensor_dev_attr_temp1_label.dev_attr.attr,
		NULL
	}
};

#define BUS_MASK  0xffffffff00000000ul
#define ADDR_MASK 0x00000000fffffffful

static int sd5075_probe(struct platform_device *dev)
{
	char i2c_name[16];
	struct i2c_board_info info;
	struct i2c_adapter *adapter = NULL;
	int found = 0, i = 0, r = 0, i2c_bus;
	struct sensor_device *sdev = (struct sensor_device *)dev->dev.platform_data;

	memset(&info, 0, sizeof(struct i2c_board_info));

	i2c_bus = ((sdev->base_addr & BUS_MASK) >> 32) - 1;
	if (i2c_bus < 0)
		sprintf(i2c_name, "SMBus PIIX4");
	else 
		sprintf(i2c_name, "LS2X I2C%d", i2c_bus);

	/* get i2c_adapter */
	adapter = i2c_get_adapter(i++);
	while (adapter) {
		if (strncmp(adapter->name, i2c_name, strlen(i2c_name)) == 0) {
			found = 1;
			break;
		}

		adapter = i2c_get_adapter(i++);
	}

	if (!found)
		goto fail;

	info.addr = sdev->base_addr & ADDR_MASK;
	info.platform_data = dev->dev.platform_data;
	strncpy(info.type, "sd5075", I2C_NAME_SIZE);
	sd5075_client = i2c_new_device(adapter, &info);
	if (sd5075_client == NULL) {
		pr_err("failed to attach sd5075 sensor\n");
		goto fail;
	}

	r = sysfs_create_files(&sd5075_hwmon_dev->kobj, sd5075_hwmon_temp[0]);
	if (r)
		goto fail;
	
	/* set alert mode */
	i2c_smbus_write_byte_data(sd5075_client, 0x1, 0x80);

	printk(KERN_INFO "Success to attach sd5075 sensor\n");

	return 0;
fail:
	pr_err("Fail to found smbus controller attach sd5075 sensor\n");

	return r;
}

static void sd5075_shutdown(struct platform_device *dev)
{
	sd5075_client = NULL;
	msleep(15); /* Release I2C/SMBus resources */ 
}

static ssize_t get_sd5075_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct sensor_device *sdev = (struct sensor_device *)sd5075_client->dev.platform_data;
	return sprintf(buf, "%s\n", sdev->label);
}

int sd5075_internal_temp(int id)
{
	int reg_value = 0, temp;
	
	reg_value = i2c_smbus_read_word_swapped(sd5075_client, 0x0);

	if (reg_value & 0x8000)  
	{
		reg_value |= 0xffff0000;
		temp = (reg_value >> 4) * 1000 / 16;

	}
	else 
		temp = (reg_value >> 4) * 1000 / 16;

	return temp;
}

static ssize_t get_sd5075_temp(struct device *dev, struct device_attribute *attr, char *buf) {
	
	int id = (to_sensor_dev_attr(attr))->index -1;
	int value = sd5075_internal_temp(id);

	return sprintf(buf, "%d\n", value);

}

static struct platform_driver sd5075_driver = {
	.driver		= {
		.name	= "sd5075",
		.owner	= THIS_MODULE,
	},
	.probe		= sd5075_probe,
	.shutdown   = sd5075_shutdown,
};

static int __init sd5075_init(void)
{
	int ret;
	
	sd5075_hwmon_dev = hwmon_device_register(NULL);
	
	if (IS_ERR(sd5075_hwmon_dev)) {
		ret = -ENOMEM;
		printk(KERN_ERR "hwmon_device_register fail!\n");
		goto fail_hwmon_device_register;
	}

	ret = sysfs_create_group(&sd5075_hwmon_dev->kobj,
				&sd5075_hwmon_attribute_group);
	if (ret) {
		printk(KERN_ERR "fail to create loongson hwmon!\n");
		goto fail_sysfs_create_group_hwmon;
	}

	platform_driver_register(&sd5075_driver);

	return 	0;

fail_sysfs_create_group_hwmon:
	hwmon_device_unregister(sd5075_hwmon_dev);

fail_hwmon_device_register:
	return ret;
}

static void __exit sd5075_exit(void)
{
	platform_driver_unregister(&sd5075_driver);
	sysfs_remove_group(&sd5075_hwmon_dev->kobj,
				&sd5075_hwmon_attribute_group);
	hwmon_device_unregister(sd5075_hwmon_dev);
}

late_initcall(sd5075_init);
module_exit(sd5075_exit);

MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_AUTHOR("Liangliang Huang <huangll@lemote.com>");
MODULE_DESCRIPTION("SD5075 driver");
MODULE_LICENSE("GPL");
