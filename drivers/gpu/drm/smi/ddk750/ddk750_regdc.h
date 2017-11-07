/*******************************************************************
* 
*         Copyright (c) 2008 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  RegDC.h --- SMI DDK 
*  This file contains the definitions for the Display Controller registers.
* 
*******************************************************************/

/* Primary Graphics Control */

#define PRIMARY_DISPLAY_CTRL                            0x080000
#define PRIMARY_DISPLAY_CTRL_RESERVED_1_MASK            31:30
#define PRIMARY_DISPLAY_CTRL_RESERVED_1_MASK_DISABLE    0
#define PRIMARY_DISPLAY_CTRL_RESERVED_1_MASK_ENABLE     3
#define PRIMARY_DISPLAY_CTRL_SELECT                     29:28
#define PRIMARY_DISPLAY_CTRL_SELECT_PRIMARY             0
#define PRIMARY_DISPLAY_CTRL_SELECT_VGA                 1
#define PRIMARY_DISPLAY_CTRL_SELECT_SECONDARY           2
#define PRIMARY_DISPLAY_CTRL_FPEN                       27:27
#define PRIMARY_DISPLAY_CTRL_FPEN_LOW                   0
#define PRIMARY_DISPLAY_CTRL_FPEN_HIGH                  1
#define PRIMARY_DISPLAY_CTRL_VBIASEN                    26:26
#define PRIMARY_DISPLAY_CTRL_VBIASEN_LOW                0
#define PRIMARY_DISPLAY_CTRL_VBIASEN_HIGH               1
#define PRIMARY_DISPLAY_CTRL_DATA                       25:25
#define PRIMARY_DISPLAY_CTRL_DATA_DISABLE               0
#define PRIMARY_DISPLAY_CTRL_DATA_ENABLE                1
#define PRIMARY_DISPLAY_CTRL_FPVDDEN                    24:24
#define PRIMARY_DISPLAY_CTRL_FPVDDEN_LOW                0
#define PRIMARY_DISPLAY_CTRL_FPVDDEN_HIGH               1
#define PRIMARY_DISPLAY_CTRL_RESERVED_2_MASK            23:20
#define PRIMARY_DISPLAY_CTRL_RESERVED_2_MASK_DISABLE    0
#define PRIMARY_DISPLAY_CTRL_RESERVED_2_MASK_ENABLE     15
#define PRIMARY_DISPLAY_CTRL_DUAL_DISPLAY               19:19
#define PRIMARY_DISPLAY_CTRL_DUAL_DISPLAY_DISABLE       0
#define PRIMARY_DISPLAY_CTRL_DUAL_DISPLAY_ENABLE        1
#define PRIMARY_DISPLAY_CTRL_DOUBLE_PIXEL               18:18
#define PRIMARY_DISPLAY_CTRL_DOUBLE_PIXEL_DISABLE       0
#define PRIMARY_DISPLAY_CTRL_DOUBLE_PIXEL_ENABLE        1
#define PRIMARY_DISPLAY_CTRL_FIFO                       17:16
#define PRIMARY_DISPLAY_CTRL_FIFO_1                     0
#define PRIMARY_DISPLAY_CTRL_FIFO_3                     1
#define PRIMARY_DISPLAY_CTRL_FIFO_7                     2
#define PRIMARY_DISPLAY_CTRL_FIFO_11                    3
#define PRIMARY_DISPLAY_CTRL_RESERVED_3_MASK            15:15
#define PRIMARY_DISPLAY_CTRL_RESERVED_3_MASK_DISABLE    0
#define PRIMARY_DISPLAY_CTRL_RESERVED_3_MASK_ENABLE     1
#define PRIMARY_DISPLAY_CTRL_CLOCK_PHASE                14:14
#define PRIMARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH    0
#define PRIMARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_LOW     1
#define PRIMARY_DISPLAY_CTRL_VSYNC_PHASE                13:13
#define PRIMARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH    0
#define PRIMARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_LOW     1
#define PRIMARY_DISPLAY_CTRL_HSYNC_PHASE                12:12
#define PRIMARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH    0
#define PRIMARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_LOW     1
#define PRIMARY_DISPLAY_CTRL_VSYNC                      11:11
#define PRIMARY_DISPLAY_CTRL_VSYNC_ACTIVE_HIGH          0
#define PRIMARY_DISPLAY_CTRL_VSYNC_ACTIVE_LOW           1
#define PRIMARY_DISPLAY_CTRL_CAPTURE_TIMING             10:10
#define PRIMARY_DISPLAY_CTRL_CAPTURE_TIMING_DISABLE     0
#define PRIMARY_DISPLAY_CTRL_CAPTURE_TIMING_ENABLE      1
#define PRIMARY_DISPLAY_CTRL_COLOR_KEY                  9:9
#define PRIMARY_DISPLAY_CTRL_COLOR_KEY_DISABLE          0
#define PRIMARY_DISPLAY_CTRL_COLOR_KEY_ENABLE           1
#define PRIMARY_DISPLAY_CTRL_TIMING                     8:8
#define PRIMARY_DISPLAY_CTRL_TIMING_DISABLE             0
#define PRIMARY_DISPLAY_CTRL_TIMING_ENABLE              1
#define PRIMARY_DISPLAY_CTRL_VERTICAL_PAN_DIR           7:7
#define PRIMARY_DISPLAY_CTRL_VERTICAL_PAN_DIR_DOWN      0
#define PRIMARY_DISPLAY_CTRL_VERTICAL_PAN_DIR_UP        1
#define PRIMARY_DISPLAY_CTRL_VERTICAL_PAN               6:6
#define PRIMARY_DISPLAY_CTRL_VERTICAL_PAN_DISABLE       0
#define PRIMARY_DISPLAY_CTRL_VERTICAL_PAN_ENABLE        1
#define PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN_DIR         5:5
#define PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN_DIR_RIGHT   0
#define PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN_DIR_LEFT    1
#define PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN             4:4
#define PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN_DISABLE     0
#define PRIMARY_DISPLAY_CTRL_HORIZONTAL_PAN_ENABLE      1
#define PRIMARY_DISPLAY_CTRL_GAMMA                      3:3
#define PRIMARY_DISPLAY_CTRL_GAMMA_DISABLE              0
#define PRIMARY_DISPLAY_CTRL_GAMMA_ENABLE               1
#define PRIMARY_DISPLAY_CTRL_PLANE                      2:2
#define PRIMARY_DISPLAY_CTRL_PLANE_DISABLE              0
#define PRIMARY_DISPLAY_CTRL_PLANE_ENABLE               1
#define PRIMARY_DISPLAY_CTRL_FORMAT                     1:0
#define PRIMARY_DISPLAY_CTRL_FORMAT_8                   0
#define PRIMARY_DISPLAY_CTRL_FORMAT_16                  1
#define PRIMARY_DISPLAY_CTRL_FORMAT_32                  2

