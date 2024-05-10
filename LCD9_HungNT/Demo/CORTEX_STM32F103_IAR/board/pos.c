/**
  ******************************************************************************
  * Project		: PC9
  * File Name           : POS.c
  * Author 	        : Nguyen Tran Duy
  * Start	        : 15/12/2016
  * Stop                : xx/12/2017
  * Version		: 1.8
  ******************************************************************************
  * Description:        Fixed check code 10, code 2
  *
  *             Update version 1.8: 22/5/2021
  *             Users can press button when the pump is fueling
  ******************************************************************************
  */
/**
28/06/2023   HungNT   Fixed auto clear amount, volume after conn timeout
                      Fixed Showing error d1
                      Removed TaskSuspen() after conn timeout
18/12/2023   HungNT   Increase counter of connection loss check

*/
#include "stm32f10x_it.h"
#include "lapis.h"
#include "math.h"
#include "TextLcd.h"
#include "semphr.h"
/** 
  * @{
  */ 

#if (POS_USED == STD_ON)
u8 LcdCode = 0; 
volatile float          fVolumeLimit=0;
QueueHandle_t            xPosRxMsgQueue;
QueueHandle_t            xPosTxMsgQueue;
SysStatus_t             sysStatus;
//add semaphore
volatile SemaphoreHandle_t xSemaphore = NULL;


EventGroupHandle_t                xPosRxEventGroup;
volatile QueueHandle_t            xPrinterMsgQueue;
volatile QueueHandle_t            xPosRxSysStatusQueue;
volatile QueueHandle_t            xPosRxErrorCodeQueue;
volatile QueueHandle_t            xPosSendDataQueue;
//volatile EventGroupHandle_t       xPosHangEventGroup;
volatile EventGroupHandle_t       xPosReceiDataEventGroup;
volatile bool         pos_IsRxDataDetected = FALSE;
volatile bool         pos_IsSendFail = FALSE;
volatile bool         pos_IsTxMode = FALSE;
volatile bool         bExitsErrcode=FALSE;
bool                  pos_StartFeedData = FALSE;

Errorcode_t           errcode;
extern          volatile EnablePreset_t         sEnablePreset;
extern          volatile TimerHandle_t xTimers[5];
extern          volatile SysConfig_t     sConfiguration;
extern          volatile bool           bKeypadEnable;
extern          volatile eLoginMode_t   Mode;
extern          volatile TypeValue_t    sTypeValues;
extern          volatile BOOLEAN        PRESET_TypeFlag[4];
extern          volatile u8             uPresetValue;
extern          volatile u8 uPrinterID;
FrameLogs_t           sFrameLogs;
/**
  * @brief  Init POS GPIO
  * @param  none
  * @retval None
  */

/**
  * @brief  Enable to transmit data over rs485
  * @param  none
  * @retval None
  */
void POS_EnableTransmit(void)
{
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
}
/**
  * @brief  Disable to transmit data over rs485
  * @param  none
  * @retval None
  */
void POS_DisableTransmit(void)
{
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
}
void Send_Error_d1(SysStatus_t* sys,u8 code_chuc,u8 code_dv);

volatile bool pos_WaitForNewMsg = FALSE;

extern u8 uSegDigits[];
extern u8 uSegErrorList[];

#define SEGCODE_DASH 0x40
#define SEGCODE_UNDERLINE 0x09

void vPOS_Task( void *pvParameters )
{
  STATUS_Typedef    STATUS=POWER_ON;  
  eReturnStatus     eStatus;
  uint8_t           conn_lost_cnt = 0;
  uint8_t ver_a, ver_b;
  
  TM_TIMER2_Init();   
  USART2_Init();
  sysStatus.uVolume=0;
  
#if 1
#if 0
  //code for check all segment codes
  for(uint8_t code = 0; code < 255; code++)
  {
    LAPIS_DisplayNumber(code);
    LATCH();
  }
#endif  
  //show version  
  ver_a = LCD_Version/10;
  ver_b = LCD_Version%10;  
  LAPIS_Clear();  
  {
    
    LAPIS_DisplayNumber(uSegDigits[ver_b]);
    LAPIS_DisplayNumber(uSegDigits[ver_a]);
    LAPIS_DisplayNumber(SEGCODE_DASH);
    LAPIS_DisplayNumber(uSegDigits[17]);//d
    LAPIS_DisplayNumber(uSegDigits[13]);//C
    LAPIS_DisplayNumber(uSegDigits[12]);//L
    
  }
  LATCH(); //output data
  vTaskDelay(1000); 
  LAPIS_Clear();
 
#endif
  
  while(1)
  {
#if 1
    if ( STATUS==POWER_ON)
    { 
      if((eStatus= POS_RegistrationToCpu()) == pTRUE)
      {
        STATUS = RUNNING;
        conn_lost_cnt = 0;
      }
    }
    else if( STATUS==RUNNING)
    {
      if( (eStatus=POS_FuelingMode())==pTRUE)
      {
        conn_lost_cnt = 0;
      }
      /* COM 5s*/
      else if(eStatus==pTIMEOUT)
      {
        if(conn_lost_cnt++ >= 5){
          Send_Error_d1(&sysStatus,'d','1');
          conn_lost_cnt = 5;
        }
      }
    }
#else
    if(show_error_flag==0){
      Send_Error_d1(&sysStatus,'d','1');
      //show_error_flag = 1;
    }
    vTaskDelay(10);
#endif
  }/*end of while(1)*/
}

