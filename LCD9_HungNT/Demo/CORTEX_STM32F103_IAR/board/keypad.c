/**
  ******************************************************************************
  * Project		: PC9
  * File Name           : KEYPAD.c
  * Author 	        : Nguyen Tran Duy
  * Start	        : 15/12/2016
  * Stop                : 15/12/2017
  * Version		: 1.7
  ******************************************************************************
  * Description:
  ******************************************************************************
  */
#include "FreeRTOS.h"
#include "keypad.h"
#include "stm32f10x_it.h"
#include "main.h"
#include "stm32f10x_adc.h"

TickType_t      xTimeCheckKeypad;
bool            bDectectLossKeypad=FALSE;
u16 ADC_Read(ADC_TypeDef *ADCx,uint8_t ADC_Channel);
extern volatile QueueHandle_t            xPosRxErrorCodeQueue;
extern          volatile bool         bExitsErrcode;
volatile QueueHandle_t   xKeypadQueue;
extern  volatile        bool     bKeypadEnable;
extern volatile          bool bSinalPFL;
#if (KEYPAD_USE74HC==KP_NOTUSE)
  u8 ButtonPush(void)
  {
     ROW1_HIGH;
     ROW2_HIGH;
     ROW3_HIGH;
     ROW4_HIGH;
     ROW5_HIGH;
     ROW6_HIGH;
     if(GPIO_ReadInputDataBit( COL_1_Port,COL_1_Pin)==(u8)Bit_SET||GPIO_ReadInputDataBit( COL_2_Port,COL_2_Pin)==(u8)Bit_SET||GPIO_ReadInputDataBit( COL_3_Port,COL_3_Pin)==(u8)Bit_SET||GPIO_ReadInputDataBit( COL_4_Port,COL_4_Pin)==(u8)Bit_SET)
     { return 1;}
     return 0;
  }
  void set_row(u8 i)
  {
    
     ROW1_LOW;
     ROW2_LOW;
     ROW3_LOW;
     ROW4_LOW;
     ROW5_LOW;
     ROW6_LOW;
     if(i==0)
     {
        ROW1_HIGH;
     }
     else if(i==1)
     {
        ROW2_HIGH;
     }
     else if(i==2)
     {
        ROW3_HIGH;
     }
     else if(i==3)
     {
        ROW4_HIGH;
     }
     else if(i==4)
     {
     ROW5_HIGH;
     }
     else
       ROW6_HIGH;
  }
  char Get_key(void)
  {
     uint8_t i;
     if(ButtonPush())
     {
       for(i=0;i<6;i++)
       {
          set_row(i);
          if(GPIO_ReadInputDataBit( COL_1_Port,COL_1_Pin)) return KeyboardNew[0][5-i];
          if(GPIO_ReadInputDataBit( COL_2_Port,COL_2_Pin)) return KeyboardNew[1][5-i];
          if(GPIO_ReadInputDataBit( COL_3_Port,COL_3_Pin)) return KeyboardNew[2][5-i];
          if(GPIO_ReadInputDataBit( COL_4_Port,COL_4_Pin)) return KeyboardNew[3][5-i];
       }
     }
     return 0xFF;
  }

#else
  u8	KEYPAD_ReadRow(void)
  {
    u8 data=0;
    u8	i;
    u8 pin=(u8)Bit_RESET;
    KB_LATCH_LOW;
    KB_LATCH_HIGH;
     for(i=0;i<8;i++)
    {	
      data<<=1;
      pin=GPIO_ReadInputDataBit(KB_DATAIN_PORT,KB_DATAIN_PIN);
      if(pin==(u8)Bit_SET)
      {
          data|=0x01;
      }
     CLOCK_LOW;
     CLOCK_HIGH;
    }
    return data;
  }
  void KEYPAD_ColumnPullDown(u8 col)
  {
    u8 i;
    KB_LATCH_LOW;
    for(i=0;i<8;i++)
    {	  
      if((col & 0x80) == 0x80)
      {	
        DATA_HIGH;	
      }
      else
      {
        DATA_LOW;
      }
      col <<=1;
      CLOCK_LOW;
      CLOCK_HIGH;	
    }	
    KB_LATCH_HIGH;
   
  }
  void KEYPAD_ScanColumn(u8 c)
  {
    u8	col;
    switch(c)
    {	
        case 0:
                col=0xFE;
                break;
        case 1:
                col=0xFD;
                break;			
        case 2:
                col=0xFB;
                break;
        case 3:
                col=0xF7;
                break;
        case 4:
                col=0xEF;
                break;	
        case 5:
                col=0xDF;
                break;    
    }
    KEYPAD_ColumnPullDown(col);
  }
  
  char KEYPAD_Getkey(void)
  {
    u8 data=0;
    u8 i;
    for(i=0;i<6;i++)
    {
      KEYPAD_ScanColumn(i);
      data=KEYPAD_ReadRow();
      if((data&0xFF)==0x01) {return Keyboard[5-i][0];}
      if((data&0xFF)==0x02) {return Keyboard[5-i][1];}
      if((data&0xFF)==0x04) {return Keyboard[5-i][2];}
      if((data&0xFF)==0x08) {return Keyboard[5-i][3];}
    }
    return 0xFF;	
  }
