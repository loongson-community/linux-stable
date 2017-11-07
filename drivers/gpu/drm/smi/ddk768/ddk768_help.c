#include "ddk768_help.h"

volatile unsigned char __iomem * mmio768 = NULL;
char revId768 = 0;
unsigned short devId768 = 0;

/* after driver mapped io registers, use this function first */
void ddk768_set_mmio(volatile unsigned char * addr,unsigned short devId,char revId)
{
	mmio768 = addr;
	devId768 = devId;
	revId768 = revId;
	printk("Found SM768 SOC Chip\n");
}


