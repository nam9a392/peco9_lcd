/**
  ******************************************************************************
  * @file    user_typedefs.h
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
#ifndef __USER_TYPEDEFS_H
#define __USER_TYPEDEFS_H

#ifdef __cplusplus
 extern "C" {
#endif

#define STD_ON 		( 1U )
#define STD_OFF 	( 0U )

#define KP_USE          (0U)
#define KP_NOTUSE       (1U)   
   
#define VERSION_1       (0U)
#define VERSION_2       (1U)
#define VERSION_3       (2U)   
#define VERSION_4       (3U)   
typedef u8               BOOLEAN;
#define bTRUE           ((BOOLEAN)(1U))
#define bFALSE          ((BOOLEAN)(0U))   
//#undefine       v (0U)
/**
  * @}
  */  
 

  
#ifdef __cplusplus
}
#endif

#endif /* __USER_TYPEDEFS_H */
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
