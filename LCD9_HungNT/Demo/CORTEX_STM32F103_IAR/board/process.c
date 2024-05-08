/**
  ******************************************************************************
  * Project		: PC9
  * File Name           : Proccess.c
  * Author 	        : Nguyen Tran Duy
  * Start	        : 15/12/2016
  * Stop                : xx/12/2017
  * Version		: 1.8
  ******************************************************************************
  * Description:
  *             Update version 1.8: 22/5/2021
  *             Users can press button when the pump is fueling
  ******************************************************************************
  */
#include "lapis.h"
#include "main.h"
#include "TextLcd.h"
#include "math.h"
#include "semphr.h"

SysStatus_t     sProcessStatus;
Errorcode_t     sErrorCode;
bool            bDetectErrContinuos=FALSE;
bool            bEnterDisable=FALSE;
//StatusLCD_Typedef       StatusLCD={FALSE,FALSE,FALSE,FALSE};
bool            bStartTimerDisplayTest=FALSE;
volatile bool   bErrcode=FALSE;
volatile        TickType_t      xFirstPFLtime;
volatile        bool bExtPFL=FALSE;
volatile bool                  bPowerOn_NzzlHang=FALSE;
//extern volatile bool                  bPowerOn;
//extern volatile EventGroupHandle_t       xLCDBitWaitSendDone;
//extern          volatile EventGroupHandle_t     xPosHangEventGroup;
//extern SemaphoreHandle_t xMutex;
extern          u8	                        uSegDigits[18];
extern          volatile  bool                  bSelectCode;
extern          volatile  bool                  bBlinkControl;
extern          volatile QueueHandle_t          xPosRxSysStatusQueue;
extern          volatile QueueHandle_t          xPosRxErrorCodeQueue;
extern          volatile eLoginMode_t           Mode;
extern          volatile SysConfig_t            sConfiguration;
extern          volatile bool                   bKeypadEnable;
extern          volatile bool                   bLogin;
extern          volatile EnablePreset_t         sEnablePreset;
extern          volatile u32                    uPresetNum;
extern          volatile bool                   bPrinterEnable;
extern          volatile bool                   bLowcase;
extern          volatile bool                   bMoveCursor;
extern          volatile u8                     uDataLeng;
extern          volatile u8                     uProcessCodeLeng;
extern          volatile bool                   bClearKeyX;  
extern          volatile bool                   bFlagValidEnterCode;
extern          volatile bool                   bChangecode_SunnyPeco;
extern          volatile bool                   bSinalPFL;
//extern          volatile        bool            bSelectP;
extern          volatile eTypePressX_t          eTypeRead_Select;
extern          volatile bool         bExitsErrcode;
extern          volatile              EventGroupHandle_t     xPosReceiDataEventGroup;
extern          volatile              QueueHandle_t          xPosSendDataQueue;
bool            bStartTimerError=FALSE;
/*Index for Display test*/
u8  uNumberDisplayTest=0; 
//static uint8_t data_temp=0;
volatile TimerHandle_t xTimers[5];
void vProcessData( void *pvParameters );
void xTimers_Init(u8 i,u16 period,TimerCallbackFunction_t callback)
{
  xTimers[i]=xTimerCreate
   ( /* Just a text name, not used by the RTOS
     kernel. */
     "Timer",
     /* The timer period in ticks, must be
     greater than 0. */
     period,
     /* The timers will auto-reload themselves
     when they expire. */
     pdTRUE,
     /* The ID is used to store a count of the
     number of times the timer has expired, which
     is initialised to 0. */
     ( void * ) 0,
     /* Each timer calls the same callback when
     it expires. */
     callback
     //vTimerCallback
   );
}
void vTimerCallback_BlinkErr( TimerHandle_t xTimer )
 {
   bErrcode=(bool)!bErrcode;
 }
