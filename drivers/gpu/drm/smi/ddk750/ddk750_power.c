/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  power.c --- Voyager GX SDK 
*  This file contains the source code for the power functions.
* 
*******************************************************************/
#include "ddk750_defs.h"
#include "ddk750_chip.h"
#include "ddk750_clock.h"
#include "ddk750_hardware.h"
#include "ddk750_power.h"
#include "ddk750_help.h"

//#include "ddkdebug.h"

/* Semaphore Counter for Bus Master Enable Bit */
unsigned long g_ulBusMasterSemaphoreCounter[MAX_SMI_DEVICE] = { 0, 0, 0, 0};
unsigned long g_ulPCISlaveBurstWriteSemaphoreCounter[MAX_SMI_DEVICE] = { 0, 0, 0, 0};
unsigned long g_ulPCISlaveBurstReadSemaphoreCounter[MAX_SMI_DEVICE] = { 0, 0, 0, 0};

/* Set DPMS state */
void setDPMS(DPMS_t state)
{
    unsigned long value;

    //if(ddk750_getChipType() == SM750LE)
    if(ddk750_getChipType() >= SM750LE)
    {
        value = peekRegisterDWord(SECONDARY_DISPLAY_CTRL);
        switch (state)
        {
        case DPMS_ON:
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, DPMS, 0);
            break;

        case DPMS_STANDBY:
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, DPMS, 1);
            break;

        case DPMS_SUSPEND:
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, DPMS, 2);
            break;

        case DPMS_OFF:
            value = FIELD_SET(value, SECONDARY_DISPLAY_CTRL, DPMS, 3);
            break;
        }

        pokeRegisterDWord(SECONDARY_DISPLAY_CTRL, value);
        return;
    }

    value = peekRegisterDWord(SYSTEM_CTRL);
    switch (state)
    {
       case DPMS_ON:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VPHP);
        break;

       case DPMS_STANDBY:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VPHN);
        break;

       case DPMS_SUSPEND:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VNHP);
        break;

       case DPMS_OFF:
        value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VNHN);
        break;
    }

    pokeRegisterDWord(SYSTEM_CTRL, value);
}

/*
 * This function gets the power mode, one of three modes: 0, 1 or Sleep.
 * On hardware reset, power mode 0 is default.
 */
unsigned long getPowerMode()
{

    return (FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL), POWER_MODE_CTRL, MODE));
}

/*
 * SM750/SM718 can operate in one of three modes: 0, 1 or Sleep.
 * On hardware reset, power mode 0 is default.
 */
void setPowerMode(unsigned long powerMode)
{
    unsigned long control_value = 0, previousPowerState, timeout, value;

    /* Get the current power mode ctrl register value and save it 
       before switching the power mode. */
    control_value = peekRegisterDWord(POWER_MODE_CTRL);
    previousPowerState = FIELD_GET(control_value, POWER_MODE_CTRL, MODE);

    /* Set the power mode to the requested power mode. */
    switch (powerMode)
    {
        case 0:
            control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE0);
            break;

        case 1:
            control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE1);
            break;

        case 2:
            control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, SLEEP);
            break;

        default:
            break;
    }

#if 0   /* These routines are removed since turning off the oscillator during sleep
           will require the software to reset the frame buffer memory. 
         */
    /* Set up other fields in Power Control Register */
    if (powerMode == POWER_MODE_CTRL_MODE_SLEEP)
        control_value = FIELD_SET(control_value, POWER_MODE_CTRL, OSC_INPUT, OFF);
    else
        control_value = FIELD_SET(control_value, POWER_MODE_CTRL, OSC_INPUT, ON);
#endif

    /* Program new power mode. */
    pokeRegisterDWord(POWER_MODE_CTRL, control_value);
    
    /* In SM718, the chip needs to wait until wake up from the sleep mode and wait for
       a few milliseconds before reseting the memory and resume the normal operation. */
    if ((powerMode != 2) && (previousPowerState != FIELD_GET(control_value, POWER_MODE_CTRL, MODE)))
    {
        /* Switching power mode between power mode 0 and 1 */
        
        /* Need a minimum of 16ms between power mode switching */
        waitMasterClock(16);
        
        /* Reset Memory. */
        ddk750_resetFrameBufferMemory();
    }
}

void setCurrentGate(unsigned long gate)
{
    unsigned long gate_reg;
    unsigned long mode;

    /* Get current power mode. */
    mode = getPowerMode();

    switch (mode)
    {
        case POWER_MODE_CTRL_MODE_MODE0:
            gate_reg = MODE0_GATE;
            break;

        case POWER_MODE_CTRL_MODE_MODE1:
            gate_reg = MODE1_GATE;
            break;

        default:
            gate_reg = MODE0_GATE;
            break;
    }
    pokeRegisterDWord(gate_reg, gate);
}

