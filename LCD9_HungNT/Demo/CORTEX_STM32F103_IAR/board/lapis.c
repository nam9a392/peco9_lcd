/**
  ******************************************************************************
  * Project		: PC9
  * File Name           : LCD.c
  * Author 	        : Nguyen Tran Duy
  * Start	        : 15/12/2016
  * Stop                : 15/6/2017
  * Version		: 1.8
  ******************************************************************************
  * Description:
  *             Update version 1.8: 22/5/2021
  *             Users can press button while  pump is fueling
  ******************************************************************************
  */
/*
  code 32,33 can changed in 81888554 mode
  code 96: 0-cant printer;1-printer;2- Auto;
  change code 37;
*/
#include "FreeRTOS.h"
#include "task.h" 
#include "semphr.h"

#include "stm32f10x_it.h"
#include "lapis.h"
#include "TextLcd.h"
#include "math.h"
#include "keypad.h"

u8              aCode[Size_Array_AdminMode]={11,12,13,15,16,18,19,20,21,24,29,30,32,33,36,37,41,44,45,46,47,48,63,64,80,90,95,99};//,21,38,
u8              aCodePeco[Size_Array_PecoMode]={70};
u8		aBuffKey[15];
u8              uCntPcode=0,uCntScode=0,uCntC=0;
u8              uPresetDecimalLeng=0;
u8              uColumn=0;
u8              uLengDataPrinter[8]={0};
u8              uBuffLeng[8]={0};
volatile        u8              uPrinterID=0;
volatile        u8              DirrectCode=0; 
uint64_t        uValue=0;
ReceivePrinterData_t    sPrinterRxData;
volatile SysConfig_t    sConfiguration;   
volatile eTypePressX_t           eTypeRead_Select=READ;

char            aPrinterBuffer[8][80]={0}; 
char 		cKEY=0;

bool            bStopTimer=FALSE;
bool            bStartTimer=FALSE;
bool            bSaveData=FALSE;
bool            bPressDot=FALSE;
bool            bSave_Success=FALSE;
bool            bSave_Fails=FALSE;
bool            bReview=FALSE;
volatile        bool            bSelectCode=TRUE;
//volatile        bool            bSelectP=FALSE;
volatile        bool            bChangecode_SunnyPeco=FALSE;
volatile bool                   bClearKeyX=FALSE; 
volatile bool                   bLowcase=FALSE;
volatile bool                   bMoveCursor=FALSE;
volatile u32                    uPresetNum=0;
volatile u8                     uLengTphan=0;
volatile u8                     uDataLeng=0;
volatile u8                     uProcessCodeLeng=0;
volatile EnablePreset_t         sEnablePreset={TRUE,TRUE,TRUE};
volatile eLoginMode_t           Mode=SUNNYXE_PRESET;
volatile bool                   bBlinkControl=FALSE;

volatile        bool         bFlagValidEnterCode=FALSE;
volatile        bool         bReadOnly=TRUE; 
volatile        bool         bHaveDot=FALSE;
volatile        BOOLEAN      PRESET_TypeFlag[4]={2,2,2,2};              
volatile        bool         bKeypadEnable=FALSE;           
volatile        bool         bLogin=FALSE;
volatile        bool         bPrinterEnable=FALSE;
//extern          SemaphoreHandle_t xMutex;


extern volatile SemaphoreHandle_t xSemaphore;


extern          volatile bool    bSinalPFL;
extern          volatile bool         bExitsErrcode;
extern          volatile              TimerHandle_t xTimers[5];
extern          volatile              EventGroupHandle_t     xKeypadEventGroup;
extern          volatile              EventGroupHandle_t     xPosReceiDataEventGroup;
extern          volatile              QueueHandle_t          xPosSendDataQueue;
extern          volatile              QueueHandle_t          xKeypadQueue;
extern          volatile              QueueHandle_t          xPrinterMsgQueue;
extern          volatile              u8                     uPresetValue;
extern           u8                      uSegDigits[18];
extern           FrameLogs_t             sFrameLogs;
extern           volatile TypeValue_t             sTypeValues;
extern          volatile float           fVolumeLimit;
extern          volatile QueueHandle_t   xPosRxSysStatusQueue;

extern volatile bool                  bPowerOn_NzzlHang;
/*Read 10 logs and Total*/
bool SUNNYXE_ReadTotal_Logs(DataSetup_t dt)
{
   bool value=FALSE;
   if(WaitTransmitDone(&dt,TRUE)==TRUE)     
   { 
     if(WaitTransmitDone(&dt,FALSE)==TRUE)
     {
       value=TRUE;      
     }
   } 

 return value;   
}
/*Send preset - Amount or Volume*/
void PRESET_SendValue(char key)
{
  bool bl=FALSE;
  DataSetup_t data;
  
  if(key=='$')
  {
    /*Amount*/
    data.AmountOrVolume=3;
    bl=TRUE;
  }
  else if(key=='L')
  {
    /*Volume*/
    data.AmountOrVolume=4; 
    data.leng_tp=uPresetDecimalLeng;
  }
  data.code=6;
  data.data64=uPresetNum; 
//  TextLcd_Display(bl,uPresetNum,uPresetDecimalLeng,0,FALSE);
  if(sEnablePreset.bNzzlHang==TRUE)//||(WaitReceivedNzzlHang()==TRUE)
  {
    if(WaitTransmitDone(&data,TRUE)==TRUE)      
    {
      sEnablePreset.bNumber=FALSE;
      TextLcd_Display(bl,uPresetNum,uPresetDecimalLeng,0,TRUE);
    }
    else
    {
     LCD_SenFalseMessage(); 
    } 
  }
}

/*Display Total*/
void LCD_ReadTotal(u8 cnt,uint64_t amount,double volume)//,SysConfig_t     *config
{
  if(cnt==0)LCD_DisplayFollowLanguage( 1,0,1,0,"So tong:","Total:" );
  else  LCD_DisplayFollowLanguage( 1,0,1,0,"Tong ngay:","Total Day:" );

  LCD_DisplayAmount(amount,2,1,2,12,TRUE);
  LCD_DisplayVolume(3,1,3,12,volume,TRUE);//sConfiguration.DecimalPlace.Volume
}
/*Display 10 logs*/
void LCD_ReadLog(u8 nLog)
{
     LCD_Default();
     LCD_DisplayNumber(nLog,1,0);
     LCD_DisplayNumber(stringToInt(sFrameLogs.logs[nLog-1].calender.uDate,2),1,4);
     LCD_Puts(1,6,"/");  
     LCD_DisplayNumber(stringToInt(sFrameLogs.logs[nLog-1].calender.uMonth,2),1,7);
     LCD_Puts(1,9,"/");           
     LCD_DisplayNumber(stringToInt(sFrameLogs.logs[nLog-1].calender.uYear,4),1,10);
     LCD_DisplayNumber(stringToInt(sFrameLogs.logs[nLog-1].calender.uHour,2),1,15);
     LCD_Puts(1,17,":");      
     LCD_DisplayNumber(stringToInt(sFrameLogs.logs[nLog-1].calender.uMin,2),1,18);
     LCD_DisplayVolume(2,6,2,14,(double)stringToInt(sFrameLogs.logs[nLog-1].data.uVolume,7)/pow(10,sConfiguration.DecimalPlace.Volume),TRUE);//sConfiguration.DecimalPlace.Volume
     LCD_DisplayAmount(stringToInt(sFrameLogs.logs[nLog-1].data.uAmount,7),3,6,3,14,TRUE);     
}
/**/
void LCD_MsgSendFalse(void)
{
 LCD_DisplayFollowLanguage( 2,3,2,2,"Dat lai Preset", "Reinstall Preset");
}
u8 ID_P(u8 key)
{
  u8 i=0;
  switch(key)
  {
    /*P1*/
  case ':':
      i=0;
      break;
      /*P2*/
    case '/':
      i=1;
      break;
      /*P3*/
    case ',':
      i=2;
      break;
      /*P4*/
    case '-':
      i=3;
  }
  return i;
}
bool PRESET_Valid(u8 P)
{
  u8 i;
  i=ID_P(P);
  if((sConfiguration.KeypadSetting.OneTouch.PA[i]>0)||(sConfiguration.KeypadSetting.OneTouch.PV[i]>0))  //((sConfiguration.KeypadSetting.OneTouch.PA[i]>=sConfiguration.UnitPrice)||(sConfiguration.KeypadSetting.OneTouch.PV[i]>=1)
  {
    return TRUE;
  }
  return FALSE;
}
/*Send preset P1-4*/
void PRESET_SendP1234(u8 key)
{
  char buffer[5];
  DataSetup_t   data;
  u8 i=0;
  uint64_t am=0,vol=0;
  i=ID_P(key);
  am=sConfiguration.KeypadSetting.OneTouch.PA[i];
  vol=(uint64_t)sConfiguration.KeypadSetting.OneTouch.PV[i];
  data.code=Code_SendData_Preset;
  if(sConfiguration.KeypadSetting.OneTouch.PA[i]>0){
    /*Amount*/
    data.AmountOrVolume=1;
    data.data64=am; 
  }
  else if(sConfiguration.KeypadSetting.OneTouch.PV[i]>0)
  {
    /*Volume*/
    data.AmountOrVolume=2;
    data.data64=vol;
    data.leng_tp=sTypeValues.len_tp[i+2];   
  }    
  /*Wait Nzzl Hang status to send cmd*/
  if((sEnablePreset.bNzzlHang==TRUE))//||(WaitReceivedNzzlHang()==TRUE)
  {
    if(WaitTransmitDone(&data,TRUE)==TRUE) 
    {
      if(PRESET_Valid(cKEY)==TRUE)
      {
        TIMER3_ENABLE(1);    
        sEnablePreset.bP1_4=FALSE;
        sprintf(buffer,"%s%u","P",i+1);
        LCD_Puts(2,1,(int8_t*)buffer);
        if(am>0)
        { 
          LCD_DisplayAmount(am,2,6,2,14,TRUE);
          //Switch_Money(2,14);
        }
        if(vol>0)
        {
          LCD_DisplayVolume(2,6,2,15,vol/pow(10,sTypeValues.len_tp[i+2]),TRUE);//sConfiguration.DecimalPlace.Volume
        }         
      }
      else
      {
        if(am>0)
        { 
          LCD_DisplayFollowLanguage( 2,1,2,1,"Loi Preset < D.Gia","Err Preset< UnitP" );
        }
        if(vol>0)
        {
          LCD_DisplayFollowLanguage( 2,1,2,1,"Loi Preset < 1.0(L)","Err Preset< 1.0(L)" );
        }    
      }
    }    
    else
    {
      LCD_SenFalseMessage();
    }     
  }
}
void LCD_SenFalseMessage(void)
{
    LCD_Default(); 
    LCD_MsgSendFalse(); 
}
void LCD_DisplayVolume(u8 x1,u8 y1,u8 x2,u8 y2,double data,bool bDisplay_Lit)
{
  u8 dot = sConfiguration.DecimalPlace.Volume;
  char buffer[12];  
  if(dot==3)
    sprintf(buffer,"%.3f",data); 
  else if(dot==2)
    sprintf(buffer,"%.2f",data);
  else if(dot==1)
    sprintf(buffer,"%.1f",data);  
  LCD_Puts(x1,y1,(int8_t*)buffer); 
  if(bDisplay_Lit==TRUE)
  {
    LCD_DisplayFollowLanguage( x2,y2,x2,y2,"(LIT)", "(L)");
  }
  
}
void PRINTER_WaitForSendDone(DataSetup_t *data)
{
  if(WaitTransmitDone(data,TRUE)==TRUE) 
  {
    LCD_Default(); 
  }
}

