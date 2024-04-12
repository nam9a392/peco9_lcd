/**/
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "sysSettingDefines.h"

#include "pos.h"
#include "comm.h"
#include "utility.h"
/** 
  * @{
  */ 
  
#if (POS_USED == STD_ON)
QueueHandle_t            xPosRxMsgQueue;
QueueHandle_t            xPosTxMsgQueue;
EventGroupHandle_t       xPosRxEventGroup;

volatile BOOLEAN         pos_IsRxDataDetected = bFALSE;
volatile BOOLEAN         pos_IsSendFail = bFALSE;
volatile BOOLEAN         pos_IsTxMode = bFALSE;
BOOLEAN                  pos_StartFeedData = bFALSE;

PosRxMsg_t               posMsg;

ePosCmdType              PosPrevCommand[DISPENSERn] = {NORMAL_CMD};
/**
  * @brief  Init POS GPIO
  * @param  none
  * @retval None
  */
void POS_CommInit(void)
{
  COMM_Init((COMM_TypeDef)POS);
}
/**
  * @brief  Enable to transmit data over rs485
  * @param  none
  * @retval None
  */
void POS_EnableTransmit(void)
{
  COMM_EnableTransmit(POS);
}
/**
  * @brief  Disable to transmit data over rs485
  * @param  none
  * @retval None
  */
void POS_DisableTransmit(void)
{
  COMM_DisableTransmit(POS);
}

void POS_SendData(uint8_t data)
{
  COMM_SendData(POS,data);
}


volatile BOOLEAN pos_WaitForNewMsg = bFALSE;
uint8_t PosBuff[POS_BUFF_LEN] = {0};
uint32_t pos_PassCnt = 0, pos_FailCnt = 0;
volatile BOOLEAN posReceptionReady = bFALSE;
ePOS_STATUS   result;

BOOLEAN       POS_IsRegistrated[DISPENSERn] = {bFALSE,bFALSE};
//BOOLEAN       POS_IsRegistrated[4] = {bFALSE,bFALSE,bFALSE,bFALSE};
uint8_t vTest = 0;
  
