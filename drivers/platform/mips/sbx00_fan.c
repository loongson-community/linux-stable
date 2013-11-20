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

#include <loongson_hwmon.h>

/* PM regs */
#define PM_OPTION_0	0x60
#define FAN0_EN		(1 << 6)
#define FAN1_EN		(1 << 2)

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

#define FAN_MISC0_REG	1
#define FAN_MISC1_REG	0xe
#define AUTPMODE0	0
#define LINEARMODE0	1
#define FAN0POLARITY	2
#define AUTPMODE1	0
#define LINEARMODE1	1
#define FAN1POLARITY	2

#define FREQDIV0_REG	2
#define FREQDIV1_REG	0xf
#define PWM_28KHZ	0  /* 28.64KHz */
#define PWM_25KHZ	1  /* 25.78KHz */
#define PWM_23KHZ	2  /* 23.44KHz */
#define PWM_21KHZ	3  /* 21.48KHz */
#define PWM_19KHZ	4  /* 19.83KHz */
#define PWM_18KHZ	5  /* 18.41KHz */

#define LOWDUTY0	3
#define LOWDUTY1	0x10

int sbx00_fan1_enable, sbx00_fan2_enable;
enum fan_control_mode sbx00_fan1_mode, sbx00_fan2_mode;

extern u8 pm_ioread(u8 reg);
extern u8 pm2_ioread(u8 reg);
extern void pm_iowrite(u8 reg, u8 val);
extern void pm2_iowrite(u8 reg, u8 val);

static struct workqueue_struct *check_workqueue;
static void fan1_adjust(struct work_struct *work);
static DECLARE_DELAYED_WORK(check_work, fan1_adjust);

/*  ==================== fan 1 ======================== */

static u8 sbx00_get_fan1_level(void);
static int sbx00_set_fan1_level(u8 level);
static enum fan_control_mode sbx00_get_fan1_mode(void);
static int sbx00_set_fan1_mode(enum fan_control_mode mode);

/* up_temp & down_temp used in fan auto adjust */
u8 fan1_up_temp, fan1_down_temp;
u8 fan1_up_temp_level, fan1_down_temp_level;

static struct loongson_fan_ops sbx00_fan1_ops = {
	.get_fan_level	= sbx00_get_fan1_level,
	.set_fan_level	= sbx00_set_fan1_level,
	.get_fan_mode	= sbx00_get_fan1_mode,
	.set_fan_mode	= sbx00_set_fan1_mode,
};

static void sbx00_fan1_init(void)
{
	u8 reg;

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
}

/* low-level fucntions */
static u8 _sbx00_get_fan1_level(void)
{
	return pm2_ioread(LOWDUTY0);
}

static int _sbx00_set_fan1_level(u8 level)
{
	pm2_iowrite(LOWDUTY0, level);
	return 0;
}

/* high-level functions */
static u8 sbx00_get_fan1_level(void)
{
	return _sbx00_get_fan1_level();
}

/* Do nothing if not at manual mode */
static int sbx00_set_fan1_level(u8 level)
{
	if (sbx00_fan1_mode == FAN_MANUAL_MODE)
		_sbx00_set_fan1_level(level);

	return 0;
}

