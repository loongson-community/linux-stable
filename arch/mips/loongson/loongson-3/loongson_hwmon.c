#include <linux/err.h>
#include <linux/module.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include <loongson_hwmon.h>

/* ========================= Hwmon ====================== */
static struct device *loongson_hwmon_dev;

static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf);
static SENSOR_DEVICE_ATTR(name, S_IRUGO, get_hwmon_name, NULL, 0);

static struct attribute *loongson_hwmon_attributes[] =
{
	&sensor_dev_attr_name.dev_attr.attr,
	NULL
};

/* Hwmon device attribute group */
static struct attribute_group loongson_hwmon_attribute_group =
{
	.attrs = loongson_hwmon_attributes,
};

/* Hwmon device get name */
static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "loongson-hwmon\n");
}

/* ========================= Temperature ====================== */

struct loongson_temp_info loongson_temp_info;
static ssize_t get_cpu0_temp(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t get_nb_temp(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t get_sb_temp(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t get_mb_temp(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t get_psy_temp(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t get_cpu1_temp(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t cpu0_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t nb_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t sb_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t mb_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t psy_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t cpu1_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf);

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, get_cpu0_temp, NULL, 1);
static SENSOR_DEVICE_ATTR(temp1_label, S_IRUGO, cpu0_temp_label, NULL, 1);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, get_nb_temp, NULL, 2);
static SENSOR_DEVICE_ATTR(temp2_label, S_IRUGO, nb_temp_label, NULL, 2);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, get_sb_temp, NULL, 3);
static SENSOR_DEVICE_ATTR(temp3_label, S_IRUGO, sb_temp_label, NULL, 3);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO, get_mb_temp, NULL, 4);
static SENSOR_DEVICE_ATTR(temp4_label, S_IRUGO, mb_temp_label, NULL, 4);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO, get_psy_temp, NULL, 5);
static SENSOR_DEVICE_ATTR(temp5_label, S_IRUGO, psy_temp_label, NULL, 5);
static SENSOR_DEVICE_ATTR(temp6_input, S_IRUGO, get_cpu1_temp, NULL, 6);
static SENSOR_DEVICE_ATTR(temp6_label, S_IRUGO, cpu1_temp_label, NULL, 6);

static const struct attribute *hwmon_temp1[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp1_label.dev_attr.attr,
	NULL
};

static const struct attribute *hwmon_temp2[] = {
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp2_label.dev_attr.attr,
	NULL
};

static const struct attribute *hwmon_temp3[] = {
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp3_label.dev_attr.attr,
	NULL
};

static const struct attribute *hwmon_temp4[] = {
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp4_label.dev_attr.attr,
	NULL
};

static const struct attribute *hwmon_temp5[] = {
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_temp5_label.dev_attr.attr,
	NULL
};

static const struct attribute *hwmon_temp6[] = {
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_temp6_label.dev_attr.attr,
	NULL
};

static ssize_t cpu0_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "CPU 0 Temprature\n");
}

static ssize_t cpu1_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "CPU 1 Temprature\n");
}

static ssize_t nb_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "North Bridge Temprature\n");
}

static ssize_t sb_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "South Bridge Temprature\n");
}

static ssize_t mb_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "Main Board Temprature\n");
}

static ssize_t psy_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "Power Supply Temprature\n");
}

static u32 get_temp(get_temp_fun fun, void *param)
{
	u32 value = NOT_VALID_TEMP;

	if (fun && !param)
		value = fun(0);
	if (fun && param)
		value = fun(*(int *)param);

	return value;
}

static ssize_t get_cpu0_temp(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 value;
	int nr = (to_sensor_dev_attr(attr))->index;

	value = get_temp(loongson_temp_info.get_cpu0_temp, &nr);
	return sprintf(buf, "%d\n", value);
}

static ssize_t get_cpu1_temp(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 value;
	int nr = (to_sensor_dev_attr(attr))->index;

	value = get_temp(loongson_temp_info.get_cpu1_temp, &nr);
	return sprintf(buf, "%d\n", value);
}

static ssize_t get_nb_temp(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 value = NOT_VALID_TEMP;
	int nr = (to_sensor_dev_attr(attr))->index;

	value = get_temp(loongson_temp_info.get_nb_temp, &nr);
	return sprintf(buf, "%d\n", value);
}

static ssize_t get_sb_temp(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 value = NOT_VALID_TEMP;
	int nr = (to_sensor_dev_attr(attr))->index;

	value = get_temp(loongson_temp_info.get_sb_temp, &nr);
	return sprintf(buf, "%d\n", value);
}