/*
 * This function enable/disable Bus Master
 */
void enableBusMaster(unsigned long enable)
{
    unsigned long busMasterCounter, value;

    /* Enable Bus Master as necessary.*/
	busMasterCounter = g_ulBusMasterSemaphoreCounter[getCurrentDevice()];
	value = peekRegisterDWord(SYSTEM_CTRL);

    /* Currently, only SM718 needs to enable the Bus Master enable bit. 
       The Bus Master in SM750 is enabled by default, without programming any bits. */
    if (enable)
    {
        if ((ddk750_getChipType() == SM718) && (busMasterCounter == 0))        
            pokeRegisterDWord(SYSTEM_CTRL, FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_MASTER, ON));
        
        busMasterCounter++;
    }
    else
    {
        if (busMasterCounter > 0)
            busMasterCounter--;
        
        if ((ddk750_getChipType() == SM718) && (busMasterCounter == 0))
            pokeRegisterDWord(SYSTEM_CTRL, FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_MASTER, OFF));
    }

    g_ulBusMasterSemaphoreCounter[getCurrentDevice()] = busMasterCounter;
}

/* 
 *	setPCIMasterBaseAddress
 *		This function set the PCI Master Base Address (used by bus master or DMA).
 *
 *	Input:	
 *		physicalSystemMemAddress	- System physical memory address which PCI
 *									  Master Base Address to be set to.
 *
 *	Output:
 *		The memory address to be set in the register.  
 */
unsigned long setPCIMasterBaseAddress(
	unsigned long physicalSystemMemAddress
)
{
	unsigned long pciMasterBaseAddress, remainingAddress;

    /* Set PCI Master Base Address */
    if (ddk750_getChipType() == SM750)
    {
#ifdef SM750_AA
        pciMasterBaseAddress = ((physicalSystemMemAddress & 0xFFF00000) >> 23) << 3;
        pokeRegisterDWord(PCI_MASTER_BASE, FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
        
        /* This errata only applies to System Memory. For local to local, no correction is
           needed. */
        remainingAddress = ((physicalSystemMemAddress & 0x00700000) << 3) + (physicalSystemMemAddress & 0x000FFFFF);
#else
        pciMasterBaseAddress = physicalSystemMemAddress >> 23;
        pciMasterBaseAddress = (pciMasterBaseAddress > 0xFF) ? 0xFF : pciMasterBaseAddress;
        pokeRegisterDWord(PCI_MASTER_BASE, FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
        remainingAddress = physicalSystemMemAddress - (pciMasterBaseAddress << 23);
#endif
    }
    else
    {
        pciMasterBaseAddress = physicalSystemMemAddress & 0xFFF00000;
        pokeRegisterDWord(SM718_PCI_MASTER_BASE, FIELD_VALUE(0, SM718_PCI_MASTER_BASE, ADDRESS, pciMasterBaseAddress));
        remainingAddress = physicalSystemMemAddress - pciMasterBaseAddress;
    }

    //DDKDEBUGPRINT((DMA_LEVEL, "pciMasterBaseAddress: %x\n", pciMasterBaseAddress));

	/* Send back the remaining address */
    return remainingAddress;
}

/*
 * 	This function enable/disable PCI Slave Burst Write provided the CPU supports Write Combine.
 *
 *	Input:
 *			enable		- Enable/Disable the PCI Slave Burst Write (0 = disable, 1 = enable)
 */
void enablePCISlaveBurstWrite(
	unsigned long enable
)
{
	unsigned long pciSlaveBurstWriteCounter, value;

    /* Enable PCI Slave Burst Write */
	pciSlaveBurstWriteCounter = g_ulPCISlaveBurstWriteSemaphoreCounter[getCurrentDevice()];
	value = peekRegisterDWord(SYSTEM_CTRL);

    if (enable != 0)
    {
        if ((ddk750_getChipType() == SM718) && (pciSlaveBurstWriteCounter == 0))
		{
			/* Enable PCI Slave Burst Write. */
			value = FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_BURST, ON); 
			pokeRegisterDWord(SYSTEM_CTRL, value);
        }

        pciSlaveBurstWriteCounter++;
    }
    else
    {
        if (pciSlaveBurstWriteCounter > 0)
            pciSlaveBurstWriteCounter--;
        
        if ((ddk750_getChipType() == SM718) && (pciSlaveBurstWriteCounter == 0))
		{
			/* Disable PCI Slave Burst Write */
			value = FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_BURST, OFF); 
			pokeRegisterDWord(SYSTEM_CTRL, value);
		}
    }

	g_ulPCISlaveBurstWriteSemaphoreCounter[getCurrentDevice()] = pciSlaveBurstWriteCounter;
}

