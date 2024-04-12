/**
  ******************************************************************************
  * @file    sw.h
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
#ifndef __UTILITY_H
#define __UTILITY_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4xx.h"

#include "modules_cfg.h"
#include "stdint.h"
/**
  * @} Defines
  */

/* Prototypes here */
u8 LengthOfInt(u32 num);
void ArrayCoppy(u8 *des, u8 *src, u16 len);
void DigitsExtraction(u8 *array, u8 size, uint64_t num);
void IntergerDigitsExtraction(u8 *array, u8 size, uint64_t num);
 uint64_t    stringToInt(u8* str,u8 len);
/**
  * @}
  */  
 

  
#ifdef __cplusplus
}
#endif

#endif /* __UTILITY_H */
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
