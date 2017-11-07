#ifndef DDK750_HELP_H__
#define DDK750_HELP_H__
#include "ddk750_chip.h"
#ifndef USE_INTERNAL_REGISTER_ACCESS

#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include "ddk750_mode.h"

#define PEEK32(addr) readl((addr)+mmio750)
#define POKE32(addr,data) writel((data),(addr)+mmio750)
#define peekRegisterDWord PEEK32
#define pokeRegisterDWord POKE32

#define peekRegisterByte(addr) readb((addr)+mmio750)
#define pokeRegisterByte(addr,data) writeb((data),(addr)+mmio750)

extern volatile unsigned  char __iomem * mmio750;
extern char revId750;
extern unsigned short devId750;
void ddk750_set_mmio(volatile unsigned char *,unsigned short,char);

/*
 *  Default Timing parameter for some popular modes.
 *  Note that the most timings in this table is made according to standard VESA 
 *  parameters for the popular modes.
 */
static mode_parameter_t gDefaultModeParamTable[] =
{
/*800x480*/
 //{0, 	800, 	0,	0,	NEG, 0, 480,  0,  0,   POS, 	0,		0,		0, NEG},
/* 320 x 240 [4:3] */
 { 352, 320, 335,  8, NEG, 265, 240, 254, 2, NEG,  5600000, 15909, 60, NEG},

/* 400 x 300 [4:3] -- Not a popular mode */
 { 528, 400, 420, 64, NEG, 314, 300, 301, 2, NEG,  9960000, 18864, 60, NEG},
 
/* 480 x 272 -- Not a popular mode --> only used for panel testing */
/* { 525, 480, 482, 41, NEG, 286, 272, 274, 10, NEG, 9009000, 17160, 60, NEG}, */

/* 640 x 480  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* { 840, 640, 680, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG}, */
/* { 832, 640, 700, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG}, */
 { 800, 640, 656, 96, NEG, 525, 480, 490, 2, NEG, 25175000, 31469, 60, NEG},
 { 840, 640, 656, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG},
 { 832, 640, 696, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG},

/* 720 x 480  [3:2] */
 { 889, 720, 738,108, POS, 525, 480, 490, 2, NEG, 28000000, 31496, 60, NEG},

/* 720 x 540  [4:3] -- Not a popular mode */
 { 886, 720, 740, 96, POS, 576, 540, 545, 2, POS, 30600000, 34537, 60, NEG},

/* 800 x 480  [5:3] -- Not a popular mode */
 { 973, 800, 822, 56, POS, 524, 480, 490, 2, NEG, 30600000, 31449, 60, NEG},

/* 800 x 600  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1062, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37665, 60, NEG}, */
/* {1054, 800, 842, 64, POS, 625, 600, 601, 3, POS, 56000000, 53131, 85, NEG}, */
 {1056, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37879, 60, NEG},
 {1056, 800, 816, 80, POS, 625, 600, 601, 3, POS, 49500000, 46875, 75, NEG},
 {1048, 800, 832, 64, POS, 631, 600, 601, 3, POS, 56250000, 53674, 85, NEG},

/* 960 x 720  [4:3] -- Not a popular mode */
 {1245, 960, 992, 64, POS, 750, 720, 721, 3, POS, 56000000, 44980, 60, NEG},
      
/* 1024 x 600  [16:9] 1.7 */
 {1313,1024,1064,104, POS, 622, 600, 601, 3, POS, 49000000, 37319, 60, NEG},
     
/* 1024 x 768  [4:3] */
/* The first 2 commented lines below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1340,1024,1060,136, NEG, 809, 768, 772, 6, NEG, 65000000, 48507, 60, NEG}, */
/* {1337,1024,1072, 96, NEG, 808, 768, 780, 3, NEG, 81000000, 60583, 75, NEG}, */
 {1344,1024,1048,136, NEG, 806, 768, 771, 6, NEG, 65000000, 48363, 60, NEG},
 {1312,1024,1040, 96, POS, 800, 768, 769, 3, POS, 78750000, 60023, 75, NEG},
 {1376,1024,1072, 96, POS, 808, 768, 769, 3, POS, 94500000, 68677, 85, NEG},
  
/* 1152 x 864  [4:3] -- Widescreen eXtended Graphics Array */
/* {1475,1152,1208, 96, NEG, 888, 864, 866, 3, NEG, 78600000, 53288, 60, NEG},*/
 {1475,1152,1208, 96, POS, 888, 864, 866, 3, POS, 78600000, 53288, 60, NEG},
 {1600,1152,1216,128, POS, 900, 864, 865, 3, POS,108000000, 67500, 75, NEG},
 
/* 1280 x 720  [16:9] -- HDTV (WXGA) */
 {1664,1280,1336,136, POS, 746, 720, 721, 3, POS, 74481000, 44760, 60, NEG},

/* 1280 x 768  [5:3] -- Not a popular mode */
 {1678,1280,1350,136, POS, 795, 768, 769, 3, POS, 80000000, 47676, 60, NEG},

/* 1280 x 800  [8:5] -- Not a popular mode */
 {1650,1280,1344,136, NEG, 824, 800, 800, 3, NEG, 81600000, 49455, 60, NEG},

/* 1280 x 960  [4:3] */
/* The first commented line below are taken from SM502, the rest timing are
   taken from the VESA Monitor Timing Standard */
/* {1618,1280,1330, 96, NEG, 977, 960, 960, 2, NEG, 94500000, 59259, 60, NEG},*/
 {1800,1280,1376,112, POS,1000, 960, 961, 3, POS,108000000, 60000, 60, NEG},
 {1728,1280,1344,160, POS,1011, 960, 961, 3, POS,148500000, 85938, 85, NEG},
    
/* 1280 x 1024 [5:4] */
#if 1
 /* GTF with C = 40, M = 600, J = 20, K = 128 */
 {1712,1280,1360,136, NEG,1060,1024,1025, 3, POS,108883200, 63600, 60, NEG},
 {1728,1280,1368,136, NEG,1069,1024,1025, 3, POS,138542400, 80175, 75, NEG},
 {1744,1280,1376,136, NEG,1075,1024,1025, 3, POS,159358000, 91375, 85, NEG},
#else
 /* VESA Standard */
 {1688,1280,1328,112, POS,1066,1024,1025, 3, POS,108000000, 63981, 60, NEG},
 {1688,1280,1296,144, POS,1066,1024,1025, 3, POS,135000000, 79976, 75, NEG},
 {1728,1280,1344,160, POS,1072,1024,1025, 3, POS,157500000, 91146, 85, NEG},
#endif

/* 1360 x 768 [16:9] */
#if 1
 /* GTF with C = 40, M = 600, J = 20, K = 128 */
 //{1776,1360,1432,136, NEG, 795, 768, 769, 3, POS, 84715200, 47700, 60, NEG},
 
 /* GTF with C = 30, M = 600, J = 20, K = 128 */
 {1664,1360,1384,128, NEG, 795, 768, 769, 3, POS, 79372800, 47700, 60, NEG},
#else
 /* Previous Calculation */
 {1776,1360,1424,144, POS, 795, 768, 769, 3, POS, 84715000, 47700, 60, NEG},
#endif
 
/* 1366 x 768  [16:9] */
 /* Previous Calculation  */
 {1722,1366,1424,112, NEG, 784, 768, 769, 3, NEG, 81000000, 47038, 60, NEG},
 
/* 1400 x 1050 [4:3] -- Hitachi TX38D95VC1CAH -- It is not verified yet, therefore
   temporarily disabled. */
 //{1688,1400,1448,112, NEG,1068,1050,1051, 3, NEG,108000000, 64000, 60, NEG},
 //{1688,1400,1464,112, NEG,1068,1050,1051, 3, NEG,108167040, 64080, 60, NEG},
 
 /* Taken from the www.tinyvga.com */
 {1880,1400,1488,152, NEG,1087,1050,1051, 3, POS,122610000, 65218, 60, NEG},
 
/* 1440 x 900  [8:5] -- Widescreen Super eXtended Graphics Array (WSXGA) */
 {1904,1440,1520,152, NEG, 932, 900, 901, 3, POS,106470000, 55919, 60, NEG},

/* 1440 x 960 [3:2] -- Not a popular mode */
 {1920,1440,1528,152, POS, 994, 960, 961, 3, POS,114509000, 59640, 60, NEG},

 /* 1600 x 900 */
 {2128,1600,1664,192, POS,932,900,901, 3, POS,119000000, 56000, 60, NEG},


/* 1600 x 1200 [4:3]. -- Ultra eXtended Graphics Array */
 /* VESA */
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,162000000, 75000, 60, NEG},
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,202500000, 93750, 75, NEG},
 {2160,1600,1664,192, POS,1250,1200,1201, 3, POS,229500000,106250, 85, NEG},

