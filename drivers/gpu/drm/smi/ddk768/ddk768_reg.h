
//#include "regsc.h"

#define SCRATCH_PAD0                                0x000060

#define SCRATCH_PAD1                                0x000064

#define CLOCK_ENABLE                                0x000068
#define CLOCK_ENABLE_JPU1R                          30:30
#define CLOCK_ENABLE_JPU1R_NORMAL                   0
#define CLOCK_ENABLE_JPU1R_RESET                    1
#define CLOCK_ENABLE_HDMIR                          29:29
#define CLOCK_ENABLE_HDMIR_NORMAL                   0
#define CLOCK_ENABLE_HDMIR_RESET                    1
#define CLOCK_ENABLE_USBHR                          28:28
#define CLOCK_ENABLE_USBHR_NORMAL                   0
#define CLOCK_ENABLE_USBHR_RESET                    1
#define CLOCK_ENABLE_USBSR                          27:27
#define CLOCK_ENABLE_USBSR_NORMAL                   0
#define CLOCK_ENABLE_USBSR_RESET                    1
#define CLOCK_ENABLE_ZVR                            26:26
#define CLOCK_ENABLE_ZVR_NORMAL                     0
#define CLOCK_ENABLE_ZVR_RESET                      1
#define CLOCK_ENABLE_JPUR                           25:25
#define CLOCK_ENABLE_JPUR_NORMAL                    0
#define CLOCK_ENABLE_JPUR_RESET                     1
#define CLOCK_ENABLE_VPUR                           24:24
#define CLOCK_ENABLE_VPUR_NORMAL                    0
#define CLOCK_ENABLE_VPUR_RESET                     1
#define CLOCK_ENABLE_DER                            23:23
#define CLOCK_ENABLE_DER_NORMAL                     0
#define CLOCK_ENABLE_DER_RESET                      1
#define CLOCK_ENABLE_DMAR                           22:22
#define CLOCK_ENABLE_DMAR_NORMAL                    0
#define CLOCK_ENABLE_DMAR_RESET                     1
#define CLOCK_ENABLE_DC1R                           18:18
#define CLOCK_ENABLE_DC1R_NORMAL                    0
#define CLOCK_ENABLE_DC1R_RESET                     1
#define CLOCK_ENABLE_DC0R                           17:17
#define CLOCK_ENABLE_DC0R_NORMAL                    0
#define CLOCK_ENABLE_DC0R_RESET                     1
#define CLOCK_ENABLE_ARMR                           16:16
#define CLOCK_ENABLE_ARMR_NORMAL                    0
#define CLOCK_ENABLE_ARMR_RESET                     1
#define CLOCK_ENABLE_JPU1                           14:14
#define CLOCK_ENABLE_JPU1_OFF                       0
#define CLOCK_ENABLE_JPU1_ON                        1
#define CLOCK_ENABLE_HDMI                           13:13
#define CLOCK_ENABLE_HDMI_OFF                       0 
#define CLOCK_ENABLE_HDMI_ON                        1
#define CLOCK_ENABLE_USBH                           12:12
#define CLOCK_ENABLE_USBH_OFF                       0
#define CLOCK_ENABLE_USBH_ON                        1
#define CLOCK_ENABLE_USBS                           11:11
#define CLOCK_ENABLE_USBS_OFF                       0
#define CLOCK_ENABLE_USBS_ON                        1
#define CLOCK_ENABLE_ZV                             10:10
#define CLOCK_ENABLE_ZV_OFF                         0
#define CLOCK_ENABLE_ZV_ON                          1
#define CLOCK_ENABLE_JPU                            9:9
#define CLOCK_ENABLE_JPU_OFF                        0
#define CLOCK_ENABLE_JPU_ON                         1
#define CLOCK_ENABLE_VPU                            8:8
#define CLOCK_ENABLE_VPU_OFF                        0
#define CLOCK_ENABLE_VPU_ON                         1
#define CLOCK_ENABLE_DE                             7:7
#define CLOCK_ENABLE_DE_OFF                         0
#define CLOCK_ENABLE_DE_ON                          1
#define CLOCK_ENABLE_DMA                            6:6
#define CLOCK_ENABLE_DMA_OFF                        0
#define CLOCK_ENABLE_DMA_ON                         1
#define CLOCK_ENABLE_UART                           5:5
#define CLOCK_ENABLE_UART_OFF                       0
#define CLOCK_ENABLE_UART_ON                        1
#define CLOCK_ENABLE_I2S                            4:4
#define CLOCK_ENABLE_I2S_OFF                        0
#define CLOCK_ENABLE_I2S_ON                         1
#define CLOCK_ENABLE_SSP                            3:3
#define CLOCK_ENABLE_SSP_OFF                        0
#define CLOCK_ENABLE_SSP_ON                         1
#define CLOCK_ENABLE_DC1                            2:2
#define CLOCK_ENABLE_DC1_OFF                        0
#define CLOCK_ENABLE_DC1_ON                         1
#define CLOCK_ENABLE_DC0                            1:1
#define CLOCK_ENABLE_DC0_OFF                        0
#define CLOCK_ENABLE_DC0_ON                         1
#define CLOCK_ENABLE_ARM                            0:0
#define CLOCK_ENABLE_ARM_OFF                        0
#define CLOCK_ENABLE_ARM_ON                         1

/* No bit fields for VGA PLL since DDK don't use it */
#define VGA25PLL                                    0x00006C
#define VGA28PLL                                    0x000070


/* Master clock for DDR and core */
#define MCLK_PLL                                     0x000074
#define MCLK_PLL_VCO                                23:22
#define MCLK_PLL_INT                                21:16
#define MCLK_PLL_POWER                                0:0
#define MCLK_PLL_POWER_NORMAL                        0
#define MCLK_PLL_POWER_DOWN                            1

/* Video clock 0  */
#define VCLK0_PLL                                      0x000078
/* VCLK Definition for FPGA */
#define VCLK0_PLL_FREQ                                 4:0
#define VCLK0_PLL_FREQ_25MHZ                           0
#define VCLK0_PLL_FREQ_40MHZ                           1
#define VCLK0_PLL_FREQ_65MHZ                           3
#define VCLK0_PLL_FREQ_74MHZ                           4
#define VCLK0_PLL_FREQ_80MHZ                           5
#define VCLK0_PLL_FREQ_108MHZ                          6

/* VCLK Definition for ASIC */
#define VCLK_PLL_SSCG                                30:30
#define VCLK_PLL_SSC                                29:28
#define VCLK_PLL_FN                                    27:27
#define VCLK_PLL_FN_INT_MODE                            0
#define VCLK_PLL_FN_FRAC_MODE                            1
#define VCLK_PLL_BS                                    26:25
#define VCLK_PLL_VCO                                24:22
#define VCLK_PLL_INT                                21:16
#define VCLK_PLL_FRAC                                15:1
#define VCLK_PLL_POWER                                0:0
#define VCLK_PLL_POWER_NORMAL                        0
#define VCLK_PLL_POWER_DOWN                            1

/* Video clock 1. Bit field definiton is same as VCLK0 */
#define VCLK1_PLL                                      0x00007C

#define PROTOCOL_SEMAPHORE0                            0x000080
#define PROTOCOL_SEMAPHORE1                            0x000084


#define VGA_CONFIGURATION                          	0x000088
#define VGA_CONFIGURATION_PLL                      	2:2
#define VGA_CONFIGURATION_PLL_VGA                  	0
#define VGA_CONFIGURATION_PLL_PANEL                	1
#define VGA_CONFIGURATION_MODE                     	1:1
#define VGA_CONFIGURATION_MODE_TEXT                	0
#define VGA_CONFIGURATION_MODE_GRAPHIC             	1
#define VGA_CONFIGURATION_PREFETCH                 	0:0
#define VGA_CONFIGURATION_PREFETCH_DISABLE         	0
#define VGA_CONFIGURATION_PREFETCH_ENABLE          	1

/* Lock or unlock PCIE bar 2 to 5 */
#define PCIE_BAR									0x00008C
#define PCIE_BAR_LOCK								0:0
#define PCIE_BAR_LOCK_UNLOCK						0
#define PCIE_BAR_LOCK_LOCK							1


#define RAW_INT                                    0x000090
#define RAW_INT_CSC                                5:5
#define RAW_INT_CSC_INACTIVE                       0
#define RAW_INT_CSC_ACTIVE                         1
#define RAW_INT_CSC_CLEAR                          1
#define RAW_INT_DE                                 4:4
#define RAW_INT_DE_INACTIVE                        0
#define RAW_INT_DE_ACTIVE                          1
#define RAW_INT_DE_CLEAR                           1
#define RAW_INT_ZVPORT_VSYNC                       3:3
#define RAW_INT_ZVPORT_VSYNC_INACTIVE              0
#define RAW_INT_ZVPORT_VSYNC_ACTIVE                1
#define RAW_INT_ZVPORT_VSYNC_CLEAR                 1
#define RAW_INT_CHANNEL1_VSYNC                     2:2
#define RAW_INT_CHANNEL1_VSYNC_INACTIVE            0
#define RAW_INT_CHANNEL1_VSYNC_ACTIVE              1
#define RAW_INT_CHANNEL1_VSYNC_CLEAR               1
#define RAW_INT_CHANNEL0_VSYNC                     1:1
#define RAW_INT_CHANNEL0_VSYNC_INACTIVE            0
#define RAW_INT_CHANNEL0_VSYNC_ACTIVE              1
#define RAW_INT_CHANNEL0_VSYNC_CLEAR               1
#define RAW_INT_VGA_VSYNC                          0:0
#define RAW_INT_VGA_VSYNC_INACTIVE                 0
#define RAW_INT_VGA_VSYNC_ACTIVE                   1
#define RAW_INT_VGA_VSYNC_CLEAR                    1

#ifdef SMI_ARM
#define INT_STATUS                                 0x0000A8
#else
#define INT_STATUS                                 0x000094
#endif
#define INT_STATUS_TIMER3                          31:31
#define INT_STATUS_TIMER3_INACTIVE                 0
#define INT_STATUS_TIMER3_ACTIVE                   1
#define INT_STATUS_TIMER2                          30:30
#define INT_STATUS_TIMER2_INACTIVE                 0
#define INT_STATUS_TIMER2_ACTIVE                   1
#define INT_STATUS_TIMER1                          29:29
#define INT_STATUS_TIMER1_INACTIVE                 0
#define INT_STATUS_TIMER1_ACTIVE                   1
#define INT_STATUS_TIMER0                          28:28
#define INT_STATUS_TIMER0_INACTIVE                 0
#define INT_STATUS_TIMER0_ACTIVE                   1
#define INT_STATUS_VPU                             27:27
#define INT_STATUS_VPU_INACTIVE                    0
#define INT_STATUS_VPU_ACTIVE                      1
#define INT_STATUS_JPU                             26:26
#define INT_STATUS_JPU_INACTIVE                    0
#define INT_STATUS_JPU_ACTIVE                      1
#define INT_STATUS_USBH                            25:25
#define INT_STATUS_USBH_INACTIVE                   0
#define INT_STATUS_USBH_ACTIVE                     1
#define INT_STATUS_USBS                            24:24
#define INT_STATUS_USBS_INACTIVE                   0
#define INT_STATUS_USBS_ACTIVE                     1
#define INT_STATUS_I2S                             23:23
#define INT_STATUS_I2S_INACTIVE                    0
#define INT_STATUS_I2S_ACTIVE                      1
#define INT_STATUS_UART1                           22:22
#define INT_STATUS_UART1_INACTIVE                  0
#define INT_STATUS_UART1_ACTIVE                    1
#define INT_STATUS_UART0                           21:21
#define INT_STATUS_UART0_INACTIVE                  0
#define INT_STATUS_UART0_ACTIVE                    1 
#define INT_STATUS_SSP1                            20:20
#define INT_STATUS_SSP1_INACTIVE                   0
#define INT_STATUS_SSP1_ACTIVE                     1
#define INT_STATUS_SSP0                            19:19
#define INT_STATUS_SSP0_INACTIVE                   0
#define INT_STATUS_SSP0_ACTIVE                     1
#define INT_STATUS_I2C1                            18:18
#define INT_STATUS_I2C1_INACTIVE                   0
#define INT_STATUS_I2C1_ACTIVE                     1
#define INT_STATUS_I2C0                            17:17
#define INT_STATUS_I2C0_INACTIVE                   0
#define INT_STATUS_I2C0_ACTIVE                     1
#define INT_STATUS_PWM                             16:16
#define INT_STATUS_PWM_INACTIVE                    0
#define INT_STATUS_PWM_ACTIVE                      1 
#define INT_STATUS_GPIO6                           15:15
#define INT_STATUS_GPIO6_INACTIVE                  0
#define INT_STATUS_GPIO6_ACTIVE                    1
#define INT_STATUS_GPIO5                           14:14
#define INT_STATUS_GPIO5_INACTIVE                  0
#define INT_STATUS_GPIO5_ACTIVE                    1
#define INT_STATUS_GPIO4                           13:13
#define INT_STATUS_GPIO4_INACTIVE                  0
#define INT_STATUS_GPIO4_ACTIVE                    1
#define INT_STATUS_GPIO3                           12:12
#define INT_STATUS_GPIO3_INACTIVE                  0
#define INT_STATUS_GPIO3_ACTIVE                    1
#define INT_STATUS_GPIO2                           11:11
#define INT_STATUS_GPIO2_INACTIVE                  0
#define INT_STATUS_GPIO2_ACTIVE                    1
#define INT_STATUS_GPIO1                           10:10
#define INT_STATUS_GPIO1_INACTIVE                  0
#define INT_STATUS_GPIO1_ACTIVE                    1
#define INT_STATUS_GPIO0                           9:9
#define INT_STATUS_GPIO0_INACTIVE                  0
#define INT_STATUS_GPIO0_ACTIVE                    1
#define INT_STATUS_DMA                             8:8
#define INT_STATUS_DMA_INACTIVE                    0
#define INT_STATUS_DMA_ACTIVE                      1
#define INT_STATUS_PCI                             7:7
#define INT_STATUS_PCI_INACTIVE                    0
#define INT_STATUS_PCI_ACTIVE                      1
#define INT_STATUS_HDMI							   6:6
#define INT_STATUS_HDMI_INACTIVE				   0
#define INT_STATUS_HDMI_ACTIVE					   1
#define INT_STATUS_CSC                             5:5
#define INT_STATUS_CSC_INACTIVE                    0
#define INT_STATUS_CSC_ACTIVE                      1
#define INT_STATUS_DE                              4:4
#define INT_STATUS_DE_INACTIVE                     0
#define INT_STATUS_DE_ACTIVE                       1
#define INT_STATUS_ZVPORT_VSYNC                    3:3
#define INT_STATUS_ZVPORT_VSYNC_INACTIVE           0
#define INT_STATUS_ZVPORT_VSYNC_ACTIVE             1
#define INT_STATUS_CHANNEL1_VSYNC                  2:2
#define INT_STATUS_CHANNEL1_VSYNC_INACTIVE         0
#define INT_STATUS_CHANNEL1_VSYNC_ACTIVE           1
#define INT_STATUS_CHANNEL0_VSYNC                  1:1
#define INT_STATUS_CHANNEL0_VSYNC_INACTIVE         0
#define INT_STATUS_CHANNEL0_VSYNC_ACTIVE           1
#define INT_STATUS_VGA_VSYNC                       0:0
#define INT_STATUS_VGA_VSYNC_INACTIVE              0
#define INT_STATUS_VGA_VSYNC_ACTIVE                1

#ifdef SMI_ARM
#define INT_MASK                                   0x00009C
#else
#define INT_MASK                                   0x000098
#endif

#define INT_MASK_TIMER3                            31:31
#define INT_MASK_TIMER3_DISABLE                    0
#define INT_MASK_TIMER3_ENABLE                     1
#define INT_MASK_TIMER2                            30:30
#define INT_MASK_TIMER2_DISABLE                    0
#define INT_MASK_TIMER2_ENABLE                     1
#define INT_MASK_TIMER1                            29:29
#define INT_MASK_TIMER1_DISABLE                    0
#define INT_MASK_TIMER1_ENABLE                     1
#define INT_MASK_TIMER0                            28:28
#define INT_MASK_TIMER0_DISABLE                    0
#define INT_MASK_TIMER0_ENABLE                     1
#define INT_MASK_VPU                               27:27
#define INT_MASK_VPU_DISABLE                       0
#define INT_MASK_VPU_ENABLE                        1
#define INT_MASK_JPU                               26:26
#define INT_MASK_JPU_DISABLE                       0
#define INT_MASK_JPU_ENABLE                        1
#define INT_MASK_USBH                              25:25
#define INT_MASK_USBH_DISABLE                      0
#define INT_MASK_USBH_ENABLE                       1
#define INT_MASK_USBS                              24:24
#define INT_MASK_USBS_DISABLE                      0
#define INT_MASK_USBS_ENABLE                       1
#define INT_MASK_I2S                               23:23
#define INT_MASK_I2S_DISABLE                       0
#define INT_MASK_I2S_ENABLE                        1
#define INT_MASK_UART1                             22:22
#define INT_MASK_UART1_DISABLE                     0
#define INT_MASK_UART1_ENABLE                      1
#define INT_MASK_UART0                             21:21
#define INT_MASK_UART0_DISABLE                     0
#define INT_MASK_UART0_ENABLE                      1 
#define INT_MASK_SSP1                              20:20
#define INT_MASK_SSP1_DISABLE                      0
#define INT_MASK_SSP1_ENABLE                       1
#define INT_MASK_SSP0                              19:19
#define INT_MASK_SSP0_DISABLE                      0
#define INT_MASK_SSP0_ENABLE                       1
#define INT_MASK_I2C1                              18:18
#define INT_MASK_I2C1_DISABLE                      0
#define INT_MASK_I2C1_ENABLE                       1
#define INT_MASK_I2C0                              17:17
#define INT_MASK_I2C0_DISABLE                      0
#define INT_MASK_I2C0_ENABLE                       1
#define INT_MASK_PWM                               16:16
#define INT_MASK_PWM_DISABLE                       0
#define INT_MASK_PWM_ENABLE                        1 
#define INT_MASK_GPIO6                             15:15
#define INT_MASK_GPIO6_DISABLE                     0
#define INT_MASK_GPIO6_ENABLE                      1
#define INT_MASK_GPIO5                             14:14
#define INT_MASK_GPIO5_DISABLE                     0
#define INT_MASK_GPIO5_ENABLE                      1
#define INT_MASK_GPIO4                             13:13
#define INT_MASK_GPIO4_DISABLE                     0
#define INT_MASK_GPIO4_ENABLE                      1
#define INT_MASK_GPIO3                             12:12
#define INT_MASK_GPIO3_DISABLE                     0
#define INT_MASK_GPIO3_ENABLE                      1
#define INT_MASK_GPIO2                             11:11
#define INT_MASK_GPIO2_DISABLE                     0
#define INT_MASK_GPIO2_ENABLE                      1
#define INT_MASK_GPIO1                             10:10
#define INT_MASK_GPIO1_DISABLE                     0
#define INT_MASK_GPIO1_ENABLE                      1
#define INT_MASK_GPIO0                             9:9
#define INT_MASK_GPIO0_DISABLE                     0
#define INT_MASK_GPIO0_ENABLE                      1
#define INT_MASK_DMA                               8:8
#define INT_MASK_DMA_DISABLE                       0
#define INT_MASK_DMA_ENABLE                        1
#define INT_MASK_CPU                               7:7
#define INT_MASK_CPU_DISABLE                       0
#define INT_MASK_CPU_ENABLE                        1
#define INT_MASK_HDMI							   6:6
#define INT_MASK_HDMI_DISABLE					   0
#define INT_MASK_HDMI_ENABLE					   1
#define INT_MASK_CSC                               5:5
#define INT_MASK_CSC_DISABLE                       0
#define INT_MASK_CSC_ENABLE                        1
#define INT_MASK_DE                                4:4
#define INT_MASK_DE_DISABLE                        0
#define INT_MASK_DE_ENABLE                         1
#define INT_MASK_ZVPORT_VSYNC                      3:3
#define INT_MASK_ZVPORT_VSYNC_DISABLE              0
#define INT_MASK_ZVPORT_VSYNC_ENABLE               1
#define INT_MASK_CHANNEL1_VSYNC                    2:2
#define INT_MASK_CHANNEL1_VSYNC_DISABLE            0
#define INT_MASK_CHANNEL1_VSYNC_ENABLE             1
#define INT_MASK_CHANNEL0_VSYNC                    1:1
#define INT_MASK_CHANNEL0_VSYNC_DISABLE            0
#define INT_MASK_CHANNEL0_VSYNC_ENABLE             1
#define INT_MASK_VGA_VSYNC                         0:0
#define INT_MASK_VGA_VSYNC_DISABLE                 0
#define INT_MASK_VGA_VSYNC_ENABLE                  1

