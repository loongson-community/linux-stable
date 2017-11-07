#ifndef  _HDMIREGS_H_
#define  _HDMIREGS_H_

// SLI-204-06012 Rev.2.01a
//-----------------------------------------------------------------------------
// SLISHDMI13T Regsiter Defines         Addr        RW  init    Description
//-----------------------------------------------------------------------------
#define     X00_SYSTEM_CONTROL      0       //  RW  10h     Power save and interrupt output control
	#define 		X00_SYSTEM_CONTROL_MODE													7:4
	#define			X00_SYSTEM_CONTROL_MODE_A												1
	#define			X00_SYSTEM_CONTROL_MODE_B												2
	#define			X00_SYSTEM_CONTROL_MODE_D												4
	#define			X00_SYSTEM_CONTROL_MODE_E												8
	#define 		X00_SYSTEM_CONTROL_PLLB													3:3
	#define			X00_SYSTEM_CONTROL_PLLB_RST			  						 	1
	#define			X00_SYSTEM_CONTROL_PLLB_UNRST			  					  0
	#define			X00_SYSTEM_CONTROL_INTOUT												1:1
	#define			X00_SYSTEM_CONTROL_INTOUT_N	    								0						//Interrupt output mode
	#define			X00_SYSTEM_CONTROL_INTOUT_PP	    							1						//Interrupt output mode
	#define			X00_SYSTEM_CONTROL_INTOUT_POLARITY							0:0
	#define			X00_SYSTEM_CONTROL_INTOUT_POLARITY_L	    			0						//Interrupt output polarity
	#define			X00_SYSTEM_CONTROL_INTOUT_POLARITY_H	    		  1						//Interrupt output polarity

#define     X01_N19_16            			  1       //  RW  00h     20-bit N used for cycle time stamp
	#define			X01_N19_16_LRC_SWAP							7:4
	#define			X01_N19_16_LRC_SWAP_NO					0						//L/R data swap
	#define			X01_N19_16_LRC_SWAP_SP1					2
	#define			X01_N19_16_LRC_SWAP_SP2					4
	#define			X01_N19_16_LRC_SWAP_SP3					8
	#define			X01_N19_16_AUDCLK								3:0
#define     X02_N15_8               2       //  RW  00h
#define     X03_N7_0                3       //  RW  00h
#define     X04_SPDIF_FS            4       //  RO  00h     SPDIF sampling frequency/CTS[19:16] internal
	#define			X04_SPDIF_FS_K							7:4
	#define			X04_SPDIF_FS_K32						3
	#define			X04_SPDIF_FS_K44_1					0
	#define			X04_SPDIF_FS_K48						2
	#define			X04_SPDIF_FS_K88_2					8
	#define			X04_SPDIF_FS_K96			  		10
	#define			X04_SPDIF_FS_K176_4		 			12
	#define			X04_SPDIF_FS_K192						14
	#define			X04_SPDIF_FS_CTSIN_19_16 		3:0

#define     X05_CTS_INT             5       //  RO  00h     CTS[15:8] internal
#define     X06_CTS_INT             6       //  RO  00h     CTS[7:0] internal
#define     X07_CTS_EXT             7       //  RW  00h     CTS[19:16] external
#define     X08_CTS_EXT             8       //  RW  00h     CTS[15:8] external
#define     X09_CTS_EXT             9       //  RW  00h     CTS[7:0] external
#define     X0A_AUDIO_SOURCE        10      //  RW  00h     Audio setting.1
	#define			X0A_AUDIO_SOURCE_CTS						7:7
	#define			X0A_AUDIO_SOURCE_CTS_INTERNAL					0
	#define			X0A_AUDIO_SOURCE_CTS_EXTERNAL					1
	#define			X0A_AUDIO_SOURCE_DS							6:5
	#define			X0A_AUDIO_SOURCE_DS_NONE				0
	#define			X0A_AUDIO_SOURCE_DS_2						1
	#define			X0A_AUDIO_SOURCE_DS_4						2
	#define			X0A_AUDIO_SOURCE_SEL						4:3
	#define			X0A_AUDIO_SOURCE_SEL_I2S				0
	#define			X0A_AUDIO_SOURCE_SEL_SPDIF			1
	#define			X0A_AUDIO_SOURCE_SEL_DSD				2
	#define			X0A_AUDIO_SOURCE_SEL_HBR				3
	#define			X0A_AUDIO_SOURCE_MCLK						2:2
	#define			X0A_AUDIO_SOURCE_MCLK_OFF				0
	#define			X0A_AUDIO_SOURCE_MCLK_ON				1
	#define			X0A_AUDIO_SOURCE_MRATIO				1:0
	#define			X0A_AUDIO_SOURCE_MRATIO_128FS		0
	#define			X0A_AUDIO_SOURCE_MRATIO_256FS		1
	#define			X0A_AUDIO_SOURCE_MRATIO_384FS		2
	#define			X0A_AUDIO_SOURCE_MRATIO_512FS		3
	
#define     X0B_AUDIO_SET2         			  11      //  RW  00h     Audio setting.2
	#define			X0B_AUDIO_SET2_IDCLK					7:7
	#define			X0B_AUDIO_SET2_IDCLK1					0						//BIT.7
	#define			X0B_AUDIO_SET2_IDCLK1_2				1						//BIT.7
	#define			X0B_AUDIO_SET2_PRI						6:6
	#define			X0B_AUDIO_SET2_PRI_ASP				0
	#define			X0B_AUDIO_SET2_PRI_ACR				1
	#define			X0B_AUDIO_SET2_IDCLK_MODE					5:5
	#define			X0B_AUDIO_SET2_IDCLK_MODE_AUTO			0						//BIT.5
	#define			X0B_AUDIO_SET2_IDCLK_MODE_FIX  		1						//BIT.5
	#define			X0B_AUDIO_SET2_FLATLINE				4:4
	#define			X0B_AUDIO_SET2_FLATLINE_NORMAL  0
	#define			X0B_AUDIO_SET2_FLATLINE_FLAT  	1
	#define			X0B_AUDIO_SET2_CHANNEL				3:0         //NOT ALL LIST 
	#define			X0B_AUDIO_SET2_CHANNEL_NOT		0
	#define			X0B_AUDIO_SET2_CHANNEL_LEFT		8
	#define			X0B_AUDIO_SET2_CHANNEL_RIGHT	4

#define     X0C_I2S_MODE           					  12      //  RW  00h     I2S audio setting
	#define			X0C_I2S_MODE_MCLKFQ									7:7	
	#define			X0C_I2S_MODE_MCLKFQ_BY0AH						0
	#define			X0C_I2S_MODE_MCLKFQ_OVERRIDE				1
	#define			X0C_I2S_MODE_CHANNEL  							5:2
	#define			X0C_I2S_MODE_CHANNEL_2							1
	#define			X0C_I2S_MODE_CHANNEL_4							3
	#define			X0C_I2S_MODE_CHANNEL_6						  7
	#define			X0C_I2S_MODE_CHANNEL_8							15
	#define			X0C_I2S_MODE_SEL										1:0
	#define			X0C_I2S_MODE_SEL_STANDARD						0
	#define			X0C_I2S_MODE_SEL_JSTF_RIGHT					1
	#define			X0C_I2S_MODE_SEL_JSTF_LEFT					2

#define     X0D_DSD_MODE            13      //  RW  00h     DSD audio setting
	#define			X0D_DSD_MODE_CHANNEL						7:4
	#define			X0D_DSD_MODE_CHANNEL_2					1
	#define			X0D_DSD_MODE_CHANNEL_4					3
	#define			X0D_DSD_MODE_CHANNEL_6					7
	#define			X0D_DSD_MODE_CHANNEL_8					0XF		

#define     X0E_DEBUG_MONITOR1      14      //  RO  00h     Reserved
#define     X0F_DEBUG_MONITOR2      15      //  RO  00h     Reserved
#define     X10_I2S_PINMODE         16      //  RW  00h     I2S input pin swap

