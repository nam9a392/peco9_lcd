/*
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
/* Library includes. */
#include "stm32f10x_it.h"
#include "stm32f10x_adc.h"
#include "keypad.h"
#include "main.h"

#define USE_FULL_ASSERT    1
/*
 * Configure the clocks, GPIO and other peripherals.
 */
u8 idLCD=0;
//SemaphoreHandle_t xMutex=NULL;
volatile bool    bSinalPFL=FALSE;
extern  volatile bool   bKeypadEnable;
extern volatile        TickType_t      xFirstPFLtime;
extern volatile        bool bExtPFL;
/*-----------------------------------------------------------*/
extern u8 uSegDigits[];

 int main( void )
{
  
  #ifdef DEBUG
    debug();
  #endif
  prvSetupHardware();  
  GPIO_Configuration();
  ADC_Config();
  DetectLossPower_Config();
  getID();
  POS_Init(); 
  Keypad_Init();  

  //xMutex=xSemaphoreCreateMutex();
  xTaskCreate(vPOS_Task, "CPUTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL );
  xTaskCreate(vKeypadPollTask,"KeypadTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);     
  xTaskCreate(vLcdTask,"LcdTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
  xTaskCreate(vProcessData,"DATATask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
  /* Start the scheduler. */
  vTaskStartScheduler();
  
  /* Will only get here if there was not enough heap space to create the
  idle task. */
  return 0;
}
static void prvSetupHardware( void )
{
  /* Start with the clocks in their expected state. */
  RCC_DeInit();

  /* Enable HSE (high speed external clock). */
  RCC_HSEConfig( RCC_HSE_ON );

  /* Wait till HSE is ready. */
  while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
  {
  }

  /* 2 wait states required on the flash. */
  *( ( unsigned long * ) 0x40022000 ) = 0x02;

  /* HCLK = SYSCLK */
  RCC_HCLKConfig( RCC_SYSCLK_Div1 );

  /* PCLK2 = HCLK */
  RCC_PCLK2Config( RCC_HCLK_Div1 );

  /* PCLK1 = HCLK/2 */
  RCC_PCLK1Config( RCC_HCLK_Div2 );

  /* PLLCLK = 8MHz * 9 = 72 MHz. */
  RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );

  /* Enable PLL. */
  RCC_PLLCmd( ENABLE );

  /* Wait till PLL is ready. */
  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
  {
  }

  /* Select PLL as system clock source. */
  RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

  /* Wait till PLL is used as system clock source. */
  while( RCC_GetSYSCLKSource() != 0x08 )
  {
  }


  /* Set the Vector Table base address at 0x08000000 */
  NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

  NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

  /* Configure HCLK clock as SysTick clock source. */
  SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

void getID(void)
{
  uint8_t addr=0;
#if (VERSION==VERSION_4)
  addr=(Addr_B2)|(Addr_B0<<1);
  idLCD=0x44-addr;
#endif   
}
void ADC_Config(void)
{  
    ADC_InitTypeDef  ADC_InitStructure;
 
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);    // ADCCLKmax = 14MHZ --> ADCCLK = PCLK/6 = 72/6 = 12MHz
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
 
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;    // Cau hinh che do hoat dong cho bo ADC(Single)
                                                          // ADC_Mode_Independent             
                                                          // ADC_Mode_RegInjecSimult                 
                                                          // ADC_Mode_RegSimult_AlterTrig           
                                                          // ADC_Mode_InjecSimult_FastInterl       
                                                          // ADC_Mode_InjecSimult_SlowInterl         
                                                          // ADC_Mode_InjecSimult                 
                                                          // ADC_Mode_RegSimult                     
                                                          // ADC_Mode_FastInterl                     
                                                          // ADC_Mode_SlowInterl                     
                                                          //ADC_Mode_AlterTrig                     
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;        // ENABLE - DISABLE che do Scan (multichannels)
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // ENABLE - DISABLE che do Continuous
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;    // Dinh nghia Trigger ben ngoai de bat dau qua trinh chuyen doi tren kenh Regular
                                                                          // ADC_ExternalTrigConv_T1_CC1
                                                                          // ADC_ExternalTrigConv_T1_CC2
                                                                          // ADC_ExternalTrigConv_T2_CC2
                                                                          // ADC_ExternalTrigConv_T3_TRGO
                                                                          // ADC_ExternalTrigConv_T4_CC4
                                                                          // ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO
                                                                          // ADC_ExternalTrigConv_T1_CC3               
                                                                          // ADC_ExternalTrigConv_None                                                               
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;    // Cau hinh kieu luu tru du lieu
                                                              // ADC_DataAlign_Right
                                                              // ADC_DataAlign_Left
    ADC_InitStructure.ADC_NbrOfChannel = 1;    // Cau hinh so kenh ADC regular su dung
                                              // 1 - 16
    ADC_Init(ADC1, &ADC_InitStructure);    // Cau hinh ADC1
    ADC_Cmd(ADC1, ENABLE);                // Kich hoat ADC1
 
    ADC_ResetCalibration(ADC1);                    // Reset thanh ghi hieu chinh ADC1
    while(ADC_GetResetCalibrationStatus(ADC1));    // Cho qua trinh Reset ket thuc
    ADC_StartCalibration(ADC1);                    // Bat dau hieu chinh ADC1
    while(ADC_GetCalibrationStatus(ADC1));
}
void GPIO_Configuration(void)
{
  
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
  RCC->APB2ENR |=(1 <<  2)|(1 << 0)|(1 << 3);        /* Enable PortA, PortB, Enable AFIO  *///(1 <<  2)|
//  AFIO->MAPR &= 0xF8FFFFFF;                            /* reset SWJ_CFG     */
//  AFIO->MAPR |= 0x02000000;                            /* enable sw, disable jtag   */
  Enable_SWD(AFIO);
  /*Version using 74HC*/
#if (KEYPAD_USE74HC==KP_USE)
  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(GPIOA, LED_SYSTEM_Pin|LCD_EN_Pin|LCD_RS_Pin|LCD_Enable_Pin 
                          |LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin,Bit_RESET);//|LCD_RW_Pin
  GPIO_WriteBit(GPIOC,KB_LATCH_PIN| KB_DATAOUT_PIN |KB_CLOCK_PIN,Bit_RESET);//|SPEAKER_Pin|LED_SYSTEM_Pin
  GPIO_WriteBit(GPIOB,SPEAKER_Pin| LAPIS_TEST_Pin|LAPIS_BLANK_Pin|LAPIS_LATCH_Pin|LAPIS_CLK_Pin|LAPIS_DATA_Pin,Bit_RESET);
  //GPIO_WriteBit(KB_LATCH_PORT,KB_LATCH_PIN,Bit_RESET);
  
   /*Configure Pin PA1 as ADC Pin*/
  GPIO_InitStructure.GPIO_Pin=Keypad_Loss_PW_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(Keypad_Loss_PW_Port, &GPIO_InitStructure); 
  /*Configure GPIO pins : SW1_Pin SW2_Pin SW3_Pin SW4_Pin */
  GPIO_InitStructure.GPIO_Pin = SW1_Pin|SW2_Pin|SW3_Pin|SW4_Pin|SW5_Pin|SW6_Pin|SW7_Pin|SW8_Pin ; 
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
 /*Configure GPIO pins : LCD_EN_Pin LCD_RW_Pin LCD_RS_Pin LCD_Enable_Pin 
                           LCD_D4_Pin LCD_D5_Pin LCD_D6_Pin LCD_D7_Pin */
  GPIO_InitStructure.GPIO_Pin = LED_SYSTEM_Pin|LCD_EN_Pin|LCD_RS_Pin|LCD_Enable_Pin //
                          |LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
   /*Configure GPIO pins : LAPIS_TEST_Pin LAPIS_BLANK_Pin LAPIS_LATCH_Pin */
  GPIO_InitStructure.GPIO_Pin=SPEAKER_Pin|LAPIS_TEST_Pin|LAPIS_BLANK_Pin|LAPIS_LATCH_Pin|LAPIS_CLK_Pin|LAPIS_DATA_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LAPIS_DATA_GPIO_Port, &GPIO_InitStructure);
   /*Configure GPIO pins : KEYPAD_DOUT_Pin KEYPAD_CLK_Pin KEYPAD_LATCH_Pin */
  GPIO_InitStructure.GPIO_Pin=KB_DATAOUT_PIN|KB_CLOCK_PIN|KB_LATCH_PIN;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(KB_DATAOUT_PORT, &GPIO_InitStructure);
  /*Configure GPIO pins : KEYPAD_DIN_Pin PFL_Pin */
  GPIO_InitStructure.GPIO_Pin=KB_DATAIN_PIN;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(KB_DATAIN_PORT, &GPIO_InitStructure);
 
  /*Version not use 74HC*/
#else 
  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(GPIOA, LCD_EN_Pin|LCD_RS_Pin|LCD_Enable_Pin 
                          |LED_SYSTEM_Pin|LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin|LCD_RW_Pin,Bit_RESET);//|LCD_RW_Pin
  GPIO_WriteBit(GPIOC, SW5_Pin|SW6_Pin|SW7_Pin|SW8_Pin 
                          ,Bit_RESET);
  GPIO_WriteBit(GPIOB,SPEAKER_Pin| LAPIS_TEST_Pin|LAPIS_BLANK_Pin|LAPIS_LATCH_Pin|LAPIS_CLK_Pin|LAPIS_DATA_Pin,Bit_RESET);  
 /*Configure GPIO pins : SW1_Pin SW2_Pin SW3_Pin SW4_Pin */
  GPIO_InitStructure.GPIO_Pin = SW1_Pin|SW2_Pin|SW3_Pin|SW4_Pin|SW5_Pin|SW6_Pin|SW7_Pin|SW8_Pin ; 
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
 /*Configure GPIO pins : LCD_EN_Pin LCD_RW_Pin LCD_RS_Pin LCD_Enable_Pin 
                           LCD_D4_Pin LCD_D5_Pin LCD_D6_Pin LCD_D7_Pin */
  GPIO_InitStructure.GPIO_Pin = LCD_EN_Pin|LCD_RW_Pin|LCD_RS_Pin|LCD_Enable_Pin //
                          |LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
 /*Configure GPIO pins : LAPIS_TEST_Pin LAPIS_BLANK_Pin LAPIS_LATCH_Pin */
  GPIO_InitStructure.GPIO_Pin=LAPIS_TEST_Pin|LAPIS_BLANK_Pin|LAPIS_LATCH_Pin|LAPIS_CLK_Pin|LAPIS_DATA_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin=ROW_1_Pin|ROW_2_Pin|ROW_3_Pin|ROW_4_Pin 
                          |ROW_5_Pin|ROW_6_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = COL_1_Pin|COL_2_Pin|COL_3_Pin|COL_4_Pin; 
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin=SPEAKER_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(SPEAKER_GPIO_Port, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin=LED_SYSTEM_Pin;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LED_SYSTEM_GPIO_Port, &GPIO_InitStructure); 
#endif 
}
void EXTI9_5_IRQHandler( void )
{
/* Define the message sent to the LCD task from this interrupt. */
//long lHigherPriorityTaskWoken = pdFALSE;
if(EXTI_GetITStatus(PFL_EXTI_LINE) != RESET)
{
  bKeypadEnable=FALSE;
  //bSinalPFL=TRUE;
  bExtPFL=TRUE;
  xFirstPFLtime=xTaskGetTickCountFromISR();
  EXTI_ClearITPendingBit( PFL_EXTI_LINE );
}
    //EXTI_ClearITPendingBit( PFL_EXTI_LINE );
//portEND_SWITCHING_ISR( lHigherPriorityTaskWoken );
}
void DetectLossPower_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;     
    RCC_APB2PeriphClockCmd(PFL_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(PFL_AFIO_CLK, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;
    GPIO_InitStructure.GPIO_Pin = PFL_PIN; 
    GPIO_Init(PFL_GPIO_PORT, &GPIO_InitStructure); 
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
        
    NVIC_InitStructure.NVIC_IRQChannel =EXTI9_5_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  }
#ifdef  DEBUG
/* Keep the linker happy. */
void assert_failed( unsigned char* pcFile, unsigned long ulLine )
{
  for( ;; )
  {
  }
}
#endif

