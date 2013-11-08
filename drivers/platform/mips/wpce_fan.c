#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>

#include <ec_wpce775l.h>
#include <loongson_hwmon.h>

#ifdef MAX_FAN_LEVEL
#undef MAX_FAN_LEVEL
#endif
#define MAX_FAN_LEVEL 5

int fan1_enable;
enum fan_control_mode fan1_mode;

static struct workqueue_struct *notify_workqueue;
static void notify_temp(struct work_struct *work);
static DECLARE_DELAYED_WORK(notify_work, notify_temp);

static u8 get_fan1_level(void);
static int set_fan1_level(u8);
static u32 get_fan1_speed(void);
static int set_fan1_speed(u32);
static enum fan_control_mode get_fan1_mode(void);
static int set_fan1_mode(enum fan_control_mode mode);

static struct loongson_fan_ops fan1_ops = {
	.set_fan_mode  = set_fan1_mode,
	.get_fan_mode  = get_fan1_mode,
	.set_fan_level = set_fan1_level,
	.get_fan_level = get_fan1_level,
	.set_fan_speed = set_fan1_speed,
	.get_fan_speed = get_fan1_speed,
};

static int _set_fan1_level(u8 level)
{
	if (level > MAX_FAN_LEVEL)
		level = MAX_FAN_LEVEL;

	ec_write(INDEX_FAN_SPEED_LEVEL, level);
	return 0;
}

static int set_fan1_level(u8 level)
{
	if (fan1_mode == FAN_MANUAL_MODE)
		_set_fan1_level(level);

	return 0;
}

static u8 get_fan1_level(void)
{
	return ec_read(INDEX_FAN_SPEED_LEVEL);
}

static int set_fan1_speed(u32 speed)
{
	/* not implement */
	return 0;
}

static u32 get_fan1_speed(void)
{
	return (ec_read(INDEX_FAN_SPEED_HIGH) << 8) +
			ec_read(INDEX_FAN_SPEED_LOW);
}

static void notify_temp(struct work_struct *work)
{
	u8 temp;
	struct loongson_fan_policy *policy;

	policy = fan1_ops.fan_policy;
	temp =  policy->depend_temp() / 1000;

	ec_write_noindex(0x4d, temp);

	queue_delayed_work(notify_workqueue, &notify_work,
				policy->adjust_period * HZ);
}

static int notify_temp_to_EC(void)
{
	notify_workqueue = create_singlethread_workqueue("Temprature Notify");
	queue_delayed_work(notify_workqueue, &notify_work, HZ);

	return 0;
}

static int kernel_control_fan1(void)
{
	ec_write(INDEX_FAN_CTRLMOD,FAN_CTRL_BYHOST);
	return 0;
}

static int ec_control_fan1(void)
{
	ec_write(INDEX_FAN_CTRLMOD,FAN_CTRL_BYEC);
	return 0;
}

static void fan1_start_auto(void)
{
	struct loongson_fan_policy *policy;

	policy = fan1_ops.fan_policy;
	if (policy == NULL)
		goto fan1_no_policy;

	ec_control_fan1();

	switch (policy->type) {
	case KERNEL_HELPER_POLICY:
		notify_temp_to_EC();
		break;
	default:
		printk(KERN_ERR "wpce fan1 not support fan policy id %d!\n", policy->type);
		goto fan1_no_policy;
	}

	return;

fan1_no_policy:
	_set_fan1_level(MAX_FAN_LEVEL);
	printk(KERN_ERR "fan1 have no fan policy!\n");

	return;
}

static void fan1_stop_auto(void)
{
	if ((fan1_ops.fan_policy->type == KERNEL_HELPER_POLICY) &&
		       (fan1_mode == FAN_AUTO_MODE)) {
			cancel_delayed_work(&notify_work);
			destroy_workqueue(notify_workqueue);
	}

	kernel_control_fan1();
}

static int set_fan1_mode(enum fan_control_mode mode)
{
	if (mode >= FAN_MODE_END)
		return -EINVAL;

	if (mode == fan1_mode)
		return 0;

	switch (mode) {
	case FAN_FULL_MODE:
		fan1_stop_auto();
		_set_fan1_level(MAX_FAN_LEVEL);
		break;
	case FAN_MANUAL_MODE:
		fan1_stop_auto();
		break;
	case FAN_AUTO_MODE:
		fan1_start_auto();
		break;
	default:
		break;
	}

	fan1_mode = mode;

	return 0;
}

static enum fan_control_mode get_fan1_mode(void)
{
	return fan1_mode;
}

static int __devinit fan1_probe(struct platform_device *dev)
{
	/* get fan policy */
	fan1_ops.fan_policy = loongson_fan1_ops.fan_policy;

	/* set loongson_fan1_ops */
	loongson_fan1_ops = fan1_ops;

	/* force fan1 in auto mode first */
	set_fan1_mode(FAN_AUTO_MODE);

	return 0;
}

static struct platform_driver fan1_driver = {
	.probe		= fan1_probe,
	.driver		= {
		.name	= "wpce-fan1",
		.owner	= THIS_MODULE,
	},
};

static int __init wpce_fan_init(void)
{
	int ret;

	ret = platform_driver_register(&fan1_driver);
	if (ret)
		printk(KERN_ERR "register fan1 fail\n");

	return ret;
}

static void __exit wpce_fan_exit(void)
{
	/* set fan at full speed mode before module exit */
	if (fan1_enable)
		set_fan1_mode(FAN_FULL_MODE);

	platform_driver_unregister(&fan1_driver);
}

module_init(wpce_fan_init);
module_exit(wpce_fan_exit);

MODULE_AUTHOR("Xiang Yu <xiangy@lemote.com>");
MODULE_DESCRIPTION("WPCE775L fan control driver");
MODULE_LICENSE("GPL");
