/*
 * nct7511 - Driver for Nuvoton NCT7511Y
 *
 * Copyright (C) 2014  Guenter Roeck <linux@roeck-us.net>
 * Copyright (C) 2019  Ainux <wangyao@lemote.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#define DRVNAME "nct7511"

static const u8 REG_VOLTAGE[5] = { 0x09, 0x0a, 0x0c, 0x0d, 0x0e };

static const u8 REG_VOLTAGE_LIMIT_LSB[2][5] = {
	{ 0x40, 0x00, 0x42, 0x44, 0x46 },
	{ 0x3f, 0x00, 0x41, 0x43, 0x45 },
};

static const u8 REG_VOLTAGE_LIMIT_MSB[5] = { 0x48, 0x00, 0x47, 0x47, 0x48 };

static const u8 REG_VOLTAGE_LIMIT_MSB_SHIFT[2][5] = {
	{ 0, 0, 4, 0, 4 },
	{ 2, 0, 6, 2, 6 },
};

#define REG_RTD1_MSB		0x01
#define REG_RTD2_MSB		0x02
#define REG_LTD_MSB		0x04
#define REG_TEMP_LSB		0x05
#define REG_FANCOUNT_MSB	0x10
#define REG_FANCOUNT_LSB	0x13
#define REG_DFAS		0x17
#define REG_LAS			0x18
#define REG_HAS			0x19
#define REG_FAS			0x1a
#define REG_CAS			0x1b
#define REG_START		0x21
#define REG_MODE		0x22 /* 7.2.32 Mode Selection Register */
#define REG_FAN_ENABLE		0x24
#define REG_VMON_ENABLE		0x25
#define REG_RTD1_THL		0x30
#define REG_RTD1_TLL		0x31
#define REG_RTD2_THL		0x32
#define REG_RTD2_TLL		0x33
#define REG_LTD_THL		0x36
#define REG_LTD_TLL		0x37
#define REG_RTD1_CRIT		0x3a
#define REG_RTD2_CRIT		0x3b
#define REG_LTD_CRIT		0x3d
#define REG_FANLIMIT_LSB	0x49
#define REG_FANLIMIT_MSB	0x4c
#define REG_FANOUTPUT_CRTTYPE	0x5e
#define REG_PWM(x)		(0x60 + (x))
#define REG_SMARTFAN_EN(x)      (0x64 + (x) / 2)
#define SMARTFAN_EN_SHIFT(x)    ((x) % 2 * 4)
#define REG_VENDOR_ID		0xfd
#define REG_CHIP_ID		0xfe
#define REG_DEVICE_ID		0xff

/*
 * Data structures and manipulation thereof
 */

struct nct7511_data {
	struct regmap *regmap;
	struct mutex access_lock; /* for multi-byte read and write operations */
};

static ssize_t show_temp_label(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct sensor_device_attribute *sattr = to_sensor_dev_attr(attr);

	switch (sattr->index) {
	case 0:
		return sprintf(buf, "%s\n", "RTD1 Temperature");
		break;
	case 1:
		return sprintf(buf, "%s\n", "RTD2 Temperature");
		break;
	case 2:
		return sprintf(buf, "%s\n", "LTD Temperature");
		break;
	default:
		return -EOPNOTSUPP;
	}
}

static ssize_t show_pwm_mode(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct sensor_device_attribute *sattr = to_sensor_dev_attr(attr);
	struct nct7511_data *data = dev_get_drvdata(dev);
	unsigned int regval;
	int ret;

	if (sattr->index > 1)
		return sprintf(buf, "1\n");

	ret = regmap_read(data->regmap, REG_FANOUTPUT_CRTTYPE, &regval);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%u\n", regval & (1 << sattr->index));
}

static ssize_t show_pwm(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct nct7511_data *data = dev_get_drvdata(dev);
	unsigned int val;
	int ret;

	if (!attr->index)
		return sprintf(buf, "255\n");

	ret = regmap_read(data->regmap, attr->index, &val);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%d\n", val);
}