void POS_Task( void *pvParameters )
{
  TickType_t     xTickToWait;  
  uint8_t addr = 0x40;
  uint8_t LcdCode = 0;
  EventBits_t   uxBits;
  uint8_t       deviceID;
  uint8_t CRCed[2] = {0};

  CRC16_Init();
  
  TM_TIMER2_Init();
  
  posReceptionReady = bTRUE;
  
  vTaskDelay(1000);
 

  while(1)
  {
    pos_FailCnt = 255;
    if((result = POS_WaitForMsgNew(&posMsg, 500, 500)) == POS_OK)
    {
      if( posMsg.type == pENQ_POL )
      {
       
        if(POS_CheckAddrIsValid(posMsg.addr) == bTRUE)
        {
          /* then get device id */
          addr = posMsg.addr;
          deviceID = POS_GetDeviceId(posMsg.addr);
          
          if(POS_IsRegistrated[deviceID] != bTRUE)
          {
            if( addr == posMsg.addr)
            {
               pos_PassCnt = 1;
               
               //POS_RegistrationProcess(deviceID,&posMsg.ctrlPart);    
#if 1
                /*1. Send TEXT(1) */
                if(POS_SendTEXT00((ControlPart_t *)&posMsg.ctrlPart, CRCed) == bTRUE)
                {
                  /*2. Wait for ACK1 */
                  pos_PassCnt = 2;
                  vTest = 1;
                  if((result = POS_WaitForMsgNew(&posMsg,10, 100)) == POS_OK)
                  {
                    pos_PassCnt = 3;
                    if(posMsg.type == pACK1)
                    {
                      /*3. send EOT */
                      pos_PassCnt = 4;
                      PosBuff[0] = EOT;
                      if(POS_Send(PosBuff,1,20) == bTRUE)
                      {                   
                        pos_PassCnt = 5;
                        /*4. Wait for ENQ_SEL */
                        if((result = POS_WaitForMsgNew(&posMsg, 10, 100)) == POS_OK)
                        {
                          pos_PassCnt = 6;
                          if(posMsg.type == pENQ_SEL && addr == posMsg.addr)
                          {
                            /*5. Send ACK0*/
                            pos_PassCnt = 7;
                            PosBuff[0] = ACK; PosBuff[1] = (uint8_t)0x30;
                            if(POS_Send(PosBuff,2,20) == bTRUE)
                            {
                              pos_PassCnt = 8;
                              /*6. Wait for TEXT2 */
                              if((result = POS_WaitForMsgNew(&posMsg, 10, 100)) == POS_OK)
                              {
                                pos_PassCnt = 9;
                                if(posMsg.type == pTEXT && posMsg.code == 0)
                                {
                                  pos_PassCnt = 10;
                                  if(CRCed[0] == posMsg.data[0] && CRCed[1] == posMsg.data[1])
                                  {
                                     pos_PassCnt = 11;
                                     /*7. Send ACK1 */
                                      PosBuff[0] = ACK; PosBuff[1] = 0x31;
                                      if(POS_Send(PosBuff,2,20) == bTRUE)
                                      {
                                        /*8. Wait for ENQ_POL */
                                        if((result = POS_WaitForMsgNew(&posMsg, 10, 100)) == POS_OK)
                                        {
                                          if(posMsg.type == pENQ_POL && addr == posMsg.addr)
                                          {
                                            /*9. Send TEXT60 - POWER ON*/
                                            pos_PassCnt = 12;
                                            if(POS_SendTEXT_60((ControlPart_t *)&posMsg.ctrlPart) == bTRUE)
                                            {
                                              /* 10. Wait for ACK1 */
                                              if((result = POS_WaitForMsgNew(&posMsg,5, 100)) == POS_OK)
                                              {
                                                if(posMsg.type == pACK1)
                                                {
                                                  pos_PassCnt = 13;
                                                  /*11. Send EOT */
                                                  PosBuff[0] = EOT;
                                                  if(POS_Send(PosBuff,1,20) == bTRUE)
                                                  {
                                                    pos_PassCnt = 14;
                                                    POS_IsRegistrated[deviceID] = bTRUE;
                                                    //vTaskDelay(3);
                                                  }
                                                }else pos_FailCnt = 15;
                                              }else pos_FailCnt = 16;
                                            }else pos_FailCnt = 17;
                                          }else pos_FailCnt = 18;
                                        }else pos_FailCnt = 19;
                                      }else pos_FailCnt = 20;
                                  } else pos_FailCnt = 10;
                                }else pos_FailCnt = 5;
                              }else pos_FailCnt = 4;
                            }else pos_FailCnt = 3;
                          }else pos_FailCnt = 2;
                        }else pos_FailCnt = 1;
                      }else pos_FailCnt = 6;
                    }else pos_FailCnt = 7;                
                  }else pos_FailCnt = 8;
                }
                else pos_FailCnt = 9;
#endif
            }
          }
          else /* The device is registrated */
          {
            /*Else if the device state has changed, have to send TEXT to POS */
            if(PosPrevCommand[deviceID] == ODOMETER_DEMAND)
            {
              PosPrevCommand[deviceID] = NORMAL_CMD;
              /* Send TEXT 65 */
              if(POS_SendTEXT_65(&posMsg) == bTRUE)
              {
                /* 10. Wait for ACK1 */
                  if((result = POS_WaitForMsgNew(&posMsg,10, 100)) == POS_OK)
                  {
                    if(posMsg.type == pACK1)
                    {
                      pos_PassCnt = 13;
                      /*11. Send EOT */
                      PosBuff[0] = EOT;
                      if(POS_SendEOT() == bTRUE)
                      {
                        pos_PassCnt = 100;
                      }
                    }
                  }
              }
            }
            else
            {
              /* IF the device has no change of state, just send EOT */
              PosBuff[0] = EOT;
              if(POS_Send(PosBuff,1,20) == bTRUE)
              {
                
              }
            }
            
          }        
      }
      }
      else if( posMsg.type == pENQ_SEL)
      {
#if 1
        if(POS_CheckAddrIsValid(posMsg.addr) == bTRUE)
        {
          /* then get device id */
          addr = posMsg.addr;
          deviceID = POS_GetDeviceId(posMsg.addr);
            
          if(POS_IsRegistrated[deviceID] == bTRUE)
          {           
            if(POS_SendACK0() == bTRUE)
            {
                 if((result = POS_WaitForMsgNew(&posMsg, 10, 100)) == POS_OK)
                 {
                    if( posMsg.type == pTEXT)
                    {
                        if(POS_TextProcess(&posMsg) == bTRUE)
                        {
                          /* Send ACK1 */
                          if(POS_SendACK1() == bTRUE)
                          {
                            /* Last step: Wait for EOT */
                            if((result = POS_WaitForMsgNew(&posMsg, 10, 10)) == POS_OK)
                            {
                              if( posMsg.type == pEOT)
                              {
                                
                              }
                            }
                          }
                        }
                    }
                 }
            }           
          }
        }
        else
        {
          /*POS transmit in timing error due to the device has not registrated yet */
        }
#endif
      }
    }
    else 
    {
      if(result != POS_RX_NONE && result != POS_RX_ADDR_ERR)  pos_FailCnt++;
    }
     
  }

}
uint8_t _PosBuff[15] = {0};
BOOLEAN POS_SendTEXT00(ControlPart_t *cpart, uint8_t *CRCed)
{
  CrcType_t crc;
  uint8_t addtion;
  uint8_t _unpacked[2];

  _PosBuff[0] =  STX;
  _PosBuff[1] = (uint8_t)cpart->SA;
  _PosBuff[2] = (uint8_t)cpart->UA;
  _PosBuff[3] = (uint8_t)0x30;
  _PosBuff[4] = (uint8_t)0x30;
  _PosBuff[5] = (uint8_t)0x32;
  _PosBuff[6] = (uint8_t)0x31;
  _PosBuff[7] = (uint8_t)0x42;
  _PosBuff[8] = (uint8_t)0x31;
  _PosBuff[9] =  ETX;
  _PosBuff[10] = Set_BCC(_PosBuff);
  
  crc.v = CRC16_CITT(&_PosBuff[3],6);
  addtion = crc.a[0] + crc.a[1];  
  *CRCed++ = Byte2Ascci(((addtion >> 4) & (uint8_t)0x0F));
  *CRCed = Byte2Ascci((addtion & (uint8_t)0x0F));        /* convert to ascii */

  if(POS_Send(_PosBuff,11, 100) == bTRUE)
  {
    return bTRUE;
  }
  else
    return bFALSE;
  
}