#define     X11_ASTATUS1            17      //  RW  00h     Audio status bits setting1
	#define     X11_ASTATUS1_VALID          	  7:7
	#define     X11_ASTATUS1_FREQ            		3:0
	#define     X11_ASTATUS1_FREQ_44_1K       	15
	#define     X11_ASTATUS1_FREQ_88_2K       	7
	#define     X11_ASTATUS1_FREQ_22_05K     	  11
	#define     X11_ASTATUS1_FREQ_176_4K     	  3
	#define     X11_ASTATUS1_FREQ_48K       		13
	#define     X11_ASTATUS1_FREQ_96K	       		5
	#define     X11_ASTATUS1_FREQ_24K       		9
	#define     X11_ASTATUS1_FREQ_192K       		1
	#define     X11_ASTATUS1_FREQ_8K       			6
	#define     X11_ASTATUS1_FREQ_11_025K    	  10
	#define     X11_ASTATUS1_FREQ_12K       		2
	#define     X11_ASTATUS1_FREQ_32K		       	12
	#define     X11_ASTATUS1_FREQ_16K       		8
	#define     X11_ASTATUS1_FREQ_DEFAUT       	0
#define     X12_ASTATUS2            18      //  RW  00h     Audio status bits setting2
	#define     X12_ASTATUS2_CS1		    			 	7:7
	#define     X12_ASTATUS2_CS1_LPCM		 				0
	#define     X12_ASTATUS2_CS1_NONLPCM				1
	#define     X12_ASTATUS2_CS0								6:6
	#define     X12_ASTATUS2_CS0_CONSUMER			0
	#define     X12_ASTATUS2_CS0_PROFESSIONAL 	1
	#define     X12_ASTATUS2_COPYRIGHT					5:5
	#define     X12_ASTATUS2_ADDITION						4:2
	#define     X12_ASTATUS2_ADDITION_PREM_NO		0
	#define     X12_ASTATUS2_ADDITION_PREM_50US	1
	#define     X12_ASTATUS2_CLKLEVEL						1:0
	#define     X12_ASTATUS2_CLKLEVEL_II				0
	#define     X12_ASTATUS2_CLKLEVEL_I					1
	#define     X12_ASTATUS2_CLKLEVEL_III				2
	
#define     X13_CAT_CODE            19      //  RW  00h     Category code
#define     X14_A_SOURCE            20      //  RW  00h     Source number/ Audio word length
#define     X14_A_SOURCE_NUM        7:4
	#define     X14_A_SOURCE_NUM_NOCOUNT             0
	#define     X14_A_SOURCE_NUM_1			         1
	#define     X14_A_SOURCE_NUM_2			         2
	#define     X14_A_SOURCE_NUM_3			         3
	#define     X14_A_SOURCE_NUM_4			         4
	#define     X14_A_SOURCE_NUM_5			         5
	#define     X14_A_SOURCE_NUM_6			         6
	#define     X14_A_SOURCE_NUM_7			         7
	#define     X14_A_SOURCE_NUM_8			         8
	#define     X14_A_SOURCE_NUM_9			         9
	#define     X14_A_SOURCE_NUM_10			         10
	#define     X14_A_SOURCE_NUM_11			         11
	#define     X14_A_SOURCE_NUM_12			         12
	#define     X14_A_SOURCE_NUM_13			         13
	#define     X14_A_SOURCE_NUM_14			         14
	#define     X14_A_SOURCE_NUM_15			         15
	#define     X14_A_SOURCE_LENGTH        		     3:0
	#define     X14_A_SOURCE_LENGTH_16B				 2		
	#define     X14_A_SOURCE_LENGTH_17B				 12
	#define     X14_A_SOURCE_LENGTH_18B				 4
	#define     X14_A_SOURCE_LENGTH_19B				 8
	#define     X14_A_SOURCE_LENGTH_20B				 10
	#define     X14_A_SOURCE_LENGTH_20BIT	  	     3
	#define     X14_A_SOURCE_LENGTH_21B				 13
	#define     X14_A_SOURCE_LENGTH_22B				 5
	#define     X14_A_SOURCE_LENGTH_23B				 9
	#define     X14_A_SOURCE_LENGTH_24B				 11
	
#define     X15_AVSET1              21      //  RW  00h     Audio/Video setting.1
		#define			X15_AVSET1_AUDFREQ				 7:4
		#define			X15_AVSET1_AUDFREQ_32K 		 3
		#define			X15_AVSET1_AUDFREQ_44_1K	 0
		#define			X15_AVSET1_AUDFREQ_48K		 2
		#define			X15_AVSET1_AUDFREQ_88_2K	 8
		#define			X15_AVSET1_AUDFREQ_96K		 10
		#define			X15_AVSET1_AUDFREQ_176_4K	 12
		#define			X15_AVSET1_AUDFREQ_192K	 	 14
		#define			X15_AVSET1_AUDFREQ_768K	 	 9
		#define			X15_AVSET1_VIDFORM							 								3:1				
		#define			X15_AVSET1_VIDFORM_RGB_YCC444		        		  	 0
		#define			X15_AVSET1_VIDFORM_YCC422				   			 			   1
		#define			X15_AVSET1_VIDFORM_YCC422_SAVEAV           	     2
		#define			X15_AVSET1_VIDFORM_SEPA_SYNCS		        		  	 3	
		#define			X15_AVSET1_VIDFORM_EMBED_SYNCS	          	     4	
		#define			X15_AVSET1_VIDFORM_RGB444_YCC444       		       5		
		#define			X15_AVSET1_VIDFORM_DDR_YCC422				     				 6
		#define			X15_AVSET1_DE																		 0:0
		#define			X15_AVSET1_DE_INTERNAL				 	   				  		  0
		#define			X15_AVSET1_DE_EXTERNAL					  						   1

#define     X16_VIDEO1              22      //  RW  34h     Video setting.1
		#define			X16_VIDEO1_OUT							 7:6
		#define			X16_VIDEO1_OUT_RGB444				     0
		#define			X16_VIDEO1_OUT_YCC444				     1
		#define			X16_VIDEO1_OUT_YCC422				     2
		#define			X16_VIDEO1_IN							 5:4
		#define			X16_VIDEO1_IN_WID_12				     0
		#define			X16_VIDEO1_IN_WID_10				     1
		#define			X16_VIDEO1_IN_WID_8					     3
		#define			X16_VIDEO1_SAV_EAV					     3:2
		#define			X16_VIDEO1_SAV_EAV_CH0			         0
		#define			X16_VIDEO1_SAV_EAV_CH1			         1
		#define			X16_VIDEO1_SAV_EAV_CH2			         2
		#define			X16_VIDEO1_IN_COLOR					     0:0
		#define			X16_VIDEO1_IN_COLOR_RGB			         0
		#define			X16_VIDEO1_IN_COLOR_YCC			         1

#define     X17_DC_REG              23      //  RW  20h     Deep color setting
		#define	X17_DC_REG_TMDS								 7:6
		#define	X17_DC_REG_TMDS_8b						     0
		#define	X17_DC_REG_TMDS_10b						     1
		#define	X17_DC_REG_TMDS_12b						     2				
		#define	X17_DC_REG_FIFO								 5:5
		#define	X17_DC_REG_ESART							 4:4
		#define	X17_DC_REG_ELAST							 3:3
		#define	X17_DC_REG_EBYTE							 2:2
		#define	X17_DC_REG_EXTCLR							 1:1
		#define	X17_DC_REG_EXTCLR_SOF					     0
		#define	X17_DC_REG_EXTCLR_SOH					     1
		
#define     X18_CSC_C0_HI           24      //  RW  04h     Color Space Conversion Parameters		
#define     X19_CSC_C0_LO           25      //  RW  00h
#define     X1A_CSC_C1_HI           26      //  RW  05h     Color Space Conversion Parameters
#define     X1B_CSC_C1_LO           27      //  RW  09h
#define     X1C_CSC_C2_HI           28      //  RW  00h
#define     X1D_CSC_C2_LO           29      //  RW  00h
#define     X1E_CSC_C3_HI           30      //  RW  02h
#define     X1F_CSC_C3_LO           31      //  RW  A1h
#define     X20_CSC_C4_HI           32      //  RW  04h
#define     X21_CSC_C4_LO           33      //  RW  00h
#define     X22_CSC_C5_HI           34      //  RW  12h
#define     X23_CSC_C5_LO           35      //  RW  91h
#define     X24_CSC_C6_HI           36      //  RW  11h
#define     X25_CSC_C6_LO           37      //  RW  59h
#define     X26_CSC_C7_HI           38      //  RW  00h
#define     X27_CSC_C7_LO           39      //  RW  7Dh
#define     X28_CSC_C8_HI           40      //  RW  04h
#define     X29_CSC_C8_LO           41      //  RW  00h
#define     X2A_CSC_C9_HI           42      //  RW  00h
#define     X2B_CSC_C9_LO           43      //  RW  00h
#define     X2C_CSC_C10_HI          44      //  RW  06h
#define     X2D_CSC_C10_LO          45      //  RW  EFh
#define     X2E_CSC_C11_HI          46      //  RW  02h
#define     X2F_CSC_C11_LO          47      //  RW  DDh
    
