#ifndef _DDK768_HELP_H__
#define _DDK768_HELP_H__

#ifndef USE_INTERNAL_REGISTER_ACCESS

#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define PEEK32(addr) readl((addr)+mmio768)
#define POKE32(addr,data) writel((data),(addr)+mmio768)

#define peekRegisterDWord(addr) readl((addr)+mmio768)
#define pokeRegisterDWord(addr,data) writel((data),(addr)+mmio768)

#define peekRegisterByte(addr) readb((addr)+mmio768)
#define pokeRegisterByte(addr,data) writeb((data),(addr)+mmio768)

/* Size of SM768 MMIO and memory */
#define SM768_PCI_ALLOC_MMIO_SIZE       (2*1024*1024)
#define SM768_PCI_ALLOC_MEMORY_SIZE     (128*1024*1024)

void ddk768_set_mmio(volatile unsigned char * addr,unsigned short devId,char revId);

extern volatile unsigned  char __iomem * mmio768;
extern char revId768;
extern unsigned short devId768;
#else
/* implement if you want use it*/
#endif

#endif