static ssize_t store_pwm(struct device *dev, struct device_attribute *devattr,
			 const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct nct7511_data *data = dev_get_drvdata(dev);
	int err;
	u8 val;

	err = kstrtou8(buf, 0, &val);
	if (err < 0)
		return err;

	err = regmap_write(data->regmap, attr->index, val);
	return err ? : count;
}

static ssize_t show_pwm_enable(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct nct7511_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *sattr = to_sensor_dev_attr(attr);
	unsigned int reg, enabled;
	int ret;

	ret = regmap_read(data->regmap, REG_SMARTFAN_EN(sattr->index), &reg);
	if (ret < 0)
		return ret;
	enabled = reg >> SMARTFAN_EN_SHIFT(sattr->index) & 1;
	return sprintf(buf, "%u\n", enabled);
}

static ssize_t store_pwm_enable(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct nct7511_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute *sattr = to_sensor_dev_attr(attr);
	u8 val;
	int ret;

	ret = kstrtou8(buf, 0, &val);
	if (ret < 0)
		return ret;
	if (val > 1)
		return -EINVAL;
	ret = regmap_update_bits(data->regmap, REG_SMARTFAN_EN(sattr->index),
				 1 << SMARTFAN_EN_SHIFT(sattr->index),
				 val << SMARTFAN_EN_SHIFT(sattr->index));
	return ret ? : count;
}

static int nct7511_read_temp(struct nct7511_data *data,
			     u8 reg_temp, u8 reg_temp_low, int *temp)
{
	unsigned int t1, t2 = 0;
	int err;

	*temp = 0;

	mutex_lock(&data->access_lock);
	err = regmap_read(data->regmap, reg_temp, &t1);
	if (err < 0)
		goto abort;
	t1 <<= 8;
	if (reg_temp_low) {	/* 11 bit data */
		err = regmap_read(data->regmap, reg_temp_low, &t2);
		if (err < 0)
			goto abort;
	}
	t1 |= t2 & 0xe0;
	*temp = (s16)t1 / 32 * 125;
abort:
	mutex_unlock(&data->access_lock);
	return err;
}

static int nct7511_read_fan(struct nct7511_data *data, u8 reg_fan)
{
	unsigned int f1, f2;
	int ret;

	mutex_lock(&data->access_lock);
	ret = regmap_read(data->regmap, reg_fan, &f1);
	if (ret < 0)
		goto abort;
	ret = regmap_read(data->regmap, REG_FANCOUNT_LSB, &f2);
	if (ret < 0)
		goto abort;
	ret = (f1 << 5) | (f2 >> 3);
	/* convert fan count to rpm */
	if (ret == 0x1fff)	/* maximum value, assume fan is stopped */
		ret = 0;
	else if (ret)
		ret = DIV_ROUND_CLOSEST(1350000U, ret);
abort:
	mutex_unlock(&data->access_lock);
	return ret;
}

static int nct7511_read_fan_min(struct nct7511_data *data, u8 reg_fan_low,
				u8 reg_fan_high)
{
	unsigned int f1, f2;
	int ret;

	mutex_lock(&data->access_lock);
	ret = regmap_read(data->regmap, reg_fan_low, &f1);
	if (ret < 0)
		goto abort;
	ret = regmap_read(data->regmap, reg_fan_high, &f2);
	if (ret < 0)
		goto abort;
	ret = f1 | ((f2 & 0xf8) << 5);
	/* convert fan count to rpm */
	if (ret == 0x1fff)	/* maximum value, assume no limit */
		ret = 0;
	else if (ret)
		ret = DIV_ROUND_CLOSEST(1350000U, ret);
	else
		ret = 1350000U;
abort:
	mutex_unlock(&data->access_lock);
	return ret;
}

