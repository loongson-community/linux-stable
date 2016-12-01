/*
 * Driver for Lemote Loongson-3A/2Gq Laptops with WPCE775l Embeded Controller
 *
 * Copyright (C) 2011 Lemote Inc.
 * Author : Huang Wei <huangw@lemote.com>
 *        : Wang Rui <wangr@lemote.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/reboot.h>
#include <linux/power_supply.h>
#include <linux/input.h>
#include <linux/input/sparse-keymap.h>
#include <linux/jiffies.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <asm/bootinfo.h>

#include <ec_wpce775l.h>

#define KEY_TOUCHPAD_SW	KEY_F21
#define KEY_MODEM	KEY_F24

/* Backlight */
#define MAX_BRIGHTNESS	9

/* Power supply */
#define BIT_BAT_POWER_ACIN		(1 << 0)
enum
{
	APM_AC_OFFLINE =	0,
	APM_AC_ONLINE,
	APM_AC_BACKUP,
	APM_AC_UNKNOWN =	0xff
};
enum
{
	APM_BAT_STATUS_HIGH =		0,
	APM_BAT_STATUS_LOW,
	APM_BAT_STATUS_CRITICAL,
	APM_BAT_STATUS_CHARGING,
	APM_BAT_STATUS_NOT_PRESENT,
	APM_BAT_STATUS_UNKNOWN =	0xff
};

enum /* bat_reg_flag */
{
	BAT_REG_TEMP_FLAG = 1,
	BAT_REG_VOLTAGE_FLAG,
	BAT_REG_CURRENT_FLAG,
	BAT_REG_AC_FLAG,
	BAT_REG_RC_FLAG,
	BAT_REG_FCC_FLAG,
	BAT_REG_ATTE_FLAG,
	BAT_REG_ATTF_FLAG,
	BAT_REG_RSOC_FLAG,
	BAT_REG_CYCLCNT_FLAG
};

/* Power info cached timeout */
#define POWER_INFO_CACHED_TIMEOUT	100	/* jiffies */

/* SCI device */
#define EC_SCI_DEV		"sci"	/* < 10 bytes. */
#define SCI_IRQ_NUM		0x07
#define GPIO_SIZE		256

const char *version = EC_VERSION;

/* Power information structure */
struct lemote3a_power_info
{
	/* AC insert or not */
	unsigned int ac_in;
	/* Battery insert or not */
	unsigned int bat_in;
	unsigned int health;

	/* Battery designed capacity */
	unsigned int design_capacity;
	/* Battery designed voltage */
	unsigned int design_voltage;
	/* Battery capacity after full charged */
	unsigned int full_charged_capacity;
	/* Battery Manufacture Date */
	unsigned char manufacture_date[11];
	/* Battery Serial number */
	unsigned char serial_number[8];
	/* Battery Manufacturer Name, max 11 + 1(length) bytes */
	unsigned char manufacturer_name[12];
	/* Battery Device Name, max 7 + 1(length) bytes */
	unsigned char device_name[8];
	/* Battery Technology */
	unsigned int technology;
	/* Battery cell count */
	unsigned char cell_count;

	/* Battery dynamic charge/discharge voltage */
	unsigned int voltage_now;
	/* Battery dynamic charge/discharge average current */
	int current_now;
	int current_sign;
	int current_average;
	/* Battery current remaining capacity */
	unsigned int remain_capacity;
	/* Battery current remaining capacity percent */
	unsigned int remain_capacity_percent;
	/* Battery current temperature */
	unsigned int temperature;
	/* Battery current remaining time (AverageTimeToEmpty) */
	unsigned int remain_time;
	/* Battery current full charging time (averageTimeToFull) */
	unsigned int fullchg_time;
	/* Battery Status */
	unsigned int charge_status;
	/* Battery current cycle count (CycleCount) */
	unsigned int cycle_count;
};

/* SCI device structure */
struct sci_device
{
	/* The sci number get from ec */
	unsigned char number;
	/* Sci count */
	unsigned char parameter;
	/* Irq relative */
	unsigned char irq;
	unsigned char irq_data;
	/* Device name */
	unsigned char name[10];
};
/* SCI device event structure */
struct sci_event
{
	int index;
	sci_handler handler;
};

/* Platform driver init handler */
static int __init lemote3a_laptop_init(void);
/* Platform driver exit handler */
static void __exit lemote3a_laptop_exit(void);
/* Platform device suspend handler */
static int lemote3a_laptop_suspend(struct platform_device * pdev, pm_message_t state);
/* Platform device resume handler */
static int lemote3a_laptop_resume(struct platform_device * pdev);

static ssize_t version_show(struct device_driver * driver, char * buf);

/* Camera control misc device open handler */
static int lemote3a_cam_misc_open(struct inode * inode, struct file * filp);
/* Camera control misc device release handler */
static int lemote3a_cam_misc_release(struct inode * inode, struct file * filp);
/* Camera control misc device read handler */
ssize_t lemote3a_cam_misc_read(struct file * filp,
			char __user * buffer, size_t size, loff_t * offset);
/* Camera control misc device write handler */
static ssize_t lemote3a_cam_misc_write(struct file * filp,
			const char __user * buffer, size_t size, loff_t * offset);

/* Backlight device set brightness handler */
static int lemote3a_set_brightness(struct backlight_device * pdev);
/* Backlight device get brightness handler */
static int lemote3a_get_brightness(struct backlight_device * pdev);

/* >>>Power management operation */
/* Update battery information handle function. */
static void lemote3a_power_battery_info_update(unsigned char bat_reg_flag);
/* Clear battery static information. */
static void lemote3a_power_info_battery_static_clear(void);
/* Get battery static information. */
static void lemote3a_power_info_battery_static_update(void);
/* Update power_status value */
static void lemote3a_power_info_power_status_update(void);
static void lemote3a_bat_get_string(unsigned char index, unsigned char *bat_string);
/* Power supply Battery get property handler */
static int lemote3a_bat_get_property(struct power_supply * pws,
			enum power_supply_property psp, union power_supply_propval * val);