void Send_Error_d1(SysStatus_t* sys,u8 code_chuc,u8 code_dv)
{
#if 1
    bKeypadEnable=FALSE;
    bExitsErrcode=TRUE;
    
    sys->uLeng[0]= LengthOfInt(sys->uPrice);    
    sys->uLeng[1]=LengthOfInt(sys->uVolume);
    sys->uLeng[2]=LengthOfInt(sys->uAmount);
    sys->uPosValue=0;
    Split_Digit(sys->uPrice,sys->uArray_UP,5);    
    Split_Digit(sys->uVolume,sys->uArray_VL,7);
    Split_Digit(sys->uAmount,sys->uArray_AM,7);
    sys->NZZLStatus=LCD_DETECTED_ERROR; 
    sys->errCode.Code[0]=code_chuc;
    sys->errCode.Code[1]=code_dv;
    xQueueSend(xPosRxSysStatusQueue, sys,100); 
#else 
    sys->errCode.Code[0]=code_chuc;
    sys->errCode.Code[1]=code_dv;
#endif
     //vTaskSuspend(NULL);
    //xTimerStart( xTimers[0], 100 );
}

void Err_D1(void)
{
    SysStatus_t           sysStatus;
    bKeypadEnable=FALSE;
    bExitsErrcode=TRUE;
    QueueSend_Err(&sysStatus,'d','1');
    vTaskSuspend(NULL);
}
void POS_LitmitSetup(u32 up,volatile float *volume)
{
  u32 amount=9999999;
 // if(PosMode==2||PosMode==3)  
  {
    if(up>0)
    *volume=(float)amount/up;  
    else
     *volume=9999.999; 
  }
}
void POS_UpdateConfig(u8* ptr,volatile SysConfig_t     *config)
{
  u8 i;
  DataFrame_t    dtFrame;  
  taskENTER_CRITICAL();
    memcpy(&dtFrame,ptr,sizeof(dtFrame));
  taskEXIT_CRITICAL();
  config->UnitPrice=(UnitPrice_t)stringToInt( dtFrame.uPrice,5);
  config->UserPassword=stringToInt( dtFrame.UserPassword,4);
  config->PosMode=(ePosMode_t)stringToInt( dtFrame.PosMode,1);
  config->CommID=stringToInt( dtFrame.CommID,2);
  config->FuelType=(eFuelTypeCode_t)stringToInt( dtFrame.FuelType,1);
  config->KeypadSetting.Condition=(eKeypadCondition_t)stringToInt( dtFrame.KeypadSetting.uCondition,1);
  config->CoPassword=stringToInt( dtFrame.CoPassword,8);
  config->CountryCode=(eCountryCode_t)stringToInt( dtFrame.CountryCode,3);
  config->FirstIndication=stringToInt( dtFrame.FirstIndication,2);
  config->PresetOverRunMasking=stringToInt( dtFrame.PresetOverRunMasking,1);
  config->AmountPresetMethod=stringToInt( dtFrame.AmountPresetMethod,1);
  config->Totalizer.volume= stringToInt( dtFrame.fTotal.uVolume,10);
  config->Totalizer.amount=stringToInt( dtFrame.fTotal.uAmount,10);
  config->DailyTotal.volume=stringToInt( dtFrame.fDailyTotal.uVolume,10);
  config->DailyTotal.amount=stringToInt( dtFrame.fDailyTotal.uAmount,10);  
  config->FuelingLimit=stringToInt( dtFrame.FuelingLimit,7);   
  config->DecimalPlace.Amount = stringToInt(dtFrame.DecimalPlace.uAmount,1);
   config->DecimalPlace.Volume = stringToInt(dtFrame.DecimalPlace.uVolume,1);
   config->DecimalPlace.UnitPrice = stringToInt(dtFrame.DecimalPlace.uPrice,1); 
  for(i=0;i<4;i++)
  {
    if(stringToInt( dtFrame.KeypadSetting.P[i].uType,1)==0)
    {
       PRESET_TypeFlag[i]=bFALSE;
       uPresetValue=1;
       sTypeValues.len_tp[i+2]=config->DecimalPlace.Volume;
       config->KeypadSetting.OneTouch.PV[i]=stringToInt( dtFrame.KeypadSetting.P[i].uValue,7);
    }
    else if(stringToInt( dtFrame.KeypadSetting.P[i].uType,1)==1)
    {
        PRESET_TypeFlag[i]=bTRUE;  
        uPresetValue=0; 
        config->KeypadSetting.OneTouch.PA[i]=stringToInt( dtFrame.KeypadSetting.P[i].uValue,7);
    }
  }  
  for(i=0;i<6;i++)
  {
    config->PresetSlowdownPosition.F[i]=stringToInt( dtFrame.PresetSlowdownPosition[i].uValue,2);
  }
  config->SlowdownTimeForPulseStop=stringToInt( dtFrame.SlowdownTimeForPulseStop,2);
  config->PumpLockTimeAfterUPriceChange=stringToInt( dtFrame.PumpLockTimeAfterUPriceChange,1);
  config->PumpLockTimeForPulseStop=stringToInt( dtFrame.PumpLockTimeForPulseStop,1);
  config->FuelingMode=(eFuelingMode_t)stringToInt( dtFrame.FuelingMode,1);
  config->CommTimeout=stringToInt( dtFrame.CommTimeout,2);
  config->FuelingCount=stringToInt( dtFrame.FuelingCount,10);
  config->PowerOnOffCount=stringToInt( dtFrame.PowerOnOffCount,6);
  config->DisplayTest=stringToInt( dtFrame.DisplayTest,1);
  config->MaintenancePassword=stringToInt( dtFrame.MaintenancePassword,8);
  config->Calendar.year=stringToInt( dtFrame.Calendar.uYear,4);
  config->Calendar.month=stringToInt( dtFrame.Calendar.uMonth,2);
  config->Calendar.date=stringToInt( dtFrame.Calendar.uDate,2);
  config->Calendar.hour=stringToInt( dtFrame.Calendar.uHour,2);
  config->Calendar.minutes=stringToInt( dtFrame.Calendar.uMin,2);
  config->Version.versionCpu=stringToInt( dtFrame.bVersion,2);
  config->Version.versionLcd=LCD_Version;
   

  sTypeValues.len_tp[0]=config->DecimalPlace.Volume;//config->DecimalPlace.Volume;
  sTypeValues.len_tp[1]=config->DecimalPlace.Volume;//config->DecimalPlace.Volume; 
  
  config->PosVersion= stringToInt(dtFrame.PosVersion,1);
  config->UAV.uUPLossPower=stringToInt(dtFrame.UAV.uUPLossPower,6);
  config->UAV.uVolumeLossPower=stringToInt(dtFrame.UAV.uVolumeLossPower,7);
  config->UAV.uAmountLossPower=stringToInt(dtFrame.UAV.uAmountLossPower,7);
  config->uDisplay_Err=stringToInt(dtFrame.uDisplay_Err,1);
  config->uDisplay_LastData=stringToInt(dtFrame.uDisplay_LastData,1);
}