void FUELING_SendData(char key)
{
  char buf[10];
  DataSetup_t data;
  data.code=Code_SendData_Fueling;
  data.chr=key;
  if(WaitTransmitDone(&data,TRUE)==TRUE) 
  {
    if(key=='A')
    {
      strcpy(buf,"STOP $");
    }
    else if(key=='B')
    {
      strcpy(buf,"STOP L");

    }
    LCD_Puts(3,0,(int8_t*)buf);
  }
}
void LCD_Default(void)
{
   if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
   {
      LCD_Clear(); 
      LCD_Puts(0,8,"PECO" );   
      xSemaphoreGive( xSemaphore );
   }

}
void TextLcd_Display(bool bl,u32 num,u8 leng,u8 value,bool bDisplay_Donvi)
{ 
  LCD_Default();
  if(value!=0)LCD_Puts(2,1,"POS:");
  if(bl==FALSE)
  { 
    LCD_DisplayVolume(2,6,2,15,num/pow(10,leng),bDisplay_Donvi);//sConfiguration.DecimalPlace.Volume
  }
  /*Amount*/
  else
  {             
    LCD_DisplayAmount(num,2,6,2,14,bDisplay_Donvi);
  }
}
void LCD_DisplayNumber(u32 num,u8 x,u8 y)
{
  char buffer[10];
  sprintf(buffer,"%lu",num);     
  LCD_Puts(x,y,(int8_t*)buffer); 
}
void Switch_Money(u8 x2,u8 y2)
{
  char buff[10];
  switch(sConfiguration.CountryCode)
  {
    /*Taiwan*/
    case Taiwan:
      //LCD_Puts(x2,y2,"(NT)"); 
      strcpy(buff,"NT");
      break;
      /*Myanma*/
    case Mianma:
      //LCD_Puts(x2,y2,"(Kyat)"); 
      strcpy(buff,"(Kyat)");
      break;
      /*Hong kong*/
//    case HongKong:
//     // LCD_Puts(x2,y2,"(HKD)");  
//      strcpy(buff,"(HKD)");
//      break;
      /*India*/
//    case India:
//      //LCD_Puts(x2,y2,"(Rs)");
//       strcpy(buff,"(Rs)");
//      break;
//      /*Indonesia*/
//    case Indonesia:
//      //LCD_Puts(x2,y2,"(Rp)");
//       strcpy(buff,"(Rp)");
//      break;
//      /*Korea*/
//    case Korea:
//      //LCD_Puts(x2,y2,"(W)");
//       strcpy(buff,"(W)");
//      break;
//      /*Malaysia*/
//    case Malaysia:
//      //LCD_Puts(x2,y2,"(R)");
//       strcpy(buff,"(R)");
//      break;
//      /*Philippin*/
//    case Philippines:
//      //LCD_Puts(x2,y2,"(P)");
//       strcpy(buff,"(P)");
//      break;
      /*Viet Nam*/
    case VietNam:
      //LCD_Puts(x2,y2,"(VND)");
       strcpy(buff,"(VND)");
      break;
      /*Thailand*/
//    case ThaiLand:
//     // LCD_Puts(x2,y2,"(Baht)");
//       strcpy(buff,"(Baht)");
//      break;
      /*Code =0*/
    default:
      //LCD_Puts(x2,y2,"($)");
       strcpy(buff,"($)");
  }  
  LCD_Puts(x2,y2,(int8_t*)buff);
    
}
void LCD_DisplayAmount(uint64_t amount,u8 x1,u8 y1,u8 x2,u8 y2,bool bDisplay_DvTien)
{
    char buffer[12]; 
    sprintf(buffer,"%llu",amount);              
    LCD_Puts(x1,y1,(int8_t*)buffer);
    if(bDisplay_DvTien==TRUE)
    { 
      Switch_Money(x2,y2);
    }
}

bool PRESET_CheckValid(u32 value,u8 lengtp,bool AV)
{
  /*check Amount*/
  if(AV==TRUE)
  {
    if(( value >9999999) ||(value==0) ||(lengtp>0))//(value<sConfiguration.UnitPrice)
    {
      return FALSE;
    }
     return TRUE;
  }
  /*Check Volume*/
  else
  {
    if( ((value/pow(10,lengtp) )<=fVolumeLimit)&& (value>0 )&&(sConfiguration.UnitPrice>0))// (value>=1 )
    {
      return TRUE;
    }
    return FALSE;
  }
}

void vTimerCallback_Printer( TimerHandle_t xTimer )
 {
   xTimerStop( xTimers[2], 0 );   
   if(('A'<=aPrinterBuffer[uPrinterID][uLengDataPrinter[uPrinterID]]) && (aPrinterBuffer[uPrinterID][uLengDataPrinter[uPrinterID]]<='Y'))bLowcase=TRUE;//xMessageButton.cMessageValue
   if(bMoveCursor==FALSE)
   {
     uLengDataPrinter[uPrinterID]++;
     uBuffLeng[uPrinterID]++;   
     LCD_MoveCursor(uLengDataPrinter[uPrinterID]/20+1,uLengDataPrinter[uPrinterID]%20 ); 
   }
   bStopTimer=TRUE;
   uColumn=0;
 } 

void vTimerCallback_ClearCode( TimerHandle_t xTimer )
 {
     xTimerStop( xTimers[3], 0 );
     bStartTimer=FALSE;
     uCntC=0;
     cKEY=0;
     
    if(uDataLeng>0)
    {
      uDataLeng--;
      
      if(uLengTphan>0)
      {
        uLengTphan=uLengTphan-2;
        if(uLengTphan==0xFF)
        {
          bHaveDot=FALSE;
          uLengTphan=0;
        }
      }
      Change_Values(Mode,aCode);  
      if(uDataLeng==0)
      {
         LAPIS_ChangeValue(aCode[uCntPcode],uCntScode,setValue(Mode,aCode[uCntPcode],uCntScode,0,0,2),sConfiguration.DecimalPlace.Volume,getSizeFieldDataChange(aCode[uCntPcode],uCntScode));  //sConfiguration.DecimalPlace.Volume
      }
    }       
    if(uProcessCodeLeng>0)
    {
      bSelectCode=TRUE;
      uProcessCodeLeng--;
      if(uProcessCodeLeng>0)DirrectCode=aBuffKey[0]-0x30;
      sfEnterNewCode(DirrectCode,uProcessCodeLeng);
    }  
 } 

void PRINTER_WaitPassword(u8 lengPW)
{
  u8 i=0;
  LCD_Clear();
  LCD_DisplayFollowLanguage( 0,3,0,3,"CAI DAT MAY IN", "SET UP PRINTER");
  LCD_DisplayFollowLanguage( 1,0,1,0,"Mat khau:", "Password:");
  for(i=0;i<lengPW;i++)
  {
    LCD_Puts(1,9+i,"*");
  }
}

void PRESET_SendSelect(bool AorV)
{
  bool Invalid=TRUE;
  if((sEnablePreset.bNumber==TRUE)&&(sEnablePreset.bP1_4==TRUE))
  {
   if(PRESET_CheckValid(uPresetNum,uPresetDecimalLeng,AorV)==TRUE)
   {
        PRESET_SendValue(cKEY);  
        Invalid=FALSE;
   }
  if(Invalid==TRUE)
  {
    LCD_MsgPresetInvalid(AorV);  
  }
  bPressDot=FALSE;
  uPresetDecimalLeng=0;              
  uDataLeng=0;
  uPresetNum=0;
  bClearKeyX=FALSE;
  }   
}
void LCD_MsgPresetInvalid(bool AorV)
{
  char message1[30],message2[30];
  LCD_Default();  
  if(sConfiguration.UnitPrice!=0)
  {
    if(AorV==TRUE)
    {
      strcpy(message1,"So tien khong hop le");      
      //LCD_DisplayFollowLanguage( 2,0,2,0,"So tien khong hop le", "Invalid Amount"); 
    }

    else
    {
      strcpy(message1,"So lit khong hop le");
      //LCD_DisplayFollowLanguage( 2,0,2,0,"So lit khong hop le", "Invalid Volume");
    }     
    strcpy(message2,"Invalid Amount");
  }
  else
  {
   //LCD_DisplayFollowLanguage( 2,0,2,0,"Loi! Don gia bang 0", "Error! UP is zero"); 
    strcpy(message1,"Loi! Don gia bang 0");
    strcpy(message2,"Error! UP is zero");
  }  
  LCD_DisplayFollowLanguage( 2,0,2,0,(int8_t*)message1,(int8_t*) message2); 
}