#define PRIMARY_PAN_CTRL                                0x080004
#define PRIMARY_PAN_CTRL_VERTICAL_PAN                   31:24
#define PRIMARY_PAN_CTRL_VERTICAL_VSYNC                 21:16
#define PRIMARY_PAN_CTRL_HORIZONTAL_PAN                 15:8
#define PRIMARY_PAN_CTRL_HORIZONTAL_VSYNC               5:0

#define PRIMARY_COLOR_KEY                               0x080008
#define PRIMARY_COLOR_KEY_MASK                          31:16
#define PRIMARY_COLOR_KEY_VALUE                         15:0

#define PRIMARY_FB_ADDRESS                              0x08000C
#define PRIMARY_FB_ADDRESS_STATUS                       31:31
#define PRIMARY_FB_ADDRESS_STATUS_CURRENT               0
#define PRIMARY_FB_ADDRESS_STATUS_PENDING               1
#define PRIMARY_FB_ADDRESS_EXT                          27:27
#define PRIMARY_FB_ADDRESS_EXT_LOCAL                    0
#define PRIMARY_FB_ADDRESS_EXT_EXTERNAL                 1
#define PRIMARY_FB_ADDRESS_ADDRESS                      25:0

#define PRIMARY_FB_WIDTH                                0x080010
#define PRIMARY_FB_WIDTH_WIDTH                          29:16
#define PRIMARY_FB_WIDTH_OFFSET                         13:0

#define PRIMARY_WINDOW_WIDTH                            0x080014
#define PRIMARY_WINDOW_WIDTH_WIDTH                      27:16
#define PRIMARY_WINDOW_WIDTH_X                          11:0

#define PRIMARY_WINDOW_HEIGHT                           0x080018
#define PRIMARY_WINDOW_HEIGHT_HEIGHT                    27:16
#define PRIMARY_WINDOW_HEIGHT_Y                         11:0

#define PRIMARY_PLANE_TL                                0x08001C
#define PRIMARY_PLANE_TL_TOP                            26:16
#define PRIMARY_PLANE_TL_LEFT                           10:0

#define PRIMARY_PLANE_BR                                0x080020
#define PRIMARY_PLANE_BR_BOTTOM                         26:16
#define PRIMARY_PLANE_BR_RIGHT                          10:0

#define PRIMARY_HORIZONTAL_TOTAL                        0x080024
#define PRIMARY_HORIZONTAL_TOTAL_TOTAL                  27:16
#define PRIMARY_HORIZONTAL_TOTAL_DISPLAY_END            11:0

#define PRIMARY_HORIZONTAL_SYNC                         0x080028
#define PRIMARY_HORIZONTAL_SYNC_WIDTH                   23:16
#define PRIMARY_HORIZONTAL_SYNC_START                   11:0

#define PRIMARY_VERTICAL_TOTAL                          0x08002C
#define PRIMARY_VERTICAL_TOTAL_TOTAL                    26:16
#define PRIMARY_VERTICAL_TOTAL_DISPLAY_END              10:0

#define PRIMARY_VERTICAL_SYNC                           0x080030
#define PRIMARY_VERTICAL_SYNC_HEIGHT                    21:16
#define PRIMARY_VERTICAL_SYNC_START                     10:0

#define PRIMARY_CURRENT_LINE                            0x080034
#define PRIMARY_CURRENT_LINE_LINE                       10:0

/* Video Control */

#define VIDEO_DISPLAY_CTRL                              0x080040
#define VIDEO_DISPLAY_CTRL_LINE_BUFFER                  18:18
#define VIDEO_DISPLAY_CTRL_LINE_BUFFER_DISABLE          0
#define VIDEO_DISPLAY_CTRL_LINE_BUFFER_ENABLE           1
#define VIDEO_DISPLAY_CTRL_FIFO                         17:16
#define VIDEO_DISPLAY_CTRL_FIFO_1                       0
#define VIDEO_DISPLAY_CTRL_FIFO_3                       1
#define VIDEO_DISPLAY_CTRL_FIFO_7                       2
#define VIDEO_DISPLAY_CTRL_FIFO_11                      3
#define VIDEO_DISPLAY_CTRL_BUFFER                       15:15
#define VIDEO_DISPLAY_CTRL_BUFFER_0                     0
#define VIDEO_DISPLAY_CTRL_BUFFER_1                     1
#define VIDEO_DISPLAY_CTRL_CAPTURE                      14:14
#define VIDEO_DISPLAY_CTRL_CAPTURE_DISABLE              0
#define VIDEO_DISPLAY_CTRL_CAPTURE_ENABLE               1
#define VIDEO_DISPLAY_CTRL_DOUBLE_BUFFER                13:13
#define VIDEO_DISPLAY_CTRL_DOUBLE_BUFFER_DISABLE        0
#define VIDEO_DISPLAY_CTRL_DOUBLE_BUFFER_ENABLE         1
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP                    12:12
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP_DISABLE            0
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP_ENABLE             1
#define VIDEO_DISPLAY_CTRL_VERTICAL_SCALE               11:11
#define VIDEO_DISPLAY_CTRL_VERTICAL_SCALE_NORMAL        0
#define VIDEO_DISPLAY_CTRL_VERTICAL_SCALE_HALF          1
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_SCALE             10:10
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_SCALE_NORMAL      0
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_SCALE_HALF        1
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
#define VIDEO_DISPLAY_CTRL_FORMAT_8                     0
#define VIDEO_DISPLAY_CTRL_FORMAT_16                    1
#define VIDEO_DISPLAY_CTRL_FORMAT_32                    2
#define VIDEO_DISPLAY_CTRL_FORMAT_YUV                   3

#define VIDEO_FB_0_ADDRESS                              0x080044
#define VIDEO_FB_0_ADDRESS_STATUS                       31:31
#define VIDEO_FB_0_ADDRESS_STATUS_CURRENT               0
#define VIDEO_FB_0_ADDRESS_STATUS_PENDING               1
#define VIDEO_FB_0_ADDRESS_EXT                          27:27
#define VIDEO_FB_0_ADDRESS_EXT_LOCAL                    0
#define VIDEO_FB_0_ADDRESS_EXT_EXTERNAL                 1
#define VIDEO_FB_0_ADDRESS_ADDRESS                      25:0

