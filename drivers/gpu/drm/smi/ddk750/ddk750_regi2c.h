/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  RegI2C.h --- Voyager GX SDK 
*  This file contains the definitions for the HW I2C registers.
* 
*******************************************************************/
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