/* 
 * The timing below is taken from the www.tinyvga.com/vga-timing.
 * With the exception of 1920x1080.
 */
 
/* 1680 x 1050 [8:5]. -- Widescreen Super eXtended Graphics Array Plus (WSXGA+) */ 
/* The first commented timing might be used for DVI LCD Monitor timing. */
/* {1840,1680,1728, 32, NEG,1080,1050,1053, 6, POS,119232000, 64800, 60, NEG}, */
 /* GTF with C = 30, M = 600, J = 20, K = 128 */
 {2256,1680,1784,184, NEG,1087,1050,1051, 3, POS,147140000, 65222, 60, NEG},
/*
 {2272,1680,1792,184, NEG,1093,1050,1051, 3, POS,173831000, 76510, 70, NEG},
 {2288,1680,1800,184, NEG,1096,1050,1051, 3, POS,188074000, 82200, 75, NEG},
*/
 
/* 1792 x 1344 [4:3]. -- Not a popular mode */ 
 {2448,1792,1920,200, NEG,1394,1344,1345, 3, POS,204800000, 83660, 60, NEG},
 {2456,1792,1888,216, NEG,1417,1344,1345, 3, POS,261000000,106270, 75, NEG},
 
/* 1856 x 1392 [4:3]. -- Not a popular mode 
   The 1856 x 1392 @ 75Hz has not been tested due to high Horizontal Frequency
   where not all monitor can support it (including the developer monitor)
 */
 {2528,1856,1952,224, NEG,1439,1392,1393, 3, POS,218300000, 86353, 60, NEG},