#define X30_EXT_VPARAMS                                         48      //  RW  00h     External video parameter settings
#define X30_EXT_VPARAMS_VSYNC_OFFSET               7:4
#define X30_EXT_VPARAMS_VSYNC_PHASE                 3:3
#define X30_EXT_VPARAMS_VSYNC_PHASE_NEG  	 0
#define X30_EXT_VPARAMS_VSYNC_PHASE_POS		 1
#define X30_EXT_VPARAMS_HSYNC_PHASE                 2:2
#define X30_EXT_VPARAMS_HSYNC_PHASE_NEG 	 0
#define X30_EXT_VPARAMS_HSYNC_PHASE_POS		 1
#define X30_EXT_VPARAMS_PI                                     1:1
#define X30_EXT_VPARAMS_PI_PROGRESSIVE             0
#define X30_EXT_VPARAMS_PI_INTERLACE			 1
#define X30_EXT_VPARAMS_USE                                  0:0
#define X30_EXT_VPARAMS_USE_PRE_PROG                0
#define X30_EXT_VPARAMS_USE_EXTERNAL                1

#define     X31_EXT_HTOTAL          49      //  RW  00h     External horizontal total
#define     X32_EXT_HTOTAL          50      //
#define     X33_EXT_HBLANK          51      //  RW  00h     External horizontal blank
#define     X34_EXT_HBLANK          52      //
#define     X35_EXT_HDLY            53      //  RW  00h     External horizontal delay
#define     X36_EXT_HDLY            54      //
#define     X37_EXT_HS_DUR          55      //  RW  00h     External horizontal duration
#define     X38_EXT_HS_DUR          56      //
#define     X39_EXT_VTOTAL          57      //  RW  00h     External vertical total
#define     X3A_EXT_VTOTAL          58      //
#define     X3B_AVSET2                              59              //  RW                                          00h     Audio/Video setting.2
		#define X3B_AVSET2_CTS_DEBUG				 7:7
		#define X3B_AVSET2_CD_ZERO					 6:6	
		#define X3B_AVSET2_EXT_DE					 5:5
		#define X3B_AVSET2_DCC_I2C_RST			     4:4
		#define X3B_AVSET2_VID_CODE39				 3:3
		#define X3B_AVSET2_BLACK_FULL		 		 1:1
		#define X3B_AVSET2_CSC		 				 0:0
//				#define     CSC_ENABLE                                  0x01    //          R/W                                     1b  Color Space Conversion enable
//        #define     SEL_FULL_RANGE                              0x02    //          R/W                                     1b  Select Full/Limited range for Send black video mode
//        #define     EN_M0_LOAD                                  0x04    //          R/W                                     1b  Load M0 into Akey area
//        #define     EXT_DE_CNT                                  0x20    //          R/W                                     1b  External DE control
//        #define     CD_ZERO                                     0x40    //          R/W                                     1b  CD all zero override
//        #define     CTS_DEBUG                                   0x80    //          R/W                                     1b  Debug bit for CTS timing
#define     X3C_EX_VID              60      //  RW  00h     External input Video ID(VID)
#define     X3D_EXT_VBLANK          61      //  RW  00h     External virtical blank
#define     X3E_EXT_VDLY            62      //  RW  00h     External virtical delay
#define     X3F_EXT_VS_DUR          63      //  RW  00h     External virtical duration
//				#define     X40_CTRL_PKT_EN         64      //  RW  00h     Control packet enable
	#define X40_CTRL_PKT_EN										   0X40
		#define X40_CTRL_PKT_EN_GENE_CTRL					    						  7:7
		#define X40_CTRL_PKT_EN_GENE_CTRL_DIS					 						   0
		#define X40_CTRL_PKT_EN_GENE_CTRL_EN					  						  1
		#define X40_CTRL_PKT_EN_MPEG                                6:6
		#define X40_CTRL_PKT_EN_MPEG_DIS														0
		#define X40_CTRL_PKT_EN_MPEG_EN															1
		#define X40_CTRL_PKT_EN_PROD                                5:5
		#define X40_CTRL_PKT_EN_PROD_DIS							 						   0
		#define X40_CTRL_PKT_EN_PROD_EN						     						   1
		#define X40_CTRL_PKT_EN_VENDOR                              4:4
		#define X40_CTRL_PKT_EN_VENDOR_DIS					    						0
		#define X40_CTRL_PKT_EN_VENDOR_EN														1
		#define X40_CTRL_PKT_EN_GAMUT                               3:3
		#define X40_CTRL_PKT_EN_GAMUT_DIS														0
		#define X40_CTRL_PKT_EN_GAMUT_EN														1
		#define X40_CTRL_PKT_EN_ISRC                                2:2
		#define X40_CTRL_PKT_EN_ISRC_DIS														0
		#define X40_CTRL_PKT_EN_ISRC_EN															1
		#define X40_CTRL_PKT_EN_ACP	                                1:1
		#define X40_CTRL_PKT_EN_ACP_DIS															0
		#define X40_CTRL_PKT_EN_ACP_EN							 								1
		#define X40_CTRL_PKT_EN_GENE                                0:0
		#define X40_CTRL_PKT_EN_GENE_DIS															0
		#define X40_CTRL_PKT_EN_GENE_EN																1

#define     X41_SEND_CPKT_AUTO      65      //  RW  00h     HB0 for generic control packet
		#define X41_SEND_CPKT_AUTO_GENE_CTRL                        7:7
		#define X41_SEND_CPKT_AUTO_GENE_CTRL_DIS			    					0
		#define X41_SEND_CPKT_AUTO_GENE_CTRL_EN					  				  1
		#define X41_SEND_CPKT_AUTO_MPEG                             6:6
		#define X41_SEND_CPKT_AUTO_MPEG_DIS													0
		#define X41_SEND_CPKT_AUTO_MPEG_EN													1
		#define X41_SEND_CPKT_AUTO_PROD                             5:5
		#define X41_SEND_CPKT_AUTO_PROD_DIS													0
		#define X41_SEND_CPKT_AUTO_PROD_EN													1
		#define X41_SEND_CPKT_AUTO_VENDOR                           4:4
		#define X41_SEND_CPKT_AUTO_VENDOR_DIS												0
		#define X41_SEND_CPKT_AUTO_VENDOR_EN												1
		#define X41_SEND_CPKT_AUTO_GAMUT                            3:3
		#define X41_SEND_CPKT_AUTO_GAMUT_DIS												0
		#define X41_SEND_CPKT_AUTO_GAMUT_EN													1
		#define X41_SEND_CPKT_AUTO_ISRC                             2:2
		#define X41_SEND_CPKT_AUTO_ISRC_DIS													0
		#define X41_SEND_CPKT_AUTO_ISRC_EN													1
		#define X41_SEND_CPKT_AUTO_ACP                              1:1
		#define X41_SEND_CPKT_AUTO_ACP_DIS													0
		#define X41_SEND_CPKT_AUTO_ACP_EN                           1
		#define X41_SEND_CPKT_AUTO_GENERIC                          0:0
		#define X41_SEND_CPKT_AUTO_GENERIC_DIS				    			  	0
		#define X41_SEND_CPKT_AUTO_GENERIC_EN												1
		
#define     X42_AUTO_CHECKSUM       66      //  RW  00h     Auto checksum option
#define X42_AUTO_CHECKSUM_OPT						0:0
#define X42_AUTO_CHECKSUM_OPT_DIS	    	0
#define X42_AUTO_CHECKSUM_OPT_EN		    1

