#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include <boot_param.h>
#include <ec_wpce775l.h>
#include <loongson_hwmon.h>

#ifdef MAX_FAN_LEVEL
#undef MAX_FAN_LEVEL
#endif
#define MAX_FAN_LEVEL 5

int fan_enable;
enum fan_control_mode fan_mode;
static struct loongson_fan_policy fan_policy;

static struct device *wpce775l_hwmon_dev;

static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf);
static SENSOR_DEVICE_ATTR(name, S_IRUGO, get_hwmon_name, NULL, 0);

static struct attribute *wpce775l_hwmon_attributes[] =
{
	&sensor_dev_attr_name.dev_attr.attr,
	NULL
};

/* Hwmon device attribute group */
static struct attribute_group wpce775l_hwmon_attribute_group =
{
	.attrs = wpce775l_hwmon_attributes,
};

/* Hwmon device get name */
static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "wpce775l-fan\n");
}

static ssize_t get_fan_level(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan_level(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan_mode(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan_mode(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan_speed(struct device *dev,
			struct device_attribute *attr, char *buf);

static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR | S_IRUGO,
				get_fan_level, set_fan_level, 1);
static SENSOR_DEVICE_ATTR(pwm1_enable, S_IWUSR | S_IRUGO,
				get_fan_mode, set_fan_mode, 1);
static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, get_fan_speed, NULL, 1);

static const struct attribute *hwmon_fan1[] = {
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_pwm1_enable.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	NULL
};

static struct workqueue_struct *notify_workqueue;
static void notify_temp(struct work_struct *work);
static DECLARE_DELAYED_WORK(notify_work, notify_temp);

static int wpce_set_fan_level(u8 level)
{
	if (level > MAX_FAN_LEVEL)
		level = MAX_FAN_LEVEL;

	ec_write(INDEX_FAN_SPEED_LEVEL, level);
	return 0;
}

static ssize_t get_fan_level(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u8 val;

	val = ec_read(INDEX_FAN_SPEED_LEVEL);
	return sprintf(buf, "%d\n", val);
}

static ssize_t set_fan_level(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_level;

	new_level = clamp_val(simple_strtoul(buf, NULL, 10), 0, 255);
	if (fan_mode == FAN_MANUAL_MODE)
		wpce_set_fan_level(new_level);

	return count;
}

static ssize_t get_fan_speed(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 val;

	val = (ec_read(INDEX_FAN_SPEED_HIGH) << 8) +
			ec_read(INDEX_FAN_SPEED_LOW);
	return sprintf(buf, "%d\n", val);
}

static void notify_temp(struct work_struct *work)
{
	u8 temp;

	temp =  fan_policy.depend_temp(0) / 1000;

	ec_write_noindex(0x4d, temp);

	queue_delayed_work(notify_workqueue, &notify_work,
				fan_policy.adjust_period * HZ);
}

static int notify_temp_to_EC(void)
{
	notify_workqueue = create_singlethread_workqueue("Temprature Notify");
	queue_delayed_work(notify_workqueue, &notify_work, HZ);

	return 0;
}

static int kernel_control_fan(void)
{
	ec_write(INDEX_FAN_CTRLMOD,FAN_CTRL_BYHOST);
	return 0;
}

static int ec_control_fan(void)
{
	ec_write(INDEX_FAN_CTRLMOD,FAN_CTRL_BYEC);
	return 0;
}

static void fan_start_auto(void)
{
	ec_control_fan();

	switch (fan_policy.type) {
	case KERNEL_HELPER_POLICY:
		notify_temp_to_EC();
		break;
	default:
		printk(KERN_ERR "wpce fan not support fan policy id %d!\n", fan_policy.type);
		wpce_set_fan_level(MAX_FAN_LEVEL);
	}

	return;
}

static void fan_stop_auto(void)
{
	if ((fan_policy.type == KERNEL_HELPER_POLICY) &&
		       (fan_mode == FAN_AUTO_MODE)) {
			cancel_delayed_work(&notify_work);
			destroy_workqueue(notify_workqueue);
	}

	kernel_control_fan();
}

static ssize_t set_fan_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_mode;

	new_mode = clamp_val(simple_strtoul(buf, NULL, 10),
					FAN_FULL_MODE, FAN_AUTO_MODE);

	if (new_mode == fan_mode)
		return count;

	switch (new_mode) {
	case FAN_FULL_MODE:
		fan_stop_auto();
		wpce_set_fan_level(MAX_FAN_LEVEL);
		break;
	case FAN_MANUAL_MODE:
		fan_stop_auto();
		break;
	case FAN_AUTO_MODE:
		fan_start_auto();
		break;
	default:
		break;
	}

	fan_mode = new_mode;

	return count;
}

static ssize_t get_fan_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", fan_mode);
}