void vLcdTask(void * pvParameters)
{
  char    abuffer[15];
  bool    bGoToReadMode=TRUE;  
  u8      aCodeReadMode[Size_Array_ReadMode]={1,2,3,8};
  xQueueMessage   xMessageButton;
  LCD_Initial();
//  sConfiguration.DecimalPlace.Amount=0;
//  sConfiguration.DecimalPlace.Volume=3;
//  sConfiguration.DecimalPlace.UnitPrice=0;
  //LCD_Test();
  while(1)
  {
    if((bKeypadEnable==TRUE) && (bSinalPFL==FALSE))                                                              
    {
      if(Mode==SUNNYXE_PRESET) // pumpsetting on main display
      {
        u8  cntTotal=0;        
        do
        {
          if(xQueueReceive(xKeypadQueue, &xMessageButton,(TickType_t)2))
          {
            //bEnableLCDStatusDisplay=TRUE;
            cKEY=xMessageButton.cMessageValue;              
            /*Keys P1-P4*/
            if((cKEY==':' )||(cKEY=='/') ||(cKEY==',') ||(cKEY=='-'))
            {
              if((sEnablePreset.bNumber==TRUE)&&(sConfiguration.KeypadSetting.Condition==PRESET_AVAILABLE))
              {
                LCD_Default();
                PRESET_SendP1234(cKEY);
              }
            }
           /*Key send volume preset or print*/
            else if((cKEY=='$'))
            {
              if(bPrinterEnable==FALSE)
              {               
                 PRESET_SendSelect(TRUE);
              }                
              /*Wait Nzzl Hang status to send print cmd*/
              else //if((sEnablePreset.bNzzlHang==TRUE))//||(WaitReceivedNzzlHang()==TRUE)
              {
                  /*Send cmd Printer*/
                  DataSetup_t dt;
                  dt.code=Code_Request_Printer;dt.Index[0]='@'-0x30;
                  PRINTER_WaitForSendDone(&dt); 
              }
            }
            /*Key send volume preset*/
            else if(cKEY=='L')
            {   
               PRESET_SendSelect(FALSE);
            }
            /*Key '.'*/
            else if(cKEY==' ') 
            {
              /*If the Numbers are Pressed then enable press Dot button*/
              if((sEnablePreset.bNumber==TRUE)&&(sEnablePreset.bP1_4==TRUE)&&(bPressDot==FALSE)&&(uDataLeng>=1)&&(bClearKeyX==TRUE))
              {
                bPressDot=TRUE;
                LCD_Puts(2,6+uDataLeng,".");   
              }            
            }  
            /*Key read total*/
            else if(cKEY=='S' )
            {
              uDataLeng=0;
              //num->leng=0;
              bPressDot=FALSE;
              //num->Dot=FALSE;
              DataSetup_t data;
              data.code=Code_Read_Total;
              if((sEnablePreset.bNumber==TRUE)&&(sEnablePreset.bP1_4==TRUE))
              {
                if((sEnablePreset.bNzzlHang==TRUE))//||(WaitReceivedNzzlHang()==TRUE)
                {
                  LCD_Default();
                  if(SUNNYXE_ReadTotal_Logs(data)==TRUE)
                  {
                    if(cntTotal==0)
                    {
                      LCD_ReadTotal(cntTotal,sConfiguration.Totalizer.amount,sConfiguration.Totalizer.volume/pow(10,sConfiguration.DecimalPlace.Volume));//sConfiguration.DecimalPlace.Volume
                      cntTotal=1;
                    }
                    else if(cntTotal==1)
                    {                      
                      LCD_ReadTotal(cntTotal,sConfiguration.DailyTotal.amount,sConfiguration.DailyTotal.volume/pow(10,sConfiguration.DecimalPlace.Volume));//sConfiguration.DecimalPlace.Volume
                      cntTotal=0;
                    }                      
                  } 
                }             
              }
            }    
            /*Key add three zero number 000*/
            else if(cKEY=='.' )/*'F'=>'.'*/
            {
              if((sEnablePreset.bNumber==TRUE)&&(sEnablePreset.bP1_4==TRUE)&&(sConfiguration.KeypadSetting.Condition==PRESET_AVAILABLE))
              {
                  uPresetNum=stringToInt(aBuffKey,uDataLeng); 
                  LCD_Default(); 
                  if((uPresetNum<=9999) && (uPresetNum>0) && (uPresetDecimalLeng==0) )
                  {
                    if(uDataLeng<=5 && (sEnablePreset.bP1_4==TRUE))
                    {
                      aBuffKey[uDataLeng]='0';
                      aBuffKey[uDataLeng+1]='0';
                      aBuffKey[uDataLeng+2]='0';
                      uDataLeng+=3;
                      uPresetNum*=1000;            
                      LCD_DisplayNumber(uPresetNum,2,6);
                    }
                    
                  }
                  else
                  {
                    uPresetDecimalLeng=0;                 
                    uDataLeng=0;   
                    LCD_DisplayFollowLanguage( 2,2,2,3,"So khong hop le", "Invalid Number");               
                  } 
                  bPressDot=FALSE;     
              }
            }            
            else
            {
              aBuffKey[uDataLeng++]=cKEY;
              if((cKEY>='0')&&(cKEY<='9'))
              {
                if((sEnablePreset.bP1_4==TRUE)&&(sConfiguration.KeypadSetting.Condition==PRESET_AVAILABLE))
                {
                  if(sEnablePreset.bNumber==TRUE)
                  {
                    bPrinterEnable=FALSE;
                    bClearKeyX=TRUE;
                    LCD_Default();
                    if((uDataLeng>Size_Number_Preset) ||(uPresetDecimalLeng==3)||(uPresetNum==0 && (bPressDot==FALSE)))//sConfiguration.DecimalPlace.Volume
                    {
                      aBuffKey[0]=cKEY;
                      uDataLeng=1;  
                      uPresetDecimalLeng=0;
                      bPressDot=FALSE;
                    }
                    uPresetNum=stringToInt(aBuffKey,uDataLeng);                    
                    if(bPressDot==TRUE )
                    {
                      if( uPresetDecimalLeng<sConfiguration.DecimalPlace.Volume)//sConfiguration.DecimalPlace.Volume
                      {
                        uPresetDecimalLeng++;
                        sprintf(abuffer,"%.3f",(float)uPresetNum/pow(10,uPresetDecimalLeng));    
                      }                           
                    }
                    else
                    {
                        sprintf(abuffer,"%lu",uPresetNum);                
                    }
                    LCD_Puts(2,6,(int8_t*)abuffer);
                  }
                  else
                  {
                    Reset_Buffer_Keypad();
                  }                                              
                }
              }
              else if((sEnablePreset.bNumber==TRUE) &&(sEnablePreset.bP1_4==TRUE))
              {
                  if(cKEY=='X')
                  {
                    /*the key is pressed*/
                    if((bClearKeyX==TRUE) && (uDataLeng>0))
                    {
                      aBuffKey[--uDataLeng]=0;
                      
                    }
                    else
                    {                 
                      TIMER4_ENABLE(15000);                      
                      if(memcmp(aBuffKey,"XXXXX",5)==0)
                      {
                        DataSetup_t data;
                        data.code=Code_Login_Printer;
                        /*Wait send code request data printer to done*/
                        if(WaitTransmitDone(&data,TRUE)==TRUE)    
                        {
                           Mode=SUNNYXE_PRINT; 
                           bExitsErrcode=FALSE;
                           TIMER4_DISABLE();
                        }                             
                        Reset_Buffer_Keypad();                        
                      }
                    }
                  }
                  else if(cKEY=='C')
                  {
                    if((uDataLeng>1) && (bClearKeyX==TRUE))
                    {
                      uDataLeng=uDataLeng-2;
                      uPresetNum=stringToInt(aBuffKey,uDataLeng); 
                      LCD_Default();
                      if(bPressDot==TRUE )
                      {
                        if(uPresetDecimalLeng>0)uPresetDecimalLeng--;
                        if(uPresetDecimalLeng==2)
                        {
                          sprintf(abuffer,"%.2f",(float)uPresetNum/100);                       
                        }
                        else if(uPresetDecimalLeng==1)
                        {
                          sprintf(abuffer,"%.1f",(float)uPresetNum/10);                           
                        }
                        else
                        {
                          bPressDot=FALSE;
                          sprintf(abuffer,"%u",uPresetNum);                         
                        } 
                      }
                      else
                      {
                        if(uPresetNum>0)
                          sprintf(abuffer,"%u",uPresetNum);
                        else
                        {
                          bClearKeyX=FALSE;
                          memset(abuffer,0,15);//sizeof(abuffer)                          
                        }                          
                      } 
                      LCD_Puts(2,6,(int8_t*)abuffer); 
                    }
                    else if(uDataLeng<=1)
                    {
                      cntTotal=0;
                      uDataLeng=0;
                      memset(abuffer,0,15);//sizeof(abuffer)
                      LCD_Default();                      
                    }
                    /*Check XXCXC and XCXC code*/
                    if((memcmp(aBuffKey,"XXCXC",5)==0))
                    {                       
                      /*send data code to pos*/                      
                      DataSetup_t data;
                      data.code=Code_Login_Mode;
                      if(WaitTransmitDone(&data,TRUE)==TRUE)    
                      {
                        bExitsErrcode=FALSE;
                        uCntC=0;
                        bGoToReadMode=TRUE;                                     
                        Mode=SUNNYXE_CODE;    
                        LCD_Default();       // print "Peco" in preset lcd
                        LAPIS_DisplayCode(); // print CodE in col|row = 0|2
                        TIMER4_DISABLE();
                      }                                             
                      Reset_Buffer_Keypad();
                    }                      
                  }
                  else if(uDataLeng==5) // why do max input buffer size has to be 5 ?????
                  {                  
                    Reset_Buffer_Keypad();
                  }                    
               }       
            }     
          }
        }while(Mode==SUNNYXE_PRESET);					
      }
      else if(Mode==SUNNYXE_CODE) // pumpsetting check pass
      {
        do
        {
          if(xQueueReceive(xKeypadQueue, &xMessageButton,(TickType_t)2))
          {                                         
            //bEnableLCDStatusDisplay=TRUE;
            cKEY=xMessageButton.cMessageValue;
            if(cKEY=='X')
            {
              uCntPcode=0;
              uCntScode=0;
              if(bGoToReadMode==TRUE)
              {
                Mode=SUNNYXE_READ;
                goto NHAN;
              }
              else
              {
                uDataLeng=0;
                if((atoi((char*)aBuffKey)==sConfiguration.UserPassword)||(atoi((char*)aBuffKey)==Pass_Admin)||(atoi((char*)aBuffKey)==Pass_Oil)||(strcmp((char*)aBuffKey,Pass_Peco)==0))
                {   
                  if(atoi((char*)aBuffKey)==sConfiguration.UserPassword)    Mode=SUNNYXE_USER;
                  else if(atoi((char*)aBuffKey)==Pass_Admin)                Mode=SUNNYXE_ADMIN;
                  else if(atoi((char*)aBuffKey)==Pass_Oil)                  Mode=SUNNYXE_OILCOMP;
                  else if(strcmp((char*)aBuffKey,Pass_Peco)==0)             Mode=SUNNYXE_PECO;
                  LAPIS_WaitCodeState();	
                }
                else 
                {
                  memset(aBuffKey,0,15);//sizeof(aBuffKey)
                  LAPIS_WaitPasswordState(uDataLeng);		
                } 
              }
            }
            else if(cKEY=='C')
            {
              LAPIS_CheckCCC();
            }
            else if((cKEY>='0')&&(cKEY<='9'))
            {
              if(uDataLeng<8)
              {
                bGoToReadMode=FALSE;
                aBuffKey[uDataLeng++]=cKEY;                            
              }
              else
              {
                memset(aBuffKey,0,15);//sizeof(aBuffKey)
                aBuffKey[0]=cKEY;
                uDataLeng=1;
              }
              LAPIS_WaitPasswordState(uDataLeng);   
            }        
          }
        }while(Mode==SUNNYXE_CODE);
      }  
      else if(Mode==SUNNYXE_PRINT) // printersetting
      {
        cKEY=0;
         bool   bGoPrinterMode=FALSE;
         bool   bReceivedDataPrinter=FALSE;
         u8   uIDFont=1;      
         u8 i;
         u8 hang=0,cot=0;
        uPrinterID=0;
        DataSetup_t data;   
        if(xQueueReceive(xPrinterMsgQueue,&sPrinterRxData,(TickType_t)5))
        {
          memcpy(&aPrinterBuffer[0][0],sPrinterRxData.data,Size_Name);
          memcpy(&aPrinterBuffer[1][0],&sPrinterRxData.data[Size_Name],Size_Address);
          memcpy(&aPrinterBuffer[2][0],&sPrinterRxData.data[Size_Name+Size_Address],Size_KieuCB);
          memcpy(&aPrinterBuffer[3][0],&sPrinterRxData.data[Size_Name+Size_Address+Size_KieuCB],Size_Kyhieu);
          memcpy(&aPrinterBuffer[4][0],&sPrinterRxData.data[Size_Name+Size_Address+Size_KieuCB+Size_Kyhieu],Size_Serial);
          memcpy(&aPrinterBuffer[5][0],&sPrinterRxData.data[Size_Name+Size_Address+Size_KieuCB+Size_Kyhieu+Size_Serial],Size_Nhienlieu);
          memcpy(&aPrinterBuffer[6][0],&sPrinterRxData.data[Size_Name+Size_Address+Size_KieuCB+Size_Kyhieu+Size_Serial+Size_Nhienlieu],Size_TenCty);
          memcpy(&aPrinterBuffer[7][0],&sPrinterRxData.data[Size_Name+Size_Address+Size_KieuCB+Size_Kyhieu+Size_Serial+Size_Nhienlieu+Size_TenCty],Size_Dthoai);
          for(i=0;i<8;i++)
          {              
            uLengDataPrinter[i]=strlen((char*)&aPrinterBuffer[i][0]);
            uBuffLeng[i]=uLengDataPrinter[i];
          }
          bReceivedDataPrinter=TRUE;
        }
        if(bReceivedDataPrinter==TRUE)
        {
            bLowcase=FALSE;
            bReceivedDataPrinter=FALSE;
            /*blink text lcd*/
            bBlinkControl=TRUE;
            LCD_BlinkOnOff(0x0E);
            PRINTER_WaitPassword(0);
            do
            {
              if(xQueueReceive(xKeypadQueue, &xMessageButton,(TickType_t)2))
              {
                if(bGoPrinterMode==FALSE)
                {
                  if((xMessageButton.cMessageValue<='9') && (xMessageButton.cMessageValue>='0'))
                  {
                    aBuffKey[uDataLeng++]=xMessageButton.cMessageValue;
                  }
                  if(atoi((char*)aBuffKey)==Pass_Printer)
                  {
                     bGoPrinterMode=TRUE;                       
                     LCD_ChangeInfo(aPrinterBuffer[uPrinterID],uPrinterID,0,uIDFont);
                  } 
                  else
                  {
                    if(uDataLeng>=8)
                    {
                      Reset_Buffer_Keypad();
                    }
                    else
                    {
                      if(xMessageButton.cMessageValue=='C')
                      {
                        if(uDataLeng>0)aBuffKey[--uDataLeng]=0;
                      }
                    }
                    PRINTER_WaitPassword(uDataLeng);                     
                  }  
                }
                else
                {
                  if(xMessageButton.cMessageValue=='L' ||(xMessageButton.cMessageValue=='$' &&(uBuffLeng[uPrinterID]<uLengDataPrinter[uPrinterID])))
                  {
                     if((aPrinterBuffer[uPrinterID][uBuffLeng[uPrinterID]-2]==' ' && xMessageButton.cMessageValue=='L' )||(aPrinterBuffer[uPrinterID][uBuffLeng[uPrinterID]]==' ' && xMessageButton.cMessageValue=='$' ))bLowcase=FALSE;
                     else bLowcase=TRUE;
                     bMoveCursor=TRUE;
                  }                      
                  if(((xMessageButton.cMessageValue>='0' && xMessageButton.cMessageValue<='9')||(xMessageButton.cMessageValue==' ')||(xMessageButton.cMessageValue==':')
                     ||(xMessageButton.cMessageValue=='/')||(xMessageButton.cMessageValue==',')||(xMessageButton.cMessageValue=='-')||(xMessageButton.cMessageValue=='.'))&& (uLengDataPrinter[uPrinterID]<=59))
                  {
                    if(cKEY!=xMessageButton.cMessageValue)//|| column++==3
                    {
                      uColumn=0;
                      cKEY=xMessageButton.cMessageValue;                
                    }
                    if( xTimerStart( xTimers[2], 0 ) == pdPASS )
                    {
                      bStopTimer=FALSE;
                      switch(xMessageButton.cMessageValue)
                      {
                        case ' ':
                        case ':':  
                        case '/':
                        case ',':
                        case '-':
                        case '.': 
                          aPrinterBuffer[uPrinterID][uLengDataPrinter[uPrinterID]]=xMessageButton.cMessageValue;
                          if(xMessageButton.cMessageValue ==' ')bLowcase=FALSE;
                          break;
                        default:
                          if(uIDFont==1)
                          {
                            if(bLowcase==FALSE)
                            {
                              aPrinterBuffer[uPrinterID][uBuffLeng[uPrinterID]]=Font_AA[cKEY-'0'][uColumn];       
                            }
                            else
                            {
                               aPrinterBuffer[uPrinterID][uBuffLeng[uPrinterID]]=Font_aa[cKEY-'0'][uColumn];  
                            }                              
                          }
                          else if(uIDFont==2)
                          {
                              aPrinterBuffer[uPrinterID][uBuffLeng[uPrinterID]]=Font_AA[cKEY-'0'][uColumn];
                          }
                          else if(uIDFont==3)
                          {
                              aPrinterBuffer[uPrinterID][uBuffLeng[uPrinterID]]=Font_aa[cKEY-'0'][uColumn];
                          }                           
                           break;
                      }
                      if(uColumn++==3)uColumn=0;
                    }                     
                  }    
                 /*Change Font*/
                  /*ID=1 --> Aa
                    ID=2 --> AA
                    ID=3 --> aa
                  */
                  else if(xMessageButton.cMessageValue=='S')
                  {
                    uIDFont++;
                    if(uIDFont==4)
                    {
                      uIDFont=1;                       
                    }                     
                  }
                  /*Shift left Control*/
                  else if(xMessageButton.cMessageValue=='L')
                  {
                    if(uBuffLeng[uPrinterID]>0)
                    {
                      uBuffLeng[uPrinterID]--;                   
                    }                      
                  }
                  /*Shift Right Control*/
                  else if(xMessageButton.cMessageValue=='$')
                  {
                    if(uBuffLeng[uPrinterID]<uLengDataPrinter[uPrinterID])
                    {                    
                      uBuffLeng[uPrinterID]++;                  
                      if( uBuffLeng[uPrinterID]==uLengDataPrinter[uPrinterID])
                      {
                        bMoveCursor=FALSE;
                      }
                    }
                  }                            
                  else if(xMessageButton.cMessageValue=='X')
                  {
                    if(bStopTimer==FALSE)
                    {
                      vTimerCallback_Printer(xTimers[2]);
                    }  
                    bMoveCursor=FALSE;
                    data.code=Code_SendData_Printer;
                    memset(aBuffKey,0,15);//sizeof(aBuffKey)
                    memset(data.dataArr,0,Size_Name);
                    memcpy(data.dataArr,aPrinterBuffer[uPrinterID],uLengDataPrinter[uPrinterID]);
                    uBuffLeng[uPrinterID]=uLengDataPrinter[uPrinterID];
                    if(uPrinterID==0||uPrinterID==1)
                    {
                      data.leng_data=Size_Name;                
                    }
                    else if(uPrinterID==6)data.leng_data=Size_TenCty; 
                    else if(uPrinterID==7)data.leng_data=Size_Dthoai; 
                    else data.leng_data=Size_KieuCB;
                    PRINTER_WaitForSendDone(&data);
//                    if(WaitTransmitDone(&data,TRUE)==TRUE) 
//                    {
//                      LCD_Default(); 
//                    }
                    if(++uPrinterID==8)uPrinterID=0;
                    if((aPrinterBuffer[uPrinterID][0]==0) ||(aPrinterBuffer[uPrinterID][uLengDataPrinter[uPrinterID]-1]==' '))bLowcase=FALSE;
                    else bLowcase=TRUE;
                  }
                  else if(xMessageButton.cMessageValue=='C')
                  {                   
                    xTimerStop( xTimers[2], 0 );
                    if(uBuffLeng[uPrinterID]>0)
                    {
                       for(i=(uBuffLeng[uPrinterID]-1);i<uLengDataPrinter[uPrinterID];i++)
                      {
                         aPrinterBuffer[uPrinterID][i]=aPrinterBuffer[uPrinterID][i+1];
                      }
                      uBuffLeng[uPrinterID]--;
                      uLengDataPrinter[uPrinterID]--;   
                    }
                    if((uBuffLeng[uPrinterID]==0)||(aPrinterBuffer[uPrinterID][uBuffLeng[uPrinterID]-1]==' '))bLowcase=FALSE;
                    else bLowcase=TRUE;
                  }  
                  LCD_ChangeInfo(aPrinterBuffer[uPrinterID],uPrinterID,uLengDataPrinter[uPrinterID],uIDFont); 
                  hang=uBuffLeng[uPrinterID]/20+1;
                  cot=uBuffLeng[uPrinterID]%20;                  
                  if(bMoveCursor==FALSE)
                  {
                    if(uLengDataPrinter[uPrinterID]<59)
                    {
                      if((uLengDataPrinter[uPrinterID]%20) >19)
                      {
                         cot=0;
                      }
                    }
                    else
                    {
                       hang=3;
                       cot=19;
                    }                      
                  }
                  LCD_MoveCursor(hang,cot);  
                } 
              }
            }
            while(Mode==SUNNYXE_PRINT);             
        }
      }
      if(Mode==SUNNYXE_READ)
      {
        do
        {
          if(xQueueReceive(xKeypadQueue, &xMessageButton,(TickType_t)2))
          {                  
            DataSetup_t data_print;
            cKEY=xMessageButton.cMessageValue;
            if(cKEY=='X')
            {
              NHAN:            
              LAPIS_DisplaySetup(SUNNYXE_READ,aCodeReadMode[uCntPcode],++uCntScode);//sConfiguration.DecimalPlace.Volume
              /*read log*/
              if(uCntPcode==3)
              {
                DataSetup_t data_read;
                /*Send code ask log*/
                data_read.code=Code_Read_Logs;
                if(SUNNYXE_ReadTotal_Logs(data_read)==TRUE)
                {
                  data_print.Index[0]=uCntScode-1;
                  LCD_ReadLog(uCntScode);
                }
              }
              if(uCntScode== sumSubcode(aCodeReadMode[uCntPcode],Mode))
              {   
                data_print.Index[0]=9;               
                uCntScode=0;
                uCntPcode++;
                if(uCntPcode==Size_Array_ReadMode)
                {
                  uCntPcode=0;
                }
                else data_print.Index[0]=0;
              }
            }
            else if(cKEY=='$')
            {
              if((uCntPcode==3)||(data_print.Index[0]==9))
              {
                /*Send code print log*/
                data_print.code=Code_Request_Printer;
                if(WaitTransmitDone(&data_print,TRUE)==TRUE) 
                {                   
                }
              }
            }
            else if(cKEY=='C')
            {  
              LAPIS_CheckCCC();
            }
          }
        }while(Mode==SUNNYXE_READ);  
      }
      else if(Mode==SUNNYXE_USER ||Mode==SUNNYXE_ADMIN||Mode==SUNNYXE_OILCOMP)
      {
        u8  sumScode=0; 
        do
        {
          if(xQueueReceive(xKeypadQueue, &xMessageButton,(TickType_t)2))
          {
            //bEnableLCDStatusDisplay=TRUE;
            cKEY=xMessageButton.cMessageValue;
            if(cKEY=='X')
            {
              bReview=TRUE;
              bSelectCode=FALSE;
              if(eTypeRead_Select==SELECT && SUNNYXE_CheckValidEnterCode(Mode,aCode,DirrectCode)==TRUE)
              {
                bFlagValidEnterCode=TRUE;
                uProcessCodeLeng=0; 
              }
              if((uProcessCodeLeng==1) && (eTypeRead_Select==SELECT))bSelectCode=TRUE;
              if(bSaveData==TRUE)
              {
                if(SUNNYXE_SaveData(&sConfiguration,uValue,aCode[uCntPcode],uCntScode)==TRUE)
                {
                    eTypeRead_Select=READ;
                } 
                bSaveData=FALSE;
                bReadOnly=TRUE;
                uDataLeng=0;
                bHaveDot=FALSE;
                uLengTphan=0;
              }                  
              if(bFlagValidEnterCode==TRUE ||eTypeRead_Select==READ)sumScode=sumSubcode(aCode[uCntPcode],Mode);               
              if(eTypeRead_Select==READ)
              {                          
                if(uCntScode==sumScode )
                {    
                  //if(uCntScode>0)
                  {
                    uCntScode=0;         
                  }
                  uCntPcode++;           
                  if(((Mode==SUNNYXE_USER) && (uCntPcode>(Size_Array_UserMode-1)))||((Mode==SUNNYXE_ADMIN||Mode==SUNNYXE_OILCOMP)&&(uCntPcode>(Size_Array_AdminMode-1))))//repair 9,26
                  {
                     uCntPcode=0;           
                  }
                }    
                else
                {
                  uCntScode++;
                }    
                sumScode=sumSubcode(aCode[uCntPcode],Mode);
                if(sumScode>0)
                {
                  if(uCntScode==0) uCntScode++;  
                }
              }
              else if((eTypeRead_Select==SELECT) && (bFlagValidEnterCode==TRUE))
              {
                eTypeRead_Select=READ;
                if(sumScode>0)
                {
                  if(uCntScode==0) uCntScode++;  
                }          
              }
              if((bFlagValidEnterCode==TRUE) ||(eTypeRead_Select==READ))
               LAPIS_DisplaySetup(Mode,aCode[uCntPcode],uCntScode);//sConfiguration.DecimalPlace.Volume
             }
            else if(cKEY=='C')
            {
              uCntC++;
             if(bStartTimer==FALSE)
             {
               if( xTimerStart( xTimers[3], 0 ) == pdPASS )
               {
                 bStartTimer=TRUE;
               }               
             }                
              if(uCntC==3)
              {
                xTimerStop( xTimers[3], 0 );
                if(bStartTimer==TRUE)bStartTimer=FALSE;
                uCntC=0;
                uCntScode=0;
                uCntPcode=0;
                bFlagValidEnterCode=FALSE;
                bSaveData=FALSE;
                uProcessCodeLeng=0;
                uDataLeng=0;
                uLengTphan=0;
                bHaveDot=FALSE;
                bSelectCode=TRUE;
                /*restore decimal volume to origin value*/
                if(bReview==FALSE)
                {
                  uPresetValue=1;
                }
                LAPIS_WaitCodeState();
              }
            }
            /*Key '.'*/
            else if(cKEY==' ')
            {
              bHaveDot=TRUE;      
              if(aCode[uCntPcode]==24)
              {
                u8 i;
                u8 buff[15];
                if(uValue<=9999)
                {
                  Dots(0,sConfiguration.DecimalPlace.Volume,0);//sConfiguration.DecimalPlace.Volume
                  sfRow1(24,0); 
                  IntergerDigitsExtraction(buff,7,uValue*(uint64_t)(pow(10,sConfiguration.DecimalPlace.Volume))); //sConfiguration.DecimalPlace.Volume                 
                  LAPIS_DisplayNumber(uSegDigits[buff[6]]);
                  for( i=6;i>0;i--)
                  {
                    LAPIS_CheckNumberDisplay(buff,i,0);
                  }
                  LAPIS_DisplayNumber(uSegDigits[uCntScode-1]);
                  LAPIS_DisplayNumber(uSegDigits[15]);
                  LAPIS_ClearSegment(35);
                  LATCH();                
                }                
              }
            }
            else if(cKEY>='0' && cKEY<='9')
            {
                Change_Values(Mode,aCode);
            } 
            if(Mode==SUNNYXE_ADMIN||Mode==SUNNYXE_OILCOMP)
            {
              if(aCode[uCntPcode]==24)
              {
                u8 leng_tmp=0;
                if(cKEY=='L' )
                {
                  leng_tmp=sTypeValues.len_tp[uCntScode];
                  if(bHaveDot==TRUE) sTypeValues.len_tp[uCntScode]=uLengTphan;  
                  else 
                  {
                    sTypeValues.len_tp[uCntScode]=0;
                  }
                  if(PRESET_CheckValidVolume(uValue,&PRESET_TypeFlag[uCntScode-2],bHaveDot)==TRUE)
                  {
                    /*Volume*/
                    uPresetValue=1;                
                    if(SUNNYXE_SaveData24(&sConfiguration,uValue,uCntScode)==TRUE)
                    {                    
                      uCntScode++;   
                      bSave_Success=TRUE;
                      uValue=0;
                    }
                    else
                    {
                      sTypeValues.len_tp[uCntScode]=leng_tmp;//0;   
                    }
                    if(uCntScode==6)
                    {
                      uCntPcode++;
                      uCntScode=0;
                    }                  
                  }
                  else
                  {
                    sTypeValues.len_tp[uCntScode]=leng_tmp; 
                    bSave_Success=FALSE;
                    bSave_Fails=TRUE;                     
                  }
                }
                else if(cKEY=='$')
                {
                  if(PRESET_CheckValidAmount(uValue,&PRESET_TypeFlag[uCntScode-2],bHaveDot)==TRUE)
                  {
                      /*Amount*/
                      uPresetValue=0;
                    /*save value*/
                      if(SUNNYXE_SaveData24(&sConfiguration,uValue,uCntScode)==TRUE)
                      {  
                        uCntScode++;   
                        bSave_Success=TRUE;
                        uValue=0;
                      }                  
                      if(uCntScode==6)
                      {
                        uCntPcode++;
                        uCntScode=0;
                      }                                         
                    }
                    else
                    {
                      bSave_Success=FALSE;
                      bSave_Fails=TRUE;  
                    }                     
                  }
                  if(cKEY=='L'||cKEY=='$')
                  {
                    uDataLeng=0;
                    if(bHaveDot==TRUE) bHaveDot=FALSE;
                    LAPIS_DisplaySetup(Mode,aCode[uCntPcode],uCntScode);//sConfiguration.DecimalPlace.Volume
                  }
                }              
            }          
          } 
        }while(Mode==SUNNYXE_USER || Mode==SUNNYXE_ADMIN||Mode==SUNNYXE_OILCOMP);
      }              
      else if(Mode==SUNNYXE_PECO)
      {
        do
        {
          if(xQueueReceive(xKeypadQueue, &xMessageButton,(TickType_t)2))
          {
            //bEnableLCDStatusDisplay=TRUE;
            cKEY=xMessageButton.cMessageValue;
            if(cKEY=='X')
            {
              bChangecode_SunnyPeco=TRUE;
              if((eTypeRead_Select==SELECT) && (SUNNYXE_CheckValidEnterCode(Mode,aCodePeco,DirrectCode)==TRUE))
              {
                bFlagValidEnterCode=TRUE;
              }
              if(bSaveData==TRUE)
              {

                if(SUNNYXE_SaveData(&sConfiguration,uValue,aCodePeco[uCntPcode],uCntScode)==TRUE)
                {
                  eTypeRead_Select=READ;
                }
                //else eTypeRead_Select=SELECT;
                uDataLeng=0;
                bSaveData=FALSE;
              }
              if((bFlagValidEnterCode==TRUE) ||(eTypeRead_Select==READ))
              LAPIS_DisplaySetup(Mode,aCodePeco[uCntPcode],uCntScode); //sConfiguration.DecimalPlace.Volume
            }
            else if(cKEY=='C')
            {
              uCntC++;
              if(uCntC==3)
              {              
                uCntC=0;
                uCntPcode=0;              
                bSaveData=FALSE;
                uProcessCodeLeng=0;
                bChangecode_SunnyPeco=FALSE;   
                bFlagValidEnterCode=FALSE;
                /*restore decimal volume to origin value*/               
                LAPIS_WaitCodeState();
              }
            }
            else if(cKEY>='0' && cKEY<='9') 
            {
              Change_Values(Mode,aCodePeco);
            }
          }
        }while(Mode==SUNNYXE_PECO);
      }
      else if(Mode==SUNNYXE_FUELING)
      {
        do
        {
          if(xQueueReceive(xKeypadQueue, &xMessageButton,(TickType_t)5)) 
          {
            cKEY=xMessageButton.cMessageValue;      
            if((cKEY==':')||(cKEY=='/'))
            {
              if(cKEY==':')
              {
                cKEY='A';
              }
              else if(cKEY=='/') cKEY='B';
              FUELING_SendData(cKEY);
            }
          }
        }while(Mode==SUNNYXE_FUELING);	
      }
    }  
  vTaskDelay(5);
  }
}

