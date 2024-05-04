/**
  ******************************************************************************
  * Project		: PC9
  * File Name           : function.c
  * Author 	        : Nguyen Tran Duy
  * Start	        : 15/12/2016
  * Stop                : xx/12/2017
  * Version		: 1.7
  ******************************************************************************
  * Description:
  *
  *
  ******************************************************************************
  */
#include        "FreeRTOS.h"
#include        "task.h"
#include        "lapis.h"
#include        "math.h"

                            /* 0 ,  1,   2,   3,   4,   5,   6,   7,   8,   9,   A,   F    L,   C,   E,   P,   b,   d*/
u8	        uSegDigits[18]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x71,0x38,0x39,0x79,0x73,0x7c,0x5e};
                            /* 0 ,  1,   2,   3,   4,   5,   6,   7,   8,   9,              C,   d,   E*/
u8       uSegErrorList[15]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0xFF,0xFF, 0x39,0x5E,0x79};
volatile TypeValue_t                     sTypeValues;
volatile u8                     uPresetValue=2;
extern  volatile SysConfig_t             sConfiguration;
volatile u8                      aDecimalBuffer[3];
extern  volatile  u8            uLengTphan;
extern  u8                      uCntScode;
extern volatile u8              uLengCalender;
extern  volatile BOOLEAN        PRESET_TypeFlag[4];
extern  volatile bool           bErrcode;
extern  bool                    bReview;
extern  volatile bool           bReadOnly;
extern volatile bool            bHaveDot;
extern          volatile eLoginMode_t     Mode;
void    LAPIS_DisplayTest(u8 j )
{
  u8 i;
  LAPIS_Clear();
  if(j<10)
  {
    for(i=0;i<21;i++)
    {
      LAPIS_DisplayNumber(uSegDigits[j]);
    }
  }
  else
  {
    for(i=0;i<24;i++)
    {
      LAPIS_DisplayNumber(uSegDigits[8]);
    } 
  }
  LATCH(); 
}
void	Split_Digit(u32 num,u8 *arr,u8 leng)
{
  IntergerDigitsExtraction(arr,leng,num);
}

u8 Split_Number(uint64_t x,u8 *arr)
{
  u8 i,leng=0;
  char str[15]={0};
  sprintf(str,"%llu",x);
  leng=strlen(str);
  for(i=0;i<leng;i++)
  {
    *arr++=(str[i]-0x30);
  }
  return leng;
}


void LAPIS_BusyState(void)
{
  LAPIS_Clear();
  LAPIS_ClearSegment(63);  
  LAPIS_DisplayNumber(0x6E);
  LAPIS_DisplayNumber(0x6D);
  LAPIS_DisplayNumber(0x3E);
  LAPIS_DisplayNumber(0x7C);   
  LAPIS_ClearSegment(56);  
  LATCH();
}
void 	LAPIS_WaitCodeState(void)
{
  LAPIS_Clear();  
  LAPIS_DisplayNumber(uSegDigits[0]);
  LAPIS_DisplayNumber(uSegDigits[0]);
  LAPIS_ClearSegment(105);
  LATCH();
}

void	LAPIS_WaitPasswordState(u8 leng)
{
  u8	i;
  LAPIS_DisplayNumber(0x79); // 'E'
  LAPIS_DisplayNumber(0x5E); // 'd'
  LAPIS_DisplayNumber(0x5C); // 'o'
  LAPIS_DisplayNumber(0x39); // 'C'
  LAPIS_ClearSegment(21);    // reserve 21 digits
  for(i=leng;i>0;i--)
  {
    LAPIS_DisplayNumber(0x40); // '-'
  }
  LAPIS_ClearSegment(98-7*leng);
  LATCH();
}