#endif
void Keypad_Init(void)
{
  xKeypadQueue = xQueueCreate( 1, sizeof( xQueueMessage ) );
}

 void vKeypadPollTask( void *pvParameters )
{
  u16  uADCValue;  
  static char cLastState=0;
  char    cNowState; 
  xQueueMessage xMessage;
  DataSetup_t data;
  /* This tasks performs the button polling functionality as described at the
  top of this file. */
  for( ;; )
  {
    if((bKeypadEnable==TRUE) && (bSinalPFL==FALSE))
    {
      cNowState=KEYPAD_Getkey();
      /* Check the button state. */
      if( cNowState != cLastState )
      {
          /* The state has changed, send a message to the Keypad task. */
          cLastState = cNowState;      
          if(cNowState !=0xFF)
          {
            if(bExitsErrcode==FALSE ||(bExitsErrcode==TRUE &&(cNowState=='X' || cNowState=='C'||cNowState=='$')))
            {
              if((cNowState!=':')&&(cNowState!='/')&&(cNowState!=',')&&(cNowState!='-'))
              {
                TIMER3_ENABLE(1);               
              }
              xMessage.cMessageValue = cNowState;
              xQueueSend( xKeypadQueue, &xMessage, 0 ); 
            }
          }
      }  
    }
    if(bSinalPFL==FALSE)
    {
      uADCValue=ADC_Read(ADC1,1);      
      if((uADCValue<=ADC_Level_Vol_High) && (Addr_B7==1))
      {
        if(bDectectLossKeypad==FALSE)
        {
          xTimeCheckKeypad=xTaskGetTickCount();
          bDectectLossKeypad=TRUE;
        }
        /*Check keypad after 200ms*/
        else
        {
           if((xTaskGetTickCount()-xTimeCheckKeypad)>200)
            {
              data.code=Code_Keypad_Loss_Power;
              if(WaitTransmitDone(&data,TRUE)==TRUE)
              {         
                
              } 
            } 
        }
      }
      else if(bDectectLossKeypad==TRUE)bDectectLossKeypad=FALSE;
    }
  
    /* Block for 30 milliseconds so this task does not utilise all the CPU
    time and debouncing of the button is not necessary. */
    vTaskDelay( 30 );
  }
}
u16 ADC_Read(ADC_TypeDef *ADCx,uint8_t ADC_Channel)
{
  ADC_RegularChannelConfig(ADCx,ADC_Channel,1,ADC_SampleTime_55Cycles5);    // Cau hinh kenh chuyen doi Regular
  ADC_SoftwareStartConvCmd(ADCx, ENABLE);                                   // Bat dau qua trinh chuyen doi
  while(ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == RESET);                    // Cho qua trinh chuyen doi ket thuc
  ADC_ClearFlag(ADCx, ADC_FLAG_EOC);
  ADC_SoftwareStartConvCmd(ADCx, DISABLE);                                  // Khong cho phep chuyen doi
  return ADC_GetConversionValue(ADCx);                                      // Tra ve gia tri ADC 
}
 void TIMER3_Init(u32 Period)
{
   TIM_TimeBaseInitTypeDef     TIM_BaseStruct;
    NVIC_InitTypeDef            nvicStructure;

    /* Enable clock for TIM3 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    /*                     
    But, timer has internal PLL, which double this frequency for timer, up to 84MHz     
    Remember: Not each timer is connected to APB1, there are also timers connected     
    on APB2, which works at 84MHz by default, and internal PLL increase                 
    this to up to 168MHz                                                             
    
    Set timer prescaller 
    Timer count frequency is set with 
    
    timer_tick_frequency = Timer_default_frequency / (prescaller_set + 1)        
    
    In our case, we want a max frequency for timer, so we set prescaller to 0         
    And our timer will have tick frequency        
    
    timer_tick_frequency = 150000000 / (74 + 1) = 1000000
*/    
    TIM_BaseStruct.TIM_Prescaler = 8582;/*2048hz*/
    /* Count up */
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
/*
    Set timer period when it have reset
    First you have to know max value for timer
    In our case it is 16bit = 65535
    To get your frequency for PWM, equation is simple
    
    PWM_frequency = timer_tick_frequency / (TIM_Period + 1)
    
    If you know your PWM frequency you want to have timer period set correct
    
    TIM_Period = timer_tick_frequency / PWM_frequency - 1
    
    In our case, for 10Khz PWM_frequency, set Period to
    
    TIM_Period = 1000000 / 1000 - 1 = 999
    
    If you get TIM_Period larger than max timer value (in our case 65535),
    you have to choose larger prescaler and slow down timer tick frequency
*/
    TIM_BaseStruct.TIM_Period =Period ; /* 1ms period 50000*/
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
   // TIM_BaseStruct.TIM_RepetitionCounter = 0;
    /* Initialize TIM3 */
    TIM_TimeBaseInit(TIM3, &TIM_BaseStruct);
    /* Start count on TIM3 */
    TIM_Cmd(TIM3, DISABLE);    

    TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);

    /* Enable interrupt */
    nvicStructure.NVIC_IRQChannel = TIM3_IRQChannel;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
    nvicStructure.NVIC_IRQChannelSubPriority = 0;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void TIMER3_ENABLE(u32 Period)
{
  TIMER3_Init(Period);
  TIM_Cmd(TIM3, ENABLE);   
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}
void TIM3_IRQHandler( void )
{  
  static u16 cnt=0;
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
    cnt++;
    if(cnt==400)
    {
     cnt=0;
     Disble_TM3();
    }
    GPIO_TogglePin(SPEAKER_GPIO_Port,SPEAKER_Pin);
    TIM_SetCounter(TIM3, 0U);
  }
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
void Disble_TM3(void)
{
  TIM_Cmd(TIM3, DISABLE);   
  TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
}
void GPIO_TogglePin(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
  GPIOx->ODR ^=GPIO_Pin;
}