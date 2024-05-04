/*
 * ugui_SSD1963.c
 *
 *  Created on: Dec 4, 2023
 *      Author: leoja
 */


/*******************************************************************************
* File Name: ugui_SSD1963.c
*
* Description:
*  This is a driver for the uGui graphical library developed by
*  Achim Döbler.
*  It is for SSD1963 graphic controller found in a lot of low cost graphics
*  chinese displays an to be used with PSoC microcontrollers from Cypress.
*  Will test it with other PSoC micros as soon as I can.
*
* Note:
*  For more information about uGui...
*  Website: http://www.embeddedlightning.com/ugui/
*  Git: https://github.com/achimdoebler/UGUI
*  Forum: http://www.embeddedlightning.com/forum/
*  PDF Reference manual (excellent): http://www.embeddedlightning.com/download/reference-guide/?wpdmdl=205
*
*  Thanks to Achim Döbler for such a god job.
*
* Log version:
*  1.0 - June, 2018.       First version.
*
********************************************************************************
* Copyright (c) 2018 Andres F. Navas
* This driver follows the same license than the uGui library.
*******************************************************************************/

#include "ugui_types.h"
#include "ugui_SSD1963.h"
#include "stdint.h"
#include "stm32f7xx_hal.h"

#define FMC_DELAY_CYCLES 12

//////      Private Defines   ///////
#define HDP (DISPLAY_WIDTH - 1)
#define VDP (DISPLAY_HEIGHT - 1)

#define LCD_REG              (*((volatile uint16_t *) 0x60000000)) 	/* RS = 0 */
#define LCD_RAM              (*((volatile uint16_t *) 0x60000100)) 	/* RS = 1 */

#ifdef USE_COLOR_RGB565
#define DATA_t uint16_t
#endif
#ifdef USE_COLOR_RGB888
#define DATA_t uint8_t
#endif


//////      Private Function Prototypes   ///////

void write_command(uint8_t index);
void write_data(uint16_t data);
uint16_t read_data(void);
void write_multi_data(uint16_t *data, uint16_t size);


//////      Public Function Definitions  ///////

void SSD1963_Reset()
{
    write_command(0x01);         //Software reset
    HAL_Delay(10);
}

void SSD1963_Init()
{
    SSD1963_Reset();                    //Software reset

    write_command(0xe0);
    write_data(0x01);            //Enable PLL
    //HAL_Delay(50);

    write_command(0xe0);
    write_data(0x03);            //Lock PLL
    //HAL_Delay(50);
    SSD1963_Reset();                    //Software reset
    //HAL_Delay(50);

    write_command(0xb0);  //set LCD mode set TFT 18Bits mode

	write_data(0x08); //set TFT Mode - 0x0c
    write_data(0x80); //set TFT mode and hsync + vsync + DEN mode
    write_data(0x01); //set horizontal size = 480 - 1 hightbyte
    write_data(0xdf); //set horizontal size = 480 - 1 lowbyte
    write_data(0x01); //set vertical sive = 272 - 1 hightbyte
    write_data(0x0f); //set vertical size = 272 - 1 lowbyte
    write_data(0x00); //set even/odd line RGB seq

    write_command(0xf0); //set pixel data I/F format = 16 bit
    write_data(0x03);

    //write_command(0x3a); //set RGB format = 6 6 6
    //write_data(0x60);

    write_command(0xe6); //set PCLK freq = 4.94 MHz; pixel clock frequency
    write_data(0x01);    //02
    write_data(0x45);    //ff
    write_data(0x47);    //ff

    write_command(0xb4); //set HBP
    write_data(0x02); //set Hsync = 600
    write_data(0x0d);
    write_data(0x00);    //set HBP 68
    write_data(0x2b);
    write_data(0x28);    //set VBP 16
    write_data(0x00);    //Set Hsync start position
    write_data(0x00);
    write_data(0x00);    //set Hsync pulse subpixel start pos

    write_command(0xb6); //set VBP
    write_data(0x01);    //set Vsync total 360
    write_data(0x1d);
    write_data(0x00);    //set VBP = 19
    write_data(0x0c);
    write_data(0x09);    //set Vsync pulse 8
    write_data(0x00);    //set Vsync pulse start pos
    write_data(0x00);

    write_command(0x2a); //set column address
    write_data(0x00);    //set start column address 0
    write_data(0x00);
    write_data(0x01);    //set end column address = 479
    write_data(0xdf);

    write_command(0x2b); //set page address
    write_data(0x00);    //set start page address = 0
    write_data(0x00);
    write_data(0x01);    //set end column address = 479
    write_data(0x0f);

    write_command(0x13); //set normal mode
    write_command(0x38); //set normal mode
    write_command(0x29); //set display on
}

void SSD1963_WindowSet(unsigned int s_x, unsigned int e_x, unsigned int s_y, unsigned int e_y)
{
    uint16_t data[4];

    data[0] = ((s_x)>>8) & 0x00FF;                   //SET start column address
    data[1] = (s_x) & 0x00FF;
    data[2] = ((e_x)>>8) & 0x00FF;			        //SET end column address
    data[3] = (e_x) & 0x00FF;
	write_command(0x2a);		        //SET column address
    write_multi_data(data, 4);


    data[0] = ((s_y)>>8) & 0x00FF;                   //SET start row address
    data[1] = (s_y) & 0x00FF;
    data[2] = ((e_y)>>8) & 0x00FF;			        //SET end row address
    data[3] = (e_y) & 0x00FF;
	write_command(0x2b);		        //SET row address
    write_multi_data(data, 4);
}