void vTimerCallback_Displaytest( TimerHandle_t xTimer )
 {
   if(uNumberDisplayTest==11)uNumberDisplayTest=0;
   LAPIS_DisplayTest(uNumberDisplayTest);
   uNumberDisplayTest++;
 }
 void vTimerCallback_BlinkLedSys( TimerHandle_t xTimer )
{
  GPIO_TogglePin(LED_SYSTEM_GPIO_Port,LED_SYSTEM_Pin);
  if(Addr_B7==0)
  {
    bKeypadEnable=FALSE;
  }
  else
  {
     if((bKeypadEnable==FALSE) && (Mode==SUNNYXE_FUELING))bKeypadEnable=TRUE;
  }  
}
void Disable_SWD(AFIO_TypeDef *AFIO)
{
  AFIO->MAPR &= 0xF8FFFFFF;  
  AFIO->MAPR |= 0x04000000;
}
void Enable_SWD(AFIO_TypeDef *AFIO)
{
  AFIO->MAPR &= 0xF8FFFFFF;  
  AFIO->MAPR |= 0x02000000;
}
//volatile   u32    ubuffvalue=0; 
//volatile  bool  bNZZL_TAKE_OFF=FALSE;
//volatile  bool  bNZZL_REFUELING=FALSE;
void vProcessData( void *pvParameters )
{ 
  bool  bNZZL_TAKE_OFF=FALSE;
  bool  bNZZL_REFUELING=FALSE;
 // bool  bBitWaitPresetDone=FALSE;
   u32    ubuffvalue=0;
  eFuelingMode_t  eFuelMode=NORMAL;
  //xSemaphore=xSemaphoreCreateMutex();
  xTimers_Init(0,800,vTimerCallback_BlinkErr);
  xTimers_Init(1,500,vTimerCallback_Displaytest);
  xTimers_Init(2,500,vTimerCallback_Printer);
  xTimers_Init(3,800,vTimerCallback_ClearCode);
  xTimers_Init(4,1000,vTimerCallback_BlinkLedSys);

  while(1)
  {   
      if(bExtPFL==TRUE)
      {
        if(xTaskGetTickCount()-xFirstPFLtime>1000)
        {
          if(GPIO_ReadInputDataBit(PFL_GPIO_PORT,PFL_PIN)==(u8)Bit_RESET)
          {
            bSinalPFL=TRUE;
            bExtPFL=FALSE;
          }
        }
      }
      if( xQueueReceive( xPosRxSysStatusQueue, &sProcessStatus , ( TickType_t ) 5 ) && (bSinalPFL==FALSE))
      {       
         if(sConfiguration.uDisplay_LastData==0)
          sConfiguration.UnitPrice=sProcessStatus.uPrice;
         switch(sProcessStatus.NZZLStatus)
         {
            case NZZL_HANG:         
            case NZZL_REFUELING_END:   
                Mode=SUNNYXE_PRESET;   
               /* Stop software timer 1 if the mode displayTest is used*/
                if(bStartTimerDisplayTest==TRUE)
                {
                  xTimerStop( xTimers[1], 0 ); 
                  bStartTimerDisplayTest=FALSE;
                }                 
                /*set mode fuel to Normal*/
               if(eFuelMode!=NORMAL)eFuelMode=NORMAL;                      
                /*Reset takeoff var*/
                if(bNZZL_TAKE_OFF==TRUE ||(bNZZL_REFUELING==TRUE))
                {
                  LCD_Default();  
                  bNZZL_TAKE_OFF=FALSE;
                  bNZZL_REFUELING=FALSE;
                  Clear_Error();
                  /*Reset variables*/
                  ubuffvalue=0;
                  Reset_SomeValues();                                   
                } 
                if(sConfiguration.uDisplay_Err==1)
                {
                  sConfiguration.UAV.uAmountLossPower=0;
                  sConfiguration.UAV.uVolumeLossPower=0;
                  sConfiguration.uDisplay_LastData=0;
                  sConfiguration.uDisplay_Err=0;
                }
                
                bPowerOn_NzzlHang=TRUE;
                break;            
            case  NZZL_TAKE_OFF:              
                /*Disable KEYPAD and speaker*/
                bKeypadEnable=FALSE;
                //bEnableLCDStatusDisplay=FALSE;
                /*if printer mode was set then blink control is OFF*/
                if(bBlinkControl==TRUE)
                {
                  LCD_BlinkOnOff(0x0c); 
                  bBlinkControl=FALSE;
                }      
                bNZZL_TAKE_OFF=TRUE;
                if(( (bNZZL_REFUELING==FALSE)&&(sProcessStatus.errCode.Code[0]=='0')&&(sProcessStatus.errCode.Code[1]=='0')))//(bNZZL_TAKE_OFF==FALSE||
                { 
                  if(bPowerOn_NzzlHang==TRUE)
                  {
                    if((sEnablePreset.bNumber==TRUE)&&(sEnablePreset.bP1_4==TRUE))
                    {
                      LCD_Default();
                    }   
                    bPowerOn_NzzlHang=FALSE;
                    LAPIS_Clear();               
                    TEST_PIN_HIGH;
                    vTaskDelay(500);
                    TEST_PIN_LOW;        
                    reset_AmountVolume(&sProcessStatus);
                  }                   
 
                  sEnablePreset.bNzzlHang=FALSE;              
                }
//                /*check volume from POS*/
                if(sProcessStatus.uPosValue==1)
                {
                  if(ubuffvalue!=sProcessStatus.uPosVolume)
                  {
                    TextLcd_Display(FALSE,sProcessStatus.uPosVolume,sConfiguration.DecimalPlace.Volume,1,TRUE);
                    ubuffvalue=sProcessStatus.uPosVolume;
                  }                    
                }
                /*check Amount from POS*/
                else if(sProcessStatus.uPosValue==2)
                {                   
                  if(ubuffvalue!=sProcessStatus.uPosAmount)
                  {
                    TextLcd_Display(TRUE,sProcessStatus.uPosAmount,sConfiguration.DecimalPlace.Amount,2,TRUE);
                    ubuffvalue=sProcessStatus.uPosAmount;
                  }                  
                } 
                /*check mode display test */
                if(sConfiguration.DisplayTest==1)
                {
                  if(bStartTimerDisplayTest==FALSE)
                  {
                    if( xTimerStart( xTimers[1], 0 )==pdPASS)
                    {
                      bStartTimerDisplayTest=TRUE;
                    }
                  }
                }
                /*Check flowrate mode*/
                if(sConfiguration.FuelingMode==FLOW_RATE_DISPLAY)
                {
                  eFuelMode=FLOW_RATE_DISPLAY;
                } 
                /*if power loss*/
                if(sConfiguration.uDisplay_LastData==1)
                {
                  /*if exits error while nzzl is refueling before Power loss*/                  
                  if(sConfiguration.uDisplay_Err==1)
                  {
                    sProcessStatus.errCode.Code[0]=0x36;
                    sProcessStatus.errCode.Code[1]=0x30;
                  }
                }
                /*Reset Amount - Volume*/
              //  reset_AmountVolume(&sProcessStatus);
                LAPIS_DetectError(); 
                break;             
            case NZZL_REFUELING: 
            case LCD_DETECTED_ERROR:
                Mode=SUNNYXE_FUELING;                 
                bNZZL_REFUELING=TRUE;
                bPowerOn_NzzlHang=FALSE;
               if(sProcessStatus.NZZLStatus==NZZL_REFUELING)
               {
                  bPrinterEnable=TRUE;
               }              
               LAPIS_DetectError();
               break;
         }
         if(bStartTimerDisplayTest==FALSE)
         {
           LAPIS_DISPLAY(&sProcessStatus,eFuelMode);       
         }         
      }
  }
}