/*
 * 	This function enable/disable PCI Slave Burst Read provided the CPU supports it.
 *
 *	Input:
 *			enable			- Enable/Disable the PCI Slave Burst Read (0 = disable, 1 = enable)
 *			burstReadSize	- Burst Read Size in 32-words (valid values are 1, 2, 4, and 8)
 */
void enablePCISlaveBurstRead(
	unsigned long enable,
	unsigned long burstReadSize
)
{
	unsigned long pciSlaveBurstReadCounter, value;

    /* Currently, only SM718 needs to enable the Bus Master enable bit. 
       The Bus Master in SM750 is enabled by default, without programming any bits. */
	pciSlaveBurstReadCounter = g_ulPCISlaveBurstReadSemaphoreCounter[getCurrentDevice()];
	value = peekRegisterDWord(SYSTEM_CTRL);

    if (enable != 0)
    {
        if ((ddk750_getChipType() == SM718) && (pciSlaveBurstReadCounter == 0))
		{
			value = peekRegisterDWord(SYSTEM_CTRL);

			/* Enable PCI Slave Burst Read. */
			value = FIELD_SET(value, SYSTEM_CTRL, PCI_BURST_READ, ON); 

			/* Set the Read Size */
			switch(burstReadSize)
			{
				case 1:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 1);
					break;
				case 2:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 2);
					break;
				case 4:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 4);
					break;
				default:
				case 8:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 8);
					break;
			}
			pokeRegisterDWord(SYSTEM_CTRL, value);
        }

        pciSlaveBurstReadCounter++;
    }
    else
    {
        if (pciSlaveBurstReadCounter > 0)
            pciSlaveBurstReadCounter--;
        
        if ((ddk750_getChipType() == SM718) && (pciSlaveBurstReadCounter == 0))
		{
			/* Disable PCI Slave Burst */
			value = FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_BURST_READ, OFF); 
			pokeRegisterDWord(SYSTEM_CTRL, value);
		}
    }

	g_ulPCISlaveBurstReadSemaphoreCounter[getCurrentDevice()] = pciSlaveBurstReadCounter;
}

/* 
 * This function enable/disable the 2D engine.
 */
void enable2DEngine(unsigned long enable)
{
    unsigned long gate;

    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
    {
        gate = FIELD_SET(gate, CURRENT_GATE, DE,  ON);
        gate = FIELD_SET(gate, CURRENT_GATE, CSC, ON);
    }
    else
    {
        gate = FIELD_SET(gate, CURRENT_GATE, DE,  OFF);
        gate = FIELD_SET(gate, CURRENT_GATE, CSC, OFF);
    }

    setCurrentGate(gate);
}

/* 
 * This function enable/disable the ZV Port.
 */
void enableZVPort(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable ZV Port Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
    {
        gate = FIELD_SET(gate, CURRENT_GATE, ZVPORT, ON);
#if 1
        /* Using Software I2C */
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, ON);
#endif
    }
    else
    {
        /* Disable ZV Port Gate. There is no way to know whether the GPIO pins are being used
           or not. Therefore, do not disable the GPIO gate. */
        gate = FIELD_SET(gate, CURRENT_GATE, ZVPORT, OFF);
    }
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the SSP.
 */
void enableSSP(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable SSP Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, SSP, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, SSP, OFF);
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the DMA Engine
 */
void enableDMA(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable DMA Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, DMA, ON);
    else
        gate = FIELD_SET(gate, CURRENT_GATE, DMA, OFF);

    setCurrentGate(gate);
}

/* 
 * This function enable/disable the GPIO Engine
 */
void enableGPIO(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable GPIO Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, GPIO, OFF);
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the PWM Engine
 */
void enablePWM(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable PWM Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, PWM, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, PWM, OFF);
    
    setCurrentGate(gate);
}

/* 
 * This function enable/disable the I2C Engine
 */
void enableI2C(unsigned long enable)
{
    unsigned long gate;
    
    /* Enable I2C Gate */
    gate = peekRegisterDWord(CURRENT_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_GATE, I2C, ON);        
    else
        gate = FIELD_SET(gate, CURRENT_GATE, I2C, OFF);
    
    setCurrentGate(gate);
}