void Dots(u8 r1_dot,u8 r2_dot,u8 r3_dot)
{
    u8 i,temp1, temp2, temp3;
    temp1 = r1_dot == 0 ? 0 : r1_dot + 6;
    temp2 = r2_dot == 0 ? 0 : r2_dot + 3;
    temp3 = r3_dot == 0 ? 0 : r3_dot + 0;
    for(i=1;i<=9;i++) 
    {
      if((i == temp1) || (i == temp2) || (i == temp3))
      {
        DATA_LCD_HIGH;
      }
      else
      {
        DATA_LCD_LOW;
      }
      SHIFT();
    }
}
void sfDots(u8 n)
{
  u8 i,digit;
  digit=0x40;
  for(i=0;i<n;i++) 
  {
    if(((digit&0x40)==0x40) || (i == 4) || (i == 7))//0x40
    {
      DATA_LCD_HIGH;
    }
    else
    {
       DATA_LCD_LOW;
    }
    digit<<=1;
    SHIFT();
  }
}
//void    LAPIS_DISPLAY_NEW(SysStatus_t *status,SDisplay_t *displayStatus, DecimalPlaces_t *decimal,eFuelingMode_t fuelMode)//u8 Dot,
//{
//u8 i;
//  LAPIS_Clear();
//  Dots(decimal->Volume);
//  if((fuelMode==NORMAL)||(fuelMode== TRAINING)||(fuelMode==  ACTUAL))
//  {
//    /*unit price*/
//    for(i=0;i<status->uLeng[0];i++)//arr[0]=status->uLeng[0]
//    {
//      LAPIS_DisplayNumber(uSegDigits[status->uArray_UP[4-i]]);
//    }
//    for(i=0;i<Unitprice_MaxLeng-status->uLeng[0];i++)LAPIS_ClearSegment(7); 
//  }
//  /*Flow rate Display*/
//  else if(fuelMode==FLOW_RATE_DISPLAY)
//  {
//    LAPIS_DisplayNumber(uSegDigits[12]);
//    LAPIS_DisplayNumber(uSegDigits[status->uFlowrate%10]);//sProcessStatus.uFlowrate%10
//    LAPIS_DisplayNumber(uSegDigits[status->uFlowrate/10]);//sProcessStatus.uFlowrate/10
//    LAPIS_ClearSegment(14);    
//  }
//  if(bErrcode==TRUE)
//  { 
//    LAPIS_DisplayNumber(uSegErrorList[status->errCode.Code[1]-0x30]);   //errCode.Code[1]
//    if(status->errCode.Code[0]=='d')
//    { 
//      LAPIS_DisplayNumber(uSegErrorList[13]);       
//    }
//    else if(status->errCode.Code[0]=='E')
//    {  
//      LAPIS_DisplayNumber(uSegErrorList[14]);       
//    }
//    else
//    {   
//      LAPIS_DisplayNumber(uSegErrorList[ status->errCode.Code[0]-0x30]);  
//    }
//  }
//   /*Clear Error*/
//  else LAPIS_ClearSegment(14);
//  /*Volume*/ 
//  for(i=0;i<decimal->Volume+1;i++)
//  {
//    LAPIS_DisplayNumber(uSegDigits[displayStatus->uArray_VL[6-i]]);
//  }
//  if(displayStatus->uLeng[1]>(decimal->Volume+1))
//  {
//    for(i=decimal->Volume+1;i<displayStatus->uLeng[1];i++)LAPIS_DisplayNumber(uSegDigits[displayStatus->uArray_VL[6-i]]);//arr[1]
//    for(i=0;i<Volume_MaxLeng-displayStatus->uLeng[1];i++)LAPIS_ClearSegment(7); 
//  }
//  else
//  {
//    for(i=0;i<6-decimal->Volume;i++)LAPIS_ClearSegment(7); 
//  }
//  /*Amount*/
//  for(i=0;i<displayStatus->uLeng[2];i++)
//  {
//    LAPIS_DisplayNumber(uSegDigits[displayStatus->uArray_AM[6-i]]);
//  }
//  for(i=0;i<Amount_MaxLeng-displayStatus->uLeng[2];i++)LAPIS_ClearSegment(7); 
//
//  LATCH();
//}
//
void    LAPIS_DISPLAY(SysStatus_t *status,eFuelingMode_t fuelMode)//,volatile SysConfig_t *config
{
  u8 i;
  LAPIS_Clear();
  Dots(sConfiguration.DecimalPlace.Amount,sConfiguration.DecimalPlace.Volume,sConfiguration.DecimalPlace.UnitPrice);// change

  if((fuelMode==NORMAL)||(fuelMode== TRAINING)||(fuelMode==  ACTUAL))
  {
    /*unit price*/
    for(i=0;i<status->uLeng[0];i++)//arr[0]=status->uLeng[0]
    {
      LAPIS_DisplayNumber(uSegDigits[status->uArray_UP[4-i]]);
    }
    for(i=0;i<Unitprice_MaxLeng-status->uLeng[0];i++)LAPIS_ClearSegment(7); 
  }
  /*Flow rate Display*/
  else if(fuelMode==FLOW_RATE_DISPLAY)
  {
    LAPIS_DisplayNumber(uSegDigits[12]);
    LAPIS_DisplayNumber(uSegDigits[status->uFlowrate%10]);
    LAPIS_DisplayNumber(uSegDigits[status->uFlowrate/10]);
    LAPIS_ClearSegment(14);    
  }
  if(bErrcode==TRUE)
  { 
    //if((status->errCode.Code[0]-0x30)!=0)
    {
      LAPIS_DisplayNumber(uSegErrorList[status->errCode.Code[1]-0x30]);   //errCode.Code[1]
      if(status->errCode.Code[0]=='d')
      { 
        LAPIS_DisplayNumber(uSegErrorList[13]);       
      }
      else if(status->errCode.Code[0]=='E')
      {  
        LAPIS_DisplayNumber(uSegErrorList[14]);       
      }
      else
      {   
        LAPIS_DisplayNumber(uSegErrorList[ status->errCode.Code[0]-0x30]);  
      }
    }  
//    else
//      LAPIS_ClearSegment(14);
  }
   /*Clear Error*/
  else LAPIS_ClearSegment(14);
  /*Volume*/ 
  for(i=0;i<sConfiguration.DecimalPlace.Volume+1;i++)//sConfiguration.DecimalPlace.Volume+1
  {
    LAPIS_DisplayNumber(uSegDigits[status->uArray_VL[6-i]]);
  }
  if(status->uLeng[1]>(sConfiguration.DecimalPlace.Volume+1))//(sConfiguration.DecimalPlace.Volume+1)
  {
    for(i=sConfiguration.DecimalPlace.Volume+1;i<status->uLeng[1];i++)LAPIS_DisplayNumber(uSegDigits[status->uArray_VL[6-i]]);//sConfiguration.DecimalPlace.Volume+1
    for(i=0;i<Volume_MaxLeng-status->uLeng[1];i++)LAPIS_ClearSegment(7); 
  }
  else
  {
    for(i=0;i<(6-sConfiguration.DecimalPlace.Volume);i++)LAPIS_ClearSegment(7); //6-sConfiguration.DecimalPlace.Volume
  }
  /*Amount*/
  // for(i=0;i<status->uLeng[2];i++)
  // {
  //   LAPIS_DisplayNumber(uSegDigits[status->uArray_AM[6-i]]);
  // }
  for(i=0;i<sConfiguration.DecimalPlace.Amount+1;i++)
  {
    LAPIS_DisplayNumber(uSegDigits[status->uArray_AM[6-i]]);
  }
  if(status->uLeng[2]>(sConfiguration.DecimalPlace.Amount+1))//(sConfiguration.DecimalPlace.Amount+1)
  {
    for(i=sConfiguration.DecimalPlace.Amount+1;i<status->uLeng[2];i++)LAPIS_DisplayNumber(uSegDigits[status->uArray_AM[6-i]]);//sConfiguration.DecimalPlace.Volume+1
    for(i=0;i<Amount_MaxLeng-status->uLeng[2];i++)LAPIS_ClearSegment(7); 
  }else
  {
    for(i=0;i<(6-sConfiguration.DecimalPlace.Amount);i++)LAPIS_ClearSegment(7); //6-sConfiguration.DecimalPlace.Volume
  }
  // for(i=0;i<Amount_MaxLeng-status->uLeng[2];i++)LAPIS_ClearSegment(7); 
  LATCH();
}

void 	LAPIS_Clear(void)
{
  LAPIS_ClearSegment(160);
  LATCH();
}

void LAPIS_DisplayCode(void)
{
  LAPIS_Clear();
  LAPIS_DisplayNumber(0x79);
  LAPIS_DisplayNumber(0x5E);
  LAPIS_DisplayNumber(0x5C);
  LAPIS_DisplayNumber(0x39);
  LAPIS_ClearSegment(119);
  LATCH();
}
void LAPIS_DisplayNumber(u8 num)
{
  u8 i;
  for(i=0;i<7;i++)
  {
    if((num&0x40)==0x40)
    {
        DATA_LCD_HIGH;
    }
    else
    {
        DATA_LCD_LOW;
    }
    num<<=1;
    SHIFT();
  }
}

void LATCH(void)
{
  LATCH_HIGH;
  LATCH_LOW;  
}
void SHIFT(void)
{
  SHIFT_CLOCK_HIGH;
  SHIFT_CLOCK_LOW;
}
void 	LAPIS_Init(void)
{
  BLANK_PIN_HIGH;
  TEST_PIN_HIGH;
  vTaskDelay(500);
  TEST_PIN_LOW;
  BLANK_PIN_LOW;
  LAPIS_Clear();
}
u8      sumSubcode(u8 pcode,eLoginMode_t mode)
{
  switch(pcode)
  {
    case 1:
    case 2:
    case 11:
    case 12:
    case 99:
      return 2;
  case 8:
      return 10;
    case 3:
    case 4:
    case 13:
    case 14:
    case 16:
    case 20:
    case 21:      
    case 46:      
    case 63:
    case 79:      
      return 1;
    case 15:
    case 18:
    case 19:
    case 29:
    case 30:
    case 32:      
    case 33: 
    case 36:
    case 43:
    case 44:
    case 45:
    case 47:
    case 60:
    case 62:
    case 64:  
    case 65:
    case 66:
    case 67:
    case 69:
    case 80:
    case 90:
    case 95:        
      return 0;
  case 37:
    return 3;
    case 24: 
        return 5;
  case 41:
      return 6;
  }
    return 0;
}

