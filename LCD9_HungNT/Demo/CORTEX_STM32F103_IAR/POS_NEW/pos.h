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
#include "stm32f4xx.h"
#include "modules_cfg.h"
#include "comm.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"

/**
  * @} Defines
  */
typedef enum _permissionCodition
{
  REFUELING_USUALLY=0x30,
  LIMITATION_AMOUNT=0x31,
  PRESET_REFUELING_1=0x32,  /* change permission by fuel dispenser */
  PRESET_REFUELING_2=0x33   /* change prohitbition by fuel dispenser */
}ePermissionCondition;

typedef enum _quantityType
{
  AMOUNT=0x31,
  MONEY=0x32
}eQuantityType;

typedef enum _productFlag
{
  REFUELING_PROHIBITION=0x30,
  REFUELING_PERMISSION_1=0x31,  /* unit price none */
  REFUELING_PERMISSION_2=0x32,  /* there is a unit price */
  REFUELING_PERMISSION_3=0x33   /* used for the consumption tax */
  
}eProductFlag;

typedef struct _productKind
{
    eProductFlag   flag;
    uint32_t       uprice;
    
}ProductKind_t;

typedef struct _rxPosMessage
{
  ePermissionCondition       perCondition;
  eQuantityType              qType;     /* L/Yen */ 
  double                     quantity;  /* amount or money */
  ProductKind_t              pKind;
  
}rxPosMessage_t;

typedef enum _posStatus
{
  POS_OK = 0,
  POS_RX_NONE,
  POS_RX_BLOCK_TIMEOUT,
  POS_RX_ADDR_ERR,
  POS_RX_TEXT_BCC_FAIL,
  POS_RX_TEXT_FAIL,
  POS_RX_UNVALID,    
  POS_TX_FAIL

}ePOS_STATUS;

typedef enum _msgType
{
  pENQ_POL = 0,
  pENQ_SEL,
  pTEXT,
  pEOT,
  pACK0,
  pACK1,
  pNAK,
  pUNKNOWN
}ePosMsgType;

typedef enum _posCmdType
{
  NORMAL_CMD=0,
  REFUEL_PERMISSION,
  REFUEL_CANCELLATION,
  PUMP_ROCK,
  PUMP_RELEASE,
  STATUS_DEMAND,
  ODOMETER_DEMAND
  
}ePosCmdType;

#define POS_BUFF_SIZE  (300)

typedef struct _controlPart
{
  uint8_t  SA;  /* station address, range 0x40 -> 0x7F */
  uint8_t  UA;
}ControlPart_t;

typedef struct _posRxMsg
{
  ePosMsgType    type;
  ControlPart_t  ctrlPart;
  uint8_t        addr;
  uint8_t        code;
  uint8_t        data[POS_BUFF_SIZE];
}PosRxMsg_t;

/* Prototypes here */
#if (POS_USED == STD_ON)

#define POS_RX_DONE_EVENT_BIT        ( 1 << 0)
#define POS_RX_FIRST_BYTE_EVENT_BIT  ( 1 << 1 )
#define POS_RX_EVENT_BIT             (POS_RX_DONE_EVENT_BIT | POS_RX_FIRST_BYTE_EVENT_BIT)
#define POS_BUFF_LEN                 ( 150U )  
#define POS_FIRST_TIMEOUT            ((TickType_t)1 )  

void POS_CommInit(void);  
void POS_EnableTransmit(void);
void POS_DisableTransmit(void);    

void POS_Task( void *pvParameters );
void TM_TIMER2_Init(void);
BOOLEAN POS_Init(void);
BOOLEAN POS_WaitForMsg( uint8_t addr, eEnqType_t enqType, uint8_t *Code, uint8_t *pMsg, TickType_t timeout );
uint8_t POS_CheckSum(uint8_t *pBuff, uint8_t from, uint8_t to);
BOOLEAN POS_Send(uint8_t *pBuff, uint8_t len, TickType_t timeout);
BOOLEAN POS_WaitForDataSentDone(TickType_t timeout);
void USART1_Init(void);

void POS_RegistrationProcess(uint8_t _deviceID, ControlPart_t *cpart);
ePOS_STATUS POS_WaitForMsgNew( PosRxMsg_t *pMsg, TickType_t ftimeout, TickType_t btimeout );
BOOLEAN POS_WaitForFirstByte(TickType_t timeout);
BOOLEAN POS_CheckAddrIsValid(uint8_t addr);
uint8_t POS_GetDeviceId(uint8_t addr);

BOOLEAN POS_SendTEXT00(ControlPart_t *cpart, uint8_t *CRCed);
BOOLEAN POS_SendTEXT_60(ControlPart_t *cpart);
BOOLEAN POS_SendTEXT_65(PosRxMsg_t *msg);

BOOLEAN POS_SendACK0(void);
BOOLEAN POS_SendACK1(void);
BOOLEAN POS_SendEOT(void);

BOOLEAN POS_TextProcess(PosRxMsg_t *msg);

void POS_SendData(uint8_t data);

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