//  #define     RESERVED                67      //  RW  00h
//  #define     RESERVED                68      //  RW  00h
#define     X45_VIDEO2                              69              //  RW                                          00h     Video setting.2
		#define X45_VIDEO2_CLR_AVMUTE						    7:7
		#define X45_VIDEO2_CLR_AVMUTE_DIS				    	0
		#define X45_VIDEO2_CLR_AVMUTE_EN				        1
		#define X45_VIDEO2_SET_AVMUTE					    	6:6
		#define X45_VIDEO2_SET_AVMUTE_DIS					    0
		#define X45_VIDEO2_SET_AVMUTE_EN				    	1
		#define X45_VIDEO2_AUDIO_RESET						    2:2
		#define X45_VIDEO2_AUDIO_RESET_CLR			 	        0
		#define X45_VIDEO2_AUDIO_RESET_SET			            1
		#define X45_VIDEO2_NOAUDIO							    1:1
		#define X45_VIDEO2_NOAUDIO_CLR			 	        0
		#define X45_VIDEO2_NOAUDIO_SET			            1
		#define X45_VIDEO2_NOVIDEO					        0:0
		#define X45_VIDEO2_NOVIDEO_CLR			 	        0
		#define X45_VIDEO2_NOVIDEO_SET			            1
//				#define     NOVIDEO                                     0x01    //          R/W                                     1b  Send black video
//        #define     NOAUDIO                                     0x02    //          R/W                                     1b  Send no audio
//        #define     AUDIORST                                    0x04    //          R/W                                     1b  Audio capture logic reset
//        #define     SET_AV_MUTE                                 0x40    //          R/W                                     1b  Send gSet AV muteh
//        #define     CLEAR_AV_MUTE                               0x80    //          R/W                                     1b  Clear AV muteh
//  #define     RESERVED                70      //  RW  00h     Reserved
    #define     X46_OUTPUT_OPTION                70      //  RW  00h     Reserved
//    #define     X47_VIDEO4              71      //  RW  00h     Video setting.4
//    #define     X48_ACT_LN_STRT_LSB     72      //  RW  00h     Active Line Start LSB
//    #define     X49_ACT_LN_STRT_MSB     73      //  RW  00h     Active Line Start MSB
//    #define     X4A_ACT_LN_END_LSB      74      //  RW  00h     Active Line End LSB
//    #define     X4B_ACT_LN_END_MSB      75      //  RW  00h     Active Line End MSB
//    #define     X4C_ACT_PIX_STRT_LSB    76      //  RW  00h     Active Pixel Start LSB
//    #define     X4D_ACT_PIX_STRT_MSB    77      //  RW  00h     Active Pixel Start MSB
//    #define     X4E_ACT_PIX_END_LSB     78      //  RW  00h     Active Pixel End LSB
//    #define     X4F_ACT_PIX_END_MSB     79      //  RW  00h     Active Pixel End MSB
//    #define     X50_EXT_AUDIO_SET       80      //  RW  00h     Extra audio setting
//SLI10131
//    #define     X51_SPEAKER_MAP         81      //  RW  00h     speaker Mapping CA[7:0]
//IP V2.11
#define     X51_PHY_CTRL            81      //  RW  00h     Revervd[7:4],PHY_OPTION[3:0]

#define     X52_HSYNC_PLACE_656     82      //  RW  00h     HSYNC placement at ITU656
#define     X53_HSYNC_PLACE_656     83      //
#define     X54_VSYNC_PLACE_656     84      //  RW  00h     VSYNC placement at ITU656
#define     X55_VSYNC_PLACE_656     85      //
	#define     X55_VSYNC_PLACE_656_EN     			             7:7
	#define     X55_VSYNC_PLACE_656_EN_INACT                     0
	#define     X55_VSYNC_PLACE_656_EN_ACT   	                 1
#define     X56_PHY_CTRL            86      //  RW  0Fh     SLIPHDMIT parameter settings
	#define X56_PHY_CTRL_DRV_TEST_IN							 7:7
	#define X56_PHY_CTRL_DRV_TEST_EN							 6:6
	#define X56_PHY_CTRL_PLLA_BYPAS								 4:4
	#define X56_PHY_CTRL_SPEED_B									 3:2
	#define X56_PHY_CTRL_SPEED_A					             1:0

#define     X57_PHY_CTRL            87      //  RW  00h
	#define	X57_PHY_CTRL_PLLB_CONFIG17					     	 2:2
	#define	X57_PHY_CTRL_PLLA_CONFIG17					    	 1:1
#define     X58_PHY_CTRL            88      //  RW  00h
		#define X58_PHY_CTRL_BGR_DISC							 7:7
		#define X58_PHY_CTRL_BGR_V_OFFSET						 6:4				//BIT6-4
		#define X58_PHY_CTRL_BGR_I_OFFSET						 2:0				//BIT2-0
				
#define     X59_PHY_CTRL            89      //  RW  20h
#define     X5A_PHY_CTRL            90      //  RW  00h
#define     X5B_PHY_CTRL            91      //  RW  20h
#define     X5C_PHY_CTRL            92      //  RW  00h

#define     X5D_PHY_CTRL            93      //  RW  00h
		#define	X5D_PHY_CTRL_DRV_CONFIG2_0			6:4																		//BIT6-4
		#define	X5D_PHY_CTRL_PE_CNTRL2_0				         2:0					//BIT2-0				
#define     X5E_PHY_CTRL         					             94      												//  RW  00h
		#define	X5E_PHY_CTRL_PLLB_CONFIG16						 5:5
		#define	X5E_PHY_CTRL_PLLA_CONFIG16						 4:4
		#define	X5E_PHY_CTRL_AMON_SEL2_0					   	 2:0
#define     X5F_PACKET_INDEX                        95              //  R/W                                         00h     Packet Buffer Index
        #define     GENERIC_PACKET                              0x00    //          R/W                                     00h Generic packet
        #define     ACP_PACKET                                  0x01    //          R/W                                     00h ACP packet
        #define     ISRC1_PACKET                                0x02    //          R/W                                     00h ISRC1 packet
        #define     ISRC2_PACKET                                0x03    //          R/W                                     00h ISRC2 packet
        #define     GAMUT_PACKET                                0x04    //          R/W                                     00h Gamut metadata packet
        #define     VENDOR_INFO_PACKET                          0x05    //          R/W                                     00h Vendor specific InfoFrame
        #define     AVI_INFO_PACKET                             0x06    //          R/W                                     00h AVI InfoFrame
        #define     PRODUCT_INFO_PACKET                         0x07    //          R/W                                     00h Source product descriptor InfoFrame
        #define     AUDIO_INFO_PACKET                           0x08    //          R/W                                     00h Audio InfoFrame packet
        #define     MPEG_SRC_INFO_PACKET                        0x09    //          R/W                                     00h MPEG source InfoFrame
#define     X60_PACKET_HB0          96      //  RW  00h     HB0
#define     X61_PACKET_HB1          97      //  RW  00h     HB1
#define     X62_PACKET_HB2          98      //  RW  00h     HB2
#define     X63_PACKET_PB0          99      //  RW  00h     PB0
#define     X64_PACKET_PB1          100     //  RW  00h
#define     X65_PACKET_PB2          101     //  RW  00h
#define     X66_PACKET_PB3          102     //  RW  00h
#define     X67_PACKET_PB4          103     //  RW  00h
#define     X68_PACKET_PB5          104     //  RW  00h
#define     X69_PACKET_PB6          105     //  RW  00h
#define     X6A_PACKET_PB7          106     //  RW  00h
#define     X6B_PACKET_PB8          107     //  RW  00h
#define     X6C_PACKET_PB9          108     //  RW  00h
#define     X6D_PACKET_PB10         109     //  RW  00h
#define     X6E_PACKET_PB11         110     //  RW  00h
#define     X6F_PACKET_PB12         111     //  RW  00h
#define     X70_PACKET_PB13         112     //  RW  00h
#define     X71_PACKET_PB14         113     //  RW  00h
#define     X72_PACKET_PB15         114     //  RW  00h
#define     X73_PACKET_PB16         115     //  RW  00h
#define     X74_PACKET_PB17         116     //  RW  00h
#define     X75_PACKET_PB18         117     //  RW  00h
#define     X76_PACKET_PB19         118     //  RW  00h
#define     X77_PACKET_PB20         119     //  RW  00h
#define     X78_PACKET_PB21         120     //  RW  00h
#define     X79_PACKET_PB22         121     //  RW  00h
#define     X7A_PACKET_PB23         122     //  RW  00h
#define     X7B_PACKET_PB24         123     //  RW  00h
#define     X7C_PACKET_PB25         124     //  RW  00h
#define     X7D_PACKET_PB26         125     //  RW  00h
#define     X7E_PACKET_PB27         126     //  RW  00h     PB27
//  #define     RESERVED                127     //  Reserved
#define     X80_EDID                128     //  RO  -       Access window for EDID buffer
#define     X81_ISRC2_PB0           129     //  RW  00h     ISRC2 PB0-15
#define     X82_ISRC2_PB1           130     //  RW  00h