void    LAPIS_DisplaySetup(eLoginMode_t mode,u8 pcode,u8 cntSub)
{
  TypeValue_t value;
  TypeSubCode_t tScode;
  bool valueDbOrInt=TRUE;
  bReadOnly=FALSE;
  value=getValue(pcode,&sConfiguration,cntSub);
  tScode=getSubcode(pcode,cntSub);
  LAPIS_Clear();
  if(mode==SUNNYXE_READ)
  {
    if(pcode!=3) valueDbOrInt=FALSE;
    bReadOnly=TRUE;    
    fDisplay(pcode,tScode,value,valueDbOrInt,getSizeFieldDataChange(pcode,cntSub));             
  }
  else if(mode==SUNNYXE_USER)
  {
    switch(pcode)
    {

      case 11:
      case 12: 
      case 13:
      case 15:
      case 16:         
      case 18:  
      case 19:
      case 20:
      case 21:
      case 29:        
        if(pcode==11||pcode==12||pcode==16)valueDbOrInt=FALSE;         
        if(pcode==11||pcode==16||pcode==19||pcode==20||pcode==21)bReadOnly=TRUE; //||pcode==21
        fDisplay(pcode,tScode,value,valueDbOrInt,getSizeFieldDataChange(pcode,cntSub));        
        break;
      case 24:          
        bReadOnly=TRUE;  
        fDisplay24(pcode,tScode,value,cntSub,uPresetValue);
        break;
    }
  }
  else if(mode==SUNNYXE_ADMIN||mode==SUNNYXE_OILCOMP)
  {
    switch(pcode)
    {
      case 11:          
      case 12:
      case 13:
      case 15: 
      case 16:        
      case 18:            
      case 19:        
      case 20:
      case 21: 
      case 29: 
      case 30:    
      case 32:         
      case 33:
      case 36:
      case 44:
      case 45:        
      case 46:      
      case 47: 
      case 48:          
      case 63:
      case 64:         
      case 80:     
      case 90:    
        if(pcode==11||pcode==12||pcode==16)valueDbOrInt=FALSE;//double
        if(pcode==11||pcode==12||pcode==21||((pcode==32||pcode==33||pcode==45||pcode==46)&& (mode==SUNNYXE_ADMIN))||pcode==90||
        (mode==SUNNYXE_OILCOMP && (pcode==11||pcode==15||pcode==16||pcode==19||pcode==20)))//||pcode==21
        { 
          bReadOnly=TRUE;
        }
        fDisplay(pcode,tScode,value,valueDbOrInt,getSizeFieldDataChange(pcode,cntSub));        
        break;
       case 24: 
        fDisplay24(pcode,tScode,value,cntSub,uPresetValue);
        break; 
      case 37:
//        if(pcode==37 && (mode==SUNNYXE_ADMIN))
//        {
          // bReadOnly=TRUE;//if(pcode==37 && (mode==SUNNYXE_ADMIN))
//        }
        aDecimalBuffer[0] = sConfiguration.DecimalPlace.Amount;
        aDecimalBuffer[1] = sConfiguration.DecimalPlace.Volume;
        aDecimalBuffer[2] = sConfiguration.DecimalPlace.UnitPrice;
        fDisplay37(cntSub);
      break;
      case 41:
      case 99:
        if (pcode==99)bReadOnly=TRUE;
        valueDbOrInt=FALSE;
        fDisplay(pcode,tScode,value,valueDbOrInt,getSizeFieldDataChange(pcode,cntSub));
        break;
      case 95:
        fDisplay95(pcode,value);
      break;       
    }    
  }
  else if(mode==SUNNYXE_PECO)
  {
    if(pcode==70)
    {
      fDisplay(pcode,tScode,value,valueDbOrInt,getSizeFieldDataChange(pcode,cntSub)); 
    }
  }
}
TypeValue_t   setValue( eLoginMode_t mode,u8 pcode,u8 cntSub,double value,u8 len,u8 choosePreset)//eLoginMode_t mode,
{
  TypeValue_t tValue;
  u16 static year=0;
  u8 static month=0,date=0,hours=0; 
  tValue.db=0;tValue._u32=0;    
  if(mode==SUNNYXE_USER)
  {
    switch(pcode)
    {
      case 12:      
        if(cntSub==1)
        {
          tValue.db=(double)value;
          tValue.len_tp[0]=uLengTphan;
        }
        else if(cntSub==2)
        {
          tValue._u64=(uint64_t)value;      
        }
         break;
      case 13:
      case 15:
      case 18:
      case 29: 
         tValue._u32=(u32)value;      
         break;
      case 24:
        if(cntSub==1)
        {
          tValue._u32=(u32)value;    
        }
        else
        {
          if(choosePreset==0)
          {
            tValue._u32=(u32)value;        
          }
          else if(choosePreset==1)
          {
            tValue.db=value;      
          }
        }
        break;
      }
  }
  else if(mode==SUNNYXE_ADMIN||mode==SUNNYXE_OILCOMP)
  {
     switch(pcode)
      {
        case 13:
        case 15:
        case 18:
        case 19:
        case 20:
        case 29:
        case 30:
        case 32:
        case 33:
        case 36:
         
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 63:
        case 64:
        case 80:
           tValue._u32=(u32)value;    
           break;
        case 16:         
           tValue.db=value;  
           tValue.len_tp[1]=uLengTphan;
          break;
        case 41:
        case 99:          
           tValue.db=value;
           break;
      case 37: 
      {
          switch (cntSub)
          {
            case 1:
                aDecimalBuffer[0]=(u8)value;
                break;
            case 2:
                aDecimalBuffer[1]=(u8)value;
                break;
            case 3:
                aDecimalBuffer[2]=(u8)value;
              break;
            default:
              break;
          }
      }
      case 24:
      if(cntSub==1)
      {
        tValue._u32=(u32)value;      
      }
      else
      {
        if(choosePreset==0)
        {
          tValue._u32=(u32)value;        
        }
        else if(choosePreset==1)
        {
          tValue.db=value;     
        }
        else if(choosePreset==2)
        {
          if(bHaveDot==FALSE)
          {
            tValue._u32=(u32)value; 
             tValue.len_tp[cntSub-2]=0;     
          }
          else
          {
            tValue.len_tp[cntSub-2]=uLengTphan;            
            tValue.db=value;          
          }
        }
      }
      break;     
        case 95:
          if(len<=4)
          {
            year=(u16)value;
            tValue._u16[0]=(u16)value; 
            tValue._u16[1]=sConfiguration.Calendar.month;
            tValue._u16[2]=sConfiguration.Calendar.date;
            tValue._u16[3]=sConfiguration.Calendar.hour;
            tValue._u16[4]=sConfiguration.Calendar.minutes;            
          }
          else if(len<=6)
          {
            month=(u8)value;
            tValue._u16[0]=year; 
            tValue._u16[1]=(u8)value;
            tValue._u16[2]=sConfiguration.Calendar.date;
            tValue._u16[3]=sConfiguration.Calendar.hour;
            tValue._u16[4]=sConfiguration.Calendar.minutes;             
          }
          else if(len<=8)
          {
            date=(u8)value;
            tValue._u16[0]=year; 
            tValue._u16[1]=month;
            tValue._u16[2]=(u8)value;
            tValue._u16[3]=sConfiguration.Calendar.hour;
            tValue._u16[4]=sConfiguration.Calendar.minutes;     
          }   
          else if(len<=10)
          {
            hours=(u8)value;
            tValue._u16[0]=year; 
            tValue._u16[1]=month;
            tValue._u16[2]=date;
            tValue._u16[3]=(u8)value;
            tValue._u16[4]=sConfiguration.Calendar.minutes;     
          }   
          else if(len<=12)
          {
            tValue._u16[0]=year; 
            tValue._u16[1]=month;
            tValue._u16[2]=date;
            tValue._u16[3]=hours;
            tValue._u16[4]=(u8)value;     
          }               
      }       
  }
  else if(mode==SUNNYXE_PECO)
  {
   if(pcode==70)tValue._u32=(u32)value; 
  }
  return tValue;
}
void LAPIS_ChangeValue(u8 pcode,u8 cntSub,TypeValue_t tValue,u8 dot,u8 sizefield)//eLoginMode_t mode,
{
  bool valueDbOrInt;
  TypeSubCode_t tScode;
  tScode=getSubcode(pcode,cntSub);
  valueDbOrInt=TRUE;
  switch(pcode)
  {  
    case 12:    
    case 13:
    case 15:
    case 16:      
    case 18:
    case 19:
    case 20:
    case 24:      
    case 29:
    case 30:
    case 32:   
    case 33:
    case 36:
    //case 38:
    case 41:      
    case 44:
    case 45:
    case 46: 
    case 47:
    case 48:
    case 63:      
    case 64:
    case 70:      
    case 80:
    case 95:

      if(pcode==12||pcode==16||pcode==41||pcode==63||pcode==95)valueDbOrInt=FALSE;
        fDisplay(pcode,tScode,tValue,valueDbOrInt,sizefield);  //getSizeFieldDataChange(pcode,cntSub)
      break; 
   case 37:
     fDisplay37(cntSub);
     break;
  }
}

