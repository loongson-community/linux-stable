#include <linux/string.h>
#include "ddk768_reg.h"
#include "ddk768_help.h"
#include "uda1345.h"

static void setdat(int v);
static void setclk(int v);
static void setmode(int v);

static const u8 uda1346_reg[UDA1345_REGS_NUM] =
{
    /* Status, data regs */
    0x00, 0x00, 0x80, 0xc1,
};

static u8 cache[sizeof(uda1346_reg)];

static struct uda1345_data falcon_uda1345 =
{
    .l3 = {
        .setdat = setdat,
        .setclk = setclk,
        .setmode = setmode,
        .data_hold = 1,
        .data_setup = 1,
        .clock_high = 1,
        .mode_hold = 1,
        .mode = 1,
        .mode_setup = 1,
    },
    .reg_cache_default = (void*)uda1346_reg,
    .cache_init = 0,
};

static void setdat(int v)
{
    unsigned long value;

    value = peekRegisterDWord(GPIO_DATA);
    pokeRegisterDWord(GPIO_DATA, 0 < v ? (value | (1 << GPIO_DATA_GPIO_CODEC_DATA_SHIFT)) : \
                      (value & ~(1 << GPIO_DATA_GPIO_CODEC_DATA_SHIFT)));

}

static void setclk(int v)
{
    unsigned long value;

		value = peekRegisterDWord(GPIO_DATA);
		pokeRegisterDWord(GPIO_DATA, 0 < v ? (value | (1 << GPIO_DATA_GPIO_CODEC_CLK_SHIFT)) : \
						  (value & ~(1 << GPIO_DATA_GPIO_CODEC_CLK_SHIFT)));


}

static void setmode(int v)
{
    unsigned long value;

		value = peekRegisterDWord(GPIO_DATA);
		pokeRegisterDWord(GPIO_DATA, 0 < v ? (value | (1 << GPIO_DATA_GPIO_CODEC_MODE_SHIFT)) : \
						  (value & ~(1 << GPIO_DATA_GPIO_CODEC_MODE_SHIFT)));

}

static void uda1345_GPIOInit(void)
{
    unsigned long value;
   		 /*l3 mode control pins*/

		value = peekRegisterDWord(GPIO_MUX);
		value &= ~(1 << GPIO_MUX_GPIO_CODEC_MODE_SHIFT);
		value &= ~(1 << GPIO_MUX_GPIO_CODEC_CLK_SHIFT);
		value &= ~(1 << GPIO_MUX_GPIO_CODEC_DATA_SHIFT);
		pokeRegisterDWord(GPIO_MUX, value);

		/*set 3 pins as input*/
		value = peekRegisterDWord(GPIO_DATA_DIRECTION);
		value |= (1 << GPIO_DATA_DIRECTION_GPIO_CODEC_MODE_SHIFT);
		value |= (1 << GPIO_DATA_DIRECTION_GPIO_CODEC_CLK_SHIFT);
		value |= (1 << GPIO_DATA_DIRECTION_GPIO_CODEC_DATA_SHIFT);
		pokeRegisterDWord(GPIO_DATA_DIRECTION, value);


}

static inline unsigned int uda1345_read_reg_cache(struct uda1345_data *codec,
        unsigned int reg)
{
    u8 *cache = (u8 *)codec->reg_cache;

    if (reg >= UDA1345_REGS_NUM)
        return -1;
	
    return cache[reg];
}

/*
 * Write the register cache
 */
static inline int uda1345_write_reg_cache(struct uda1345_data *codec,
        u8 reg, unsigned int value)
{
    u8 *cache = (u8 *)codec->reg_cache;

    if (reg >= UDA1345_REGS_NUM)
        return -1;
    cache[reg] = value;
    return 0;
}


/*
 * Write to the uda134x registers
 *
 */
static int uda1345_write(unsigned int reg,
                         unsigned int value)
{
    int ret;
    u8 addr;
    u8 data = value;

    if (reg >= UDA1345_REGS_NUM)
    {

        return -1;
    }

    uda1345_write_reg_cache(&falcon_uda1345, reg, value);

    switch (reg)
    {
    case UDA1345_STATUS:
        addr = UDA1345_STATUS_ADDR;
        break;
    case UDA1345_DATA_VOLUME:
    case UDA1345_DATA_DE_MUTE:
    case UDA1345_DATA_POWER:
        addr = UDA1345_DATA_ADDR;
        break;
    default:
        break;
    }

    ret = l3_write(&falcon_uda1345.l3,
                   addr, &data, 1);
   
   if (ret != 1)
        return -1;

    return 0;
}

int uda1345_setsysclkfs(enum uda1345_sysclkf sysclk)
{
    u8 data;
	
    data = uda1345_read_reg_cache(&falcon_uda1345,UDA1345_STATUS);
	
    data &= ~UDA1345_SYSCLKF_MASK;
    switch(sysclk)
    {
    case SYSCLKF_512FS:
        data |= UDA1345_SYSCLKF_512FS;
        break;
    case SYSCLKF_384FS:
        data |= UDA1345_SYSCLKF_384FS;
        break;
    case SYSCLKF_256FS:
        data |= UDA1345_SYSCLKF_256FS;
        break;
    default:
        return -1;
    }
	
    return uda1345_write(UDA1345_STATUS,data);

}

