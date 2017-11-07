/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  HARDWARE.h --- Voyager GX SDK 
*  This file contains the definitions for the physical hardware.
* 
*******************************************************************/
#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/* Use IO Port to access the MMIO 
   NOTE:
        The easiest way to run the application in MS-DOS when using the SM750 as 
        secondary adapter is using the following steps:
        1. Disable the VGA enable bit in the primary card's PCI to PCI bridge.
           This bit is location at offset 0x3E bit 3. --> This can be implemented
           in the application or run the PCI Tools to modify the PCI to PCI bridge manually.
        2. Enable the VGA enable bit in the secondary (SM750) card's PCI to PCI bridge.
           --> This can be implemented
           in the application or run the PCI Tools to modify the PCI to PCI bridge manually.
        3. Enable "USE_IO_PORT" definition and compile it. --> It will automatically 
           enable the IO Port Access in the PCI Configuration Register offset 4, bit 1.
        4. Configure the DOS system to use Serial port. Hints: Use "mode" command
           and "ctty com1". It requires NULL model serial cable and another system to
           operate the DOS from the other serial port. --> This step can be done in 
           any sequence prior running the application. 
        5. Run the application from the serial port.
 */
//#define USE_IO_PORT

/* Use 0xA0000 as the frame buffer access. */
//#define USE_A0000

/* Silicon Motion PCI vendor ID */
#define SMI_PCI_VENDOR_ID               0x126F
#define HIS_PCI_VENDOR_ID               0x19E5

/* Maximum number of devices with the same ID supported by this library. */
#define MAX_SMI_DEVICE                  4

/* List of Silicon Motion PCI Device ID that is supported by this library. */
#define SMI_DEVICE_ID_SM750             0x0750
#define SMI_DEVICE_ID_SM718             0x0718
#define HIS_DEVICE_ID_SM750             0x1711

/* Size of the SM750 MMIO and memory */
#define SM750_PCI_ALLOC_MMIO_SIZE       (2*1024*1024)
#define SM750_PCI_ALLOC_MEMORY_SIZE     (64*1024*1024)

/* Register types */
typedef enum _reg_type_t
{
    MISCELLANEOUS_REGISTER,
    SEQUENCER_REGISTER,
    CRT_REGISTER,
    GRAPHICS_REGISTER,
    OTHER_REGISTER
}
reg_type_t;

#ifdef USE_A0000
/*
 * Set the bank size when using A0000h memory addressing .
 */
void setBankSize(
    unsigned long bankSize
);
#endif

/*
 * Get the physical address of an offset location in frame buffer.
 * In DOS, the physical and the logical address most likely are the same.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getPhysicalAddress(unsigned long offset /* Offset from base of physical frame buffer */);

/*
 * Get the logical address of an offset location in frame buffer.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getAddress(unsigned long offset /*Offset from base of frame buffer*/);

/*
 * Get the IRQ number of the current device.
 */
unsigned char getIRQ(void);

/*
 * This function detects what SMI chips is in the system.
 *
 * Return: A non-zero device ID if SMI chip is detected.
 *         Zero means NO SMI chip is detected.
 */
unsigned short detectDevices(void);

/*
 *  Function to return vendor ID, chip ID and revision of the current chip, if any.
 */
unsigned short getVendorId(void);
unsigned short getDeviceId(void);
unsigned char  getDeviceRev(void);

/*
 * How many devices of the same ID are there.
 */
unsigned short getNumOfDevices(void);

/*
 * This function sets up the current accessible device, if more
 * than one of the same ID exist in the system.
 *
 * Note:
 * Single device application don't need to call this function.
 * This function is to control multiple devices.
 */
long setCurrentDevice(unsigned short dev);
//unsigned short getCurrentDevice(void);
#define getCurrentDevice(...) SM750

/* Video Memory read/write functions */
unsigned char peekByte(unsigned long offset);
unsigned short peekWord(unsigned long offset);
unsigned long peekDWord(unsigned long offset);
void poke_4_Byte(unsigned long offset, unsigned char *buf);
void pokeByte(unsigned long offset, unsigned char value);
void pokeWord(unsigned long offset, unsigned short value);
void pokeDWord(unsigned long offset, unsigned long value);
#if 0 //by ilena
/* MMIO read/write functions */
unsigned char peekRegisterByte(unsigned long offset);
unsigned short peekRegisterWord(unsigned long offset);
unsigned long peekRegisterDWord(unsigned long offset);
void pokeRegisterByte(unsigned long offset, unsigned char value);
void pokeRegisterWord(unsigned long offset, unsigned short value);
void pokeRegisterDWord(unsigned long offset, unsigned long value);
#endif

#if 0   /* For Testing Purpose */
/*
 * Read SM718 PCI byte value 
 */
unsigned char readDevicePCIByte(
    unsigned short offset
);

/*
 * Read SM718 PCI word value 
 */
unsigned short readDevicePCIWord(
    unsigned short offset
);

/*
 * Read SM718 PCI dword value 
 */
unsigned long readDevicePCIDWord(
    unsigned short offset
);

/*
 * Write SM718 PCI byte value 
 */
long writeDevicePCIByte(
    unsigned short offset,
    unsigned char value
);

/*
 * Write SM718 PCI word value 
 */
long writeDevicePCIWord(
    unsigned short offset,
    unsigned short value
);

/*
 * Write SM718 PCI dword value 
 */
long writeDevicePCIDWord(
    unsigned short offset,
    unsigned long value
);
#endif

#endif /* _HARDWARE_H_ */