static int nct7511_write_fan_min(struct nct7511_data *data, u8 reg_fan_low,
				 u8 reg_fan_high, unsigned long limit)
{
	int err;

	if (limit)
		limit = DIV_ROUND_CLOSEST(1350000U, limit);
	else
		limit = 0x1fff;
	limit = clamp_val(limit, 0, 0x1fff);

	mutex_lock(&data->access_lock);
	err = regmap_write(data->regmap, reg_fan_low, limit & 0xff);
	if (err < 0)
		goto abort;

	err = regmap_write(data->regmap, reg_fan_high, (limit & 0x1f00) >> 5);
abort:
	mutex_unlock(&data->access_lock);
	return err;
}

static ssize_t show_temp(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct nct7511_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	int err, temp;

	err = nct7511_read_temp(data, sattr->nr, sattr->index, &temp);
	if (err < 0)
		return err;

	return sprintf(buf, "%d\n", temp);
}

static ssize_t store_temp(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct nct7511_data *data = dev_get_drvdata(dev);
	int nr = sattr->nr;
	long val;
	int err;

	err = kstrtol(buf, 10, &val);
	if (err < 0)
		return err;

	val = DIV_ROUND_CLOSEST(clamp_val(val, -128000, 127000), 1000);

	err = regmap_write(data->regmap, nr, val & 0xff);
	return err ? : count;
}

static ssize_t show_fan(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct sensor_device_attribute *sattr = to_sensor_dev_attr(attr);
	struct nct7511_data *data = dev_get_drvdata(dev);
	int speed;

	speed = nct7511_read_fan(data, sattr->index);
	if (speed < 0)
		return speed;

	return sprintf(buf, "%d\n", speed);
}

static ssize_t show_fan_min(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct nct7511_data *data = dev_get_drvdata(dev);
	int speed;

	speed = nct7511_read_fan_min(data, sattr->nr, sattr->index);
	if (speed < 0)
		return speed;

	return sprintf(buf, "%d\n", speed);
}

static ssize_t store_fan_min(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	struct nct7511_data *data = dev_get_drvdata(dev);
	unsigned long val;
	int err;

	err = kstrtoul(buf, 10, &val);
	if (err < 0)
		return err;

	err = nct7511_write_fan_min(data, sattr->nr, sattr->index, val);
	return err ? : count;
}

static ssize_t show_alarm(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct nct7511_data *data = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *sattr = to_sensor_dev_attr_2(attr);
	int bit = sattr->index;
	unsigned int val;
	int ret;

	ret = regmap_read(data->regmap, sattr->nr, &val);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%u\n", !!(val & (1 << bit)));
}

static SENSOR_DEVICE_ATTR(temp1_label, S_IRUGO,
			  show_temp_label, NULL, 0);
static SENSOR_DEVICE_ATTR_2(temp1_input, S_IRUGO, show_temp, NULL, REG_RTD1_MSB,
			    REG_TEMP_LSB);
static SENSOR_DEVICE_ATTR_2(temp1_min, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_RTD1_TLL, 0);
static SENSOR_DEVICE_ATTR_2(temp1_max, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_RTD1_THL, 0);
static SENSOR_DEVICE_ATTR_2(temp1_crit, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_RTD1_CRIT, 0);

static SENSOR_DEVICE_ATTR(temp2_label, S_IRUGO,
			  show_temp_label, NULL, 1);
static SENSOR_DEVICE_ATTR_2(temp2_input, S_IRUGO, show_temp, NULL, REG_RTD2_MSB,
			    REG_TEMP_LSB);
static SENSOR_DEVICE_ATTR_2(temp2_min, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_RTD2_TLL, 0);
static SENSOR_DEVICE_ATTR_2(temp2_max, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_RTD2_THL, 0);
static SENSOR_DEVICE_ATTR_2(temp2_crit, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_RTD2_CRIT, 0);

static SENSOR_DEVICE_ATTR(temp3_label, S_IRUGO,
			  show_temp_label, NULL, 2);
