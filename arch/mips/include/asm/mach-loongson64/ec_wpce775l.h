/*
 * EC (Embedded Controller) WPCE775L device driver header for Linux
 *
 * Copyright (C) 2011 Lemote Inc.
 * Author : Wang Rui <wangr@lemote.com>
 * Author : Huang Wei <huangw@lemote.com>
 * Date   : 2011-02-21
 *
 * EC relative header file. All the EC registers should be defined here.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at you option) and later version.
 */

#ifndef __EC_WPCE775L_H__
#define __EC_WPCE775L_H__

#define EC_VERSION		"1.11"

/*
 * The following registers are determined by the EC index configureation.
 * 1. fill the PORT_INDEX as EC register.
 * 2. fill the PORT_DATA as EC register write data or get the data from it.
 */
/* base address for io access for Loongson3A+rs780e Notebook platform */
#define EC_BASE_ADDR_PORT	(0xb8000000)
#define SIO_INDEX_PORT		0x2E
#define SIO_DATA_PORT		0x2F

/*
 * EC delay time 500us for register and status access
 * Unit : us
 */
#define EC_REG_DELAY		30000
#define EC_CMD_TIMEOUT		0x1000
#define EC_SEND_TIMEOUT		0xffff
#define EC_RECV_TIMEOUT		0xffff

/*
 * EC access port for with Host communication.
 */
#define EC_CMD_PORT		0x66
#define EC_STS_PORT		0x66
#define EC_DAT_PORT		0x62

/*
 * ACPI legacy commands.
 */
#define CMD_READ_EC		0x80	/* Read EC command. */
#define CMD_WRITE_EC		0x81	/* Write EC command. */
#define CMD_GET_EVENT_NUM	0x84	/* Query EC command, for get SCI event number. */

/*
 * ACPI OEM commands.
 */
#define CMD_RESET		0x4E	/* Reset and poweroff the machine auto-clear: rd/wr */
enum
{
	RESET_OFF = 0,
	RESET_ON,
	PWROFF_ON,
	SUSPEND_ON,
	STANDBY_ON
};

#define CMD_EC_VERSION		0x4F	/* EC Version OEM command: 36 Bytes */

/*
 * Used ACPI legacy command 80h to do active.
 */
/* >>> Read/Write temperature & fan index for ACPI 80h/81h command. */
#define INDEX_TEMPERATURE_VALUE		0x1B	/* Current CPU temperature value, Read and Write(81h command). */
#define INDEX_FAN_MAXSPEED_LEVEL	0x5B	/* Fan speed maxinum levels supported. Defaut is 6. */
#define INDEX_FAN_SPEED_LEVEL		0x5C	/* FAn speed level. [0,5] or [0x06, 0x38]*/
#define INDEX_FAN_CTRLMOD		0x5D	/* Fan control mode, 0 = by EC, 1 = by Host.*/
enum
{
	FAN_CTRL_BYEC = 0,
	FAN_CTRL_BYHOST
};
#define INDEX_FAN_STSCTRL		0x5E	/* Fan status/control, 0 = stop, 1 = run. */
enum
{
	FAN_STSCTRL_OFF = 0,
	FAN_STSCTRL_ON
};
#define INDEX_FAN_ERRSTS		0x5F	/* Fan error status, 0 = no error, 1 = has error. */
enum
{
	FAN_ERRSTS_NO = 0,
	FAN_ERRSTS_HAS
};
#define INDEX_FAN_SPEED_LOW		0x08	/* Fan speed low byte.*/
#define INDEX_FAN_SPEED_HIGH		0x09	/* Fan speed high byte. */
/* <<< End Temp & Fan */

/* >>> Read/Write LCD backlight information/control index for ACPI 80h/81h command. */
#define INDEX_BACKLIGHT_CTRLMODE	0x57	/* LCD backlight control mode: 0 = by EC, 1 = by HOST */
enum
{
	BACKLIGHT_CTRL_BYEC = 0,
	BACKLIGHT_CTRL_BYHOST
};
#define INDEX_BACKLIGHT_STSCTRL		0x58	/* LCD backlight status or control: 0 = turn off, 1 = turn on */
enum
{
	BACKLIGHT_OFF = 0,
	BACKLIGHT_ON
};
#define	INDEX_DISPLAY_MAXBRIGHTNESS_LEVEL	0x59	/* LCD backlight brightness max level */
#define	INDEX_DISPLAY_BRIGHTNESS	0x5A	/* 10 stages (0~9) LCD backlight brightness adjust */
enum
{
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_0	= 0,	/* This level is backlight turn off. */
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_1,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_2,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_3,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_4,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_5,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_6,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_7,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_8,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_9,
	FLAG_DISPLAY_BRIGHTNESS_LEVEL_10
};
/* <<< End Backlight */