BOOLEAN POS_SendTEXT_60(ControlPart_t *cpart)
{
  PosBuff[0] =  STX;
  PosBuff[1] = (uint8_t)cpart->SA;
  PosBuff[2] = (uint8_t)cpart->UA;
  PosBuff[3] = (uint8_t)0x36;
  PosBuff[4] = (uint8_t)0x30;
  PosBuff[5] = (uint8_t)0x31; //POWER ON
  PosBuff[6] = (uint8_t)0x30;
  PosBuff[7] = ETX;
  PosBuff[8] = Set_BCC(PosBuff);
  
  if(POS_Send(PosBuff,9, 50) == bTRUE)
  {
    return bTRUE;
  }
  else
    return bFALSE;
}

/* TEXT65: odometer value */
BOOLEAN POS_SendTEXT_65(PosRxMsg_t *msg)
{
  PosBuff[0] =  STX;
  PosBuff[1] = (uint8_t)msg->ctrlPart.SA;
  PosBuff[2] = (uint8_t)msg->ctrlPart.UA;
  PosBuff[3] = (uint8_t)0x36;
  PosBuff[4] = (uint8_t)0x35;
  PosBuff[5] = (uint8_t)0x31; //nozzle - product kind1
  PosBuff[6] = (uint8_t)0x20;
  PosBuff[7] = (uint8_t)0x20;
  PosBuff[8] = (uint8_t)0x20;
  PosBuff[9] = (uint8_t)0x20;
  PosBuff[10] = (uint8_t)0x20;
  /* odometer value of nozzle 1*/
  for(int i = 0; i < 20; i++)
  {
    PosBuff[i+11] = 0x35;
  }
  PosBuff[31] = ETX;
  PosBuff[32] = Set_BCC(PosBuff);
  
  if(POS_Send(PosBuff,33, 100) == bTRUE)
  {
    return bTRUE;
  }
  else
    return bFALSE;
  
}
BOOLEAN POS_SendACK0(void)
{
   PosBuff[0] =  ACK;
   PosBuff[1] = 0x30;
   if(POS_Send(PosBuff,2, 10) == bTRUE)
   {
      return bTRUE;
   }
   return bFALSE;
}
BOOLEAN POS_SendACK1(void)
{
   PosBuff[0] =  ACK;
   PosBuff[1] =  0x31;
   if(POS_Send(PosBuff,2, 10) == bTRUE)
   {
      return bTRUE;
   }
   return bFALSE;
}

BOOLEAN POS_SendEOT(void)
{
   PosBuff[0] =  EOT;
   
   if(POS_Send(PosBuff,1, 10) == bTRUE)
   {
      return bTRUE;
   }
   return bFALSE;
}