void POS_UpdateSysStatusData(u8* ptr,SysStatus_t* status)
{
  /*Get Flowrate, error code*/
  status->uFlowrate=stringToInt(ptr+20,2);
  status->errCode.Code[0]=*(ptr+22);
  status->errCode.Code[1]=*(ptr+23); 

  status->NZZLStatus=(eNozzleStatus_t)(*ptr -0x30);
  /*If no Loss power*/
  if(sConfiguration.uDisplay_Err==0||(status->NZZLStatus==NZZL_REFUELING))
  {
     status->uPrice=stringToInt(ptr+8,5);
     status->uPosValue=stringToInt(ptr+24,1); 
    if(status->uPosValue==0) 
    {  
      status->uVolume=stringToInt(ptr+1,7);
      status->uAmount=stringToInt(ptr+13,7);
    }
    else if(status->uPosValue==1)
    {
      status->uPosVolume=stringToInt(ptr+1,7); 
      status->uVolume=0;
      status->uAmount=0;
    }
    else if(status->uPosValue==2)
    {
      status->uPosAmount=stringToInt(ptr+13,7);
      status->uVolume=0;
      status->uAmount=0;    
    }        
    if(status->NZZLStatus!=NZZL_HANG )
    {
      status->uLeng[0]=LengthOfInt(status->uPrice);
      status->uLeng[1]=LengthOfInt((u32)(status->uVolume));
      status->uLeng[2]=LengthOfInt(status->uAmount);
      Split_Digit(status->uPrice,status->uArray_UP,5);
      Split_Digit((u32)(status->uVolume),status->uArray_VL,7);
      Split_Digit(status->uAmount,status->uArray_AM,7);
    }
    if(status->errCode.Code[0]!=0x30&&(status->errCode.Code[0]!='F')&&(status->errCode.Code[1]!='F'))bExitsErrcode=TRUE; 
    else bExitsErrcode=FALSE; 
  } 
}

bool Send_Text01(void)
{
  u8 Buff[10];
  Buff[0]=STX;Buff[1]=idLCD;Buff[2]=ENQ_POL;Buff[3]=0x30;
  Buff[4]=0x31;Buff[5]=ETX;Buff[6]=POS_CheckSum(Buff,1,6);
  if(POS_Send(Buff,7,20) == TRUE)
  {
    return TRUE;
  }
  return FALSE;
}
bool Send_Text02(void)
{
  u8 Buff[10];
  Buff[0]=STX;Buff[1]=idLCD;Buff[2]=ENQ_POL;Buff[3]=0x30;
  Buff[4]=0x33; Buff[5]=0x30;Buff[6]=ETX;Buff[7]=POS_CheckSum(Buff,1,7);
  if(POS_Send(Buff,8,20) == TRUE)
  {
    return TRUE;
  }
  return FALSE;
}
bool Send_Text05(void)
{
  u8 Buff[10];
  Buff[0]=STX;Buff[1]=idLCD;Buff[2]=ENQ_POL;Buff[3]=0x30;
  Buff[4]=0x35; Buff[5]=ETX;Buff[6]=POS_CheckSum(Buff,1,6);
  if(POS_Send(Buff,7,20) == TRUE)
  {
    return TRUE;
  }
  return FALSE;
}

bool Send_ACK0(void)
{
  u8 Buff[3];
  Buff[0]=ACK;Buff[1]=0x30;
  if(POS_Send(Buff,2,20) == TRUE)
  {
    return TRUE;
  }
  return FALSE;
}
bool Send_ACK1(void)
{
  u8 Buff[3];
  Buff[0]=ACK;Buff[1]=0x31;
  if(POS_Send(Buff,2,20) == TRUE)
  {
    return TRUE;
  }
  return FALSE;
}
bool Send_EOT(void)
{
  u8 Buff[2];
  Buff[0]=EOT;
  if(POS_Send(Buff,1,20) == TRUE)
  {
    return TRUE;
  }
  return FALSE;
}