#define INDEX_TOUCHPAD_ENABLE_LED	0x56
enum
{
	TP_EN_LED_OFF,
	TP_EN_LED_ON
};

/* >>> Read battery(BQ3060) index for ACPI 80h command */
/*
 * The reported battery die temperature.
 * The temperature is expressed in units of 0.25 seconds and is updated every 2.56 seconds.
 * The equation to calculate reported pack temperature is:
 * Temperature = 0.1 * (256 * TEMPH + TEMPL) Kelvin
 * Temperature -= 273 Degrees Celsius
 * The host sytem has read-only access to this register pair.
 */
#define INDEX_BATTERY_TEMP_LOW		0x20	/* Battery temperature low byte. */
#define INDEX_BATTERY_TEMP_HIGH		0x21	/* Battery temperature high byte. */
#define INDEX_BATTERY_VOL_LOW		0x22	/* Battery Voltage Low byte. */
#define INDEX_BATTERY_VOL_HIGH		0x23	/* Battery Voltage High byte. */
#define INDEX_BATTERY_CURRENT_LOW	0x24	/* Battery Current Low byte. */
#define INDEX_BATTERY_CURRENT_HIGH	0x25	/* Battery Current High byte. */
#define INDEX_BATTERY_AC_LOW		0x26	/* Battery AverageCurrent Low byte. */
#define INDEX_BATTERY_AC_HIGH		0x27	/* Battery AverageCurrent High byte. */
#define INDEX_BATTERY_CAPACITY		0x2A	/* Battery RemainingCapacity percent. */
#define INDEX_BATTERY_STATUS_LOW	0x2C	/* Battery Status low byte. */
enum
{
	BIT_BATTERY_STATUS_FD = 4,	/* Battery Fully Discharged Notify. 1 = Fully Discharged */
	BIT_BATTERY_STATUS_FC,		/* Battery Fully Charged Notify. 1 = Fully Charged. */
	BIT_BATTERY_STATUS_DSG,		/* Battery Discharging mode. 0 = in charging mode, 1 = in discharging mode,
	                               relaxation mode, or valid charge termination has occurred. */
	BIT_BATTERY_STATUS_INIT		/* Battery Initialization. 1 = Initialization */
};
#define INDEX_BATTERY_STATUS_HIGH	0x2D	/* Battery Status high byte. */
enum
{
	BIT_BATTERT_STATUS_RTA = 0,	/* Battery Remaining Time Alarm. <= 10min */
	BIT_BATTERY_STATUS_RCA,		/* Battery Remaining Capacity Alarm. <= 430mAh */
	BIT_BATTERY_STATUS_TDA = 3,	/* Battery Terminate Discharge Alarm. */
	BIT_BATTERY_STATUS_OTA,		/* Battery Over Temperature Alarm. */
	BIT_BATTERY_STATUS_TCA = 6,	/* Battery Terminate Charge Alarm. */
	BIT_BATTERY_STATUS_OCA		/* Battery Over Charged Alarm. */
};
#define INDEX_BATTERY_RC_LOW		0x2E	/* Battery RemainingCapacity Low byte. */
#define INDEX_BATTERY_RC_HIGH		0x2F	/* Battery RemainingCapacity High byte. */
#define INDEX_BATTERY_ATTE_LOW		0x30	/* Battery AverageTimeToEmpty Low byte. */
#define INDEX_BATTERY_ATTE_HIGH		0x31	/* Battery AverageTimeToEmpty High byte. */
#define INDEX_BATTERY_ATTF_LOW		0x32	/* Battery AverageTimeToFull Low byte. */
#define INDEX_BATTERY_ATTF_HIGH		0x33	/* Battery AverageTimeToFull High byte. */
#define INDEX_BATTERY_FCC_LOW		0x34	/* Battery FullChargeCapacity Low byte. */
#define INDEX_BATTERY_FCC_HIGH		0x35	/* Battery FullChargeCapacity High byte. */
#define INDEX_BATTERY_CC_LOW		0x36	/* Battery ChargingCurrent Low byte. */
#define INDEX_BATTERY_CC_HIGH		0x37	/* Battery ChargingCurrent High byte. */
#define INDEX_BATTERY_CV_LOW		0x38	/* Battery ChargingVoltage Low byte. */
#define INDEX_BATTERY_CV_HIGH		0x39	/* Battery ChargingVoltage High byte. */
#define INDEX_BATTERY_CHGSTS_LOW	0x3A	/* Battery ChargingStatus Low byte. */
enum
{
	BIT_BATTERY_CHGSTS_XCHGLV = 0,	/* 1 = Battery is depleted */
	BIT_BATTERY_CHGSTS_OC,		/* 1 = Overcharge fault */
	BIT_BATTERY_CHGSTS_OCHGI,	/* 1 = Overcharge current fault */
	BIT_BATTERY_CHGSTS_OCHGV,	/* 1 = Overcharge voltage fault */
	BIT_BATTERY_CHGSTS_FCMTO,	/* 1 = Fast-charge timeout fault */
	BIT_BATTERY_CHGSTS_PCMTO,	/* 1 = Precharge timeout fault */
	BIT_BATTERY_CHGSTS_CB		/* 1 = Cell balancing in progress */
};
#define INDEX_BATTERY_CHGSTS_HIGH	0x3B	/* Battery ChargingStatus High byte. */
enum
{
	BIT_BATTERY_CHGSTS_HTCHG = 0,	/* 1 = Low temperature charging */
	BIT_BATTERY_CHGSTS_ST2CHG,	/* 1 = Standard temperature charging 2 */
	BIT_BATTERY_CHGSTS_ST1CHG,	/* 1 = Standard temperature charging 1 */
	BIT_BATTERY_CHGSTS_LTCHG,	/* 1 = Low temperature charging */
	BIT_BATTERY_CHGSTS_PCHG = 5,	/* 1 = Precharging conditions exist */
	BIT_BATTERY_CHGSTS_CHGSUSP,	/* 1 = Charging suspended */
	BIT_BATTERY_CHGSTS_XCHG		/* 1 = Charging disabled */
};
#define INDEX_BATTERY_CYCLECNT_LOW	0x3C	/* Battery CycleCount Low byte. */
#define INDEX_BATTERY_CYCLECNT_HIGH	0x3D	/* Battery CycleCount High byte. */

