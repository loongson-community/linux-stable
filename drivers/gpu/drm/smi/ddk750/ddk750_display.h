/*******************************************************************
* 
*         Copyright (c) 2007 by Silicon Motion, Inc. (SMI)
* 
*  All rights are reserved. Reproduction or in part is prohibited
*  without the written consent of the copyright owner.
* 
*  display.h --- SM750/SM718 DDK
*  This file contains the function prototypes for the display.
* 
*******************************************************************/
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "ddk750_mode.h"

typedef enum _disp_path_t
{
    PANEL_PATH = 0,
    CRT_PATH   = 1,
}
disp_path_t;

typedef enum _disp_state_t
{
    DISP_OFF = 0,
    DISP_ON  = 1,
}
disp_state_t;

typedef enum _disp_output_t
{
    NO_DISPLAY,             /* All display off. */
    LCD1_ONLY,              /* LCD1 only */
    LCD2_ONLY,              /* LCD2 only */
    CRT2_ONLY,              /* CRT2 only */
    LCD1_CRT2_SIMUL,        /* Both LCD1 and CRT2 displaying the same content. */
    LCD1_LCD2_SIMUL,        /* Both LCD1 and LCD2 displaying the same content. */
    CRT2_LCD2_SIMUL,        /* CRT2 and LCD2 displaying the same content. */
    LCD1_LCD2_CRT2_SIMUL,   /* LCD1, LCD2, and CRT2 displaying the same content. */
    LCD1_CRT2_DUAL,         /* LCD1 and CRT2 displaying different contents. */
    LCD1_LCD2_DUAL,         /* LCD1 and LCD2 displaying different contents. */
    LCD1_LCD2_CRT2_DUAL     /* LCD2 and CRT2 displaying the same content while
                               the Panel displaying different content. */
}
disp_output_t;

typedef enum _panel_type_t
{
    TFT_18BIT = 0,
    TFT_24BIT,
    TFT_36BIT
}
panel_type_t;

void setPath(
    disp_path_t dispPath, 
    disp_control_t dispControl, 
    disp_state_t dispState
);
/*
 * This functions sets the CRT Path.
 */
void setCRTPath(disp_control_t dispControl);

/*
 * This functions uses software sequence to turn on/off the panel.
 */
void swPanelPowerSequence(disp_state_t dispState, unsigned long vsync_delay);

/* 
 * This function turns on/off the DAC for CRT display control.
 * Input: On or off
 */
void setDAC(disp_state_t state);

/*
 * This function turns on/off the display control.
 * Currently, it for CRT and Panel controls only.
 * Input: Panel or CRT, or ...
 *        On or Off.
 */
void setDisplayControl(disp_control_t dispControl, disp_state_t dispState);

/*
 * This function set the logical display output.
 *
 * The output is called logical because it is independent of physical implementation.
 * For example, CRT2 only mode is not using the internal secondary control. It uses the
 * Primary Control with its output directed to CRT DAC.
 *
 * Input:
 *      output          - Logical Display output
 *      dispCtrlUsage   - Display Control Flag Usage:
 *                          0 : Use primary display control (PRIMARY_CTRL) to control 
 *                              primary output (LCD1 & CRT1) and use secondary display 
 *                              control (SECONDARY_CTRL) to control secondary output 
 *                              (CRT2 & LCD2)
 *                          1 : Use primary display control (PRIMARY_CTRL) to control 
 *                              secondary output (LCD2 & CRT2) and use secondary display 
 *                              control (SECONDARY_CTRL) to control primary output 
 *                              (LCD1 & CRT1)
 *
 * Output:
 *      0   - Success
 *     -1   - Fail 
 */
long setLogicalDispOutput(
    disp_output_t output,
    unsigned char dispCtrlUsage
);

/*
 * This function set the logical display output.
 *
 * The output is called logical because it is independent of physical implementation.
 * For example, CRT2 only mode is not using the internal secondary control. It uses the
 * Primary Control with its output directed to CRT DAC.
 *
 * Input:
 *		isSecondDispay  - Enable primary or secondary control
 *							0 : Enable primary display control
 *							1 : Enable secondary display control
 *      output          - Logical Display output
 *      dispCtrlUsage   - Display Control Flag Usage:
 *                          0 : Use primary display control (PRIMARY_CTRL) to control 
 *                              primary output (LCD1 & CRT1) and use secondary display 
 *                              control (SECONDARY_CTRL) to control secondary output 
 *                              (CRT2 & LCD2)
 *                          1 : Use primary display control (PRIMARY_CTRL) to control 
 *                              secondary output (LCD2 & CRT2) and use secondary display 
 *                              control (SECONDARY_CTRL) to control primary output 
 *                              (LCD1 & CRT1)
 *
 * Output:
 *      0   - Success
 *     -1   - Fail 
 */
long setLogicalDispOutputExt(
    long isSecondDisplay,
    disp_output_t output,
    unsigned char dispCtrlUsage
);
/*
 * Use vertical sync as time delay function.
 * Input:
 *          dispControl - Display Control (PRIMARY_CTRL or SECONDARY_CTRL) 
 *          vsync_count - Number of vertical sync to wait.
 *
 * Note:
 *      This function is waiting for the next vertical sync.
 */
void waitNextVerticalSync(disp_control_t dispControl, unsigned long vsync_count);

/*
 * Use panel vertical sync line as time delay function.
 * This function does not wait for the next VSync. Instead, it will wait
 * until the current line reaches the Vertical Sync line.
 * This function is really useful when flipping display to prevent tearing.
 *
 * Input: display control (PRIMARY_CTRL or SECONDARY_CTRL)
 */
void waitVSyncLine(disp_control_t dispControl);

/*
 * This function gets the panel type
 *
 * Output:
 *      panelType   - The type of the panel to be set  
 */
panel_type_t getPanelType(void);

long setPanelType(panel_type_t panelType);


/*
 * This function gets the CRT Monitor Detection Threshold value.
 *
 * Input:
 *      pRedValue   - Pointer to a variable to store the Red color threshold
 *                    value.
 *      pGreenValue - Pointer to a variable to store the Green color threshold
 *                    value.
 *      pBlueValue  - Pointer to a variable to store the Blue color threshold
 *                    value. 
 */
void getCRTDetectThreshold(
    unsigned char *pRedValue,
    unsigned char *pGreenValue,
    unsigned char *pBlueValue
);

/*
 * This function detects if the CRT monitor is attached.
 *
 * Input:
 *      redValue    - Threshold value to be detected on the red color.
 *      greenValue  - Threshold value to be detected on the green color.
 *      blueValue   - Threshold value to be detected on the blue color.
 *
 * Output:
 *      0   - Success
 *     -1   - Fail 
 */
long ddk750_detectCRTMonitor(
    unsigned char redValue,
    unsigned char greenValue,
    unsigned char blueValue
);

/*
 * This function checks whether the scaling is enabled.
 *
 * Input:
 *      dispCtrl    - Display control to be checked.
 *
 * Output:
 *      0   - Scaling is not enabled
 *      1   - Scaling is enabled
 */
unsigned char isScalingEnabled(
    disp_control_t dispCtrl
);


#endif /* _DISPLAY_H_ */