#define ARM_PROTOCOL_INT                           	0x0000A0
#define ARM_PROTOCOL_INT_TOKEN						31:1
#define ARM_PROTOCOL_INT_ENABLE						0:0
#define ARM_PROTOCOL_INT_ENABLE_CLEAR				0
#define ARM_PROTOCOL_INT_ENABLE_ENABLE				1

#define PCIE_PROTOCOL_INT                          	0x0000A4
#define PCIE_PROTOCOL_INT_TOKEN						31:1
#define PCIE_PROTOCOL_INT_ENABLE					0:0
#define PCIE_PROTOCOL_INT_ENABLE_CLEAR				0
#define PCIE_PROTOCOL_INT_ENABLE_ENABLE				1

#define ARM_STARTUP_CONFIG	                        0x000100
#define ARM_STARTUP_CONFIG_USBH						30:30
#define ARM_STARTUP_CONFIG_USBH_NORMAL				0
#define ARM_STARTUP_CONFIG_USBH_RESET				1
#define ARM_STARTUP_CONFIG_USBHPHY					29:29
#define ARM_STARTUP_CONFIG_USBHPHY_NORMAL			0
#define ARM_STARTUP_CONFIG_USBHPHY_RESET			1
#define ARM_STARTUP_CONFIG_USBSPHY					28:28
#define ARM_STARTUP_CONFIG_USBSPHY_NORMAL			0
#define ARM_STARTUP_CONFIG_USBSPHY_RESET			1
#define ARM_STARTUP_CONFIG_USBS						27:27
#define ARM_STARTUP_CONFIG_USBS_NORMAL				0
#define ARM_STARTUP_CONFIG_USBS_RESET				1
#define ARM_STARTUP_CONFIG_ARM						0:0
#define ARM_STARTUP_CONFIG_ARM_STOP					0
#define ARM_STARTUP_CONFIG_ARM_START				1

#define ARM_CONTROL                          		0x000110
#define ARM_CONTROL_RESET							0:0
#define ARM_CONTROL_RESET_RESET						0
#define ARM_CONTROL_RESET_NORMAL					1

/* DDK shoudn't need to program this register */
#define USBH_HOST_CLOCK                                0x000114

#define TEST_CONTROL                                0x00011C
#define TEST_CONTROL_I2C                            11:11
#define TEST_CONTROL_I2C_I2C1                        0
#define TEST_CONTROL_I2C_HDMI                        1
#define TEST_CONTROL_SSP1                            10:10
#define TEST_CONTROL_SSP1_GPIO                        0
#define TEST_CONTROL_SSP1_USBSPHY                    1


#define STRAP_PINS                                    0x00012C
#define STRAP_PINS_15_2                                15:2
#define STRAP_PINS_MEM_SIZE                         1:0
#define STRAP_PINS_MEM_SIZE_128M                    0
#define STRAP_PINS_MEM_SIZE_256M                    1
#define STRAP_PINS_MEM_SIZE_512M                    2
#define STRAP_PINS_MEM_SIZE_1024M                   3


#define DDR_CONTROL                                  0x000130
#define DDR_CONTROL_COL                              3:2
#define DDR_CONTROL_COL_1024                         0   
#define DDR_CONTROL_COL_2048                         1
#define DDR_CONTROL_COL_256                          2
#define DDR_CONTROL_COL_512                          3
#define DDR_CONTROL_SIZE                             1:0   
#define DDR_CONTROL_SIZE_256M                        0
#define DDR_CONTROL_SIZE_512M                        1
#define DDR_CONTROL_SIZE_1024M                       2   
#define DDR_CONTROL_SIZE_128M                        3

#define JPU_PERFORMANCE_MODE                         0x000134
#define JPU_PERFORMANCE_MODE_JPU1                    3:2
#define JPU_PERFORMANCE_MODE_JPU1_DISABLE            0
#define JPU_PERFORMANCE_MODE_JPU1_HD                 1
#define JPU_PERFORMANCE_MODE_JPU1_UHD                2
#define JPU_PERFORMANCE_MODE_JPU0                    1:0
#define JPU_PERFORMANCE_MODE_JPU0_DISABLE	     0
#define JPU_PERFORMANCE_MODE_JPU0_HD                 1
#define JPU_PERFORMANCE_MODE_JPU0_UHD                2

#define DDR_PRIORITY1                                0x000138

#define DDR_PRIORITY2                                0x00013C


//#include "reggpio.h"



#define GPIO_DATA                                       0x010000
#define GPIO_DATA_IIC0_DATA_DVI_SHIFT                     31
#define GPIO_DATA_IIC0_CLK_DVI_SHIFT                        30
#define GPIO_DATA_SSP1_CLK_OUT_SHIFT                      29
#define GPIO_DATA_GPIO_CODEC_DATA_SHIFT               29
#define GPIO_DATA_SSP1_FRAMEINPUT_SHIFT                          28
#define GPIO_DATA_IIS_RX_SHIFT                                   28
#define GPIO_DATA_SSP1_FRAMEOUT_SHIFT                   27
#define GPIO_DATA_GPIO_IIS_TX_SHIFT                         27
#define GPIO_DATA_SSP1_RX_SHIFT                          26
#define GPIO_DATA_GPIO_IIS_WS_SHIFT                        26
#define GPIO_DATA_SSP1_TX_SHIFT                   25
#define GPIO_DATA_GPIO_IIS_CLK_SHIFT                       25
#define GPIO_DATA_SSP0_CLK_OUT_SHIFT                     24
#define GPIO_DATA_GPIO_SWSSP_CLK_OUT_SHIFT                     24
#define GPIO_DATA_SSP0_FRAMINPUT_SHIFT                 23
#define GPIO_DATA_GPIO_SWSSP_FRAMINPUT_SHIFT                 23
#define GPIO_DATA_SSP0_FRAMEOUT_SHIFT                  22
#define GPIO_DATA_GPIO_SWSSP_FRAMEOUT_SHIFT                  22
#define GPIO_DATA_SSP0_RX_SHIFT                               21
#define GPIO_DATA_GPIO_SWSSP_RX_SHIFT                               21
#define GPIO_DATA_SSP0_TX_SHIFT                               20
#define GPIO_DATA_GPIO_SWSSP_TX_SHIFT                               20
#define GPIO_DATA_PWM2_SHIFT                                   19
#define GPIO_DATA_GPIO_UART1_EXT_CLK_SHIFT        19
#define GPIO_DATA_GPIO_UART0_EXT_CLK_SHIFT        19
#define GPIO_DATA_PWM1_SHIFT                                   18
#define GPIO_DATA_PWM0_SHIFT                                   17
#define GPIO_DATA_PCIE_SHIFT                                     16
#define GPIO_DATA_UART0_SIN_SHIFT                          15
#define GPIO_DATA_UART0_NCTSIN_SHIFT                   14
#define GPIO_DATA_UART0_TXD_SHIFT                         13
#define GPIO_DATA_UART0_NRTS_SHIFT                       12
#define GPIO_DATA_UART1_SIN_SHIFT                         11
#define GPIO_DATA_GPIO_CODEC_CLK_SHIFT               11
#define GPIO_DATA_UART1_NCTSIN_SHIFT                   10
#define GPIO_DATA_GPIO_CODEC_MODE_SHIFT            10
#define GPIO_DATA_UART1_TXD_SHIFT                         9
#define GPIO_DATA_GPIO_IIC_DATA_HDMI_SHIFT       9
#define GPIO_DATA_GPIO_IIC_CLK_HDMI_SHIFT          8
#define GPIO_DATA_UART1_NRTS_SHIFT                       8
#define GPIO_DATA_IIC1_DATA_CRT_SHIFT                  7
#define GPIO_DATA_IIC1_CLK_CRT_SHIFT                     6
#define GPIO_DATA_GPIO_VBUS_SHIFT                         5
#define GPIO_DATA_IIS_TX_SELECT_SHIFT                   4
#define GPIO_DATA_GPIO_DVI_PNP_SHIFT                   4
#define GPIO_DATA_IIS_WS_SELECT_SHIFT                 3
#define GPIO_DATA_GPIO_AUDIO_PNP_SHIFT              3
#define GPIO_DATA_IIS_CLK_SELECT_SHIFT                2
#define GPIO_DATA_UART1_EXT_CLK_SELECT_SHIFT   1
#define GPIO_DATA_GPIO_HDMI_PNP_SHIFT                1
#define GPIO_DATA_UART0_EXT_CLK_SELECT_SHIFT   0

#define GPIO_DATA_DIRECTION                             0x010004
#define GPIO_DATA_DIRECTION_IIC0_DATA_DVI_SHIFT                     31
#define GPIO_DATA_DIRECTION_IIC0_CLK_DVI_SHIFT                        30
#define GPIO_DATA_DIRECTION_SSP1_CLK_OUT_SHIFT                      29
#define GPIO_DATA_DIRECTION_GPIO_CODEC_DATA_SHIFT               29
#define GPIO_DATA_DIRECTION_SSP1_FRAMEINPUT_SHIFT                          28
#define GPIO_DATA_DIRECTION_IIS_RX_SHIFT                                   28
#define GPIO_DATA_DIRECTION_SSP1_FRAMEOUT_SHIFT                   27
#define GPIO_DATA_DIRECTION_GPIO_IIS_TX_SHIFT                         27
#define GPIO_DATA_DIRECTION_SSP1_RX_SHIFT                          26
#define GPIO_DATA_DIRECTION_GPIO_IIS_WS_SHIFT                        26
#define GPIO_DATA_DIRECTION_SSP1_TX_SHIFT                   25
#define GPIO_DATA_DIRECTION_GPIO_IIS_CLK_SHIFT                       25
#define GPIO_DATA_DIRECTION_SSP0_CLK_OUT_SHIFT                     24
#define GPIO_DATA_DIRECTION_GPIO_SWSSP_CLK_OUT_SHIFT                     24
#define GPIO_DATA_DIRECTION_SSP0_FRAMEINPUT_SHIFT                         23
#define GPIO_DATA_DIRECTION_GPIO_SWSSP_FRAMEINPUT_SHIFT                         23
#define GPIO_DATA_DIRECTION_SSP0_FRAMEOUT_SHIFT                  22
#define GPIO_DATA_DIRECTION_GPIO_SWSSP_FRAMEOUT_SHIFT                  22
#define GPIO_DATA_DIRECTION_SSP0_RX_SHIFT                         21
#define GPIO_DATA_DIRECTION_GPIO_SWSSP_RX_SHIFT                         21
#define GPIO_DATA_DIRECTION_SSP0_TX_SHIFT                  20
#define GPIO_DATA_DIRECTION_GPIO_SWSSP_TX_SHIFT                  20
#define GPIO_DATA_DIRECTION_PWM2_SHIFT                                   19
#define GPIO_DATA_DIRECTION_GPIO_UART1_EXT_CLK_SHIFT        19
#define GPIO_DATA_DIRECTION_GPIO_UART0_EXT_CLK_SHIFT        19
#define GPIO_DATA_DIRECTION_PWM1_SHIFT                                   18
#define GPIO_DATA_DIRECTION_PWM0_SHIFT                                   17
#define GPIO_DATA_DIRECTION_PCIE_SHIFT                                     16
#define GPIO_DATA_DIRECTION_UART0_SIN_SHIFT                          15
#define GPIO_DATA_DIRECTION_UART0_NCTSIN_SHIFT                   14
#define GPIO_DATA_DIRECTION_UART0_TXD_SHIFT                         13
#define GPIO_DATA_DIRECTION_UART0_NRTS_SHIFT                       12
#define GPIO_DATA_DIRECTION_UART1_SIN_SHIFT                         11
#define GPIO_DATA_DIRECTION_GPIO_CODEC_CLK_SHIFT               11
#define GPIO_DATA_DIRECTION_UART1_NCTSIN_SHIFT                   10
#define GPIO_DATA_DIRECTION_GPIO_CODEC_MODE_SHIFT            10
#define GPIO_DATA_DIRECTION_UART1_TXD_SHIFT                         9
#define GPIO_DATA_DIRECTION_GPIO_IIC_DATA_HDMI_SHIFT       9
#define GPIO_DATA_DIRECTION_GPIO_IIC_CLK_HDMI_SHIFT          8
#define GPIO_DATA_DIRECTION_UART1_NRTS_SHIFT                       8
#define GPIO_DATA_DIRECTION_IIC1_DATA_CRT_SHIFT                  7
#define GPIO_DATA_DIRECTION_IIC1_CLK_CRT_SHIFT                     6
#define GPIO_DATA_DIRECTION_GPIO_VBUS_SHIFT                         5
#define GPIO_DATA_DIRECTION_IIS_TX_SELECT_SHIFT                   4
#define GPIO_DATA_DIRECTION_GPIO_DVI_PNP_SHIFT                   4
#define GPIO_DATA_DIRECTION_IIS_WS_SELECT_SHIFT                 3
#define GPIO_DATA_DIRECTION_GPIO_AUDIO_PNP_SHIFT              3
#define GPIO_DATA_DIRECTION_IIS_CLK_SELECT_SHIFT                2
#define GPIO_DATA_DIRECTION_UART1_EXT_CLK_SELECT_SHIFT   1
#define GPIO_DATA_DIRECTION_GPIO_HDMI_PNP_SHIFT                1
#define GPIO_DATA_DIRECTION_UART0_EXT_CLK_SELECT_SHIFT   0


#define GPIO_INTERRUPT_SETUP                            0x010008
#define GPIO_INTERRUPT_SETUP_VBUS_TRIGGER_EDGE_SHIFT    21
#define GPIO_INTERRUPT_SETUP_VBUS_TRIGGER_LEVEL_SHIFT   21
#define GPIO_INTERRUPT_SETUP_DVI_PNP_TRIGGER_EDGE_SHIFT    20
#define GPIO_INTERRUPT_SETUP_DVI_PNP_TRIGGER_LEVEL_SHIFT   20
#define GPIO_INTERRUPT_SETUP_AUDIO_PNP_TRIGGER_EDGE_SHIFT    19
#define GPIO_INTERRUPT_SETUP_AUDIO_PNP_TRIGGER_LEVEL_SHIFT   19
#define GPIO_INTERRUPT_SETUP_HDMI_PNP_TRIGGER_EDGE_SHIFT    17
#define GPIO_INTERRUPT_SETUP_HDMI_PNP_TRIGGER_LEVEL_SHIFT   17


#define GPIO_INTERRUPT_SETUP_VBUS_ACTIVE_LOW_SHIFT    13
#define GPIO_INTERRUPT_SETUP_VBUS_ACTIVE_HIGH_SHIFT  13
#define GPIO_INTERRUPT_SETUP_DVI_PNP_ACTIVE_LOW_SHIFT    12
#define GPIO_INTERRUPT_SETUP_DVI_PNP_ACTIVE_HIGH_SHIFT   12
#define GPIO_INTERRUPT_SETUP_AUDIO_PNP_ACTIVE_LOW_SHIFT    11
#define GPIO_INTERRUPT_SETUP_AUDIO_PNP_ACTIVE_HIGH_SHIFT   11
#define GPIO_INTERRUPT_SETUP_HDMI_PNP_ACTIVE_LOW_SHIFT    9
#define GPIO_INTERRUPT_SETUP_HDMI_PNP_ACTIVE_HIGH_SHIFT   9


#define GPIO_INTERRUPT_SETUP_VBUS_ENABLE_SHIFT             5
#define GPIO_INTERRUPT_SETUP_VBUS_GPIO_ENABLE_SHIFT             5
#define GPIO_INTERRUPT_SETUP_DVI_PNP_ENABLE_SHIFT            4
#define GPIO_INTERRUPT_SETUP_DVI_PNP_GPIO_ENABLE_SHIFT        4
#define GPIO_INTERRUPT_SETUP_AUDIO_PNP_ENABLE_SHIFT             3
#define GPIO_INTERRUPT_SETUP_AUDIO_PNP_GPIO_ENABLE_SHIFT        3
#define GPIO_INTERRUPT_SETUP_HDMI_PNP_ENABLE_SHIFT             1
#define GPIO_INTERRUPT_SETUP_HDMI_PNP_GPIO_ENABLE_SHIFT        1

#define GPIO_INTERRUPT_STATUS                          0x01000C
#define GPIO_INTERRUPT_STATUS_VBUS_INACTIVE               5
#define GPIO_INTERRUPT_STATUS_VBUS_ACTIVE                 5
#define GPIO_INTERRUPT_STATUS_VBUS_RESET                  5
#define GPIO_INTERRUPT_STATUS_DVI_PNP_INACTIVE               4
#define GPIO_INTERRUPT_STATUS_DVI_PNP_ACTIVE                 4
#define GPIO_INTERRUPT_STATUS_DVI_PNP_RESET                  4
#define GPIO_INTERRUPT_STATUS_AUDIO_PNP_INACTIVE              3
#define GPIO_INTERRUPT_STATUS_AUDIO_PNP_ACTIVE                 3
#define GPIO_INTERRUPT_STATUS_AUDIO_PNP_RESET                  3
#define GPIO_INTERRUPT_STATUS_HDMI_PNP_INACTIVE             1
#define GPIO_INTERRUPT_STATUS_HDMI_PNP_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_HDMI_PNP_RESET                  1