void Clear_Error(void)
{
  if(bStartTimerError==TRUE)
  {
     xTimerStop( xTimers[0], 0 );
     bStartTimerError=FALSE;
     bErrcode=FALSE;                    
  }
}
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
void reset_AmountVolume(SysStatus_t *sProcess)
{
  if(sConfiguration.uDisplay_LastData==0)
  {
    sProcess->uLeng[1]=1;
    sProcess->uLeng[2]=1;
    sProcess->uFlowrate=0;      
    memset(sProcess->uArray_VL,0,7);
    memset(sProcess->uArray_AM,0,7);  
  }  
}
void LAPIS_DetectError(void)
{
  if(sProcessStatus.errCode.Code[0]!='0' &&(sProcessStatus.errCode.Code[0]!='F')&&(sProcessStatus.errCode.Code[1]!='F'))
  {
    if(bStartTimerDisplayTest==TRUE)
    {
      xTimerStop( xTimers[1], 0 );   
      bStartTimerDisplayTest=FALSE;
    }  
    if(bStartTimerError==FALSE)
    {
      if( xTimerStart( xTimers[0], 0 ) == pdPASS )
      {
        bStartTimerError=TRUE;
      }        
    }
  }
  else
  {
      Clear_Error();
  }
 
}
void Reset_SomeValues(void)
{
//  ubuffvalue=0;
  uProcessCodeLeng=0;
  uPresetNum=0;  
  uNumberDisplayTest=0;   
  bLowcase=FALSE;     
  bMoveCursor=FALSE;
  bClearKeyX=FALSE;       
  bChangecode_SunnyPeco=FALSE;
  bFlagValidEnterCode=FALSE;
  bSelectCode=TRUE;
  sEnablePreset.bNumber=TRUE;
  sEnablePreset.bP1_4=TRUE; 
  sEnablePreset.bNzzlHang=TRUE;                                     
  eTypeRead_Select=READ;  
  Reset_Buffer_Keypad(); 
  if(Addr_B7==1)bKeypadEnable=TRUE; 
}

uint8_t GetNumberAfterDotVolume(SysStatus_t *sys)
{
  uint16_t buff;
  uint8_t result;
  buff=sys->uVolume%1000;
  result=buff/100;
  return result;
}