/* Power supply AC get property handler */
static int lemote3a_ac_get_property(struct power_supply * pws,
			enum power_supply_property psp, union power_supply_propval * val);
/* <<<End power management operation */

/* SCI device pci driver init handler */
static int sci_pci_driver_init(void);
/* SCI device pci driver exit handler */
static void sci_pci_driver_exit(void);
/* SCI device pci driver init */
static int sci_pci_init(void);
/* SCI event routine handler */
static irqreturn_t lemote3a_sci_int_routine(int irq, void * dev_id);
/* SCI event handler */
void lemote3a_sci_event_handler(int event);
/* SCI device dpms event handler */
static int lemote3a_dpms_handler(int status);
/* SCI device over temperature event handler */
static int lemote3a_over_temp_handler(int status);
/* SCI device Throttling the CPU event handler */
static int lemote3a_throttling_CPU_handler(int status);
/* SCI device AC event handler */
static int lemote3a_ac_handler(int status);
/* SCI device Battery event handler */
static int lemote3a_bat_handler(int status);
/* SCI device Battery low event handler */
static int lemote3a_bat_low_handler(int status);
/* SCI device Battery very low event handler */
static int lemote3a_bat_very_low_handler(int status);
/* SCI device LID event handler */
static int lemote3a_lid_handler(int status);

static void lemote3a_tp_led_set(struct led_classdev *led_cdev,
			       enum led_brightness brightness);

/* Hotkey device init handler */
static int lemote3a_hotkey_init(void);
/* Hotkey device exit handler */
static void lemote3a_hotkey_exit(void);
extern int ec_query_get_event_num(void);

static int wpce775l_probe(struct platform_device *dev);

/* Platform driver object */
static struct platform_driver platform_driver =
{
	.probe	= wpce775l_probe,
	.driver =
	{
		.name = "wpce775l",
		.owner = THIS_MODULE,
	},
#ifdef CONFIG_PM
	.suspend = lemote3a_laptop_suspend,
	.resume  = lemote3a_laptop_resume,
#endif /* CONFIG_PM */
};
static DRIVER_ATTR_RO(version);

/* Camera control misc device object file operations */
static const struct file_operations lemote3a_cam_misc_fops =
{
	.open = lemote3a_cam_misc_open,
	.release = lemote3a_cam_misc_release,
	.read = lemote3a_cam_misc_read,
	.write = lemote3a_cam_misc_write
};

/* Camera control misc device object */
static struct miscdevice lemote3a_cam_misc_dev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "webcam",
	.fops = &lemote3a_cam_misc_fops
};

/* Backlight device object */
static struct backlight_device * lemote3a_backlight_dev = NULL;

/* Backlight device operations table object */
static struct backlight_ops lemote3a_backlight_ops =
{
	.get_brightness = lemote3a_get_brightness,
	.update_status =  lemote3a_set_brightness,
};

/* Power info object */
static struct lemote3a_power_info * power_info = NULL;
/* Power supply Battery property object */
static enum power_supply_property lemote3a_bat_props[] =
{
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL, /* in uAh */
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY, /* in percents! */
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG,
	POWER_SUPPLY_PROP_TIME_TO_FULL_AVG,
	/* Properties of type `const char *' */
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_SERIAL_NUMBER,
};

/* Power supply Battery device object */
static struct power_supply_desc lemote3a_bat_desc =
{
	.name = "lemote3a-bat",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = lemote3a_bat_props,
	.num_properties = ARRAY_SIZE(lemote3a_bat_props),
	.get_property = lemote3a_bat_get_property,
};

static struct power_supply *lemote3a_bat;

/* Power supply AC property object */
static enum power_supply_property lemote3a_ac_props[] =
{
	POWER_SUPPLY_PROP_ONLINE,
};
/* Power supply AC device object */
static struct power_supply_desc lemote3a_ac_desc =
{
	.name = "lemote3a-ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = lemote3a_ac_props,
	.num_properties = ARRAY_SIZE(lemote3a_ac_props),
	.get_property = lemote3a_ac_get_property,
};

static struct power_supply *lemote3a_ac;

/* SCI device object */
static struct sci_device * lemote3a_sci_device = NULL;

