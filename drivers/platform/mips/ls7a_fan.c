#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <asm/io.h>

#include <boot_param.h>
#include <loongson-pch.h>
#include <loongson_hwmon.h>

#define LS7A_PWM_REG_BASE              (void *)TO_UNCAC(LS7A_MISC_REG_BASE + 0x20000)

#define LS7A_PWM0_LOW                   0x004
#define LS7A_PWM0_FULL                  0x008
#define LS7A_PWM0_CTRL                  0x00c

#define LS7A_PWM1_LOW                   0x104
#define LS7A_PWM1_FULL                  0x108
#define LS7A_PWM1_CTRL                  0x10c

#define LS7A_PWM2_LOW                   0x204
#define LS7A_PWM2_FULL                  0x208
#define LS7A_PWM2_CTRL                  0x20c

#define LS7A_PWM3_LOW                   0x304
#define LS7A_PWM3_FULL                  0x308
#define LS7A_PWM3_CTRL                  0x30c

#define MAX_LS7A_FANS 4

static struct device *pwm_hwmon_dev;
static enum fan_control_mode ls7a_fan_mode[MAX_LS7A_FANS];
static struct loongson_fan_policy fan_policy[MAX_LS7A_FANS];

/* up_temp & down_temp used in fan auto adjust */
static u8 fan_up_temp[MAX_LS7A_FANS];
static u8 fan_down_temp[MAX_LS7A_FANS];
static u8 fan_up_temp_level[MAX_LS7A_FANS];
static u8 fan_down_temp_level[MAX_LS7A_FANS];

static void fan1_adjust(struct work_struct *work);
static void fan2_adjust(struct work_struct *work);
static void fan3_adjust(struct work_struct *work);
static void fan4_adjust(struct work_struct *work);

#define pwm_read(addr)	readl(LS7A_PWM_REG_BASE + (addr))

#define pwm_write(val, addr)				\
	do {						\
		writel(val, LS7A_PWM_REG_BASE + (addr));\
		readl(LS7A_PWM_REG_BASE + (addr));	\
	} while (0)

static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf);
static SENSOR_DEVICE_ATTR(name, S_IRUGO, get_hwmon_name, NULL, 0);

static struct attribute *pwm_hwmon_attributes[] =
{
	&sensor_dev_attr_name.dev_attr.attr,
	NULL
};

/* Hwmon device attribute group */
static struct attribute_group pwm_hwmon_attribute_group =
{
	.attrs = pwm_hwmon_attributes,
};

/* Hwmon device get name */
static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "ls7a-fan\n");
}

static ssize_t get_fan_level(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan_level(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan_mode(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan_mode(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);

static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR | S_IRUGO,
				get_fan_level, set_fan_level, 1);
static SENSOR_DEVICE_ATTR(pwm1_enable, S_IWUSR | S_IRUGO,
				get_fan_mode, set_fan_mode, 1);

static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR | S_IRUGO,
				get_fan_level, set_fan_level, 2);
static SENSOR_DEVICE_ATTR(pwm2_enable, S_IWUSR | S_IRUGO,
				get_fan_mode, set_fan_mode, 2);

static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR | S_IRUGO,
				get_fan_level, set_fan_level, 3);
static SENSOR_DEVICE_ATTR(pwm3_enable, S_IWUSR | S_IRUGO,
				get_fan_mode, set_fan_mode, 3);

static SENSOR_DEVICE_ATTR(pwm4, S_IWUSR | S_IRUGO,
				get_fan_level, set_fan_level, 4);
static SENSOR_DEVICE_ATTR(pwm4_enable, S_IWUSR | S_IRUGO,
				get_fan_mode, set_fan_mode, 4);

static const struct attribute *ls7a_hwmon_fan[4][3] = {
	{
		&sensor_dev_attr_pwm1.dev_attr.attr,
		&sensor_dev_attr_pwm1_enable.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_pwm2.dev_attr.attr,
		&sensor_dev_attr_pwm2_enable.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_pwm3.dev_attr.attr,
		&sensor_dev_attr_pwm3_enable.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_pwm4.dev_attr.attr,
		&sensor_dev_attr_pwm4_enable.dev_attr.attr,
		NULL
	}
};

