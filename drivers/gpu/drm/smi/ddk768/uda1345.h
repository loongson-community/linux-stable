#ifndef _UDA1345_CODEC_H
#define _UDA1345_CODEC_H
#include "l3.h"


#define UDA1345_L3ADDR	5
#define UDA1345_DATA_ADDR	((UDA1345_L3ADDR << 2) | 0)
#define UDA1345_STATUS_ADDR	((UDA1345_L3ADDR << 2) | 2)


/* UDA1345 registers */
#define UDA1345_STATUS 0
#define UDA1345_DATA_VOLUME 1
#define UDA1345_DATA_DE_MUTE 2
#define UDA1345_DATA_POWER 3

#define UDA1345_REGS_NUM 4

/*UDA1345 System clock frequency*/
#define UDA1345_SYSCLKF_MASK (3 << 4)
#define UDA1345_SYSCLKF_512FS  (0 << 4)
#define UDA1345_SYSCLKF_384FS  (1 << 4)
#define UDA1345_SYSCLKF_256FS  (2 << 4)
enum uda1345_sysclkf
{
    SYSCLKF_512FS = 0,
    SYSCLKF_384FS,
    SYSCLKF_256FS,
};

/*UDA1345 data input format*/
#define UDA1345_FORMAT_MASK (7 << 1)
#define  UDA1345_I2S_BUS (0 << 1)
#define  UDA1345_LSB_16BITS (1 << 1)
#define  UDA1345_LSB_18BITS (2 << 1)
#define  UDA1345_LSB_20BITS (3 << 1)
#define  UDA1345_MSB_JUSTIFIED (4 << 1)
#define  UDA1345_MSB_OUTPUT_LSB_16BITS_INPUT (5 << 1)
#define  UDA1345_MSB_OUTPUT_LSB_18BITS_INPUT (6 << 1)
#define  UDA1345_MSB_OUTPUT_LSB_20BITS_INPUT (7 << 1)
enum uda1345_input_format
{
    I2S_BUS = 0,
    LSB_16BITS,
    LSB_18BITS,
    LSB_20BITS,
    MSB_JUSTIFIED,
    MSB_OUTPUT_LSB_16BITS_INPUT,
    MSB_OUTPUT_LSB_18BITS_INPUT,
    MSB_OUTPUT_LSB_20BITS_INPUT,
};

/*UDA1345 dc filter*/
#define UDA1345_DC_MASK 1
#define UDA1345_DC_FILTERING 1
#define UDA1345_NO_DC_FILTERING 0
enum uda1345_dc_filter
{
    NO_DC_FILTERING = 0,
    DC_FILTERING,
};

/*UDA1345 volume*/
#define UDA1345_VOLUME_MASK 0x3f
#define UDA1345_VOLUME_N5DB 6
#define UDA1345_VOLUME_N10DB 0xB

/*UDA1345 DE-EMPHASIS*/
#define UDA1345_DE_EMPHASIS_MASK (3 << 3)
#define UDA1345_NO_DE_EMPHASIS (0 << 3)
#define UDA1345_DE_EMPHASIS_32KHZ (1 << 3)
#define UDA1345_DE_EMPHASIS_44KHZ (2 << 3)
#define UDA1345_DE_EMPHASIS_48KHZ (3 << 3)
enum uda1345_de_emphasis
{
    NO_DE_EMPHASIS = 0,
    DE_EMPHASIS_32KHZ,
    DE_EMPHASIS_44KHZ,
    DE_EMPHASIS_48KHZ,
};

/*UDA1345 mute*/
#define UDA1345_MUTE_MASK (1 << 2)
#define UDA1345_NO_MUTE (0 << 2)
#define UDA1345_MUTE (1 << 2)
enum uda1345_mute
{
    NO_MUTE = 0,
    MUTE,
};

/*UDA1345 power control*/
#define UDA1345_ADC_DAC_MASK 3
#define UDA1345_ADCOFF_DACOFF 0
#define UDA1345_ADCOFF_DACON 1
#define UDA1345_ADCON_DACOFF 2
#define UDA1345_ADCON_DACON 3
enum uda1345_power
{
    ADCOFF_DACOFF = 0,
    ADCOFF_DACON,
    ADCON_DACOFF,
    ADCON_DACON,
};


struct uda1345_data
{
    struct l3_pins l3;
    void * reg_cache_default;
    void * reg_cache;
    unsigned int cache_size;
    unsigned int cache_init : 1;

};





int uda1345_setsysclkfs(enum uda1345_sysclkf sysclk);
int uda1345_setformat(enum uda1345_input_format informat);
int uda1345_setdcfilter(enum uda1345_dc_filter onoff);
int uda1345_setvolume(u8 dB);
int uda1345_setdemphasis(enum uda1345_de_emphasis emphasis);
int uda1345_setmute(enum uda1345_mute onoff);
int uda1345_setpower(enum uda1345_power onoff);
int uda1345_init(void);
int uda1345_deinit(void);
#endif