#define VIDEO_FB_WIDTH                                  0x080048
#define VIDEO_FB_WIDTH_WIDTH                            29:16
#define VIDEO_FB_WIDTH_OFFSET                           13:0

#define VIDEO_FB_0_LAST_ADDRESS                         0x08004C
#define VIDEO_FB_0_LAST_ADDRESS_EXT                     27:27
#define VIDEO_FB_0_LAST_ADDRESS_EXT_LOCAL               0
#define VIDEO_FB_0_LAST_ADDRESS_EXT_EXTERNAL            1
#define VIDEO_FB_0_LAST_ADDRESS_ADDRESS                 25:0

#define VIDEO_PLANE_TL                                  0x080050
#define VIDEO_PLANE_TL_TOP                              26:16
#define VIDEO_PLANE_TL_LEFT                             10:0

#define VIDEO_PLANE_BR                                  0x080054
#define VIDEO_PLANE_BR_BOTTOM                           26:16
#define VIDEO_PLANE_BR_RIGHT                            10:0

#define VIDEO_SCALE                                     0x080058
#define VIDEO_SCALE_VERTICAL_MODE                       31:31
#define VIDEO_SCALE_VERTICAL_MODE_EXPAND                0
#define VIDEO_SCALE_VERTICAL_MODE_SHRINK                1
#define VIDEO_SCALE_VERTICAL_SCALE                      27:16
#define VIDEO_SCALE_HORIZONTAL_MODE                     15:15
#define VIDEO_SCALE_HORIZONTAL_MODE_EXPAND              0
#define VIDEO_SCALE_HORIZONTAL_MODE_SHRINK              1
#define VIDEO_SCALE_HORIZONTAL_SCALE                    11:0

#define VIDEO_INITIAL_SCALE                             0x08005C
#define VIDEO_INITIAL_SCALE_FB_1                        27:16
#define VIDEO_INITIAL_SCALE_FB_0                        11:0

#define VIDEO_YUV_CONSTANTS                             0x080060
#define VIDEO_YUV_CONSTANTS_Y                           31:24
#define VIDEO_YUV_CONSTANTS_R                           23:16
#define VIDEO_YUV_CONSTANTS_G                           15:8
#define VIDEO_YUV_CONSTANTS_B                           7:0

#define VIDEO_FB_1_ADDRESS                              0x080064
#define VIDEO_FB_1_ADDRESS_STATUS                       31:31
#define VIDEO_FB_1_ADDRESS_STATUS_CURRENT               0
#define VIDEO_FB_1_ADDRESS_STATUS_PENDING               1
#define VIDEO_FB_1_ADDRESS_EXT                          27:27
#define VIDEO_FB_1_ADDRESS_EXT_LOCAL                    0
#define VIDEO_FB_1_ADDRESS_EXT_EXTERNAL                 1
#define VIDEO_FB_1_ADDRESS_ADDRESS                      25:0

#define VIDEO_FB_1_LAST_ADDRESS                         0x080068
#define VIDEO_FB_1_LAST_ADDRESS_EXT                     27:27
#define VIDEO_FB_1_LAST_ADDRESS_EXT_LOCAL               0
#define VIDEO_FB_1_LAST_ADDRESS_EXT_EXTERNAL            1
#define VIDEO_FB_1_LAST_ADDRESS_ADDRESS                 25:0

#define VIDEO_EDGE_DETECTION                            0x080074
#define VIDEO_EDGE_DETECTION_DETECT                     24:24
#define VIDEO_EDGE_DETECTION_DETECT_DISABLE             0
#define VIDEO_EDGE_DETECTION_DETECT_ENABLE              1
#define VIDEO_EDGE_DETECTION_VALUE                      9:0

/* Video Alpha Control */

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
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_SCALE             11:11
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_SCALE_NORMAL      0
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_SCALE_HALF        1
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_SCALE             10:10
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_SCALE_NORMAL      0
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_SCALE_HALF        1
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_MODE              9:9
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_MODE_REPLICATE    0
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_MODE_INTERPOLATE  1
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_MODE              8:8
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_MODE_REPLICATE    0
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_MODE_INTERPOLATE  1
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
#define VIDEO_ALPHA_FB_ADDRESS_EXT                      27:27
#define VIDEO_ALPHA_FB_ADDRESS_EXT_LOCAL                0
#define VIDEO_ALPHA_FB_ADDRESS_EXT_EXTERNAL             1
#define VIDEO_ALPHA_FB_ADDRESS_ADDRESS                  25:0

#define VIDEO_ALPHA_FB_WIDTH                            0x080088
#define VIDEO_ALPHA_FB_WIDTH_WIDTH                      29:16
#define VIDEO_ALPHA_FB_WIDTH_OFFSET                     13:0

#define VIDEO_ALPHA_FB_LAST_ADDRESS                     0x08008C
#define VIDEO_ALPHA_FB_LAST_ADDRESS_EXT                 27:27
#define VIDEO_ALPHA_FB_LAST_ADDRESS_EXT_LOCAL           0
#define VIDEO_ALPHA_FB_LAST_ADDRESS_EXT_EXTERNAL        1
#define VIDEO_ALPHA_FB_LAST_ADDRESS_ADDRESS             25:0

#define VIDEO_ALPHA_PLANE_TL                            0x080090
#define VIDEO_ALPHA_PLANE_TL_TOP                        26:16
#define VIDEO_ALPHA_PLANE_TL_LEFT                       10:0

#define VIDEO_ALPHA_PLANE_BR                            0x080094
#define VIDEO_ALPHA_PLANE_BR_BOTTOM                     26:16
#define VIDEO_ALPHA_PLANE_BR_RIGHT                      10:0

#define VIDEO_ALPHA_SCALE                               0x080098
#define VIDEO_ALPHA_SCALE_VERTICAL_MODE                 31:31
#define VIDEO_ALPHA_SCALE_VERTICAL_MODE_EXPAND          0
#define VIDEO_ALPHA_SCALE_VERTICAL_MODE_SHRINK          1
#define VIDEO_ALPHA_SCALE_VERTICAL_SCALE                27:16
#define VIDEO_ALPHA_SCALE_HORIZONTAL_MODE               15:15
#define VIDEO_ALPHA_SCALE_HORIZONTAL_MODE_EXPAND        0
#define VIDEO_ALPHA_SCALE_HORIZONTAL_MODE_SHRINK        1
#define VIDEO_ALPHA_SCALE_HORIZONTAL_SCALE              11:0