/* {2560,1856,1984,224, NEG,1500,1392,1393, 3, POS,288000000,112500, 75, NEG},*/

/* 1920 x 1080 [16:9]. This is a make-up value, need to be proven. 
   The Pixel clock is calculated based on the maximum resolution of
   "Single Link" DVI, which support a maximum 165MHz pixel clock.
   The second values are taken from:
   http://www.tek.com/Measurement/App_Notes/25_14700/eng/25W_14700_3.pdf
 */
/* {2560,1920,2048,208, NEG,1125,1080,1081, 3, POS,172800000, 67500, 60, NEG}, */
 {2200,1920,2008, 44, NEG,1125,1080,1081, 3, POS,148500000, 67500, 60, NEG},

/* 1920 x 1200 [8:5]. -- Widescreen Ultra eXtended Graphics Array (WUXGA) */
 {2592,1920,2048,208, NEG,1242,1200,1201, 3, POS,193160000, 74522, 60, NEG},

/* 1920 x 1440 [4:3]. */
/* In the databook, it mentioned only support up to 1920x1440 @ 60Hz. 
   The timing for 75 Hz is provided here if necessary to do testing. - Some underflow
   has been noticed. */
 {2600,1920,2048,208, NEG,1500,1440,1441, 3, POS,234000000, 90000, 60, NEG},
/* {2640,1920,2064,224, NEG,1500,1440,1441, 3, POS,297000000,112500, 75, NEG}, */

/* End of table. */
 { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
};


#else
/* implement if you want use it*/
#endif

#endif