/*xxx_GPIO_xxxx means 0 is valid*/
#define GPIO_MUX                                                     0x010010
#define GPIO_MUX_IIC0_DATA_DVI_SHIFT                     31
#define GPIO_MUX_IIC0_CLK_DVI_SHIFT                        30
#define GPIO_MUX_SSP1_CLK_OUT_SHIFT                      29
#define GPIO_MUX_GPIO_CODEC_DATA_SHIFT               29
#define GPIO_MUX_SSP1_FRAMEINPUT_SHIFT                28
#define GPIO_MUX_IIS_RX_SHIFT                                   28
#define GPIO_MUX_SSP1_FRAMEOUT_SHIFT                   27
#define GPIO_MUX_GPIO_IIS_TX_SHIFT                         27
#define GPIO_MUX_SSP1_RX_SHIFT                                26
#define GPIO_MUX_GPIO_IIS_WS_SHIFT                        26
#define GPIO_MUX_SSP1_TX_SHIFT                               25
#define GPIO_MUX_GPIO_IIS_CLK_SHIFT                       25
#define GPIO_MUX_SSP0_CLK_OUT_SHIFT                     24
#define GPIO_MUX_GPIO_SWSSP_CLK_OUT_SHIFT                  24
#define GPIO_MUX_SSP0_FRAMEINPUT_SHIFT              23
#define GPIO_MUX_GPIO_SWSSP_FRAMEINPUT_SHIFT           23
#define GPIO_MUX_SSP0_FRAMEOUT_SHIFT                  22
#define GPIO_MUX_GPIO_SWSSP_FRAMEOUT_SHIFT               22
#define GPIO_MUX_SSP0_RX_SHIFT                               21
#define GPIO_MUX_GPIO_SWSSP_RX_SHIFT                            21
#define GPIO_MUX_SSP0_TX_SHIFT                               20
#define GPIO_MUX_GPIO_SWSSP_TX_SHIFT                            20
#define GPIO_MUX_PWM2_SHIFT                                   19
#define GPIO_MUX_GPIO_UART1_EXT_CLK_SHIFT        19
#define GPIO_MUX_GPIO_UART0_EXT_CLK_SHIFT        19
#define GPIO_MUX_PWM1_SHIFT                                   18
#define GPIO_MUX_PWM0_SHIFT                                   17
#define GPIO_MUX_PCIE_SHIFT                                     16
#define GPIO_MUX_UART0_SIN_SHIFT                          15
#define GPIO_MUX_UART0_NCTSIN_SHIFT                   14
#define GPIO_MUX_UART0_TXD_SHIFT                         13
#define GPIO_MUX_UART0_NRTS_SHIFT                       12
#define GPIO_MUX_UART1_SIN_SHIFT                         11
#define GPIO_MUX_GPIO_CODEC_CLK_SHIFT               11
#define GPIO_MUX_UART1_NCTSIN_SHIFT                   10
#define GPIO_MUX_GPIO_CODEC_MODE_SHIFT            10
#define GPIO_MUX_UART1_TXD_SHIFT                         9
#define GPIO_MUX_GPIO_IIC_DATA_HDMI_SHIFT       9
#define GPIO_MUX_GPIO_IIC_CLK_HDMI_SHIFT          8
#define GPIO_MUX_UART1_NRTS_SHIFT                       8
#define GPIO_MUX_IIC1_DATA_CRT_SHIFT                  7
#define GPIO_MUX_IIC1_CLK_CRT_SHIFT                     6
#define GPIO_MUX_GPIO_VBUS_SHIFT                         5
#define GPIO_MUX_IIS_TX_SELECT_SHIFT                   4
#define GPIO_MUX_GPIO_DVI_PNP_SHIFT                   4
#define GPIO_MUX_IIS_WS_SELECT_SHIFT                 3
#define GPIO_MUX_GPIO_AUDIO_PNP_SHIFT              3
#define GPIO_MUX_IIS_CLK_SELECT_SHIFT                2
#define GPIO_MUX_UART1_EXT_CLK_SELECT_SHIFT   1
#define GPIO_MUX_GPIO_HDMI_PNP_SHIFT                1
#define GPIO_MUX_UART0_EXT_CLK_SELECT_SHIFT   0



#define GPIO_DATA_31                                    31:31
#define GPIO_DATA_30                                    30:30
#define GPIO_DATA_29                                    29:29
#define GPIO_DATA_28                                    28:28
#define GPIO_DATA_27                                    27:27
#define GPIO_DATA_26                                    26:26
#define GPIO_DATA_25                                    25:25
#define GPIO_DATA_24                                    24:24
#define GPIO_DATA_23                                    23:23
#define GPIO_DATA_22                                    22:22
#define GPIO_DATA_21                                    21:21
#define GPIO_DATA_20                                    20:20
#define GPIO_DATA_19                                    19:19
#define GPIO_DATA_18                                    18:18
#define GPIO_DATA_17                                    17:17
#define GPIO_DATA_16                                    16:16
#define GPIO_DATA_15                                    15:15
#define GPIO_DATA_14                                    14:14
#define GPIO_DATA_13                                    13:13
#define GPIO_DATA_12                                    12:12
#define GPIO_DATA_11                                    11:11
#define GPIO_DATA_10                                    10:10
#define GPIO_DATA_9                                     9:9
#define GPIO_DATA_8                                     8:8
#define GPIO_DATA_7                                     7:7
#define GPIO_DATA_6                                     6:6
#define GPIO_DATA_5                                     5:5
#define GPIO_DATA_4                                     4:4
#define GPIO_DATA_3                                     3:3
#define GPIO_DATA_2                                     2:2
#define GPIO_DATA_1                                     1:1
#define GPIO_DATA_0                                     0:0


#define GPIO_DATA_DIRECTION_31                          31:31
#define GPIO_DATA_DIRECTION_31_INPUT                    0
#define GPIO_DATA_DIRECTION_31_OUTPUT                   1
#define GPIO_DATA_DIRECTION_30                          30:30
#define GPIO_DATA_DIRECTION_30_INPUT                    0
#define GPIO_DATA_DIRECTION_30_OUTPUT                   1
#define GPIO_DATA_DIRECTION_29                          29:29
#define GPIO_DATA_DIRECTION_29_INPUT                    0
#define GPIO_DATA_DIRECTION_29_OUTPUT                   1
#define GPIO_DATA_DIRECTION_28                          28:28
#define GPIO_DATA_DIRECTION_28_INPUT                    0
#define GPIO_DATA_DIRECTION_28_OUTPUT                   1
#define GPIO_DATA_DIRECTION_27                          27:27
#define GPIO_DATA_DIRECTION_27_INPUT                    0
#define GPIO_DATA_DIRECTION_27_OUTPUT                   1
#define GPIO_DATA_DIRECTION_26                          26:26
#define GPIO_DATA_DIRECTION_26_INPUT                    0
#define GPIO_DATA_DIRECTION_26_OUTPUT                   1
#define GPIO_DATA_DIRECTION_25                          25:25
#define GPIO_DATA_DIRECTION_25_INPUT                    0
#define GPIO_DATA_DIRECTION_25_OUTPUT                   1
#define GPIO_DATA_DIRECTION_24                          24:24
#define GPIO_DATA_DIRECTION_24_INPUT                    0
#define GPIO_DATA_DIRECTION_24_OUTPUT                   1
#define GPIO_DATA_DIRECTION_23                          23:23
#define GPIO_DATA_DIRECTION_23_INPUT                    0
#define GPIO_DATA_DIRECTION_23_OUTPUT                   1
#define GPIO_DATA_DIRECTION_22                          22:22
#define GPIO_DATA_DIRECTION_22_INPUT                    0
#define GPIO_DATA_DIRECTION_22_OUTPUT                   1
#define GPIO_DATA_DIRECTION_21                          21:21
#define GPIO_DATA_DIRECTION_21_INPUT                    0
#define GPIO_DATA_DIRECTION_21_OUTPUT                   1
#define GPIO_DATA_DIRECTION_20                          20:20
#define GPIO_DATA_DIRECTION_20_INPUT                    0
#define GPIO_DATA_DIRECTION_20_OUTPUT                   1
#define GPIO_DATA_DIRECTION_19                          19:19
#define GPIO_DATA_DIRECTION_19_INPUT                    0
#define GPIO_DATA_DIRECTION_19_OUTPUT                   1
#define GPIO_DATA_DIRECTION_18                          18:18
#define GPIO_DATA_DIRECTION_18_INPUT                    0
#define GPIO_DATA_DIRECTION_18_OUTPUT                   1
#define GPIO_DATA_DIRECTION_17                          17:17
#define GPIO_DATA_DIRECTION_17_INPUT                    0
#define GPIO_DATA_DIRECTION_17_OUTPUT                   1
#define GPIO_DATA_DIRECTION_16                          16:16
#define GPIO_DATA_DIRECTION_16_INPUT                    0
#define GPIO_DATA_DIRECTION_16_OUTPUT                   1
#define GPIO_DATA_DIRECTION_15                          15:15
#define GPIO_DATA_DIRECTION_15_INPUT                    0
#define GPIO_DATA_DIRECTION_15_OUTPUT                   1
#define GPIO_DATA_DIRECTION_14                          14:14
#define GPIO_DATA_DIRECTION_14_INPUT                    0
#define GPIO_DATA_DIRECTION_14_OUTPUT                   1
#define GPIO_DATA_DIRECTION_13                          13:13
#define GPIO_DATA_DIRECTION_13_INPUT                    0
#define GPIO_DATA_DIRECTION_13_OUTPUT                   1
#define GPIO_DATA_DIRECTION_12                          12:12
#define GPIO_DATA_DIRECTION_12_INPUT                    0
#define GPIO_DATA_DIRECTION_12_OUTPUT                   1
#define GPIO_DATA_DIRECTION_11                          11:11
#define GPIO_DATA_DIRECTION_11_INPUT                    0
#define GPIO_DATA_DIRECTION_11_OUTPUT                   1
#define GPIO_DATA_DIRECTION_10                          10:10
#define GPIO_DATA_DIRECTION_10_INPUT                    0
#define GPIO_DATA_DIRECTION_10_OUTPUT                   1
#define GPIO_DATA_DIRECTION_9                           9:9
#define GPIO_DATA_DIRECTION_9_INPUT                     0
#define GPIO_DATA_DIRECTION_9_OUTPUT                    1
#define GPIO_DATA_DIRECTION_8                           8:8
#define GPIO_DATA_DIRECTION_8_INPUT                     0
#define GPIO_DATA_DIRECTION_8_OUTPUT                    1
#define GPIO_DATA_DIRECTION_7                           7:7
#define GPIO_DATA_DIRECTION_7_INPUT                     0
#define GPIO_DATA_DIRECTION_7_OUTPUT                    1
#define GPIO_DATA_DIRECTION_6                           6:6
#define GPIO_DATA_DIRECTION_6_INPUT                     0
#define GPIO_DATA_DIRECTION_6_OUTPUT                    1
#define GPIO_DATA_DIRECTION_5                           5:5
#define GPIO_DATA_DIRECTION_5_INPUT                     0
#define GPIO_DATA_DIRECTION_5_OUTPUT                    1
#define GPIO_DATA_DIRECTION_4                           4:4
#define GPIO_DATA_DIRECTION_4_INPUT                     0
#define GPIO_DATA_DIRECTION_4_OUTPUT                    1
#define GPIO_DATA_DIRECTION_3                           3:3
#define GPIO_DATA_DIRECTION_3_INPUT                     0
#define GPIO_DATA_DIRECTION_3_OUTPUT                    1
#define GPIO_DATA_DIRECTION_2                           2:2
#define GPIO_DATA_DIRECTION_2_INPUT                     0
#define GPIO_DATA_DIRECTION_2_OUTPUT                    1
#define GPIO_DATA_DIRECTION_1                           1:1
#define GPIO_DATA_DIRECTION_1_INPUT                     0
#define GPIO_DATA_DIRECTION_1_OUTPUT                    1
#define GPIO_DATA_DIRECTION_0                           0:0
#define GPIO_DATA_DIRECTION_0_INPUT                     0
#define GPIO_DATA_DIRECTION_0_OUTPUT                    1


#define GPIO_INTERRUPT_SETUP_TRIGGER_31                 22:22
#define GPIO_INTERRUPT_SETUP_TRIGGER_31_EDGE            0
#define GPIO_INTERRUPT_SETUP_TRIGGER_31_LEVEL           1
#define GPIO_INTERRUPT_SETUP_TRIGGER_30                 21:21
#define GPIO_INTERRUPT_SETUP_TRIGGER_30_EDGE            0
#define GPIO_INTERRUPT_SETUP_TRIGGER_30_LEVEL           1
#define GPIO_INTERRUPT_SETUP_TRIGGER_29                 20:20
#define GPIO_INTERRUPT_SETUP_TRIGGER_29_EDGE            0
#define GPIO_INTERRUPT_SETUP_TRIGGER_29_LEVEL           1
#define GPIO_INTERRUPT_SETUP_TRIGGER_28                 19:19
#define GPIO_INTERRUPT_SETUP_TRIGGER_28_EDGE            0
#define GPIO_INTERRUPT_SETUP_TRIGGER_28_LEVEL           1
#define GPIO_INTERRUPT_SETUP_TRIGGER_27                 18:18
#define GPIO_INTERRUPT_SETUP_TRIGGER_27_EDGE            0
#define GPIO_INTERRUPT_SETUP_TRIGGER_27_LEVEL           1
#define GPIO_INTERRUPT_SETUP_TRIGGER_26                 17:17
#define GPIO_INTERRUPT_SETUP_TRIGGER_26_EDGE            0
#define GPIO_INTERRUPT_SETUP_TRIGGER_26_LEVEL           1
#define GPIO_INTERRUPT_SETUP_TRIGGER_25                 16:16
#define GPIO_INTERRUPT_SETUP_TRIGGER_25_EDGE            0
#define GPIO_INTERRUPT_SETUP_TRIGGER_25_LEVEL           1
#define GPIO_INTERRUPT_SETUP_ACTIVE_31                  14:14
#define GPIO_INTERRUPT_SETUP_ACTIVE_31_LOW              0
#define GPIO_INTERRUPT_SETUP_ACTIVE_31_HIGH             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_30                  13:13
#define GPIO_INTERRUPT_SETUP_ACTIVE_30_LOW              0
#define GPIO_INTERRUPT_SETUP_ACTIVE_30_HIGH             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_29                  12:12
#define GPIO_INTERRUPT_SETUP_ACTIVE_29_LOW              0
#define GPIO_INTERRUPT_SETUP_ACTIVE_29_HIGH             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_28                  11:11
#define GPIO_INTERRUPT_SETUP_ACTIVE_28_LOW              0
#define GPIO_INTERRUPT_SETUP_ACTIVE_28_HIGH             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_27                  10:10
#define GPIO_INTERRUPT_SETUP_ACTIVE_27_LOW              0
#define GPIO_INTERRUPT_SETUP_ACTIVE_27_HIGH             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_26                  9:9
#define GPIO_INTERRUPT_SETUP_ACTIVE_26_LOW              0
#define GPIO_INTERRUPT_SETUP_ACTIVE_26_HIGH             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_25                  8:8
#define GPIO_INTERRUPT_SETUP_ACTIVE_25_LOW              0
#define GPIO_INTERRUPT_SETUP_ACTIVE_25_HIGH             1
#define GPIO_INTERRUPT_SETUP_ENABLE_31                  6:6
#define GPIO_INTERRUPT_SETUP_ENABLE_31_GPIO             0
#define GPIO_INTERRUPT_SETUP_ENABLE_31_INTERRUPT        1
#define GPIO_INTERRUPT_SETUP_ENABLE_30                  5:5
#define GPIO_INTERRUPT_SETUP_ENABLE_30_GPIO             0
#define GPIO_INTERRUPT_SETUP_ENABLE_30_INTERRUPT        1
#define GPIO_INTERRUPT_SETUP_ENABLE_29                  4:4
#define GPIO_INTERRUPT_SETUP_ENABLE_29_GPIO             0
#define GPIO_INTERRUPT_SETUP_ENABLE_29_INTERRUPT        1
#define GPIO_INTERRUPT_SETUP_ENABLE_28                  3:3
#define GPIO_INTERRUPT_SETUP_ENABLE_28_GPIO             0
#define GPIO_INTERRUPT_SETUP_ENABLE_28_INTERRUPT        1
#define GPIO_INTERRUPT_SETUP_ENABLE_27                  2:2
#define GPIO_INTERRUPT_SETUP_ENABLE_27_GPIO             0
#define GPIO_INTERRUPT_SETUP_ENABLE_27_INTERRUPT        1
#define GPIO_INTERRUPT_SETUP_ENABLE_26                  1:1
#define GPIO_INTERRUPT_SETUP_ENABLE_26_GPIO             0
#define GPIO_INTERRUPT_SETUP_ENABLE_26_INTERRUPT        1
#define GPIO_INTERRUPT_SETUP_ENABLE_25                  0:0
#define GPIO_INTERRUPT_SETUP_ENABLE_25_GPIO             0
#define GPIO_INTERRUPT_SETUP_ENABLE_25_INTERRUPT        1


#define GPIO_INTERRUPT_STATUS_31                        22:22
#define GPIO_INTERRUPT_STATUS_31_INACTIVE               0
#define GPIO_INTERRUPT_STATUS_31_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_31_RESET                  1
#define GPIO_INTERRUPT_STATUS_30                        21:21
#define GPIO_INTERRUPT_STATUS_30_INACTIVE               0
#define GPIO_INTERRUPT_STATUS_30_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_30_RESET                  1
#define GPIO_INTERRUPT_STATUS_29                        20:20
#define GPIO_INTERRUPT_STATUS_29_INACTIVE               0
#define GPIO_INTERRUPT_STATUS_29_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_29_RESET                  1
#define GPIO_INTERRUPT_STATUS_28                        19:19
#define GPIO_INTERRUPT_STATUS_28_INACTIVE               0
#define GPIO_INTERRUPT_STATUS_28_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_28_RESET                  1
#define GPIO_INTERRUPT_STATUS_27                        18:18
#define GPIO_INTERRUPT_STATUS_27_INACTIVE               0
#define GPIO_INTERRUPT_STATUS_27_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_27_RESET                  1
#define GPIO_INTERRUPT_STATUS_26                        17:17
#define GPIO_INTERRUPT_STATUS_26_INACTIVE               0
#define GPIO_INTERRUPT_STATUS_26_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_26_RESET                  1
#define GPIO_INTERRUPT_STATUS_25                        16:16
#define GPIO_INTERRUPT_STATUS_25_INACTIVE               0
#define GPIO_INTERRUPT_STATUS_25_ACTIVE                 1
#define GPIO_INTERRUPT_STATUS_25_RESET                  1


#define GPIO_MUX_I2C0                                 31:30
#define GPIO_MUX_I2C0_DISABLE                         0
#define GPIO_MUX_I2C0_ENABLE                          3
#define GPIO_MUX_31                                   31:31
#define GPIO_MUX_31_GPIO                              0
#define GPIO_MUX_31_I2C0                              1
#define GPIO_MUX_30                                   30:30
#define GPIO_MUX_30_GPIO                              0
#define GPIO_MUX_30_I2C0                              1
#define GPIO_MUX_29                                   29:29
#define GPIO_MUX_29_GPIO                              0
#define GPIO_MUX_29_SSP1                              1
#define GPIO_MUX_28                                   28:28
#define GPIO_MUX_28_GPIO                              0
#define GPIO_MUX_28_SSP1                              1
#define GPIO_MUX_28_I2S_RX                            1
#define GPIO_MUX_27                                   27:27
#define GPIO_MUX_27_GPIO                              0
#define GPIO_MUX_27_I2S_TX                            0
#define GPIO_MUX_27_SSP1                              1
#define GPIO_MUX_26                                   26:26
#define GPIO_MUX_26_GPIO                              0
#define GPIO_MUX_26_I2S_WS                            0
#define GPIO_MUX_26_SSP1                              1
#define GPIO_MUX_25                                   25:25
#define GPIO_MUX_25_GPIO                              0
#define GPIO_MUX_25_I2S_CK                            0
#define GPIO_MUX_25_SSP1                              1
#define GPIO_MUX_24                                   24:24
#define GPIO_MUX_24_GPIO                              0
#define GPIO_MUX_24_SSP0                              1
#define GPIO_MUX_23                                   23:23
#define GPIO_MUX_23_GPIO                              0
#define GPIO_MUX_23_SSP0                              1
#define GPIO_MUX_22                                   22:22
#define GPIO_MUX_22_GPIO                              0
#define GPIO_MUX_22_SSP0                              1
#define GPIO_MUX_21                                   21:21
#define GPIO_MUX_21_GPIO                              0
#define GPIO_MUX_21_SSP0                              1
#define GPIO_MUX_20                                   20:20
#define GPIO_MUX_20_GPIO                              0
#define GPIO_MUX_20_SSP0                              1
#define GPIO_MUX_19                                   19:19
#define GPIO_MUX_19_GPIO                              0
#define GPIO_MUX_19_PWM2                              1
#define GPIO_MUX_18                                   18:18
#define GPIO_MUX_18_GPIO                              0
#define GPIO_MUX_18_PWM1                              1
#define GPIO_MUX_17                                   17:17
#define GPIO_MUX_17_GPIO                              0
#define GPIO_MUX_17_PWM0                              1
#define GPIO_MUX_16                                   16:16
#define GPIO_MUX_16_GPIO                              0
#define GPIO_MUX_16_UNUSED                            1
#define GPIO_MUX_UART0                                15:12
#define GPIO_MUX_UART0_DISABLE                        0
#define GPIO_MUX_UART0_ENABLE                         15
#define GPIO_MUX_15                                   15:15
#define GPIO_MUX_15_GPIO                              0
#define GPIO_MUX_15_UART0                             1
#define GPIO_MUX_14                                   14:14
#define GPIO_MUX_14_GPIO		                      0
#define GPIO_MUX_14_UART0	                          1
#define GPIO_MUX_13                                   13:13
#define GPIO_MUX_13_GPIO		                      0
#define GPIO_MUX_13_UART0                             1
#define GPIO_MUX_12                                   12:12
#define GPIO_MUX_12_GPIO		                      0
#define GPIO_MUX_12_UART0                             1
#define GPIO_MUX_UART1                                11:8
#define GPIO_MUX_UART1_DISABLE                        0
#define GPIO_MUX_UART1_ENABLE                         15
#define GPIO_MUX_11                                   11:11
#define GPIO_MUX_11_GPIO                              0
#define GPIO_MUX_11_UART1                             1
#define GPIO_MUX_10                                   10:10
#define GPIO_MUX_10_GPIO		                      0
#define GPIO_MUX_10_UART1	                          1
#define GPIO_MUX_9                                    9:9
#define GPIO_MUX_9_GPIO			                      0
#define GPIO_MUX_9_UART1                              1
#define GPIO_MUX_8                                    8:8
#define GPIO_MUX_8_GPIO			                      0
#define GPIO_MUX_8_UART1                              1
#define GPIO_MUX_I2C1                                 7:6
#define GPIO_MUX_I2C1_DISABLE                         0
#define GPIO_MUX_I2C1_ENABLE                          3
#define GPIO_MUX_7                                    7:7
#define GPIO_MUX_7_GPIO			                      0
#define GPIO_MUX_7_I2C1                               1
#define GPIO_MUX_6                                    6:6
#define GPIO_MUX_6_GPIO			                      0
#define GPIO_MUX_6_I2C1                               1
#define GPIO_MUX_5                                    5:5
#define GPIO_MUX_5_GPIO			                      0
#define GPIO_MUX_5_RESERVE                            1
#define GPIO_MUX_4                                    4:4
#define GPIO_MUX_4_GPIO27		                      0
#define GPIO_MUX_4_I2S_TX                             1
#define GPIO_MUX_3                                    3:3
#define GPIO_MUX_3_GPIO26		                      0
#define GPIO_MUX_3_I2S_WS                             1
#define GPIO_MUX_2                                    2:2
#define GPIO_MUX_2_GPIO25		                      0
#define GPIO_MUX_2_I2S_CK                             1
#define GPIO_MUX_1                                    1:1
#define GPIO_MUX_1_UART1_INT_CLK                      0
#define GPIO_MUX_1_UART1_EXT_CLK                      1
#define GPIO_MUX_0                                    0:0
#define GPIO_MUX_0_UART0_INT_CLK                      0
#define GPIO_MUX_0_UART0_EXT_CLK                      1