bool Send_Text(DataSetup_t *data)
{
  bool sendStatus=FALSE; 
  u8 i,j;
  u8 buff[100]={0};
  u8 buff_code[3]={0};
  u8 buff_data[80]={0};
  DigitsExtraction(buff_code,2,data->code);
  buff[0]=STX;buff[1]=idLCD; buff[2]=ENQ_POL;buff[3]=buff_code[0];buff[4]=buff_code[1];  

  if(data->code==12)
  {    
    if(data->AmountOrVolume==1)
    {
      /*volume*/     
      DigitsExtraction(buff_data,10,(data->data64)*(uint64_t)pow(10,(3 - data->leng_tp)));      
      buff[5]='0';          
    }
    else if(data->AmountOrVolume==2)
    {
      /*amount*/
      DigitsExtraction(buff_data,10,(data->data64));            
      buff[5]='1';          
    }
    for(i=0;i<10;i++)
    {
      buff[i+6]=buff_data[i];
    }     
    buff[16]=ETX;buff[17]=POS_CheckSum(buff,1,17); 
    if(POS_Send(buff,18,50) == TRUE)
    {
      sendStatus=TRUE;
    }   
  }
  else if(data->code==16)
  {  
    DigitsExtraction(buff_data,7,(data->data64)*(uint64_t)pow(10,(3 - data->leng_tp)));         
    for(i=0;i<7;i++)
    {
      buff[i+5]=buff_data[i];
    }    
    buff[12]=ETX;buff[13]=POS_CheckSum(buff,1,13);
    if(POS_Send(buff,14,50) == TRUE)
    {
      sendStatus=TRUE;
    }  
  }
  else if(data->code==24)
  {
    switch(data->AmountOrVolume)
    {     
      case 1:
      case 2:
        /*Volume*/
        buff[5]='4';buff[6]=data->Index[0]+0x30;      
        if(data->AmountOrVolume==2)
        {
          buff[7]='0';
          DigitsExtraction(buff_data,7,(data->data64)*(uint64_t)pow(10,(sConfiguration.DecimalPlace.Volume- data->leng_tp)));            
        }  
        /*Amount*/
        else
        {
          buff[7]='1';
            DigitsExtraction(buff_data,7,data->data64);         
        }
        for(i=0;i<7;i++)
        {
          buff[i+8]=buff_data[i];
        }         
        buff[15]=ETX;buff[16]=POS_CheckSum(buff,1,16);
        if(POS_Send(buff,17,50) == TRUE)
        {
          sendStatus=TRUE;
        }                 
        break;
      case 0:           
      case 3:
        buff[5]=data->AmountOrVolume+0x30;
        buff[6]=ETX;buff[7]=POS_CheckSum(buff,1,7); 
        if(POS_Send(buff,8,50) == TRUE)
        {
          sendStatus=TRUE;
        }         
        break;      
    }
  }  
  else if(data->code==37)
  {
    buff[5]=data->dataArr[0]+0x30;
    buff[6]=data->dataArr[1]+0x30;
    buff[7]=data->dataArr[2]+0x30;
    buff[8]=ETX;buff[9]=POS_CheckSum(buff,1,9); 
    if(POS_Send(buff,10,20) == TRUE)
    {
      sendStatus=TRUE;
    }    
  }
  else if(data->code==40)
  {
    buff[5]=data->Index[0]+0x30;
    buff[6]=ETX;buff[7]=POS_CheckSum(buff,1,7); 
    if(POS_Send(buff,8,20) == TRUE)
    {
      sendStatus=TRUE;
    }
  }
  else if(data->code==41)
  {
    buff[5]=(data->AmountOrVolume)+0x30;
    DigitsExtraction(buff_data,2,data->data64);
    for(i=0;i<2;i++)
    {
      buff[i+6]=buff_data[i];
    }    
    buff[8]=ETX;buff[9]=POS_CheckSum(buff,1,9);
    if(POS_Send(buff,10,50) == TRUE)
    {
      sendStatus=TRUE;
    }        
  }  
  /*
    code=5 -> xcxc; code=8->read 8 logs; code=10 ->read total; code=7->request data printer 
  */
  else if((data->code==Code_Login_Mode)||(data->code==Code_Read_Logs)||(data->code==Code_Read_Total) ||(data->code==Code_Login_Printer)||(data->code==Code_Keypad_Loss_Power))
  {
    buff[5]=ETX;buff[6]=POS_CheckSum(buff,1,6); 
    if(POS_Send(buff,7,20) == TRUE)
    {
      sendStatus=TRUE;
    }     
  }
  else if(data->code==95)
  {
    DigitsExtraction(buff_data,4,data->data_calender[0]);
    for(i=0;i<4;i++)
    {
      buff[i+5]=buff_data[i];
    }
    for(j=1;j<5;j++)
    {
      DigitsExtraction(buff_data,2,data->data_calender[j]);
      for(i=0;i<2;i++)
      {
        buff[(i+9)+2*(j-1)]=buff_data[i];
      }  
    }
    buff[17]=ETX;buff[18]=POS_CheckSum(buff,1,18); 
    if(POS_Send(buff,19,50) == TRUE)
    {
      sendStatus=TRUE;
    }      
  }
 /*code=6 -> send preset*/
  else if(data->code==Code_SendData_Preset)
  {
    /*Amount Preset P1-4 (1)OR Amount Preset Select from Keypad (3)*/
    if(data->AmountOrVolume==1 ||data->AmountOrVolume==3)
    {
      buff[5]='1'; 
      DigitsExtraction(buff_data,7,data->data64);
    }
    /*Volume Preset P1-4*/
    else if(data->AmountOrVolume==2 ||data->AmountOrVolume==4 )
    {
      buff[5]='0';       
      DigitsExtraction(buff_data,7,(data->data64)*(uint64_t)pow(10,(sConfiguration.DecimalPlace.Volume - data->leng_tp)));             
    }
    for(i=0;i<7;i++)
    {
      buff[i+6]=buff_data[i];
    }    
    buff[13]=ETX;buff[14]=POS_CheckSum(buff,1,14);  
    //tang timeout 50->500ms
    if(POS_Send(buff,15,500) == TRUE)
    {
      sendStatus=TRUE;
    }     
  }
  /*code=50 ->send config information printer*/
  else if(data->code==Code_SendData_Printer)
  {
    /*Ten-Dia chi- Kieu - Ky hieu - So serial - Loai Xang dau*/
    buff[5]=uPrinterID+0x30;
    for(i=0;i<data->leng_data;i++)
    {
      buff[6+i]=data->dataArr[i]; //aPrinterBuffer[uPrinterID][i];
    }
    buff[6+data->leng_data]=ETX;buff[7+data->leng_data]=POS_CheckSum(buff,1,7+data->leng_data);   
    if(POS_Send(buff,8+data->leng_data,200) == TRUE)
    {
      sendStatus=TRUE;
    }     
  }
  else if(data->code==Code_SendData_Fueling)
  {
    if((sysStatus.NZZLStatus==NZZL_REFUELING))
    {
      buff[5]=sysStatus.NZZLStatus+0x30;
      buff[6]=data->chr;
      for(i=7;i<15;i++)
      {
          buff[i]=0x20;
      }
      buff[15]=ETX;
      buff[16]=POS_CheckSum(buff,1,16);   
      if(POS_Send(buff,17,200) == TRUE)
      {
        sendStatus=TRUE;
      }    
    }
  }
  else
  {
    DigitsExtraction(buff_data,data->leng_default,data->data64);
    for(i=0;i<data->leng_default;i++)
    {
      buff[i+5]=buff_data[i];
    }     
    buff[data->leng_default +5]=ETX;buff[data->leng_default +6]=POS_CheckSum(buff,1,data->leng_default +6); 
    if(POS_Send(buff,data->leng_default +7,50) == TRUE)
    {
      sendStatus=TRUE;
    }   
  }
  return sendStatus;
}
/*Update*/
eReturnStatus POS_RegistrationToCpu(void)
{ 
  eReturnStatus retValue = pINVALID;
 // SysStatus_t           sysStatus;
  eReturnStatus eStatus;
  u8 addr = idLCD;
  u8 PosBuffCom[POS_BUFF_LEN] = {0};
  u8 configBuff[POS_BUFF_LEN]={0};
 // xTickToWait = 10000;
#define LCD_TICKS_WAIT 1000
  
  if((eStatus=POS_WaitForMsg( addr, ENQ_POL,ENQ_SEL, &LcdCode, &PosBuffCom[0],(TickType_t)LCD_TICKS_WAIT )) == pTRUE)
  {
        if(PosBuffCom[0]==EOT && PosBuffCom[1]==addr && PosBuffCom[2]==ENQ_POL && PosBuffCom[3]==ENQ )
        {          
            if (Send_Text01()==TRUE)
            {
                if((POS_WaitForMsg( addr, ENQ_POL,ENQ_SEL, &LcdCode, &PosBuffCom[0], (TickType_t)20 ) == pTRUE))
                {
                    if(PosBuffCom[0] == ACK && PosBuffCom[1] == 0x31)
                    {
                        if(Send_EOT()==TRUE)
                        {
                          if((POS_WaitForMsg( addr, ENQ_POL,ENQ_SEL, &LcdCode, &PosBuffCom[0], (TickType_t)20 ) == pTRUE))
                          {
                            if(PosBuffCom[0]==EOT && PosBuffCom[1]==addr && PosBuffCom[2]==ENQ_SEL && PosBuffCom[3]==ENQ )
                            {                             
                              if(Send_ACK0()==TRUE)
                              {
                               // xTickToWait =160;
                                if((POS_WaitForMsg( addr, ENQ_POL,ENQ_SEL, &LcdCode, &configBuff[0], (TickType_t)160 ) == pTRUE))
                                {
                                  if( LcdCode == 2)
                                  {
                                    if(Send_ACK1()==TRUE) 
                                    {
                                      //xTickToWait = 20;
                                      if((POS_WaitForMsg( addr, ENQ_POL,ENQ_SEL, &LcdCode, &PosBuffCom[0], (TickType_t)20 ) == pTRUE))
                                      {
                                        if(PosBuffCom[0]==EOT && PosBuffCom[1]==addr && PosBuffCom[2]==ENQ_POL && PosBuffCom[3]==ENQ )
                                        {
                                          if(Send_Text02()==TRUE)
                                          {
                                            //xTickToWait = 20;
                                            if((POS_WaitForMsg( addr, ENQ_POL,ENQ_SEL, &LcdCode, &PosBuffCom[0], (TickType_t)20 ) == pTRUE))
                                            {
                                              if(PosBuffCom[0]==ACK && PosBuffCom[1]==0x31 )
                                              {
                                                if(Send_EOT()==TRUE)
                                                {
                                                  POS_UpdateConfig(configBuff,&sConfiguration);                                              
                                                  POS_LitmitSetup(sConfiguration.UnitPrice,&fVolumeLimit);                                                  
                                                  /*Start led system*/
                                                  xTimerStart( xTimers[4],1) ;   
                                                  LCD_Default();
                                                  QueueSend_Err(&sysStatus,'0','0');
                                                  retValue = pTRUE;  
                                                  //bPowerOn=TRUE;
                                                }//else pos_FailCnt=1;
                                              }//else pos_FailCnt=2;
                                            }//else pos_FailCnt=3;
                                          }//else pos_FailCnt=4;
                                        }//else pos_FailCnt=5;
                                      }//else pos_FailCnt=6;
                                    }//else pos_FailCnt=7;
                                  }//else pos_FailCnt=8;
                                }//else pos_FailCnt=9;
                              }//else pos_FailCnt=10;
                            }//else pos_FailCnt=11;
                          }//else pos_FailCnt=12;
                        }//else pos_FailCnt=13;
                    }//else pos_FailCnt=14;
                }//else pos_FailCnt=15;
            }//else pos_FailCnt=16;
        }//else pos_FailCnt=17;
   }
else
{
  if(eStatus==pTIMEOUT)retValue=pTIMEOUT;
}
   return retValue;
}


