/**
  ******************************************************************************
  * @file    pos.h
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
#ifndef __POS_H
#define __POS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "modules_cfg.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "sysSetting.h"
#include "stdio.h"
#include "stm32f10x_tim.h"

/**
  * @} Defines
  */

/* Prototypes here */
#if (POS_USED == STD_ON)

#define          POS_RX_DONE_EVENT_BIT          ( 1 << 0)
#define          BIT_0                          (1<<0)
#define          BIT_4                          (1<<4)
 /*Added*/
#define          HANG_DONE                      (1<<0)
#define          POS_BUFF_LEN                   (400U)  
#define          POS_QUEUE_LEN                  (400U)    
#define          EOT                            ((uint8_t)0x04)
#define          STX                            ((uint8_t)0x02)
#define          ETX                            ((uint8_t)0x03)
#define          NAK                            ((uint8_t)0x15)
#define          ENQ                            ((uint8_t)0x05)
#define          ACK                            ((uint8_t)0x10)
#define         POS_RX_FIRST_BYTE_EVENT_BIT     ( 1 << 1 )
#define         TIME_OUT_PRESET                 1500
   
typedef enum _RetStatus
{
  pTIMEOUT=0,
  pFALSE,
  pTRUE,
  pINVALID
}eReturnStatus;

typedef enum 
{
  NZZL_HANG = 0,
  NZZL_TAKE_OFF,
  NZZL_REFUELING,
  NZZL_REFUELING_END,
  LCD_DETECTED_ERROR,
    
}eNozzleStatus_t; 
typedef struct
{
    u8 Code[2];
 }Errorcode_t;
 
typedef struct
{
    eNozzleStatus_t  NZZLStatus;
    u32  uVolume;
    u32  uPrice;
    u32  uAmount;    
    u8   uFlowrate;
    Errorcode_t errCode;
    u8  uPosValue;
    u32 uPosVolume;
    u32 uPosAmount;
    u8  uLeng[3];
    u8  uArray_UP[5];
    u8  uArray_VL[7];
    u8  uArray_AM[7];
    //bool bNzzlIsHang;
 }SysStatus_t;
//typedef struct
//{
//  u8  uLeng[3];
//  u8  uArray_VL[7];
//  u8  uArray_AM[7];
//  u32  uVolume;
//  u32  uAmount;
//}SDisplay_t;
typedef enum
{
  ENQ_POL = 0x51,
  ENQ_SEL = 0x41,
  
}eEnqType_t;

typedef	enum
{
  POWER_ON=0,
  RUNNING
}STATUS_Typedef;


typedef struct _fuelingValue
{
  u8 uVolume[10];
  u8 uAmount[10];
  
}FuelingValue_t;
typedef struct _UAV
{
  u8 uUPLossPower[6];
  u8 uVolumeLossPower[7];
  u8 uAmountLossPower[7];
}UpAmVolLossPower_t;
typedef struct _onetouch
{
  u8   uType[1];
  u8   uValue[7];
  
}Onetouch_t;

typedef struct _keypad
{
  u8		 uCondition[1];
  Onetouch_t	 P[4];
  
}Keypad_t;

typedef struct _decimal
{
  u8	uAmount[1];
  u8	uVolume[1];
  u8	uPrice[1];
  
}Decimal_t;

typedef struct _calendar
{
  u8  uYear[4];
  u8  uMonth[2];
  u8  uDate[2];
  u8  uHour[2];
  u8  uMin[2];
  
}Calendar_t;

typedef struct _pSlowdownPos
{
  u8	uValue[2];
  
}pSlowdownPos_t;

__packed typedef struct _DataFrame
{
  FuelingValue_t            fTotal;
  FuelingValue_t	    fDailyTotal;
  u8		            uPrice[5];                 
  u8			    UserPassword[4];                       
  u8 			    FuelingLimit[7];                       
  u8			    PosMode[1];                        
  u8			    CommID[2]; 		                    
  u8		            FuelType[1];                       
  Keypad_t		    KeypadSetting; 			        
  u8			    CoPassword[8]; 			            
  u8 			    CountryCode[3]; 			            
  u8 			    FirstIndication[2]; 		            
  u8 			    PresetOverRunMasking[1]; 		        
  u8 			    AmountPresetMethod[1]; 		        
  Decimal_t		    DecimalPlace; 			        
  pSlowdownPos_t   	    PresetSlowdownPosition[6]; 	        
  u8 			    SlowdownTimeForPulseStop[2]; 	        
  u8 			    PumpLockTimeAfterUPriceChange[1];      
  u8 			    PumpLockTimeForPulseStop[1]; 	        
  u8		            FuelingMode[1]; 			        
  u8 			    CommTimeout[2]; 		                
  u8			    FuelingCount[10]; 			            
  u8			    PowerOnOffCount[6];		            
  u8 			    DisplayTest[1]; 			            
  u8			    MaintenancePassword[8]; 		        
  Calendar_t		    Calendar; 	
  //u8                        EnablePrinter[1];
  u8                        bVersion[2];     
  u8                        PosVersion[1];
  UpAmVolLossPower_t        UAV; 
  u8                        uDisplay_Err[1];
  u8                        uDisplay_LastData[1];
}DataFrame_t;

typedef struct _LogData
{
  u8 uVolume[7];
  u8 uAmount[7];
}LogData_t;
 typedef struct _LOGS
{
  Calendar_t calender;
  LogData_t     data;
}Logs_t;
__packed  typedef struct _Log
{
  Logs_t logs[10];
}FrameLogs_t;

void POS_EnableTransmit(void);
void POS_DisableTransmit(void);    
void vPOS_Task( void *pvParameters );
void POS_LitmitSetup(u32 up,volatile float *volume);
void POS_UpdateSysStatusData(u8* ptr,SysStatus_t* status);
//void POS_UpdateSysStatusData_NEW(u8* ptr,SysStatus_t* status);
void POS_UpdateConfig(u8* ptr,volatile SysConfig_t     *config);
eReturnStatus POS_WaitForMsg( u8 addr, eEnqType_t enqType1,eEnqType_t enqType2,u8 *Code, u8 *pMsg, TickType_t timeout );//add leng data
eReturnStatus POS_FuelingMode(void);
eReturnStatus POS_RegistrationToCpu(void);
void TM_TIMER2_Init(void);
void POS_Init(void);
u8 POS_CheckSum(u8 *pBuff, u8 from, u8 to);
bool POS_Send(u8 *pBuff, u8 len, TickType_t timeout);
bool POS_WaitForDataSentDone(TickType_t timeout);
void TIM2_IRQHandler( void );
void USART2_IRQHandler( void );
void USART2_Init(void);
bool Send_Text01(void);
bool Send_Text02(void);
bool Send_Text05(void);
bool Send_ACK0(void);
bool Send_ACK1(void);
bool Send_EOT(void);
bool Sellecting_ReceivedEOTFromCPU(u8 lcdcode,u8 *Posbuff);
bool Send_Text(DataSetup_t *data);
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ); 
void Err_D1(void);
extern u8 idLCD;
#endif


/**
  * @}
  */  
 

  
#ifdef __cplusplus
}
#endif

#endif /* __POS_H */
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

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