/* Battery static information. */
#define INDEX_BATTERY_DC_LOW		0x60	/* Battery DesignCapacity Low byte. */
#define INDEX_BATTERY_DC_HIGH		0x61	/* Battery DesignCapacity High byte. */
#define INDEX_BATTERY_DV_LOW		0x62	/* Battery DesignVoltage Low byte. */
#define INDEX_BATTERY_DV_HIGH		0x63	/* Battery DesignVoltage High byte. */
#define INDEX_BATTERY_MFD_LOW		0x64	/* Battery ManufactureDate Low byte. */
#define INDEX_BATTERY_MFD_HIGH		0x65	/* Battery ManufactureDate High byte. */
#define INDEX_BATTERY_SN_LOW		0x66	/* Battery SerialNumber Low byte. */
#define INDEX_BATTERY_SN_HIGH		0x67	/* Battery SerialNumber High byte. */
#define INDEX_BATTERY_MFN_LENG		0x68	/* Battery ManufacturerName string length. */
#define INDEX_BATTERY_MFN_START		0x69	/* Battery ManufacturerName string start byte. */

#define INDEX_BATTERY_DEVNAME_LENG	0x74	/* Battery DeviceName string length. */
#define INDEX_BATTERY_DEVNAME_START	0x75	/* Battery DeviceName string start byte. */
#define INDEX_BATTERY_DEVCHEM_LENG	0x7C	/* Battery DeviceChemitry string length. */
#define INDEX_BATTERY_DEVCHEM_START	0x7D	/* Battery DeviceChemitry string start byte. */
#define INDEX_BATTERY_MFINFO_LENG	0x81	/* Battery ManufacturerInfo string length. */
#define INDEX_BATTERY_MFINFO_START	0x82	/* Battery ManufacturerInfo string start byte. */
#define INDEX_BATTERY_CELLCNT_START	0x95    /* Battery packaging fashion string start byte(=4). Unit: ASCII. */
#define BATTERY_CELLCNT_LENG	  	4		/* Battery packaging fashion string size. */
#define FLAG_BAT_CELL_3S1P 		"3S1P"

#define BIT_BATTERY_CURRENT_PN		7       /* Battery current sign is positive or negative */
#define BIT_BATTERY_CURRENT_PIN		0x07	/* Battery current sign is positive or negative.*/
/* <<< End Battery */

#define MASK(x)	(1 << x)