bool Sellecting_ReceivedEOTFromCPU(u8 lcdcode,u8 *Posbuff)
{
  bool receivedCode=FALSE;
  u8 code=0;
  /*
  2:            Received sys config
  4:            received Sys status
  7:            Received error code
  8:            Received Frame data Log
  9:            Received Busy code
  10:           Received Total
  50:           Received data Printer
  */
  if(lcdcode==2||lcdcode==4||lcdcode==7||lcdcode==8||lcdcode==9||lcdcode==10||lcdcode==50)
  {
    if(Send_ACK1()==TRUE) 
    {
       //xTickToWait = 15;
       receivedCode=TRUE;  
       if (POS_WaitForMsg( idLCD,ENQ_SEL, ENQ_POL, &code, Posbuff, (TickType_t)20 ) == pTRUE)
       {
          if(Posbuff[0] == EOT )
          {   
            receivedCode=TRUE;       
          }
       }

    }  
  }
  return receivedCode;
}

eReturnStatus POS_FuelingMode(void)
{
  ReceivePrinterData_t rxPrinterData;
 // SysStatus_t           sysStatus;
  u8 PosBuffCom[POS_BUFF_LEN] = {0};   
  eReturnStatus retValue = pTRUE;
  eReturnStatus eStatus;  
  //EventBits_t   uxBits;
  u8 addr = idLCD;
  u8 buff1[POS_BUFF_LEN] = {0}; 
  DataSetup_t   dataSetup;
  //xTickToWait=5000;
#define LCD_TICKS_WAIT_2 1000
  
  if ((eStatus=POS_WaitForMsg( addr, ENQ_SEL,ENQ_POL, &LcdCode, &PosBuffCom[0], (TickType_t)LCD_TICKS_WAIT_2 ) )== pTRUE)
  {
    /*If received ENQ Selecting*/
    if((PosBuffCom[0] == EOT) && (PosBuffCom[1] == addr )&& (PosBuffCom[2] == ENQ_SEL) && (PosBuffCom[3] == ENQ)) 
    {
      if(Send_ACK0()==TRUE)
      {
       // xTickToWait=500;
        if (POS_WaitForMsg( addr, ENQ_SEL,ENQ_POL, &LcdCode, &buff1[0], (TickType_t)500 ) == pTRUE)
        {
          if(Sellecting_ReceivedEOTFromCPU(LcdCode,PosBuffCom)==TRUE)
          {
            /*Received system configuration*/
            if(LcdCode==Code_Received_SysConfig)
            {
               POS_UpdateConfig(buff1,&sConfiguration);
               POS_LitmitSetup(sConfiguration.UnitPrice,&fVolumeLimit);      
            }
            /*Received system status*/
            else if(LcdCode==Code_Received_SysStatus)
            {               
              POS_UpdateSysStatusData(buff1,&sysStatus);                
              xQueueOverwrite(xPosRxSysStatusQueue, &sysStatus);  
              //LCD_NozzleFromCPU(&sysStatus);   
            }
            else if(LcdCode==Code_Read_Logs)
            {   /*10 Log*/
              memcpy(&sFrameLogs,buff1,260);//260  
              xEventGroupSetBits(xPosReceiDataEventGroup,BIT_0);    
            }
            /*Received Busy code*/
            else if(LcdCode==Code_Received_BusyStatus)
            {
              bKeypadEnable=FALSE;  
             // bExitsErrcode=FALSE;
              Mode=SUNNYXE_BUSY;
              LAPIS_BusyState(); 
            }
            /*Received Total*/
            else if(LcdCode==Code_Read_Total)
            {        
              sConfiguration.Totalizer.volume=stringToInt(buff1,10);
              sConfiguration.Totalizer.amount=stringToInt(buff1+10,10);
              sConfiguration.DailyTotal.volume=stringToInt(buff1+20,10);
              sConfiguration.DailyTotal.amount=stringToInt(buff1+30,10);                         
              xEventGroupSetBits(xPosReceiDataEventGroup,BIT_0); 
            }
            /*Received data printer*/
            else if(LcdCode==Code_Received_DataPrinter)
            {
              memcpy(rxPrinterData.data,buff1,360);
              if(xQueueSend(xPrinterMsgQueue,&rxPrinterData, ( TickType_t )0) != pdTRUE)
              {
              }              
            }            
          } 
        }
      }
    }
    /*If received ENQ POLLING*/
   if(PosBuffCom[0] == EOT && PosBuffCom[1] == addr && PosBuffCom[2] == ENQ_POL && PosBuffCom[3] == ENQ)
   {
      if( xQueueReceive( xPosSendDataQueue, &dataSetup, ( TickType_t ) 5 ) )
      { 
        /*Send code*/
        if(Send_Text(&dataSetup)==TRUE )
        {
          //tang time out tu 500ms->5s
          if (POS_WaitForMsg( addr, ENQ_SEL,ENQ_POL, &LcdCode, &PosBuffCom[0], (TickType_t)500 ) == pTRUE)
          {
            if(PosBuffCom[0] == ACK && PosBuffCom[1] == 0x31)
            {
              xEventGroupSetBits(xPosReceiDataEventGroup,BIT_0);
              if(Send_EOT()==TRUE)                    
              {
              }                   
            }
          }
        }
      }                                       
   /*if Not have data then send EOT */
    else
    {
      if(Send_EOT()==TRUE)
      {
        
      }
    }          
   }              
 }
  else 
  {
    if(eStatus==pTIMEOUT)
    {
      retValue=pTIMEOUT;
    }
  }
  return retValue;
}