#define VIDEO_ALPHA_INITIAL_SCALE                       0x08009C
#define VIDEO_ALPHA_INITIAL_SCALE_VERTICAL              27:16
#define VIDEO_ALPHA_INITIAL_SCALE_HORIZONTAL            11:0

#define VIDEO_ALPHA_CHROMA_KEY                          0x0800A0
#define VIDEO_ALPHA_CHROMA_KEY_MASK                     31:16
#define VIDEO_ALPHA_CHROMA_KEY_VALUE                    15:0

#define VIDEO_ALPHA_COLOR_LOOKUP_01                     0x0800A4
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_23                     0x0800A8
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_45                     0x0800AC
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_67                     0x0800B0
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_89                     0x0800B4
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_AB                     0x0800B8
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_CD                     0x0800BC
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_EF                     0x0800C0
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_BLUE              4:0

/* Primary Cursor Control */

#define PRIMARY_HWC_ADDRESS                             0x0800F0
#define PRIMARY_HWC_ADDRESS_ENABLE                      31:31
#define PRIMARY_HWC_ADDRESS_ENABLE_DISABLE              0
#define PRIMARY_HWC_ADDRESS_ENABLE_ENABLE               1
#define PRIMARY_HWC_ADDRESS_MODE                        30:30
#define PRIMARY_HWC_ADDRESS_MODE_MONO                   0
#define PRIMARY_HWC_ADDRESS_MODE_COLOR                  1
#define PRIMARY_HWC_ADDRESS_EXT                         27:27
#define PRIMARY_HWC_ADDRESS_EXT_LOCAL                   0
#define PRIMARY_HWC_ADDRESS_EXT_EXTERNAL                1
#define PRIMARY_HWC_ADDRESS_ADDRESS                     25:0

#define PRIMARY_HWC_LOCATION                            0x0800F4
#define PRIMARY_HWC_LOCATION_TOP                        27:27
#define PRIMARY_HWC_LOCATION_TOP_INSIDE                 0
#define PRIMARY_HWC_LOCATION_TOP_OUTSIDE                1
#define PRIMARY_HWC_LOCATION_Y                          26:16
#define PRIMARY_HWC_LOCATION_LEFT                       11:11
#define PRIMARY_HWC_LOCATION_LEFT_INSIDE                0
#define PRIMARY_HWC_LOCATION_LEFT_OUTSIDE               1
#define PRIMARY_HWC_LOCATION_X                          10:0

#define PRIMARY_HWC_COLOR_12                            0x0800F8
#define PRIMARY_HWC_COLOR_12_2_RGB565                   31:16
#define PRIMARY_HWC_COLOR_12_1_RGB565                   15:0

#define PRIMARY_HWC_COLOR_3                             0x0800FC
#define PRIMARY_HWC_COLOR_3_RGB565                      15:0

/* Alpha Control */

#define ALPHA_DISPLAY_CTRL                              0x080100
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

#define ALPHA_FB_ADDRESS                                0x080104
#define ALPHA_FB_ADDRESS_STATUS                         31:31
#define ALPHA_FB_ADDRESS_STATUS_CURRENT                 0
#define ALPHA_FB_ADDRESS_STATUS_PENDING                 1
#define ALPHA_FB_ADDRESS_EXT                            27:27
#define ALPHA_FB_ADDRESS_EXT_LOCAL                      0
#define ALPHA_FB_ADDRESS_EXT_EXTERNAL                   1
#define ALPHA_FB_ADDRESS_ADDRESS                        25:0

#define ALPHA_FB_WIDTH                                  0x080108
#define ALPHA_FB_WIDTH_WIDTH                            29:16
#define ALPHA_FB_WIDTH_OFFSET                           13:0

#define ALPHA_PLANE_TL                                  0x08010C
#define ALPHA_PLANE_TL_TOP                              26:16
#define ALPHA_PLANE_TL_LEFT                             10:0

#define ALPHA_PLANE_BR                                  0x080110
#define ALPHA_PLANE_BR_BOTTOM                           26:16
#define ALPHA_PLANE_BR_RIGHT                            10:0

#define ALPHA_CHROMA_KEY                                0x080114
#define ALPHA_CHROMA_KEY_MASK                           31:16
#define ALPHA_CHROMA_KEY_VALUE                          15:0

#define ALPHA_COLOR_LOOKUP_01                           0x080118
#define ALPHA_COLOR_LOOKUP_01_1                         31:16
#define ALPHA_COLOR_LOOKUP_01_1_RED                     31:27
#define ALPHA_COLOR_LOOKUP_01_1_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_01_1_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_01_0                         15:0
#define ALPHA_COLOR_LOOKUP_01_0_RED                     15:11
#define ALPHA_COLOR_LOOKUP_01_0_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_01_0_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_23                           0x08011C
#define ALPHA_COLOR_LOOKUP_23_3                         31:16
#define ALPHA_COLOR_LOOKUP_23_3_RED                     31:27
#define ALPHA_COLOR_LOOKUP_23_3_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_23_3_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_23_2                         15:0
#define ALPHA_COLOR_LOOKUP_23_2_RED                     15:11
#define ALPHA_COLOR_LOOKUP_23_2_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_23_2_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_45                           0x080120
#define ALPHA_COLOR_LOOKUP_45_5                         31:16
#define ALPHA_COLOR_LOOKUP_45_5_RED                     31:27
#define ALPHA_COLOR_LOOKUP_45_5_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_45_5_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_45_4                         15:0
#define ALPHA_COLOR_LOOKUP_45_4_RED                     15:11
#define ALPHA_COLOR_LOOKUP_45_4_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_45_4_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_67                           0x080124
#define ALPHA_COLOR_LOOKUP_67_7                         31:16
#define ALPHA_COLOR_LOOKUP_67_7_RED                     31:27
#define ALPHA_COLOR_LOOKUP_67_7_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_67_7_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_67_6                         15:0
#define ALPHA_COLOR_LOOKUP_67_6_RED                     15:11
#define ALPHA_COLOR_LOOKUP_67_6_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_67_6_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_89                           0x080128
#define ALPHA_COLOR_LOOKUP_89_9                         31:16
#define ALPHA_COLOR_LOOKUP_89_9_RED                     31:27
#define ALPHA_COLOR_LOOKUP_89_9_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_89_9_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_89_8                         15:0
#define ALPHA_COLOR_LOOKUP_89_8_RED                     15:11
#define ALPHA_COLOR_LOOKUP_89_8_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_89_8_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_AB                           0x08012C
#define ALPHA_COLOR_LOOKUP_AB_B                         31:16
#define ALPHA_COLOR_LOOKUP_AB_B_RED                     31:27
#define ALPHA_COLOR_LOOKUP_AB_B_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_AB_B_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_AB_A                         15:0
#define ALPHA_COLOR_LOOKUP_AB_A_RED                     15:11
#define ALPHA_COLOR_LOOKUP_AB_A_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_AB_A_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_CD                           0x080130
#define ALPHA_COLOR_LOOKUP_CD_D                         31:16
#define ALPHA_COLOR_LOOKUP_CD_D_RED                     31:27
#define ALPHA_COLOR_LOOKUP_CD_D_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_CD_D_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_CD_C                         15:0
#define ALPHA_COLOR_LOOKUP_CD_C_RED                     15:11
#define ALPHA_COLOR_LOOKUP_CD_C_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_CD_C_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_EF                           0x080134
#define ALPHA_COLOR_LOOKUP_EF_F                         31:16
#define ALPHA_COLOR_LOOKUP_EF_F_RED                     31:27
#define ALPHA_COLOR_LOOKUP_EF_F_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_EF_F_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_EF_E                         15:0
#define ALPHA_COLOR_LOOKUP_EF_E_RED                     15:11
#define ALPHA_COLOR_LOOKUP_EF_E_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_EF_E_BLUE                    4:0