void SSD1963_PSet(UG_S16 x, UG_S16 y, UG_COLOR c)
{
    if((x < 0) ||(x >= DISPLAY_WIDTH) || (y < 0) || (y >= DISPLAY_HEIGHT)) return;

    SSD1963_WindowSet(x, x + 1, y, y + 1);
    write_data(c);
}

void SSD1963_WriteMemoryStart()  // command to start writing pixels to frame buffer. Use before SSD1963_ConsecutivePSet
{
	write_command(0x2c);
}

void SSD1963_ConsecutivePSet(UG_COLOR c)  // Write pixel data without setting frame. Use SSD1963_WriteMemoryStart() before first pixel
{
    write_data(c);
}

UG_RESULT HW_FillFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c)
{
    uint16_t loopx, loopy;

    if((x1 < 0) ||(x1 >= DISPLAY_WIDTH) || (y1 < 0) || (y1 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
    if((x2 < 0) ||(x2 >= DISPLAY_WIDTH) || (y2 < 0) || (y2 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;

    SSD1963_WindowSet(x1,x2,y1,y2);

    write_command(0x2c);
    for (loopx = x1; loopx < x2 + 1; loopx++)
    {
        for (loopy = y1; loopy < y2 + 1; loopy++)
        {
            write_data(c);
        }
    }

    return UG_RESULT_OK;
}

UG_RESULT HW_DrawLine( UG_S16 x1 , UG_S16 y1 , UG_S16 x2 , UG_S16 y2 , UG_COLOR c )
{
    if((x1 < 0) ||(x1 >= DISPLAY_WIDTH) || (y1 < 0) || (y1 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
    if((x2 < 0) ||(x2 >= DISPLAY_WIDTH) || (y2 < 0) || (y2 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;

    // If it is a vertical or a horizontal line, draw it.
    // If not, then use original drawline routine.
    if ((x1 == x2) || (y1 == y2))
    {
        HW_FillFrame(x1, y1, x2, y2, c);
        return UG_RESULT_OK;
    }

    return UG_RESULT_FAIL;
}

UG_RESULT HW_DrawImage(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, uint8_t *image, uint16_t pSize)
{

    if((x1 < 0) ||(x1 >= DISPLAY_WIDTH) || (y1 < 0) || (y1 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
    if((x2 < 0) ||(x2 >= DISPLAY_WIDTH) || (y2 < 0) || (y2 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;

    SSD1963_WindowSet(x1,x2,y1,y2);

    write_command(0x2c);
    write_multi_data((DATA_t*)image, pSize*3);

    return UG_RESULT_OK;
}

//
//UG_RESULT HW_DrawImage_Special(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, uint8_t *image, uint16_t pSize)
//{
//
//	if((x1 < 0) ||(x1 >= DISPLAY_WIDTH) || (y1 < 0) || (y1 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
//	if((x2 < 0) ||(x2 >= DISPLAY_WIDTH) || (y2 < 0) || (y2 >= DISPLAY_HEIGHT)) return UG_RESULT_FAIL;
//
//	SSD1963_WindowSet(x1,x2,y1,y2);
//
//	write_command(0x2c);
//
//    GraphicLCDIntf_WriteM8_Compressed(1, image, pSize);
//
//    return UG_RESULT_OK;
//}
//
//void GraphicLCDIntf_WriteM8_Compressed(uint8 d_c, uint8 wrData[], uint16 num)
//{
//    uint32 i;
//    uint8 j;
//    uint8 k;
//    uint8 l;
//
//    uint8_t* color;
//
//    uint8_t BLUE[] = {0x02, 0x28, 0x51};
//    uint8_t GOLD[] = {0xFF, 0xBF, 0x00};
//    uint8_t WHITE[] = {0xFF, 0xFF, 0xFF};
//
//    for(i = 0u; i < num; i++)
//    {
//
//        if(wrData[i]&0x80){
//            color = WHITE;
//            k = wrData[i]&0x7F;
//        } else {
//            if(wrData[i]&0x40)
//                color = BLUE;
//            else
//                color = GOLD;
//
//            k = wrData[i]&0x3F;
//        }
//
//        for(l=0; l<=k; l++){
//            for(j = 0; j<3; j++){
//                while((GraphicLCDIntf_STATUS_REG & GraphicLCDIntf_CMD_QUEUE_FULL) != 0u)
//                {
//                    /* The command queue is full */
//                }
//                GraphicLCDIntf_CMD_FIFO_REG = d_c;
//
//                #if (GraphicLCDIntf_BUS_WIDTH == 16u)
//                    CY_SET_REG16(GraphicLCDIntf_DATA_FIFO_PTR, wrData[i]);
//                #else /* 8-bit interface */
//                    GraphicLCDIntf_DATA_FIFO_REG = color[j];
//                #endif /* GraphicLCDIntf_BUS_WIDTH == 16u */
//            }
//        }
//    }
//}


//////      Private Function Definitions   ///////

void write_command(uint8_t index)
{
	LCD_REG	= index;
	for(uint16_t i = 0; i<FMC_DELAY_CYCLES; i++);  // delay to allow FMC to complete write operation, replace with something that actually checks for FMC completion?
}


void write_data(DATA_t data)
{
	LCD_RAM = data;
	for(uint16_t i = 0; i<FMC_DELAY_CYCLES; i++);   // delay to allow FMC to complete write operation, replace with something that actually checks for FMC completion?
 }


uint16_t read_data(void)
{
	return LCD_RAM;
}

void write_multi_data(DATA_t *data, uint16_t size)
{
    for(int16_t i = 0; i < size; i++)
    {
    	LCD_RAM = data[i];
    	for(uint16_t j = 0; j<FMC_DELAY_CYCLES; j++);   // delay to allow FMC to complete write operation, replace with something that actually checks for FMC completion?
    }
}

/* [] END OF FILE */