RangeData_t   getRangeDataNeedChange(u8 pcode,u8 dot,u8 cntScode)
{
  RangeData_t range;
  switch(pcode)
  {
    case 12:
    case 63:
      range.minNum=0;
      range.maxDbNum=9999999999;                  
      break;
    case 13:
      range.minNum=0;
      range.maxDbNum=99999.1;
      break;
    case 15:
    case 95:
      range.minNum=0;
      range.maxDbNum=9999;
      break;
    case 16:
      range.minNum=0;
//      if(dot==2)
//      {
//        range.maxDbNum=999999;      
//      }
      //else if(dot==3)
      {
        range.maxDbNum=9999999;        
      }
      break;
    case 18:
    case 37:
      range.minNum=0;
      range.maxDbNum=3;
      break;  
    case 19:
      range.minNum=1;
      range.maxDbNum=64;
      break; 
    case 20:
      range.minNum=1;
      range.maxDbNum=6;
      break;  
    case 24:
      if(cntScode==1)
      {
        range.minNum=0;
        range.maxDbNum=3;      
      }
      else
      {
          range.minNum=0;
          range.maxDbNum=9999999;                     
      }
      break;     
    case 30:
      range.minNum=0;
      range.maxDbNum=999;
      break; 
    case 32:
      range.minNum=1;
      range.maxDbNum=10;
      break;
    case 33:
      range.minNum=0;
      range.maxDbNum=5;
      break;      
    case 36:
    case 45:
      range.minNum=0;
      range.maxDbNum=9;
      break;  
//    case 37:
//      range.minNum=0;
//      range.maxDbNum=3;
//      break;  
    case 41:
      range.minNum=0;
      range.maxDbNum=20;
      break; 
    case 44:
    case 48:
      range.minNum=0;
      range.maxDbNum=99;
      break;  
//    case 45:
//      range.minNum=0;
//      range.maxDbNum=9;
//      break;
    case 46:
      range.minNum=0;
      range.maxDbNum=10;
      break; 
    case 47:
      range.minNum=1;
      range.maxDbNum=4;
      break;     
//    case 48:
//      range.minNum=0;
//      range.maxDbNum=99;
//      break;     
//    case 63:
//      range.minNum=0;
//      range.maxDbNum=9999999999;
//      break;   
    case 64:
      range.minNum=0;
      range.maxDbNum=999999;
      break;    
    case 70:
    case 80:      
      range.minNum=0;
      range.maxDbNum=1;
      break;
//    case 80:
//      range.minNum=0;
//      range.maxDbNum=1;
//      break;      
//    case 95:
//      range.minNum=0;
//      range.maxDbNum=9999;  
//      break; 
    case 29:
      range.minNum=0;
      range.maxDbNum=2;  
      break;       
  }
  return range;
}
u8      getSizeFieldDataChange(u8 pcode,u8 cntSub)
{
  switch(pcode)
  {
    case 11:
    case 12:  
    case 63:
        return 10;
    case 13:
    /*Unit price*/
        return 5;
     /*user Pass*/    
    case 15:
        return 4;
     /*Fueling Limit*/   
    case 16:
        return 7;
    case 18:
    case 20:
    case 29:
    case 33:
    case 36:   
    case 37: 

    case 45:
    case 47:   
    case 80:       
        return 1;
    case 19:
    case 32: 
    case 41:
    case 44: 
    case 46:
    case 48:
   
        return 2;
    case 24:
      if(cntSub==1)
      {
        return 1;
      }
      else
      {
        return 7;      
      }
      break;
    case 90:      
        return 8;
    case 64:
      return 6;
    case 95:
        return 12; 
    case 30:
    case 99:
      return    3;
    case 70:
      return 1;
  }
  return 0;
}
TypeValue_t    getValue(u8 pcode,volatile SysConfig_t *config,u8 cntSub)
{
  TypeValue_t Value;
  Value.db=0;Value._u32=0;Value._u64=0;

  switch(pcode)
  {
 
    case 1:
    case 11:      
      if(cntSub==1)
      {
        Value.db=config->Totalizer.volume;
        Value.len_tp[0]=sTypeValues.len_tp[0];   
      }
      else if(cntSub==2)
      {
        Value._u64=config->Totalizer.amount;
      }
      break;
    case 2:
    case 12:      
      if(cntSub==1)
      {
        Value.db=config->DailyTotal.volume;
        Value.len_tp[0]=sTypeValues.len_tp[0];        
      }
      else if(cntSub==2)
      {
        Value._u64=config->DailyTotal.amount;
      }
      break;
    case 3:
      Value._u32=config->UnitPrice;
       break;
  case 8:
      Value._u32=cntSub;
       break;    
    case 13:
       Value._u32=config->UnitPrice;
       break;
    case 15:
       Value._u32=config->UserPassword;

       break;
    case 16:
       Value.db=config->FuelingLimit;
       Value.len_tp[1]=sTypeValues.len_tp[1];

       break;
    case 18:
       Value._u32=config->PosMode; 
      break;
    case 19:
       Value._u32=config->CommID;    
      break;
    case 20:
       Value._u32=config->FuelType;    
       break;
    case 21:
       Value._u32=config->PositonRoundUpAmount;  
     break;
    case 24:
      if(uPresetValue==2)
      {
        if(cntSub==1)
            Value._u64=config->KeypadSetting.Condition;                  
      }
      else
      {
        switch(cntSub)
        {
          case 1:
            Value._u64=(uint64_t)config->KeypadSetting.Condition;            
            break;
        default:
            if(PRESET_TypeFlag[cntSub-2]==bTRUE)
            {
              Value._u64=config->KeypadSetting.OneTouch.PA[cntSub-2];
            }
            else if(PRESET_TypeFlag[cntSub-2]==bFALSE)
            {
              Value.db=config->KeypadSetting.OneTouch.PV[cntSub-2];
              Value.len_tp[cntSub]=sTypeValues.len_tp[cntSub];
            }             
           break;              
        }              
      }
      break;
    case 29:
       Value._u32=config->CoPassword;
       break;
    case 30:
       Value._u32=config->CountryCode; 
      break;
    case 32:
        Value._u32=config->FirstIndication;
      break;
    case 33:
      Value._u32=config->PresetOverRunMasking;
      break;
    case 36:
      Value._u32=config->AmountPresetMethod;
      break;      
    case 41:
      Value.db=config->PresetSlowdownPosition.F[cntSub-1];      
      break;
    case 44:
      Value._u32=config->SlowdownTimeForPulseStop;  
      break;      
    case 45:
      Value._u32=config->PumpLockTimeAfterUPriceChange;  
      break;       
    case 46:
      Value._u32=config->PumpLockTimeForPulseStop;  
      break;
    case 47:
      Value._u32=config->FuelingMode;  
      break;
    case 48:
      Value._u32=config->CommTimeout;  
      break;//    case 60:
    case 63:
       Value._u32=config->FuelingCount;
       break;      
    case 64:
       Value._u32=config->PowerOnOffCount;
       break;
    case 70:
      Value._u32=config->PosVersion;
      break;
    break;
    case 80:
       Value._u32=config->DisplayTest;
       break; 
    case 90:
       Value._u32=config->MaintenancePassword;
       break;       
    case 95:
      Value._u16[0]=config->Calendar.year;
      Value._u16[1]=config->Calendar.month;
      Value._u16[2]=config->Calendar.date;
      Value._u16[3]=config->Calendar.hour;
      Value._u16[4]=config->Calendar.minutes;
      break;
   case 99:
    if(cntSub==1)
    {
      Value.db=config->Version.versionCpu;
    }
    else if(cntSub==2)
    {
      Value.db=config->Version.versionLcd;
    }
      break;
  }
  return Value;
}
TypeSubCode_t   getSubcode(u8 pcode,u8 cntScode)
{
  TypeSubCode_t tScode;
  memset(tScode.cSub,0,2); 
  tScode.iSub=0;
  switch(pcode)
  {
    case 1:
    case 2:
    case 11:
    case 12:  
      switch(cntScode)
      {
        case 1:
          strcpy(tScode.cSub,"1L");
          break;
        case 2:
          strcpy(tScode.cSub,"1A");
          break;          
      }
      break;
    case 21:
     strcpy(tScode.cSub,"A");
     break;
    case 24:
      if(cntScode==1)
      {
      }     
      else
      {
        if(uPresetValue==2)
        {
          memcpy(tScode.cSub,"0",1);  
        }
        else
        {
          if(PRESET_TypeFlag[cntScode-2]==bTRUE)
          {
             memcpy(tScode.cSub,"A",1);        
          }
          else if(PRESET_TypeFlag[cntScode-2]==bFALSE)
          {
            memcpy(tScode.cSub,"L",1);               
          }
          else if(PRESET_TypeFlag[cntScode-2]==2)
          {
            memcpy(tScode.cSub,"0",1);               
          }                   
        }      
      }  
      break;   
    case 41:  
      switch(cntScode)
      {
        case 1:
          strcpy(tScode.cSub,"F1");
          break;
        case 2:
          strcpy(tScode.cSub,"F2");
          break;
        case 3:
          strcpy(tScode.cSub,"F3");
          break;
        case 4:
          strcpy(tScode.cSub,"F4");
          break;
        case 5:
          strcpy(tScode.cSub,"F5");
          break;
        case 6:
          strcpy(tScode.cSub,"F6");
          break;        
      }
      break;
    case 3:     
    case 13:
    case 16:
    case 20:
    case 63:      

      tScode.iSub=cntScode;
      break;

    case 99:
      if(cntScode==1)
      {
        strcpy(tScode.cSub,"b2");        
      }    
      else if(cntScode==2)
      {
        strcpy(tScode.cSub,"d2");         
      }
      break;
  }
  return tScode;
}
void sfRow1(u8 code,u8 index)
{
  LAPIS_DisplayNumber(uSegDigits[index]);   
  LAPIS_ClearSegment(21);   
  LAPIS_DisplayNumber(uSegDigits[code%10]);
  LAPIS_DisplayNumber(uSegDigits[code/10]);    
  LAPIS_ClearSegment(7);  
}
void LAPIS_CheckNumberDisplay(u8 *arr,u8 leng,u8 from)
{
  u8 i;
  bool f=FALSE;
  for(i=from;i<leng;i++)
  {
   if(arr[i]>0)
   {
      f=TRUE;
   }
  }
  if(f==FALSE)LAPIS_ClearSegment(7);
  else LAPIS_DisplayNumber(uSegDigits[arr[leng-1]]); 
}
void    Row3(u8 d1,u8 d2,u8 d3, u8 d4)
{
   if(d4!=0)
   {
      LAPIS_DisplayNumber(d4);
      LAPIS_DisplayNumber(d3);
      LAPIS_ClearSegment(14); 
   }
   else
   {
      LAPIS_ClearSegment(28); 
   }   
   LAPIS_DisplayNumber(d2);
   LAPIS_DisplayNumber(d1);
   LAPIS_ClearSegment(7);
}
void sfDisplayValueChange(u8 pcode,TypeSubCode_t tScode,TypeValue_t tValue,u8 dot,bool valueDbOrInt,u8 sizeField)//SysConfig_t config
{
  u8 i;
  u8 buff[15]={0};
  u8 len=0;
    if(pcode==24)
    {      
      if(uCntScode==1)
      {
        Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
        if(tValue._u32==0)
        {
          LAPIS_DisplayNumber(uSegDigits[0]);        
        }
        else if(tValue._u32==3)
        { 
          LAPIS_DisplayNumber(uSegDigits[3]);      
        }
        LAPIS_ClearSegment(91);    
      }
      else
      {

        if(tValue.len_tp[uCntScode-2]>0)
        {
          Dots(0,dot,0);
          sfRow1(pcode,0);              
          IntergerDigitsExtraction(buff,7,(uint64_t)(tValue.db)*(uint64_t)pow(10,dot-tValue.len_tp[uCntScode-2]));       
           Display_ArrayNumber(buff,6,3,0);
           if(dot==3)LAPIS_DisplayNumber(uSegDigits[buff[3]]);  
           else
           { 
            LAPIS_CheckNumberDisplay(buff,4,0);
           }
        }            
        else
        {
          sfRow1(pcode,0);  
          IntergerDigitsExtraction(buff,7,(uint64_t)tValue._u32);  
          LAPIS_DisplayNumber(uSegDigits[buff[6]]); 
          LAPIS_CheckDisplay(buff,6,3);
        }      
        LAPIS_CheckDisplay(buff,3,0);
        LAPIS_DisplayNumber(uSegDigits[uCntScode-1]);
        LAPIS_DisplayNumber(uSegDigits[15]);
        LAPIS_ClearSegment(35);         
      }            
    }
    else if(pcode==95)
    {
      LAPIS_Clear();
      Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
      for(i=4;i>1;i--)
      {
        IntergerDigitsExtraction(buff,2,tValue._u16[i]);
        LAPIS_DisplayNumber(uSegDigits[buff[1]]);
        LAPIS_DisplayNumber(uSegDigits[buff[0]]);      
      }

      LAPIS_ClearSegment(7);  
      IntergerDigitsExtraction(buff,2,tValue._u16[1]);      
      LAPIS_DisplayNumber(uSegDigits[buff[1]]);
      LAPIS_DisplayNumber(uSegDigits[buff[0]]); 
      IntergerDigitsExtraction(buff,4,tValue._u16[0]);      
      LAPIS_DisplayNumber(uSegDigits[buff[3]]);  
      LAPIS_CheckDisplay(buff,3,0);
      LAPIS_ClearSegment(7);             
    }
    else
    {
      if(tScode.cSub[0]!=NULL)
      {
        if(valueDbOrInt==FALSE)
        {
          if(tScode.cSub[1]=='L')
          {
            Dots(0,dot,0);  
            Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],uSegDigits[tScode.cSub[0]-0x30],uSegDigits[12]);
            IntergerDigitsExtraction(buff,10,(uint64_t)(tValue.db)*(uint64_t)pow(10,dot-tValue.len_tp[0]));          
            Display_ArrayNumber(buff,9,6,0);
             if(dot==3)LAPIS_DisplayNumber(uSegDigits[buff[6]]);  
             else
             {
               LAPIS_CheckNumberDisplay(buff,7,0);
             }
            LAPIS_CheckDisplay(buff,6,0);                                 
            LAPIS_ClearSegment(28);            

          }
          else if(tScode.cSub[1]=='A')
          {
            Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],uSegDigits[tScode.cSub[0]-0x30],uSegDigits[10]);
           len= Split_Number((tValue._u64),buff);
           display_valueChange(pcode,buff,len,0,sizeField,bHaveDot);//strlen((char*)Volume)    
          }
          else if(tScode.cSub[0]=='F')
          {
            Dots(0,1,0);
             Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],uSegDigits[11],uSegDigits[tScode.cSub[1]-0x30]);
             i=(u8)tValue.db;
            LAPIS_DisplayNumber(uSegDigits[i%10]);
            LAPIS_DisplayNumber(uSegDigits[i/10]);
            LAPIS_ClearSegment(84);           
          }
          else if((tScode.cSub[0]=='L'))//|(tScode.cSub[0]=='A')
          {
            sfRow1(pcode,tScode.cSub[0]-0x30);
            len=Split_Number((u32)(tValue._u32),buff);
            display_valueChange(pcode,buff,len,0,sizeField,bHaveDot);          
          }       
        }
        else if(valueDbOrInt==TRUE) //value Interger
        {
          if(tScode.cSub[0]!=0)
          {
            if(tScode.cSub[0]=='A')
            {
              sfRow1(pcode,10);
              len=Split_Number((u32)(tValue._u32),buff);
              display_valueChange(pcode,buff,len,0,sizeField,bHaveDot);//strlen((char*)Volume)               
            }   
            else if(tScode.cSub[0]=='L')
            {
              sfRow1(pcode,12);
              len=Split_Number((u32)(tValue.db),buff);
              display_valueChange(pcode,buff,len,2,sizeField,bHaveDot);//strlen((char*)Volume)              
            }
          }
        }
       }
       else if(tScode.iSub!=0)
        {
          if(tValue.db!=0)
          {
            //Dots(3);
            Dots(0,dot,0);
          }
          sfRow1(pcode,tScode.iSub);    
          if(tValue.db!=0)
          {       
            if(tValue.len_tp[1]>0)
            {                 
              IntergerDigitsExtraction(buff,7,(uint64_t)(tValue.db)*(uint64_t)pow(10,dot-tValue.len_tp[1]));                        
              Display_ArrayNumber(buff,6,3,0); 
             if(dot==3)LAPIS_DisplayNumber(uSegDigits[buff[3]]);  
             else 
             {
               LAPIS_CheckNumberDisplay(buff,4,0);
             }                
              LAPIS_CheckDisplay(buff,3,0);
            }            
            else
            { 
              IntergerDigitsExtraction(buff,7,(uint64_t)tValue.db);   
              LAPIS_DisplayNumber(uSegDigits[0]);
              LAPIS_DisplayNumber(uSegDigits[0]);
              if(dot==3)
              {
                LAPIS_DisplayNumber(uSegDigits[0]); 
              }  
              LAPIS_DisplayNumber(uSegDigits[buff[6]]);                    
              for(i=6;i>3;i--)
             {
               LAPIS_CheckNumberDisplay(buff,i,3); 
             }               
             if(dot==2 )
             {
               LAPIS_CheckNumberDisplay(buff,3,2);
             }
            }
            LAPIS_ClearSegment(49); 
          }
          else
          {
            len=Split_Number((u32)(tValue._u32),buff);
            display_valueChange(pcode,buff,len,0,sizeField,bHaveDot);
          }
        }
        else //NO subcode
        {
          Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
          {
            len=Split_Number((u32)(tValue._u32),&buff[0]);
            display_valueChange(pcode,buff,len,0,sizeField,bHaveDot);
          }         
        }
    }    
  LATCH();
}
void fDisplay(u8 pcode,TypeSubCode_t tScode,TypeValue_t tValue,bool valueDbOrInt,u8 size)//SysConfig_t config
{
  u8 i,dot;
  dot = 3;
  u8 buff[15]={0};
  u8 len=0;
    if(pcode==24)
    {      
      if(uCntScode==1)
      {
        Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
        if(tValue._u32==0)
        {
          LAPIS_DisplayNumber(uSegDigits[0]);        
        }
        else if(tValue._u32==3)
        { 
          LAPIS_DisplayNumber(uSegDigits[3]);      
        }
        LAPIS_ClearSegment(91);    
      }
      else
      {

        if(tValue.len_tp[uCntScode-2]>0)
        {
          Dots(0,dot,0);
          sfRow1(pcode,0);              
          IntergerDigitsExtraction(buff,7,(uint64_t)(tValue.db)*(uint64_t)pow(10,dot-tValue.len_tp[uCntScode-2]));       
           Display_ArrayNumber(buff,6,3,0);
           if(dot==3)LAPIS_DisplayNumber(uSegDigits[buff[3]]);  
           else
           { 
            LAPIS_CheckNumberDisplay(buff,4,0);
           }
        }            
        else
        {
          sfRow1(pcode,0);  
          IntergerDigitsExtraction(buff,7,(uint64_t)tValue._u32);  
          LAPIS_DisplayNumber(uSegDigits[buff[6]]); 
          LAPIS_CheckDisplay(buff,6,3);
        }      
        LAPIS_CheckDisplay(buff,3,0);
        LAPIS_DisplayNumber(uSegDigits[uCntScode-1]);
        LAPIS_DisplayNumber(uSegDigits[15]);
        LAPIS_ClearSegment(35);         
      }            
    }
    else if(pcode==95)
    {
      LAPIS_Clear();
      Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
      for(i=4;i>1;i--)
      {
        IntergerDigitsExtraction(buff,2,tValue._u16[i]);
        LAPIS_DisplayNumber(uSegDigits[buff[1]]);
        LAPIS_DisplayNumber(uSegDigits[buff[0]]);      
      }

      LAPIS_ClearSegment(7);  
      IntergerDigitsExtraction(buff,2,tValue._u16[1]);      
      LAPIS_DisplayNumber(uSegDigits[buff[1]]);
      LAPIS_DisplayNumber(uSegDigits[buff[0]]); 
      IntergerDigitsExtraction(buff,4,tValue._u16[0]);      
      LAPIS_DisplayNumber(uSegDigits[buff[3]]);  
      LAPIS_CheckDisplay(buff,3,0);
      LAPIS_ClearSegment(7);             
    }else
    {
      if(tScode.cSub[0]!=NULL)
      {
        if(valueDbOrInt==FALSE)
        {
          if(tScode.cSub[1]=='L')
          {
            dot = sConfiguration.DecimalPlace.Volume;
            Dots(0,dot,0);
            Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],uSegDigits[tScode.cSub[0]-0x30],uSegDigits[12]);
            IntergerDigitsExtraction(buff,10,(uint64_t)tValue.db *(uint64_t)pow(10,dot-tValue.len_tp[0]));       
            Display_ArrayNumber(buff,9,6,0);
           if(dot==3)LAPIS_DisplayNumber(uSegDigits[buff[6]]);  
           else 
           {
             LAPIS_CheckNumberDisplay(buff,7,0);
           }      
           LAPIS_CheckDisplay(buff,6,0);
           LAPIS_ClearSegment(28);          
          }
          else if(tScode.cSub[1]=='A')
          {
            dot = sConfiguration.DecimalPlace.Amount;
            Dots(0,dot,0);
            Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],uSegDigits[tScode.cSub[0]-0x30],uSegDigits[10]);
            len=Split_Number((tValue._u64),buff);//(u32)
            display_valueChange(pcode,buff,len,0,size,bHaveDot);    
          }
          else if((tScode.cSub[0]=='b') ||(tScode.cSub[0]=='d')||(tScode.cSub[0]=='F'))
          {
            dot = 1;
            Dots(0,dot,0);
            LAPIS_DisplayNumber(uSegDigits[tScode.cSub[1]-0x30]);
            if(tScode.cSub[0]=='b')
              LAPIS_DisplayNumber(uSegDigits[16]);  
            else if(tScode.cSub[0]=='d')
              LAPIS_DisplayNumber(uSegDigits[17]); 
            else if(tScode.cSub[0]=='F')
              LAPIS_DisplayNumber(uSegDigits[11]);
            LAPIS_ClearSegment(14);
            LAPIS_DisplayNumber(uSegDigits[pcode%10]);
            LAPIS_DisplayNumber(uSegDigits[pcode/10]);
            LAPIS_ClearSegment(7);  
            i=(u8)(tValue.db );//*10
            LAPIS_DisplayNumber(uSegDigits[i%10]);
            LAPIS_DisplayNumber(uSegDigits[i/10]);
            LAPIS_ClearSegment(84);            
          }   
        }
        else if(valueDbOrInt==TRUE) //value Interger
        {
          dot = sConfiguration.DecimalPlace.Amount;
          if(tScode.cSub[0]!=0)
          {
            if(tScode.cSub[0]=='A')
            {
              Dots(0,dot,0);
              sfRow1(pcode,10); 
            }   
            len=Split_Number((u32)(tValue._u32),buff);
            display_valueChange(pcode,buff,len,0,size,bHaveDot);                        
          }
        }
      }
      else if(tScode.iSub!=0)
      {
        if(tValue.db!=0)
        {
          Dots(0,dot,0);
        }
        sfRow1(pcode,tScode.iSub);      
        if(tValue.db!=0)
        {
          IntergerDigitsExtraction(buff,7,(uint64_t)tValue.db *(uint64_t)pow(10,dot-tValue.len_tp[1]));      
         Display_ArrayNumber(buff,6,3,0); 
         if(dot==3)LAPIS_DisplayNumber(uSegDigits[buff[3]]);  
         else 
         {  
           LAPIS_CheckNumberDisplay(buff,4,0);
         }   
         LAPIS_CheckDisplay(buff,3,0);
         LAPIS_ClearSegment(49); 
        }
        else
        {
          len=Split_Number((u32)(tValue._u32),buff);
          display_valueChange(pcode,buff,len,0,size,bHaveDot);
        }
      }
      else //NO subcode
      { 
        Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
        len=Split_Number((u32)(tValue._u32),&buff[0]);
        display_valueChange(pcode,buff,len,0,size,bHaveDot);                     
      }
    }
    LATCH();
}
void fDisplay24(u8 pcode,TypeSubCode_t tScode,TypeValue_t tValue,u8 cntScode,u8 selectPreset)//SysConfig_t config
{
  u8 i,dot;
  dot = 3;
  u8 buff[15]={0};
  u8 len=0;
  
  if(tScode.cSub[0]!=NULL)
  {
    if((tScode.cSub[0]=='L'))
    {            
      dot =  sConfiguration.DecimalPlace.Volume;
      if(cntScode==2 ||cntScode==3 ||cntScode==4 || cntScode==5)
      {
        if(tValue.len_tp[cntScode]>0)
        {           
          Dots(0,dot,0);
          sfRow1(pcode,12);   
          IntergerDigitsExtraction(buff,7,(uint64_t)tValue.db *(uint64_t)pow(10,dot-tValue.len_tp[cntScode]));
          Display_ArrayNumber(buff,6,3,0); 
          if(dot==3)LAPIS_DisplayNumber(uSegDigits[buff[3]]);  
          else 
          {             
              LAPIS_CheckNumberDisplay(buff,4,0);
          } 
          LAPIS_CheckDisplay(buff,3,0);
        }
        else
        {
          len=LengthOfInt((u32)tValue.db);
          IntergerDigitsExtraction(buff,7,(uint64_t)tValue.db); 
          if(len<=4)
          {
            Dots(0,dot,0);
            sfRow1(pcode,12);   
            LAPIS_DisplayNumber(uSegDigits[0]);
            LAPIS_DisplayNumber(uSegDigits[0]);                     
           if(dot==3)
            {
              LAPIS_DisplayNumber(uSegDigits[0]); 
            }  
            LAPIS_DisplayNumber(uSegDigits[buff[6]]);                 
           for(i=6;i>3;i--)
           {
             LAPIS_CheckNumberDisplay(buff,i,3);
           }              
             if(dot==2 )
             {
               LAPIS_CheckNumberDisplay(buff,3,2);
             }              
          }
          else
          {
            sfRow1(pcode,12);   
            Display_ArrayNumber(buff,6,1,0);
            LAPIS_CheckNumberDisplay(buff,2,0);
            LAPIS_CheckNumberDisplay(buff,1,0);
          }
        }
        LAPIS_DisplayNumber(uSegDigits[cntScode-1]);
        LAPIS_DisplayNumber(uSegDigits[15]);
        LAPIS_ClearSegment(35);                                                      
      }                 
    }      
    else if(tScode.cSub[0]=='A')
    {
      dot =  sConfiguration.DecimalPlace.Amount;
      sfRow1(pcode,10);   
      if(cntScode==2 ||cntScode==3 ||cntScode==4 || cntScode==5)
      {
        len=Split_Number(tValue._u64,buff);  
        Display_ArrayNumber(buff,len,0,1); 
        LAPIS_ClearSegment((7-len)*7);                 
        LAPIS_DisplayNumber(uSegDigits[cntScode-1]);
        LAPIS_DisplayNumber(uSegDigits[15]);
        LAPIS_ClearSegment(35);           
      }                        
    }
    else if(tScode.cSub[0]=='0')
    {
      sfRow1(pcode,0);  
      LAPIS_DisplayNumber(uSegDigits[0]);
      LAPIS_ClearSegment(42);                 
      LAPIS_DisplayNumber(uSegDigits[cntScode-1]);
      LAPIS_DisplayNumber(uSegDigits[15]);
      LAPIS_ClearSegment(35);                              
    }       
  }
  else 
  {        
    Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
    {
      len=Split_Number((u32)(tValue._u64),&buff[0]);
      //display_value(buff,len);          
      for(i=len;i>0;i--)
      {
        LAPIS_DisplayNumber(uSegDigits[buff[i-1]]);
      }
      LAPIS_ClearSegment((14-len)*7);
    }         
  }
  LATCH();
}
void fDisplay37(u8 row)
{
    if(row==1)//Amount
    {
      //sfRow1(37,sConfiguration.DecimalPlace.UnitPrice);  
      sfRow1(37,aDecimalBuffer[2]); 
      LAPIS_DisplayNumber(uSegDigits[aDecimalBuffer[1]]);      //sConfiguration.DecimalPlace.Volume
      LAPIS_ClearSegment(42);   
      LAPIS_DisplayNumber(uSegDigits[aDecimalBuffer[0]]); //sConfiguration.DecimalPlace.Amount
      LAPIS_DisplayNumber(0x08);
      LAPIS_ClearSegment(35);         
    }
    else if(row==2)//volume
    {
      sfRow1(37,aDecimalBuffer[2]);
      LAPIS_DisplayNumber(uSegDigits[aDecimalBuffer[1]]);  //aDecimalBuffer[1]
      LAPIS_DisplayNumber(0x08);      
      LAPIS_ClearSegment(35);   
      LAPIS_DisplayNumber(uSegDigits[aDecimalBuffer[0]]); 
      LAPIS_ClearSegment(42);        
    }
    else if(row==3)
    {
      Row3(uSegDigits[3],uSegDigits[7],0x08,uSegDigits[aDecimalBuffer[2]]);
      LAPIS_DisplayNumber(uSegDigits[aDecimalBuffer[1]]);       
      LAPIS_ClearSegment(42);   
      LAPIS_DisplayNumber(uSegDigits[aDecimalBuffer[0]]); 
      LAPIS_ClearSegment(42);     
    }             
  LATCH();
}

