/*
 * SB700/SB710/SB800 series South Bridge provide 4 Fan controllers,
 * Loongson products almost use 1/2 of them.
 *
 * Fan1 & Fan2 usage:
 * 1. configure gpio3 as gpio48 to Fanout mode
 * 2. configure fan controll mode(depmod on HW design)
 * 3. set pwm freqency
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include <boot_param.h>
#include <loongson_hwmon.h>

/* PM regs */
#define PM_OPTION_0	0x60
#define FAN0_EN		(1 << 6)
#define FAN1_EN		(1 << 2)
#define FAN2_EN		(1 << 3)

/* PM2 regs*/
#define FAN_CNTR_REG	0

#define FAN_MODE0	0
#define FAN_MODE1	1
#define FAN_MODE2	2
#define FAN_MODE3	3
#define FAN_MODE_MASK	3
#define FAN0_CNTR_SHIFT	0
#define FAN1_CNTR_SHIFT	2
#define FAN2_CNTR_SHIFT	4
#define FAN3_CNTR_SHIFT	6

#define FAN_MISC0_REG	0x01
#define FAN_MISC1_REG	0x0e
#define FAN_MISC2_REG	0x1b
#define AUTPMODE0	0
#define LINEARMODE0	1
#define FAN0POLARITY	2
#define AUTPMODE1	0
#define LINEARMODE1	1
#define FAN1POLARITY	2

#define FREQDIV0_REG	0x02
#define FREQDIV1_REG	0x0f
#define FREQDIV2_REG	0x1c
#define PWM_28KHZ	0  /* 28.64KHz */
#define PWM_25KHZ	1  /* 25.78KHz */
#define PWM_23KHZ	2  /* 23.44KHz */
#define PWM_21KHZ	3  /* 21.48KHz */
#define PWM_19KHZ	4  /* 19.83KHz */
#define PWM_18KHZ	5  /* 18.41KHz */

#define LOWDUTY0	0x03
#define LOWDUTY1	0x10
#define LOWDUTY2	0x1d

#define FAN0DETCONTROL  0x31
#define FAN0SPEEDLIMLO  0x32
#define FAN0SPEEDLIMHI  0x33
#define FAN0SPEEDLO	0x34
#define FAN0SPEEDHI	0x35

#define FAN1DETCONTROL	0x36
#define FAN1SPEEDLIMLO  0x37
#define FAN1SPEEDLIMHI  0x38
#define FAN1SPEEDLO	0x39
#define FAN1SPEEDHI	0x3A

#define FAN2DETCONTROL	0x3B
#define FAN2SPEEDLIMLO  0x3C
#define FAN2SPEEDLIMHI  0x3D
#define FAN2SPEEDLO	0x3E
#define FAN2SPEEDHI	0x3F

#define FANSTATUS	1 /* bit 0 */
#define FANTOOSLOW	1

#define FANDET_EN	1 /* bit 0 */
#define USEAVERAGE	2 /* bit 1 */
#define ShutDown_EN	0x10 /* bit 4 */

#define MAX_SBX00_FANS 3
int sbx00_fan_enable[MAX_SBX00_FANS];
static enum fan_control_mode sbx00_fan_mode[MAX_SBX00_FANS];
static struct loongson_fan_policy fan_policy[MAX_SBX00_FANS];

/* threshold = SpeedOfPWM(0)/SpeedOfPWM(255) */
static int speed_percent_threshold[MAX_SBX00_FANS];

extern u8 pm_ioread(u8 reg);
extern u8 pm2_ioread(u8 reg);
extern void pm_iowrite(u8 reg, u8 val);
extern void pm2_iowrite(u8 reg, u8 val);

/* up_temp & down_temp used in fan auto adjust */
static u8 fan_up_temp[MAX_SBX00_FANS];
static u8 fan_down_temp[MAX_SBX00_FANS];
static u8 fan_up_temp_level[MAX_SBX00_FANS];
static u8 fan_down_temp_level[MAX_SBX00_FANS];

static void fan1_adjust(struct work_struct *work);
static void fan2_adjust(struct work_struct *work);
static void fan3_adjust(struct work_struct *work);

static struct device *sbx00_hwmon_dev;

static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf);
static SENSOR_DEVICE_ATTR(name, S_IRUGO, get_hwmon_name, NULL, 0);

static struct attribute *sbx00_hwmon_attributes[] =
{
	&sensor_dev_attr_name.dev_attr.attr,
	NULL
};