BOOLEAN POS_TextProcess(PosRxMsg_t *msg)
{
  uint8_t index = POS_GetDeviceId(msg->addr);
  uint8_t code = msg->code;
  
  switch(code)
  {
    case 11:
     {
       vTest = 11;
       break;
     }       
    case 12:
     {
        vTest = 12;
        break;
     }     
     case 13:
     {
       vTest = 13;
       break;
     }     
     case 14:
     {
       vTest = 14;
       break;
     }
     
     case 15:
     {
       vTest = 15;
       break;
     }
     
     case 20:
     {
       vTest = 20;
       break;
     }
     
     case 21:
     {
       PosPrevCommand[index] = ODOMETER_DEMAND;
       vTest = 21;
       break;
     }
    
     default:
     {
       /* not supported yet */
        break;
     }
  }
  
  return bTRUE;
}
/*
* Inputs:
*        addr: specifed address of POS node to be compared with received address
*/
ePOS_STATUS POS_WaitForMsgNew( PosRxMsg_t *pMsg, TickType_t ftimeout, TickType_t btimeout )
{
  ePOS_STATUS   retValue = POS_OK;
  EventBits_t   uxBits;
  uint8_t       pxRxedMessage;
  uint8_t       cnt = 0, dataLen = 0, i = 0;
  uint8_t       BCC = 0;
  uint8_t       buff[POS_BUFF_LEN] = {0};
  uint8_t       *ptr = buff;
 
    pos_WaitForNewMsg = bTRUE;
    
    if(POS_WaitForFirstByte(ftimeout) != bTRUE) 
    {
       pos_WaitForNewMsg = bFALSE;
       pMsg->type = pUNKNOWN;
      if(uxQueueMessagesWaiting(xPosRxMsgQueue) != 0)
      {
         xQueueReset(xPosRxMsgQueue);
         return POS_RX_UNVALID; 
      }
      
      return POS_RX_NONE; 
    } 
    
    uxBits = xEventGroupWaitBits(xPosRxEventGroup,      /* The event group being tested. */
                                 POS_RX_DONE_EVENT_BIT, /* The bits within the event group to wait for. */
                                 pdTRUE,                /* BIT_0 & BIT_4 should be cleared before returning. */
                                 pdFALSE,               /* Don't wait for both bits, either bit will do. */
                                 btimeout );             /* Wait a maximum of 100ms for either bit to be set. */
              
    if(uxBits & POS_RX_DONE_EVENT_BIT == POS_RX_DONE_EVENT_BIT)
    {
      
      do
      {
        if( xQueueReceive( xPosRxMsgQueue, &( pxRxedMessage ), ( TickType_t ) 0 ) )
        {          
          *ptr++ = pxRxedMessage;
          //ptr++;
          dataLen++;
        }
      }while(uxQueueMessagesWaiting( xPosRxMsgQueue )!= (UBaseType_t)0);
      
      pMsg->type = pUNKNOWN;
      
      if(pos_PassCnt == 8)
      {
        vTest = 0;
      }
      if(dataLen < 3) /* EOT , NAK, ACK0, ACK1 */
      {
        if(buff[0] == EOT) pMsg->type = pEOT;        
        else if(buff[0] == NAK) pMsg->type = pNAK;
        else if(buff[0] == ACK && buff[1] == 0x30) pMsg->type = pACK0;
        else if(buff[0] == ACK && buff[1] == 0x31) pMsg->type = pACK1;
        //else { return POS_RX_UNVALID; }
      }
      else /* ENQ seq or TEXT */
      {
        if(buff[0] == EOT)
        {
          if(buff[2] == (uint8_t)0x41 && buff[3] == ENQ)        pMsg->type = pENQ_SEL;
          else if(buff[2] == (uint8_t)0x51 && buff[3] == ENQ)   pMsg->type = pENQ_POL;
          else { return POS_RX_UNVALID; }
          
        }
        else if((buff[0] == STX) && (buff[dataLen-2] == ETX))
        {
          pMsg->type = pTEXT;
          
          pMsg->code = ((buff[3]-(uint8_t)0x30) * 10) + (buff[4]-0x30); 
          
          if(dataLen > 8) memcpy(&pMsg->data,&buff[5], dataLen-8);          
          else            memset(&pMsg->data,0, POS_BUFF_SIZE);
          
          if(Check_BCC(buff, buff[dataLen-1]) != bTRUE) return POS_RX_TEXT_BCC_FAIL;
        }     
        else
        {
          return POS_RX_UNVALID;
        }
        pMsg->ctrlPart.SA = buff[1];
        pMsg->ctrlPart.UA = buff[2];
        pMsg->addr = (buff[1] >= (uint8_t)0x40) ? ((buff[1] - (uint8_t)0x40) + 1):0x00;
        /* Verify checksum and address */
        if(pMsg->ctrlPart.UA != (uint8_t)0x41 && pMsg->ctrlPart.UA != (uint8_t)0x51)  return POS_RX_UNVALID;
        if(POS_CheckAddrIsValid(pMsg->addr) != bTRUE) return POS_RX_ADDR_ERR;
        
        if( pMsg->ctrlPart.SA == 0x43)
        {
          vTest = 1;
        }
      }

    }
    else  /* timeout has occured */
    {
        pMsg->type = pUNKNOWN;
        retValue = POS_RX_BLOCK_TIMEOUT;
    }

  vTaskDelay(5);
  return retValue;
}

BOOLEAN POS_WaitForFirstByte(TickType_t timeout)
{
   EventBits_t   uxBits;
   
      /* Then wait for send done by loopback check method */
    uxBits = xEventGroupWaitBits(
            xPosRxEventGroup,   /* The event group being tested. */
            POS_RX_FIRST_BYTE_EVENT_BIT, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            timeout );/* Wait a maximum of 100ms for either bit to be set. */
    if((uxBits & POS_RX_FIRST_BYTE_EVENT_BIT) == POS_RX_FIRST_BYTE_EVENT_BIT) 
    {
      return bTRUE;   
    }
    else 
    { return bFALSE; }
}

BOOLEAN POS_CheckAddrIsValid(uint8_t addr)
{
  //if(addr != xSysConfig[0].CommID && addr != xSysConfig[1].CommID) return bFALSE;
  for(int i = 0; i < 2; i++)
  {
    if( addr == xSysConfig[i].CommID) return bTRUE;
  }
  return bFALSE;
}

uint8_t POS_GetDeviceId(uint8_t addr)
{  
  for(uint8_t i = 0; i < DISPENSERn; i++)
  {
    if(xSysConfig[i].CommID == addr) return i;
  }
  return 5;
}