void LAPIS_CheckCCC(void)
{
  SysStatus_t     _sysStatus;
  uCntC++;
  if(uCntC>=3)
  {
    uCntC=0;
    uCntScode=0;
    uCntPcode=0;
    Reset_Buffer_Keypad(); 
    LCD_Default();
    Mode=SUNNYXE_PRESET; 
    QueueSend_Err(&_sysStatus,'0','0');
  }      
}
void QueueSend_Err(SysStatus_t *sys,u8 code_chuc,u8 code_dv)
{
  if(sConfiguration.uDisplay_LastData==1)
  {
    sys->uPrice=sConfiguration.UAV.uUPLossPower;
    sys->uAmount=sConfiguration.UAV.uAmountLossPower;
    sys->uVolume=sConfiguration.UAV.uVolumeLossPower;
  }
  else
  {     
    sys->uPrice=sConfiguration.UnitPrice;
    sys->uAmount=0;
    sys->uVolume=0;
  }
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
    xQueueSend(xPosRxSysStatusQueue, sys,0);   

}
void LCD_ChangeInfo(char *infor,u8 index,u8 leng,u8 indexfont)
{
  char buffin[3][80]={0};
  char buf_VNM[30],buf_ENG[30];
  if(leng<60 )
  {
    LCD_Clear();
  } 
  switch(index)
  {
      case 0:    
       // LCD_DisplayFollowLanguage(0,0,0,0,"1.Ten:","1.Name:");
        strcpy(buf_VNM,"1.Ten:");
        strcpy(buf_ENG,"1.Name:");
        break;
      case 1:  
      //  LCD_DisplayFollowLanguage(0,0,0,0,"2.Dia Chi:","2.Address:");
        strcpy(buf_VNM,"2.Dia Chi:");
        strcpy(buf_ENG,"2.Address:");
        break;
      case 2:
       // LCD_DisplayFollowLanguage(0,0,0,0,"3.Kieu Cot Bom:","3.Type of Pump:");
        strcpy(buf_VNM,"3.Kieu Cot Bom:");
        strcpy(buf_ENG,"3.Type of Pump:");
        break;
      case 3:
       // LCD_DisplayFollowLanguage(0,0,0,0,"4.Ky hieu:","4.Symbol:");
        strcpy(buf_VNM,"4.Ky hieu:");
        strcpy(buf_ENG,"4.Symbol:");
        break;
      case 4:
       // LCD_DisplayFollowLanguage(0,0,0,0,"5.Seri:","5.Serial:");
        strcpy(buf_VNM,"5.Seri:");
        strcpy(buf_ENG,"5.Serial:");
        break;
      case 5:
       // LCD_DisplayFollowLanguage(0,0,0,0,"6.Loai NL:","6.Type of Fuel:");
        strcpy(buf_VNM,"6.Loai NL:");
        strcpy(buf_ENG,"6.Type of Fuel:");
        break;    
      case 6:
       // LCD_DisplayFollowLanguage(0,0,0,0,"7.Ten CTY:","7.Company:");
        strcpy(buf_VNM,"7.Ten CTY:");
        strcpy(buf_ENG,"7.Company:");
        break;  
      case 7:
        //LCD_DisplayFollowLanguage(0,0,0,0,"8.Dien Thoai:","8.Mobile:");
        strcpy(buf_VNM,"8.Dien Thoai:");
        strcpy(buf_ENG,"8.Mobile:");
        break;
  }
  LCD_DisplayFollowLanguage(0,0,0,0,(int8_t*)buf_VNM,(int8_t*)buf_ENG);
  if(indexfont==1)LCD_Puts(0,18,"Aa");
  else if(indexfont==2)LCD_Puts(0,18,"AA");
  else if(indexfont==3)LCD_Puts(0,18,"aa");
  if(leng<20)
  {
    LCD_Puts(1,0,(int8_t*)infor);
    //if(leng==19) lcd_write(0,0x80+0x14);
  }
  else if(leng<40)
  {
    memcpy(buffin[0],infor,20);
    memcpy(buffin[1],infor+20,leng-19);
    LCD_Puts(1,0,(int8_t*)buffin[0]);
    LCD_Puts(2,0,(int8_t*)buffin[1]);
    if(leng==39) LCD_Write(0,0x80+0x54);
  }
  else if(leng<60)
  {

    memcpy(buffin[0],infor,20);
    memcpy(buffin[1],infor+20,20);    
    memcpy(buffin[2],infor+40,leng-39);
    LCD_Puts(1,0,(int8_t*)buffin[0]);
    LCD_Puts(2,0,(int8_t*)buffin[1]);    
    LCD_Puts(3,0,(int8_t*)buffin[2]);
    //if(leng==59) lcd_write(0,0x80+0x54);
  }  
}
void LCD_DisplayFollowLanguage(u8 xv,u8 yv,u8 xe,u8 ye,int8_t *vietnamese,int8_t *english)
{
   if(sConfiguration.CountryCode==VietNam)
    LCD_Puts(xv,yv,vietnamese);
   else
    LCD_Puts(xe,ye,english);
}

