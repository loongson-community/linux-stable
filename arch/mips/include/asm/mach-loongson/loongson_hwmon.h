#ifndef __LOONGSON_HWMON_H_
#define __LOONGSON_HWMON_H_

#include <linux/types.h>

#define MIN_TEMP	0
#define MAX_TEMP	255
#define NOT_VALID_TEMP	999

typedef int (*get_temp_fun)(void);

struct loongson_temp_info {
	int (*get_cpu_temp)(void); /* get cpu temprature */
	int (*get_nb_temp)(void);  /* get North Bridge temprature */
	int (*get_sb_temp)(void);  /* get South Bridge temprature */
	int (*get_mb_temp)(void);  /* get main board temprature */
};

extern int loongson3_cpu_temp(void);
extern int emc1412_external_temp(void);
extern int emc1412_internal_temp(void);

extern struct loongson_temp_info loongson_temp_info;

/* 0:Max speed, 1:Manual, 2:Auto */
enum fan_control_mode {
	FAN_FULL_MODE = 0,
	FAN_MANUAL_MODE = 1,
	FAN_AUTO_MODE = 2,
	FAN_MODE_END
};

struct temp_range {
	u8 low;
	u8 high;
	u8 level;
};

/* loongson_fan_policy works when fan work at FAN_AUTO_MODE */
struct loongson_fan_policy {
	u8	type;
#define CONSTANT_SPEED_POLICY	0	/* at constent speed */
#define STEP_SPEED_POLICY	1	/* use up/down arrays to describe policy */
#define KERNEL_HELPER_POLICY	2	/* kernel as a helper to fan control */
#define KERNEL_NOTHING_POLICY	3	/* kernel do nothing to fan control */
#define OTHER_POLICY		4	/* others */

	/* percent only used when type is CONSTANT_SPEED_POLICY */
	u8	percent;

	/* period between two check. (Unit: S) */
	u8	adjust_period;

	/* fan adjust usually depend on a temprature input */
	get_temp_fun	depend_temp;

	/* up_step/down_step used when type is STEP_SPEED_POLICY */
#define MAX_STEP_NUM	16
	u8	up_step_num;
	u8	down_step_num;
	struct temp_range up_step[MAX_STEP_NUM];
	struct temp_range down_step[MAX_STEP_NUM];
};

struct loongson_fan_ops {
	/* get fan information*/
	u32 (*get_fan_speed)(void); /* Unit: RPM */
	u8  (*get_fan_level)(void); /* 0~255, 255 is max(100%) */
#define MAX_FAN_LEVEL	255
	u32 (*get_fan_min)(void);   /* Unit: RPM */
	u32 (*get_fan_max)(void);   /* Unit: RPM */

	enum fan_control_mode (*get_fan_mode)(void);
	int (*set_fan_mode)(enum fan_control_mode); /* 0:Max speed, 1:Manual, 2:Auto */
	int (*set_fan_level)(u8); /* 0~255, 255 is max(100%) */
	int (*set_fan_speed)(u32);/* Unit: RPM */

	/* default fan control policy is depend on product */
	struct loongson_fan_policy *fan_policy;
};

extern struct loongson_fan_ops loongson_fan1_ops, loongson_fan2_ops;

#endif /* __LOONGSON_HWMON_H_*/