eReturnStatus POS_WaitForMsg( u8 addr, eEnqType_t enqType1,eEnqType_t enqType2,u8 *Code, u8 *pMsg,TickType_t timeout )//add leng data
{
  eReturnStatus       retValue = pTRUE;
  EventBits_t   uxBits;
  u8       pxRxedMessage;
  u16       dataLen = 0;
  u8       BCC = 0;
  u8       buff[POS_BUFF_LEN] = {0};
  u8       *ptr = buff;
  /* empty rx buffer before receiving data */
  memset(pMsg,0,POS_BUFF_LEN);
  pos_WaitForNewMsg = TRUE;
    uxBits = xEventGroupWaitBits(
              xPosRxEventGroup,   /* The event group being tested. */
              POS_RX_DONE_EVENT_BIT, /* The bits within the event group to wait for. */
              pdTRUE,        /* BIT_0 should be cleared before returning. */
              pdFALSE,       /* Don't wait for both bits, either bit will do. */
              timeout );/* Wait a maximum of 100ms for either bit to be set. */
    if((uxBits & POS_RX_DONE_EVENT_BIT) == POS_RX_DONE_EVENT_BIT)
    {
      do
      {
        if( xQueueReceive( xPosRxMsgQueue, &( pxRxedMessage ), ( TickType_t ) 0 ) )
        {          
          *ptr = pxRxedMessage;
          ptr++;
          dataLen++;
        }
      }while(uxQueueMessagesWaiting( xPosRxMsgQueue )!= (UBaseType_t)0);
     
      if(buff[0] == STX && buff[dataLen-2] == ETX)
      {
        if((buff[1] == addr && buff[2] == enqType1)||(buff[1] == addr && buff[2] == enqType2))
        {
          BCC = POS_CheckSum(buff,1, dataLen-1);          
                    
          if(BCC == buff[dataLen-1])
          {       
            *Code = ((buff[3]-(u8)0x30) * 10) + (buff[4]-0x30); 
            if(*Code==10)
            {
              if(dataLen==47)
              {
                 ArrayCoppy(pMsg, &buff[5], dataLen-7);
                 
              }
              else
              {
                retValue = pINVALID;//pFALSE;
              }
            }
            else if(*Code==2)
            {
              if(dataLen==CODE2_LENGTH)
              {
                 ArrayCoppy(pMsg, &buff[5], dataLen-7);
                 
              }
              else
              {
                retValue = pINVALID;//pFALSE;
              }
            }
            else
            {
                 ArrayCoppy(pMsg, &buff[5], dataLen-7);
            }
          }
          else
          {            
            retValue = pINVALID;//pFALSE;
          }
        }
        else 
        {      
          *Code=0;
          retValue = pINVALID;
        }
      }
      else  
      { 
        /* save data */
        if(dataLen > 2)
        {
          if(buff[1] != addr )
          {
            retValue = pINVALID;
          }
          else
          {
            if((buff[2] == enqType1)||(buff[2] == enqType2))
            {
             ArrayCoppy(pMsg, buff, dataLen);  
            }
            else
              retValue = pINVALID;
          }
        }
        else
        {
           ArrayCoppy(pMsg, buff, dataLen);  
        }
        *Code = 0;         
      }
    }
    else
    {
        retValue = pTIMEOUT;
    }
  vTaskDelay(3);
  return retValue;
}