void fDisplay95(u8 pcode,TypeValue_t tValue)//SysConfig_t config
{
  u8 i,j;
  u8 buff[15]={0};
  u8 len=0;
   
  Row3(uSegDigits[pcode/10],uSegDigits[pcode%10],0,0);
  for(i=5;i>0;i--)
  {
    len=Split_Number(tValue._u16[i-1],buff); 
    for(j=len;j>0;j--)
    {
      if(len==1)
      {
        if(i==1)
        {
           LAPIS_DisplayNumber(uSegDigits[buff[j-1]]);
           LAPIS_DisplayNumber(uSegDigits[0]);
           LAPIS_DisplayNumber(uSegDigits[0]);
           LAPIS_DisplayNumber(uSegDigits[0]);
        }
        else
        {
          LAPIS_DisplayNumber(uSegDigits[buff[j-1]]);
          LAPIS_DisplayNumber(uSegDigits[0]);
        }
      }
      else
      {
        LAPIS_DisplayNumber(uSegDigits[buff[j-1]]);      
      }
    }
    if(i==1||i==3)
    {
      LAPIS_ClearSegment(7);     
    }
  }                            
  LATCH();
}
void LAPIS_ClearSegment(u8 len)
{
  u8 i;
  for(i=0;i<len;i++)
  {
    DATA_LCD_LOW;
    SHIFT();
  } 
}