static u32 ls7a_get_fan_level(int id)
{
	unsigned long low = 0, full = 0;

	if (id == 0) {
		low = pwm_read(LS7A_PWM0_LOW);
		full = pwm_read(LS7A_PWM0_FULL);
	}
	if (id == 1) {
		low = pwm_read(LS7A_PWM1_LOW);
		full = pwm_read(LS7A_PWM1_FULL);
	}
	if (id == 2) {
		low = pwm_read(LS7A_PWM2_LOW);
		full = pwm_read(LS7A_PWM2_FULL);
	}
	if (id == 3) {
		low = pwm_read(LS7A_PWM3_LOW);
		full = pwm_read(LS7A_PWM3_FULL);
	}

	return 255 * (full - low) / full;
}

static void ls7a_set_fan_level(u8 level, int id)
{
	if (id == 0){
		pwm_write(255, LS7A_PWM0_FULL);
		pwm_write(255 - level, LS7A_PWM0_LOW);
	}
	if (id == 1){
		pwm_write(255, LS7A_PWM1_FULL);
		pwm_write(255 - level, LS7A_PWM1_LOW);
	}
	if (id == 2){
		pwm_write(255, LS7A_PWM2_FULL);
		pwm_write(255 - level, LS7A_PWM2_LOW);
	}
	if (id == 3){
		pwm_write(255, LS7A_PWM3_FULL);
		pwm_write(255 - level, LS7A_PWM3_LOW);
	}

}

static void get_up_temp(struct loongson_fan_policy *policy,
			u8 current_temp, u8* up_temp, u8* up_temp_level)
{
	int i;

	for (i = 0; i < policy->up_step_num; i++) {
		if (current_temp <= policy->up_step[i].low) {
			*up_temp = policy->up_step[i].low;
			*up_temp_level = i;
			return;
		}
	}

	*up_temp = MAX_TEMP;
	*up_temp_level = policy->up_step_num - 1;
}

static void get_down_temp(struct loongson_fan_policy *policy, u8 current_temp,
				u8* down_temp, u8* down_temp_level)
{
	int i;

	for (i = policy->down_step_num-1; i >= 0; i--) {
		if (current_temp >= policy->down_step[i].high) {
			*down_temp = policy->down_step[i].high;
			*down_temp_level = i;
			return;
		}
	}

	*down_temp = MIN_TEMP;
	*down_temp_level = 0;
}


static ssize_t get_fan_level(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int id = (to_sensor_dev_attr(attr))->index - 1;
	u32 val = ls7a_get_fan_level(id);

	return sprintf(buf, "%d\n", val);
}

static ssize_t set_fan_level(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_speed;
	int id = (to_sensor_dev_attr(attr))->index - 1;

	if(!ls7a_fan_mode[id])
		return count;

	new_speed = clamp_val(simple_strtoul(buf, NULL, 10), 0, 255);

	ls7a_set_fan_level(new_speed, id);

	return count;
}

static ssize_t get_fan_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int id = (to_sensor_dev_attr(attr))->index - 1;
	return sprintf(buf, "%d\n", ls7a_fan_mode[id]);
}

static void ls7a_fan_step_mode(get_temp_fun fun,
			struct loongson_fan_policy *policy, int id)
{
	u8 current_temp;
	int init_temp_level, target_fan_level;

	current_temp = fun(0) / 1000;
	get_up_temp(policy, current_temp, &fan_up_temp[id],
			&fan_up_temp_level[id]);
	get_down_temp(policy, current_temp, &fan_down_temp[id],
			&fan_down_temp_level[id]);

	/* current speed is not sure, setting now */
	init_temp_level = (fan_up_temp_level[id] + fan_down_temp_level[id]) / 2;
	target_fan_level = policy->up_step[init_temp_level].level * 255 / 100;
	ls7a_set_fan_level(target_fan_level * policy->percent / 100, id);