bool POS_Send(u8 *pBuff, u8 len, TickType_t timeout)
{
  bool retValue = TRUE;
  u8 i = 0;
  
  xQueueReset( xPosRxMsgQueue );
  xQueueReset( xPosTxMsgQueue );
  /* Enable Data to be transmitted over RS485 */
  POS_EnableTransmit();  
      
  /* ENQ POL: EOT - Addr - ENQ_POL - ENQ */
  for( i = 0; i < len; i++)
  {
    xQueueSend( xPosTxMsgQueue, ( void * ) (pBuff+i), ( TickType_t ) 0 );
  }
  if(POS_WaitForDataSentDone((TickType_t)(timeout)) != TRUE){ retValue = FALSE; }
 
  return retValue;
}

bool POS_WaitForDataSentDone(TickType_t timeout)
{
  bool     retValue = TRUE;
  EventBits_t uxBits;

  pos_IsSendFail = FALSE;
  /* Enable Tx interrupt. This way, data is transmitted immediately  */
  USART_ITConfig( USART2, USART_IT_TXE, ENABLE );
  
   /* Then wait for send done by loopback check method */
  uxBits = xEventGroupWaitBits(
            xPosRxEventGroup,   /* The event group being tested. */
            POS_RX_DONE_EVENT_BIT, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            timeout );/* Wait a maximum of 100ms for either bit to be set. */

  if((uxBits & POS_RX_DONE_EVENT_BIT) == POS_RX_DONE_EVENT_BIT)
  {   
    if(pos_IsSendFail == TRUE){
      retValue = FALSE;
    }
    else retValue = TRUE;
  }
  else
  {
      retValue = FALSE;
  }
  return retValue;
}

/*
    Function: Calculate checksum for specified array of bytes
*/
u8 POS_CheckSum(u8 *pBuff, u8 from, u8 to)
{
    u8 BCC = 0;
    u16 i = 0;
    
    for(i = from; i < to; i++)
    {
        BCC |= *(pBuff+i);
    }
    BCC = BCC << 2;
    BCC = BCC + (u8)1;
    
    return BCC;
}
void POS_Init(void)
{    
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);
    xPrinterMsgQueue    = xQueueCreate( 1 , sizeof( ReceivePrinterData_t ) );
    xPosRxMsgQueue      = xQueueCreate(POS_QUEUE_LEN, sizeof( u8 ) );
    xPosTxMsgQueue      = xQueueCreate(POS_QUEUE_LEN, sizeof( u8 ) ); 
    xPosRxSysStatusQueue= xQueueCreate(1U,sizeof( SysStatus_t ) );
    xPosSendDataQueue   = xQueueCreate(1U,sizeof(DataSetup_t));
    xPosRxErrorCodeQueue= xQueueCreate(1U,sizeof( Errorcode_t ) );    
    xPosRxEventGroup    = xEventGroupCreate();
    xPosReceiDataEventGroup= xEventGroupCreate();
    //xPosHangEventGroup= xEventGroupCreate();
    //return TRUE;
    xSemaphore = xSemaphoreCreateMutex();
    
}