void sf1(u8 leng,u8 lengtp,u8 digit[])
{
  u8 i;
  Display_ArrayNumber(0,3-lengtp,0,0);  
  for(i=0;i<lengtp;i++)
  {
     LAPIS_DisplayNumber(uSegDigits[digit[leng-i-1]]);        
  } 
  Display_ArrayNumber(digit,leng-lengtp,0,1);
  LAPIS_ClearSegment((11-leng+lengtp)*7);  
}
void LAPIS_CheckDisplay(u8 * arr,u8 num1,u8 num2)
{
  u8 i;
  for(i=num1;i>num2;i--)
  {
    LAPIS_CheckNumberDisplay(arr,i,0);
  }    
}
void Display_ArrayNumber(u8 *arr,u8 from,u8 to,u8 index)
{
  u8 i;
   for(i=from;i>to;i--)
   {
     LAPIS_DisplayNumber(uSegDigits[arr[i-index]]);
   }  
}
void display_valueChange(u8 code,u8 num[],u8 len,u8 dot,u8 size,bool bHaveDot)
{
  u8 i;
  u8 lengtp=0;
  if(dot>0)
  {
    if(bHaveDot==FALSE)
    {
      if(code==12 ||code==16)
      {
        if(code==12)lengtp=sTypeValues.len_tp[0];
        if(code==16)lengtp=sTypeValues.len_tp[1];
        if(bReview==TRUE)
       {
         sf1(len,lengtp,num);         
       }
       else
       {
          sf1(len,uLengTphan,num);          
       }                                 
      }
      else if(code==24)
      {
         if(uCntScode>=2 && uCntScode<=5)
         {
            sf1(len,sTypeValues.len_tp[uCntScode],num);      
         }
      }
      else
      {    
          Display_ArrayNumber(0,size-len,0,0);
          Display_ArrayNumber(num,len,0,1);
          LAPIS_ClearSegment((14-size)*7);  
      } 
     }
    else
    {
      Display_ArrayNumber(0,3-uLengTphan,0,0);
      for(i=0;i<uLengTphan;i++)
      {
          LAPIS_DisplayNumber(uSegDigits[num[len-i-1]]);     
      }
      Display_ArrayNumber(num,len-uLengTphan,0,1);
      if(code==24)
      {
        LAPIS_ClearSegment((4+uLengTphan-len)*7);   
        if(uCntScode>=2 && uCntScode<=5)
        {
          LAPIS_DisplayNumber(uSegDigits[uCntScode-1]);
          LAPIS_DisplayNumber(uSegDigits[15]);
          LAPIS_ClearSegment(35);              
        }
      }
      else
      LAPIS_ClearSegment((11+uLengTphan-len)*7);         
    }
  }
  else
  {
      Display_ArrayNumber(num,len,0,1);
      LAPIS_ClearSegment((14-len)*7);   
  }
}
