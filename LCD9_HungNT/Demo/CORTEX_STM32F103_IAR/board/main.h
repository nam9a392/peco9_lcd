/**
  ******************************************************************************
  * @file    main.h
  * @brief   This file contains definitions for Leds, push-buttons
  *          and COM ports hardware resources.
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define SW1_Pin                         GPIO_Pin_0
#define SW1_GPIO_Port                   GPIOC
#define SW2_Pin                         GPIO_Pin_1
#define SW2_GPIO_Port                   GPIOC
#define SW3_Pin                         GPIO_Pin_2
#define SW3_GPIO_Port                   GPIOC
#define SW4_Pin                         GPIO_Pin_3
#define SW4_GPIO_Port                   GPIOC  
#define SW5_Pin                         GPIO_Pin_4
#define SW5_GPIO_Port                   GPIOC
#define SW6_Pin                         GPIO_Pin_5
#define SW6_GPIO_Port                   GPIOC   
#define SW7_Pin                         GPIO_Pin_6
#define SW7_GPIO_Port                   GPIOC
#define SW8_Pin                         GPIO_Pin_7
#define SW8_GPIO_Port                   GPIOC   
#define	Addr_B0		                GPIO_ReadInputDataBit(SW1_GPIO_Port,SW1_Pin)
#define	Addr_B1		                GPIO_ReadInputDataBit(SW2_GPIO_Port,SW2_Pin)
#define	Addr_B2		                GPIO_ReadInputDataBit(SW3_GPIO_Port,SW3_Pin)
#define	Addr_B3		                GPIO_ReadInputDataBit(SW4_GPIO_Port,SW4_Pin)
#define	Addr_B4		                GPIO_ReadInputDataBit(SW5_GPIO_Port,SW5_Pin)
#define	Addr_B5		                GPIO_ReadInputDataBit(SW6_GPIO_Port,SW6_Pin)
#define	Addr_B6		                GPIO_ReadInputDataBit(SW7_GPIO_Port,SW7_Pin)
#define	Addr_B7		                GPIO_ReadInputDataBit(SW8_GPIO_Port,SW8_Pin) 
#define Keypad_Loss_PW_Pin              GPIO_Pin_1
#define Keypad_Loss_PW_Port             GPIOA            
#define LCD_TX_Pin                      GPIO_Pin_2
#define LCD_TX_GPIO_Port                GPIOA
#define LCD_RX_Pin                      GPIO_Pin_3
#define LCD_RX_GPIO_Port                GPIOA
#define LCD_EN_Pin                      GPIO_Pin_4
#define LCD_EN_GPIO_Port                GPIOA
//#define LCD_RW_Pin                      GPIO_Pin_5
//#define LCD_RW_GPIO_Port                GPIOA
#define LCD_RS_Pin                      GPIO_Pin_6
#define LCD_RS_GPIO_Port                GPIOA
#define LCD_Enable_Pin                  GPIO_Pin_7
#define LCD_Enable_GPIO_Port            GPIOA

#define LCD_D4_Pin                      GPIO_Pin_8
#define LCD_D4_GPIO_Port                GPIOA
#define LCD_D5_Pin                      GPIO_Pin_9
#define LCD_D5_GPIO_Port                GPIOA
#define LCD_D6_Pin                      GPIO_Pin_10
#define LCD_D6_GPIO_Port                GPIOA
#define LCD_D7_Pin                      GPIO_Pin_11
#define LCD_D7_GPIO_Port                GPIOA

#define PFL_PIN                         GPIO_Pin_8
#define PFL_GPIO_PORT                   GPIOB
#define LAPIS_TEST_Pin                  GPIO_Pin_11
#define LAPIS_TEST_GPIO_Port            GPIOB
#define LAPIS_BLANK_Pin                 GPIO_Pin_12
#define LAPIS_BLANK_GPIO_Port           GPIOB
#define LAPIS_CLK_Pin                   GPIO_Pin_13
#define LAPIS_CLK_GPIO_Port             GPIOB
#define LAPIS_LATCH_Pin                 GPIO_Pin_14
#define LAPIS_LATCH_GPIO_Port           GPIOB
#define LAPIS_DATA_Pin                  GPIO_Pin_15
#define LAPIS_DATA_GPIO_Port            GPIOB
#define PFL_EXTI_LINE                   EXTI_Line8
#define PFL_GPIO_CLK                  RCC_APB2Periph_GPIOB
#define PFL_AFIO_CLK                  RCC_APB2Periph_AFIO  

#if 1
  #define KB_DATAOUT_PIN		GPIO_Pin_8//3
  #define KB_DATAOUT_PORT		GPIOC//B
  #define KB_DATAIN_PIN		        GPIO_Pin_9//4
  #define KB_DATAIN_PORT		GPIOC//B
  #define KB_CLOCK_PIN		        GPIO_Pin_10//5
  #define KB_CLOCK_PORT		        GPIOC//B
  #define KB_LATCH_PIN		        GPIO_Pin_12//6
  #define KB_LATCH_PORT		        GPIOC//B
  #define SPEAKER_Pin                   GPIO_Pin_0
  #define SPEAKER_GPIO_Port             GPIOB
 // #define Pin_Read165                   GPIO_ReadInputDataBit(KEYPAD_DIN_GPIO_Port,KEYPAD_DIN_Pin);

#else
  #define   COL_1_Pin                     GPIO_Pin_3
  #define   COL_1_Port                    GPIOB
  #define   COL_2_Pin                     GPIO_Pin_4
  #define   COL_2_Port                    GPIOB
  #define   COL_3_Pin                     GPIO_Pin_5
  #define   COL_3_Port                    GPIOB
  #define   COL_4_Pin                     GPIO_Pin_6
  #define   COL_4_Port                    GPIOB
  #define   ROW_1_Pin                     GPIO_Pin_8
  #define   ROW_1_Port                    GPIOC
  #define   ROW_2_Pin                     GPIO_Pin_9
  #define   ROW_2_Port                    GPIOC
  #define   ROW_3_Pin                     GPIO_Pin_10
  #define   ROW_3_Port                    GPIOC
  #define   ROW_4_Pin                     GPIO_Pin_11
  #define   ROW_4_Port                    GPIOC
  #define   ROW_5_Pin                     GPIO_Pin_12
  #define   ROW_5_Port                    GPIOC
  #define   ROW_6_Pin                     GPIO_Pin_13
  #define   ROW_6_Port                    GPIOC

  #define   ROW1_LOW                      GPIO_WriteBit(ROW_1_Port,ROW_1_Pin,Bit_RESET);
  #define   ROW1_HIGH                     GPIO_WriteBit(ROW_1_Port,ROW_1_Pin,Bit_SET);
  #define   ROW2_LOW                      GPIO_WriteBit(ROW_2_Port,ROW_2_Pin,Bit_RESET);
  #define   ROW2_HIGH                     GPIO_WriteBit(ROW_2_Port,ROW_2_Pin,Bit_SET);
  #define   ROW3_LOW                      GPIO_WriteBit(ROW_3_Port,ROW_3_Pin,Bit_RESET);
  #define   ROW3_HIGH                     GPIO_WriteBit(ROW_3_Port,ROW_3_Pin,Bit_SET);
  #define   ROW4_LOW                      GPIO_WriteBit(ROW_4_Port,ROW_4_Pin,Bit_RESET);
  #define   ROW4_HIGH                     GPIO_WriteBit(ROW_4_Port,ROW_4_Pin,Bit_SET);
  #define   ROW5_LOW                      GPIO_WriteBit(ROW_5_Port,ROW_5_Pin,Bit_RESET);
  #define   ROW5_HIGH                     GPIO_WriteBit(ROW_5_Port,ROW_5_Pin,Bit_SET);
  #define   ROW6_LOW                      GPIO_WriteBit(ROW_6_Port,ROW_6_Pin,Bit_RESET);
  #define   ROW6_HIGH                     GPIO_WriteBit(ROW_6_Port,ROW_6_Pin,Bit_SET);
      
  #define   COL1                          GPIO_ReadInputDataBit( COL_1_Port,COL_1_Pin);
  #define   COL2                          GPIO_ReadInputDataBit( COL_2_Port,COL_2_Pin);
  #define   COL3                          GPIO_ReadInputDataBit( COL_3_Port,COL_3_Pin);
  #define   COL4                          GPIO_ReadInputDataBit( COL_4_Port,COL_4_Pin);
  #define   SPEAKER_Pin                   GPIO_Pin_0
  #define   SPEAKER_GPIO_Port             GPIOB
  #define   LED_SYSTEM_Pin                GPIO_Pin_15
  #define   LED_SYSTEM_GPIO_Port          GPIOA

#endif
static void prvSetupHardware( void );
void getID(void);
void GPIO_Configuration(void);
void ADC_Config(void);
void vProcessData( void *pvParameters );
void DetectLossPower_Config(void);
#endif /* __MAIN_H */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */  