static ssize_t get_mb_temp(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 value = NOT_VALID_TEMP;
	int nr = (to_sensor_dev_attr(attr))->index;

	value = get_temp(loongson_temp_info.get_mb_temp, &nr);
	return sprintf(buf, "%d\n", value);
}

static ssize_t get_psy_temp(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 value = NOT_VALID_TEMP;
	int nr = (to_sensor_dev_attr(attr))->index;

	value = get_temp(loongson_temp_info.get_psy_temp, &nr);
	return sprintf(buf, "%d\n", value);
}

static int create_sysfs_temp_files(struct kobject *kobj)
{
	int ret;

	if (loongson_temp_info.get_cpu0_temp) {
		ret = sysfs_create_files(kobj, hwmon_temp1);
		if (ret)
			goto sysfs_create_temp1_fail;
	}

	if (loongson_temp_info.get_nb_temp) {
		ret = sysfs_create_files(kobj, hwmon_temp2);
		if (ret)
			goto sysfs_create_temp2_fail;
	}

	if (loongson_temp_info.get_sb_temp) {
		ret = sysfs_create_files(kobj, hwmon_temp3);
		if (ret)
			goto sysfs_create_temp3_fail;
	}

	if (loongson_temp_info.get_mb_temp) {
		ret = sysfs_create_files(kobj, hwmon_temp4);
		if (ret)
			goto sysfs_create_temp4_fail;
	}

	if (loongson_temp_info.get_psy_temp) {
		ret = sysfs_create_files(kobj, hwmon_temp5);
		if (ret)
			goto sysfs_create_temp5_fail;
	}

	if (loongson_temp_info.get_cpu1_temp) {
		ret = sysfs_create_files(kobj, hwmon_temp6);
		if (ret)
			goto sysfs_create_temp6_fail;
	}
	return 0;

sysfs_create_temp6_fail:
	if (loongson_temp_info.get_psy_temp)
		sysfs_remove_files(kobj, hwmon_temp5);

sysfs_create_temp5_fail:
	if (loongson_temp_info.get_mb_temp)
		sysfs_remove_files(kobj, hwmon_temp4);

sysfs_create_temp4_fail:
	if (loongson_temp_info.get_sb_temp)
		sysfs_remove_files(kobj, hwmon_temp3);

sysfs_create_temp3_fail:
	if (loongson_temp_info.get_nb_temp)
		sysfs_remove_files(kobj, hwmon_temp2);

sysfs_create_temp2_fail:
	if (loongson_temp_info.get_cpu0_temp)
		sysfs_remove_files(kobj, hwmon_temp1);

sysfs_create_temp1_fail:
	return -1;
}

static void remove_sysfs_temp_files(struct kobject *kobj)
{
	if (loongson_temp_info.get_cpu0_temp)
		sysfs_remove_files(&loongson_hwmon_dev->kobj, hwmon_temp1);

	if (loongson_temp_info.get_nb_temp)
		sysfs_remove_files(&loongson_hwmon_dev->kobj, hwmon_temp2);

	if (loongson_temp_info.get_sb_temp)
		sysfs_remove_files(&loongson_hwmon_dev->kobj, hwmon_temp3);

	if (loongson_temp_info.get_mb_temp)
		sysfs_remove_files(&loongson_hwmon_dev->kobj, hwmon_temp4);

	if (loongson_temp_info.get_psy_temp)
		sysfs_remove_files(&loongson_hwmon_dev->kobj, hwmon_temp5);

	if (loongson_temp_info.get_cpu1_temp)
		sysfs_remove_files(&loongson_hwmon_dev->kobj, hwmon_temp6);
}


struct loongson_fan_ops loongson_fan1_ops, loongson_fan2_ops, loongson_fan3_ops;

/* ========================= Fan 1 ====================== */