/* Secondary Graphics Control */

#define SECONDARY_DISPLAY_CTRL                              0x080200

/* SM750 and SM718 definition */
#define SECONDARY_DISPLAY_CTRL_RESERVED_1_MASK              31:27
#define SECONDARY_DISPLAY_CTRL_RESERVED_1_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_1_MASK_ENABLE       0x1F
/* SM750LE definition */
#define SECONDARY_DISPLAY_CTRL_DPMS                         31:30
#define SECONDARY_DISPLAY_CTRL_DPMS_0                       0
#define SECONDARY_DISPLAY_CTRL_DPMS_1                       1
#define SECONDARY_DISPLAY_CTRL_DPMS_2                       2
#define SECONDARY_DISPLAY_CTRL_DPMS_3                       3
#define SECONDARY_DISPLAY_CTRL_CLK                          29:27
#define SECONDARY_DISPLAY_CTRL_CLK_PLL25                    0
#define SECONDARY_DISPLAY_CTRL_CLK_PLL41                    1
#define SECONDARY_DISPLAY_CTRL_CLK_PLL62                    2
#define SECONDARY_DISPLAY_CTRL_CLK_PLL65                    3
#define SECONDARY_DISPLAY_CTRL_CLK_PLL74                    4
#define SECONDARY_DISPLAY_CTRL_CLK_PLL80                    5
#define SECONDARY_DISPLAY_CTRL_CLK_PLL108                   6
#define SECONDARY_DISPLAY_CTRL_CLK_RESERVED                 7
#define SECONDARY_DISPLAY_CTRL_SHIFT_VGA_DAC                26:26
#define SECONDARY_DISPLAY_CTRL_SHIFT_VGA_DAC_DISABLE        1
#define SECONDARY_DISPLAY_CTRL_SHIFT_VGA_DAC_ENABLE         0

/* SM750 and SM718 definition */
#define SECONDARY_DISPLAY_CTRL_RESERVED_2_MASK              25:24
#define SECONDARY_DISPLAY_CTRL_RESERVED_2_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_2_MASK_ENABLE       3

/* SM750LE definition */
#define SECONDARY_DISPLAY_CTRL_CRTSELECT                    25:25
#define SECONDARY_DISPLAY_CTRL_CRTSELECT_VGA                0
#define SECONDARY_DISPLAY_CTRL_CRTSELECT_CRT                1
#define SECONDARY_DISPLAY_CTRL_RGBBIT                       24:24
#define SECONDARY_DISPLAY_CTRL_RGBBIT_24BIT                 0
#define SECONDARY_DISPLAY_CTRL_RGBBIT_12BIT                 1

#define SECONDARY_DISPLAY_CTRL_LOCK_TIMING                  23:23
#define SECONDARY_DISPLAY_CTRL_LOCK_TIMING_DISABLE          0
#define SECONDARY_DISPLAY_CTRL_LOCK_TIMING_ENABLE           1
#define SECONDARY_DISPLAY_CTRL_EXPANSION                    22:22
#define SECONDARY_DISPLAY_CTRL_EXPANSION_DISABLE            0
#define SECONDARY_DISPLAY_CTRL_EXPANSION_ENABLE             1
#define SECONDARY_DISPLAY_CTRL_VERTICAL_MODE                21:21
#define SECONDARY_DISPLAY_CTRL_VERTICAL_MODE_REPLICATE      0
#define SECONDARY_DISPLAY_CTRL_VERTICAL_MODE_INTERPOLATE    1
#define SECONDARY_DISPLAY_CTRL_HORIZONTAL_MODE              20:20
#define SECONDARY_DISPLAY_CTRL_HORIZONTAL_MODE_REPLICATE    0
#define SECONDARY_DISPLAY_CTRL_HORIZONTAL_MODE_INTERPOLATE  1
#define SECONDARY_DISPLAY_CTRL_SELECT                       19:18
#define SECONDARY_DISPLAY_CTRL_SELECT_PRIMARY               0
#define SECONDARY_DISPLAY_CTRL_SELECT_VGA                   1
#define SECONDARY_DISPLAY_CTRL_SELECT_SECONDARY             2
#define SECONDARY_DISPLAY_CTRL_FIFO                         17:16
#define SECONDARY_DISPLAY_CTRL_FIFO_1                       0
#define SECONDARY_DISPLAY_CTRL_FIFO_3                       1
#define SECONDARY_DISPLAY_CTRL_FIFO_7                       2
#define SECONDARY_DISPLAY_CTRL_FIFO_11                      3
#define SECONDARY_DISPLAY_CTRL_RESERVED_3_MASK              15:15
#define SECONDARY_DISPLAY_CTRL_RESERVED_3_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_3_MASK_ENABLE       1
#define SECONDARY_DISPLAY_CTRL_CLOCK_PHASE                  14:14
#define SECONDARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH      0
#define SECONDARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_LOW       1
#define SECONDARY_DISPLAY_CTRL_VSYNC_PHASE                  13:13
#define SECONDARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH      0
#define SECONDARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_LOW       1
#define SECONDARY_DISPLAY_CTRL_HSYNC_PHASE                  12:12
#define SECONDARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH      0
#define SECONDARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_LOW       1
#define SECONDARY_DISPLAY_CTRL_BLANK                        10:10
#define SECONDARY_DISPLAY_CTRL_BLANK_OFF                    0
#define SECONDARY_DISPLAY_CTRL_BLANK_ON                     1
#define SECONDARY_DISPLAY_CTRL_RESERVED_4_MASK              9:9
#define SECONDARY_DISPLAY_CTRL_RESERVED_4_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_4_MASK_ENABLE       1
#define SECONDARY_DISPLAY_CTRL_TIMING                       8:8
#define SECONDARY_DISPLAY_CTRL_TIMING_DISABLE               0
#define SECONDARY_DISPLAY_CTRL_TIMING_ENABLE                1
#define SECONDARY_DISPLAY_CTRL_PIXEL                        7:4
#define SECONDARY_DISPLAY_CTRL_GAMMA                        3:3
#define SECONDARY_DISPLAY_CTRL_GAMMA_DISABLE                0
#define SECONDARY_DISPLAY_CTRL_GAMMA_ENABLE                 1
#define SECONDARY_DISPLAY_CTRL_PLANE                        2:2
#define SECONDARY_DISPLAY_CTRL_PLANE_DISABLE                0
#define SECONDARY_DISPLAY_CTRL_PLANE_ENABLE                 1
#define SECONDARY_DISPLAY_CTRL_FORMAT                       1:0
#define SECONDARY_DISPLAY_CTRL_FORMAT_8                     0
#define SECONDARY_DISPLAY_CTRL_FORMAT_16                    1
#define SECONDARY_DISPLAY_CTRL_FORMAT_32                    2
#define SECONDARY_DISPLAY_CTRL_RESERVED_BITS_MASK           0xFF000200

