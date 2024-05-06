/**
  ******************************************************************************
  * @file    textlcd.h
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

#ifndef		__LCDTXT_H
#define		__LCDTXT_H

#include "stm32f10x_gpio.h"
#include "sysSetting.h"
//#define LCD20xN 

#define RS_PORT		GPIOA
#define RS_PIN		GPIO_Pin_6 

#define RW_PORT		GPIOA
#define RW_PIN		GPIO_Pin_5

#define EN_PORT		GPIOA
#define EN_PIN		GPIO_Pin_7

#define D7_PORT		GPIOA
#define D7_PIN		GPIO_Pin_11

#define D6_PORT		GPIOA
#define D6_PIN		GPIO_Pin_10

#define D5_PORT		GPIOA
#define D5_PIN		GPIO_Pin_9

#define D4_PORT		GPIOA
#define D4_PIN		GPIO_Pin_8

#define PIN_LOW(PORT,PIN)       GPIO_WriteBit(PORT,PIN,Bit_RESET);
#define PIN_HIGH(PORT,PIN)	GPIO_WriteBit(PORT,PIN,Bit_SET);

void LCD_Init(void);
void LCD_Initial(void);
void LCD_Write(uint8_t type,uint8_t data);
void LCD_Puts(uint8_t x, uint8_t y, int8_t *string);
void LCD_Clear(void);
void LCD_BlinkOnOff(u8 Control);
void LCD_DisplayAmount(uint64_t amount,u8 x1,u8 y1,u8 x2,u8 y2,bool bDisplay_DvTien);
void LCD_DisplayNumber(u32 num,u8 x,u8 y);
void LCD_MsgSendFalse(void);
void LCD_ReadLog(u8 nLog);
void LCD_ReadTotal(u8 cnt,uint64_t amount,double volume);//,SysConfig_t     *config
void LCD_ChangeInfo(char *infor,u8 index,u8 leng,u8 indexfont);
void LCD_DisplayVolume(u8 x1,u8 y1,u8 x2,u8 y2,double data,bool bDisplay_Lit);
void LCD_MoveCursor(u8 x,u8 y);
void Delay(volatile uint32_t nCount);
 
void LCD_Test(int8_t *buff);
#endif