//		#define     X83_ISRC2_PB2           131     //  RW  00h
//    #define     X84_ISRC2_PB3           132     //  RW  00h
//    #define     X85_ISRC2_PB4           133     //  RW  00h
//    #define     X86_ISRC2_PB5           134     //  RW  00h
//    #define     X87_ISRC2_PB6           135     //  RW  00h
//    #define     X88_ISRC2_PB7           136     //  RW  00h
//    #define     X89_ISRC2_PB8           137     //  RW  00h
//    #define     X8A_ISRC2_PB9           138     //  RW  00h     ISRC2 PB0-15
//    #define     X8B_ISRC2_PB10          139     //  RW  00h
//    #define     X8C_ISRC2_PB11          140     //  RW  00h
//    #define     X8D_ISRC2_PB12          141     //  RW  00h
//    #define     X8E_ISRC2_PB13          142     //  RW  00h
//    #define     X8F_ISRC2_PB14          143     //  RW  00h
//    #define     X90_ISRC2_PB15          144     //  RW  00h
//    #define     X91_ISRC1_HB1           145     //  RW  00h     ISRC2 HB1
#define     X92_INT_MASK1                   146             //  RW                                          C0h     Mask for Interrupt Group1
		#define X92_INT_MASK1_HPG										 7:7        //'1B' NO MASKED
		#define X92_INT_MASK1_MSENS										 6:6				 //'1B' NO MASKED
		#define X92_INT_MASK1_VSYNC										 5:5				 //'1B' NO MASKED
		#define X92_INT_MASK1_AFIFO_FULL								 4:4				 //'1B' NO MASKED
		#define X92_INT_MASK1_EDID_RDY									 2:2				 //'1B' NO MASKED
		#define X92_INT_MASK1_EDID_ERR									 1:1				 //'1B' NO MASKED
				

//				#define     EDID_ERR_MSK                        0x02    //          R/W                                     1b  EDID error detect interrupt mask
//        #define     EDID_RDY_MSK                        0x04    //          R/W                                     1b  EDID ready interrupt mask
//        #define     AFIFO_FULL_MSK                      0x10    //          R/W                                     0b  Audio FIFO full detect interrupt mask
//        #define     VSYNC_MSK                           0x20    //          R/W                                     0b  VSYNC detect interrupt mask
//        #define     MSENS_MSK                           0x40    //          R/W                                     0b  MSENS detect interrupt mask
//        #define     HPG_MSK                             0x80    //          R/W                                     0b  Hot plug detect interrupt mask
#define     X93_INT_MASK2                   147             //  RW                                          78h     Mask for Interrupt Group2
		#define X93_INT_MASK2_HDCP_ERR									 7:7				 //'1B' NO MASKED
		#define X93_INT_MASK2_BKSV_RPRDY								 6:6				 //'1B' NO MASKED
		#define X93_INT_MASK2_BKSV_RCRDY								 5:5				 //'1B' NO MASKED
		#define X93_INT_MASK2_AUTH_DONE									 4:4				 //'1B' NO MASKED
		#define X93_INT_MASK2_RDY_AUTH									 3:3				 //'1B' NO MASKED
			

//				#define     RDY_AUTH_MSK                        0x08    //          R/W                                     1b  Authentication ready interrupt mask
//        #define     AUTH_DONE_MSK                       0x10    //          R/W                                     1b  HDCP authentication done interrupt mask
//        #define     BKSV_RCRDY_MSK                      0x20    //          R/W                                     1b  BKSV ready from receiver interrupt mask
//        #define     BKSV_RPRDY_MSK                      0x40    //          R/W                                     1b  BKSVs list ready from repeater interrupt mask
//        #define     HDCP_ERR_MSK                        0x80    //          R/W                                     0b  HDCP error detect interrupt mask
#define     X94_INT1_ST                     148             //  RW                                          00h     Interrupt status Group1
		#define X94_INT1_ST_HPG											 7:7				// 1b means Interrupted
		#define X94_INT1_ST_HPG_EN										 1
		#define X94_INT1_ST_HPG_NO										 0
		#define X94_INT1_ST_MSENS									 	 6:6				// 1b means Interrupted		
		#define X94_INT1_ST_MSENS_EN									 1
		#define X94_INT1_ST_MSENS_NO									 0
		#define X94_INT1_ST_VSYNC										 5:5				// 1b means Interrupted	
		#define X94_INT1_ST_VSYNC_EN									 1
		#define X94_INT1_ST_VSYNC_NO									 0
		#define X94_INT1_ST_AFIFO_FULL							         4:4				// 1b means Interrupted	
		#define X94_INT1_ST_AFIFO_FULL_EN							     1
		#define X94_INT1_ST_AFIFO_FULL_NO							     0
		#define X94_INT1_ST_EDID_RDY								     2:2				// 1b means Interrupted	
		#define X94_INT1_ST_EDID_RDY_EN								     1
		#define X94_INT1_ST_EDID_RDY_NO								     0
		#define X94_INT1_ST_EDID_ERR								     1:1				// 1b means Interrupted		
		#define X94_INT1_ST_EDID_ERR_EN								     1
		#define X94_INT1_ST_EDID_ERR_NO								     0
		
//				#define     EDID_ERR_INT                        0x02    //          R/W                                     0b  EDID error detect interrupt
//        #define     EDID_RDY_INT                        0x04    //          R/W                                     0b  EDID ready interrupt
//        #define     AFIFO_FULL_INT                      0x10    //          R/W                                     0b  Audio FIFO full detect interrupt
//        #define     VSYNC_INT                           0x20    //          R/W                                     0b  VSYNC detect interrupt
//        #define     MSENS_INT                           0x40    //          R/W                                     0b  MSENS detect interrupt
//        #define     HPG_INT                             0x80    //          R/W                                     0b  Hot plug detect interrupt
#define     X95_INT2_ST                     149             //  RW                                          00h     Interrupt status Group2
		#define X95_INT2_ST_HDCP_ERR            		7:7 
		#define X95_INT2_ST_BKSV_RPRDY            	    6:6
		#define X95_INT2_ST_BKSV_RCRDY            	    5:5
		#define X95_INT2_ST_AUTH_DONE            	 	4:4
		#define X95_INT2_ST_RDY_AUTH            	 	3:3
				
//				#define     RDY_AUTH_INT                        0x08    //          R/W                                     0b  Authentication ready interrupt
//        #define     AUTH_DONE_INT                       0x10    //          R/W                                     0b  HDCP authentication done interrupt
//        #define     BKSV_RCRDY_INT                      0x20    //          R/W                                     0b  BKSV ready from receiver interrupt
//        #define     BKSV_RPRDY_INT                      0x40    //          R/W                                     0b  BKSVs list ready from repeater interrupt
//        #define     HDCP_ERR_INT                        0x80    //          R/W                                     0b  HDCP error detect interrupt
#define     X96_INT_MASK3                   0X96
#define     X97_INT_MASK3                   0X97
#define     X98_INT3_ST                   	0X98
		#define     X98_INT3_ST_SF_MODE_READY           7:7
		#define     X98_INT3_ST_RI_READY     			6:6
		#define     X98_INT3_ST_PJ_READY     			5:5
		#define     X98_INT3_ST_AN_READY     			4:4
		#define     X98_INT3_ST_SHA1_READY     		    3:3
		#define     X98_INT3_ST_ENC_EN     				2:2
		#define     X98_INT3_ST_ENC_DIS_AVMUTE 		    1:1
		#define     X98_INT3_ST_ENC_DIS_NO_AVMUTE       0:0
		
#define     X99_INT4_ST               	    0X99
		#define     X99_INT4_ST_I2C_ACK              7:7
		#define     X99_INT4_ST_I2C_ERR_ACK          6:6
		#define     X99_INT4_ST_RI_SAVE_READY        5:5
		#define     X99_INT4_ST_PJ_SAVE_READY        4:4
		#define     X99_INT4_ST_FR_CNT_UPDATE        3:3
		