//#include "regpwm.h"

#define PWM_CONTROL                                               0x010020
#define PWM_CONTROL_HIGH_COUNTER                                  31:20
#define PWM_CONTROL_LOW_COUNTER                                   19:8
#define PWM_CONTROL_CLOCK_DIVIDE                                  7:4
#define PWM_CONTROL_CLOCK_DIVIDE_1                                0
#define PWM_CONTROL_CLOCK_DIVIDE_2                                1
#define PWM_CONTROL_CLOCK_DIVIDE_4                                2
#define PWM_CONTROL_CLOCK_DIVIDE_8                                3
#define PWM_CONTROL_CLOCK_DIVIDE_16                               4
#define PWM_CONTROL_CLOCK_DIVIDE_32                               5
#define PWM_CONTROL_CLOCK_DIVIDE_64                               6
#define PWM_CONTROL_CLOCK_DIVIDE_128                              7
#define PWM_CONTROL_CLOCK_DIVIDE_256                              8
#define PWM_CONTROL_CLOCK_DIVIDE_512                              9
#define PWM_CONTROL_CLOCK_DIVIDE_1024                             10
#define PWM_CONTROL_CLOCK_DIVIDE_2048                             11
#define PWM_CONTROL_CLOCK_DIVIDE_4096                             12
#define PWM_CONTROL_CLOCK_DIVIDE_8192                             13
#define PWM_CONTROL_CLOCK_DIVIDE_16384                            14
#define PWM_CONTROL_CLOCK_DIVIDE_32768                            15
#define PWM_CONTROL_INTERRUPT_STATUS                              3:3
#define PWM_CONTROL_INTERRUPT_STATUS_NOT_PENDING                  0
#define PWM_CONTROL_INTERRUPT_STATUS_PENDING                      1
#define PWM_CONTROL_INTERRUPT_STATUS_CLEAR                        1
#define PWM_CONTROL_INTERRUPT                                     2:2
#define PWM_CONTROL_INTERRUPT_DISABLE                             0
#define PWM_CONTROL_INTERRUPT_ENABLE                              1
#define PWM_CONTROL_STATUS                                        0:0
#define PWM_CONTROL_STATUS_DISABLE                                0
#define PWM_CONTROL_STATUS_ENABLE                                 1


//#include "regssp.h"

#define SSP_0_CONTROL_0                                 0x020000
#define SSP_0_CONTROL_0_CLOCK_RATE                      15:8
#define SSP_0_CONTROL_0_SCLKOUT_PHASE                   7:7
#define SSP_0_CONTROL_0_SCLKOUT_PHASE_0                 0
#define SSP_0_CONTROL_0_SCLKOUT_PHASE_1                 1
#define SSP_0_CONTROL_0_SCLKOUT_POLARITY                6:6
#define SSP_0_CONTROL_0_SCLKOUT_POLARITY_RISING         0
#define SSP_0_CONTROL_0_SCLKOUT_POLARITY_FALLING        1
#define SSP_0_CONTROL_0_FRAME_FORMAT                    5:4
#define SSP_0_CONTROL_0_FRAME_FORMAT_MOTOROLA           0
#define SSP_0_CONTROL_0_FRAME_FORMAT_TI                 1
#define SSP_0_CONTROL_0_FRAME_FORMAT_NATIONAL           2
#define SSP_0_CONTROL_0_DATA_SIZE                       3:0
#define SSP_0_CONTROL_0_DATA_SIZE_4                     3
#define SSP_0_CONTROL_0_DATA_SIZE_5                     4
#define SSP_0_CONTROL_0_DATA_SIZE_6                     5
#define SSP_0_CONTROL_0_DATA_SIZE_7                     6
#define SSP_0_CONTROL_0_DATA_SIZE_8                     7
#define SSP_0_CONTROL_0_DATA_SIZE_9                     8
#define SSP_0_CONTROL_0_DATA_SIZE_10                    9
#define SSP_0_CONTROL_0_DATA_SIZE_11                    10
#define SSP_0_CONTROL_0_DATA_SIZE_12                    11
#define SSP_0_CONTROL_0_DATA_SIZE_13                    12
#define SSP_0_CONTROL_0_DATA_SIZE_14                    13
#define SSP_0_CONTROL_0_DATA_SIZE_15                    14
#define SSP_0_CONTROL_0_DATA_SIZE_16                    15

#define SSP_0_CONTROL_1                                 0x020004
#define SSP_0_CONTROL_1_SLAVE_OUTPUT                    6:6
#define SSP_0_CONTROL_1_SLAVE_OUTPUT_ENABLE             0
#define SSP_0_CONTROL_1_SLAVE_OUTPUT_DISABLE            1
#define SSP_0_CONTROL_1_MODE_SELECT                     5:5
#define SSP_0_CONTROL_1_MODE_SELECT_MASTER              0
#define SSP_0_CONTROL_1_MODE_SELECT_SLAVE               1
#define SSP_0_CONTROL_1_STATUS                          4:4
#define SSP_0_CONTROL_1_STATUS_DISABLE                  0
#define SSP_0_CONTROL_1_STATUS_ENABLE                   1
#define SSP_0_CONTROL_1_LOOP_BACK                       3:3
#define SSP_0_CONTROL_1_LOOP_BACK_DISABLE               0
#define SSP_0_CONTROL_1_LOOP_BACK_ENABLE                1
#define SSP_0_CONTROL_1_OVERRUN_INTERRUPT               2:2
#define SSP_0_CONTROL_1_OVERRUN_INTERRUPT_DISABLE       0
#define SSP_0_CONTROL_1_OVERRUN_INTERRUPT_ENABLE        1
#define SSP_0_CONTROL_1_TRANSMIT_INTERRUPT              1:1
#define SSP_0_CONTROL_1_TRANSMIT_INTERRUPT_DISABLE      0
#define SSP_0_CONTROL_1_TRANSMIT_INTERRUPT_ENABLE       1
#define SSP_0_CONTROL_1_RECEIVE_INTERRUPT               0:0
#define SSP_0_CONTROL_1_RECEIVE_INTERRUPT_DISABLE       0
#define SSP_0_CONTROL_1_RECEIVE_INTERRUPT_ENABLE        1

#define SSP_0_DATA                                      0x020008
#define SSP_0_DATA_DATA                                 15:0

#define SSP_0_STATUS                                    0x02000C
#define SSP_0_STATUS_STATUS                             4:4
#define SSP_0_STATUS_STATUS_IDLE                        0
#define SSP_0_STATUS_STATUS_BUSY                        1
#define SSP_0_STATUS_RECEIVE_FIFO                       3:2
#define SSP_0_STATUS_RECEIVE_FIFO_EMPTY                 0
#define SSP_0_STATUS_RECEIVE_FIFO_NOT_EMPTY             1
#define SSP_0_STATUS_RECEIVE_FIFO_FULL                  3
#define SSP_0_STATUS_TRANSMIT_FIFO                      1:0
#define SSP_0_STATUS_TRANSMIT_FIFO_FULL                 0
#define SSP_0_STATUS_TRANSMIT_FIFO_NOT_FULL             2
#define SSP_0_STATUS_TRANSMIT_FIFO_EMPTY                3

#define SSP_0_CLOCK_PRESCALE                            0x020010
#define SSP_0_CLOCK_PRESCALE_DIVISOR                    7:0

#define SSP_0_INTERRUPT_STATUS                          0x020014
#define SSP_0_INTERRUPT_STATUS_OVERRUN                  2:2
#define SSP_0_INTERRUPT_STATUS_OVERRUN_NOT_ACTIVE       0
#define SSP_0_INTERRUPT_STATUS_OVERRUN_ACTIVE           1
#define SSP_0_INTERRUPT_STATUS_OVERRUN_CLEAR            1
#define SSP_0_INTERRUPT_STATUS_TRANSMIT                 1:1
#define SSP_0_INTERRUPT_STATUS_TRANSMIT_NOT_ACTIVE      0
#define SSP_0_INTERRUPT_STATUS_TRANSMIT_ACTIVE          1
#define SSP_0_INTERRUPT_STATUS_RECEIVE                  0:0
#define SSP_0_INTERRUPT_STATUS_RECEIVE_NOT_ACTIVE       0
#define SSP_0_INTERRUPT_STATUS_RECEIVE_ACTIVE           1

/* SSP 1 */

#define SSP_1_CONTROL_0                                 0x020100
#define SSP_1_CONTROL_0_CLOCK_RATE                      15:8
#define SSP_1_CONTROL_0_SCLKOUT_PHASE                   7:7
#define SSP_1_CONTROL_0_SCLKOUT_PHASE_0                 0
#define SSP_1_CONTROL_0_SCLKOUT_PHASE_1                 1
#define SSP_1_CONTROL_0_SCLKOUT_POLARITY                6:6
#define SSP_1_CONTROL_0_SCLKOUT_POLARITY_RISING         0
#define SSP_1_CONTROL_0_SCLKOUT_POLARITY_FALLING        1
#define SSP_1_CONTROL_0_FRAME_FORMAT                    5:4
#define SSP_1_CONTROL_0_FRAME_FORMAT_MOTOROLA           0
#define SSP_1_CONTROL_0_FRAME_FORMAT_TI                 1
#define SSP_1_CONTROL_0_FRAME_FORMAT_NATIONAL           2
#define SSP_1_CONTROL_0_DATA_SIZE                       3:0
#define SSP_1_CONTROL_0_DATA_SIZE_4                     3
#define SSP_1_CONTROL_0_DATA_SIZE_5                     4
#define SSP_1_CONTROL_0_DATA_SIZE_6                     5
#define SSP_1_CONTROL_0_DATA_SIZE_7                     6
#define SSP_1_CONTROL_0_DATA_SIZE_8                     7
#define SSP_1_CONTROL_0_DATA_SIZE_9                     8
#define SSP_1_CONTROL_0_DATA_SIZE_10                    9
#define SSP_1_CONTROL_0_DATA_SIZE_11                    10
#define SSP_1_CONTROL_0_DATA_SIZE_12                    11
#define SSP_1_CONTROL_0_DATA_SIZE_13                    12
#define SSP_1_CONTROL_0_DATA_SIZE_14                    13
#define SSP_1_CONTROL_0_DATA_SIZE_15                    14
#define SSP_1_CONTROL_0_DATA_SIZE_16                    15

#define SSP_1_CONTROL_1                                 0x020104
#define SSP_1_CONTROL_1_SLAVE_OUTPUT                    6:6
#define SSP_1_CONTROL_1_SLAVE_OUTPUT_ENABLE             0
#define SSP_1_CONTROL_1_SLAVE_OUTPUT_DISABLE            1
#define SSP_1_CONTROL_1_MODE_SELECT                     5:5
#define SSP_1_CONTROL_1_MODE_SELECT_MASTER              0
#define SSP_1_CONTROL_1_MODE_SELECT_SLAVE               1
#define SSP_1_CONTROL_1_STATUS                          4:4
#define SSP_1_CONTROL_1_STATUS_DISABLE                  0
#define SSP_1_CONTROL_1_STATUS_ENABLE                   1
#define SSP_1_CONTROL_1_LOOP_BACK                       3:3
#define SSP_1_CONTROL_1_LOOP_BACK_DISABLE               0
#define SSP_1_CONTROL_1_LOOP_BACK_ENABLE                1
#define SSP_1_CONTROL_1_OVERRUN_INTERRUPT               2:2
#define SSP_1_CONTROL_1_OVERRUN_INTERRUPT_DISABLE       0
#define SSP_1_CONTROL_1_OVERRUN_INTERRUPT_ENABLE        1
#define SSP_1_CONTROL_1_TRANSMIT_INTERRUPT              1:1
#define SSP_1_CONTROL_1_TRANSMIT_INTERRUPT_DISABLE      0
#define SSP_1_CONTROL_1_TRANSMIT_INTERRUPT_ENABLE       1
#define SSP_1_CONTROL_1_RECEIVE_INTERRUPT               0:0
#define SSP_1_CONTROL_1_RECEIVE_INTERRUPT_DISABLE       0
#define SSP_1_CONTROL_1_RECEIVE_INTERRUPT_ENABLE        1

#define SSP_1_DATA                                      0x020108
#define SSP_1_DATA_DATA                                 15:0

#define SSP_1_STATUS                                    0x02010C
#define SSP_1_STATUS_STATUS                             4:4
#define SSP_1_STATUS_STATUS_IDLE                        0
#define SSP_1_STATUS_STATUS_BUSY                        1
#define SSP_1_STATUS_RECEIVE_FIFO                       3:2
#define SSP_1_STATUS_RECEIVE_FIFO_EMPTY                 0
#define SSP_1_STATUS_RECEIVE_FIFO_NOT_EMPTY             1
#define SSP_1_STATUS_RECEIVE_FIFO_FULL                  3
#define SSP_1_STATUS_TRANSMIT_FIFO                      1:0
#define SSP_1_STATUS_TRANSMIT_FIFO_FULL                 0
#define SSP_1_STATUS_TRANSMIT_FIFO_NOT_FULL             2
#define SSP_1_STATUS_TRANSMIT_FIFO_EMPTY                3

#define SSP_1_CLOCK_PRESCALE                            0x020110
#define SSP_1_CLOCK_PRESCALE_DIVISOR                    7:0

#define SSP_1_INTERRUPT_STATUS                          0x020114
#define SSP_1_INTERRUPT_STATUS_OVERRUN                  2:2
#define SSP_1_INTERRUPT_STATUS_OVERRUN_NOT_ACTIVE       0
#define SSP_1_INTERRUPT_STATUS_OVERRUN_ACTIVE           1
#define SSP_1_INTERRUPT_STATUS_OVERRUN_CLEAR            1
#define SSP_1_INTERRUPT_STATUS_TRANSMIT                 1:1
#define SSP_1_INTERRUPT_STATUS_TRANSMIT_NOT_ACTIVE      0
#define SSP_1_INTERRUPT_STATUS_TRANSMIT_ACTIVE          1
#define SSP_1_INTERRUPT_STATUS_RECEIVE                  0:0
#define SSP_1_INTERRUPT_STATUS_RECEIVE_NOT_ACTIVE       0
#define SSP_1_INTERRUPT_STATUS_RECEIVE_ACTIVE           1

//#include "regdc.h"


#define HDMI_CONTROL									0x0800C4
#define HDMI_CONTROL_MODE_SELECT						7:4
#define HDMI_CONTROL_MODE_SELECT_A						1
#define HDMI_CONTROL_MODE_SELECT_B						2
#define HDMI_CONTROL_MODE_SELECT_D						4
#define HDMI_CONTROL_MODE_SELECT_E						8
#define HDMI_CONTROL_PLLB								3:3
#define HDMI_CONTROL_PLLB_NORMAL						0
#define HDMI_CONTROL_PLLB_RESET							1
#define HDMI_CONTROL_PLLA								2:2
#define HDMI_CONTROL_PLLA_NORMAL						0
#define HDMI_CONTROL_PLLA_RESET							1
#define HDMI_CONTROL_INTMODE							1:1
#define HDMI_CONTROL_INT_MODE_OPEN						0
#define HDMI_CONTROL_INT_MODE_PULL						1
#define HDMI_CONTROL_INT_POLARITY						0:0
#define HDMI_CONTROL_INT_POLARITY_LOW					0
#define HDMI_CONTROL_INT_POLARITY_HIGH					1

#define HDMI_CONFIG										0x0800C0
#define HDMI_CONFIG_READ								17:17
#define HDMI_CONFIG_READ_LATCH							0
#define HDMI_CONFIG_READ_ENABLE							1
#define HDMI_CONFIG_WRITE								16:16
#define HDMI_CONFIG_WRITE_LATCH							0
#define HDMI_CONFIG_WRITE_ENABLE						1
#define HDMI_CONFIG_DATA								15:8
#define HDMI_CONFIG_ADDRESS								7:0