/* Hwmon device attribute group */
static struct attribute_group sbx00_hwmon_attribute_group =
{
	.attrs = sbx00_hwmon_attributes,
};

/* Hwmon device get name */
static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "sbx00-fan\n");
}

/* low-level fucntions */
static void sbx00_set_fan_level(u8 level, int id)
{
	if (id == 0)
		pm2_iowrite(LOWDUTY0, level);
	if (id == 1)
		pm2_iowrite(LOWDUTY1, level);
	if (id == 2)
		pm2_iowrite(LOWDUTY2, level);
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

static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR | S_IRUGO,
				get_fan_level, set_fan_level, 2);
static SENSOR_DEVICE_ATTR(pwm2_enable, S_IWUSR | S_IRUGO,
				get_fan_mode, set_fan_mode, 2);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, get_fan_speed, NULL, 2);

static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR | S_IRUGO,
				get_fan_level, set_fan_level, 3);
static SENSOR_DEVICE_ATTR(pwm3_enable, S_IWUSR | S_IRUGO,
				get_fan_mode, set_fan_mode, 3);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, get_fan_speed, NULL, 3);

static const struct attribute *sbx00_hwmon_fan[3][4] = {
	{
		&sensor_dev_attr_pwm1.dev_attr.attr,
		&sensor_dev_attr_pwm1_enable.dev_attr.attr,
		&sensor_dev_attr_fan1_input.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_pwm2.dev_attr.attr,
		&sensor_dev_attr_pwm2_enable.dev_attr.attr,
		&sensor_dev_attr_fan2_input.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_pwm3.dev_attr.attr,
		&sensor_dev_attr_pwm3_enable.dev_attr.attr,
		&sensor_dev_attr_fan3_input.dev_attr.attr,
		NULL
	}
};

static ssize_t get_fan_level(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u8 val = 0;
	int id = (to_sensor_dev_attr(attr))->index - 1;

	if (id == 0)
		val = pm2_ioread(LOWDUTY0);
	if (id == 1)
		val = pm2_ioread(LOWDUTY1);
	if (id == 2)
		val = pm2_ioread(LOWDUTY2);

	return sprintf(buf, "%d\n", val);
}

static ssize_t set_fan_level(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_level;
	int id = (to_sensor_dev_attr(attr))->index - 1;

	new_level = clamp_val(simple_strtoul(buf, NULL, 10), 0, 255);
	if (sbx00_fan_mode[id] == FAN_MANUAL_MODE) {
		if (id == 0)
			pm2_iowrite(LOWDUTY0, new_level);
		if (id == 1)
			pm2_iowrite(LOWDUTY1, new_level);
		if (id == 2)
			pm2_iowrite(LOWDUTY2, new_level);
	}

	return count;
}

static ssize_t get_fan_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int id = (to_sensor_dev_attr(attr))->index - 1;
	return sprintf(buf, "%d\n", sbx00_fan_mode[id]);
}

static void sbx00_fan_step_mode(get_temp_fun fun,
			struct loongson_fan_policy *policy, int id)
{
	u8 current_temp;
	int init_temp_level, target_fan_level, bias;

	current_temp = fun(0) / 1000;
	get_up_temp(policy, current_temp, &fan_up_temp[id],
			&fan_up_temp_level[id]);
	get_down_temp(policy, current_temp, &fan_down_temp[id],
			&fan_down_temp_level[id]);

	/* current speed is not sure, setting now */
	init_temp_level = (fan_up_temp_level[id] + fan_down_temp_level[id]) / 2;
	target_fan_level = policy->up_step[init_temp_level].level * 255 / 100;
	bias = (speed_percent_threshold[id] * 255) / 100; /* bias of pwm(0) */
	bias = bias * (255 - target_fan_level) / 255;     /* bias of pwm(target) */
	target_fan_level = (target_fan_level > bias) ? target_fan_level - bias : 0;
	target_fan_level = target_fan_level * fan_policy[id].percent / 100;
	sbx00_set_fan_level(target_fan_level, id);

	schedule_delayed_work_on(0, &policy->work, policy->adjust_period * HZ);
}