static SENSOR_DEVICE_ATTR_2(temp3_input, S_IRUGO, show_temp, NULL, REG_LTD_MSB,
			    REG_TEMP_LSB);
static SENSOR_DEVICE_ATTR_2(temp3_min, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_LTD_TLL, 0);
static SENSOR_DEVICE_ATTR_2(temp3_max, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_LTD_THL, 0);
static SENSOR_DEVICE_ATTR_2(temp3_crit, S_IRUGO | S_IWUSR, show_temp,
			    store_temp, REG_LTD_CRIT, 0);

static SENSOR_DEVICE_ATTR_2(temp1_min_alarm, S_IRUGO, show_alarm, NULL,
			    REG_LAS, 0);
static SENSOR_DEVICE_ATTR_2(temp2_min_alarm, S_IRUGO, show_alarm, NULL,
			    REG_LAS, 1);
static SENSOR_DEVICE_ATTR_2(temp3_min_alarm, S_IRUGO, show_alarm, NULL,
			    REG_LAS, 2);

static SENSOR_DEVICE_ATTR_2(temp1_max_alarm, S_IRUGO, show_alarm, NULL,
			    REG_HAS, 0);
static SENSOR_DEVICE_ATTR_2(temp2_max_alarm, S_IRUGO, show_alarm, NULL,
			    REG_HAS, 1);
static SENSOR_DEVICE_ATTR_2(temp3_max_alarm, S_IRUGO, show_alarm, NULL,
			    REG_HAS, 2);

static SENSOR_DEVICE_ATTR_2(temp1_crit_alarm, S_IRUGO, show_alarm, NULL,
			    REG_CAS, 0);
static SENSOR_DEVICE_ATTR_2(temp2_crit_alarm, S_IRUGO, show_alarm, NULL,
			    REG_CAS, 1);
static SENSOR_DEVICE_ATTR_2(temp3_crit_alarm, S_IRUGO, show_alarm, NULL,
			    REG_CAS, 2);

static SENSOR_DEVICE_ATTR_2(temp1_fault, S_IRUGO, show_alarm, NULL, REG_DFAS, 0);
static SENSOR_DEVICE_ATTR_2(temp2_fault, S_IRUGO, show_alarm, NULL, REG_DFAS, 1);
static SENSOR_DEVICE_ATTR_2(temp3_fault, S_IRUGO, show_alarm, NULL, REG_DFAS, 2);

static struct attribute *nct7511_temp_attrs[] = {
	&sensor_dev_attr_temp1_label.dev_attr.attr,
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp1_min.dev_attr.attr,
	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp1_crit.dev_attr.attr,
	&sensor_dev_attr_temp1_min_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_max_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_crit_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_fault.dev_attr.attr,

	&sensor_dev_attr_temp2_label.dev_attr.attr,		/* 9 */
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp2_min.dev_attr.attr,
	&sensor_dev_attr_temp2_max.dev_attr.attr,
	&sensor_dev_attr_temp2_crit.dev_attr.attr,
	&sensor_dev_attr_temp2_min_alarm.dev_attr.attr,
	&sensor_dev_attr_temp2_max_alarm.dev_attr.attr,
	&sensor_dev_attr_temp2_crit_alarm.dev_attr.attr,
	&sensor_dev_attr_temp2_fault.dev_attr.attr,

	&sensor_dev_attr_temp3_label.dev_attr.attr,		/* 18 */
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp3_min.dev_attr.attr,
	&sensor_dev_attr_temp3_max.dev_attr.attr,
	&sensor_dev_attr_temp3_crit.dev_attr.attr,
	&sensor_dev_attr_temp3_min_alarm.dev_attr.attr,
	&sensor_dev_attr_temp3_max_alarm.dev_attr.attr,
	&sensor_dev_attr_temp3_crit_alarm.dev_attr.attr,
	&sensor_dev_attr_temp3_fault.dev_attr.attr,