/* SCI device event handler table */
static const struct sci_event se[] =
{
	[SCI_EVENT_NUM_LID] =			{INDEX_DEVICE_STATUS, lemote3a_lid_handler},
	[SCI_EVENT_NUM_SLEEP] =			{0, NULL},
	[SCI_EVENT_NUM_WLAN] =			{0, NULL},
	[SCI_EVENT_NUM_BRIGHTNESS_DN] =		{0, NULL},
	[SCI_EVENT_NUM_BRIGHTNESS_UP] =		{0, NULL},
	[SCI_EVENT_NUM_AUDIO_MUTE] =		{0, NULL},
	[SCI_EVENT_NUM_VOLUME_DN] =		{0, NULL},
	[SCI_EVENT_NUM_VOLUME_UP] =		{0, NULL},
	[SCI_EVENT_NUM_BLACK_SCREEN] =		{0, lemote3a_dpms_handler},
	[SCI_EVENT_NUM_DISPLAY_TOGGLE] =	{0, NULL},
	[SCI_EVENT_NUM_3G] =			{0, NULL},
	[SCI_EVENT_NUM_SIM] =			{0, NULL},
	[SCI_EVENT_NUM_CAMERA] =		{0, NULL},
	[SCI_EVENT_NUM_TP] =			{0, NULL},
	[SCI_EVENT_NUM_OVERTEMP] =		{0, lemote3a_over_temp_handler},
	[SCI_EVENT_NUM_AC] =			{0, lemote3a_ac_handler},
	[SCI_EVENT_NUM_BAT] =			{INDEX_POWER_STATUS, lemote3a_bat_handler},
	[SCI_EVENT_NUM_BATL] =			{0, lemote3a_bat_low_handler},
	[SCI_EVENT_NUM_BATVL] =			{0, lemote3a_bat_very_low_handler},
	[SCI_EVENT_NUM_THROT] =			{0, lemote3a_throttling_CPU_handler},
	[SCI_EVENT_NUM_POWER] =			{0, NULL},
	[SCI_EVENT_RESOLUTION_SETTING] = 	{0, NULL},
	[SCI_EVENT_MEDIA_RUN_PAUSE] =		{0, NULL},
	[SCI_EVENT_MEDIA_STOP] = 		{0, NULL},
	[SCI_EVENT_MEDIA_LAST] = 		{0, NULL},
	[SCI_EVENT_MEDIA_NEXT] = 		{0, NULL},
	[SCI_EVENT_RECOVERY] =			{0, NULL},
};
/* Hotkey device object */
static struct input_dev * lemote3a_hotkey_dev = NULL;
/* Hotkey keymap object */
static const struct key_entry lemote3a_keymap[] =
{
	{KE_SW,  SCI_EVENT_NUM_LID, { SW_LID } },
	{KE_KEY, SCI_EVENT_NUM_SLEEP, { KEY_SLEEP } }, /* A1004: Fn + ESC; A1201: Fn + F10 */
	{KE_KEY, SCI_EVENT_NUM_BRIGHTNESS_DN, { KEY_BRIGHTNESSDOWN } }, /* A1004: Fn + F2; A1201: Fn + F6 */
	{KE_KEY, SCI_EVENT_NUM_BRIGHTNESS_UP, { KEY_BRIGHTNESSUP } }, /* A1004: Fn + F3; A1201: Fn + F5 */
	{KE_KEY, SCI_EVENT_NUM_AUDIO_MUTE, { KEY_MUTE } }, /* A1004: Fn + F4; A1201: Fn + F7 */
	{KE_KEY, SCI_EVENT_NUM_VOLUME_DN, { KEY_VOLUMEDOWN } }, /* A1004: Fn + F5; A1201: Fn + F4 */
	{KE_KEY, SCI_EVENT_NUM_VOLUME_UP, { KEY_VOLUMEUP } }, /* A1004: Fn + F6; A1201: Fn + F3 */
	{KE_KEY, SCI_EVENT_NUM_BLACK_SCREEN, { KEY_DISPLAYTOGGLE } }, /* A1004: Fn + F7 */
	{KE_KEY, SCI_EVENT_NUM_DISPLAY_TOGGLE, { KEY_SWITCHVIDEOMODE } }, /* A1004: Fn + F8; A1201: Fn + F2 */
	{KE_KEY, SCI_EVENT_NUM_3G, { KEY_MODEM } }, /* Fn + F9 */
	{KE_KEY, SCI_EVENT_NUM_CAMERA, { KEY_CAMERA } }, /* Fn + F10 */
	{KE_KEY, SCI_EVENT_NUM_TP, { KEY_TOUCHPAD_SW } }, /* Fn + F11 */
	{KE_KEY, SCI_EVENT_NUM_POWER, { KEY_POWER } }, /* Power */
	{KE_KEY, SCI_EVENT_RESOLUTION_SETTING, { KEY_SETUP } },
	{KE_KEY, SCI_EVENT_MEDIA_RUN_PAUSE, { KEY_PLAYPAUSE } },
	{KE_KEY, SCI_EVENT_MEDIA_STOP, { KEY_STOPCD } },
	{KE_KEY, SCI_EVENT_MEDIA_LAST, { KEY_PREVIOUSSONG } },
	{KE_KEY, SCI_EVENT_MEDIA_NEXT, { KEY_NEXTSONG } },
	{KE_KEY, SCI_EVENT_RECOVERY, { KEY_PROG1 } },
	{KE_END, 0 }
};

/* Touchpad EN/DIS led*/
static struct led_classdev lemote3a_tp_led = {
	.name = "psmouse::touchpad",
	.brightness_set = lemote3a_tp_led_set,
	.flags = LED_CORE_SUSPENDRESUME,
};

static int tp_led_shutdown_notify(struct notifier_block *unused1,
			   unsigned long unused2, void *unused3)
{
	lemote3a_tp_led_set(&lemote3a_tp_led, LED_OFF);
	return NOTIFY_DONE;
}

static struct notifier_block tp_led_nb = {
	.notifier_call = tp_led_shutdown_notify,
};

