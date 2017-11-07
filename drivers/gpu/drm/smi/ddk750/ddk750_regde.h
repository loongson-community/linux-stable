/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  RegDE.h --- Voyager GX SDK 
*  This file contains the definitions for the Drawing Engine registers.
* 
*******************************************************************/
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
#define DE_WINDOW_SOURCE_BASE_EXT                       27:27
#define DE_WINDOW_SOURCE_BASE_EXT_LOCAL                 0
#define DE_WINDOW_SOURCE_BASE_EXT_EXTERNAL              1
#define DE_WINDOW_SOURCE_BASE_CS                        26:26
#define DE_WINDOW_SOURCE_BASE_CS_0                      0
#define DE_WINDOW_SOURCE_BASE_CS_1                      1
#define DE_WINDOW_SOURCE_BASE_ADDRESS                   25:0

#define DE_WINDOW_DESTINATION_BASE                      0x100044
#define DE_WINDOW_DESTINATION_BASE_EXT                  27:27
#define DE_WINDOW_DESTINATION_BASE_EXT_LOCAL            0
#define DE_WINDOW_DESTINATION_BASE_EXT_EXTERNAL         1
#define DE_WINDOW_DESTINATION_BASE_CS                   26:26
#define DE_WINDOW_DESTINATION_BASE_CS_0                 0
#define DE_WINDOW_DESTINATION_BASE_CS_1                 1
#define DE_WINDOW_DESTINATION_BASE_ADDRESS              25:0

#define DE_ALPHA                                        0x100048
#define DE_ALPHA_VALUE                                  7:0

#define DE_WRAP                                         0x10004C
#define DE_WRAP_X                                       31:16
#define DE_WRAP_Y                                       15:0

#define DE_STATUS                                       0x100050
#define DE_STATUS_CSC                                   1:1
#define DE_STATUS_CSC_CLEAR                             0
#define DE_STATUS_CSC_NOT_ACTIVE                        0
#define DE_STATUS_CSC_ACTIVE                            1
#define DE_STATUS_2D                                    0:0
#define DE_STATUS_2D_CLEAR                              0
#define DE_STATUS_2D_NOT_ACTIVE                         0
#define DE_STATUS_2D_ACTIVE                             1

/* New register for SM750LE */
#define DE_STATE1                                        0x100054
#define DE_STATE1_DE_ABORT                               0:0
#define DE_STATE1_DE_ABORT_OFF                           0
#define DE_STATE1_DE_ABORT_ON                            1

#define DE_STATE2                                        0x100058
#define DE_STATE2_DE_FIFO                                3:3
#define DE_STATE2_DE_FIFO_NOTEMPTY                       0
#define DE_STATE2_DE_FIFO_EMPTY                          1
#define DE_STATE2_DE_STATUS                              2:2
#define DE_STATE2_DE_STATUS_IDLE                         0
#define DE_STATE2_DE_STATUS_BUSY                         1
#define DE_STATE2_DE_MEM_FIFO                            1:1
#define DE_STATE2_DE_MEM_FIFO_NOTEMPTY                   0
#define DE_STATE2_DE_MEM_FIFO_EMPTY                      1
#define DE_STATE2_DE_RESERVED                            0:0

/* Color Space Conversion registers. */

#define CSC_Y_SOURCE_BASE                               0x1000C8
#define CSC_Y_SOURCE_BASE_EXT                           27:27
#define CSC_Y_SOURCE_BASE_EXT_LOCAL                     0
#define CSC_Y_SOURCE_BASE_EXT_EXTERNAL                  1
#define CSC_Y_SOURCE_BASE_CS                            26:26
#define CSC_Y_SOURCE_BASE_CS_0                          0
#define CSC_Y_SOURCE_BASE_CS_1                          1
#define CSC_Y_SOURCE_BASE_ADDRESS                       25:0

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
#define CSC_U_SOURCE_BASE_EXT                           27:27
#define CSC_U_SOURCE_BASE_EXT_LOCAL                     0
#define CSC_U_SOURCE_BASE_EXT_EXTERNAL                  1
#define CSC_U_SOURCE_BASE_CS                            26:26
#define CSC_U_SOURCE_BASE_CS_0                          0
#define CSC_U_SOURCE_BASE_CS_1                          1
#define CSC_U_SOURCE_BASE_ADDRESS                       25:0

#define CSC_V_SOURCE_BASE                               0x1000DC
#define CSC_V_SOURCE_BASE_EXT                           27:27
#define CSC_V_SOURCE_BASE_EXT_LOCAL                     0
#define CSC_V_SOURCE_BASE_EXT_EXTERNAL                  1
#define CSC_V_SOURCE_BASE_CS                            26:26
#define CSC_V_SOURCE_BASE_CS_0                          0
#define CSC_V_SOURCE_BASE_CS_1                          1
#define CSC_V_SOURCE_BASE_ADDRESS                       25:0

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
#define CSC_DESTINATION_BASE_EXT                        27:27
#define CSC_DESTINATION_BASE_EXT_LOCAL                  0
#define CSC_DESTINATION_BASE_EXT_EXTERNAL               1
#define CSC_DESTINATION_BASE_CS                         26:26
#define CSC_DESTINATION_BASE_CS_0                       0
#define CSC_DESTINATION_BASE_CS_1                       1
#define CSC_DESTINATION_BASE_ADDRESS                    25:0

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