#if 0
void POS_RegistrationProcess(uint8_t _deviceID, ControlPart_t *cpart)
{
   uint8_t CRCed[2] = {0};  

   /*1. Send TEXT(1) */
    if(POS_SendTEXT00((ControlPart_t *)cpart, CRCed) == bTRUE)
    {
      /*2. Wait for ACK1 */
      pos_PassCnt = 2;
      vTest = 1;
      if((result = POS_WaitForMsgNew(&posMsg,10, 100)) == POS_OK)
      {
        pos_PassCnt = 3;
        if(posMsg.type == pACK1)
        {
          /*3. send EOT */
          pos_PassCnt = 4;
          PosBuff[0] = EOT;
          if(POS_Send(PosBuff,1,20) == bTRUE)
          {                   
            pos_PassCnt = 5;
            /*4. Wait for ENQ_SEL */
            if((result = POS_WaitForMsgNew(&posMsg, 10, 100)) == POS_OK)
            {
              pos_PassCnt = 6;
              if(posMsg.type == pENQ_SEL)
              {
                /*5. Send ACK0*/
                pos_PassCnt = 7;
                PosBuff[0] = ACK; PosBuff[1] = (uint8_t)0x30;
                if(POS_Send(PosBuff,2,20) == bTRUE)
                {
                  pos_PassCnt = 8;
                  /*6. Wait for TEXT2 */
                  if((result = POS_WaitForMsgNew(&posMsg, 10, 100)) == POS_OK)
                  {
                    pos_PassCnt = 9;
                    if(posMsg.type == pTEXT && posMsg.code == 0)
                    {
                      pos_PassCnt = 10;
                      if(CRCed[0] == posMsg.data[0] && CRCed[1] == posMsg.data[1])
                      {
                         pos_PassCnt = 11;
                         /*7. Send ACK1 */
                          PosBuff[0] = ACK; PosBuff[1] = 0x31;
                          if(POS_Send(PosBuff,2,20) == bTRUE)
                          {
                            /*8. Wait for ENQ_POL */
                            if((result = POS_WaitForMsgNew(&posMsg, 10, 100)) == POS_OK)
                            {
                              if(posMsg.type == pENQ_POL )
                              {
                                /*9. Send TEXT60 - POWER ON*/
                                pos_PassCnt = 12;
                                if(POS_SendTEXT_60((ControlPart_t *)&posMsg.ctrlPart) == bTRUE)
                                {
                                  /* 10. Wait for ACK1 */
                                  if((result = POS_WaitForMsgNew(&posMsg,5, 100)) == POS_OK)
                                  {
                                    if(posMsg.type == pACK1)
                                    {
                                      pos_PassCnt = 13;
                                      /*11. Send EOT */
                                      PosBuff[0] = EOT;
                                      if(POS_Send(PosBuff,1,20) == bTRUE)
                                      {
                                        pos_PassCnt = 14;
                                        POS_IsRegistrated[_deviceID] = bTRUE;
                                       
                                      }
                                    }else pos_FailCnt = 15;
                                  }else pos_FailCnt = 16;
                                }else pos_FailCnt = 17;
                              }else pos_FailCnt = 18;
                            }else pos_FailCnt = 19;
                          }else pos_FailCnt = 20;
                      } else pos_FailCnt = 10;
                    }else pos_FailCnt = 5;
                  }else pos_FailCnt = 4;
                }else pos_FailCnt = 3;
              }else pos_FailCnt = 2;
            }else pos_FailCnt = 1;
          }else pos_FailCnt = 6;
        }else pos_FailCnt = 7;                
      }else pos_FailCnt = 8;
    } else pos_FailCnt = 9;   

}
       if(PosBuff[0]==EOT && PosBuff[1]==addr && PosBuff[2]==ENQ_POL && PosBuff[3]==ENQ )
        {
            xTickToWait = 10;
            PosBuff[0]=STX;PosBuff[1]=addr;PosBuff[2]=ENQ_POL;PosBuff[3]=0x34;PosBuff[4]=0x38;PosBuff[5]=ETX;
            PosBuff[6]=POS_CheckSum(PosBuff,1,6);
            if(POS_Send(PosBuff,7,xTickToWait) == bTRUE)
            {
                xTickToWait = 10;
                if (POS_WaitForMsg( addr, ENQ_POL, &LcdCode, &PosBuff[0], xTickToWait ) == bTRUE)
                {
                    if(PosBuff[0] == ACK && PosBuff[1] == 0x31)
                    {
                        PosBuff[0]=EOT; xTickToWait = 15;
                        if(POS_Send(PosBuff,1,xTickToWait) == bTRUE)
                        {
                           xTickToWait = 15;
                          if (POS_WaitForMsg( addr, ENQ_POL, &LcdCode, &PosBuff[0], xTickToWait ) == bTRUE)
                          {
                            if(PosBuff[0]==EOT && PosBuff[1]==addr && PosBuff[2]==ENQ_SEL && PosBuff[3]==ENQ )
                            {
                              PosBuff[0]=ACK;PosBuff[1]=0x30; xTickToWait = 15;
                              if(POS_Send(PosBuff,2,xTickToWait) == bTRUE)
                              {
                                xTickToWait = 200;
                                if (POS_WaitForMsg( addr, ENQ_SEL, &LcdCode, &PosBuff[0], xTickToWait ) == bTRUE)
                                {
                                  if( LcdCode == 15)
                                  {
                                    PosBuff[0]=ACK;PosBuff[1]=0x31; xTickToWait = 15;
                                    if(POS_Send(PosBuff,2,xTickToWait) == bTRUE)
                                    {
                                      xTickToWait = 15;
                                      if (POS_WaitForMsg( addr, ENQ_POL, &LcdCode, &PosBuff[0], xTickToWait ) == bTRUE)
                                      {
                                        if(PosBuff[0]==EOT && PosBuff[1]==addr && PosBuff[2]==ENQ_POL && PosBuff[3]==ENQ )
                                        {
                                          /* Send TEXT 49 */
                                          PosBuff[0]=STX;PosBuff[1]=addr;PosBuff[2]=ENQ_POL;PosBuff[3]=0x34;PosBuff[4]=0x39;PosBuff[5]=0x30;PosBuff[6]=ETX;
                                          PosBuff[7]=POS_CheckSum(PosBuff,1,7);
                                          xTickToWait = 20;
                                          if(POS_Send(PosBuff,8,xTickToWait) == bTRUE)
                                          {
                                            xTickToWait = 20;
                                            if (POS_WaitForMsg( addr, ENQ_POL, &LcdCode, &PosBuff[0], xTickToWait ) == bTRUE)
                                            {
                                              if(PosBuff[0]==ACK && PosBuff[1]==0x31 )
                                              {
                                                PosBuff[0]=EOT; xTickToWait = 15;
                                                if(POS_Send(PosBuff,1,xTickToWait) == bTRUE)
                                                {
                                                  pos_PassCnt++;
                                                  
                                                }else pos_FailCnt=1;
                                              }else pos_FailCnt=2;
                                            }else pos_FailCnt=3;
                                          }else pos_FailCnt=4;
                                        }else pos_FailCnt=5;
                                      }else pos_FailCnt=6;
                                    }else pos_FailCnt=7;
                                  }else pos_FailCnt=8;
                                }else pos_FailCnt=9;
                              }else pos_FailCnt=10;
                            }else pos_FailCnt=11;
                          }else pos_FailCnt=12;
                        }else pos_FailCnt=13;
                    }else pos_FailCnt=14;
                }else pos_FailCnt=15;
            }else pos_FailCnt=16;
        }//else pos_FailCnt=17;
    }
   