#define DISPLAY_CTRL                            0x080000
#define DISPLAY_CTRL_DPMS                         31:30
#define DISPLAY_CTRL_DPMS_VPHP                       0
#define DISPLAY_CTRL_DPMS_VPHN                       1
#define DISPLAY_CTRL_DPMS_VNHP                       2
#define DISPLAY_CTRL_DPMS_VNHN                       3
#define DISPLAY_CTRL_DATA_PATH                 29:29
#define DISPLAY_CTRL_DATA_PATH_VGA    	         0
#define DISPLAY_CTRL_DATA_PATH_EXTENDED         1
#define DISPLAY_CTRL_DIRECTION                   28:28
#define DISPLAY_CTRL_DIRECTION_INPUT             	 0
#define DISPLAY_CTRL_DIRECTION_OUTPUT                1
#define DISPLAY_CTRL_FPEN                       27:27
#define DISPLAY_CTRL_FPEN_LOW                   0
#define DISPLAY_CTRL_FPEN_HIGH                  1
#define DISPLAY_CTRL_VBIASEN                    26:26
#define DISPLAY_CTRL_VBIASEN_LOW                0
#define DISPLAY_CTRL_VBIASEN_HIGH               1
#define DISPLAY_CTRL_DATA                       25:25
#define DISPLAY_CTRL_DATA_DISABLE               0
#define DISPLAY_CTRL_DATA_ENABLE                1
#define DISPLAY_CTRL_FPVDDEN                    24:24
#define DISPLAY_CTRL_FPVDDEN_LOW                0
#define DISPLAY_CTRL_FPVDDEN_HIGH               1
#define DISPLAY_CTRL_CAPTURE_DATA_SELECT	                    23:23
#define DISPLAY_CTRL_CAPTURE_DATA_SELECT_CHANNEL0              0
#define DISPLAY_CTRL_CAPTURE_DATA_SELECT_CHANNEL1              1
#define DISPLAY_CTRL_LOOP_BACK_SELECT              			22:22
#define DISPLAY_CTRL_LOOP_BACK_SELECT_CHANNEL0        			0
#define DISPLAY_CTRL_LOOP_BACK_SELECT_CHANNEL1        			1
#define DISPLAY_CTRL_CHANNEL_OUTPUT_FORMAT         			21:20
#define DISPLAY_CTRL_CHANNEL_OUTPUT_FORMAT_CHANNEL0_24BIT      0
#define DISPLAY_CTRL_CHANNEL_OUTPUT_FORMAT_CHANNEL1_24BIT      1
#define DISPLAY_CTRL_CHANNEL_OUTPUT_FORMAT_CHANNEL0_48BIT      2
#define DISPLAY_CTRL_CHANNEL_OUTPUT_FORMAT_CHANNEL1_48BIT      3
#define DISPLAY_CTRL_HDMI_CLK                   19:19
#define DISPLAY_CTRL_HDMI_CLK_NEG               0
#define DISPLAY_CTRL_HDMI_CLK_POS               1
#define DISPLAY_CTRL_HDMI_SELECT				18:18
#define DISPLAY_CTRL_HDMI_SELECT_CHANNEL0		0
#define DISPLAY_CTRL_HDMI_SELECT_CHANNEL1		1
#define DISPLAY_CTRL_LVDS_OUTPUT_FORMAT						17:16
#define DISPLAY_CTRL_LVDS_OUTPUT_FORMAT_CHANNEL0_24BIT		0
#define DISPLAY_CTRL_LVDS_OUTPUT_FORMAT_CHANNEL1_24BIT		1
#define DISPLAY_CTRL_LVDS_OUTPUT_FORMAT_CHANNEL0_48BIT		2
#define DISPLAY_CTRL_LVDS_OUTPUT_FORMAT_CHANNEL1_48BIT		3
#define DISPLAY_CTRL_PIXEL_CLOCK_SELECT         15:15
#define DISPLAY_CTRL_PIXEL_CLOCK_SELECT_SINGLE  0
#define DISPLAY_CTRL_PIXEL_CLOCK_SELECT_HALF    1
#define DISPLAY_CTRL_CLOCK_PHASE                14:14
#define DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH    0
#define DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_LOW     1
#define DISPLAY_CTRL_VSYNC_PHASE                13:13
#define DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH    0
#define DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_LOW     1
#define DISPLAY_CTRL_HSYNC_PHASE                12:12
#define DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH    0
#define DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_LOW     1
#define DISPLAY_CTRL_DOUBLE_PIXEL_CLOCK         11:11
#define DISPLAY_CTRL_DOUBLE_PIXEL_CLOCK_DISABLE 0
#define DISPLAY_CTRL_DOUBLE_PIXEL_CLOCK_ENABLE  1
#define DISPLAY_CTRL_VSYNC						11:11
#define DISPLAY_CTRL_VSYNC_INACTIVE				0
#define DISPLAY_CTRL_VSYNC_ACTIVE				1
#define DISPLAY_CTRL_DAC_FORMAT	             10:10
#define DISPLAY_CTRL_DAC_FORMAT_6BIT		     0
#define DISPLAY_CTRL_DAC_FORMAT_8BIT		     1
#define DISPLAY_CTRL_COLOR_KEY                  9:9
#define DISPLAY_CTRL_COLOR_KEY_DISABLE          0
#define DISPLAY_CTRL_COLOR_KEY_ENABLE           1
#define DISPLAY_CTRL_TIMING                     8:8
#define DISPLAY_CTRL_TIMING_DISABLE             0
#define DISPLAY_CTRL_TIMING_ENABLE              1
#define DISPLAY_CTRL_PIXEL                      7:4
#define DISPLAY_CTRL_GAMMA                      3:3
#define DISPLAY_CTRL_GAMMA_DISABLE              0
#define DISPLAY_CTRL_GAMMA_ENABLE               1
#define DISPLAY_CTRL_PLANE                      2:2
#define DISPLAY_CTRL_PLANE_DISABLE              0
#define DISPLAY_CTRL_PLANE_ENABLE               1
#define DISPLAY_CTRL_FORMAT                     1:0
#define DISPLAY_CTRL_FORMAT_8                   0
#define DISPLAY_CTRL_FORMAT_16                  1
#define DISPLAY_CTRL_FORMAT_32                  2

#define FB_ADDRESS                              0x080004
#define FB_ADDRESS_STATUS                       31:31
#define FB_ADDRESS_STATUS_CURRENT               0
#define FB_ADDRESS_STATUS_PENDING               1
#define FB_ADDRESS_ADDRESS                      29:0

#define CHANNEL1_FB_ADDRESS   					0x088004
#define CHANNEL1_FB_ADDRESS_STATUS                       31:31
#define CHANNEL1_FB_ADDRESS_STATUS_CURRENT               0
#define CHANNEL1_FB_ADDRESS_STATUS_PENDING               1
#define CHANNEL1_FB_ADDRESS_ADDRESS                      29:0



#define FB_WIDTH                                0x080008
#define FB_WIDTH_WIDTH                          29:16
#define FB_WIDTH_OFFSET                         13:0

#define HORIZONTAL_TOTAL                        0x08000C
#define HORIZONTAL_TOTAL_TOTAL                  28:16
#define HORIZONTAL_TOTAL_DISPLAY_END            11:0

#define HORIZONTAL_SYNC                         0x080010
#define HORIZONTAL_SYNC_WIDTH                   25:16
#define HORIZONTAL_SYNC_START                   12:0

#define VERTICAL_TOTAL                          0x080014
#define VERTICAL_TOTAL_TOTAL                    27:16
#define VERTICAL_TOTAL_DISPLAY_END              11:0

#define VERTICAL_SYNC                           0x080018
#define VERTICAL_SYNC_HEIGHT                    21:16
#define VERTICAL_SYNC_START                     11:0

#define CURRENT_LINE                            0x080020
#define CURRENT_LINE_LINE                       11:0

#define LVDS_CTRL1				0x080020
#define LVDS_CTRL1_CLKSEL_PLL2			31:31
#define LVDS_CTRL1_CLKSEL_PLL2_RE		0
#define LVDS_CTRL1_CLKSEL_PLL2_FE		1
#define LVDS_CTRL1_CLKSEL_PLL1			30:30
#define LVDS_CTRL1_CLKSEL_PLL1_RE		0
#define LVDS_CTRL1_CLKSEL_PLL1_FE		1
#define LVDS_CTRL1_DCLK2			29:23
#define LVDS_CTRL1_DCLK2_DEFAULT		0x63
#define LVDS_CTRL1_DCLK1			22:16
#define LVDS_CTRL1_DCLK1_DEFAULT		0x63
#define LVDS_CTRL1_PLL2_LOCK			15:15
#define LVDS_CTRL1_PLL1_LOCK			14:14
#define LVDS_CTRL1_POR25			13:13
#define LVDS_CTRL1_POR12			12:12

#define CRT_DETECT                     0x080024
#define CRT_DETECT_DAC				   31:31
#define CRT_DETECT_DAC_ENABLE		   0
#define CRT_DETECT_DAC_DOWN			   1
#define CRT_DETECT_CRT_CLK             27:27
#define CRT_DETECT_CRT_CLK_POS         0
#define CRT_DETECT_CRT_CLK_NEG         1   
#define CRT_DETECT_LVDS_CLK            26:26
#define CRT_DETECT_LVDS_CLK_POS		   0
#define CRT_DETECT_LVDS_CLK_NEG		   1
#define CRT_DETECT_CRT                 25:25
#define CRT_DETECT_CRT_ABSENT          0
#define CRT_DETECT_CRT_PRESENT         1
#define CRT_DETECT_ENABLE              24:24
#define CRT_DETECT_ENABLE_DISABLE      0
#define CRT_DETECT_ENABLE_ENABLE       1
#define CRT_DETECT_DATA_RED            23:16
#define CRT_DETECT_DATA_GREEN          15:8
#define CRT_DETECT_DATA_BLUE           7:0

#define COLOR_KEY                               0x080028
#define COLOR_KEY_MASK                          31:16
#define COLOR_KEY_VALUE                         15:0


#define LVDS_CTRL2					0x08002C
#define LVDS_CTRL2_SHTDNB2				30:30
#define LVDS_CTRL2_SHTDNB2_RESET			0
#define LVDS_CTRL2_SHTDNB2_NORMAL			1
#define LVDS_CTRL2_SHTDNB1				29:29
#define LVDS_CTRL2_SHTDNB1_RESET			0
#define LVDS_CTRL2_SHTDNB1_NORMAL			1
#define LVDS_CTRL2_CLK2_DS 				28:27
#define LVDS_CTRL2_CLK2_DS_3MA				0
#define LVDS_CTRL2_CLK2_DS_1MA				1
#define LVDS_CTRL2_CLK2_DS_5MA				2
#define LVDS_CTRL2_CLK2_DS_2MA				3
#define LVDS_CTRL2_CLK1_DS 				26:25
#define LVDS_CTRL2_CLK1_DS_3MA				0
#define LVDS_CTRL2_CLK1_DS_1MA				1
#define LVDS_CTRL2_CLK1_DS_5MA				2
#define LVDS_CTRL2_CLK1_DS_2MA				3
#define LVDS_CTRL2_DS					24:23
#define LVDS_CTRL2_DS_3MA				0
#define LVDS_CTRL2_DS_1MA				1
#define LVDS_CTRL2_DS_5MA				2
#define LVDS_CTRL2_DS_2MA				3
#define LVDS_CTRL2_CLK2_TR				22:22
#define LVDS_CTRL2_CLK2_TR_0				0
#define LVDS_CTRL2_CLK2_TR_100				1
#define LVDS_CTRL2_CLK1_TR				21:21
#define LVDS_CTRL2_CLK1_TR_0				0
#define LVDS_CTRL2_CLK1_TR_100				1
#define LVDS_CTRL2_TR					20:20
#define LVDS_CTRL2_TR_0					0
#define LVDS_CTRL2_TR_100				1
#define LVDS_CTRL2_CLK2_COMP				19:18
#define LVDS_CTRL2_CLK2_COMP_0				0
#define LVDS_CTRL2_CLK2_COMP_1				1
#define LVDS_CTRL2_CLK2_COMP_2				2
#define LVDS_CTRL2_CLK2_COMP_3				3
#define LVDS_CTRL2_CLK1_COMP				17:16
#define LVDS_CTRL2_CLK1_COMP_0				0
#define LVDS_CTRL2_CLK1_COMP_1				1
#define LVDS_CTRL2_CLK1_COMP_2				2
#define LVDS_CTRL2_CLK1_COMP_3				3
#define LVDS_CTRL2_PRE_COMP				15:14
#define LVDS_CTRL2_PRE_COMP_0				0
#define LVDS_CTRL2_PRE_COMP_1				1
#define LVDS_CTRL2_PRE_COMP_2				2
#define LVDS_CTRL2_PRE_COMP_3				3
#define LVDS_CTRL2_VCOS_PLL2				13:11
#define LVDS_CTRL2_VCOS_PLL2_10M			0
#define LVDS_CTRL2_VCOS_PLL2_20M			1
#define LVDS_CTRL2_VCOS_PLL2_40M			2
#define LVDS_CTRL2_VCOS_PLL2_80M			3
#define LVDS_CTRL2_VCOS_PLL2_160M			4
#define LVDS_CTRL2_VCOS_PLL2_350M			5
#define LVDS_CTRL2_VCOS_PLL1				10:8
#define LVDS_CTRL2_VCOS_PLL1_10M			0
#define LVDS_CTRL2_VCOS_PLL1_20M			1
#define LVDS_CTRL2_VCOS_PLL1_40M			2
#define LVDS_CTRL2_VCOS_PLL1_80M			3
#define LVDS_CTRL2_VCOS_PLL1_160M			4
#define LVDS_CTRL2_VCOS_PLL1_350M			5
#define LVDS_CTRL2_SHORTS_PLL2				7:6
#define LVDS_CTRL2_SHORTS_PLL2_2048			0
#define LVDS_CTRL2_SHORTS_PLL2_1024			1
#define LVDS_CTRL2_SHORTS_PLL2_512			2
#define LVDS_CTRL2_SHORTS_PLL2_256			3
#define LVDS_CTRL2_SHORTS_PLL1				5:4
#define LVDS_CTRL2_SHORTS_PLL1_2048			0
#define LVDS_CTRL2_SHORTS_PLL1_1024			1
#define LVDS_CTRL2_SHORTS_PLL1_512			2
#define LVDS_CTRL2_SHORTS_PLL1_256			3
#define LVDS_CTRL2_PD_PLL2				3:3
#define LVDS_CTRL2_PD_PLL2_NORMAL			0
#define LVDS_CTRL2_PD_PLL2_DOWN				1
#define LVDS_CTRL2_PD_PLL1				2:2
#define LVDS_CTRL2_PD_PLL1_NORMAL			0
#define LVDS_CTRL2_PD_PLL1_DOWN				1
#define LVDS_CTRL2_MODESEL2				1:1
#define LVDS_CTRL2_MODESEL2_DC0				0
#define LVDS_CTRL2_MODESEL2_DC1				1
#define LVDS_CTRL2_MODESEL1				0:0
#define LVDS_CTRL2_MODESEL1_DC0				0
#define LVDS_CTRL2_MODESEL1_DC1				1

/* Cursor Control */
#define HWC_CONTROL                             0x080030
#define HWC_CONTROL_MODE                        31:30
#define HWC_CONTROL_MODE_DISABLE                0
#define HWC_CONTROL_MODE_MASK                   1
#define HWC_CONTROL_MODE_MONO                   2
#define HWC_CONTROL_MODE_ALPHA                  3
#define HWC_CONTROL_ADDRESS                     29:0

#define HWC_LOCATION                            0x080034
#define HWC_LOCATION_TOP                        31:31
#define HWC_LOCATION_TOP_INSIDE                 0
#define HWC_LOCATION_TOP_OUTSIDE                1
#define HWC_LOCATION_Y                          27:16
#define HWC_LOCATION_LEFT                       15:15
#define HWC_LOCATION_LEFT_INSIDE                0
#define HWC_LOCATION_LEFT_OUTSIDE               1
#define HWC_LOCATION_X                          11:0

#define HWC_COLOR0                              0x080038
#define HWC_COLOR0_RGB888                       23:0

#define HWC_COLOR1                              0x08003C
#define HWC_COLOR1_RGB888                       23:0


/* Channel0 Cursor Control */
#define CHANNEL0_HWC_ADDRESS                             0x080030
#define CHANNEL0_HWC_ADDRESS_ENABLE                      31:30
#define CHANNEL0_HWC_ADDRESS_ENABLE_DISABLE              0
#define CHANNEL0_HWC_ADDRESS_ENABLE_MASK	             1
#define CHANNEL0_HWC_ADDRESS_ENABLE_MONO	             2
#define CHANNEL0_HWC_ADDRESS_ENABLE_ALPHA	             3
#define CHANNEL0_HWC_ADDRESS_ADDRESS                     29:0

#define CHANNEL0_HWC_LOCATION                            0x080034
#define CHANNEL0_HWC_LOCATION_TOP                        31:31
#define CHANNEL0_HWC_LOCATION_TOP_INSIDE                 0
#define CHANNEL0_HWC_LOCATION_TOP_OUTSIDE                1
#define CHANNEL0_HWC_LOCATION_Y                          27:16
#define CHANNEL0_HWC_LOCATION_LEFT                       15:15
#define CHANNEL0_HWC_LOCATION_LEFT_INSIDE                0
#define CHANNEL0_HWC_LOCATION_LEFT_OUTSIDE               1
#define CHANNEL0_HWC_LOCATION_X                          11:0

#define CHANNEL0_HWC_COLOR_0                            0x080038
#define CHANNEL0_HWC_COLOR_0_RGB888		                23:0

#define CHANNEL0_HWC_COLOR_1                            0x08003C
#define CHANNEL0_HWC_COLOR_1_RGB888                   	23:0

/* Channel1 Cursor Control */
#define CHANNEL1_HWC_ADDRESS                             0x088030
#define CHANNEL1_HWC_ADDRESS_ENABLE                      31:30
#define CHANNEL1_HWC_ADDRESS_ENABLE_DISABLE              0
#define CHANNEL1_HWC_ADDRESS_ENABLE_MASK	             1
#define CHANNEL1_HWC_ADDRESS_ENABLE_MONO	             2
#define CHANNEL1_HWC_ADDRESS_ENABLE_ALPHA	             3
#define CHANNEL1_HWC_ADDRESS_ADDRESS                     29:0

#define CHANNEL1_HWC_LOCATION                            0x088034
#define CHANNEL1_HWC_LOCATION_TOP                        31:31
#define CHANNEL1_HWC_LOCATION_TOP_INSIDE                 0
#define CHANNEL1_HWC_LOCATION_TOP_OUTSIDE                1
#define CHANNEL1_HWC_LOCATION_Y                          27:16
#define CHANNEL1_HWC_LOCATION_LEFT                       15:15
#define CHANNEL1_HWC_LOCATION_LEFT_INSIDE                0
#define CHANNEL1_HWC_LOCATION_LEFT_OUTSIDE               1
#define CHANNEL1_HWC_LOCATION_X                          11:0

#define CHANNEL1_HWC_COLOR_0                            0x088038
#define CHANNEL1_HWC_COLOR_0_RGB888		                23:0

#define CHANNEL1_HWC_COLOR_1                            0x08803C
#define CHANNEL1_HWC_COLOR_1_RGB888                   	23:0


/* Video Control */
#define VIDEO_DISPLAY_CTRL                              0x080040
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP                    12:12
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP_DISABLE            0
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP_ENABLE             1
#define VIDEO_DISPLAY_CTRL_VERTICAL_MODE                9:9
#define VIDEO_DISPLAY_CTRL_VERTICAL_MODE_REPLICATE      0
#define VIDEO_DISPLAY_CTRL_VERTICAL_MODE_INTERPOLATE    1
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_MODE              8:8
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_MODE_REPLICATE    0
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_MODE_INTERPOLATE  1
#define VIDEO_DISPLAY_CTRL_PIXEL                        7:4
#define VIDEO_DISPLAY_CTRL_GAMMA                        3:3
#define VIDEO_DISPLAY_CTRL_GAMMA_DISABLE                0
#define VIDEO_DISPLAY_CTRL_GAMMA_ENABLE                 1
#define VIDEO_DISPLAY_CTRL_PLANE                        2:2
#define VIDEO_DISPLAY_CTRL_PLANE_DISABLE                0
#define VIDEO_DISPLAY_CTRL_PLANE_ENABLE                 1
#define VIDEO_DISPLAY_CTRL_FORMAT                       1:0
#define VIDEO_DISPLAY_CTRL_FORMAT_16                    0
#define VIDEO_DISPLAY_CTRL_FORMAT_32                    1
#define VIDEO_DISPLAY_CTRL_FORMAT_YUV422                2
#define VIDEO_DISPLAY_CTRL_FORMAT_YUV420                3

#define VIDEO_FB_ADDRESS                              0x080044
#define VIDEO_FB_ADDRESS_STATUS                       31:31
#define VIDEO_FB_ADDRESS_STATUS_CURRENT               0
#define VIDEO_FB_ADDRESS_STATUS_PENDING               1
#define VIDEO_FB_ADDRESS_ADDRESS                      29:0

#define VIDEO_FB_WIDTH                                0x080048
#define VIDEO_FB_WIDTH_WIDTH                          29:16
#define VIDEO_FB_WIDTH_OFFSET                         13:0