#define INDEX_STOPCHG_STATUS	0xA1	/* Read currently stop charge status. */
enum
{
	BIT_STOPCHG_FULLYCHG = 0,
	BIT_STOPCHG_TIMEOUT,
	BIT_STOPCHG_OVERTEMP,
	BIT_STOPCHG_OVERVOLT,
	BIT_STOPCHG_OVERCURRENT,
	BIT_STOPCHG_TERMINATE
};
#define INDEX_POWER_STATUS		0xA2	/* Read current power status. */
enum
{
	BIT_POWER_PWRON = 0,	/* Power-on start status, 1 = on power-on, 0 = power-on complete. */
	BIT_POWER_BATVL,	/* Battery in very low status. */
	BIT_POWER_BATL,		/* Battery in low status. */
	BIT_POWER_BATFCHG,	/* Battery in fully charging status. */
	BIT_POWER_BATCHG,	/* Battery in charging status. */
	BIT_POWER_TERMINATE,	/* Battery in terminate charging status. */
	BIT_POWER_BATPRES,	/* Battery present. */
	BIT_POWER_ACPRES	/* AC present. */
};

#define INDEX_DEVICE_STATUS		0xA3	/* Read Current Device Status */
enum
{
	BIT_DEVICE_TP = 0,	/* TouchPad status: 0 = close, 1 = open */
	BIT_DEVICE_WLAN,	/* WLAN status: 0 = close, 1 = open */
	BIT_DEVICE_3G,		/* 3G status: 0 = close, 1 = open */
	BIT_DEVICE_CAM,		/* Camera status: 0 = close, 1 = open */
	BIT_DEVICE_MUTE,	/* Mute status: 0 = close, 1 = open */
	BIT_DEVICE_LID,		/* LID status: 0 = close, 1 = open */
	BIT_DEVICE_BKLIGHT,	/* BackLight status: 0 = close, 1 = open */
	BIT_DEVICE_SIM		/* SIM Card status: 0 = pull out, 1 = insert */
};

#define INDEX_SHUTDOWN_ID		0xA4	/* Read Shutdown ID */
enum
{
	BIT_SHUTDNID_S45 = 0,	/* in S4 or S5 */
	BIT_SHUTDNID_BATDEAD,	/* Battery Dead */
	BIT_SHUTDNID_OVERHEAT,	/* Over Heat */
	BIT_SHUTDNID_SYSCMD,	/* System command */
	BIT_SHUTDNID_LPRESSPWN,	/* Long press power button */
	BIT_SHUTDNID_PWRUNDER9V,/* Batery voltage low under 9V */
	BIT_SHUTDNID_S3,	/* Entry S3 state */
	BIT_SHUTDNID_S1		/* Entry S1 state */
};

#define INDEX_SYSTEM_CFG		0xA5		/* Read System config */
#define BIT_SYSCFG_TPSWITCH		(1 << 0)	/* TouchPad switch */
#define BIT_SYSCFG_WLANPRES		(1 << 1)	/* WLAN present */
#define BIT_SYSCFG_NB3GPRES		(1 << 2)	/* 3G present */
#define BIT_SYSCFG_CAMERAPRES		(1 << 3)	/* Camera Present */
#define BIT_SYSCFG_VOLCTRLEC		(1 << 4)	/* Volume control by EC */
#define BIT_SYSCFG_BLCTRLEC		(1 << 5)	/* Backlight control by EC */
#define BIT_SYSCFG_AUTOBRIGHT		(1 << 7)	/* Auto brightness */

#define INDEX_VOLUME_LEVEL		0xA6		/* Read Volume Level command */
#define INDEX_VOLUME_MAXLEVEL		0xA7		/* Volume MaxLevel */
#define VOLUME_MAX_LEVEL		0x0A		/* Volume level max is 11 */
enum
{
	FLAG_VOLUME_LEVEL_0 = 0,
	FLAG_VOLUME_LEVEL_1,
	FLAG_VOLUME_LEVEL_2,
	FLAG_VOLUME_LEVEL_3,
	FLAG_VOLUME_LEVEL_4,
	FLAG_VOLUME_LEVEL_5,
	FLAG_VOLUME_LEVEL_6,
	FLAG_VOLUME_LEVEL_7,
	FLAG_VOLUME_LEVEL_8,
	FLAG_VOLUME_LEVEL_9,
	FLAG_VOLUME_LEVEL_10
};

/* Camera control */
#define INDEX_CAM_STSCTRL			0xAA
enum
{
	CAM_STSCTRL_OFF = 0,
	CAM_STSCTRL_ON
};

/* data destroy led control */
#define INDEX_DATA_DESTROY			0xB0
enum
{
	DATA_DESTROY_OFF = 0,
	DATA_DESTROY_ON
};

/* The led of board healthy */
#define INDEX_BOARD_HEALTHY			0xB1