#endif
/*
* Inputs:
*        addr: specifed address of POS node to be compared with received address
*/
BOOLEAN POS_WaitForMsg( uint8_t addr, eEnqType_t enqType, uint8_t *Code, uint8_t *pMsg, TickType_t timeout )
{
  BOOLEAN       retValue = bTRUE;
  EventBits_t   uxBits;
  uint8_t       pxRxedMessage;
  uint8_t       cnt = 0, dataLen = 0, i = 0;
  uint8_t       BCC = 0;
  uint8_t       buff[POS_BUFF_LEN] = {0};
  uint8_t       *ptr = buff;
  
  /* empty rx buffer before receiving data */
 
  pos_WaitForNewMsg = bTRUE;

    uxBits = xEventGroupWaitBits(
              xPosRxEventGroup,   /* The event group being tested. */
              POS_RX_DONE_EVENT_BIT, /* The bits within the event group to wait for. */
              pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
              pdFALSE,       /* Don't wait for both bits, either bit will do. */
              timeout );/* Wait a maximum of 100ms for either bit to be set. */
    if(uxBits & POS_RX_DONE_EVENT_BIT == POS_RX_DONE_EVENT_BIT)
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
        if(buff[1] == addr && buff[2] == enqType)
        {
          BCC = POS_CheckSum(buff,1, dataLen-1);          
                    
          if(BCC == buff[dataLen-1])
          {                  
            ArrayCoppy(pMsg, &buff[5], dataLen-7);
            *Code = ((buff[3]-(uint8_t)0x30) * 10) + (buff[4]-0x30); 
             retValue = bTRUE;
          }
          else
          {
              retValue = bFALSE;
          }
        }
        else retValue = bFALSE;
      }
      else  
      { 
        /* save data */
        if(buff[0] != STX)
        {
          /* Store data for user buffer */
          ArrayCoppy(pMsg, buff, dataLen);        
        }
        *Code = 0; 
         retValue = bTRUE;
      }

    }
    else
    {
        retValue = bFALSE;
    }

  vTaskDelay(10);
  return retValue;
}