	NULL
};

static umode_t nct7511_temp_is_visible(struct kobject *kobj,
				       struct attribute *attr, int index)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct nct7511_data *data = dev_get_drvdata(dev);
	unsigned int reg;
	int err;

	err = regmap_read(data->regmap, REG_MODE, &reg);
	if (err < 0)
		return 0;

	if (index < 10 && (reg & 0x03) != 0x01)			/* RD1 */
		return 0;

	if (index >= 10 && index < 20 && (reg & 0x0c) != 0x04)	/* RD2 */
		return 0;

	if (index >= 30 && index < 38 && (reg & 0x40) != 0x40)	/* LTD */
		return 0;

	return attr->mode;
}

static const struct attribute_group nct7511_temp_group = {
	.attrs = nct7511_temp_attrs,
	.is_visible = nct7511_temp_is_visible,
};

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, show_fan, NULL, REG_FANCOUNT_MSB);
static SENSOR_DEVICE_ATTR_2(fan1_min, S_IRUGO | S_IWUSR, show_fan_min,
			    store_fan_min, REG_FANLIMIT_LSB, REG_FANLIMIT_MSB);
static SENSOR_DEVICE_ATTR_2(fan1_alarm, S_IRUGO, show_alarm, NULL, REG_FAS, 0);

/* 7.2.42 Fan Control Output Type */
static SENSOR_DEVICE_ATTR(pwm1_mode, S_IRUGO, show_pwm_mode, NULL, 0);

/* 7.2.44 Fan Control Output Value */
static SENSOR_DEVICE_ATTR(pwm1, S_IRUGO | S_IWUSR, show_pwm, store_pwm,
			  REG_PWM(0));

/* 7.2.46... Temperature to Fan mapping Relationships Register */
static SENSOR_DEVICE_ATTR(pwm1_enable, S_IRUGO | S_IWUSR, show_pwm_enable,
			  store_pwm_enable, 0);
static SENSOR_DEVICE_ATTR(pwm2_enable, S_IRUGO | S_IWUSR, show_pwm_enable,
			  store_pwm_enable, 1);
static SENSOR_DEVICE_ATTR(pwm3_enable, S_IRUGO | S_IWUSR, show_pwm_enable,
			  store_pwm_enable, 2);

static struct attribute *nct7511_fan_attrs[] = {
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan1_min.dev_attr.attr,
	&sensor_dev_attr_fan1_alarm.dev_attr.attr,

	NULL
};

static umode_t nct7511_fan_is_visible(struct kobject *kobj,
				      struct attribute *attr, int index)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct nct7511_data *data = dev_get_drvdata(dev);
	int fan = index / 4;	/* 4 attributes per fan */
	unsigned int reg;
	int err;

	err = regmap_read(data->regmap, REG_FAN_ENABLE, &reg);
	if (err < 0 || !(reg & (1 << fan)))
		return 0;

	return attr->mode;
}

static const struct attribute_group nct7511_fan_group = {
	.attrs = nct7511_fan_attrs,
	.is_visible = nct7511_fan_is_visible,
};

static struct attribute *nct7511_pwm_attrs[] = {
	&sensor_dev_attr_pwm1_enable.dev_attr.attr,
	&sensor_dev_attr_pwm1_mode.dev_attr.attr,
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_pwm2_enable.dev_attr.attr,
	&sensor_dev_attr_pwm3_enable.dev_attr.attr,
	NULL
};

static const struct attribute_group nct7511_pwm_group = {
	.attrs = nct7511_pwm_attrs,
};

/* 7.2.63... 0x80-0x83, 0x84 Temperature (X-axis) transition */
static SENSOR_DEVICE_ATTR_2(pwm1_auto_point1_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x80, 0);
static SENSOR_DEVICE_ATTR_2(pwm1_auto_point2_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x81, 0);
static SENSOR_DEVICE_ATTR_2(pwm1_auto_point3_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x82, 0);
static SENSOR_DEVICE_ATTR_2(pwm1_auto_point4_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x83, 0);
static SENSOR_DEVICE_ATTR_2(pwm1_auto_point5_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x84, 0);