#define VIDEO_FB_ADDRESS_Y                            0x080044
#define VIDEO_FB_ADDRESS_Y_ADDRESS                    29:0

#define VIDEO_FB_WIDTH_Y                              0x080048
#define VIDEO_FB_WIDTH_Y_WIDTH                        29:16
#define VIDEO_FB_WIDTH_Y_OFFSET                       13:0


#define VIDEO_FB_ADDRESS_U                            0x08004C
#define VIDEO_FB_ADDRESS_U_ADDRESS                    29:0

#define VIDEO_FB_WIDTH_U                              0x080050
#define VIDEO_FB_WIDTH_U_WIDTH                        29:16
#define VIDEO_FB_WIDTH_U_OFFSET                       13:0

#define VIDEO_FB_ADDRESS_V                            0x080054
#define VIDEO_FB_ADDRESS_V_ADDRESS                    29:0

#define VIDEO_FB_WIDTH_V                              0x080058
#define VIDEO_FB_WIDTH_V_WIDTH                        29:16
#define VIDEO_FB_WIDTH_V_OFFSET                       13:0

#define VIDEO_YUV_CONSTANTS                           0x08005C
#define VIDEO_YUV_CONSTANTS_Y                         31:24
#define VIDEO_YUV_CONSTANTS_R                         23:16
#define VIDEO_YUV_CONSTANTS_G                         15:8
#define VIDEO_YUV_CONSTANTS_B                         7:0

#define VIDEO_PLANE_TL                                  0x080060
#define VIDEO_PLANE_TL_TOP                              26:16
#define VIDEO_PLANE_TL_LEFT                             11:0

#define VIDEO_PLANE_BR                                  0x080064
#define VIDEO_PLANE_BR_BOTTOM                           26:16
#define VIDEO_PLANE_BR_RIGHT                            11:0

#define VIDEO_SCALE                                     0x080068
#define VIDEO_SCALE_VERTICAL_SCALE                      27:16
#define VIDEO_SCALE_HORIZONTAL_SCALE                    11:0

#define VIDEO_INITIAL_SCALE                             0x08006C
#define VIDEO_INITIAL_SCALE_VERTICAL                    27:16
#define VIDEO_INITIAL_SCALE_HORIZONTAL                     11:0

/* Video Alpha Control */
#if 1
#define VIDEO_ALPHA_DISPLAY_CTRL                        0x080080
#define VIDEO_ALPHA_DISPLAY_CTRL_SELECT                 28:28
#define VIDEO_ALPHA_DISPLAY_CTRL_SELECT_PER_PIXEL       0
#define VIDEO_ALPHA_DISPLAY_CTRL_SELECT_ALPHA           1
#define VIDEO_ALPHA_DISPLAY_CTRL_ALPHA                  27:24
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO                   17:16
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_1                 0
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_3                 1
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_7                 2
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_11                3
#define VIDEO_ALPHA_DISPLAY_CTRL_PIXEL                  7:4
#define VIDEO_ALPHA_DISPLAY_CTRL_CHROMA_KEY             3:3
#define VIDEO_ALPHA_DISPLAY_CTRL_CHROMA_KEY_DISABLE     0
#define VIDEO_ALPHA_DISPLAY_CTRL_CHROMA_KEY_ENABLE      1
#define VIDEO_ALPHA_DISPLAY_CTRL_PLANE                  2:2
#define VIDEO_ALPHA_DISPLAY_CTRL_PLANE_DISABLE          0
#define VIDEO_ALPHA_DISPLAY_CTRL_PLANE_ENABLE           1
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT                 1:0
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_8               0
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_16              1
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4       2
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4_4_4   3

#define VIDEO_ALPHA_FB_ADDRESS                          0x080084
#define VIDEO_ALPHA_FB_ADDRESS_STATUS                   31:31
#define VIDEO_ALPHA_FB_ADDRESS_STATUS_CURRENT           0
#define VIDEO_ALPHA_FB_ADDRESS_STATUS_PENDING           1
#define VIDEO_ALPHA_FB_ADDRESS_ADDRESS                  29:0

#define VIDEO_ALPHA_FB_WIDTH                            0x080088
#define VIDEO_ALPHA_FB_WIDTH_WIDTH                      29:16
#define VIDEO_ALPHA_FB_WIDTH_OFFSET                     13:0

#define VIDEO_ALPHA_PLANE_TL                            0x08008C
#define VIDEO_ALPHA_PLANE_TL_TOP                        26:16
#define VIDEO_ALPHA_PLANE_TL_LEFT                       11:0

#define VIDEO_ALPHA_PLANE_BR                            0x080090
#define VIDEO_ALPHA_PLANE_BR_BOTTOM                     26:16
#define VIDEO_ALPHA_PLANE_BR_RIGHT                      11:0

#define VIDEO_ALPHA_CHROMA_KEY                          0x080094
#define VIDEO_ALPHA_CHROMA_KEY_MASK                     31:16
#define VIDEO_ALPHA_CHROMA_KEY_VALUE                    15:0

#define VIDEO_ALPHA_COLOR_LOOKUP_01                     0x080098
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_23                     0x08009C
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_45                     0x0800A0
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_67                     0x0800A4
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_89                     0x0800A8
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_AB                     0x0800AC
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_CD                     0x0800B0
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_EF                     0x0800B4
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_BLUE              4:0
#endif

/* Alpha Control */

#define ALPHA_DISPLAY_CTRL                              0x080080
#define ALPHA_DISPLAY_CTRL_SELECT                       28:28
#define ALPHA_DISPLAY_CTRL_SELECT_PER_PIXEL             0
#define ALPHA_DISPLAY_CTRL_SELECT_ALPHA                 1
#define ALPHA_DISPLAY_CTRL_ALPHA                        27:24
#define ALPHA_DISPLAY_CTRL_FIFO                         17:16
#define ALPHA_DISPLAY_CTRL_FIFO_1                       0
#define ALPHA_DISPLAY_CTRL_FIFO_3                       1
#define ALPHA_DISPLAY_CTRL_FIFO_7                       2
#define ALPHA_DISPLAY_CTRL_FIFO_11                      3
#define ALPHA_DISPLAY_CTRL_PIXEL                        7:4
#define ALPHA_DISPLAY_CTRL_CHROMA_KEY                   3:3
#define ALPHA_DISPLAY_CTRL_CHROMA_KEY_DISABLE           0
#define ALPHA_DISPLAY_CTRL_CHROMA_KEY_ENABLE            1
#define ALPHA_DISPLAY_CTRL_PLANE                        2:2
#define ALPHA_DISPLAY_CTRL_PLANE_DISABLE                0
#define ALPHA_DISPLAY_CTRL_PLANE_ENABLE                 1
#define ALPHA_DISPLAY_CTRL_FORMAT                       1:0
#define ALPHA_DISPLAY_CTRL_FORMAT_16                    1
#define ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4             2
#define ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4_4_4         3

#define ALPHA_FB_ADDRESS                                0x080084
#define ALPHA_FB_ADDRESS_STATUS                         31:31
#define ALPHA_FB_ADDRESS_STATUS_CURRENT                 0
#define ALPHA_FB_ADDRESS_STATUS_PENDING                 1
#define ALPHA_FB_ADDRESS_ADDRESS                        29:0

#define ALPHA_FB_WIDTH                                  0x080088
#define ALPHA_FB_WIDTH_WIDTH                            29:16
#define ALPHA_FB_WIDTH_OFFSET                           13:0

#define ALPHA_PLANE_TL                                  0x08008C
#define ALPHA_PLANE_TL_TOP                              26:16
#define ALPHA_PLANE_TL_LEFT                             10:0

#define ALPHA_PLANE_BR                                  0x080090
#define ALPHA_PLANE_BR_BOTTOM                           26:16
#define ALPHA_PLANE_BR_RIGHT                            10:0

#define ALPHA_CHROMA_KEY                                0x080094
#define ALPHA_CHROMA_KEY_MASK                           31:16
#define ALPHA_CHROMA_KEY_VALUE                          15:0

#define ALPHA_COLOR_LOOKUP_01                           0x080098
#define ALPHA_COLOR_LOOKUP_01_1                         31:16
#define ALPHA_COLOR_LOOKUP_01_1_RED                     31:27
#define ALPHA_COLOR_LOOKUP_01_1_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_01_1_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_01_0                         15:0
#define ALPHA_COLOR_LOOKUP_01_0_RED                     15:11
#define ALPHA_COLOR_LOOKUP_01_0_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_01_0_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_23                           0x08009C
#define ALPHA_COLOR_LOOKUP_23_3                         31:16
#define ALPHA_COLOR_LOOKUP_23_3_RED                     31:27
#define ALPHA_COLOR_LOOKUP_23_3_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_23_3_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_23_2                         15:0
#define ALPHA_COLOR_LOOKUP_23_2_RED                     15:11
#define ALPHA_COLOR_LOOKUP_23_2_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_23_2_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_45                           0x0800a0
#define ALPHA_COLOR_LOOKUP_45_5                         31:16
#define ALPHA_COLOR_LOOKUP_45_5_RED                     31:27
#define ALPHA_COLOR_LOOKUP_45_5_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_45_5_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_45_4                         15:0
#define ALPHA_COLOR_LOOKUP_45_4_RED                     15:11
#define ALPHA_COLOR_LOOKUP_45_4_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_45_4_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_67                           0x0800a4
#define ALPHA_COLOR_LOOKUP_67_7                         31:16
#define ALPHA_COLOR_LOOKUP_67_7_RED                     31:27
#define ALPHA_COLOR_LOOKUP_67_7_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_67_7_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_67_6                         15:0
#define ALPHA_COLOR_LOOKUP_67_6_RED                     15:11
#define ALPHA_COLOR_LOOKUP_67_6_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_67_6_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_89                           0x0800a8
#define ALPHA_COLOR_LOOKUP_89_9                         31:16
#define ALPHA_COLOR_LOOKUP_89_9_RED                     31:27
#define ALPHA_COLOR_LOOKUP_89_9_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_89_9_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_89_8                         15:0
#define ALPHA_COLOR_LOOKUP_89_8_RED                     15:11
#define ALPHA_COLOR_LOOKUP_89_8_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_89_8_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_AB                           0x0800AC
#define ALPHA_COLOR_LOOKUP_AB_B                         31:16
#define ALPHA_COLOR_LOOKUP_AB_B_RED                     31:27
#define ALPHA_COLOR_LOOKUP_AB_B_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_AB_B_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_AB_A                         15:0
#define ALPHA_COLOR_LOOKUP_AB_A_RED                     15:11
#define ALPHA_COLOR_LOOKUP_AB_A_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_AB_A_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_CD                           0x0800B0
#define ALPHA_COLOR_LOOKUP_CD_D                         31:16
#define ALPHA_COLOR_LOOKUP_CD_D_RED                     31:27
#define ALPHA_COLOR_LOOKUP_CD_D_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_CD_D_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_CD_C                         15:0
#define ALPHA_COLOR_LOOKUP_CD_C_RED                     15:11
#define ALPHA_COLOR_LOOKUP_CD_C_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_CD_C_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_EF                           0x0800B4
#define ALPHA_COLOR_LOOKUP_EF_F                         31:16
#define ALPHA_COLOR_LOOKUP_EF_F_RED                     31:27
#define ALPHA_COLOR_LOOKUP_EF_F_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_EF_F_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_EF_E                         15:0
#define ALPHA_COLOR_LOOKUP_EF_E_RED                     15:11
#define ALPHA_COLOR_LOOKUP_EF_E_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_EF_E_BLUE                    4:0

/* Distance between channel 1 and channel 2 display control */
#define CHANNEL_OFFSET 0x8000

#define DC_OFFSET 0x8000


/* Palette RAM */
/* Panel Pallete register starts at 0x080400 ~ 0x0807FC */
#define CHANNEL0_PALETTE_RAM                             0x080C00
#define PALETTE_RAM                             0x080C00

/* Panel Pallete register starts at 0x080C00 ~ 0x080FFC */
#define CHANNEL1_PALETTE_RAM                           0x088C00

//#include "regdma.h"

/* New DMA engine definition for Falcon */
/* DMA 0 is uni-directional from Serial ROM to local DDR only */
#define DMA0_DESTINATION                                0x0D0000
#define DMA0_DESTINATION_ADDRESS                        29:0

#define DMA0_SOURCE                                     0x0D0004
#define DMA0_SOURCE_ADDRESS                             23:0

#define DMA0_CONTROL                                    0x0D0008
#define DMA0_CONTROL_STATUS                             31:31
#define DMA0_CONTROL_STATUS_IDLE                        0
#define DMA0_CONTROL_STATUS_ENABLE                      1
#define DMA0_CONTROL_SIZE                               23:0

/* DMA 1 can transfer from:
   System to local DDR.
   local DDR to system.
   local DDR to local DDR.

   The source is alwasy linear and the destination is always tiled.
*/
#define DMA1_SOURCE0                                    0x0D0010
#define DMA1_SOURCE0_SEL                                31:31 
#define DMA1_SOURCE0_SEL_LOCAL                          0
#define DMA1_SOURCE0_SEL_SYSTEM                         1  
#define DMA1_SOURCE0_DECODE                             30:30
#define DMA1_SOURCE0_DECODE_DISABLE                     0
#define DMA1_SOURCE0_DECODE_ENABLE                      1
#define DMA1_SOURCE0_ADDRESS                            29:0

#define DMA1_SOURCE0_SIZE                               0x0D0014
#define DMA1_SOURCE0_SIZE_NSC_CSS                       27:27
#define DMA1_SOURCE0_SIZE_NSC_CSS_DISABLE               0
#define DMA1_SOURCE0_SIZE_NSC_CSS_ENABLE                1
#define DMA1_SOURCE0_SIZE_NSC_CLL                       26:24
#define DMA1_SOURCE0_SIZE_SIZE                          23:0

#define DMA1_DESTINATION                                0x0D0018
#define DMA1_DESTINATION_SEL                            31:31
#define DMA1_DESTINATION_SEL_LOCAL                      0
#define DMA1_DESTINATION_SEL_SYSTEM                     1 
#define DMA1_DESTINATION_ADDRESS                        29:0

#define DMA1_CONTROL                                    0x0D001C
#define DMA1_CONTROL_STATUS                             31:31
#define DMA1_CONTROL_STATUS_IDLE                        0 
#define DMA1_CONTROL_STATUS_ENABLE                      1 
#define DMA1_CONTROL_TRI_STREAM                         30:30
#define DMA1_CONTROL_TRI_STREAM_DISABLE                 0
#define DMA1_CONTROL_TRI_STREAM_ENABLE                  1
#define DMA1_CONTROL_FORMAT                             29:28
#define DMA1_CONTROL_FORMAT_8BPP                        0
#define DMA1_CONTROL_FORMAT_16BPP                       1
#define DMA1_CONTROL_FORMAT_32BPP                       2
#define DMA1_CONTROL_TILE_HEIGHT                        27:16
#define DMA1_CONTROL_TILE_WIDTH                         12:0

#define DMA1_DESTINATION_PITCH                          0x0D0020
#define DMA1_DESTINATION_PITCH_PITCH                    14:0

#define DMA_CONTROL                                     0x0D0024
#define DMA_CONTROL_NSC_RB_SWAP                         11:11
#define DMA_CONTROL_NSC_RB_SWAP_DISABLE                 0
#define DMA_CONTROL_NSC_RB_SWAP_ENABLE                  1
#define DMA_CONTROL_CSC                                 10:10
#define DMA_CONTROL_CSC_DISABLE                         0
#define DMA_CONTROL_CSC_ENABLE                          1
#define DMA_CONTROL_DECOMP                              9:8
#define DMA_CONTROL_DECOMP_TEXT                         0
#define DMA_CONTROL_DECOMP_NSC                          1
#define DMA_CONTROL_DECOMP_GOLOMB                       2
#define DMA_CONTROL_DMA1_STATUS                         5:5
#define DMA_CONTROL_DMA1_STATUS_NORMAL                  0
#define DMA_CONTROL_DMA1_STATUS_ABORT                   1
#define DMA_CONTROL_DMA0_STATUS                         4:4
#define DMA_CONTROL_DMA0_STATUS_NORMAL                  0
#define DMA_CONTROL_DMA0_STATUS_ABORT                   1
#define DMA_CONTROL_DMA1_RAWINT                         1:1
#define DMA_CONTROL_DMA1_RAWINT_CLEAR                   0
#define DMA_CONTROL_DMA1_RAWINT_SET                     1
#define DMA_CONTROL_DMA0_RAWINT                         0:0
#define DMA_CONTROL_DMA0_RAWINT_CLEAR                   0
#define DMA_CONTROL_DMA0_RAWINT_SET                     1

/* DMA 1 source 1 and source 2 are only used when RGB is in 3 separate planes.*/
#define DMA1_SOURCE1                                    0x0D0028
#define DMA1_SOURCE1_ADDRESS                            29:0

#define DMA1_SOURCE1_SIZE                               0x0D002c
#define DMA1_SOURCE1_SIZE_NSC_CSS                       27:27
#define DMA1_SOURCE1_SIZE_NSC_CSS_DISABLE               0
#define DMA1_SOURCE1_SIZE_NSC_CSS_ENABLE                1
#define DMA1_SOURCE1_SIZE_NSC_CLL                       26:24
#define DMA1_SOURCE1_SIZE_SIZE                          23:0

#define DMA1_SOURCE2                                    0x0D0030
#define DMA1_SOURCE2_ADDRESS                            29:0

#define DMA1_SOURCE2_SIZE                               0x0D0034
#define DMA1_SOURCE2_SIZE_NSC_CSS                       27:27
#define DMA1_SOURCE2_SIZE_NSC_CSS_DISABLE               0
#define DMA1_SOURCE2_SIZE_NSC_CSS_ENABLE                1
#define DMA1_SOURCE2_SIZE_NSC_CLL                       26:24
#define DMA1_SOURCE2_SIZE_SIZE                          23:0


//#include "regde.h"
/* 2D registers. */

#define DE_SOURCE                                       0x100000
#define DE_SOURCE_WRAP                                  31:31
#define DE_SOURCE_WRAP_DISABLE                          0
#define DE_SOURCE_WRAP_ENABLE                           1

/* 
 * The following definitions are used in different setting 
 */

/* Use these definitions in XY addressing mode or linear addressing mode. */
#define DE_SOURCE_X_K1                                  27:16
#define DE_SOURCE_Y_K2                                  11:0

/* Use this definition in host write mode for mono. The Y_K2 is not used
   in host write mode. */
#define DE_SOURCE_X_K1_MONO                             20:16

/* Use these definitions in Bresenham line drawing mode. */
#define DE_SOURCE_X_K1_LINE                             29:16
#define DE_SOURCE_Y_K2_LINE                             13:0

#define DE_DESTINATION                                  0x100004
#define DE_DESTINATION_WRAP                             31:31
#define DE_DESTINATION_WRAP_DISABLE                     0
#define DE_DESTINATION_WRAP_ENABLE                      1
#if 1
    #define DE_DESTINATION_X                            27:16
    #define DE_DESTINATION_Y                            11:0
#else
    #define DE_DESTINATION_X                            28:16
    #define DE_DESTINATION_Y                            15:0
#endif

#define DE_DIMENSION                                    0x100008
#define DE_DIMENSION_X                                  28:16
#define DE_DIMENSION_Y_ET                               15:0