static int wpce775l_probe(struct platform_device *dev)
{
	int ret;

	/* Register backlight START */
	lemote3a_backlight_dev = backlight_device_register("lemote",
				NULL, NULL, &lemote3a_backlight_ops, NULL);
	if (IS_ERR(lemote3a_backlight_dev)) {
		ret = PTR_ERR(lemote3a_backlight_dev);
		goto fail_backlight_device_register;
	}
	lemote3a_backlight_dev->props.max_brightness = ec_read(INDEX_DISPLAY_MAXBRIGHTNESS_LEVEL);
	lemote3a_backlight_dev->props.brightness = ec_read(INDEX_DISPLAY_BRIGHTNESS);
	backlight_update_status(lemote3a_backlight_dev);
	/* Register backlight END */

	/* Register power supply START */
	power_info = kzalloc(sizeof(struct lemote3a_power_info), GFP_KERNEL);
	if (!power_info) {
		printk(KERN_ERR "Lemote Laptop Platform Driver: Alloc memory for power_info failed!\n");
		ret = -ENOMEM;
		goto fail_power_info_alloc;
	}

	lemote3a_power_info_power_status_update();
	if (power_info->bat_in) {
		/* Get battery static information. */
		lemote3a_power_info_battery_static_update();
	}
	else {
		printk(KERN_ERR "Lemote Laptop Platform Driver: The battery does not exist!!\n");
	}
	lemote3a_bat = power_supply_register(NULL, &lemote3a_bat_desc, NULL);
	if (IS_ERR(lemote3a_bat)) {
		ret = -ENOMEM;
		goto fail_bat_power_supply_register;
	}

	lemote3a_ac = power_supply_register(NULL, &lemote3a_ac_desc, NULL);
	if (IS_ERR(lemote3a_ac)) {
		ret = -ENOMEM;
		goto fail_ac_power_supply_register;
	}
	/* Register power supply END */

	/* Touchpad enable/disable LED */
	ret = led_classdev_register(NULL, &lemote3a_tp_led);
	if (ret == 0)
		register_reboot_notifier(&tp_led_nb);
	else
		goto fail_tp_led_register;

	/* Hotkey device START */
	ret = lemote3a_hotkey_init();
	if (ret) {
		printk(KERN_ERR "Lemote Laptop Platform Driver : Fail to register hotkey device.\n");
		goto fail_hotkey_init;
	}
	/* Hotkey device END */

	/* SCI PCI Driver Init START  */
	ret = sci_pci_driver_init();
	if (ret) {
		printk(KERN_ERR "LS3ANB Driver : Fail to register sci pci driver.\n");
		goto fail_sci_pci_driver_init;
	}
	/* SCI PCI Driver Init END */

	/* Camera control misc Device START */
	ret = misc_register(&lemote3a_cam_misc_dev);
	if (ret) {
		printk(KERN_ERR "Lemote Laptop Platform Driver : Fail to register camera control misc device.\n");
		goto fail_misc_register;
	}
	/* Camera control misc Device END */

	/* Request control for backlight device START */
	ec_write(INDEX_BACKLIGHT_CTRLMODE, BACKLIGHT_CTRL_BYHOST);
	/* Request control for backlight device END */

	return 0;

fail_misc_register:
	sci_pci_driver_exit();
fail_sci_pci_driver_init:
	lemote3a_hotkey_exit();
fail_hotkey_init:
	led_classdev_unregister(&lemote3a_tp_led);
fail_tp_led_register:
	power_supply_unregister(lemote3a_ac);
fail_ac_power_supply_register:
	power_supply_unregister(lemote3a_bat);
fail_bat_power_supply_register:
	kfree(power_info);
fail_power_info_alloc:
	backlight_device_unregister(lemote3a_backlight_dev);
fail_backlight_device_register:
	platform_driver_unregister(&platform_driver);

	return ret;
}

/* Platform driver init handler */
static int __init lemote3a_laptop_init(void)
{
	int ret;

	printk(KERN_INFO "Lemote Laptop Platform Driver version V%s\n", version);

	ret = platform_driver_register(&platform_driver);
	if(ret) {
		printk(KERN_ERR "Lemote Laptop Platform Driver: Fail to register lemote laptop platform driver.\n");
		return ret;
	}
	ret = driver_create_file(&platform_driver.driver, &driver_attr_version);

	return ret;
}

/* Platform driver exit handler */
static void __exit lemote3a_laptop_exit(void)
{
	free_irq(lemote3a_sci_device->irq, lemote3a_sci_device);

	/* Return control for backlight device START */
	ec_write(INDEX_BACKLIGHT_CTRLMODE, BACKLIGHT_CTRL_BYEC);
	/* Return control for backlight device END */

	/* Camera control misc device */
	misc_deregister(&lemote3a_cam_misc_dev);

	/* Hotkey & SCI device */
	sci_pci_driver_exit();
	lemote3a_hotkey_exit();

	/* Touchpad enable/disable LED */
	unregister_reboot_notifier(&tp_led_nb);
	led_classdev_unregister(&lemote3a_tp_led);

	/* Power supply */
	power_supply_unregister(lemote3a_ac);
	power_supply_unregister(lemote3a_bat);
	kfree(power_info);

	/* Backlight */
	backlight_device_unregister(lemote3a_backlight_dev);

	/* Platform device & driver */
	platform_driver_unregister(&platform_driver);

	printk(KERN_INFO "Lemote Laptop Platform Driver : Unload Platform Specific Driver.\n");
}

#ifdef CONFIG_PM
/* Platform device suspend handler */
static int lemote3a_laptop_suspend(struct platform_device * pdev, pm_message_t state)
{
	struct pci_dev *dev;

	dev = pci_get_device(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_SBX00_SMBUS, NULL);
	pci_disable_device(dev);

	return 0;
}

/* Platform device resume handler */
static int lemote3a_laptop_resume(struct platform_device * pdev)
{
	struct pci_dev *dev;

	dev = pci_get_device(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_SBX00_SMBUS, NULL);
	pci_enable_device(dev);

	/* Process LID event */
	lemote3a_sci_event_handler(SCI_EVENT_NUM_LID);

	/*
	 * Clear sci status: GPM9Status field in bit14 of
	 * EVENT_STATUS register for SB710, write 1 to clear
	 *
	 * Clear all SCI events when suspend
	 */
	clean_ec_event_status();

	return 0;
}
#else
static int lemote3a_laptop_suspend(struct platform_device * pdev, pm_message_t state)
{
	return 0;
}

static int lemote3a_laptop_resume(struct platform_device * pdev)
{
	return 0;
}
#endif /* CONFIG_PM */

static ssize_t version_show(struct device_driver * driver, char * buf)
{
	return sprintf(buf, "%s\n", version);
}

/* Camera control misc device open handler */
static int lemote3a_cam_misc_open(struct inode * inode, struct file * filp)
{
	return 0;
}