/* 7.2.63... 0x85-0x88 PWM (Y-axis) transition */
static SENSOR_DEVICE_ATTR(pwm1_auto_point1_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x85);
static SENSOR_DEVICE_ATTR(pwm1_auto_point2_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x86);
static SENSOR_DEVICE_ATTR(pwm1_auto_point3_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x87);
static SENSOR_DEVICE_ATTR(pwm1_auto_point4_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x88);
static SENSOR_DEVICE_ATTR(pwm1_auto_point5_pwm, S_IRUGO, show_pwm, NULL, 0);

/* 7.2.63 Table 2 X-axis Transition Point 1 Register */
static SENSOR_DEVICE_ATTR_2(pwm2_auto_point1_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x90, 0);
static SENSOR_DEVICE_ATTR_2(pwm2_auto_point2_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x91, 0);
static SENSOR_DEVICE_ATTR_2(pwm2_auto_point3_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x92, 0);
static SENSOR_DEVICE_ATTR_2(pwm2_auto_point4_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x93, 0);
static SENSOR_DEVICE_ATTR_2(pwm2_auto_point5_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0x94, 0);

/* 7.2.63 Table 2 Y-axis Transition Point 1 Register */
static SENSOR_DEVICE_ATTR(pwm2_auto_point1_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x95);
static SENSOR_DEVICE_ATTR(pwm2_auto_point2_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x96);
static SENSOR_DEVICE_ATTR(pwm2_auto_point3_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x97);
static SENSOR_DEVICE_ATTR(pwm2_auto_point4_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0x98);
static SENSOR_DEVICE_ATTR(pwm2_auto_point5_pwm, S_IRUGO, show_pwm, NULL, 0);

/* 7.2.63 Table 3 X-axis Transition Point 1 Register */
static SENSOR_DEVICE_ATTR_2(pwm3_auto_point1_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0xA0, 0);
static SENSOR_DEVICE_ATTR_2(pwm3_auto_point2_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0xA1, 0);
static SENSOR_DEVICE_ATTR_2(pwm3_auto_point3_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0xA2, 0);
static SENSOR_DEVICE_ATTR_2(pwm3_auto_point4_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0xA3, 0);
static SENSOR_DEVICE_ATTR_2(pwm3_auto_point5_temp, S_IRUGO | S_IWUSR,
			    show_temp, store_temp, 0xA4, 0);

/* 7.2.63 Table 3 Y-axis Transition Point 1 Register */
static SENSOR_DEVICE_ATTR(pwm3_auto_point1_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0xA5);
static SENSOR_DEVICE_ATTR(pwm3_auto_point2_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0xA6);
static SENSOR_DEVICE_ATTR(pwm3_auto_point3_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0xA7);
static SENSOR_DEVICE_ATTR(pwm3_auto_point4_pwm, S_IRUGO | S_IWUSR,
			  show_pwm, store_pwm, 0xA8);
static SENSOR_DEVICE_ATTR(pwm3_auto_point5_pwm, S_IRUGO, show_pwm, NULL, 0);

static struct attribute *nct7511_auto_point_attrs[] = {
	&sensor_dev_attr_pwm1_auto_point1_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point2_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point3_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point4_temp.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point5_temp.dev_attr.attr,

	&sensor_dev_attr_pwm1_auto_point1_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point2_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point3_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point4_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm1_auto_point5_pwm.dev_attr.attr,

	&sensor_dev_attr_pwm2_auto_point1_temp.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point2_temp.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point3_temp.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point4_temp.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point5_temp.dev_attr.attr,

	&sensor_dev_attr_pwm2_auto_point1_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point2_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point3_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point4_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm2_auto_point5_pwm.dev_attr.attr,