	schedule_delayed_work_on(0, &policy->work, policy->adjust_period * HZ);
}

static void ls7a_fan_start_auto(int id)
{
	u8 level;

	switch (fan_policy[id].type) {
	case CONSTANT_SPEED_POLICY:
		level = fan_policy[id].percent * 255 / 100;
		if (level > MAX_FAN_LEVEL)
			level = MAX_FAN_LEVEL;
		ls7a_set_fan_level(level, id);
		break;
	case STEP_SPEED_POLICY:
		ls7a_fan_step_mode(fan_policy[id].depend_temp, &fan_policy[id], id);
		break;
	default:
		printk(KERN_ERR "ls7a fan not support fan policy id %d!\n", fan_policy[id].type);
		ls7a_set_fan_level(MAX_FAN_LEVEL, id);
	}

	return;
}

static void ls7a_fan_stop_auto(int id)
{
	if ((fan_policy[id].type == STEP_SPEED_POLICY) &&
			(ls7a_fan_mode[id] == FAN_AUTO_MODE)) {
		cancel_delayed_work(&fan_policy[id].work);
	}
}


static ssize_t set_fan_mode(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count)
{
	int id = (to_sensor_dev_attr(attr))->index - 1;
	u8 new_mode;

	new_mode = clamp_val(simple_strtoul(buf, NULL, 10),
				FAN_FULL_MODE, FAN_AUTO_MODE);
	if (new_mode == ls7a_fan_mode[id])
		return count;

	switch (new_mode) {
	case FAN_FULL_MODE:
		ls7a_fan_stop_auto(id);
		ls7a_set_fan_level(MAX_FAN_LEVEL, id);
		break;
	case FAN_MANUAL_MODE:
		ls7a_fan_stop_auto(id);
		break;
	case FAN_AUTO_MODE:
		ls7a_fan_start_auto(id);
		break;
	default:
		break;
	}

	ls7a_fan_mode[id] = new_mode;

	return count;
}

static void fan1_adjust(struct work_struct *work)
{
	u8 current_temp;
	int target_fan_level;

	current_temp = fan_policy[0].depend_temp(0) / 1000;

	if ((current_temp <= fan_up_temp[0]) &&
		(current_temp >= fan_down_temp[0]))
		goto exit;

	if (current_temp > fan_up_temp[0])
		target_fan_level = fan_policy[0].up_step[fan_up_temp_level[0]].level * 255 / 100;

	if (current_temp < fan_down_temp[0])
		target_fan_level = fan_policy[0].down_step[fan_down_temp_level[0]].level * 255 / 100;

	ls7a_set_fan_level(target_fan_level * fan_policy[0].percent / 100, 0);

	get_up_temp(&fan_policy[0], current_temp, &fan_up_temp[0], &fan_up_temp_level[0]);
	get_down_temp(&fan_policy[0], current_temp, &fan_down_temp[0], &fan_down_temp_level[0]);

exit:
        schedule_delayed_work_on(0, &fan_policy[0].work, fan_policy[0].adjust_period * HZ);
}

static void ls7a_fan1_init(void)
{
	int ret;

	pwm_write(1, LS7A_PWM0_CTRL);
	pwm_write(255, LS7A_PWM0_FULL); /* Full = 255, so Low + Pwm = 255 */

	INIT_DEFERRABLE_WORK(&fan_policy[0].work, fan1_adjust);

	/* force fans in auto mode first */
	ls7a_fan_mode[0] = FAN_AUTO_MODE;
	ls7a_fan_start_auto(0);

	ret = sysfs_create_files(&pwm_hwmon_dev->kobj, ls7a_hwmon_fan[0]);
	if (ret)
		printk(KERN_ERR "fail to create sysfs files\n");
}

