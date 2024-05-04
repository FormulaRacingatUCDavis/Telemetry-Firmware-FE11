/*
 * frucd_display.h
 *
 *  Created on: Dec 4, 2023
 *      Author: leoja
 */

#ifndef INC_FRUCD_DISPLAY_H_
#define INC_FRUCD_DISPLAY_H_

/* ========================================
 *
 * APIs for updating FRUCD Dashboard display
 *
 * ========================================
*/

#include <stdio.h>
#include "ugui.h"
#include "ugui_SSD1963.h"
#include "stm32f7xx_hal.h"
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "frucd_defines.h"
#include "fsm.h"

// PUBLIC FUNCTION PROTOTYPES //
void Display_Init();
void Display_CalibrateScreen();
void Display_DebugTemplate();
void Display_DriveTemplate();
void Display_Update();

/* [] END OF FILE */


#endif /* INC_FRUCD_DISPLAY_H_ */