static ssize_t get_fan1_level(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan1_level(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan1_mode(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan1_mode(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan1_speed(struct device *dev,
			struct device_attribute *attr, char *buf);

static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR | S_IRUGO,
				get_fan1_level, set_fan1_level, 0);
static SENSOR_DEVICE_ATTR(pwm1_enable, S_IWUSR | S_IRUGO,
				get_fan1_mode, set_fan1_mode, 0);
static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, get_fan1_speed, NULL, 0);

static const struct attribute *hwmon_pwm1 =
			&sensor_dev_attr_pwm1.dev_attr.attr;
static const struct attribute *hwmon_pwm1_enable =
			&sensor_dev_attr_pwm1_enable.dev_attr.attr;
static const struct attribute *hwmon_fan1_input =
			&sensor_dev_attr_fan1_input.dev_attr.attr;

static ssize_t get_fan1_level(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u8 val;

	val = (loongson_fan1_ops.get_fan_level)();
	return sprintf(buf, "%d\n", val);
}

static ssize_t set_fan1_level(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_level;

	new_level = SENSORS_LIMIT(simple_strtoul(buf, NULL, 10), 0, 255);
	(loongson_fan1_ops.set_fan_level)(new_level);

	return count;
}

static ssize_t get_fan1_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	enum fan_control_mode fan1_mode;

	fan1_mode = (loongson_fan1_ops.get_fan_mode)();
	return sprintf(buf, "%d\n", fan1_mode);
}

static ssize_t set_fan1_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_mode;

	new_mode = SENSORS_LIMIT(simple_strtoul(buf, NULL, 10),
					FAN_FULL_MODE, FAN_AUTO_MODE);
	(loongson_fan1_ops.set_fan_mode)(new_mode);

	return count;
}

static ssize_t get_fan1_speed(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 val;

	val = (loongson_fan1_ops.get_fan_speed)();
	return sprintf(buf, "%d\n", val);
}

/* should be call by create_sysfs_fan_files */
static int create_sysfs_fan1_files(struct kobject *kobj)
{
	int ret;

	ret = sysfs_create_file(kobj, hwmon_pwm1_enable);
	if (ret) {
		printk(KERN_ERR "fail to create sysfs hwmon_pwm1_enable\n");
		goto fail_to_create_hwmon_pwm1_enable;
	}

	if (loongson_fan1_ops.set_fan_level) {
		ret = sysfs_create_file(kobj, hwmon_pwm1);
		if (ret) {
			printk(KERN_ERR "fail to create sysfs hwmon_pwm1\n");
			goto fail_to_create_hwmon_pwm1;
		}
	}

	if (loongson_fan1_ops.get_fan_speed) {
		ret = sysfs_create_file(kobj, hwmon_fan1_input);
		if (ret) {
			printk(KERN_ERR "fail to create sysfs hwmon_fan1_input\n");
			goto fail_to_create_hwmon_fan1_input;
		}
	}
	/* ready to support more interface later... */

	return 0;

fail_to_create_hwmon_fan1_input:
	sysfs_remove_file(kobj, hwmon_pwm1);

fail_to_create_hwmon_pwm1:
	sysfs_remove_file(kobj, hwmon_pwm1_enable);

fail_to_create_hwmon_pwm1_enable:
	return -1;
}

static void remove_sysfs_fan1_files(struct kobject *kobj)
{
	sysfs_remove_file(kobj, hwmon_pwm1_enable);

	if (loongson_fan1_ops.set_fan_level)
		sysfs_remove_file(kobj, hwmon_pwm1);

	if (loongson_fan1_ops.get_fan_speed)
		sysfs_remove_file(kobj, hwmon_fan1_input);
}

/* ========================= Fan 2 ====================== */

static ssize_t get_fan2_level(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan2_level(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan2_mode(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan2_mode(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan2_speed(struct device *dev,
			struct device_attribute *attr, char *buf);

static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR | S_IRUGO,
				get_fan2_level, set_fan2_level, 0);
static SENSOR_DEVICE_ATTR(pwm2_enable, S_IWUSR | S_IRUGO,
				get_fan2_mode, set_fan2_mode, 0);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, get_fan2_speed, NULL, 0);

static const struct attribute *hwmon_pwm2 =
			&sensor_dev_attr_pwm2.dev_attr.attr;
static const struct attribute *hwmon_pwm2_enable =
			&sensor_dev_attr_pwm2_enable.dev_attr.attr;
static const struct attribute *hwmon_fan2_input =
			&sensor_dev_attr_fan2_input.dev_attr.attr;

static ssize_t get_fan2_level(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u8 val;

	val = (loongson_fan2_ops.get_fan_level)();
	return sprintf(buf, "%d\n", val);
}

static ssize_t set_fan2_level(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_level;

	new_level = SENSORS_LIMIT(simple_strtoul(buf, NULL, 10), 0, 255);
	(loongson_fan2_ops.set_fan_level)(new_level);

	return count;
}

static ssize_t get_fan2_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	enum fan_control_mode fan2_mode;

	fan2_mode = (loongson_fan2_ops.get_fan_mode)();
	return sprintf(buf, "%d\n", fan2_mode);
}

static ssize_t set_fan2_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_mode;

	new_mode = SENSORS_LIMIT(simple_strtoul(buf, NULL, 10),
					FAN_FULL_MODE, FAN_AUTO_MODE);
	(loongson_fan2_ops.set_fan_mode)(new_mode);

	return count;
}