#define     X9A_HDCP_CTRL1               	    0X9A
		#define     X9A_HDCP_CTRL1_AUTH                7:7
		#define     X9A_HDCP_CTRL1_AUTH_HWARE          0
		#define     X9A_HDCP_CTRL1_AUTH_SWARE          1
		#define     X9A_HDCP_CTRL1_SOFT				   5:5
		#define     X9A_HDCP_CTRL1_SOFT_CLR            0
		#define     X9A_HDCP_CTRL1_SOFT_SET            1
		#define     X9A_HDCP_CTRL1_PREP_AN             4:4
		#define     X9A_HDCP_CTRL1_REPEAT        	   2:2
		#define     X9A_HDCP_CTRL1_REPEAT_CLR    	   0
		#define     X9A_HDCP_CTRL1_REPEAT_SET    	   1
		#define     X9A_HDCP_CTRL1_START_AUTH          1:1
		#define     X9A_HDCP_CTRL1_CALC_SHA1   		   0:0
		
#define     X9C_FRAME_CNT              	    		0X9C
#define     X9D_FRAME_CNT_RI              	        0X9D
#define     X9E_DDC_ACS_LENGTH          	   	    0X9E
#define     XA0_I2C_OFFSET             	    		0XA0
#define     XA1_DDC_I2C_CTRL            	    	0XA1
		#define     XA1_DDC_I2C_CTRL_SEL            	  2:2
		#define     XA1_DDC_I2C_CTRL_SEL_BUF0_4           0
		#define     XA1_DDC_I2C_CTRL_SEL_80H           	  1
		#define     XA1_DDC_I2C_CTRL_W_START              1:1
		#define     XA1_DDC_I2C_CTRL_R_START              0:0
#define     XA2_DDC_I2C_RBUF0            	    	0XA2
#define     XA3_DDC_I2C_RBUF1            	    	0XA3
#define     XA4_DDC_I2C_RBUF2            	    	0XA4
#define     XA5_DDC_I2C_RBUF3            	    	0XA5
#define     XA6_DDC_I2C_RBUF4            	    	0XA6
#define     XA7_DDC_I2C_WBUF0            	    	0XA7
#define     XA8_DDC_I2C_WBUF1            	    	0XA8
#define     XA9_DDC_I2C_WBUF2            	    	0XA9
#define     XAA_DDC_I2C_WBUF3            	   		0XAA
#define     XAB_DDC_I2C_WBUF4            	   		0XAB
#define     XAC_DDC_I2C_WBUF5            	   		0XAC
#define     XAD_DDC_I2C_WBUF6            	   		0XAD
#define     XAE_DDC_I2C_WBUF7            	   		0XAE



//  #define     VN1                     150     //  RW  00h     Generic control packet (PB1-PB23)
//  #define     VN2                     151     //  RW  00h
//  #define     VN3                     152     //  RW  00h
//  #define     VN4                     153     //  RW  00h
//  #define     VN5                     154     //  RW  00h
//  #define     VN6                     155     //  RW  00h
//  #define     VN7                     156     //  RW  00h
//  #define     VN8                     157     //  RW  00h
//  #define     PD1                     158     //  RW  00h
//  #define     PD2                     159     //  RW  00h
//  #define     PD3                     160     //  RW  00h
//  #define     PD4                     161     //  RW  00h
//  #define     PD5                     162     //  RW  00h
//  #define     PD6                     163     //  RW  00h
//  #define     PD7                     164     //  RW  00h
//  #define     PD8                     165     //  RW  00h
//  #define     PD9                     166     //  RW  00h
//  #define     PD10                    167     //  RW  00h
//  #define     PD11                    168     //  RW  00h
//  #define     PD12                    169     //  RW  00h
//  #define     PD13                    170     //  RW  00h
//  #define     PD14                    171     //  RW  00h
//  #define     PD15                    172     //  RW  00h
//  #define     PD16                    173     //  RW  00h     Generic control packet (PB24-PB25)
//  #define     SRC_DEV_INFO            174     //  RW  00h
#define     XAF_HDCP_CTRL                   175             //  R/W                                         12h     HDCP Control Register
		#define     XAF_HDCP_CTRL_HDCP_REQ          7:7
		#define     XAF_HDCP_CTRL_BKSV_PASS         6:6
		#define     XAF_HDCP_CTRL_BKSV_FAIL         5:5
		#define     XAF_HDCP_CTRL_FRAME_ENC         4:4
		#define     XAF_HDCP_CTRL_STOP_AUTH         3:3
		#define     XAF_HDCP_CTRL_ADV_CIPHER        2:2
		#define     XAF_HDCP_CTRL_MODE         		1:1
		#define     XAF_HDCP_CTRL_MODE_DVI         	0
		#define     XAF_HDCP_CTRL_MODE_HDMI         1
		#define     XAF_HDCP_CTRL_RESET      		0:0

//        #define     HDCP_RESET                          0x01    //          R/W                                     0b  Reset HDCP
//        #define     HDMI_MODE_CTRL                      0x02    //          R/W                                     0b  HDMI/DVI mode
//        #define     ADV_CIPHER                          0x04    //          R/W                                     0b  Advanced cipher mode
//        #define     STOP_AUTH                           0x08    //          R/W                                     0b  Stop HDCP authentication
//        #define     FRAME_ENC                           0x10    //          R/W                                     0b  Frame encrypt
//        #define     BKSV_FAIL                           0x20    //          R/W                                     0b  BKSV check result (FAIL)
//        #define     BKSV_PASS                           0x40    //          R/W                                     0b  BKSV check result (PASS)
//        #define     HDCP_REQ                            0x80    //          R/W                                     0b  HDCP authentication start
//  #define     AN                      176     //  RO  00h     An register
//                                      177     //
//                                      178     //
//                                      179     //
//                                      180     //
//                                      181     //
//                                      182     //
//                                      183     //
//    #define     XB0_HDCP_STATUS                 176             //  RO                                          00h     HDCP Status Register
#define     XB2_RI_FRAME_CNT                 0XB2  
#define     XB7_DDC_CTL                	 0XB7
		#define     XB7_DDC_CTL_PRESCL            7:7
		#define     XB7_DDC_CTL_PRESCL_Y          0
		#define     XB7_DDC_CTL_PRESCL_NO         1
		#define     XB7_DDC_CTL_SDA_DRV           6:6
		#define     XB7_DDC_CTL_SDA_DRV_L         0
		#define     XB7_DDC_CTL_SDA_DRV_H         1
		#define     XB7_DDC_CTL_SCL_DRV           5:5
		#define     XB7_DDC_CTL_SCL_DRV_L         0
		#define     XB7_DDC_CTL_SCL_DRV_H         1
		#define     XB7_DDC_CTL_OVERRIDE          4:4
		#define     XB7_DDC_CTL_OVERRIDE_N        0
		#define     XB7_DDC_CTL_OVERRIDE_Y        1
		#define     XB7_DDC_CTL_SDA_ST        	  1:1
		#define     XB7_DDC_CTL_SDA_ST_L          0
		#define     XB7_DDC_CTL_SDA_ST_H          1
		#define     XB7_DDC_CTL_SCL_ST        	  0:0
		#define     XB7_DDC_CTL_SCL_ST_L          0
		#define     XB7_DDC_CTL_SCL_ST_H          1
		
#define     XB8_HDCP_STATUS                 184             //  RO                                          00h     HDCP Status Register
		#define     XB8_HDCP_STATUS_AUTH 					7:7
		#define     XB8_HDCP_STATUS_AUTH_N 					0
		#define     XB8_HDCP_STATUS_AUTH_Y 		 			1
		#define     XB8_HDCP_STATUS_ENC 					6:6
		#define     XB8_HDCP_STATUS_ENC_N 				    0
		#define     XB8_HDCP_STATUS_ENC_Y 				    1
		#define     XB8_HDCP_STATUS_MODE 			 		5:5
		#define     XB8_HDCP_STATUS_MODE_DVI 			    0
		#define     XB8_HDCP_STATUS_MODE_HDMI 		        1
		#define     XB8_HDCP_STATUS_IDLE 	 				4:4
		#define     XB8_HDCP_STATUS_IDLE_N 	 			    0
		#define     XB8_HDCP_STATUS_IDLE_Y 	 			    1
		#define 		XB8_HDCP_STATUS_ADV_CIPHER		    3:3
		#define 		XB8_HDCP_STATUS_ADV_CIPHER_N		0
		#define 		XB8_HDCP_STATUS_ADV_CIPHER_Y		1				