static void sbx00_fan_start_auto(int id)
{
	u8 level;

	switch (fan_policy[id].type) {
	case CONSTANT_SPEED_POLICY:
		level = fan_policy[id].percent * 255 / 100;
		if (level > MAX_FAN_LEVEL)
			level = MAX_FAN_LEVEL;
		sbx00_set_fan_level(level, id);
		break;
	case STEP_SPEED_POLICY:
		sbx00_fan_step_mode(fan_policy[id].depend_temp, &fan_policy[id], id);
		break;
	default:
		printk(KERN_ERR "sbx00 fan not support fan policy id %d!\n", fan_policy[id].type);
		sbx00_set_fan_level(MAX_FAN_LEVEL, id);
	}

	return;
}

static void sbx00_fan_stop_auto(int id)
{
	if ((fan_policy[id].type == STEP_SPEED_POLICY) &&
			(sbx00_fan_mode[id] == FAN_AUTO_MODE)) {
		cancel_delayed_work(&fan_policy[id].work);
	}
}

static ssize_t set_fan_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_mode;
	int id = (to_sensor_dev_attr(attr))->index - 1;

	new_mode = clamp_val(simple_strtoul(buf, NULL, 10),
					FAN_FULL_MODE, FAN_AUTO_MODE);
	if (new_mode == sbx00_fan_mode[id])
		return count;

	switch (new_mode) {
	case FAN_FULL_MODE:
		sbx00_fan_stop_auto(id);
		sbx00_set_fan_level(MAX_FAN_LEVEL, id);
		break;
	case FAN_MANUAL_MODE:
		sbx00_fan_stop_auto(id);
		break;
	case FAN_AUTO_MODE:
		sbx00_fan_start_auto(id);
		break;
	default:
		break;
	}

	sbx00_fan_mode[id] = new_mode;

	return count;
}

static u32 sbx00_get_fan_speed(int id, bool warn)
{
	int speed_lo = 0, speed_hi = 0, count = 1;

	if (id == 0) {
		speed_lo = pm2_ioread(FAN0SPEEDLO);
		speed_hi = pm2_ioread(FAN0SPEEDHI);
	}
	if (id == 1) {
		speed_lo = pm2_ioread(FAN1SPEEDLO);
		speed_hi = pm2_ioread(FAN1SPEEDHI);
	}
	if (id == 2) {
		speed_lo = pm2_ioread(FAN2SPEEDLO);
		speed_hi = pm2_ioread(FAN2SPEEDHI);
	}

	if (speed_lo & FANTOOSLOW) {
		if (warn)
			pr_warn("FanSpeed too slow!\n");
		return 0;
	}
	count = speed_hi << 8 | (speed_lo & ~FANSTATUS);
	return 360000 / count;
}

static ssize_t get_fan_speed(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int id = (to_sensor_dev_attr(attr))->index - 1;
	u32 val = sbx00_get_fan_speed(id, true);

	return sprintf(buf, "%d\n", val);
}

static void sbx00_fan1_init(struct work_struct *work)
{
	u8 reg;
	int ret;
	int speed_low, speed_high;

	/* Configure gpio3 as FANOUT0 */
	reg = pm_ioread(PM_OPTION_0);
	pm_iowrite(PM_OPTION_0, reg | FAN0_EN);

	/* setting FANOUT0 mode:
	 * without temperuature input, controlled by software
	 */
	reg = pm2_ioread(FAN_CNTR_REG);
	reg &= ~(FAN_MODE_MASK << FAN0_CNTR_SHIFT);
	reg |= FAN_MODE1 << FAN0_CNTR_SHIFT;
	pm2_iowrite(FAN_CNTR_REG, reg);

	reg = pm2_ioread(FAN_MISC0_REG);
	reg &= ~(1 << AUTPMODE0);   /* Disable auto mode*/
	reg &= ~(1 << LINEARMODE0); /* Use step mode*/
	reg |= (1 << FAN0POLARITY); /* Active High */
	pm2_iowrite(FAN_MISC0_REG, reg);

	/* Set PWM freqency*/
	pm2_iowrite(FREQDIV0_REG, PWM_19KHZ);

	/* Enable Speed Detect */
	pm2_iowrite(FAN0DETCONTROL, FANDET_EN | USEAVERAGE);
	pm2_iowrite(FAN0SPEEDLIMLO, 0xFF); /* Set limits register */
	pm2_iowrite(FAN0SPEEDLIMHI, 0xFF);

	sbx00_set_fan_level(255, 0);
	msleep(250);
	speed_high = sbx00_get_fan_speed(0, false);
	sbx00_set_fan_level(0, 0);
	msleep(5000);
	speed_low = sbx00_get_fan_speed(0, false);
	sbx00_set_fan_level(255, 0);
	if (speed_high >= speed_low)
		speed_percent_threshold[0] = speed_low * 100 / speed_high;
	pr_info("SpeedPercentThreshold of FAN1 is %d%%\n", speed_percent_threshold[0]);

	/* force fans in auto mode first */
	work->func = fan1_adjust;
	sbx00_fan_mode[0] = FAN_AUTO_MODE;
	sbx00_fan_start_auto(0);

	ret = sysfs_create_files(&sbx00_hwmon_dev->kobj, sbx00_hwmon_fan[0]);
	if (ret)
		printk(KERN_ERR "fail to create sysfs files\n");
}