/* Camera control misc device release handler */
static int lemote3a_cam_misc_release(struct inode * inode, struct file * filp)
{
	return 0;
}

/* Camera control misc device read handler */
ssize_t lemote3a_cam_misc_read(struct file * filp,
			char __user * buffer, size_t size, loff_t * offset)
{
	int ret = 0;

	if (0 != *offset)
		return 0;

	ret = ec_read(INDEX_CAM_STSCTRL);
	ret = sprintf(buffer, "%d\n", ret);
	*offset = ret;

	return ret;
}

/* Camera control misc device write handler */
static ssize_t lemote3a_cam_misc_write(struct file * filp,
			const char __user * buffer, size_t size, loff_t * offset)
{
	if (0 >= size)
		return -EINVAL;

	if ('0' == buffer[0])
		ec_write(INDEX_CAM_STSCTRL, CAM_STSCTRL_OFF);
	else
		ec_write(INDEX_CAM_STSCTRL, CAM_STSCTRL_ON);

	return size;
}

/* Backlight device set brightness handler */
static int lemote3a_set_brightness(struct backlight_device * pdev)
{
	unsigned int level = 0;

	level = ((FB_BLANK_UNBLANK==pdev->props.fb_blank) &&
				(FB_BLANK_UNBLANK==pdev->props.power)) ?
					pdev->props.brightness : 0;

	if (MAX_BRIGHTNESS < level) {
		level = MAX_BRIGHTNESS;
	}
	else if (level < 0) {
		level = 0;
	}

	ec_write(INDEX_DISPLAY_BRIGHTNESS, level);

	return 0;
}

/* Backlight device get brightness handler */
static int lemote3a_get_brightness(struct backlight_device * pdev)
{
	/* Read level from ec */
	return ec_read(INDEX_DISPLAY_BRIGHTNESS);
}

/* Update battery information handle function. */
static void lemote3a_power_battery_info_update(unsigned char bat_reg_flag)
{
	short bat_info_value = 0;

	switch (bat_reg_flag) {
		/* Update power_info->temperature value */
		case BAT_REG_TEMP_FLAG:
			lemote3a_power_info_power_status_update();
			bat_info_value = (ec_read(INDEX_BATTERY_TEMP_HIGH) << 8) | ec_read(INDEX_BATTERY_TEMP_LOW);
			power_info->temperature = (power_info->bat_in) ? (bat_info_value / 10 - 273) : 0;
			break;
		/* Update power_info->voltage value */
		case BAT_REG_VOLTAGE_FLAG:
			lemote3a_power_info_power_status_update();
			bat_info_value = (ec_read(INDEX_BATTERY_VOL_HIGH) << 8) | ec_read(INDEX_BATTERY_VOL_LOW);
			power_info->voltage_now = (power_info->bat_in) ? bat_info_value : 0;
			break;
		/* Update power_info->current_now value */
		case BAT_REG_CURRENT_FLAG:
			lemote3a_power_info_power_status_update();
			bat_info_value = (ec_read(INDEX_BATTERY_CURRENT_HIGH) << 8) | ec_read(INDEX_BATTERY_CURRENT_LOW);
			power_info->current_now = (power_info->bat_in) ? bat_info_value : 0;
			break;
		/* Update power_info->current_avg value */
		case BAT_REG_AC_FLAG:
			lemote3a_power_info_power_status_update();
			bat_info_value = (ec_read(INDEX_BATTERY_AC_HIGH) << 8) | ec_read(INDEX_BATTERY_AC_LOW);
			power_info->current_average = (power_info->bat_in) ? bat_info_value : 0;
			break;
		/* Update power_info->remain_capacity value */
		case BAT_REG_RC_FLAG:
			power_info->remain_capacity = (ec_read(INDEX_BATTERY_RC_HIGH) << 8) | ec_read(INDEX_BATTERY_RC_LOW);
			break;
		/* Update power_info->full_charged_capacity value */
		case BAT_REG_FCC_FLAG:
			power_info->full_charged_capacity = (ec_read(INDEX_BATTERY_FCC_HIGH) << 8) | ec_read(INDEX_BATTERY_FCC_LOW);
			break;
		/* Update power_info->remain_time value */
		case BAT_REG_ATTE_FLAG:
			power_info->remain_time = (ec_read(INDEX_BATTERY_ATTE_HIGH) << 8) | ec_read(INDEX_BATTERY_ATTE_LOW);
			break;
		/* Update power_info->fullchg_time value */
		case BAT_REG_ATTF_FLAG:
			power_info->fullchg_time = (ec_read(INDEX_BATTERY_ATTF_HIGH) << 8) | ec_read(INDEX_BATTERY_ATTF_LOW);
			break;
		/* Update power_info->curr_cap value */
		case BAT_REG_RSOC_FLAG:
			power_info->remain_capacity_percent = ec_read(INDEX_BATTERY_CAPACITY);
			break;
		/* Update power_info->cycle_count value */
		case BAT_REG_CYCLCNT_FLAG:
			power_info->cycle_count = (ec_read(INDEX_BATTERY_CYCLECNT_HIGH) << 8) | ec_read(INDEX_BATTERY_CYCLECNT_LOW);
			break;

		default:
			break;
	}
}

/* Clear battery static information. */
static void lemote3a_power_info_battery_static_clear(void)
{
	strcpy(power_info->manufacturer_name, "Unknown");
	strcpy(power_info->device_name, "Unknown");
	power_info->technology = POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
	strcpy(power_info->serial_number, "Unknown");
	strcpy(power_info->manufacture_date, "Unknown");
	power_info->cell_count = 0;
	power_info->design_capacity = 0;
	power_info->design_voltage = 0;
}

