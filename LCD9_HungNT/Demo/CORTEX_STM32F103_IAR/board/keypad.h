/**
  ******************************************************************************
  * @file    keypad.h
  * @brief   
  *          
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

#ifndef _KEYPAD_H
#define _KEYPAD_H
#include "lapis.h"
#define   ADC_Level_Vol_High       650
/*not use 74HC */
#if (KEYPAD_USE74HC !=KP_USE)

u8 ButtonPush(void);
void set_row(u8 i);
char Get_key(void);
/*use 74*/
#else

char KEYPAD_Getkey(void);
void KEYPAD_ScanColumn(u8 c);
void KEYPAD_ColumnPullDown(u8 col);
u8   KEYPAD_ReadRow(void);
#endif
void Disble_TM3(void);
void vKeypadPollTask( void *pvParameters );
void Keypad_Init(void);
void TIMER3_Init(u32 Period);
void TIMER3_ENABLE(u32 Period);
 
#endif