static void fan1_adjust(struct work_struct *work)
{
	u8 current_temp;
	int target_fan_level, bias;

	current_temp = fan_policy[0].depend_temp(0) / 1000;

	if ((current_temp <= fan_up_temp[0]) &&
		(current_temp >= fan_down_temp[0]))
		goto exit;

	if (current_temp > fan_up_temp[0])
		target_fan_level = fan_policy[0].up_step[fan_up_temp_level[0]].level * 255 / 100;

	if (current_temp < fan_down_temp[0])
		target_fan_level = fan_policy[0].down_step[fan_down_temp_level[0]].level * 255 / 100;

	bias = (speed_percent_threshold[0] * 255) / 100;
	bias = bias * (255 - target_fan_level) / 255;
	target_fan_level = (target_fan_level > bias) ? target_fan_level - bias : 0;
	target_fan_level = target_fan_level * fan_policy[0].percent / 100;
	sbx00_set_fan_level(target_fan_level, 0);

	get_up_temp(&fan_policy[0], current_temp, &fan_up_temp[0], &fan_up_temp_level[0]);
	get_down_temp(&fan_policy[0], current_temp, &fan_down_temp[0], &fan_down_temp_level[0]);

exit:
        schedule_delayed_work_on(0, &fan_policy[0].work, fan_policy[0].adjust_period * HZ);
}

static void sbx00_fan2_init(struct work_struct *work)
{
	u8 reg;
	int ret;
	int speed_low, speed_high;

	/* Configure gpio48 as FANOUT1 */
	reg = pm_ioread(PM_OPTION_0);
	pm_iowrite(PM_OPTION_0, reg | FAN1_EN);

	/* setting FANOUT1 mode:
	 * without temperuature input, controlled by software
	 */
	reg = pm2_ioread(FAN_CNTR_REG);
	reg &= ~(FAN_MODE_MASK << FAN1_CNTR_SHIFT);
	reg |= FAN_MODE1 << FAN1_CNTR_SHIFT;
	pm2_iowrite(FAN_CNTR_REG, reg);

	reg = pm2_ioread(FAN_MISC1_REG);
	reg &= ~(1 << AUTPMODE1);   /* Disable auto mode*/
	reg &= ~(1 << LINEARMODE1); /* Use step mode*/
	reg |= (1 << FAN1POLARITY); /* Active High */
	pm2_iowrite(FAN_MISC1_REG, reg);

	/* Set PWM freqency at 19.83KHz*/
	pm2_iowrite(FREQDIV1_REG, PWM_19KHZ);

	/* Enable Speed Detect */
	pm2_iowrite(FAN1DETCONTROL, FANDET_EN | USEAVERAGE);
	pm2_iowrite(FAN1SPEEDLIMLO, 0xFF); /* Set limits register */
	pm2_iowrite(FAN1SPEEDLIMHI, 0xFF);

	sbx00_set_fan_level(255, 1);
	msleep(250);
	speed_high = sbx00_get_fan_speed(1, false);
	sbx00_set_fan_level(0, 1);
	msleep(5000);
	speed_low = sbx00_get_fan_speed(1, false);
	sbx00_set_fan_level(255, 1);
	if (speed_high >= speed_low)
		speed_percent_threshold[1] = speed_low * 100 / speed_high;
	pr_info("SpeedPercentThreshold of FAN2 is %d%%\n", speed_percent_threshold[1]);

	/* force fans in auto mode first */
	work->func = fan2_adjust;
	sbx00_fan_mode[1] = FAN_AUTO_MODE;
	sbx00_fan_start_auto(1);

	ret = sysfs_create_files(&sbx00_hwmon_dev->kobj, sbx00_hwmon_fan[1]);
	if (ret)
		printk(KERN_ERR "fail to create sysfs files\n");
}