static void fan2_adjust(struct work_struct *work)
{
	u8 current_temp;
	int target_fan_level;

	current_temp = fan_policy[1].depend_temp(0) / 1000;

	if ((current_temp <= fan_up_temp[1]) &&
		(current_temp >= fan_down_temp[1]))
		goto exit;

	if (current_temp > fan_up_temp[1])
		target_fan_level = fan_policy[1].up_step[fan_up_temp_level[1]].level * 255 / 100;

	if (current_temp < fan_down_temp[1])
		target_fan_level = fan_policy[1].down_step[fan_down_temp_level[1]].level * 255 / 100;

	ls7a_set_fan_level(target_fan_level * fan_policy[1].percent / 100, 1);

	get_up_temp(&fan_policy[1], current_temp, &fan_up_temp[1], &fan_up_temp_level[1]);
	get_down_temp(&fan_policy[1], current_temp, &fan_down_temp[1], &fan_down_temp_level[1]);

exit:
        schedule_delayed_work_on(0, &fan_policy[1].work, fan_policy[1].adjust_period * HZ);
}

static void ls7a_fan2_init(void)
{
	int ret;

	pwm_write(1, LS7A_PWM1_CTRL);
	pwm_write(255, LS7A_PWM1_FULL); /* Full = 255, so Low + Pwm = 255 */

	INIT_DEFERRABLE_WORK(&fan_policy[1].work, fan2_adjust);

	/* force fans in auto mode first */
	ls7a_fan_mode[1] = FAN_AUTO_MODE;
	ls7a_fan_start_auto(1);

	ret = sysfs_create_files(&pwm_hwmon_dev->kobj, ls7a_hwmon_fan[1]);
	if (ret)
		printk(KERN_ERR "fail to create sysfs files\n");
}

static void fan3_adjust(struct work_struct *work)
{
	u8 current_temp;
	int target_fan_level;

	current_temp = fan_policy[2].depend_temp(0) / 1000;

	if ((current_temp <= fan_up_temp[2]) &&
		(current_temp >= fan_down_temp[2]))
		goto exit;

	if (current_temp > fan_up_temp[2])
		target_fan_level = fan_policy[2].up_step[fan_up_temp_level[2]].level * 255 / 100;

	if (current_temp < fan_down_temp[2])
		target_fan_level = fan_policy[2].down_step[fan_down_temp_level[2]].level * 255 / 100;

	ls7a_set_fan_level(target_fan_level * fan_policy[2].percent / 100, 2);

	get_up_temp(&fan_policy[2], current_temp, &fan_up_temp[2], &fan_up_temp_level[2]);
	get_down_temp(&fan_policy[2], current_temp, &fan_down_temp[2], &fan_down_temp_level[2]);

exit:
        schedule_delayed_work_on(0, &fan_policy[2].work, fan_policy[2].adjust_period * HZ);
}

static void ls7a_fan3_init(void)
{
	int ret;

	pwm_write(1, LS7A_PWM2_CTRL);
	pwm_write(255, LS7A_PWM2_FULL); /* Full = 255, so Low + Pwm = 255 */

	INIT_DEFERRABLE_WORK(&fan_policy[2].work, fan3_adjust);

	/* force fans in auto mode first */
	ls7a_fan_mode[2] = FAN_AUTO_MODE;
	ls7a_fan_start_auto(2);

	ret = sysfs_create_files(&pwm_hwmon_dev->kobj, ls7a_hwmon_fan[2]);
	if (ret)
		printk(KERN_ERR "fail to create sysfs files\n");
}

static void fan4_adjust(struct work_struct *work)
{
	u8 current_temp;
	int target_fan_level;

	current_temp = fan_policy[3].depend_temp(0) / 1000;

	if ((current_temp <= fan_up_temp[3]) &&
		(current_temp >= fan_down_temp[3]))
		goto exit;

	if (current_temp > fan_up_temp[3])
		target_fan_level = fan_policy[3].up_step[fan_up_temp_level[3]].level * 255 / 100;

	if (current_temp < fan_down_temp[3])
		target_fan_level = fan_policy[3].down_step[fan_down_temp_level[3]].level * 255 / 100;

	ls7a_set_fan_level(target_fan_level * fan_policy[3].percent / 100, 3);

	get_up_temp(&fan_policy[3], current_temp, &fan_up_temp[3], &fan_up_temp_level[3]);
	get_down_temp(&fan_policy[3], current_temp, &fan_down_temp[3], &fan_down_temp_level[3]);

exit:
        schedule_delayed_work_on(0, &fan_policy[3].work, fan_policy[3].adjust_period * HZ);
}