#define SECONDARY_FB_ADDRESS                            0x080204
#define SECONDARY_FB_ADDRESS_STATUS                     31:31
#define SECONDARY_FB_ADDRESS_STATUS_CURRENT             0
#define SECONDARY_FB_ADDRESS_STATUS_PENDING             1
#define SECONDARY_FB_ADDRESS_EXT                        27:27
#define SECONDARY_FB_ADDRESS_EXT_LOCAL                  0
#define SECONDARY_FB_ADDRESS_EXT_EXTERNAL               1
#define SECONDARY_FB_ADDRESS_ADDRESS                    25:0

#define SECONDARY_FB_WIDTH                              0x080208
#define SECONDARY_FB_WIDTH_WIDTH                        29:16
#define SECONDARY_FB_WIDTH_OFFSET                       13:0

#define SECONDARY_HORIZONTAL_TOTAL                      0x08020C
#define SECONDARY_HORIZONTAL_TOTAL_TOTAL                27:16
#define SECONDARY_HORIZONTAL_TOTAL_DISPLAY_END          11:0

#define SECONDARY_HORIZONTAL_SYNC                       0x080210
#define SECONDARY_HORIZONTAL_SYNC_WIDTH                 23:16
#define SECONDARY_HORIZONTAL_SYNC_START                 11:0

#define SECONDARY_VERTICAL_TOTAL                        0x080214
#define SECONDARY_VERTICAL_TOTAL_TOTAL                  26:16
#define SECONDARY_VERTICAL_TOTAL_DISPLAY_END            10:0

#define SECONDARY_VERTICAL_SYNC                         0x080218
#define SECONDARY_VERTICAL_SYNC_HEIGHT                  21:16
#define SECONDARY_VERTICAL_SYNC_START                   10:0

#define SECONDARY_SIGNATURE_ANALYZER                    0x08021C
#define SECONDARY_SIGNATURE_ANALYZER_STATUS             31:16
#define SECONDARY_SIGNATURE_ANALYZER_ENABLE             3:3
#define SECONDARY_SIGNATURE_ANALYZER_ENABLE_DISABLE     0
#define SECONDARY_SIGNATURE_ANALYZER_ENABLE_ENABLE      1
#define SECONDARY_SIGNATURE_ANALYZER_RESET              2:2
#define SECONDARY_SIGNATURE_ANALYZER_RESET_NORMAL       0
#define SECONDARY_SIGNATURE_ANALYZER_RESET_RESET        1
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE             1:0
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE_RED         0
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE_GREEN       1
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE_BLUE        2

#define SECONDARY_CURRENT_LINE                          0x080220
#define SECONDARY_CURRENT_LINE_LINE                     10:0

#define SECONDARY_MONITOR_DETECT                        0x080224
#define SECONDARY_MONITOR_DETECT_VALUE                  25:25
#define SECONDARY_MONITOR_DETECT_VALUE_DISABLE          0
#define SECONDARY_MONITOR_DETECT_VALUE_ENABLE           1
#define SECONDARY_MONITOR_DETECT_ENABLE                 24:24
#define SECONDARY_MONITOR_DETECT_ENABLE_DISABLE         0
#define SECONDARY_MONITOR_DETECT_ENABLE_ENABLE          1
#define SECONDARY_MONITOR_DETECT_RED                    23:16
#define SECONDARY_MONITOR_DETECT_GREEN                  15:8
#define SECONDARY_MONITOR_DETECT_BLUE                   7:0

#define SECONDARY_SCALE                                 0x080228
#define SECONDARY_SCALE_VERTICAL_MODE                   31:31
#define SECONDARY_SCALE_VERTICAL_MODE_EXPAND            0
#define SECONDARY_SCALE_VERTICAL_MODE_SHRINK            1
#define SECONDARY_SCALE_VERTICAL_SCALE                  27:16
#define SECONDARY_SCALE_HORIZONTAL_MODE                 15:15
#define SECONDARY_SCALE_HORIZONTAL_MODE_EXPAND          0
#define SECONDARY_SCALE_HORIZONTAL_MODE_SHRINK          1
#define SECONDARY_SCALE_HORIZONTAL_SCALE                11:0

/* Secondary Cursor Control */

#define SECONDARY_HWC_ADDRESS                           0x080230
#define SECONDARY_HWC_ADDRESS_ENABLE                    31:31
#define SECONDARY_HWC_ADDRESS_ENABLE_DISABLE            0
#define SECONDARY_HWC_ADDRESS_ENABLE_ENABLE             1
#define SECONDARY_HWC_ADDRESS_MODE                      30:30
#define SECONDARY_HWC_ADDRESS_MODE_MONO                 0
#define SECONDARY_HWC_ADDRESS_MODE_COLOR                1
#define SECONDARY_HWC_ADDRESS_EXT                       27:27
#define SECONDARY_HWC_ADDRESS_EXT_LOCAL                 0
#define SECONDARY_HWC_ADDRESS_EXT_EXTERNAL              1
#define SECONDARY_HWC_ADDRESS_ADDRESS                   25:0