static void get_up_temp(struct loongson_fan_policy *policy,
			u8 current_temp, u8* up_temp, u8* up_temp_level)
{
	int i;

	for (i = 1; i < policy->up_step_num; i++) {
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

static void fan1_adjust(struct work_struct *work)
{
	u8 current_temp;
	struct loongson_fan_policy *policy;

	policy = sbx00_fan1_ops.fan_policy;
	current_temp =  policy->depend_temp() / 1000;

	if ((current_temp >= fan1_down_temp) &&
		(current_temp <= fan1_up_temp))
		goto exit;

	if (current_temp < fan1_down_temp)
		_sbx00_set_fan1_level(policy->up_step[fan1_down_temp_level].level * 256 / 100);

	if (current_temp > fan1_up_temp)
		_sbx00_set_fan1_level(policy->up_step[fan1_up_temp_level].level * 256 / 100);

	get_down_temp(policy, current_temp, &fan1_down_temp, &fan1_down_temp_level);
	get_up_temp(policy, current_temp, &fan1_up_temp, &fan1_up_temp_level);

exit:
        queue_delayed_work(check_workqueue, &check_work,
				policy->adjust_period * HZ);
}

static void sbx00_fan1_step_mode(get_temp_fun fun,
			struct loongson_fan_policy *policy)
{
	u8 current_temp;

	current_temp = fun() / 1000;
	get_up_temp(policy, current_temp, &fan1_up_temp, &fan1_up_temp_level);
	get_down_temp(policy, current_temp, &fan1_down_temp,
					&fan1_down_temp_level);

	/* current speed is not sure, setting now */
	_sbx00_set_fan1_level(policy->up_step[fan1_up_temp_level -1 ].level * 256 / 100);

        check_workqueue = create_singlethread_workqueue("Temprature Check");
        queue_delayed_work(check_workqueue, &check_work,
					policy->adjust_period * HZ);
}

static void sbx00_fan1_start_auto(void)
{
	u8 level;
	struct loongson_fan_policy *policy;

	policy = sbx00_fan1_ops.fan_policy;
	if (policy == NULL) {
		goto fan1_no_policy;
	}

	switch (policy->type) {
	case CONSTANT_SPEED_POLICY:
		level = policy->percent * 255 / 100;
		if (level > MAX_FAN_LEVEL)
			level = MAX_FAN_LEVEL;

		_sbx00_set_fan1_level(level);
		break;
	case STEP_SPEED_POLICY:
		sbx00_fan1_step_mode(loongson_temp_info.get_cpu_temp, policy);
		break;
	default:
		printk(KERN_ERR "sbx00 fan1 not support fan policy id %d!\n", policy->type);
		goto fan1_no_policy;
	}

	return;

fan1_no_policy:
	sbx00_set_fan1_level(MAX_FAN_LEVEL);
	printk(KERN_ERR "sbx00 fan1 have no fan policy!\n");

	return;
}

static void sbx00_fan1_stop_auto(void)
{
	if ((sbx00_fan1_ops.fan_policy->type == STEP_SPEED_POLICY) &&
			(sbx00_fan1_mode == FAN_AUTO_MODE)) {
		cancel_delayed_work(&check_work);
		destroy_workqueue(check_workqueue);
	}
}

static enum fan_control_mode sbx00_get_fan1_mode(void)
{
	return sbx00_fan1_mode;
}

static int sbx00_set_fan1_mode(enum fan_control_mode mode)
{
	if (mode >= FAN_MODE_END)
		return -EINVAL;

	if (mode == sbx00_fan1_mode)
		return 0;

	switch (mode) {
	case FAN_FULL_MODE:
		sbx00_fan1_stop_auto();
		_sbx00_set_fan1_level(MAX_FAN_LEVEL);
		break;
	case FAN_MANUAL_MODE:
		sbx00_fan1_stop_auto();
		break;
	case FAN_AUTO_MODE:
		sbx00_fan1_start_auto();
		break;
	default:
		break;
	}

	sbx00_fan1_mode = mode;

	return 0;
}

static int __devinit sbx00_fan1_probe(struct platform_device *dev)
{
	sbx00_fan1_init();
	sbx00_fan1_enable = 1;

	/* get fan policy */
	sbx00_fan1_ops.fan_policy = loongson_fan1_ops.fan_policy;

	/* set loongson_fan1_ops */
	loongson_fan1_ops = sbx00_fan1_ops;

	/* force fan1 in auto mode first */
	sbx00_set_fan1_mode(FAN_AUTO_MODE);

	return 0;
}

static struct platform_driver sbx00_fan1_driver = {
	.probe		= sbx00_fan1_probe,
	.driver		= {
		.name	= "sbx00-fan1",
		.owner	= THIS_MODULE,
	},
};

/*  ==================== fan 2 ======================== */

static u8 sbx00_get_fan2_level(void);
static int sbx00_set_fan2_level(u8 level);
static enum fan_control_mode sbx00_get_fan2_mode(void);
static int sbx00_set_fan2_mode(enum fan_control_mode mode);

static struct loongson_fan_ops sbx00_fan2_ops = {
	.get_fan_level	= sbx00_get_fan2_level,
	.set_fan_level	= sbx00_set_fan2_level,
	.get_fan_mode	= sbx00_get_fan2_mode,
	.set_fan_mode	= sbx00_set_fan2_mode,
};

static void sbx00_fan2_init(void)
{
	u8 reg;

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
}

/* low-level fucntions */
static u8 _sbx00_get_fan2_level(void)
{
	return pm2_ioread(LOWDUTY1);
}

static int _sbx00_set_fan2_level(u8 level)
{
	pm2_iowrite(LOWDUTY1, level);
	return 0;
}

/* high-level functions */
static u8 sbx00_get_fan2_level(void)
{
	return _sbx00_get_fan2_level();
}

/* Do nothing if not at manual mode */
static int sbx00_set_fan2_level(u8 level)
{
	if (sbx00_fan2_mode == FAN_MANUAL_MODE)
		_sbx00_set_fan2_level(level);

	return 0;
}

static void sbx00_fan2_start_auto(void)
{
	u8 level;
	struct loongson_fan_policy *policy;

	policy = sbx00_fan2_ops.fan_policy;
	if (policy == NULL) {
		goto fan2_no_policy;
	}

	switch (policy->type) {
	case CONSTANT_SPEED_POLICY:
		level = policy->percent * 255 / 100;
		if (level > MAX_FAN_LEVEL)
			level = MAX_FAN_LEVEL;

		_sbx00_set_fan2_level(level);
		break;
	default:
		printk(KERN_ERR "sbx00 fan2 not support fan policy id %d!\n", policy->type);
		goto fan2_no_policy;
	}

	return;

fan2_no_policy:
	sbx00_set_fan2_level(MAX_FAN_LEVEL);
	printk(KERN_ERR "sbx00 fan2 have no fan policy!\n");

	return;
}

static void sbx00_fan2_stop_auto(void)
{
	/* do nothing now */
}

static enum fan_control_mode sbx00_get_fan2_mode(void)
{
	return sbx00_fan2_mode;
}

static int sbx00_set_fan2_mode(enum fan_control_mode mode)
{
	if (mode >= FAN_MODE_END)
		return -EINVAL;

	if (mode == sbx00_fan2_mode)
		return 0;

	switch (mode) {
	case FAN_FULL_MODE:
		sbx00_fan2_stop_auto();
		_sbx00_set_fan2_level(MAX_FAN_LEVEL);
		break;
	case FAN_MANUAL_MODE:
		sbx00_fan2_stop_auto();
		break;
	case FAN_AUTO_MODE:
		sbx00_fan2_start_auto();
		break;
	default:
		break;
	}

	sbx00_fan2_mode = mode;

	return 0;
}

static int sbx00_fan2_probe(struct platform_device *dev)
{
	sbx00_fan2_init();
	sbx00_fan2_enable = 1;

	/* get fan policy */
	sbx00_fan2_ops.fan_policy = loongson_fan2_ops.fan_policy;

	/* set loongson_fan2_ops */
	loongson_fan2_ops = sbx00_fan2_ops;

	/* force fan2 in auto mode first */
	sbx00_set_fan2_mode(FAN_AUTO_MODE);

	return 0;
}

static struct platform_driver sbx00_fan2_driver = {
	.probe		= sbx00_fan2_probe,
	.driver		= {
		.name	= "sbx00-fan2",
		.owner	= THIS_MODULE,
	},
};


static int __init sbx00_fan_init(void)
{
	int ret;

	/* now only try two fan controllers */
	ret = platform_driver_register(&sbx00_fan1_driver);
	if (ret) {
		printk(KERN_ERR "register_fan1_fail\n");
		goto register_fan1_fail;
	}

	ret = platform_driver_register(&sbx00_fan2_driver);
	if (ret) {
		printk(KERN_ERR "register_fan2_fail\n");
		goto register_fan2_fail;
	}
	return ret;

register_fan2_fail:
	platform_driver_unregister(&sbx00_fan1_driver);

register_fan1_fail:
	return ret;
}

static void __exit sbx00_fan_exit(void)
{
	/* set fan at full speed before module exit */
	if (sbx00_fan1_enable)
		sbx00_set_fan1_mode(FAN_FULL_MODE);

	if (sbx00_fan2_enable)
		sbx00_set_fan2_mode(FAN_FULL_MODE);

	platform_driver_unregister(&sbx00_fan1_driver);
	platform_driver_unregister(&sbx00_fan2_driver);
}

module_init(sbx00_fan_init);
module_exit(sbx00_fan_exit);

MODULE_AUTHOR("Xiang Yu <xiangy@lemote.com>");
MODULE_DESCRIPTION("SB700/SB710/SB800 fan control driver");
MODULE_LICENSE("GPL");