static void ls7a_fan4_init(void)
{
	int ret;

	pwm_write(1, LS7A_PWM3_CTRL);
	pwm_write(255, LS7A_PWM3_FULL); /* Full = 255, so Low + Pwm = 255 */

	INIT_DEFERRABLE_WORK(&fan_policy[3].work, fan4_adjust);

	/* force fans in auto mode first */
	ls7a_fan_mode[3] = FAN_AUTO_MODE;
	ls7a_fan_start_auto(3);

	ret = sysfs_create_files(&pwm_hwmon_dev->kobj, ls7a_hwmon_fan[3]);
	if (ret)
		printk(KERN_ERR "fail to create sysfs files\n");
}

static int ls7a_fan_probe(struct platform_device *dev)
{
	int id = dev->id - 1;
	struct sensor_device *sdev = (struct sensor_device *)dev->dev.platform_data;

	/* get fan policy */
	switch (sdev->fan_policy) {
	case STEP_SPEED_POLICY:
		memcpy(&fan_policy[id], &step_speed_policy, sizeof(struct loongson_fan_policy));
		fan_policy[id].percent = sdev->fan_percent;
		if (fan_policy[id].percent == 0 || fan_policy[id].percent > 100)
			fan_policy[id].percent = 100;
		break;
	case CONSTANT_SPEED_POLICY:
	default:
		memcpy(&fan_policy[id], &constant_speed_policy, sizeof(struct loongson_fan_policy));
		fan_policy[id].percent = sdev->fan_percent;
		if (fan_policy[id].percent == 0 || fan_policy[id].percent > 100)
			fan_policy[id].percent = 100;
		break;
	}

	if (id == 0)
		ls7a_fan1_init();
	if (id == 1)
		ls7a_fan2_init();
	if (id == 2)
		ls7a_fan3_init();
	if (id == 3)
		ls7a_fan4_init();

	return 0;
}


static struct platform_driver pwm_fan_driver = {
	.probe	= ls7a_fan_probe,
	.driver = {
		   .name	= "ls7a-fan",
		   .owner	= THIS_MODULE,
	},
};


static int ls7a_fan_init(void)
{
	int ret;

	pwm_hwmon_dev = hwmon_device_register(NULL);
	if (IS_ERR(pwm_hwmon_dev)) {
		ret = -ENOMEM;
		printk(KERN_ERR "hwmon_device_register fail!\n");
		goto fail_hwmon_device_register;
	}

	ret = sysfs_create_group(&pwm_hwmon_dev->kobj,
				&pwm_hwmon_attribute_group);
	if (ret) {
		printk(KERN_ERR "fail to create loongson hwmon!\n");
		goto fail_sysfs_create_group_hwmon;
	}

	ret = platform_driver_register(&pwm_fan_driver);
	if (ret) {
		printk(KERN_ERR "fail to register fan driver!\n");
		goto fail_register_fan;
	}

	return 0;

fail_register_fan:
	sysfs_remove_group(&pwm_hwmon_dev->kobj,
				&pwm_hwmon_attribute_group);

fail_sysfs_create_group_hwmon:
	hwmon_device_unregister(pwm_hwmon_dev);

fail_hwmon_device_register:
	return ret;
}

static void ls7a_fan_exit(void)
{
	platform_driver_unregister(&pwm_fan_driver);
	sysfs_remove_group(&pwm_hwmon_dev->kobj,
				&pwm_hwmon_attribute_group);
	hwmon_device_unregister(pwm_hwmon_dev);
}

late_initcall(ls7a_fan_init);
module_exit(ls7a_fan_exit);

MODULE_AUTHOR("Sun Ce <sunc@lemote.com>");
MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("LS7A fan control driver");
MODULE_LICENSE("GPL");