static void fan2_adjust(struct work_struct *work)
{
	u8 current_temp;
	int target_fan_level, bias;

	current_temp = fan_policy[1].depend_temp(0) / 1000;

	if ((current_temp <= fan_up_temp[1]) &&
		(current_temp >= fan_down_temp[1]))
		goto exit;

	if (current_temp > fan_up_temp[1])
		target_fan_level = fan_policy[1].up_step[fan_up_temp_level[1]].level * 255 / 100;

	if (current_temp < fan_down_temp[1])
		target_fan_level = fan_policy[1].down_step[fan_down_temp_level[1]].level * 255 / 100;

	bias = (speed_percent_threshold[1] * 255) / 100;
	bias = bias * (255 - target_fan_level) / 255;
	target_fan_level = (target_fan_level > bias) ? target_fan_level - bias : 0;
	target_fan_level = target_fan_level * fan_policy[1].percent / 100;
	sbx00_set_fan_level(target_fan_level, 1);

	get_up_temp(&fan_policy[1], current_temp, &fan_up_temp[1], &fan_up_temp_level[1]);
	get_down_temp(&fan_policy[1], current_temp, &fan_down_temp[1], &fan_down_temp_level[1]);

exit:
        schedule_delayed_work_on(0, &fan_policy[1].work, fan_policy[1].adjust_period * HZ);
}

static void sbx00_fan3_init(struct work_struct *work)
{
	u8 reg;
	int ret;
	int speed_low, speed_high;

	/* Configure gpio49 as FANOUT2 */
	reg = pm_ioread(PM_OPTION_0);
	pm_iowrite(PM_OPTION_0, reg | FAN2_EN);

	/* setting FANOUT2 mode:
	 * without temperuature input, controlled by software
	 */
	reg = pm2_ioread(FAN_CNTR_REG);
	reg &= ~(FAN_MODE_MASK << FAN2_CNTR_SHIFT);
	reg |= FAN_MODE1 << FAN2_CNTR_SHIFT;
	pm2_iowrite(FAN_CNTR_REG, reg);

	reg = pm2_ioread(FAN_MISC2_REG);
	reg &= ~(1 << AUTPMODE1);   /* Disable auto mode*/
	reg &= ~(1 << LINEARMODE1); /* Use step mode*/
	reg |= (1 << FAN1POLARITY); /* Active High */
	pm2_iowrite(FAN_MISC2_REG, reg);

	/* Set PWM freqency at 19.83KHz*/
	pm2_iowrite(FREQDIV2_REG, PWM_19KHZ);

	/* Enable Speed Detect */
	pm2_iowrite(FAN2DETCONTROL, FANDET_EN | USEAVERAGE);
	pm2_iowrite(FAN2SPEEDLIMLO, 0xFF); /* Set limits register */
	pm2_iowrite(FAN2SPEEDLIMHI, 0xFF);

	sbx00_set_fan_level(255, 2);
	msleep(250);
	speed_high = sbx00_get_fan_speed(2, false);
	sbx00_set_fan_level(0, 2);
	msleep(5000);
	speed_low = sbx00_get_fan_speed(2, false);
	sbx00_set_fan_level(255, 2);
	if (speed_high >= speed_low)
		speed_percent_threshold[2] = speed_low * 100 / speed_high;
	pr_info("SpeedPercentThreshold of FAN3 is %d%%\n", speed_percent_threshold[2]);

	/* force fans in auto mode first */
	work->func = fan3_adjust;
	sbx00_fan_mode[2] = FAN_AUTO_MODE;
	sbx00_fan_start_auto(2);

	ret = sysfs_create_files(&sbx00_hwmon_dev->kobj, sbx00_hwmon_fan[2]);
	if (ret)
		printk(KERN_ERR "fail to create sysfs files\n");
}

static void fan3_adjust(struct work_struct *work)
{
	u8 current_temp;
	int target_fan_level, bias;

	current_temp =  fan_policy[2].depend_temp(0) / 1000;

	if ((current_temp <= fan_up_temp[2]) &&
		(current_temp >= fan_down_temp[2]))
		goto exit;

	if (current_temp > fan_up_temp[2])
		target_fan_level = fan_policy[2].up_step[fan_up_temp_level[2]].level * 255 / 100;

	if (current_temp < fan_down_temp[2])
		target_fan_level = fan_policy[2].down_step[fan_down_temp_level[2]].level * 255 / 100;

	bias = (speed_percent_threshold[2] * 255) / 100;
	bias = bias * (255 - target_fan_level) / 255;
	target_fan_level = (target_fan_level > bias) ? target_fan_level - bias : 0;
	target_fan_level = target_fan_level * fan_policy[2].percent / 100;
	sbx00_set_fan_level(target_fan_level, 2);

	get_up_temp(&fan_policy[2], current_temp, &fan_up_temp[2], &fan_up_temp_level[2]);
	get_down_temp(&fan_policy[2], current_temp, &fan_down_temp[2], &fan_down_temp_level[2]);

exit:
        schedule_delayed_work_on(0, &fan_policy[2].work, fan_policy[2].adjust_period * HZ);
}