volatile bool posTimerEnabled = FALSE;

void USART2_IRQHandler(void)
{
  portBASE_TYPE     xHigherPriorityTaskWoken = pdFALSE;
  u8           cChar;
  static u8    TxcChar;

    if( USART_GetITStatus( USART2, USART_IT_TXE ) == SET )
    {
        /* The interrupt was caused by the THR becoming empty.  Are there any
        more characters to transmit? */
        if( xQueueReceiveFromISR( xPosTxMsgQueue, &TxcChar, &xHigherPriorityTaskWoken ) == pdTRUE )
        {
            /* A character was retrieved from the queue so can be sent to the
            THR now. */
            pos_IsTxMode = TRUE;
            USART_SendData( USART2, TxcChar );
            USART_ITConfig( USART2, USART_IT_TXE, DISABLE );
        }
        else
        {
            pos_IsTxMode = FALSE;
            USART_ITConfig( USART2, USART_IT_TXE, DISABLE );
            POS_DisableTransmit();
            xEventGroupSetBitsFromISR( xPosRxEventGroup,   /* The event group being updated. */
                                    POS_RX_DONE_EVENT_BIT, /* The bits being set. */
                                    &xHigherPriorityTaskWoken );
        }
    }

   if( USART_GetITStatus( USART2, USART_IT_RXNE ) == SET )
    {
        cChar = (u8)USART_ReceiveData( USART2 );
        if(pos_IsTxMode == TRUE )
        {
          /* in transmit mode, verify each transmitted byte by equal comparision with each received byte 
          If this is not the case, tell that Sending was failed */
          if(cChar == TxcChar)  { USART_ITConfig( USART2, USART_IT_TXE, ENABLE );}
          else                  { pos_IsSendFail = TRUE; pos_IsTxMode = FALSE; POS_DisableTransmit(); }         
        }
        else
        {
          if(pos_WaitForNewMsg == TRUE)
          {
            if(cChar == STX || cChar == EOT || cChar == ACK || cChar == NAK )
            {
                pos_WaitForNewMsg = FALSE;
                
                pos_StartFeedData = TRUE;
                
              if(posTimerEnabled == FALSE)
              {
                posTimerEnabled = TRUE;            
                TIM_Cmd(TIM2, ENABLE);            
                TIM_SetCounter(TIM2, 0U);
                TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
                TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
              }
              //if (xEventGroupSetBitsFromISR( xPosRxEventGroup,  POS_RX_FIRST_BYTE_EVENT_BIT, &xHigherPriorityTaskWoken ) != pdTRUE) while(1);                
              xEventGroupSetBitsFromISR( xPosRxEventGroup,  POS_RX_FIRST_BYTE_EVENT_BIT, &xHigherPriorityTaskWoken ) ;
            }
          }
          /* save data if this data is belong to remote device */
          if ( xQueueIsQueueFullFromISR( xPosRxMsgQueue ) == pdFALSE && (pos_StartFeedData == TRUE))
          {
            xQueueSendFromISR( xPosRxMsgQueue, &cChar, &xHigherPriorityTaskWoken );
            
            /* Clear timer counter whenever get one byte */
            TIM_SetCounter(TIM2, 0U);
            
            pos_IsRxDataDetected = TRUE;
          }
        }
                      
        USART_ClearFlag(USART2,USART_FLAG_RXNE);
        
        /* Clear timer counter whenever get one byte */
       TIM_SetCounter(TIM2, 0U);        
       
    }           
   portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void TIM2_IRQHandler( void )
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {  

    if(pos_IsRxDataDetected == TRUE)
     {
        pos_IsRxDataDetected = FALSE;
        
        pos_StartFeedData = FALSE;
        
        posTimerEnabled = FALSE;
        TIM_Cmd(TIM2, DISABLE);
        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
        
        xEventGroupSetBitsFromISR( xPosRxEventGroup,   /* The event group being updated. */
                                    POS_RX_DONE_EVENT_BIT, /* The bits being set. */
                                    &xHigherPriorityTaskWoken );
     }
     TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
          
  }  
  portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


void TM_TIMER2_Init(void)
{
    TIM_TimeBaseInitTypeDef     TIM_BaseStruct;
    NVIC_InitTypeDef            nvicStructure;
    
    /* Enable clock for TIM2 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
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
    TIM_BaseStruct.TIM_Prescaler = 71;
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
    TIM_BaseStruct.TIM_Period = 500; /* 500us period */
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
   // TIM_BaseStruct.TIM_RepetitionCounter = 0;
    /* Initialize TIM2 */
    TIM_TimeBaseInit(TIM2, &TIM_BaseStruct);
    /* Start count on TIM2 */
    TIM_Cmd(TIM2, DISABLE);    

    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

    /* Enable interrupt */
    nvicStructure.NVIC_IRQChannel = TIM2_IRQChannel;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
    nvicStructure.NVIC_IRQChannelSubPriority = 0;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void USART2_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;
        
  /* Enable USART1 clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );	
    
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);
    /* Configure USART1 Rx (PA3) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
    
    /* Configure USART1 Tx (PA2) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init( GPIOA, &GPIO_InitStructure );

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Clock = USART_Clock_Disable;
    USART_InitStructure.USART_CPOL = USART_CPOL_Low;
    USART_InitStructure.USART_CPHA = USART_CPHA_2Edge;
    USART_InitStructure.USART_LastBit = USART_LastBit_Disable;
    
    USART_Init( USART2, &USART_InitStructure );
    
    USART_ITConfig( USART2, USART_IT_RXNE, ENABLE );
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );
    
    USART_Cmd( USART2, ENABLE );	
    
  /* Configure LCD DE (PA4) as output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init( GPIOA, &GPIO_InitStructure );
  
  /* Set the Tx Pin -Off*/
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
}

#endif
    
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