int uda1345_setformat(enum uda1345_input_format informat)
{

    u8 data;
    data = uda1345_read_reg_cache(&falcon_uda1345,UDA1345_STATUS);
    data &= ~UDA1345_FORMAT_MASK;
    switch(informat)
    {
    case I2S_BUS:
        data |= UDA1345_I2S_BUS;
        break;
    case LSB_16BITS:
        data |= UDA1345_LSB_16BITS;
        break;
    case LSB_18BITS:
        data |= UDA1345_LSB_18BITS;
        break;
    case	LSB_20BITS:
        data |= UDA1345_LSB_20BITS;
        break;
    case	MSB_JUSTIFIED:
        data |= UDA1345_MSB_JUSTIFIED;
        break;
    case	MSB_OUTPUT_LSB_16BITS_INPUT:
        data |= UDA1345_MSB_OUTPUT_LSB_16BITS_INPUT;
        break;
    case	MSB_OUTPUT_LSB_18BITS_INPUT:
        data |= UDA1345_MSB_OUTPUT_LSB_18BITS_INPUT;
        break;
    case	MSB_OUTPUT_LSB_20BITS_INPUT:
        data |= UDA1345_MSB_OUTPUT_LSB_20BITS_INPUT;
        break;
    default:
        return -1;
    }
    return  uda1345_write(UDA1345_STATUS,data);

}


int uda1345_setdcfilter(enum uda1345_dc_filter onoff)
{

    u8 data;
    data = uda1345_read_reg_cache(&falcon_uda1345,UDA1345_STATUS);
    data &= ~UDA1345_DC_MASK;
    switch(onoff)
    {
    case NO_DC_FILTERING:
        data |= UDA1345_NO_DC_FILTERING;
        break;
    case DC_FILTERING:
        data |= UDA1345_DC_FILTERING;
        break;
    default:
        return -1;
    }
    return  uda1345_write(UDA1345_STATUS,data);

}


int uda1345_setvolume(u8 dB)
{

    u8 data;
    data = uda1345_read_reg_cache(&falcon_uda1345,UDA1345_DATA_VOLUME);
    data &= ~UDA1345_VOLUME_MASK;
    data |= (dB&UDA1345_VOLUME_MASK);
    return  uda1345_write(UDA1345_DATA_VOLUME,data);

}


int uda1345_setdemphasis(enum uda1345_de_emphasis emphasis)
{
    u8 data;
    data = uda1345_read_reg_cache(&falcon_uda1345,UDA1345_DATA_DE_MUTE);
    data &= ~UDA1345_DE_EMPHASIS_MASK;
    switch(emphasis)
    {
    case NO_DE_EMPHASIS:
        data |= UDA1345_NO_DE_EMPHASIS;
        break;
    case DE_EMPHASIS_32KHZ:
        data |= UDA1345_DE_EMPHASIS_32KHZ;
        break;
    case DE_EMPHASIS_44KHZ:
        data |= UDA1345_DE_EMPHASIS_44KHZ;
        break;
    case DE_EMPHASIS_48KHZ:
        data |= UDA1345_DE_EMPHASIS_48KHZ;
        break;
    default:

        return -1;
    }
    return uda1345_write(UDA1345_DATA_DE_MUTE,data);

}

int uda1345_setmute(enum uda1345_mute onoff)
{
    u8 data;
    data = uda1345_read_reg_cache(&falcon_uda1345,UDA1345_DATA_DE_MUTE);
    data &= ~UDA1345_MUTE_MASK;
    switch(onoff)
    {
    case NO_MUTE:
        data |= UDA1345_NO_MUTE;
        break;
    case MUTE:
        data |= UDA1345_MUTE;
        break;
    default:
        return -1;
    }
    return  uda1345_write(UDA1345_DATA_DE_MUTE,data);

}

int uda1345_setpower(enum uda1345_power onoff)
{
    u8 data;
    data = uda1345_read_reg_cache(&falcon_uda1345,UDA1345_DATA_POWER);
    data &= ~UDA1345_ADC_DAC_MASK;
    switch(onoff)
    {
    case ADCOFF_DACOFF:
        data |= UDA1345_ADCOFF_DACOFF;
        break;
    case ADCOFF_DACON:
        data |= UDA1345_ADCOFF_DACON;
        break;
    case ADCON_DACOFF:
        data |= UDA1345_ADCON_DACOFF;
        break;
    case ADCON_DACON:
        data |= UDA1345_ADCON_DACON;
        break;
    default:
        return -1;
    }
    return  uda1345_write(UDA1345_DATA_POWER,data);

}

int uda1345_init(void)
{
    uda1345_GPIOInit();    

    if(0 ==  falcon_uda1345.cache_init)
    {
        falcon_uda1345.cache_size = sizeof(uda1346_reg);
        falcon_uda1345.reg_cache = cache;
        memcpy(falcon_uda1345.reg_cache, falcon_uda1345.reg_cache_default,falcon_uda1345.cache_size );
        falcon_uda1345.cache_init = 1;
    }

    if(uda1345_setsysclkfs(SYSCLKF_384FS))
        return -1;
    if(uda1345_setformat(I2S_BUS))
        return -1;
    if(uda1345_setdcfilter(NO_DC_FILTERING))
        return -1;
    if(uda1345_setdemphasis(NO_DE_EMPHASIS))
        return -1;
    if(uda1345_setvolume(0))
        return -1;
    if(uda1345_setmute(MUTE))
        return -1;
    if(uda1345_setpower(ADCOFF_DACOFF))
        return -1;

    return 0;
}

int uda1345_deinit(void)
{
    uda1345_setpower(ADCOFF_DACOFF);
    if(falcon_uda1345.cache_init&&falcon_uda1345.reg_cache)
    {
        falcon_uda1345.reg_cache = NULL;
        falcon_uda1345.cache_init =  0;
    }
    return 0;
}