#define DE_CONTROL                                      0x10000C
#define DE_CONTROL_STATUS                               31:31
#define DE_CONTROL_STATUS_STOP                          0
#define DE_CONTROL_STATUS_START                         1
#define DE_CONTROL_PATTERN                              30:30
#define DE_CONTROL_PATTERN_MONO                         0
#define DE_CONTROL_PATTERN_COLOR                        1
#define DE_CONTROL_UPDATE_DESTINATION_X                 29:29
#define DE_CONTROL_UPDATE_DESTINATION_X_DISABLE         0
#define DE_CONTROL_UPDATE_DESTINATION_X_ENABLE          1
#define DE_CONTROL_QUICK_START                          28:28
#define DE_CONTROL_QUICK_START_DISABLE                  0
#define DE_CONTROL_QUICK_START_ENABLE                   1
#define DE_CONTROL_DIRECTION                            27:27
#define DE_CONTROL_DIRECTION_LEFT_TO_RIGHT              0
#define DE_CONTROL_DIRECTION_RIGHT_TO_LEFT              1
#define DE_CONTROL_MAJOR                                26:26
#define DE_CONTROL_MAJOR_X                              0
#define DE_CONTROL_MAJOR_Y                              1
#define DE_CONTROL_STEP_X                               25:25
#define DE_CONTROL_STEP_X_POSITIVE                      0
#define DE_CONTROL_STEP_X_NEGATIVE                      1
#define DE_CONTROL_STEP_Y                               24:24
#define DE_CONTROL_STEP_Y_POSITIVE                      0
#define DE_CONTROL_STEP_Y_NEGATIVE                      1
#define DE_CONTROL_STRETCH                              23:23
#define DE_CONTROL_STRETCH_DISABLE                      0
#define DE_CONTROL_STRETCH_ENABLE                       1
#define DE_CONTROL_HOST                                 22:22
#define DE_CONTROL_HOST_COLOR                           0
#define DE_CONTROL_HOST_MONO                            1
#define DE_CONTROL_LAST_PIXEL                           21:21
#define DE_CONTROL_LAST_PIXEL_OFF                       0
#define DE_CONTROL_LAST_PIXEL_ON                        1
#define DE_CONTROL_COMMAND                              20:16
#define DE_CONTROL_COMMAND_BITBLT                       0
#define DE_CONTROL_COMMAND_RECTANGLE_FILL               1
#define DE_CONTROL_COMMAND_DE_TILE                      2
#define DE_CONTROL_COMMAND_TRAPEZOID_FILL               3
#define DE_CONTROL_COMMAND_ALPHA_BLEND                  4
#define DE_CONTROL_COMMAND_RLE_STRIP                    5
#define DE_CONTROL_COMMAND_SHORT_STROKE                 6
#define DE_CONTROL_COMMAND_LINE_DRAW                    7
#define DE_CONTROL_COMMAND_HOST_WRITE                   8
#define DE_CONTROL_COMMAND_HOST_READ                    9
#define DE_CONTROL_COMMAND_HOST_WRITE_BOTTOM_UP         10
#define DE_CONTROL_COMMAND_ROTATE                       11
#define DE_CONTROL_COMMAND_FONT                         12
#define DE_CONTROL_COMMAND_TEXTURE_LOAD                 15
#define DE_CONTROL_ROP_SELECT                           15:15
#define DE_CONTROL_ROP_SELECT_ROP3                      0
#define DE_CONTROL_ROP_SELECT_ROP2                      1
#define DE_CONTROL_ROP2_SOURCE                          14:14
#define DE_CONTROL_ROP2_SOURCE_BITMAP                   0
#define DE_CONTROL_ROP2_SOURCE_PATTERN                  1
#define DE_CONTROL_MONO_DATA                            13:12
#define DE_CONTROL_MONO_DATA_NOT_PACKED                 0
#define DE_CONTROL_MONO_DATA_8_PACKED                   1
#define DE_CONTROL_MONO_DATA_16_PACKED                  2
#define DE_CONTROL_MONO_DATA_32_PACKED                  3
#define DE_CONTROL_REPEAT_ROTATE                        11:11
#define DE_CONTROL_REPEAT_ROTATE_DISABLE                0
#define DE_CONTROL_REPEAT_ROTATE_ENABLE                 1
#define DE_CONTROL_TRANSPARENCY_MATCH                   10:10
#define DE_CONTROL_TRANSPARENCY_MATCH_OPAQUE            0
#define DE_CONTROL_TRANSPARENCY_MATCH_TRANSPARENT       1
#define DE_CONTROL_TRANSPARENCY_SELECT                  9:9
#define DE_CONTROL_TRANSPARENCY_SELECT_SOURCE           0
#define DE_CONTROL_TRANSPARENCY_SELECT_DESTINATION      1
#define DE_CONTROL_TRANSPARENCY                         8:8
#define DE_CONTROL_TRANSPARENCY_DISABLE                 0
#define DE_CONTROL_TRANSPARENCY_ENABLE                  1
#define DE_CONTROL_ROP                                  7:0

/* Pseudo fields. */

#define DE_CONTROL_SHORT_STROKE_DIR                     27:24
#define DE_CONTROL_SHORT_STROKE_DIR_225                 0
#define DE_CONTROL_SHORT_STROKE_DIR_135                 1
#define DE_CONTROL_SHORT_STROKE_DIR_315                 2
#define DE_CONTROL_SHORT_STROKE_DIR_45                  3
#define DE_CONTROL_SHORT_STROKE_DIR_270                 4
#define DE_CONTROL_SHORT_STROKE_DIR_90                  5
#define DE_CONTROL_SHORT_STROKE_DIR_180                 8
#define DE_CONTROL_SHORT_STROKE_DIR_0                   10
#define DE_CONTROL_ROTATION                             25:24
#define DE_CONTROL_ROTATION_0                           0
#define DE_CONTROL_ROTATION_270                         1
#define DE_CONTROL_ROTATION_90                          2
#define DE_CONTROL_ROTATION_180                         3

#define DE_PITCH                                        0x100010
#define DE_PITCH_DESTINATION                            28:16
#define DE_PITCH_SOURCE                                 12:0

#define DE_FOREGROUND                                   0x100014
#define DE_FOREGROUND_COLOR                             31:0

#define DE_BACKGROUND                                   0x100018
#define DE_BACKGROUND_COLOR                             31:0

#define DE_STRETCH_FORMAT                               0x10001C
#define DE_STRETCH_FORMAT_PATTERN_XY                    30:30
#define DE_STRETCH_FORMAT_PATTERN_XY_NORMAL             0
#define DE_STRETCH_FORMAT_PATTERN_XY_OVERWRITE          1
#define DE_STRETCH_FORMAT_PATTERN_Y                     29:27
#define DE_STRETCH_FORMAT_PATTERN_X                     25:23
#define DE_STRETCH_FORMAT_PIXEL_FORMAT                  21:20
#define DE_STRETCH_FORMAT_PIXEL_FORMAT_8                0
#define DE_STRETCH_FORMAT_PIXEL_FORMAT_16               1
#define DE_STRETCH_FORMAT_PIXEL_FORMAT_32               2
#define DE_STRETCH_FORMAT_ADDRESSING                    19:16
#define DE_STRETCH_FORMAT_ADDRESSING_XY                 0
#define DE_STRETCH_FORMAT_ADDRESSING_LINEAR             15
#define DE_STRETCH_FORMAT_SOURCE_HEIGHT                 11:0

#define DE_COLOR_COMPARE                                0x100020
#define DE_COLOR_COMPARE_COLOR                          23:0

#define DE_COLOR_COMPARE_MASK                           0x100024
#define DE_COLOR_COMPARE_MASK_MASKS                     23:0

#define DE_MASKS                                        0x100028
#define DE_MASKS_BYTE_MASK                              31:16
#define DE_MASKS_BIT_MASK                               15:0

#define DE_CLIP_TL                                      0x10002C
#define DE_CLIP_TL_TOP                                  31:16
#define DE_CLIP_TL_STATUS                               13:13
#define DE_CLIP_TL_STATUS_DISABLE                       0
#define DE_CLIP_TL_STATUS_ENABLE                        1
#define DE_CLIP_TL_INHIBIT                              12:12
#define DE_CLIP_TL_INHIBIT_OUTSIDE                      0
#define DE_CLIP_TL_INHIBIT_INSIDE                       1
#define DE_CLIP_TL_LEFT                                 11:0

#define DE_CLIP_BR                                      0x100030
#define DE_CLIP_BR_BOTTOM                               31:16
#define DE_CLIP_BR_RIGHT                                12:0

#define DE_MONO_PATTERN_LOW                             0x100034
#define DE_MONO_PATTERN_LOW_PATTERN                     31:0

#define DE_MONO_PATTERN_HIGH                            0x100038
#define DE_MONO_PATTERN_HIGH_PATTERN                    31:0

#define DE_WINDOW_WIDTH                                 0x10003C
#define DE_WINDOW_WIDTH_DESTINATION                     28:16
#define DE_WINDOW_WIDTH_SOURCE                          12:0

#define DE_WINDOW_SOURCE_BASE                           0x100040
#define DE_WINDOW_SOURCE_BASE_ADDRESS                   29:0

#define DE_WINDOW_DESTINATION_BASE                      0x100044
#define DE_WINDOW_DESTINATION_BASE_ADDRESS              29:0

#define DE_ALPHA                                        0x100048
#define DE_ALPHA_VALUE                                  7:0

#define DE_WRAP                                         0x10004C
#define DE_WRAP_X                                       31:16
#define DE_WRAP_Y                                       15:0

#define DE_STATE2                                        0x100054
#define DE_STATE2_DE_FIFO                                3:3
#define DE_STATE2_DE_FIFO_NOTEMPTY                       1
#define DE_STATE2_DE_FIFO_EMPTY                          0
#define DE_STATE2_DE_STATUS                              2:2
#define DE_STATE2_DE_STATUS_IDLE                         0
#define DE_STATE2_DE_STATUS_BUSY                         1
#define DE_STATE2_DE_MEM_FIFO                            1:1
#define DE_STATE2_DE_MEM_FIFO_NOTEMPTY                   0
#define DE_STATE2_DE_MEM_FIFO_EMPTY                      1
#define DE_STATE2_DE_ABORT                               0:0
#define DE_STATE2_DE_ABORT_OFF                           0
#define DE_STATE2_DE_ABORT_ON                            1

/* Color Space Conversion registers. */

#define CSC_Y_SOURCE_BASE                               0x1000C8
#define CSC_Y_SOURCE_BASE_ADDRESS                       29:0

#define CSC_CONSTANTS                                   0x1000CC
#define CSC_CONSTANTS_Y                                 31:24
#define CSC_CONSTANTS_R                                 23:16
#define CSC_CONSTANTS_G                                 15:8
#define CSC_CONSTANTS_B                                 7:0

#define CSC_Y_SOURCE_X                                  0x1000D0
#define CSC_Y_SOURCE_X_INTEGER                          26:16
#define CSC_Y_SOURCE_X_FRACTION                         15:3

#define CSC_Y_SOURCE_Y                                  0x1000D4
#define CSC_Y_SOURCE_Y_INTEGER                          27:16
#define CSC_Y_SOURCE_Y_FRACTION                         15:3

#define CSC_U_SOURCE_BASE                               0x1000D8
#define CSC_U_SOURCE_BASE_ADDRESS                       29:0

#define CSC_V_SOURCE_BASE                               0x1000DC
#define CSC_V_SOURCE_BASE_ADDRESS                       29:0

#define CSC_SOURCE_DIMENSION                            0x1000E0
#define CSC_SOURCE_DIMENSION_X                          31:16
#define CSC_SOURCE_DIMENSION_Y                          15:0

#define CSC_SOURCE_PITCH                                0x1000E4
#define CSC_SOURCE_PITCH_Y                              31:16
#define CSC_SOURCE_PITCH_UV                             15:0

#define CSC_DESTINATION                                 0x1000E8
#define CSC_DESTINATION_WRAP                            31:31
#define CSC_DESTINATION_WRAP_DISABLE                    0
#define CSC_DESTINATION_WRAP_ENABLE                     1
#define CSC_DESTINATION_X                               27:16
#define CSC_DESTINATION_Y                               11:0

#define CSC_DESTINATION_DIMENSION                       0x1000EC
#define CSC_DESTINATION_DIMENSION_X                     31:16
#define CSC_DESTINATION_DIMENSION_Y                     15:0

#define CSC_DESTINATION_PITCH                           0x1000F0
#define CSC_DESTINATION_PITCH_X                         31:16
#define CSC_DESTINATION_PITCH_Y                         15:0

#define CSC_SCALE_FACTOR                                0x1000F4
#define CSC_SCALE_FACTOR_HORIZONTAL                     31:16
#define CSC_SCALE_FACTOR_VERTICAL                       15:0

#define CSC_DESTINATION_BASE                            0x1000F8
#define CSC_DESTINATION_BASE_ADDRESS                    29:0

#define CSC_CONTROL                                     0x1000FC
#define CSC_CONTROL_STATUS                              31:31
#define CSC_CONTROL_STATUS_STOP                         0
#define CSC_CONTROL_STATUS_START                        1
#define CSC_CONTROL_SOURCE_FORMAT                       30:28
#define CSC_CONTROL_SOURCE_FORMAT_YUV422                0
#define CSC_CONTROL_SOURCE_FORMAT_YUV420I               1
#define CSC_CONTROL_SOURCE_FORMAT_YUV420                2
#define CSC_CONTROL_SOURCE_FORMAT_YVU9                  3
#define CSC_CONTROL_SOURCE_FORMAT_IYU1                  4
#define CSC_CONTROL_SOURCE_FORMAT_IYU2                  5
#define CSC_CONTROL_SOURCE_FORMAT_RGB565                6
#define CSC_CONTROL_SOURCE_FORMAT_RGB8888               7
#define CSC_CONTROL_DESTINATION_FORMAT                  27:26
#define CSC_CONTROL_DESTINATION_FORMAT_RGB565           0
#define CSC_CONTROL_DESTINATION_FORMAT_RGB8888          1
#define CSC_CONTROL_HORIZONTAL_FILTER                   25:25
#define CSC_CONTROL_HORIZONTAL_FILTER_DISABLE           0
#define CSC_CONTROL_HORIZONTAL_FILTER_ENABLE            1
#define CSC_CONTROL_VERTICAL_FILTER                     24:24
#define CSC_CONTROL_VERTICAL_FILTER_DISABLE             0
#define CSC_CONTROL_VERTICAL_FILTER_ENABLE              1
#define CSC_CONTROL_BYTE_ORDER                          23:23
#define CSC_CONTROL_BYTE_ORDER_YUYV                     0
#define CSC_CONTROL_BYTE_ORDER_UYVY                     1

#define DE_DATA_PORT                                    0x110000

//#include "regzv.h"

/* ZV0 */

#define ZV0_CAPTURE_CTRL                                0x090000
#define ZV0_CAPTURE_CTRL_FIELD_INPUT                    27:27
#define ZV0_CAPTURE_CTRL_FIELD_INPUT_EVEN_FIELD         0
#define ZV0_CAPTURE_CTRL_FIELD_INPUT_ODD_FIELD          1
#define ZV0_CAPTURE_CTRL_SCAN                           26:26
#define ZV0_CAPTURE_CTRL_SCAN_PROGRESSIVE               0
#define ZV0_CAPTURE_CTRL_SCAN_INTERLACE                 1
#define ZV0_CAPTURE_CTRL_CURRENT_BUFFER                 25:25
#define ZV0_CAPTURE_CTRL_CURRENT_BUFFER_0               0
#define ZV0_CAPTURE_CTRL_CURRENT_BUFFER_1               1
#define ZV0_CAPTURE_CTRL_VERTICAL_SYNC                  24:24
#define ZV0_CAPTURE_CTRL_VERTICAL_SYNC_INACTIVE         0
#define ZV0_CAPTURE_CTRL_VERTICAL_SYNC_ACTIVE           1
#define ZV0_CAPTURE_CTRL_OUTPUT_FORMAT                  22:22
#define ZV0_CAPTURE_CTRL_OUTPUT_FORMAT_16               0
#define ZV0_CAPTURE_CTRL_OUTPUT_FORMAT_32               1
#define ZV0_CAPTURE_CTRL_INCOME_DATA                  	21:21
#define ZV0_CAPTURE_CTRL_INCOME_DATA_16                 0
#define ZV0_CAPTURE_CTRL_INCOME_DATA_32	                1
#define ZV0_CAPTURE_CTRL_ADJ                            19:19
#define ZV0_CAPTURE_CTRL_ADJ_NORMAL                     0
#define ZV0_CAPTURE_CTRL_ADJ_DELAY                      1
#define ZV0_CAPTURE_CTRL_HA                             18:18
#define ZV0_CAPTURE_CTRL_HA_DISABLE                     0
#define ZV0_CAPTURE_CTRL_HA_ENABLE                      1
#define ZV0_CAPTURE_CTRL_VSK                            17:17
#define ZV0_CAPTURE_CTRL_VSK_DISABLE                    0
#define ZV0_CAPTURE_CTRL_VSK_ENABLE                     1
#define ZV0_CAPTURE_CTRL_HSK                            16:16
#define ZV0_CAPTURE_CTRL_HSK_DISABLE                    0
#define ZV0_CAPTURE_CTRL_HSK_ENABLE                     1
#define ZV0_CAPTURE_CTRL_FD                             15:15
#define ZV0_CAPTURE_CTRL_FD_RISING                      0
#define ZV0_CAPTURE_CTRL_FD_FALLING                     1
#define ZV0_CAPTURE_CTRL_VP                             14:14
#define ZV0_CAPTURE_CTRL_VP_HIGH                        0
#define ZV0_CAPTURE_CTRL_VP_LOW                         1
#define ZV0_CAPTURE_CTRL_HP                             13:13
#define ZV0_CAPTURE_CTRL_HP_HIGH                        0
#define ZV0_CAPTURE_CTRL_HP_LOW                         1
#define ZV0_CAPTURE_CTRL_CP                             12:12
#define ZV0_CAPTURE_CTRL_CP_HIGH                        0
#define ZV0_CAPTURE_CTRL_CP_LOW                         1
#define ZV0_CAPTURE_CTRL_UVS                            11:11
#define ZV0_CAPTURE_CTRL_UVS_DISABLE                    0
#define ZV0_CAPTURE_CTRL_UVS_ENABLE                     1
#define ZV0_CAPTURE_CTRL_BS                             10:10
#define ZV0_CAPTURE_CTRL_BS_DISABLE                     0
#define ZV0_CAPTURE_CTRL_BS_ENABLE                      1
#define ZV0_CAPTURE_CTRL_CS                             9:9
#define ZV0_CAPTURE_CTRL_CS_16                          0
#define ZV0_CAPTURE_CTRL_CS_8                           1
#define ZV0_CAPTURE_CTRL_CF                             8:8
#define ZV0_CAPTURE_CTRL_CF_YUV                         0
#define ZV0_CAPTURE_CTRL_CF_RGB                         1
#define ZV0_CAPTURE_CTRL_FS                             7:7
#define ZV0_CAPTURE_CTRL_FS_DISABLE                     0
#define ZV0_CAPTURE_CTRL_FS_ENABLE                      1
#define ZV0_CAPTURE_CTRL_WEAVE                          6:6
#define ZV0_CAPTURE_CTRL_WEAVE_DISABLE                  0
#define ZV0_CAPTURE_CTRL_WEAVE_ENABLE                   1
#define ZV0_CAPTURE_CTRL_BOB                            5:5
#define ZV0_CAPTURE_CTRL_BOB_DISABLE                    0
#define ZV0_CAPTURE_CTRL_BOB_ENABLE                     1
#define ZV0_CAPTURE_CTRL_DB                             4:4
#define ZV0_CAPTURE_CTRL_DB_DISABLE                     0
#define ZV0_CAPTURE_CTRL_DB_ENABLE                      1
#define ZV0_CAPTURE_CTRL_CC                             3:3
#define ZV0_CAPTURE_CTRL_CC_CONTINUE                    0
#define ZV0_CAPTURE_CTRL_CC_CONDITION                   1
#define ZV0_CAPTURE_CTRL_RGB                            2:2
#define ZV0_CAPTURE_CTRL_RGB_DISABLE                    0
#define ZV0_CAPTURE_CTRL_RGB_ENABLE                     1
#define ZV0_CAPTURE_CTRL_656                            1:1
#define ZV0_CAPTURE_CTRL_656_DISABLE                    0
#define ZV0_CAPTURE_CTRL_656_ENABLE                     1
#define ZV0_CAPTURE_CTRL_CAP                            0:0
#define ZV0_CAPTURE_CTRL_CAP_DISABLE                    0
#define ZV0_CAPTURE_CTRL_CAP_ENABLE                     1