bool WaitTransmitDone(DataSetup_t *dt,bool ReceiverOrSend)
{
  EventBits_t uxBits;
  xEventGroupClearBits(xPosReceiDataEventGroup,BIT_0);
  if(ReceiverOrSend==TRUE)
  {
    //xQueueSend(xPosSendDataQueue,dt,( TickType_t )5);//5 
    xQueueOverwrite(xPosSendDataQueue,dt);
  }  
  uxBits = xEventGroupWaitBits(
        xPosReceiDataEventGroup,   /* The event group being tested. */
          BIT_0,                /* The bits within the event group to wait for. */
          pdTRUE,               /* BIT_0 should be cleared before returning. */
          pdFALSE,              /* Don't wait for both bits, either bit will do. */
          (TickType_t)2000);/* Wait a maximum of 1500ms for either bit to be set. */   
  if((uxBits & BIT_0)==BIT_0) 
  {
    return TRUE;
  }
  return FALSE;
}
bool PRESET_CheckValidAmount(uint64_t value,volatile BOOLEAN *flag,bool bHaveDot)
{
  bool valid=TRUE;
  if(value<=9999999 && (value>0))//(value/sConfiguration.UnitPrice)>=1)
  {
    if(bHaveDot==TRUE)
    {
      valid=FALSE;
    }
    else if(bHaveDot==FALSE)
    {
      *flag=TRUE;
    }
  }
  else
  {
    valid=FALSE;
  }
  return valid;
}
bool PRESET_CheckValidVolume(uint64_t value,volatile BOOLEAN *flag,bool bHaveDot)
{
  bool valid=TRUE;
  if(sConfiguration.UnitPrice==0)return FALSE;
  if(bHaveDot==TRUE)
  {
    if(((value/pow(10,uLengTphan))<=(9999999/sConfiguration.UnitPrice))&&(value>0))//(value>=1)
    {
      /*flag save volume status*/
      *flag=FALSE;
    }
    else
    {
      valid=FALSE;
    }
  }
  else if(bHaveDot==FALSE)
  {
    if(((value<=9999)&&(value<=(9999999/sConfiguration.UnitPrice)))&&(value>0))//(value>=1)
    {
      *flag=FALSE;
    }
    else
    {
      valid=FALSE;
    }
  }
  return valid;
}
bool SUNNYXE_SaveData24(volatile SysConfig_t *config,uint64_t intValue,u8 cntScode)
{
  bool    saveEnable=FALSE;
  bool    saveDone=FALSE;
  DataSetup_t   data; 
  data.code=24;
  data.data64=intValue; 

  if((cntScode==2||cntScode==3||cntScode==4||cntScode==5))  
  {
    if(uPresetValue==0 ||uPresetValue==1)
    {
      saveEnable=TRUE; 
      data.AmountOrVolume=uPresetValue+1;
      if(uPresetValue==1)
      {
        data.leng_tp=uLengTphan;
      }
       else
        data.Index[1]=0;
    }
    data.Index[0]=cntScode-1;   
    data.leng_data=uDataLeng;
  } 
  if(saveEnable==TRUE)
  {    
    if(WaitTransmitDone(&data,TRUE)==TRUE)          
    {
      saveDone=TRUE;
      {
        if(uPresetValue==0)
        {
          PRESET_TypeFlag[cntScode-2]=bTRUE;
          config->KeypadSetting.OneTouch.PA[cntScode-2]=intValue;
          config->KeypadSetting.OneTouch.PV[cntScode-2]=0;
        }
        else if(uPresetValue==1)
        {
          PRESET_TypeFlag[cntScode-2]=bFALSE;
          sTypeValues.len_tp[cntScode]=uLengTphan;
          config->KeypadSetting.OneTouch.PV[cntScode-2]=intValue;
          config->KeypadSetting.OneTouch.PA[cntScode-2]=0;
          uLengTphan=0;
        }
      }
    }
  }         
 return saveDone;
}
bool SUNNYXE_SaveData( volatile SysConfig_t *config,uint64_t intValue,u8 pcode,u8 cntScode)
{
  u8 str[5];
  u8 i;
  bool    saveEnable=FALSE;
  bool    saveDone=FALSE;
  DataSetup_t   data; 
  data.code=pcode;
  data.data64=intValue;  
    
  switch(pcode)
  {
    case 12:
      if(cntScode==1)
      {
        if(config->DailyTotal.volume!=intValue)
        {
          saveEnable=TRUE;
          data.leng_tp=uLengTphan;
          data.AmountOrVolume=1;
        }
      }
      else
      {
         if(config->DailyTotal.amount!=intValue)
         {
          data.AmountOrVolume=2;
          saveEnable=TRUE;     
          data.leng_tp=0;
         }
      }      
      if(saveEnable==TRUE)
      {    

        if(WaitTransmitDone(&data,TRUE)==TRUE)
        {
          saveDone=TRUE;
          taskENTER_CRITICAL();
          if(cntScode==1)
          {
            config->DailyTotal.volume=intValue*pow(10,sConfiguration.DecimalPlace.Volume-uLengTphan); //sConfiguration.DecimalPlace.Volume
            sTypeValues.len_tp[0]=uLengTphan;
          }
          else if(cntScode==2)config->DailyTotal.amount=intValue; 
          taskEXIT_CRITICAL();
        }
      }      
      break;
    case 13:
      if(config->UnitPrice!=(UnitPrice_t)intValue){
        saveEnable=TRUE;
        data.leng_default=5; 
      }
      if(saveEnable==TRUE)
      {    

        if(WaitTransmitDone(&data,TRUE)==TRUE)
        {
          saveDone=TRUE;
          taskENTER_CRITICAL();
          config->UnitPrice=(u32)intValue;
          taskEXIT_CRITICAL();
          POS_LitmitSetup(config->UnitPrice,&fVolumeLimit); 
        }
      }       
      break;
    case 15:
      if(config->UserPassword!=(uint16_t)intValue){
        saveEnable=TRUE;   
        data.leng_default=4;      
      }
      if(saveEnable==TRUE)
      {    
        if(WaitTransmitDone(&data,TRUE)==TRUE)        
        {
          saveDone=TRUE;
          taskENTER_CRITICAL();
          config->UserPassword=(u32)intValue;
          taskEXIT_CRITICAL();
        }
      }      
      break;      
    case 16:
      if(config->FuelingLimit!=(double)intValue){
        saveEnable=TRUE;  
        data.leng_tp=uLengTphan;     
      }
      if(saveEnable==TRUE)
      {    

        if(WaitTransmitDone(&data,TRUE)==TRUE)        
        {
          saveDone=TRUE;
          taskENTER_CRITICAL();
          config->FuelingLimit=(double)intValue;
          taskEXIT_CRITICAL();
          sTypeValues.len_tp[1]=uLengTphan; 
        }
      }      
      break;      
    case 18:

     if(config->PosMode!=(ePosMode_t)intValue)
     {
      saveEnable=TRUE;
     }      
      data.leng_default=1;  
      if(saveEnable==TRUE)
      {    
        if(WaitTransmitDone(&data,TRUE)==TRUE)        
        {
          saveDone=TRUE;
          taskENTER_CRITICAL();
          config->PosMode=(ePosMode_t)intValue;
          taskEXIT_CRITICAL();
        }
      }         
        break;  
      case 19:
        if( config->CommID!=(u8)intValue){
          saveEnable=TRUE;         
          data.leng_default=2;           
        }
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
             taskENTER_CRITICAL();
            config->CommID=(u8)intValue;
            taskEXIT_CRITICAL();
          }
        }        
        break; 
      case 20:
        if( config->FuelType!= (eFuelTypeCode_t)intValue)
         {
           saveEnable=TRUE;  
         }
        data.leng_default=1;   
        if(saveEnable==TRUE)
        {    

          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();
            config->FuelType=(eFuelTypeCode_t)intValue;
            taskEXIT_CRITICAL();
          }
        }        
        break;  
      case 24:
        if(cntScode==1)
        {
          if(intValue==0)
          {
            if(config->KeypadSetting.Condition!=PRESET_UNAVAILABLE)
            {
              saveEnable=TRUE; 
              data.AmountOrVolume=0;
            }
          }     
          else if(intValue==3)
          {
            if(config->KeypadSetting.Condition!=PRESET_AVAILABLE)
            {        
              saveEnable=TRUE; 
              data.AmountOrVolume=3;
            }
          }          
        }
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();
            if(cntScode==1)config->KeypadSetting.Condition=(eKeypadCondition_t)intValue;   
            taskEXIT_CRITICAL();
          }
        }         
      break;
      case 30:
        switch((u32)intValue)
        {
          case 0:
          case 158: //taiwan  
          case 222: //Mianma
          case 344: //Hong kong
          case 356: //India
          case 360: //indonesia
          case 410: //Korea
          case 458: //malaysia          
          case 608: //philippines
          case 704: //VN
          case 764: //Thailand
          if(config->CountryCode!=(eCountryCode_t)intValue)
          {
            saveEnable=TRUE; 
          }  
          break;
        }  
        data.leng_default=3; 
        if(saveEnable==TRUE)
        {       
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();            
            config->CountryCode=(eCountryCode_t)intValue;
            taskEXIT_CRITICAL();
          }
        }                 
        break;
      case 32:
        if(config->FirstIndication!=(u32)intValue){
          saveEnable=TRUE;
        }
        data.leng_default=2; 
        if(saveEnable==TRUE)
        {      
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();            
            config->FirstIndication=(u8)intValue;
            taskEXIT_CRITICAL();
          }
        }          
        break; 
      case 33:
        if(config->PresetOverRunMasking!=(u32)intValue){
          saveEnable=TRUE;
        }
        data.leng_default=1;  
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
             taskENTER_CRITICAL();
            config->PresetOverRunMasking=(u8)intValue;
             taskEXIT_CRITICAL();
          }
        }          
        break; 
      case 36:
        if(config->AmountPresetMethod!=(u32)intValue){
          saveEnable=TRUE;
        }
        data.leng_default=1;    
        if(saveEnable==TRUE)
        {    

          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();
            config->AmountPresetMethod=(u8)intValue;
            taskEXIT_CRITICAL();
          }
        }        
        break;
     case 37:
       data.dataArr[0]=config->DecimalPlace.Amount;data.dataArr[1]=config->DecimalPlace.Volume;data.dataArr[2]=config->DecimalPlace.UnitPrice;
       if(config->DecimalPlace.Amount!=aDecimalBuffer[0])
       {
          saveEnable=TRUE;
          data.dataArr[0]=aDecimalBuffer[0];        
       }
       if(config->DecimalPlace.Volume!=aDecimalBuffer[1])
       {
          saveEnable=TRUE;
          data.dataArr[1]=aDecimalBuffer[1];
       }   
       if(config->DecimalPlace.UnitPrice!=aDecimalBuffer[2])
       {
          saveEnable=TRUE;
          data.dataArr[2]=aDecimalBuffer[2];
       } 
       if(saveEnable==TRUE)
       {  
         data.code=37;
         if(WaitTransmitDone(&data,TRUE)==TRUE)          
         {
           saveDone=TRUE;
           taskENTER_CRITICAL();
           if(config->DecimalPlace.Amount!=aDecimalBuffer[0])config->DecimalPlace.Amount=aDecimalBuffer[0]; 
           if(config->DecimalPlace.Volume!=aDecimalBuffer[1])
           {
             config->DecimalPlace.Volume=aDecimalBuffer[1];
            //  if(Mode==SUNNYXE_ADMIN)
            //    bChangeDecimalAdminMode=TRUE;
           }
           if(config->DecimalPlace.UnitPrice!=aDecimalBuffer[2])config->DecimalPlace.UnitPrice=aDecimalBuffer[2];    
//            if(Mode==SUNNYXE_OILCOMP)
//            {
//              sDecimal.Amount=config->DecimalPlace.Amount;
//              sDecimal.UnitPrice=config->DecimalPlace.UnitPrice;
//              sDecimal.Volume=config->DecimalPlace.Volume;
//            }
           taskEXIT_CRITICAL();
         }
       }         
       break;       
      case 41:
        if((config->PresetSlowdownPosition.F[cntScode-1]!=intValue)&&(intValue>=1)&&(intValue<=20)){
          saveEnable=TRUE;      
          data.AmountOrVolume=cntScode;
        }        
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
            {
              saveDone=TRUE;
              taskENTER_CRITICAL();              
              config->PresetSlowdownPosition.F[cntScode-1]= (u8)intValue;     
              taskEXIT_CRITICAL();
            }
        }         
        break; 
      case 44:
        if(config->SlowdownTimeForPulseStop!=(u32)intValue){
          saveEnable=TRUE;
          data.leng_default=2;           
        } 
        if(saveEnable==TRUE)
        {     
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
            {
              saveDone=TRUE;
              taskENTER_CRITICAL();              
              config->SlowdownTimeForPulseStop=(u8)intValue;
              taskEXIT_CRITICAL();
            }
        }            
        break; 
      case 45:
        if(config->PumpLockTimeAfterUPriceChange!=(u32)intValue){
          saveEnable=TRUE;
          data.leng_default=1;            
        }
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
            {
              saveDone=TRUE;
              taskENTER_CRITICAL();    
              config->PumpLockTimeAfterUPriceChange=(u8)intValue;
              taskEXIT_CRITICAL();
            }
        }        
        break;   
      case 46:
        if(config->PumpLockTimeForPulseStop!=(u32)intValue)
        {
          saveEnable=TRUE;
          data.leng_default=2;         
        }
        if(saveEnable==TRUE)
        {       
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
            {
              saveDone=TRUE;
              taskENTER_CRITICAL();    
              config->PumpLockTimeForPulseStop=(u8)intValue;
              taskEXIT_CRITICAL();
            }
        }         
        break; 
      case 47:   

        if(config->FuelingMode!=(eFuelingMode_t)intValue)
        {
          saveEnable=TRUE;
        }
        data.leng_default=1;   
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();   
            config->FuelingMode=(eFuelingMode_t)intValue;
            taskEXIT_CRITICAL();
          }
        }         
        break; 
      case 48:
        if( config->CommTimeout!=(u32)intValue){
          saveEnable=TRUE;
          data.leng_default=2;            
        }
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)          
            {
              saveDone=TRUE;
              taskENTER_CRITICAL();   
              config->CommTimeout=(u8)intValue;
              taskEXIT_CRITICAL();
            }
        }          
        break;   
      case 63:
        if(config->FuelingCount!=(u32)intValue){
          saveEnable=TRUE;    
          data.leng_default=10;          
        }
        if(saveEnable==TRUE)
        {     
          if(WaitTransmitDone(&data,TRUE)==TRUE)             
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();
            config->FuelingCount=(u32)intValue;
            taskEXIT_CRITICAL();
          }
        }        
        break;        
      case 64:
        if(config->PowerOnOffCount!=(u32)intValue){
          saveEnable=TRUE;
          data.leng_default=6;          
        }
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)              
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();
            config->PowerOnOffCount=(u32)intValue;
            taskEXIT_CRITICAL();
          }
        }         
        break;    
        /*Pcode Mode SUNNYXE_PECO*/
      case 70:
        if(config->PosVersion!=(u32)intValue)
        {
          saveEnable=TRUE;
          data.leng_default=1;
        }
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)              
          {
            saveDone=TRUE;
            config->PosVersion=(u32)intValue;
          }
        }          
        break;
      case 80:
      case 29:
        if((config->DisplayTest!=(u32)intValue)||(config->CoPassword!=intValue))
        {
          saveEnable=TRUE;
          if(pcode==29)
          data.leng_default=8;   
          else  data.leng_default=1;   
        }
        if(saveEnable==TRUE)
        {    
          if(WaitTransmitDone(&data,TRUE)==TRUE)              
          {
            saveDone=TRUE;
            taskENTER_CRITICAL();
            if(pcode==80)
              config->DisplayTest=(u8)intValue;
            else
              config->CoPassword=(u8)intValue;
             taskEXIT_CRITICAL();
          }
        }         
        break;      
      case 95:
        if(uDataLeng>=4)
        {
          memcpy(str,aBuffKey,4); 
          data.data_calender[0]=stringToInt(str,4);
          data.data_calender[1]=config->Calendar.month;
          data.data_calender[2]=config->Calendar.date;
          data.data_calender[3]=config->Calendar.hour;
          data.data_calender[4]=config->Calendar.minutes;          
         saveEnable=TRUE;          
        }
        for(i=3;i<7;i++)
        {
          if(uDataLeng>=(2*i-1))
          {
            if((uDataLeng-2*(i-1))==1){
             memcpy(str,aBuffKey+2*(i-1),1);
             data.data_calender[i-2]=stringToInt(str,1);
            }
            else{
              memcpy(str,aBuffKey+2*(i-1),2);
              data.data_calender[i-2]=stringToInt(str,2);          
            
            }   
          }
        }  

        if(saveEnable==TRUE)
        {     
          if(WaitTransmitDone(&data,TRUE)==TRUE)              
          {
            saveDone=TRUE;
            if(uDataLeng<=4)
            {
              config->Calendar.year= data.data_calender[0]; 
            }
            else if(uDataLeng<=6)
            {
              config->Calendar.year= data.data_calender[0];
              config->Calendar.month= data.data_calender[1];            
            }
            else if(uDataLeng<=8)
            {
              config->Calendar.year= data.data_calender[0];
              config->Calendar.month= data.data_calender[1];                          
              config->Calendar.date= data.data_calender[2];   
            }
            else if(uDataLeng<=10)
            {
              config->Calendar.year= data.data_calender[0];
              config->Calendar.month= data.data_calender[1];                          
              config->Calendar.date= data.data_calender[2];                
              config->Calendar.hour= data.data_calender[3];                  
            }
            else if(uDataLeng<=12)
            { 
              config->Calendar.year= data.data_calender[0];
              config->Calendar.month= data.data_calender[1];                          
              config->Calendar.date= data.data_calender[2];                
              config->Calendar.hour= data.data_calender[3];               
              config->Calendar.minutes= data.data_calender[4];                 
            }            
          }
        }         
        break;             
        
  }