//        #define     ADV_CIPHERI_STATUS                  0x08    //          R                                       0b  Advanced cipher status
//        #define     HDCP_IDLE                           0x10    //          R                                       0b  HDCP state machine status
//        #define     HDMI_STATUS                         0x20    //          R                                       0b  HDMI/DVI status
//        #define     ENC                                 0x40    //          R                                       0b  Encryption status
//        #define     AUTH                                0x80    //          R                                       0b  HDCP authentication status
//  #define     SHA_DISP0               185     //  RO  00h     SHA1 value
//  #define     SHA_DISP1               186     //  RO  00h
//  #define     SHA_DISP2               187     //  RO  00h
//  #define     SHA_DISP3               188     //  RO  00h
//  #define     SHA_DISP4               189     //  RO  00h
#define     XB9_SHA0								0XB9
#define     XBA_SHA1								0XBA
#define     XBB_SHA2								0XBB
#define     XBC_SHA3								0XBC
#define     XBD_SHA4								0XBD
//		#define     XBE_BCAPS								0XBE



#define     XBE_BCAPS               190     //  RO  00h     BCAPS value
#define     XBF_KSV7_0              191     //  RO  00h     KSV[7:0] - AKSV/BKSV monitor registers
#define     XC0_KSV15_8             192     //  RO  00h     KSV[15:8] - AKSV/BKSV monitor registers
#define     XC1_KSV23_16            193     //  RO  00h     KSV[23:16] - AKSV/BKSV monitor registers
#define     XC2_KSV31_24            194     //  RO  00h     KSV[31:24] - AKSV/BKSV monitor registers
#define     XC3_KSV39_32            195     //  RO  00h     KSV[39:32] - AKSV/BKSV monitor registers
#define     XC4_SEG_PTR             196     //  RW  00h     EDID segment pointer
#define     XC5_EDID_WD_ADDR        197     //  RW  00h     EDID word address
#define     XC6_GEN_PB26            198     //  RW  00h     Generic control packet (PB26)
#define     XC7_NUM_DEV							0XC7

//  #define     NUM_DEV                 199     //  RO  00h     HDCP BKSV Size
#define     XC8_HDCP_ERR            200     //  RO  00h     HDCP error
#define     BAD_BKSV                            0x01    //          R/W                                     0b  BKSV does not contain 20 0's and 20 1's
#define     XC9_TIMER               201     //  RW  46h     Timer value for 100ms
#define     XCA_TIMER               202     //  RW  2ch     Timer value for 5sec
#define     XCB_READ_RI_CNT         203     //  RW  12h     Ri read count
#define     XCC_AN_SEED             204     //  RW  00h     An Seed
#define     XCD_MAX_REC_NUM         205     //  RW  16d     maximum number of receivers allowed
#define     XCE_HDCP_MEM_CNTL1        0XCE
#define     XCE_HDCP_MEM_CNTL1_SHORT   7:7
#define     XCE_HDCP_MEM_CNTL1_W4_DATA 6:0


//  #define     //RESERVED              206     //      RO      00h
#define     XCF_HDCP_MEM_CTRL2             207             //  RW                                          0Oh     [1:0] ID_FAX_KEY, ID_HDCP_KEY
	#define     XCF_HDCP_MEM_CTRL2_LD_HDCP   1:1
	#define     XCF_HDCP_MEM_CTRL2_LD_FAX    0:0

//        #define     LD_HDCP_KEY                         0x01    //          R/W                                     0b  Trigger for loading HDCP key from external memory
//        #define     LD_FAX_KEY                          0x02    //          R/W                                     0b  Trigger for loading fax HDCP key
#define     XD0_HDCP_CTRL2                  208             //  R/W                                         08h     HDCP Control 2
#define     XD0_HDCP_CTRL2_DIS_127_CHK      7:7
#define     XD0_HDCP_CTRL2_BKSV_CHK					6:6
#define     XD0_HDCP_CTRL2_EN_PJ_CALC				5:5
#define     XD0_HDCP_CTRL2_DIS_0LEN_HASH		4:4
#define     XD0_HDCP_CTRL2_DELAY_RI_CHK 		3:3
#define     XD0_HDCP_CTRL2_USE_PRESENT_AN		2:2
#define     XD0_HDCP_CTRL2_SEL_PRESENT_AN		1:0


//        #define     DELAY_RI_CHK                        0x08    //          R/W                                     0b  Set this bit to compare Ri at 129th frame instead of 128th frame for slower receivers.
//        #define     DIS_0LEN_HASH                       0x10    //          R/W                                     0b  Some early repeaters may not load Hash value if number of devices is 0. Set this bit to skip Hash comparison when number of devices in Bstatus equals 0.
//        #define     EN_PJ_CALC                          0x20    //          R/W                                     0b  Even though this bit is set, Pj calculation is enabled only if adv_cipher mode is selected.
//        #define     DIS_127_CHK                         0x80    //          R/W                                     0b  This bit must be set to disable 127th Ri check if Ri check frequency is altered by ri_frame_cnt (B2h).
//  #define     //RESERVED              209     //      RO      00h
#define     XD2_HDCP_KEY_CTRL               210             //  R/W                                         00h     HDCP KEY memory control
#define     XD2_HDCP_KEY_CTRL_USE_KSV1   		6:6
#define     XD2_HDCP_KEY_CTRL_USE_KSV2			5:5
#define     XD2_HDCP_KEY_CTRL_LOAD_AKSV			4:4
#define     XD2_HDCP_KEY_CTRL_KSV_SEL			3:3
#define     XD2_HDCP_KEY_CTRL_KSV_VALID			2:2
#define     XD2_HDCP_KEY_CTRL_KEY_VALID			1:1
#define     XD2_HDCP_KEY_CTRL_KEY_READY			0:0
//				#define     KEY_READY                           0x01    //          R                                       0b  This bit shows that HDCP key load has finished.
//        #define     KEY_VALID                           0x02    //          R                                       0b  This bit shows whether HDCP key loaded from HDCP memory is valid.
//        #define     KSV_VALID                           0x04    //          R                                       0b  This bit shows whether the loaded KSV is valid (has 20 1fs and 0fs).
//        #define     KSV_SEL                             0x08    //          R                                       0b  This bit shows which KSV was actually loaded into memory from HDCP memory.
//        #define     LOAD_AKSV                           0x10    //          R/W                                     0b  Select which KSV to be loaded into AKSV/BKSV register (BFh~C3h). Write 1 to load AKSV.
//        #define     USE_KSV2                            0x20    //          R/W                                     0b  If this bit is set, load the 2nd KSV written in the HDCP memory. If both usv_ksv1 and use_ksv2 are 0, hardware loads whichever that has 20 1fs and 0fs.
//        #define     USE_KSV1                            0x40    //          R/W                                     0b  If this bit is set, load the 1st KSV written in the HDCP memory.
#define     XD3_CSC_CONFIG1       			  211     //  RW  81h     CSC/Video Config.1
#define     XD3_CSC_CONFIG1_MODE					7:7
#define     XD3_CSC_CONFIG1_MODE_MANU			    0
#define     XD3_CSC_CONFIG1_MODE_AUTO			    1
#define     XD3_CSC_CONFIG1_COEF					6:3
#define     XD3_CSC_CONFIG1_COEF_SDTV_LIM		    8
#define     XD3_CSC_CONFIG1_COEF_SDTV_FUL		    4
#define     XD3_CSC_CONFIG1_COEF_HDTV				2
#define     XD3_CSC_CONFIG1_COEF_HDTV_50HZ	        1
#define     XD3_CSC_CONFIG1_CSC						2:2
#define     XD3_CSC_CONFIG1_CSC_OFF					0
#define     XD3_CSC_CONFIG1_CSC_ON					1
#define     XD3_CSC_CONFIG1_VID_ID				    1:1
#define     XD3_CSC_CONFIG1_VID_ID_5_19			    0
#define     XD3_CSC_CONFIG1_VID_ID_28_29		    1
#define     XD3_CSC_CONFIG1_SWAP_BR					0:0
#define     XD3_CSC_CONFIG1_SWAP_BR_DONE		    0
#define     XD3_CSC_CONFIG1_SWAP_BR_NOT			    1



#define     XD4_VIDEO3              212     //  RW  00h     Video setting 3
//  #define     RI                      213     //  RO  00h     Ri
//                                      214     //  RO  00h     Pj
//  #define     PJ                      215     //  Dir.    Reset       Descriptions
//  #define     SHA_RD                  216     //  RW  00h     SHA index for read

#define     XD5_RI_N7_0							0XD5
#define     XD6_RI_N15_8						0XD6
#define     XD7_PJ								0XD7
#define     XD8_SHA_RD							0XD8