static ssize_t get_fan2_speed(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 val;

	val = (loongson_fan2_ops.get_fan_speed)();
	return sprintf(buf, "%d\n", val);
}

/* should be call by create_sysfs_fan_files */
static int create_sysfs_fan2_files(struct kobject *kobj)
{
	int ret;

	ret = sysfs_create_file(kobj, hwmon_pwm2_enable);
	if (ret) {
		printk(KERN_ERR "fail to create sysfs hwmon_pwm2_enable\n");
		goto fail_to_create_hwmon_pwm2_enable;
	}

	if (loongson_fan2_ops.set_fan_level) {
		ret = sysfs_create_file(kobj, hwmon_pwm2);
		if (ret) {
			printk(KERN_ERR "fail to create sysfs hwmon_pwm2\n");
			goto fail_to_create_hwmon_pwm2;
		}
	}

	if (loongson_fan2_ops.get_fan_speed) {
		ret = sysfs_create_file(kobj, hwmon_fan2_input);
		if (ret) {
			printk(KERN_ERR "fail to create sysfs hwmon_fan2_input\n");
			goto fail_to_create_hwmon_fan2_input;
		}
	}

	/* ready to support more interface later... */

	return 0;

fail_to_create_hwmon_fan2_input:
	sysfs_remove_file(kobj, hwmon_pwm2);

fail_to_create_hwmon_pwm2:
	sysfs_remove_file(kobj, hwmon_pwm2_enable);

fail_to_create_hwmon_pwm2_enable:
	return -1;
}

static void remove_sysfs_fan2_files(struct kobject *kobj)
{
	sysfs_remove_file(kobj, hwmon_pwm2_enable);

	if (loongson_fan2_ops.set_fan_level)
		sysfs_remove_file(kobj, hwmon_pwm2);

	if (loongson_fan2_ops.get_fan_speed)
		sysfs_remove_file(kobj, hwmon_fan2_input);
}

/* ========================= Fan 3 ====================== */

static ssize_t get_fan3_level(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan3_level(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan3_mode(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t set_fan3_mode(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count);
static ssize_t get_fan3_speed(struct device *dev,
			struct device_attribute *attr, char *buf);

static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR | S_IRUGO,
				get_fan3_level, set_fan3_level, 0);
static SENSOR_DEVICE_ATTR(pwm3_enable, S_IWUSR | S_IRUGO,
				get_fan3_mode, set_fan3_mode, 0);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, get_fan3_speed, NULL, 0);

static const struct attribute *hwmon_pwm3 =
			&sensor_dev_attr_pwm3.dev_attr.attr;
static const struct attribute *hwmon_pwm3_enable =
			&sensor_dev_attr_pwm3_enable.dev_attr.attr;
static const struct attribute *hwmon_fan3_input =
			&sensor_dev_attr_fan3_input.dev_attr.attr;

static ssize_t get_fan3_level(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u8 val;

	val = (loongson_fan3_ops.get_fan_level)();
	return sprintf(buf, "%d\n", val);
}

static ssize_t set_fan3_level(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_level;

	new_level = SENSORS_LIMIT(simple_strtoul(buf, NULL, 10), 0, 255);
	(loongson_fan3_ops.set_fan_level)(new_level);

	return count;
}

static ssize_t get_fan3_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	enum fan_control_mode fan3_mode;

	fan3_mode = (loongson_fan3_ops.get_fan_mode)();
	return sprintf(buf, "%d\n", fan3_mode);
}

static ssize_t set_fan3_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u8 new_mode;

	new_mode = SENSORS_LIMIT(simple_strtoul(buf, NULL, 10),
					FAN_FULL_MODE, FAN_AUTO_MODE);
	(loongson_fan3_ops.set_fan_mode)(new_mode);

	return count;
}

static ssize_t get_fan3_speed(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	u32 val;

	val = (loongson_fan3_ops.get_fan_speed)();
	return sprintf(buf, "%d\n", val);
}