#define ZV0_CAPTURE_CLIP                                0x090004
#define ZV0_CAPTURE_CLIP_YCLIP_EVEN_FIELD                25:16
#define ZV0_CAPTURE_CLIP_YCLIP                          25:16
#define ZV0_CAPTURE_CLIP_XCLIP                          9:0

#define ZV0_CAPTURE_SIZE                                0x090008
#define ZV0_CAPTURE_SIZE_HEIGHT                         26:16
#define ZV0_CAPTURE_SIZE_WIDTH                          10:0   

#define ZV0_CAPTURE_BUF0_ADDRESS                        0x09000C
#define ZV0_CAPTURE_BUF0_ADDRESS_STATUS                 31:31
#define ZV0_CAPTURE_BUF0_ADDRESS_STATUS_CURRENT         0
#define ZV0_CAPTURE_BUF0_ADDRESS_STATUS_PENDING         1
#define ZV0_CAPTURE_BUF0_ADDRESS_ADDRESS                29:0

#define ZV0_CAPTURE_BUF1_ADDRESS                        0x090010
#define ZV0_CAPTURE_BUF1_ADDRESS_STATUS                 31:31
#define ZV0_CAPTURE_BUF1_ADDRESS_STATUS_CURRENT         0
#define ZV0_CAPTURE_BUF1_ADDRESS_STATUS_PENDING         1
#define ZV0_CAPTURE_BUF1_ADDRESS_ADDRESS                29:0

#define ZV0_CAPTURE_BUF_OFFSET                          0x090014
#define ZV0_CAPTURE_BUF_OFFSET_YCLIP_ODD_FIELD          25:16
#define ZV0_CAPTURE_BUF_OFFSET_OFFSET                   15:0

#define ZV0_CAPTURE_FIFO_CTRL                           0x090018
#define ZV0_CAPTURE_FIFO_CTRL_FIFO                      2:0
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_0                    0
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_1                    1
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_2                    2
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_3                    3
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_4                    4
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_5                    5
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_6                    6
#define ZV0_CAPTURE_FIFO_CTRL_FIFO_7                    7

#define ZV0_CAPTURE_YRGB_CONST                          0x09001C
#define ZV0_CAPTURE_YRGB_CONST_Y                        31:24
#define ZV0_CAPTURE_YRGB_CONST_R                        23:16
#define ZV0_CAPTURE_YRGB_CONST_G                        15:8
#define ZV0_CAPTURE_YRGB_CONST_B                        7:0

#define ZV0_CAPTURE_LINE_COMP                           0x090020
#define ZV0_CAPTURE_LINE_COMP_LC                        10:0

/* ZV1 */

#define ZV1_CAPTURE_CTRL                                0x098000
#define ZV1_CAPTURE_CTRL_FIELD_INPUT                    27:27
#define ZV1_CAPTURE_CTRL_FIELD_INPUT_EVEN_FIELD         0
#define ZV1_CAPTURE_CTRL_FIELD_INPUT_ODD_FIELD          0
#define ZV1_CAPTURE_CTRL_SCAN                           26:26
#define ZV1_CAPTURE_CTRL_SCAN_PROGRESSIVE               0
#define ZV1_CAPTURE_CTRL_SCAN_INTERLACE                 1
#define ZV1_CAPTURE_CTRL_CURRENT_BUFFER                 25:25
#define ZV1_CAPTURE_CTRL_CURRENT_BUFFER_0               0
#define ZV1_CAPTURE_CTRL_CURRENT_BUFFER_1               1
#define ZV1_CAPTURE_CTRL_VERTICAL_SYNC                  24:24
#define ZV1_CAPTURE_CTRL_VERTICAL_SYNC_INACTIVE         0
#define ZV1_CAPTURE_CTRL_VERTICAL_SYNC_ACTIVE           1
#define ZV1_CAPTURE_CTRL_CHANNEL0                        20:20
#define ZV1_CAPTURE_CTRL_CHANNEL0_DISABLE                0
#define ZV1_CAPTURE_CTRL_CHANNEL0_ENABLE                 1
#define ZV1_CAPTURE_CTRL_ADJ                            19:19
#define ZV1_CAPTURE_CTRL_ADJ_NORMAL                     0
#define ZV1_CAPTURE_CTRL_ADJ_DELAY                      1
#define ZV1_CAPTURE_CTRL_HA                             18:18
#define ZV1_CAPTURE_CTRL_HA_DISABLE                     0
#define ZV1_CAPTURE_CTRL_HA_ENABLE                      1
#define ZV1_CAPTURE_CTRL_VSK                            17:17
#define ZV1_CAPTURE_CTRL_VSK_DISABLE                    0
#define ZV1_CAPTURE_CTRL_VSK_ENABLE                     1
#define ZV1_CAPTURE_CTRL_HSK                            16:16
#define ZV1_CAPTURE_CTRL_HSK_DISABLE                    0
#define ZV1_CAPTURE_CTRL_HSK_ENABLE                     1
#define ZV1_CAPTURE_CTRL_FD                             15:15
#define ZV1_CAPTURE_CTRL_FD_RISING                      0
#define ZV1_CAPTURE_CTRL_FD_FALLING                     1
#define ZV1_CAPTURE_CTRL_VP                             14:14
#define ZV1_CAPTURE_CTRL_VP_HIGH                        0
#define ZV1_CAPTURE_CTRL_VP_LOW                         1
#define ZV1_CAPTURE_CTRL_HP                             13:13
#define ZV1_CAPTURE_CTRL_HP_HIGH                        0
#define ZV1_CAPTURE_CTRL_HP_LOW                         1
#define ZV1_CAPTURE_CTRL_CP                             12:12
#define ZV1_CAPTURE_CTRL_CP_HIGH                        0
#define ZV1_CAPTURE_CTRL_CP_LOW                         1
#define ZV1_CAPTURE_CTRL_UVS                            11:11
#define ZV1_CAPTURE_CTRL_UVS_DISABLE                    0
#define ZV1_CAPTURE_CTRL_UVS_ENABLE                     1
#define ZV1_CAPTURE_CTRL_BS                             10:10
#define ZV1_CAPTURE_CTRL_BS_DISABLE                     0
#define ZV1_CAPTURE_CTRL_BS_ENABLE                      1
#define ZV1_CAPTURE_CTRL_CS                             9:9
#define ZV1_CAPTURE_CTRL_CS_16                          0
#define ZV1_CAPTURE_CTRL_CS_8                           1
#define ZV1_CAPTURE_CTRL_CF                             8:8
#define ZV1_CAPTURE_CTRL_CF_YUV                         0
#define ZV1_CAPTURE_CTRL_CF_RGB                         1
#define ZV1_CAPTURE_CTRL_FS                             7:7
#define ZV1_CAPTURE_CTRL_FS_DISABLE                     0
#define ZV1_CAPTURE_CTRL_FS_ENABLE                      1
#define ZV1_CAPTURE_CTRL_WEAVE                          6:6
#define ZV1_CAPTURE_CTRL_WEAVE_DISABLE                  0
#define ZV1_CAPTURE_CTRL_WEAVE_ENABLE                   1
#define ZV1_CAPTURE_CTRL_BOB                            5:5
#define ZV1_CAPTURE_CTRL_BOB_DISABLE                    0
#define ZV1_CAPTURE_CTRL_BOB_ENABLE                     1
#define ZV1_CAPTURE_CTRL_DB                             4:4
#define ZV1_CAPTURE_CTRL_DB_DISABLE                     0
#define ZV1_CAPTURE_CTRL_DB_ENABLE                      1
#define ZV1_CAPTURE_CTRL_CC                             3:3
#define ZV1_CAPTURE_CTRL_CC_CONTINUE                    0
#define ZV1_CAPTURE_CTRL_CC_CONDITION                   1
#define ZV1_CAPTURE_CTRL_RGB                            2:2
#define ZV1_CAPTURE_CTRL_RGB_DISABLE                    0
#define ZV1_CAPTURE_CTRL_RGB_ENABLE                     1
#define ZV1_CAPTURE_CTRL_656                            1:1
#define ZV1_CAPTURE_CTRL_656_DISABLE                    0
#define ZV1_CAPTURE_CTRL_656_ENABLE                     1
#define ZV1_CAPTURE_CTRL_CAP                            0:0
#define ZV1_CAPTURE_CTRL_CAP_DISABLE                    0
#define ZV1_CAPTURE_CTRL_CAP_ENABLE                     1

#define ZV1_CAPTURE_CLIP                                0x098004
#define ZV1_CAPTURE_CLIP_YCLIP                          25:16
#define ZV1_CAPTURE_CLIP_XCLIP                          9:0

#define ZV1_CAPTURE_SIZE                                0x098008
#define ZV1_CAPTURE_SIZE_HEIGHT                         26:16   
#define ZV1_CAPTURE_SIZE_WIDTH                          10:0   

#define ZV1_CAPTURE_BUF0_ADDRESS                        0x09800C
#define ZV1_CAPTURE_BUF0_ADDRESS_STATUS                 31:31
#define ZV1_CAPTURE_BUF0_ADDRESS_STATUS_CURRENT         0
#define ZV1_CAPTURE_BUF0_ADDRESS_STATUS_PENDING         1
#define ZV1_CAPTURE_BUF0_ADDRESS_ADDRESS                29:0

#define ZV1_CAPTURE_BUF1_ADDRESS                        0x098010
#define ZV1_CAPTURE_BUF1_ADDRESS_STATUS                 31:31
#define ZV1_CAPTURE_BUF1_ADDRESS_STATUS_CURRENT         0
#define ZV1_CAPTURE_BUF1_ADDRESS_STATUS_PENDING         1
#define ZV1_CAPTURE_BUF1_ADDRESS_ADDRESS                29:0

#define ZV1_CAPTURE_BUF_OFFSET                          0x098014
#define ZV1_CAPTURE_BUF_OFFSET_OFFSET                   15:0

#define ZV1_CAPTURE_FIFO_CTRL                           0x098018
#define ZV1_CAPTURE_FIFO_CTRL_FIFO                      2:0
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_0                    0
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_1                    1
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_2                    2
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_3                    3
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_4                    4
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_5                    5
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_6                    6
#define ZV1_CAPTURE_FIFO_CTRL_FIFO_7                    7

#define ZV1_CAPTURE_YRGB_CONST                          0x09801C
#define ZV1_CAPTURE_YRGB_CONST_Y                        31:24
#define ZV1_CAPTURE_YRGB_CONST_R                        23:16
#define ZV1_CAPTURE_YRGB_CONST_G                        15:8
#define ZV1_CAPTURE_YRGB_CONST_B                        7:0



//#include "regi2c.h"
#define I2C_BYTE_COUNT                                  0x010040
#define I2C_BYTE_COUNT_COUNT                            3:0

#define I2C_CTRL                                        0x010041
#define I2C_CTRL_INT                                    4:4
#define I2C_CTRL_INT_DISABLE                            0
#define I2C_CTRL_INT_ENABLE                             1
#define I2C_CTRL_CTRL                                   2:2
#define I2C_CTRL_CTRL_STOP                              0
#define I2C_CTRL_CTRL_START                             1
#define I2C_CTRL_MODE                                   1:1
#define I2C_CTRL_MODE_STANDARD                          0
#define I2C_CTRL_MODE_FAST                              1
#define I2C_CTRL_EN                                     0:0
#define I2C_CTRL_EN_DISABLE                             0
#define I2C_CTRL_EN_ENABLE                              1

#define I2C_STATUS                                      0x010042
#define I2C_STATUS_TX                                   3:3
#define I2C_STATUS_TX_PROGRESS                          0
#define I2C_STATUS_TX_COMPLETED                         1
#define I2C_STATUS_ERR                                  2:2
#define I2C_STATUS_ERR_NORMAL                           0
#define I2C_STATUS_ERR_ERROR                            1
#define I2C_STATUS_ERR_CLEAR                            0
#define I2C_STATUS_ACK                                  1:1
#define I2C_STATUS_ACK_RECEIVED                         0
#define I2C_STATUS_ACK_NOT                              1
#define I2C_STATUS_BSY                                  0:0
#define I2C_STATUS_BSY_IDLE                             0
#define I2C_STATUS_BSY_BUSY                             1

#define I2C_RESET                                       0x010042
#define I2C_RESET_BUS_ERROR                             2:2
#define I2C_RESET_BUS_ERROR_CLEAR                       0

#define I2C_SLAVE_ADDRESS                               0x010043
#define I2C_SLAVE_ADDRESS_ADDRESS                       7:1
#define I2C_SLAVE_ADDRESS_RW                            0:0
#define I2C_SLAVE_ADDRESS_RW_W                          0
#define I2C_SLAVE_ADDRESS_RW_R                          1

#define I2C_DATA0                                       0x010044
#define I2C_DATA1                                       0x010045
#define I2C_DATA2                                       0x010046
#define I2C_DATA3                                       0x010047
#define I2C_DATA4                                       0x010048
#define I2C_DATA5                                       0x010049
#define I2C_DATA6                                       0x01004A
#define I2C_DATA7                                       0x01004B
#define I2C_DATA8                                       0x01004C
#define I2C_DATA9                                       0x01004D
#define I2C_DATA10                                      0x01004E
#define I2C_DATA11                                      0x01004F
#define I2C_DATA12                                      0x010050
#define I2C_DATA13                                      0x010051
#define I2C_DATA14                                      0x010052
#define I2C_DATA15                                      0x010053

/* MMIO offset between I2C0 and I2C1 */
#define I2C_OFFSET 0x20

//#include "regtimer.h"

/* There are 4 timers in the system with the same definition,
   but different MMIO address as below
  
   TIMER 0  0x010030
   TIMER 1  0x010034
   TIMER 2  0x010038
   TIMER 3  0x01003C

   We only define the MMIO for timer 0, the MMIO for other
   timer can be work out like this:
   0x10030 + (4 x Timer number)

*/

#define TIMER_CONTROL                                   0x010030
#define TIMER_CONTROL_COUNTER                           31:4
#define TIMER_CONTROL_RAWINT_STATUS                     3:3
#define TIMER_CONTROL_RAWINT_STATUS_CLEAR               0
#define TIMER_CONTROL_RAWINT_STATUS_PENDING             1
#define TIMER_CONTROL_RAWINT_STATUS_RESET               1
#define TIMER_CONTROL_RAWINT_ENABLE                     2:2  
#define TIMER_CONTROL_RAWINT_ENABLE_DISABLE             0
#define TIMER_CONTROL_RAWINT_ENABLE_ENABLE              1
#define TIMER_CONTROL_DIV16                             1:1
#define TIMER_CONTROL_DIV16_DISABLE                     0
#define TIMER_CONTROL_DIV16_ENABLE                      1
#define TIMER_CONTROL_ENABLE                            0:0
#define TIMER_CONTROL_ENABLE_DISABLE                    0
#define TIMER_CONTROL_ENABLE_ENABLE                     1

#define I2S_TX_DATA_L                              0x0A0200
#define I2S_TX_DATA_L_DATA						   15:0

#define I2S_TX_DATA_R                              0x0A0204
#define I2S_TX_DATA_R_DATA                         15:0

#define I2S_RX_DATA_L                              0x0A0208
#define I2S_RX_DATA_L_DATA						   15:0

#define I2S_RX_DATA_R                              0x0A020C
#define I2S_RX_DATA_R_DATA						   15:0

#define I2S_STATUS                                 0x0A0210
#define I2S_STATUS_R                               11:11
#define I2S_STATUS_R_NO_ERR                        0
#define I2S_STATUS_R_OVERFLOW                      1
#define I2S_STATUS_T                               10:10
#define I2S_STATUS_T_NO_ERR                        0
#define I2S_STATUS_T_UNDERFLOW                     1
#define I2S_STATUS_TX                              2:2
#define I2S_STATUS_TX_DISABLE                      0
#define I2S_STATUS_TX_ENABLE                       1

#define I2S_CTRL                                    0x0A0214
#define I2S_CTRL_MODE                               7:7
#define I2S_CTRL_MODE_SLAVE                         0
#define I2S_CTRL_MODE_MASTER                        1
#define I2S_CTRL_CS                                 6:5
#define I2S_CTRL_CS_16                              0
#define I2S_CTRL_CS_24                              1
#define I2S_CTRL_CS_32                              2
#define I2S_CTRL_CDIV                               4:0


#define I2S_SRAM_DMA                                0x0A0218
#define I2S_SRAM_DMA_STATE                          31:31    
#define I2S_SRAM_DMA_STATE_DISABLE                  0            
#define I2S_SRAM_DMA_STATE_ENABLE                   1
#define I2S_SRAM_DMA_SIZE                           23:16     
#define I2S_SRAM_DMA_ADDRESS						8:0                               
           
#define I2S_SRAM_DMA_STATUS                         0x0A021C
#define I2S_SRAM_DMA_STATUS_TC                      0:0    
#define I2S_SRAM_DMA_STATUS_TC_COMPLETE             1            
#define I2S_SRAM_DMA_STATUS_TC_CLEAR                0



#define SRAM_OUTPUT_BASE							0x8000
#define SRAM_INPUT_BASE								0x8800
#define SRAM_SIZE									0x0800



/* Internal macros */
#define _F_START(f)             (0 ? f)
#define _F_END(f)               (1 ? f)
#define _F_SIZE(f)              (1 + _F_END(f) - _F_START(f))
#define _F_MASK(f)              (((1 << _F_SIZE(f)) - 1) << _F_START(f))
#define _F_NORMALIZE(v, f)      (((v) & _F_MASK(f)) >> _F_START(f))
#define _F_DENORMALIZE(v, f)    (((v) << _F_START(f)) & _F_MASK(f))

/* Global macros */
#define FIELD_GET(x, reg, field) \
( \
    _F_NORMALIZE((x), reg ## _ ## field) \
)

#define FIELD_SET(x, reg, field, value) \
( \
    (x & ~_F_MASK(reg ## _ ## field)) \
    | _F_DENORMALIZE(reg ## _ ## field ## _ ## value, reg ## _ ## field) \
)

#define FIELD_VALUE(x, reg, field, value) \
( \
    (x & ~_F_MASK(reg ## _ ## field)) \
    | _F_DENORMALIZE(value, reg ## _ ## field) \
)

#define FIELD_CLEAR(reg, field) \
( \
    ~ _F_MASK(reg ## _ ## field) \
)

/* FIELD MACROS */
#define FIELD_START(field)              (0 ? field)
#define FIELD_END(field)                (1 ? field)
#define FIELD_SIZE(field)               (1 + FIELD_END(field) - FIELD_START(field))
#define FIELD_MASK(field)               (((1 << (FIELD_SIZE(field)-1)) | ((1 << (FIELD_SIZE(field)-1)) - 1)) << FIELD_START(field))
#define FIELD_NORMALIZE(reg, field)     (((reg) & FIELD_MASK(field)) >> FIELD_START(field))
#define FIELD_DENORMALIZE(field, value) (((value) << FIELD_START(field)) & FIELD_MASK(field))
#define FIELD_INIT(reg, field, value)   FIELD_DENORMALIZE(reg ## _ ## field, \
                                                          reg ## _ ## field ## _ ## value)
#define FIELD_INIT_VAL(reg, field, value) \
                                        (FIELD_DENORMALIZE(reg ## _ ## field, value))
#define FIELD_VAL_SET(x, r, f, v)       x = x & ~FIELD_MASK(r ## _ ## f) \
                                              | FIELD_DENORMALIZE(r ## _ ## f, r ## _ ## f ## _ ## v)

#define RGB(r, g, b) \
( \
    (unsigned long) (((r) << 16) | ((g) << 8) | (b)) \
)

#define RGB16(r, g, b) \
( \
    (unsigned short) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3)) \
)