static int sbx00_fan_probe(struct platform_device *dev)
{
	int id = dev->id - 1;
	struct sensor_device *sdev = (struct sensor_device *)dev->dev.platform_data;

	/* get fan policy */
	switch (sdev->fan_policy) {
	case KERNEL_HELPER_POLICY:
		memcpy(&fan_policy[id], &kernel_helper_policy, sizeof(struct loongson_fan_policy));
		break;
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

	if (id == 0) {
		INIT_DEFERRABLE_WORK(&fan_policy[0].work, sbx00_fan1_init);
		schedule_delayed_work_on(0, &fan_policy[0].work, 0);
	}
	if (id == 1) {
		INIT_DEFERRABLE_WORK(&fan_policy[1].work, sbx00_fan2_init);
		schedule_delayed_work_on(0, &fan_policy[1].work, 0);
	}
	if (id == 2) {
		INIT_DEFERRABLE_WORK(&fan_policy[2].work, sbx00_fan3_init);
		schedule_delayed_work_on(0, &fan_policy[2].work, 0);
	}

	sbx00_fan_enable[id] = 1;

	return 0;
}

static struct platform_driver sbx00_fan_driver = {
	.probe		= sbx00_fan_probe,
	.driver		= {
		.name	= "sbx00-fan",
		.owner	= THIS_MODULE,
	},
};

static int __init sbx00_fan_init(void)
{
	int ret;

	sbx00_hwmon_dev = hwmon_device_register(NULL);
	if (IS_ERR(sbx00_hwmon_dev)) {
		ret = -ENOMEM;
		printk(KERN_ERR "hwmon_device_register fail!\n");
		goto fail_hwmon_device_register;
	}

	ret = sysfs_create_group(&sbx00_hwmon_dev->kobj,
				&sbx00_hwmon_attribute_group);
	if (ret) {
		printk(KERN_ERR "fail to create loongson hwmon!\n");
		goto fail_sysfs_create_group_hwmon;
	}

	ret = platform_driver_register(&sbx00_fan_driver);
	if (ret) {
		printk(KERN_ERR "fail to register fan driver!\n");
		goto fail_register_fan;
	}

	return 0;

fail_register_fan:
	sysfs_remove_group(&sbx00_hwmon_dev->kobj,
				&sbx00_hwmon_attribute_group);

fail_sysfs_create_group_hwmon:
	hwmon_device_unregister(sbx00_hwmon_dev);

fail_hwmon_device_register:
	return ret;
}

static void __exit sbx00_fan_exit(void)
{
	/* set fan at full speed before module exit */
	if (sbx00_fan_enable[0]) {
		sbx00_fan_mode[0] = FAN_FULL_MODE;
		sbx00_fan_stop_auto(0);
		sbx00_set_fan_level(MAX_FAN_LEVEL, 0);
	}

	if (sbx00_fan_enable[1]) {
		sbx00_fan_mode[1] = FAN_FULL_MODE;
		sbx00_fan_stop_auto(1);
		sbx00_set_fan_level(MAX_FAN_LEVEL, 1);
	}

	if (sbx00_fan_enable[2]) {
		sbx00_fan_mode[2] = FAN_FULL_MODE;
		sbx00_fan_stop_auto(2);
		sbx00_set_fan_level(MAX_FAN_LEVEL, 2);
	}

	platform_driver_unregister(&sbx00_fan_driver);
	sysfs_remove_group(&sbx00_hwmon_dev->kobj,
				&sbx00_hwmon_attribute_group);
	hwmon_device_unregister(sbx00_hwmon_dev);
}

late_initcall(sbx00_fan_init);
module_exit(sbx00_fan_exit);

MODULE_AUTHOR("Yu Xiang <xiangy@lemote.com>");
MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("SB700/SB710/SB800 fan control driver");
MODULE_LICENSE("GPL");