/* Get battery static information. */
static void lemote3a_power_info_battery_static_update(void)
{
	unsigned int manufacture_date, bat_serial_number;
	char device_chemistry[5];

	manufacture_date = (ec_read(INDEX_BATTERY_MFD_HIGH) << 8) | ec_read(INDEX_BATTERY_MFD_LOW);
	sprintf(power_info->manufacture_date, "%d-%d-%d", (manufacture_date >> 9) + 1980,
            (manufacture_date & 0x01E0) >> 5, manufacture_date & 0x001F);
	lemote3a_bat_get_string(INDEX_BATTERY_MFN_LENG, power_info->manufacturer_name);
	lemote3a_bat_get_string(INDEX_BATTERY_DEVNAME_LENG, power_info->device_name);
	lemote3a_bat_get_string(INDEX_BATTERY_DEVCHEM_LENG, device_chemistry);
	if ((device_chemistry[2] == 'o') || (device_chemistry[2] == 'O')) {
		power_info->technology = POWER_SUPPLY_TECHNOLOGY_LION;
	}
	else if (((device_chemistry[1] = 'h') && (device_chemistry[2] == 'm')) ||
			((device_chemistry[1] = 'H') && (device_chemistry[2] == 'M'))) {
		power_info->technology = POWER_SUPPLY_TECHNOLOGY_NiMH;
	}
	else if ((device_chemistry[2] == 'p') || (device_chemistry[2] == 'P')) {
		power_info->technology = POWER_SUPPLY_TECHNOLOGY_LIPO;
	}
	else if ((device_chemistry[2] == 'f') || (device_chemistry[2] == 'F')) {
		power_info->technology = POWER_SUPPLY_TECHNOLOGY_LiFe;
	}
	else if ((device_chemistry[2] == 'c') || (device_chemistry[2] == 'C')) {
		power_info->technology = POWER_SUPPLY_TECHNOLOGY_NiCd;
	}
	else if (((device_chemistry[1] = 'n') && (device_chemistry[2] == 'm')) ||
			((device_chemistry[1] = 'N') && (device_chemistry[2] == 'M'))) {
		power_info->technology = POWER_SUPPLY_TECHNOLOGY_LiMn;
	}
	else {
		power_info->technology = POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
	}

	bat_serial_number = (ec_read(INDEX_BATTERY_SN_HIGH) << 8) | ec_read(INDEX_BATTERY_SN_LOW);
	snprintf(power_info->serial_number, 8, "%x", bat_serial_number);

	power_info->cell_count = ((ec_read(INDEX_BATTERY_CV_HIGH) << 8) | ec_read(INDEX_BATTERY_CV_LOW)) / 4200;

	power_info->design_capacity = (ec_read(INDEX_BATTERY_DC_HIGH) << 8) | ec_read(INDEX_BATTERY_DC_LOW);
	power_info->design_voltage = (ec_read(INDEX_BATTERY_DV_HIGH) << 8) | ec_read(INDEX_BATTERY_DV_LOW);
	power_info->full_charged_capacity = (ec_read(INDEX_BATTERY_FCC_HIGH) << 8) | ec_read(INDEX_BATTERY_FCC_LOW);
	printk(KERN_INFO "LS3ANB Battery Information:\nManufacturerName: %s, DeviceName: %s, DeviceChemistry: %s\n",
			power_info->manufacturer_name, power_info->device_name, device_chemistry);
	printk(KERN_INFO "SerialNumber: %s, ManufactureDate: %s, CellNumber: %d\n",
			power_info->serial_number, power_info->manufacture_date, power_info->cell_count);
	printk(KERN_INFO "DesignCapacity: %dmAh, DesignVoltage: %dmV, FullChargeCapacity: %dmAh\n",
			power_info->design_capacity, power_info->design_voltage, power_info->full_charged_capacity);
}

/* Update power_status value */
static void lemote3a_power_info_power_status_update(void)
{
	unsigned int power_status = 0;

	power_status = ec_read(INDEX_POWER_STATUS);

	power_info->ac_in = (power_status & MASK(BIT_POWER_ACPRES)) ?
					APM_AC_ONLINE : APM_AC_OFFLINE;

	power_info->bat_in = (power_status & MASK(BIT_POWER_BATPRES)) ? 1 : 0;
	if( power_info->bat_in && ((ec_read(INDEX_BATTERY_DC_LOW) | (ec_read(INDEX_BATTERY_DC_HIGH) << 8)) == 0) )
		power_info->bat_in = 0;

	power_info->health = (power_info->bat_in) ?	POWER_SUPPLY_HEALTH_GOOD :
							POWER_SUPPLY_HEALTH_UNKNOWN;
	if (power_status & (MASK(BIT_POWER_BATL) | MASK(BIT_POWER_BATVL))) {
		power_info->health = POWER_SUPPLY_HEALTH_DEAD;
	}

	if (!power_info->bat_in) {
		power_info->charge_status = POWER_SUPPLY_STATUS_UNKNOWN;
	}
	else {
		if (power_status & MASK(BIT_POWER_BATFCHG)) {
			power_info->charge_status = POWER_SUPPLY_STATUS_FULL;
		}
		else if (power_status & MASK(BIT_POWER_BATCHG)) {
			power_info->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		}
		else if (power_status & MASK(BIT_POWER_TERMINATE)) {
			power_info->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		}
		else {
			power_info->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
		}
	}
}

/* Get battery static information string */
static void lemote3a_bat_get_string(unsigned char index, unsigned char *bat_string)
{
	unsigned char length, i;

	length = ec_read(index);
	for (i = 0; i < length; i++) {
		*bat_string++ = ec_read(++index);
	}
	*bat_string = '\0';
}

