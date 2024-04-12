/**
  ******************************************************************************
  * Project		: PC9
  * File Name           : LCD20x4.c
  * Author 	        : Nguyen Tran Duy
  * Start	        : 15/12/2016
  * Stop                : 15/6/2017
  * Version		: 1.7
  ******************************************************************************
  * Description:
  ******************************************************************************
  */
#include "FreeRTOS.h"
#include "task.h"
#include "TextLcd.h"


void LCD_Init(void)
{
  LCD_Initial();
}
void Delay(volatile uint32_t nCount)
{
 while(nCount--)
 {
 }
}
void LCD_BlinkOnOff(u8 Control)
{
 LCD_Write(0,Control);
}
void LCD_Initial(void)
{
  /*4bit-4line-5x8 dots*/
  LCD_Write(0,0x28);
  /**/
  LCD_Write(0,0x0c); 
  /*display on- cursor off - blinh on*/ 
  LCD_Write(0,0x06);
  //LCD_Write(0,0x10);
  LCD_Write(0,0x02);
  LCD_Write(0,0x01);  
}
void LCD_Test(int8_t *buff)
{
 // uint8_t buff[]="STOP";
  LCD_Puts(3,0,buff);
}
void LCD_Write(uint8_t type,uint8_t data)
{
   vTaskDelay(3);
  //for(uint16_t i=0;i<10000;i++);
  if(type)
  {
          PIN_HIGH(RS_PORT,RS_PIN);
  }else
  {
          PIN_LOW(RS_PORT,RS_PIN);
  }
  PIN_HIGH(EN_PORT,EN_PIN);
  if(data&0x80)
  {
          PIN_HIGH(D7_PORT,D7_PIN);
  }else
  {
          PIN_LOW(D7_PORT,D7_PIN);
  }
  
  if(data&0x40)
  {
          PIN_HIGH(D6_PORT,D6_PIN);
  }else
  {
          PIN_LOW(D6_PORT,D6_PIN);
  }
  
  if(data&0x20)
  {
          PIN_HIGH(D5_PORT,D5_PIN);
  }else
  {
          PIN_LOW(D5_PORT,D5_PIN);
  }
  
  if(data&0x10)
  {
          PIN_HIGH(D4_PORT,D4_PIN);
  }else
  {
          PIN_LOW(D4_PORT,D4_PIN);
  }
//  PIN_HIGH(EN_PORT,EN_PIN);
  PIN_LOW(EN_PORT,EN_PIN);
  
  //Send Low Nibble
  PIN_HIGH(EN_PORT,EN_PIN);
  if(data&0x08)
  {
          PIN_HIGH(D7_PORT,D7_PIN);
  }else
  {
          PIN_LOW(D7_PORT,D7_PIN);
  }
  
  if(data&0x04)
  {
          PIN_HIGH(D6_PORT,D6_PIN);
  }else
  {
          PIN_LOW(D6_PORT,D6_PIN);
  }
  
  if(data&0x02)
  {
          PIN_HIGH(D5_PORT,D5_PIN);
  }else
  {
          PIN_LOW(D5_PORT,D5_PIN);
  }
  
  if(data&0x01)
  {
          PIN_HIGH(D4_PORT,D4_PIN);
  }else
  {
          PIN_LOW(D4_PORT,D4_PIN);
  }
//  PIN_HIGH(EN_PORT,EN_PIN);
  PIN_LOW(EN_PORT,EN_PIN);

}

void LCD_Puts(uint8_t x, uint8_t y, int8_t *string)
{
  LCD_MoveCursor(x,y);   
  while(*string)
  {
    LCD_Write(1,*string);
    string++;
  }
}
void LCD_Clear(void)
{
  LCD_Write(0,0x01);
}
/*x: row, y col*/
void LCD_MoveCursor(u8 x,u8 y)
{
  switch(x)
  {
    case 0: //Row 0
            LCD_Write(0,0x80+0x00+y);
            break;
    case 1: //Row 1
            LCD_Write(0,0x80+0x40+y);
            break;
    case 2: //Row 2
            LCD_Write(0,0x80+0x14+y);
            break;
    case 3: //Row 3
            LCD_Write(0,0x80+0x54+y);
            break;
  }
}