/* EC_SC input */
/* EC Status query, by direct read 66h port. */
#define EC_SMI_EVT		(1 << 6)	/* 1 = SMI event padding */
#define EC_SCI_EVT		(1 << 5)	/* 1 = SCI event padding */
#define EC_BURST		(1 << 4)	/* 1 = Controller is in burst mode */
#define EC_CMD			(1 << 3)	/* 1 = Byte in data register is command */

#define EC_IBF			(1 << 1)	/* 1 = Input buffer full (data ready for ec) */
#define EC_OBF			(1 << 0)	/* 1 = Output buffer full (data ready for host) */

/* SCI Event Number from EC */
enum
{
	SCI_EVENT_NUM_WLAN = 0x21,		/* 0x21, Fn+F1, Wlan is on or off */
	SCI_EVENT_NUM_3G,			/* 0x22, Fn+F9 for 3G switch */
	SCI_EVENT_NUM_LID,			/* 0x23, press the lid or not */
	SCI_EVENT_NUM_DISPLAY_TOGGLE,		/* 0x24, Fn+F8 for display switch */
	SCI_EVENT_NUM_SLEEP,			/* 0x25, Fn+ESC for entering sleep mode */
	SCI_EVENT_NUM_BRIGHTNESS_UP,		/* 0x26, Fn+F3, LCD backlight brightness up adjust */
	SCI_EVENT_NUM_BRIGHTNESS_DN,		/* 0x27, Fn+F2, LCD backlight brightness down adjust */
	SCI_EVENT_NUM_CAMERA,			/* 0x28, Fn+F10, Camera is on or off */
	SCI_EVENT_NUM_TP,			/* 0x29, Fn+F11, TouchPad is on */
	SCI_EVENT_NUM_AUDIO_MUTE,		/* 0x2A, Fn+F4, Mute is on or off */
	SCI_EVENT_NUM_BLACK_SCREEN,		/* 0x2B, Fn+F7, Black screen is on or off */
	SCI_EVENT_NUM_VOLUME_UP,		/* 0x2C, Fn+F6, Volume up adjust */
	SCI_EVENT_NUM_VOLUME_DN,		/* 0x2D, Fn+F5, Volume down adjust */
	SCI_EVENT_NUM_OVERTEMP,			/* 0x2E, Over-temperature happened */
	SCI_EVENT_NUM_SIM,			/* 0x2F, SIM Card Detect */
	SCI_EVENT_NUM_AC,			/* 0x30, AC in/out */
	SCI_EVENT_NUM_BAT,			/* 0x31, BAT in/out */
	SCI_EVENT_NUM_BATL,			/* 0x32, Battery Low capacity alarm, < 10% */
	SCI_EVENT_NUM_BATVL,			/* 0x33, Battery VeryLow capacity alarm, < 5% */
	SCI_EVENT_NUM_THROT,			/* 0x34, CPU Throttling event alarm, CPU Temperature > 85 or < 80. */
	SCI_EVENT_NUM_POWER = 0x37,		/* 0x37, Power button */
	SCI_EVENT_RESOLUTION_SETTING,		/* 0x38, Resolution Setting */
	SCI_EVENT_MEDIA_RUN_PAUSE,		/* 0x39, Media Play or Pause */
	SCI_EVENT_MEDIA_STOP,			/* 0x3A, Media Stop */
	SCI_EVENT_MEDIA_LAST,			/* 0x3B, Media Play last one */
	SCI_EVENT_MEDIA_NEXT,			/* 0x3C, Media Play next one */
	SCI_EVENT_RECOVERY = 0x3D		/* 0x3D, Recovery Event */
};

#define SCI_EVENT_NUM_START		SCI_EVENT_NUM_WLAN
#define SCI_EVENT_NUM_END		SCI_EVENT_RECOVERY

extern unsigned char app_access_ec_flag;

typedef int (*sci_handler)(int status);

/* The general ec index-io port read action */
extern unsigned char ec_read(unsigned char index);
extern unsigned char ec_read_all(unsigned char command, unsigned char index);
extern unsigned char ec_read_noindex(unsigned char command);

/* The general ec index-io port write action */
extern int ec_write(unsigned char index, unsigned char data);
extern int ec_write_all(unsigned char command, unsigned char index, unsigned char data);
extern int ec_write_noindex(unsigned char command, unsigned char data);

/* Query sequence of 62/66 port access routine. */
extern int ec_query_seq(unsigned char command);
extern int ec_get_event_num(void);

extern void clean_ec_event_status(void);

#endif /* __EC_WPCE775L_H__ */