#define SECONDARY_HWC_LOCATION                          0x080234
#define SECONDARY_HWC_LOCATION_TOP                      27:27
#define SECONDARY_HWC_LOCATION_TOP_INSIDE               0
#define SECONDARY_HWC_LOCATION_TOP_OUTSIDE              1
#define SECONDARY_HWC_LOCATION_Y                        26:16
#define SECONDARY_HWC_LOCATION_LEFT                     11:11
#define SECONDARY_HWC_LOCATION_LEFT_INSIDE              0
#define SECONDARY_HWC_LOCATION_LEFT_OUTSIDE             1
#define SECONDARY_HWC_LOCATION_X                        10:0

#define SECONDARY_HWC_COLOR_12                          0x080238
#define SECONDARY_HWC_COLOR_12_2_RGB565                 31:16
#define SECONDARY_HWC_COLOR_12_1_RGB565                 15:0

#define SECONDARY_HWC_COLOR_3                           0x08023C
#define SECONDARY_HWC_COLOR_3_RGB565                    15:0

/* This vertical expansion below start at 0x080240 ~ 0x080264 */
#define SECONDARY_VERTICAL_EXPANSION                    0x080240
#define SECONDARY_VERTICAL_EXPANSION_CENTERING_VALUE    31:24 
#define SECONDARY_VERTICAL_EXPANSION_COMPARE_VALUE      23:16
#define SECONDARY_VERTICAL_EXPANSION_LINE_BUFFER        15:12
#define SECONDARY_VERTICAL_EXPANSION_SCALE_FACTOR       11:0

/* This horizontal expansion below start at 0x080268 ~ 0x08027C */
#define SECONDARY_HORIZONTAL_EXPANSION                  0x080268
#define SECONDARY_HORIZONTAL_EXPANSION_CENTERING_VALUE  31:24 
#define SECONDARY_HORIZONTAL_EXPANSION_COMPARE_VALUE    23:16
#define SECONDARY_HORIZONTAL_EXPANSION_SCALE_FACTOR     11:0

/* Auto Centering */
#define SECONDARY_AUTO_CENTERING_TL                     0x080280
#define SECONDARY_AUTO_CENTERING_TL_TOP                 26:16
#define SECONDARY_AUTO_CENTERING_TL_LEFT                10:0

#define SECONDARY_AUTO_CENTERING_BR                     0x080284
#define SECONDARY_AUTO_CENTERING_BR_BOTTOM              26:16
#define SECONDARY_AUTO_CENTERING_BR_RIGHT               10:0

/* SM750LE new register to control panel output */
#define DISPLAY_CONTROL_750LE                           0x80288
#define DISPLAY_CONTROL_750LE_RESERVED                  31:5
#define DISPLAY_CONTROL_750LE_PANEL                     4:4
#define DISPLAY_CONTROL_750LE_PANEL_NORMAL              0
#define DISPLAY_CONTROL_750LE_PANEL_TRISTATE            1
#define DISPLAY_CONTROL_750LE_EN                        3:3
#define DISPLAY_CONTROL_750LE_EN_LOW                    0
#define DISPLAY_CONTROL_750LE_EN_HIGH                   1
#define DISPLAY_CONTROL_750LE_BIAS                      2:2
#define DISPLAY_CONTROL_750LE_BIAS_LOW                  0
#define DISPLAY_CONTROL_750LE_BIAS_HIGH                 1
#define DISPLAY_CONTROL_750LE_DATA                      1:1
#define DISPLAY_CONTROL_750LE_DATA_DISABLE              0
#define DISPLAY_CONTROL_750LE_DATA_ENABLE               1
#define DISPLAY_CONTROL_750LE_VDD                       0:0 
#define DISPLAY_CONTROL_750LE_VDD_LOW                   0
#define DISPLAY_CONTROL_750LE_VDD_HIGH                  1

/* SM750LE new register for display interrtup control */
#define RAW_INT_750LE                                    0x080290
#define RAW_INT_750LE_RESERVED1                          31:3
#define RAW_INT_750LE_SECONDARY_VSYNC                    2:2
#define RAW_INT_750LE_SECONDARY_VSYNC_INACTIVE           0
#define RAW_INT_750LE_SECONDARY_VSYNC_ACTIVE             1
#define RAW_INT_750LE_SECONDARY_VSYNC_CLEAR              1
#define RAW_INT_750LE_PRIMARY_VSYNC                      1:1
#define RAW_INT_750LE_PRIMARY_VSYNC_INACTIVE             0
#define RAW_INT_750LE_PRIMARY_VSYNC_ACTIVE               1
#define RAW_INT_750LE_PRIMARY_VSYNC_CLEAR                1
#define RAW_INT_750LE_VGA_VSYNC                          0:0
#define RAW_INT_750LE_VGA_VSYNC_INACTIVE                 0
#define RAW_INT_750LE_VGA_VSYNC_ACTIVE                   1
#define RAW_INT_750LE_VGA_VSYNC_CLEAR                    1

#define INT_STATUS_750LE                                 0x080294
#define INT_STATUS_750LE_RESERVED1                       31:3
#define INT_STATUS_750LE_SECONDARY_VSYNC                 2:2
#define INT_STATUS_750LE_SECONDARY_VSYNC_INACTIVE        0
#define INT_STATUS_750LE_SECONDARY_VSYNC_ACTIVE          1
#define INT_STATUS_750LE_PRIMARY_VSYNC                   1:1
#define INT_STATUS_750LE_PRIMARY_VSYNC_INACTIVE          0
#define INT_STATUS_750LE_PRIMARY_VSYNC_ACTIVE            1
#define INT_STATUS_750LE_VGA_VSYNC                       0:0
#define INT_STATUS_750LE_VGA_VSYNC_INACTIVE              0
#define INT_STATUS_750LE_VGA_VSYNC_ACTIVE                1