#define     XD9_GEN_PB27            217     //  RW  00h     Generic InfoFrame PB27
//  #define     MPEG_B0                 218     //  RW  00h     MPEG source InfoFrame
//  #define     MPEG_B1                 219     //  RW  00h
//  #define     MPEG_B2                 220     //  RW  00h
//  #define     MPEG_B3                 221     //  RW  00h
//  #define     MPEG_FR_MF              222     //  RW  00h
#define     XDA_RI_N15_8_SAV				    0XDA
#define     XDB_PJ_SAV							0XDB
#define     XDC_NUM_DEVICE					    0XDC


#define     XDF_HPG_STATUS                  223             //  R                                           00h     Hot plug/MSENS status
		#define     XDF_HPG_STATUS_HPG_PRT					7:7
		#define     XDF_HPG_STATUS_HPG_PRT_L				 0
		#define     XDF_HPG_STATUS_HPG_PRT_H				 1
		#define     XDF_HPG_STATUS_MSENS_PRT				6:6
		#define     XDF_HPG_STATUS_MSENS_PRT_L			     0
		#define     XDF_HPG_STATUS_MSENS_PRT_H			     1
		#define     XDF_HPG_STATUS_BIST						1:0
		#define     XDF_HPG_STATUS_BIST_PASS				 2
		#define     XDF_HPG_STATUS_BIST_FAIL		         1

//        #define     BIST_FAIL                           0x01    //          R                                       0b  Dual port RAM BIST result fail
//        #define     BIST_PASS                           0x02    //          R                                       0b  Dual port RAM BIST result passed
//        #define     MSENS_PRT                           0x40    //          R                                       0b  MSENS input port status
//        #define     HPG_PRT                             0x80    //          R                                       0b  Hot plug input port status
#define     XE0_GAMUT_HB1           224     //  RW  00h     gamut metadata HB1
#define     XE1_GAMUT_HB2           225     //  RW  00h     gamut metadata HB2
//  #define     GAMUT_PB0               226     //  RW  00h     gamut metadata PB0
//  #define     GAMUT_PB1               227     //  RW  00h     gamut metadata PB1
//  #define     GAMUT_PB2               228     //  RW  00h     gamut metadata PB2
//  #define     GAMUT_PB3               229     //  RW  00h     gamut metadata PB3
//  #define     GAMUT_PB4               230     //  RW  00h     gamut metadata PB4
//  #define     GAMUT_PB5               231     //  RW  00h     gamut metadata PB5
#define     XE3_BKSV_N7_0						0XE3
#define     XE4_BKSV_N15_8					    0XE4
#define     XE5_BKSV_N23_16					    0XE5
#define     XE6_BKSV_N31_24					    0XE6
#define     XE7_BKSV_N39_32					    0XE7
//		#define     XE8_AN_N7_0							0XE8


#define     XE8_AN0                 232     //  RW  00h     An[7:0]
#define     XE9_AN1									0XE9
#define     XEA_AN2							 		0XEA
#define     XEB_AN3									0XEB
#define     XEC_AN4									0XEC 
#define     XED_AN5									0XED 
#define     XEE_AN6									0XEE 
#define     XEF_AN7									0XEF

#define     XF0_PROD_ID							    0XF0
#define     XF1_REVS_ID							    0XF1
#define     XF3_START_BLK1_LSB			            0XF3
#define     XF4_START_BLK1_MSB			            0XF4
#define     XF5_DURA_BLK1						    0XF5
#define     XF6_DURA_BLK2_LSB				        0XF6
#define     XF7_DURA_BLK2_MSB			        	0XF7
#define     XF8_DURA_BLK2						    0XF8
#define     XF9_START_BLK3_LSB			            0XF9
#define     XFA_START_BLK3_MSB			            0XFA
#define     XFB_DURA_BLK3					    	0XFB
#define     XFC_VID_INPUT					    	0XFC
	#define     XFC_VID_INPUT_HSYNC					1:1
	#define     XFC_VID_INPUT_HSYNC_ADJ_Y		    0
	#define     XFC_VID_INPUT_HSYNC_ADJ_N	    	1
	#define     XFC_VID_INPUT_VSYNC					0:0
	#define     XFC_VID_INPUT_VSYNC_ADJ_Y		    0
	#define     XFC_VID_INPUT_VSYNC_ADJ_N		    1
#define     XFD_RDAT_OUTPUT						    0XFD
#define     XFE_TEST								0XFE
#define     XFE_TEST_FBCK							7:7
#define     XFE_TEST_FBCK_DIS					    0
#define     XFE_TEST_FBCK_EN				    	1
#define     XFE_TEST_FBCK20					    	5:5
#define     XFE_TEST_FBCK20_DIS				        0
#define     XFE_TEST_FBCK20_EN				        1
#define     XFE_TEST_FBCK10						    4:4
#define     XFE_TEST_FBCK10_DIS				        0
#define     XFE_TEST_FBCK10_EN				        1
#define     XFE_TEST_BIST_FLAG				        3:3
#define     XFE_TEST_BIST_FLAG_END		            0
#define     XFE_TEST_BIST_FLAG_WORK		            1
#define     XFE_TEST_PHY_ONLY				    	2:2
#define     XFE_TEST_PHY_ONLY_DIS		        	0
#define     XFE_TEST_PHY_ONLY_EN			        1
#define     XFE_TEST_AUTO_LB				    	1:1
#define     XFE_TEST_AUTO_LB_MANUAL			        0
#define     XFE_TEST_AUTO_LB_RANDOM			        1
#define     XFE_TEST_LB 							0:0
#define     XFE_TEST_LB_DIS 						0
#define     XFE_TEST_LB_EN 							1

//  #define     GAMUT_PB7               233     //  RW  00h     gamut metadata PB7
//  #define     GAMUT_PB8               234     //  RW  00h     gamut metadata PB8
//  #define     GAMUT_PB9               235     //  RW  00h     gamut metadata PB9
//  #define     GAMUT_PB10              236     //  RW  00h     gamut metadata PB10
//  #define     GAMUT_PB11              237     //  RW  00h     gamut metadata PB11
//  #define     GAMUT_PB12              238     //  RW  00h     gamut metadata PB12
//  #define     GAMUT_PB13              239     //  RW  00h     gamut metadata PB13
//  #define     GAMUT_PB14              240     //  RW  00h     gamut metadata PB14
//  #define     GAMUT_PB15              241     //  RW  00h     gamut metadata PB15
//  #define     GAMUT_PB16              242     //  RW  00h     gamut metadata PB16
//  #define     GAMUT_PB17              243     //  RW  00h     gamut metadata PB17
//  #define     GAMUT_PB18              244     //  RW  00h     gamut metadata PB18
//  #define     GAMUT_PB19              245     //  RW  00h     gamut metadata PB19
//  #define     GAMUT_PB20              246     //  RW  00h     gamut metadata PB20
//  #define     GAMUT_PB21              247     //  RW  00h     gamut metadata PB21
//  #define     GAMUT_PB22              248     //  RW  00h     gamut metadata PB22
//  #define     GAMUT_PB23              249     //  RW  00h     gamut metadata PB23
//  #define     GAMUT_PB24              250     //  RW  00h     gamut metadata PB24
//  #define     GAMUT_PB25              251     //  RW  00h     gamut metadata PB25
//  #define     GAMUT_PB26              252     //  RW  00h     gamut metadata PB26
//  #define     GAMUT_PB27              253     //  RW  00h     gamut metadata PB27
//  #define     TEST_MODE               254     //  RW  00h     test mode register
//    #define     XFF_IP_COM_CONTROL              255             //  R/W                                         40h     I/P conversion control
//        #define     IP_CONV_PIX_REP                     0x01    //          R/W                                     0b  I/P conversion control
//        #define     IP_CONV_EN                          0x02    //          R/W                                     0b  I/P conversion mode control
//        #define     PRE_COLOR_CONV_ON                   0x10    //          R/W                                     0b  Pre-color space converter (RGB->YCbCr) control
//        #define     PRE_DOWN_CONV_ON                    0x20    //          R/W                                     0b  Pre-pixel encoding converter (down sampler) control
//        #define     STGAM_OFF                           0x40    //          R/W                                     1b  Gamma correction control
//        #define     IP_REG_OFFSET                       0x80    //          R/W                                     0b  I/P conversion control register access control


#endif  /* _HDMIREGS_H_ */
