#ifndef _LOONGSON_SMC_H_
#define _LOONGSON_SMC_H_

union smc_message {
	u32 value;
	struct {
		u32 arg : 24;
		u8  cmd : 7; /* Return 0x7f if command failed */
		u8  complete : 1;
	};
};

/* Commands in SMC mailbox registers */

#define CMD_GET_VERSION 0x1
/* Interface Version, input none, return version */

/* Features */
#define CMD_GET_FEATURES 0x2
/* Get features that SMC implemented, input index, output feature flags */
#define CMD_GET_ENABLED_FEATURES 0x3
/* Get currently enabled features, input index, output feature flags */
#define CMD_SET_ENABLED_FEATURES 0x4
/* Set features enabled state, input index and flags, output sucessfully enabled flags */
struct feature_args {
	u16 flags : 16;
	u8  index : 8;
};

#define FEATURE_INDEX_GENERAL 0x0
#define FEATURE_INDEX_ADVANCED 0x1

/* General Feature Flags */
#define FEATURE_FREQ_SCALE (1 << 0)
#define FEATURE_VOLTAGE_SCALE (1 << 1)
#define FEATURE_BOOST (1 << 2) /* Enable Boost means set PLL from normal to higher */
#define FEATURE_SENSORS (1 << 3) /* Temperature/Voltage sensors */
#define FEATURE_FAN_CONTROL (1 << 4)

/* Advanced Features Flags */
#define FEATURE_ADJUST_DVFS_PARAM (1 << 0) /* Allow OS adjust FreqScale/VID of each level. */

/* Freqscale Related */
#define CMD_GET_CPU_FREQUENCY 0x5
/* Input CPUNum, output frequency, in MHz */
#define CMD_GET_FREQ_LEVELS 0x6
/* Input none, output freq levels */
struct freq_level_args {
	u8 min_level : 8;
	u8 max_normal_level : 8;
	u8 max_boost_level : 8;
};

#define CMD_GET_FREQ_INFO 0x7
/* Input index and level, output info */
#define CMD_SET_FREQ_INFO 0x8
/* Input index and info, output none, available only with FEATURE_ADJUST_DVFS_PARAM */
#define FREQ_INFO_INDEX_FREQ 0x0 /* Freq in MHz */
struct freq_info_args {
	u16 info : 16;
	u8  index : 8;
};

#define CMD_SET_CPU_LEVEL 0x9
/* Input cpu mask and level, output none */
/*
 * Note: This command return as completed only means
 * SMC already knows the request, does not means the
 * CPU freqency have changed. SMC should ensure constant
 * counter frequency unchanged.
 */
struct freq_level_setting_args {
	u16 cpumask : 16;
	u8  level : 8;
};

/* TEMP Sensors */
#define CMD_GET_SENSOR_NUM 0x10
/* Input none, output Number of sensors in u4 */

#define CMD_GET_SENSOR_STATUS 0x11
/* Input sensor_id and info_type, output info */
#define SENSOR_INFO_TYPE_TEMP 0x0
#define SENSOR_INFO_TYPE_VOLTAGE 0x1
#define SENSOR_INFO_TYPE_FLAGS 0xf
#define SENSOR_FLAG_TEMP (1 << 0)
#define SENSOR_FLAG_VOLTAGE (1 << 1)
struct sensor_info_args {
	union {
		u16 val;
		s16 temp; /* Signed 16bit, in Celsius */
	};
	u8 info_type : 4;
	u8 sensor_id : 4;
};

/* Fan Control */
#define CMD_GET_FAN_NUM 0x12
/* Input none, output Number of fans in u4 */

#define CMD_GET_FAN_INFO 0x13
/* Input sensor_id and info_type, output info */
#define CMD_SET_FAN_INFO 0x14
/* Input sensor_id and info_type info, output none */
#define FAN_INFO_RPM 0x0 /* Return RPM, can not set */
#define FAN_INFO_LEVEL 0x1 /* PWM Level, 0~255, only set with manual mode */
#define FAN_INFO_FLAGS 0xf /* Determine Mode */
#define FAN_FLAG_AUTO (1 << 0)
#define FAN_FLAG_MANUAL (1 << 1)

struct fan_info_args {
	u16 val;
	u8  info_type : 4;
	u8  fan_id : 4;
};

static inline int do_service_request(u8 cmd, void *arg)
{
	int retries;
	union smc_message msg;

	msg.value = csr_readl(LOONGSON_CSR_SMCMBX);
	if (!msg.complete)
		return -1;

	msg.cmd = cmd;
	msg.arg = *(u32 *)arg;
	msg.complete = 0x0;

	csr_writel(msg.value, LOONGSON_CSR_SMCMBX);
	csr_writel(csr_readl(LOONGSON_CSR_SMCINT) | 1<<10, LOONGSON_CSR_SMCINT);

	for (retries = 0; retries < 5000; retries++) {
		msg.value = csr_readl(LOONGSON_CSR_SMCMBX);
		if (msg.complete)
			break;

		usleep_range(1, 4);
	}

	if (!msg.complete || msg.cmd == 0x7f)
		return -1;

	*(u32 *)arg = msg.arg;

	return 0;
}

#endif