return saveDone;
}

void Change_Values(eLoginMode_t mode,u8 arr[])
{
  /*Select code*/
  if( bSelectCode==TRUE && bChangecode_SunnyPeco==FALSE)//just clear CCC;uCntPcode==0
  { 
    /*
    *  - get input data as processcode
    *  - convert string buffer into interget value
    *  - jump to setting corressponding getting processcode
    */
    if(uProcessCodeLeng<2)
    {
      eTypeRead_Select=SELECT;
      aBuffKey[uProcessCodeLeng++]=cKEY; 
      if(uProcessCodeLeng==1)
      {
        DirrectCode=aBuffKey[0]-0x30;
        sfEnterNewCode(DirrectCode,uProcessCodeLeng);                
      }
      else if(uProcessCodeLeng==2)
      {
          DirrectCode=(aBuffKey[0]-0x30)*10+aBuffKey[1]-0x30;
          sfEnterNewCode(DirrectCode,uProcessCodeLeng);  
          if(Mode==SUNNYXE_PECO)bChangecode_SunnyPeco=TRUE;
      }      
    }
  }
  /*Change values*/
  else
  {
    if(bReadOnly==FALSE) //enable write
    {
      bSaveData=TRUE;
      if(Mode==SUNNYXE_PECO)
        Display_DataChange(aCodePeco[uCntPcode],uCntScode,mode,&uValue,bHaveDot,&sTypeValues);  
      else         
        Display_DataChange(aCode[uCntPcode],uCntScode,mode,&uValue,bHaveDot,&sTypeValues);  
    }      
  }   
}
void sfEnterNewCode(u8 newcode,u8 len)
{
  Row3(uSegDigits[0],uSegDigits[0],0,0);
  if(len==1)
  {
    LAPIS_DisplayNumber(uSegDigits[newcode%10]);   
    LAPIS_ClearSegment(91);
  }

  else if(len==2)
  {
    LAPIS_DisplayNumber(uSegDigits[newcode%10]);   
    LAPIS_DisplayNumber(uSegDigits[newcode/10]);  
    LAPIS_ClearSegment(84);
  }
  else if(len==0)
  {
    LAPIS_ClearSegment(98);
  }
  LATCH();
}
u8 SUNNYXE_FindCode(u8 lengArr,u8 arr[],u8 code)
{
  u8 i;
  u8 value=0xFF;
   for(i=0;i<lengArr;i++)
   {
      if(arr[i]==code)
      {
        value=i;
        uCntScode=0;
        break;
      }
   }
   return value;
}
bool SUNNYXE_CheckValidEnterCode(eLoginMode_t mode,u8 arr[],u8 code)
{
  uCntPcode=0;
  u8 value;
  if((mode==SUNNYXE_ADMIN)||(mode==SUNNYXE_OILCOMP))
  { 
   value=SUNNYXE_FindCode(Size_Array_AdminMode,arr,code);//29
  }
  else if(mode==SUNNYXE_USER)
  {    
    value=SUNNYXE_FindCode(Size_Array_UserMode,arr,code);
  }
  else if(mode==SUNNYXE_PECO)
  {
    value=SUNNYXE_FindCode(Size_Array_PecoMode,arr,code);
  }
  if(value!=0xFF)
  {
    uCntPcode=value;
    return TRUE;
  }
  return FALSE;
}
void Display_DataChange(u8 code,u8 cntScode,eLoginMode_t mode,uint64_t *intValue,bool bDot, volatile TypeValue_t   *tValue)
{
  u8 i;
  u8 dot=3;
  TypeValue_t value;
  uint64_t nguyen=0;
  u8 str[2]={0};
  u8 choosePreset=0;
  u8 sizeField=0;
  uint64_t   uvalue=0;
  RangeData_t rangData;
  aBuffKey[uDataLeng]=cKEY;
  if(cKEY>='0' && cKEY<='9')
  {
    uDataLeng++;
  }  
  bReview=FALSE;
  
  if(code==12||code==16 )
  {
    rangData=getRangeDataNeedChange(code,dot,cntScode);
    sizeField=getSizeFieldDataChange(code,cntScode);   
   if(uDataLeng<=sizeField)
   {
     bReview=FALSE;       
      if(bDot==FALSE)
      {
        if((code==16 && uDataLeng>4)||(code==12 && ((uDataLeng>7 && cntScode==1)||(uDataLeng>10 && cntScode==2)) ))
        {
          {
            nguyen=0;   
            aBuffKey[0]=cKEY; 
            uDataLeng=1;           
          }
        }         
        for(i=0;i<uDataLeng;i++)
        {
          nguyen =nguyen*10+aBuffKey[i]-0x30;
        }       
        *intValue=nguyen;
      }
      else
      {
        if(cntScode==1)
        {
          if(uLengTphan<dot)
          {
            uLengTphan++;
            for(i=0;i<uDataLeng;i++)
            {
              nguyen =nguyen*10+aBuffKey[i]-0x30;
            }                  
          }
          else
          {
            uLengTphan=0;
            aBuffKey[0]=cKEY;
            nguyen=cKEY-0x30;
            uDataLeng=1;
            bHaveDot=FALSE;
          }              
        }
       else
        {
          bHaveDot=FALSE;
          nguyen=0;
          uDataLeng=0; 
        }
        *intValue=nguyen;                
      }
    }
    else
    {
      uLengTphan=0;            
      nguyen=cKEY-0x30;
      aBuffKey[0]=cKEY;
      *intValue=cKEY-0x30;
      bHaveDot=FALSE;            
      uDataLeng=1;
    }        
   if((*intValue>=rangData.minNum) && (*intValue<=rangData.maxDbNum))
   {
      value=setValue(mode,code,cntScode,*intValue,uDataLeng,choosePreset);       
      LAPIS_ChangeValue(code,cntScode,value,dot,sizeField);                       
   }   
  }
  else if(code==24)
  {
    if(bSave_Success==TRUE||bSave_Fails==TRUE )
    {
      if(bSave_Success==TRUE)bSave_Success=FALSE;
      if(bSave_Fails==TRUE)bSave_Fails=FALSE;
      uDataLeng=1;
      nguyen=0;
      aBuffKey[0]=cKEY;
      uLengTphan=0;
    }
    if(cntScode>1)
    {
      choosePreset=2;
    }
    if(uDataLeng<8)
    {
      for(i=0;i<uDataLeng;i++)
      {
        nguyen =nguyen*10+aBuffKey[i]-0x30;
      }
      *intValue=nguyen;
      if(bHaveDot==TRUE)
      {
        if((uDataLeng-uLengTphan) <=5 &&(uDataLeng>0))
        {
          uLengTphan++;  
          if(uLengTphan>dot)
          {
            uLengTphan=0;
            nguyen=0;
            uDataLeng=1;
            *intValue=cKEY-0x30;
            aBuffKey[0]=cKEY;    
            bHaveDot=FALSE;
          }                   
        }       
        else /* Dot is not Invalid*/
        {
          bHaveDot=FALSE;
        }
      }
    }
    else
    {
      uDataLeng=1;
      uLengTphan=0;
      nguyen=0;
      *intValue=cKEY-0x30;
      aBuffKey[0]=cKEY;
      if(bHaveDot==TRUE) bHaveDot=FALSE;
    }
    value=setValue(mode,code,cntScode,*intValue,uDataLeng,choosePreset); 
    LAPIS_ChangeValue(code,cntScode,value,dot,sizeField);     
  
  }
  else if(code==95)
  {
    if(uDataLeng>12)
    {
      uDataLeng=1;
    }
    if(uDataLeng<=4)
    {
      for(i=0;i<uDataLeng;i++)
      {
        uvalue=uvalue*10+aBuffKey[i]-0x30;
      }        
    }
    else if(uDataLeng<=6)
    {
       memcpy(str,aBuffKey+4,uDataLeng-4);  
      for(i=0;i<uDataLeng-4;i++)
      {
        uvalue=uvalue*10+str[i]-0x30;
      }        
    }
    else if(uDataLeng<=8)
    {
       memcpy(str,aBuffKey+6,uDataLeng-6);  
      for(i=0;i<uDataLeng-6;i++)
      {
        uvalue=uvalue*10+str[i]-0x30;
      }       
    }
    else if(uDataLeng<=10)
    {
       memcpy(str,aBuffKey+8,uDataLeng-8);  
      for(i=0;i<uDataLeng-8;i++)
      {
        uvalue=uvalue*10+str[i]-0x30;
      }       
    }
    else if(uDataLeng<=12)
    {
       memcpy(str,aBuffKey+10,uDataLeng-10);  
      for(i=0;i<uDataLeng-10;i++)
      {
        uvalue=uvalue*10+str[i]-0x30;
      }
    }    
      value=setValue(mode,code,cntScode,(u32)uvalue,uDataLeng,choosePreset); 
      LAPIS_ChangeValue(code,cntScode,value,dot,sizeField);   
  }
  else 
  {
    if(bHaveDot==TRUE)bHaveDot=FALSE;
    rangData=getRangeDataNeedChange(code,dot,cntScode);
    sizeField=getSizeFieldDataChange(code,cntScode);
    if(uDataLeng<=sizeField)
    {
      for(i=0;i<uDataLeng;i++)
      {
        uvalue +=(aBuffKey[i]-0x30)*pow(10,uDataLeng-1-i);
      }
       *intValue=uvalue;
        if(uvalue==0)
        {
          uDataLeng=0;
        }
    }
    else
    {

      uDataLeng=0;
      aBuffKey[uDataLeng++]=cKEY;
      *intValue=cKEY-0x30;
      uvalue=cKEY-0x30;
      if(*intValue==0)
      {
        uDataLeng--;
      }
    }
     if((*intValue>=rangData.minNum) && (*intValue<=rangData.maxDbNum))
     {
        value=setValue(mode,code,cntScode,*intValue,uDataLeng,choosePreset);   
        if(code==41)dot=1;
        LAPIS_ChangeValue(code,cntScode,value,dot,sizeField);               
     }          
  }
}