BOOLEAN POS_Send(uint8_t *pBuff, uint8_t len, TickType_t timeout)
{
  BOOLEAN retValue = bTRUE;
  uint8_t i = 0;
  
   xQueueReset( xPosRxMsgQueue );
   xQueueReset( xPosTxMsgQueue );
  /* Enable Data to be transmitted over RS485 */
  POS_EnableTransmit();  
    
   
  /* ENQ POL: EOT - Addr - ENQ_POL - ENQ */
  for( i = 0; i < len; i++)
  {
    //xQueueSend( xPosTxMsgQueue, ( void * ) (pBuff+i), ( TickType_t ) 0 );
    xQueueSend( xPosTxMsgQueue, (pBuff+i), ( TickType_t ) 0 );
  }
  
  if(POS_WaitForDataSentDone((TickType_t)(timeout)) != bTRUE){ retValue = bFALSE; }

   
  return retValue;
}

BOOLEAN POS_WaitForDataSentDone(TickType_t timeout)
{
  BOOLEAN     retValue = bTRUE;
  EventBits_t uxBits;
  
  pos_IsTxMode = bTRUE;
  pos_IsSendFail = bFALSE;
  /* Enable Tx interrupt. This way, data is transmitted immediately  */
  USART_ITConfig( USART1, USART_IT_TXE, ENABLE );
  
   /* Then wait for send done by loopback check method */
  uxBits = xEventGroupWaitBits(
            xPosRxEventGroup,   /* The event group being tested. */
            POS_RX_DONE_EVENT_BIT, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            timeout );/* Wait a maximum of 100ms for either bit to be set. */

  if(uxBits & POS_RX_DONE_EVENT_BIT == POS_RX_DONE_EVENT_BIT)
  {   

    if(pos_IsSendFail == bTRUE){
      retValue = bFALSE;
    }
    else retValue = bTRUE;
  }
  else
  {
      retValue = bFALSE;
  }
  pos_IsTxMode = bFALSE;
  
  return retValue;
}

/*
    Function: Calculate checksum for specified array of bytes
*/
uint8_t POS_CheckSum(uint8_t *pBuff, uint8_t from, uint8_t to)
{
    uint8_t BCC = 0;
    uint8_t i = 0;
    
    for(i = from; i < to; i++)
    {
        BCC |= *(pBuff+i);
    }
    BCC = BCC << 2;
    BCC = BCC + 1;
    
    return BCC;
}

BOOLEAN POS_Init(void)
{    
    xPosRxMsgQueue = xQueueCreate( 150U, sizeof( uint8_t ) );
    xPosTxMsgQueue = xQueueCreate( 150U, sizeof( uint8_t ) ); 
    
    xPosRxEventGroup = xEventGroupCreate();
    
    USART1_Init();
    
    
    return bTRUE;
}

/*------------------------------------------------------------------------------
*                   POS Protocol ( TEXT )
================================================================================
* I. POS ---> Dispenser                                 Code
--------------------------------------------------------------------------------
* 1. Refueling permission :                             |'11'|
-------------------------------------------------------------|------------------
* 2. Calcellation of Refueling permission :             |'12'|
-------------------------------------------------------------|------------------
* 3. Demand of fuel dispenser status :                  |'15'|
-------------------------------------------------------------|------------------
* 4. Demand of fuel dispenser odometer value :          |'20'|
================================================================================
* II. Fuel dispenser ---> POS
* 1. Situation report of fuel dispenser :               |'60'|
-------------------------------------------------------------|------------------
* 2. Fuel dispenser status :                            |'61'|
-------------------------------------------------------------|------------------
* 3. Reception protocol error message :                 |'62'|
-------------------------------------------------------------|------------------
* 4. Calcellation demand of refueling permission :      |'64'|
-------------------------------------------------------------|------------------
* 5. Odometer value of fuel dispenser :                 |'65'|
-------------------------------------------------------------|------------------
* 6. Notification of urgent halt condition :            |'66'|
-------------------------------------------------------------------------------*/
char chrTest = 0;

volatile BOOLEAN posTimerEnabled = bFALSE;