static int fan_probe(struct platform_device *dev)
{
	int ret;
	struct sensor_device *sdev = (struct sensor_device *)dev->dev.platform_data;

	/* get fan policy */
	switch (sdev->fan_policy) {
	case KERNEL_HELPER_POLICY:
		memcpy(&fan_policy, &kernel_helper_policy, sizeof(fan_policy));
		break;
	case STEP_SPEED_POLICY:
		memcpy(&fan_policy, &step_speed_policy, sizeof(fan_policy));
		break;
	case CONSTANT_SPEED_POLICY:
	default:
		memcpy(&fan_policy, &constant_speed_policy, sizeof(fan_policy));
		fan_policy.percent = sdev->fan_percent;
		if (fan_policy.percent == 0 || fan_policy.percent > 100)
			fan_policy.percent = 100;
		break;
	}

	/* force fan in auto mode first */
	fan_mode = FAN_AUTO_MODE;
	fan_start_auto();

	ret = sysfs_create_files(&wpce775l_hwmon_dev->kobj, hwmon_fan1);
	if (ret) {
		printk(KERN_ERR "fail to create sysfs files\n");
		return ret;
	}

	return 0;
}

static struct platform_driver fan_driver = {
	.probe		= fan_probe,
	.driver		= {
		.name	= "wpce-fan",
		.owner	= THIS_MODULE,
	},
};

static int __init wpce_fan_init(void)
{
	int ret;

	wpce775l_hwmon_dev = hwmon_device_register(NULL);
	if (IS_ERR(wpce775l_hwmon_dev)) {
		ret = -ENOMEM;
		printk(KERN_ERR "hwmon_device_register fail!\n");
		goto fail_hwmon_device_register;
	}

	ret = sysfs_create_group(&wpce775l_hwmon_dev->kobj,
				&wpce775l_hwmon_attribute_group);
	if (ret) {
		printk(KERN_ERR "fail to create loongson hwmon!\n");
		goto fail_sysfs_create_group_hwmon;
	}

	ret = platform_driver_register(&fan_driver);
	if (ret) {
		printk(KERN_ERR "fail to register fan driver!\n");
		goto fail_register_fan;
	}

	return 0;

fail_register_fan:
	sysfs_remove_group(&wpce775l_hwmon_dev->kobj,
				&wpce775l_hwmon_attribute_group);

fail_sysfs_create_group_hwmon:
	hwmon_device_unregister(wpce775l_hwmon_dev);

fail_hwmon_device_register:
	return ret;
}

static void __exit wpce_fan_exit(void)
{
	/* set fan at full speed mode before module exit */
	if (fan_enable)
		fan_mode = FAN_FULL_MODE;
	fan_stop_auto();
	wpce_set_fan_level(MAX_FAN_LEVEL);

	platform_driver_unregister(&fan_driver);
	sysfs_remove_group(&wpce775l_hwmon_dev->kobj,
				&wpce775l_hwmon_attribute_group);
	hwmon_device_unregister(wpce775l_hwmon_dev);
}

late_initcall(wpce_fan_init);
module_exit(wpce_fan_exit);

MODULE_AUTHOR("Yu Xiang <xiangy@lemote.com>");
MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("WPCE775L fan control driver");
MODULE_LICENSE("GPL");