/* Power supply Battery get property handler */
static int lemote3a_bat_get_property(struct power_supply * pws,
			enum power_supply_property psp, union power_supply_propval * val)
{
	switch (psp) {
		/* Get battery static information. */
		case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
			val->intval = power_info->design_voltage * 1000; /* mV -> uV */
			break;
		case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
			val->intval = power_info->design_capacity * 1000; /* mAh -> uAh */
			break;
		case POWER_SUPPLY_PROP_MODEL_NAME:
			val->strval = power_info->device_name;
			break;
		case POWER_SUPPLY_PROP_MANUFACTURER:
			val->strval = power_info->manufacturer_name;
			break;
		case POWER_SUPPLY_PROP_SERIAL_NUMBER:
			val->strval = power_info->serial_number;
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = power_info->technology;
			break;

		/* Get battery dynamic information. */
		case POWER_SUPPLY_PROP_STATUS:
			lemote3a_power_info_power_status_update();
			val->intval = power_info->charge_status;
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			lemote3a_power_info_power_status_update();
			val->intval = power_info->bat_in;
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			lemote3a_power_info_power_status_update();
			val->intval = power_info->health;
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			lemote3a_power_battery_info_update(BAT_REG_CURRENT_FLAG);
			val->intval = power_info->current_now * 1000; /* mA -> uA */
			break;
		case POWER_SUPPLY_PROP_CURRENT_AVG:
			lemote3a_power_battery_info_update(BAT_REG_AC_FLAG);
			val->intval = power_info->current_average * 1000; /* mA -> uA */
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			lemote3a_power_battery_info_update(BAT_REG_VOLTAGE_FLAG);
			val->intval =  power_info->voltage_now * 1000; /* mV -> uV */
			break;
		case POWER_SUPPLY_PROP_CHARGE_NOW:
			lemote3a_power_battery_info_update(BAT_REG_RC_FLAG);
			val->intval = power_info->remain_capacity * 1000; /* mAh -> uAh */
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			lemote3a_power_battery_info_update(BAT_REG_RSOC_FLAG);
			val->intval = power_info->remain_capacity_percent;	/* Percentage */
			break;
		case POWER_SUPPLY_PROP_TEMP:
			lemote3a_power_battery_info_update(BAT_REG_TEMP_FLAG);
			val->intval = power_info->temperature;	 /* Celcius */
			break;
		case POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG:
			lemote3a_power_battery_info_update(BAT_REG_ATTE_FLAG);
			if (power_info->remain_time == 0xFFFF) {
				power_info->remain_time = 0;
			}
			val->intval = power_info->remain_time * 60;  /* seconds */
			break;
		case POWER_SUPPLY_PROP_TIME_TO_FULL_AVG:
			lemote3a_power_battery_info_update(BAT_REG_ATTF_FLAG);
			if (power_info->fullchg_time == 0xFFFF) {
				power_info->fullchg_time = 0;
			}
			val->intval = power_info->fullchg_time * 60;  /* seconds */
			break;
		case POWER_SUPPLY_PROP_CHARGE_FULL:
			lemote3a_power_battery_info_update(BAT_REG_FCC_FLAG);
			val->intval = power_info->full_charged_capacity * 1000;/* mAh -> uAh */
			break;
		case POWER_SUPPLY_PROP_CYCLE_COUNT:
			lemote3a_power_battery_info_update(BAT_REG_CYCLCNT_FLAG);
			val->intval = power_info->cycle_count;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

/* Power supply AC get property handler */
static int lemote3a_ac_get_property(struct power_supply * pws,
			enum power_supply_property psp, union power_supply_propval * val)
{
	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			lemote3a_power_info_power_status_update();
			val->intval = power_info->ac_in;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

/* SCI device pci driver init handler */
static int sci_pci_driver_init(void)
{
	int ret;

	ret = sci_pci_init();
	if (ret) {
		printk(KERN_ERR "Lemote Laptop Platform Driver : Register pci driver error.\n");
		return ret;
	}

	printk(KERN_INFO "Lemote Laptop Platform Driver : SCI event handler on WPCE775L Embedded Controller init.\n");

	return ret;
}

/* SCI device pci driver exit handler */
static void sci_pci_driver_exit(void)
{
	printk(KERN_INFO "Lemote Laptop Platform Driver : SCI event handler on WPCE775L Embedded Controll exit.\n");
}

/* SCI device pci driver init */
static int sci_pci_init(void)
{
	int ret = -EIO;
	struct pci_dev *pdev;

	pdev = pci_get_device(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_SBX00_SMBUS, NULL);

	/* Create the sci device */
	lemote3a_sci_device = kmalloc(sizeof(struct sci_device), GFP_KERNEL);
	if (NULL == lemote3a_sci_device) {
		printk(KERN_ERR "LS3ANB Drvier : Malloc memory for sci_device failed!\n");
		return -ENOMEM;
	}

	/* Fill sci device */
	lemote3a_sci_device->irq = SCI_IRQ_NUM;
	lemote3a_sci_device->irq_data = 0x00;
	lemote3a_sci_device->number = 0x00;
	lemote3a_sci_device->parameter = 0x00;
	strcpy(lemote3a_sci_device->name, EC_SCI_DEV);

	/* Enable pci device and get the GPIO resources. */
	ret = pci_enable_device(pdev);
	if (ret) {
		printk(KERN_ERR "Lemote Laptop Platform Driver : Enable pci device failed!\n");
		ret = -ENODEV;
		goto out_pdev;
	}

	/* Clear sci status: GPM9Status field in bit14 of
	 * EVENT_STATUS register for SB710, write 1 to clear */
	clean_ec_event_status();

	/* Alloc the interrupt for sci not pci */
	ret = request_irq(lemote3a_sci_device->irq, lemote3a_sci_int_routine,
				IRQF_SHARED, lemote3a_sci_device->name, lemote3a_sci_device);
	if (ret) {
		printk(KERN_ERR "Lemote Laptop Platform Driver : Request irq %d failed!\n", lemote3a_sci_device->irq);
		ret = -EFAULT;
		goto out_irq;
	}

	ret = 0;
	printk(KERN_DEBUG "Lemote Laptop Platform Driver : PCI Init successful!\n");
	goto out;

out_irq:
	pci_disable_device(pdev);
out_pdev:
	kfree(lemote3a_sci_device);
out:
	return ret;
}

/* SCI event routine handler */
static irqreturn_t lemote3a_sci_int_routine(int irq, void * dev_id)
{
	int event;

	if (lemote3a_sci_device->irq != irq) {
		return IRQ_NONE;
	}

	event = ec_query_get_event_num();
	if ((SCI_EVENT_NUM_START > event) || (SCI_EVENT_NUM_END < event)) {
		goto exit_event_action;
	}

	/* Do event action */
	lemote3a_sci_event_handler(event);

	/* Clear sci status: GPM9Status field in bit14 of
	 * EVENT_STATUS register for SB710, write 1 to clear */
	clean_ec_event_status();

	return IRQ_HANDLED;

exit_event_action:
	clean_ec_event_status();
	return IRQ_NONE;
}

/* SCI device event handler */
void lemote3a_sci_event_handler(int event)
{
	int status = 0;
	struct key_entry * ke = NULL;
	struct sci_event * sep = NULL;

	sep = (struct sci_event*)&(se[event]);
	if (0 != sep->index) {
		status = ec_read(sep->index);
	}
	if (NULL != sep->handler) {
		status = sep->handler(status);
	}

	ke = sparse_keymap_entry_from_scancode(lemote3a_hotkey_dev, event);
	if (ke) {
		if (SW_LID == ke->keycode) {
			/* report LID event. */
			input_report_switch(lemote3a_hotkey_dev, SW_LID, status);
			input_sync(lemote3a_hotkey_dev);
		}
		else {
			sparse_keymap_report_entry(lemote3a_hotkey_dev, ke, 1, true);
		}
	}
}

extern void radeon_lvds_dpms_on(void);
extern void radeon_lvds_dpms_off(void);

static void lemote3a_lvds_dpms_callback(struct work_struct *dummy)
{
	int backlight_on = ec_read(INDEX_BACKLIGHT_STSCTRL);

	if (backlight_on)
		radeon_lvds_dpms_on();
	else
		radeon_lvds_dpms_off();
}

static DECLARE_WORK(lvds_dpms_work, lemote3a_lvds_dpms_callback);

static int lemote3a_dpms_handler(int status)
{
	schedule_work(&lvds_dpms_work);

	return 0;
}

/* SCI device over temperature event handler */
static int lemote3a_over_temp_handler(int status)
{
	return 0;
}

/* SCI device Throttling the CPU event handler */
static int lemote3a_throttling_CPU_handler(int status)
{
	return 0;
}

/* SCI device AC event handler */
static int lemote3a_ac_handler(int status)
{
	/* Report status changed */
	power_supply_changed(lemote3a_ac);

	return 0;
}

/* SCI device Battery event handler */
static int lemote3a_bat_handler(int status)
{
	/* Battery insert/pull-out to handle battery static information. */
	if (status & MASK(BIT_POWER_BATPRES)) {
		/* If battery is insert, get battery static information. */
		lemote3a_power_info_battery_static_update();
	}
	else {
		/* Else if battery is pull-out, clear battery static information. */
		lemote3a_power_info_battery_static_clear();
	}
	/* Report status changed */
	power_supply_changed(lemote3a_bat);

	return 0;
}

/* SCI device Battery low event handler */
static int lemote3a_bat_low_handler(int status)
{
	/* Report status changed */
	power_supply_changed(lemote3a_bat);

	return 0;
}

/* SCI device Battery very low event handler */
static int lemote3a_bat_very_low_handler(int status)
{
	/* Report status changed */
	power_supply_changed(lemote3a_bat);

	return 0;
}

/* SCI device LID event handler */
static int lemote3a_lid_handler(int status)
{
	if (status & BIT(BIT_DEVICE_LID)) {
		return 1;
	}

	return 0;
}

/* Set touchpad en/dis led */
static void lemote3a_tp_led_set(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
	int val = brightness ? TP_EN_LED_ON : TP_EN_LED_OFF;

	ec_write(INDEX_TOUCHPAD_ENABLE_LED, val);
}

/* Hotkey device init handler */
static int lemote3a_hotkey_init(void)
{
	int ret;

	lemote3a_hotkey_dev = input_allocate_device();
	if (!lemote3a_hotkey_dev) {
		return -ENOMEM;
	}

	lemote3a_hotkey_dev->name = "Lemote Laptop Hotkeys";
	lemote3a_hotkey_dev->phys = "button/input0";
	lemote3a_hotkey_dev->id.bustype = BUS_HOST;
	lemote3a_hotkey_dev->dev.parent = NULL;

	ret = sparse_keymap_setup(lemote3a_hotkey_dev, lemote3a_keymap, NULL);
	if (ret) {
		printk(KERN_ERR "Lemote Laptop Platform Driver : Fail to setup input device keymap\n");
		input_free_device(lemote3a_hotkey_dev);
		return ret;
	}

	ret = input_register_device(lemote3a_hotkey_dev);
	if (ret) {
		input_free_device(lemote3a_hotkey_dev);
		return ret;
	}
	return 0;
}

/* Hotkey device exit handler */
static void lemote3a_hotkey_exit(void)
{
	if (lemote3a_hotkey_dev) {
		input_unregister_device(lemote3a_hotkey_dev);
		lemote3a_hotkey_dev = NULL;
	}
}

module_init(lemote3a_laptop_init);
module_exit(lemote3a_laptop_exit);

MODULE_AUTHOR("Huang Wei <huangw@lemote.com>; Wang Rui <wangr@lemote.com>");
MODULE_DESCRIPTION("Lemote Loongson-3A/2Gq Laptop Driver");
MODULE_LICENSE("GPL");