/* should be call by create_sysfs_fan_files */
static int create_sysfs_fan3_files(struct kobject *kobj)
{
	int ret;

	ret = sysfs_create_file(kobj, hwmon_pwm3_enable);
	if (ret) {
		pr_err("fail to create sysfs hwmon_pwm3_enable\n");
		goto fail_to_create_hwmon_pwm3_enable;
	}

	if (loongson_fan3_ops.set_fan_level) {
		ret = sysfs_create_file(kobj, hwmon_pwm3);
		if (ret) {
			pr_err("fail to create sysfs hwmon_pwm3\n");
			goto fail_to_create_hwmon_pwm3;
		}
	}

	if (loongson_fan3_ops.get_fan_speed) {
		ret = sysfs_create_file(kobj, hwmon_fan3_input);
		if (ret) {
			pr_err("fail to create sysfs hwmon_fan3_input\n");
			goto fail_to_create_hwmon_fan3_input;
		}
	}

	/* ready to support more interface later... */

	return 0;

fail_to_create_hwmon_fan3_input:
	sysfs_remove_file(kobj, hwmon_pwm3);

fail_to_create_hwmon_pwm3:
	sysfs_remove_file(kobj, hwmon_pwm3_enable);

fail_to_create_hwmon_pwm3_enable:
	return -1;
}

static void remove_sysfs_fan3_files(struct kobject *kobj)
{
	sysfs_remove_file(kobj, hwmon_pwm3_enable);

	if (loongson_fan3_ops.set_fan_level)
		sysfs_remove_file(kobj, hwmon_pwm3);

	if (loongson_fan3_ops.get_fan_speed)
		sysfs_remove_file(kobj, hwmon_fan3_input);
}

/* ========================= Fan 3 End ====================== */

static int create_sysfs_fan_files(struct kobject *kobj)
{
	/* Mothed set_fan_mode must be implement if the fan under control */
	if (loongson_fan1_ops.set_fan_mode)
		create_sysfs_fan1_files(kobj);

	if (loongson_fan2_ops.set_fan_mode)
		create_sysfs_fan2_files(kobj);

	if (loongson_fan3_ops.set_fan_mode)
		create_sysfs_fan3_files(kobj);

	return 0;
}

static void remove_sysfs_fan_files(struct kobject *kobj)
{
	/* Mothed set_fan_mode must be implement if the fan under control */
	if (loongson_fan1_ops.set_fan_mode)
		remove_sysfs_fan1_files(kobj);

	if (loongson_fan2_ops.set_fan_mode)
		remove_sysfs_fan2_files(kobj);

	if (loongson_fan3_ops.set_fan_mode)
		remove_sysfs_fan3_files(kobj);
}

static int __init loongson_hwmon_init(void)
{
	int ret;

	printk(KERN_INFO "Loongson Hwmon Enter...\n");

	loongson_hwmon_dev = hwmon_device_register(NULL);
	if (IS_ERR(loongson_hwmon_dev)) {
		ret = -ENOMEM;
		printk(KERN_ERR "hwmon_device_register fail!\n");
		goto fail_hwmon_device_register;
	}

	ret = sysfs_create_group(&loongson_hwmon_dev->kobj,
				&loongson_hwmon_attribute_group);
	if (ret) {
		printk(KERN_ERR "fail to create loongson hwmon!\n");
		goto fail_sysfs_create_group_hwmon;
	}

	ret = create_sysfs_temp_files(&loongson_hwmon_dev->kobj);
	if (ret) {
		printk(KERN_ERR "fail to create temprature interface!\n");
		goto fail_create_sysfs_temp_files;
	}

	ret = create_sysfs_fan_files(&loongson_hwmon_dev->kobj);
	if (ret) {
		printk(KERN_ERR "fail to create fan interface!\n");
		goto fail_create_sysfs_fan_files;
	}

	return ret;

fail_create_sysfs_fan_files:
	remove_sysfs_temp_files(&loongson_hwmon_dev->kobj);

fail_create_sysfs_temp_files:
	sysfs_remove_group(&loongson_hwmon_dev->kobj,
				&loongson_hwmon_attribute_group);

fail_sysfs_create_group_hwmon:
	hwmon_device_unregister(loongson_hwmon_dev);

fail_hwmon_device_register:
	return ret;
}

static void __exit loongson_hwmon_exit(void)
{
	remove_sysfs_fan_files(&loongson_hwmon_dev->kobj);
	remove_sysfs_temp_files(&loongson_hwmon_dev->kobj);
	sysfs_remove_group(&loongson_hwmon_dev->kobj,
				&loongson_hwmon_attribute_group);
	hwmon_device_unregister(loongson_hwmon_dev);
}

late_initcall(loongson_hwmon_init);
module_exit(loongson_hwmon_exit);

MODULE_AUTHOR("Xiang Yu <xiangy@lemote.com>");
MODULE_DESCRIPTION("Loongson Hwmon driver");