void TIM4_IRQHandler( void )
{ 
  static u8  cnt_TM4=0;
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {   
    cnt_TM4++;
    if(cnt_TM4==2)
    {
      cnt_TM4=0;
      Reset_Buffer_Keypad();
      TIMER4_DISABLE();
    }
    TIM_SetCounter(TIM4, 0U);
  }
  TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}
/*Enable enter Key in 2s*/
void TIMER4_ENABLE(u32 Period)
{
  TM_TIMER4_Init(Period);
  TIM_SetCounter(TIM4, 0U);  
  TIM_Cmd(TIM4, ENABLE);   
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
}
void TIMER4_DISABLE(void)
{
      TIM_Cmd(TIM4, DISABLE);   
      TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
}
void Reset_Buffer_Keypad(void)
{
      uDataLeng=0;
      memset(aBuffKey,0,15);
}
void TM_TIMER4_Init(u32 Period)
{
    TIM_TimeBaseInitTypeDef     TIM_BaseStruct;
    NVIC_InitTypeDef            nvicStructure;

    /* Enable clock for TIM3 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
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
    TIM_BaseStruct.TIM_Prescaler = 7199;
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
    /* Initialize TIM4 */
    TIM_TimeBaseInit(TIM4, &TIM_BaseStruct);
    /* Start count on TIM4 */
    TIM_Cmd(TIM4, DISABLE);    

    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);

    /* Enable interrupt */
    nvicStructure.NVIC_IRQChannel = TIM4_IRQChannel;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
    nvicStructure.NVIC_IRQChannelSubPriority = 0;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