void USART1_IRQHandler(void)
{
  portBASE_TYPE     xHigherPriorityTaskWoken = pdFALSE;
  uint8_t           cChar;
  static uint8_t    TxcChar;

    if( USART_GetITStatus( USART1, USART_IT_TXE ) == SET )
    {
        /* The interrupt was caused by the THR becoming empty.  Are there any
        more characters to transmit? */
        if( xQueueReceiveFromISR( xPosTxMsgQueue, &TxcChar, &xHigherPriorityTaskWoken ) == pdTRUE )
        {
            /* A character was retrieved from the queue so can be sent to the
            THR now. */
            USART_SendData( USART1, TxcChar );
            USART_ITConfig( USART1, USART_IT_TXE, DISABLE );
        }
        else
        {
            USART_ITConfig( USART1, USART_IT_TXE, DISABLE );
            
            xEventGroupSetBitsFromISR( xPosRxEventGroup,   /* The event group being updated. */
                                    POS_RX_DONE_EVENT_BIT, /* The bits being set. */
                                    &xHigherPriorityTaskWoken );
            POS_DisableTransmit();
        }
        
         USART_ClearFlag(USART1,USART_FLAG_TC);
    }

   if( USART_GetITStatus( USART1, USART_IT_RXNE ) == SET )
    {
        cChar = (uint8_t)USART_ReceiveData( USART1 );
        
        if(pos_IsTxMode == bTRUE )
        {
          /* in transmit mode, verify each transmitted byte by equal comparision with each received byte 
          If this is not the case, tell that Sending was failed */
          if(cChar == TxcChar)  
          { 
            USART_ITConfig( USART1, USART_IT_TXE, ENABLE ); 
          }
          else                  
          { 
            pos_IsSendFail = bTRUE; 
            POS_DisableTransmit(); 
          }         
        }
        else
        {
          if(pos_WaitForNewMsg == bTRUE)
          {
              if(cChar == STX || cChar == EOT || cChar == ACK || cChar == NAK )
              {
                  pos_WaitForNewMsg = bFALSE;
                  
                  pos_StartFeedData = bTRUE;
                  
                  if(posTimerEnabled == bFALSE)
                  {
                    posTimerEnabled = bTRUE;            
                    //TIM_Cmd(TIM2, ENABLE);            
                    TIM_SetCounter(TIM2, 0U);
                    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
                    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
                  }
        
                  if (xEventGroupSetBitsFromISR( xPosRxEventGroup,  POS_RX_FIRST_BYTE_EVENT_BIT, &xHigherPriorityTaskWoken ) != pdTRUE) while(1);
                                    
              }
          }
          /* save data if this data is belong to remote device */
          if ( xQueueIsQueueFullFromISR( xPosRxMsgQueue ) == pdFALSE && pos_StartFeedData == bTRUE)
          {
            xQueueSendFromISR( xPosRxMsgQueue, &cChar, &xHigherPriorityTaskWoken );
            
                 /* Clear timer counter whenever get one byte */
            TIM_SetCounter(TIM2, 0U);
            
            pos_IsRxDataDetected = bTRUE;
          }
        }
               
         
        USART_ClearFlag(USART1,USART_FLAG_RXNE);
        
        /* Clear timer counter whenever get one byte */
        //TIM_SetCounter(TIM2, 0U);        
       
    }
       
    
   portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void TIM2_IRQHandler( void )
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  BaseType_t result;
  
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {  
     
     if(pos_IsRxDataDetected == bTRUE)
     {
        pos_IsRxDataDetected = bFALSE;
        
        pos_StartFeedData = bFALSE;
        pos_WaitForNewMsg = bTRUE;
        posTimerEnabled = bFALSE;
        //TIM_Cmd(TIM2, DISABLE);
        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
        
        result = xEventGroupSetBitsFromISR( xPosRxEventGroup,   /* The event group being updated. */
                                    POS_RX_DONE_EVENT_BIT, /* The bits being set. */
                                    &xHigherPriorityTaskWoken );
        if(result != pdTRUE)
        {
          result = pdTRUE;
        }
     }
     TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
          
  }
  
  portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


void TM_TIMER2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    NVIC_InitTypeDef nvicStructure;
    
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
    TIM_BaseStruct.TIM_Prescaler = 74;
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
    TIM_BaseStruct.TIM_Period = 1699; /* 1ms period */
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;
    /* Initialize TIM2 */
    TIM_TimeBaseInit(TIM2, &TIM_BaseStruct);
    /* Start count on TIM2 */
    TIM_Cmd(TIM2, ENABLE);    

    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

    /* Enable interrupt */
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY;
    nvicStructure.NVIC_IRQChannelSubPriority = 0;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}
void USART1_Init(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        USART_ClockInitTypeDef USART_ClockInitStructure;
        NVIC_InitTypeDef NVIC_InitStruct;
        EXTI_InitTypeDef EXTI_InitStruct;
        
        /* init for POS_485 ( USART 1 ) */
        
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
        
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
        // Initialize pins as alternating function
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /* Enable UART clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_InitStructure.USART_BaudRate = 19200;        
        USART_InitStructure.USART_Parity = USART_Parity_Even;
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

        
//        USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
//        USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low; //
//        USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;
//        USART_ClockInitStructure.USART_LastBit = USART_LastBit_Enable;
        
        /* USART configuration */
        USART_Init(USART1, &USART_InitStructure);
//        USART_ClockInit(USART1, &USART_ClockInitStructure); 
        
        /* Enable USART */
        USART_Cmd(USART1, ENABLE); 
        
        /**
         * Enable RX interrupt
         */
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        /**
         * Set Channel to USART1
         * Set Channel Cmd to enable. That will enable USART1 channel in NVIC
         * Set Both priorities to 0. This means high priority
         *
         * Initialize NVIC
         */
        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = configLIBRARY_LOWEST_INTERRUPT_PRIORITY;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
        NVIC_Init(&NVIC_InitStruct);
        
}


#endif
    
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