#define INT_MASK_750LE                                   0x080298
#define INT_MASK_750LE_RESERVED1                         31:3
#define INT_MASK_750LE_SECONDARY_VSYNC                   2:2
#define INT_MASK_750LE_SECONDARY_VSYNC_DISABLE           0
#define INT_MASK_750LE_SECONDARY_VSYNC_ENABLE            1
#define INT_MASK_750LE_PRIMARY_VSYNC                     1:1
#define INT_MASK_750LE_PRIMARY_VSYNC_DISABLE             0
#define INT_MASK_750LE_PRIMARY_VSYNC_ENABLE              1
#define INT_MASK_750LE_VGA_VSYNC                         0:0
#define INT_MASK_750LE_VGA_VSYNC_DISABLE                 0
#define INT_MASK_750LE_VGA_VSYNC_ENABLE                  1

/* SM750HS new register and values for PLL control */
#define CRT_PLL1_750HS                         0x802a8    
#define CRT_PLL1_750HS_25MHZ                   0x00
#define CRT_PLL1_750HS_40MHZ                   0x01
#define CRT_PLL1_750HS_65MHZ                   0x03
#define CRT_PLL1_750HS_78MHZ                   0x05
#define CRT_PLL1_750HS_74MHZ                   0x04
#define CRT_PLL1_750HS_80MHZ                   0x05
#define CRT_PLL1_750HS_108MHZ                  0x06
#define CRT_PLL1_750HS_162MHZ                  0x21
#define CRT_PLL1_750HS_148MHZ                  0x20
#define CRT_PLL1_750HS_193MHZ                  0x22

/*
#define CRT_PLL1_750HS_F_25MHZ	               0x1D40A02
#define CRT_PLL1_750HS_F_40MHZ	               0x3940801
#define CRT_PLL1_750HS_F_65MHZ	               0x3940D01
#define CRT_PLL1_750HS_F_78MHZ	               0x1540F82
#define CRT_PLL1_750HS_F_74MHZ	               0x1541D82
#define CRT_PLL1_750HS_F_80MHZ	               0x3941001
#define CRT_PLL1_750HS_F_108MHZ	               0x3941B01
#define CRT_PLL1_750HS_F_162MHZ	               0x3942881
#define CRT_PLL1_750HS_F_148MHZ	               0x1541D82
#define CRT_PLL1_750HS_F_193MHZ	               0x1542682
*/
#define CRT_PLL1_750HS_F_25MHZ	               0x23d40f02
#define CRT_PLL1_750HS_F_40MHZ	               0x23940801
#define CRT_PLL1_750HS_F_65MHZ	               0x23940d01
#define CRT_PLL1_750HS_F_78MHZ	               0x23540F82
#define CRT_PLL1_750HS_F_74MHZ	               0x23941dc2
#define CRT_PLL1_750HS_F_80MHZ	               0x23941001
#define CRT_PLL1_750HS_F_80MHZ_1152	        0x23540fc2
#define CRT_PLL1_750HS_F_108MHZ	               0x23b41b01
#define CRT_PLL1_750HS_F_162MHZ	               0x23480681
#define CRT_PLL1_750HS_F_148MHZ	               0x23541dc2
#define CRT_PLL1_750HS_F_193MHZ	               0x234807c1

#define CRT_PLL1_750HS_A_25MHZ	               0x1D40A02
#define CRT_PLL1_750HS_A_40MHZ	               0x3940801
#define CRT_PLL1_750HS_A_65MHZ	               0x3940D01
#define CRT_PLL1_750HS_A_78MHZ	               0x1540F82
#define CRT_PLL1_750HS_A_74MHZ	               0x1541D82
#define CRT_PLL1_750HS_A_80MHZ	               0x3941001
#define CRT_PLL1_750HS_A_108MHZ	               0x3941B01
#define CRT_PLL1_750HS_A_162MHZ	               0x3942881
#define CRT_PLL1_750HS_A_148MHZ	               0x1541D82
#define CRT_PLL1_750HS_A_193MHZ	               0x1542682

#define CRT_PLL2_750HS                         0x802ac
#define CRT_PLL2_750HS_25MHZ                   0x0
#define CRT_PLL2_750HS_40MHZ                   0x0
#define CRT_PLL2_750HS_65MHZ                   0x0
#define CRT_PLL2_750HS_78MHZ                   0x0
#define CRT_PLL2_750HS_74MHZ                   0x0
#define CRT_PLL2_750HS_80MHZ                   0x0
#define CRT_PLL2_750HS_108MHZ                  0x0
#define CRT_PLL2_750HS_162MHZ                  0x0
#define CRT_PLL2_750HS_148MHZ                  0x0
#define CRT_PLL2_750HS_193MHZ                  0x0

#define CRT_PLL2_750HS_F_25MHZ	               0x206B851E
#define CRT_PLL2_750HS_F_40MHZ	               0x30000000
#define CRT_PLL2_750HS_F_65MHZ	               0x40000000
#define CRT_PLL2_750HS_F_78MHZ	               0x50E147AE
#define CRT_PLL2_750HS_F_74MHZ	               0x602B6AE7
#define CRT_PLL2_750HS_F_80MHZ	               0x70000000
#define CRT_PLL2_750HS_F_108MHZ	               0x80000000
#define CRT_PLL2_750HS_F_162MHZ	               0xA0000000
#define CRT_PLL2_750HS_F_148MHZ	               0xB0CCCCCD
#define CRT_PLL2_750HS_F_193MHZ	               0xC0872B02

#define CRT_PLL2_750HS_A_25MHZ	               0x206B851E
#define CRT_PLL2_750HS_A_40MHZ	               0x30000000
#define CRT_PLL2_750HS_A_65MHZ	               0x40000000
#define CRT_PLL2_750HS_A_78MHZ	               0x50E147AE
#define CRT_PLL2_750HS_A_74MHZ	               0x602B6AE7
#define CRT_PLL2_750HS_A_80MHZ	               0x70000000
#define CRT_PLL2_750HS_A_108MHZ	               0x80000000
#define CRT_PLL2_750HS_A_162MHZ	               0xA0000000
#define CRT_PLL2_750HS_A_148MHZ	               0xB0CCCCCD
#define CRT_PLL2_750HS_A_193MHZ	               0xC0872B02

/* SM750HS, the following PLL are for BIOS to set VGA modes.
   DDK don't use them. Just keep here for the record. */
#define VGA1_PLL1_750HS                               0x802b0
#define VGA1_PLL2_750HS                               0x802b4
#define VGA2_PLL1_750HS                               0x802b8
#define VGA2_PLL2_750HS                               0x802bc

/* Palette RAM */

/* Panel Pallete register starts at 0x080400 ~ 0x0807FC */
#define PRIMARY_PALETTE_RAM                             0x080400

/* Panel Pallete register starts at 0x080C00 ~ 0x080FFC */
#define SECONDARY_PALETTE_RAM                           0x080C00