	&sensor_dev_attr_pwm3_auto_point1_temp.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point2_temp.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point3_temp.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point4_temp.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point5_temp.dev_attr.attr,

	&sensor_dev_attr_pwm3_auto_point1_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point2_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point3_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point4_pwm.dev_attr.attr,
	&sensor_dev_attr_pwm3_auto_point5_pwm.dev_attr.attr,

	NULL
};

static const struct attribute_group nct7511_auto_point_group = {
	.attrs = nct7511_auto_point_attrs,
};

static const struct attribute_group *nct7511_groups[] = {
	&nct7511_temp_group,
	&nct7511_fan_group,
	&nct7511_pwm_group,
	&nct7511_auto_point_group,
	NULL
};

static int nct7511_detect(struct i2c_client *client,
			  struct i2c_board_info *info)
{
	int reg;

	reg = i2c_smbus_read_byte_data(client, REG_VENDOR_ID);
	if (reg != 0x50)
		return -ENODEV;

	reg = i2c_smbus_read_byte_data(client, REG_CHIP_ID);
	if (reg != 0xc3)
		return -ENODEV;

	reg = i2c_smbus_read_byte_data(client, REG_DEVICE_ID);
	/* Device id range is 2xh(x = 0,1,2...) */
	if (reg < 0x20 || reg > 0x2f)
		return -ENODEV;

	/* Also validate lower bits of temperature registers */
	reg = i2c_smbus_read_byte_data(client, REG_TEMP_LSB);
	if (reg < 0 || (reg & 0x1f))
		return -ENODEV;

	strlcpy(info->type, "nct7511", I2C_NAME_SIZE);
	return 0;
}

static bool nct7511_regmap_is_volatile(struct device *dev, unsigned int reg)
{
	return (reg <= 0x20) ||
		(reg >= REG_PWM(0) && reg <= REG_PWM(2));
}

static const struct regmap_config nct7511_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.volatile_reg = nct7511_regmap_is_volatile,
};

static int nct7511_init_chip(struct nct7511_data *data)
{
	int err;

	/* Enable ADC */
	err = regmap_update_bits(data->regmap, REG_START, 0x01, 0x01);
	if (err)
		return err;

	/* Enable local temperature sensor */
	err = regmap_update_bits(data->regmap, REG_MODE, 0x40, 0x40);
	if (err)
		return err;

	/* Enable Vcore and VCC voltage monitoring */
	return regmap_update_bits(data->regmap, REG_VMON_ENABLE, 0x03, 0x03);
}

static int nct7511_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct nct7511_data *data;
	struct device *hwmon_dev;
	int ret;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (data == NULL)
		return -ENOMEM;

	data->regmap = devm_regmap_init_i2c(client, &nct7511_regmap_config);
	if (IS_ERR(data->regmap))
		return PTR_ERR(data->regmap);

	mutex_init(&data->access_lock);

	ret = nct7511_init_chip(data);
	if (ret < 0)
		return ret;

	hwmon_dev = devm_hwmon_device_register_with_groups(dev, client->name,
							   data,
							   nct7511_groups);
	return PTR_ERR_OR_ZERO(hwmon_dev);
}

static const unsigned short nct7511_address_list[] = {
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, I2C_CLIENT_END
};

static const struct i2c_device_id nct7511_idtable[] = {
	{ "nct7511", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, nct7511_idtable);

static struct i2c_driver nct7511_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = DRVNAME,
	},
	.detect = nct7511_detect,
	.probe = nct7511_probe,
	.id_table = nct7511_idtable,
	.address_list = nct7511_address_list,
};

module_i2c_driver(nct7511_driver);

MODULE_AUTHOR("Guenter Roeck <linux@roeck-us.net>");
MODULE_AUTHOR("Ainux <wangyao@lemote.com>");
MODULE_DESCRIPTION("NCT7511Y Hardware Monitoring Driver");
MODULE_LICENSE("GPL v2");
