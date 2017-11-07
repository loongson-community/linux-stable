#ifdef USE_HDMICHIP

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>	


#include "siHdmiTx_902x_TPI.h"
#include "ddk750_hwi2c.h"
#include "ddk750_swi2c.h"


#define SII9022_DEVICE_ID					0xb0

#define i2cWriteReg swI2CWriteReg
#define i2cReadReg  swI2CReadReg



unsigned char sii9022xIsConnected(void)	
{
	return 1;
}


static void dump9022a(uint8_t reg){
#if 1
	printk("reg%x=%x\n",reg,i2cReadReg(SII9022A_I2C_ADDRESS,reg));
#endif
}
int sii9022xSetMode(int num)
{
	int ret;

	swI2CInit(DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
	siHdmiTx_VideoSel(num);
	siHdmiTx_AudioSel(0x02);
	siHdmiTx_TPI_Init();
	ret = siHdmiTx_VideoSet();
	i2cWriteReg(SII9022A_I2C_ADDRESS,0x63,0);
	return ret;
}

int sii9022xInitChip(void)
{
	int  rcc,retries = 10;
    
	swI2CInit(DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
	
	/* enter TPI mode */
    	i2cWriteReg(SII9022A_I2C_ADDRESS,0xc7,0);
	do{
		msleep(1);
		rcc = i2cReadReg(SII9022A_I2C_ADDRESS,0x1B);
	}while((rcc != SII9022_DEVICE_ID) && retries--);

	if(rcc != SII9022_DEVICE_ID){
		printk("cannot detect sii9022a chip:rcc=%x\n",rcc);
		return -1;
	}
	return 0;
}


#endif
