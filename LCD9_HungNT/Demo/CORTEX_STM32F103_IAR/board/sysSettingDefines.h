/**
  ******************************************************************************
  * @file    iar_stm32f407zg_sk.h
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
#ifndef __SYS_SETTING_DEFINES_H
#define __SYS_SETTING_DEFINES_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "nozzle.h"
   
#define DISPENSERn    (2U)
   
/**
 * @brief Definition for SYSTEM configuration structure
 */ 
__packed typedef struct _TotalizerValue
{
  double volume; // L
  double amount;
	
}TotalizerValue_t;

__packed typedef struct _DailyTotalizerValue
{
  double volume; // L
  double amount;
	
}DailyTotalizerValue_t;

typedef uint32_t UnitPrice_t;

typedef float Density_t;

typedef enum _PosMode
{
  OFFLINE0 = 0,
  OFFLINE1,
  AUTO,
  ONLINE
	
}ePosMode_t;

typedef enum _FuleTypeCode
{
  GASOLINE1 = 1,
  GASOLINE2,
  LIGHT_OIL,
  KEROSENE,
  GASOLINE3,
  GASOLINE4
	
}eFuelTypeCode_t;

/* - setting of units for fueling with rounding-up numbers 
   - position of a digit which is rounded off in case of fueling with rounding-up numbers is set */
__packed typedef struct _UnitDesignation
{
  uint8_t volume;
  uint8_t amount;
	
}UnitDesignation_t;

typedef enum _KeypadCondition
{
  PRESET_UNAVAILABLE = 0,
  PRESET_AVAILABLE = 3
	
}eKeypadCondition_t;

__packed typedef struct _PresetValue
{
  uint32_t   PA[5];
  double     PV[5];	
}PresetValue_t;

__packed typedef struct _KeypadSetting
{
  eKeypadCondition_t    Condition;
  PresetValue_t         OneTouch;   
	
}KeypadSetting_t;

__packed typedef struct _DecimalPlaces
{
  uint8_t	Amount;
  uint8_t	Volume;
  uint8_t	UnitPrice;
	
}DecimalPlaces_t;

/* total of 12 digits */
__packed typedef struct _CalendarSetting
{
  uint16_t    year;
  uint8_t     month;
  uint8_t     date;
  uint8_t     hour;
  uint8_t     minutes;
	
}CalendarSetting_t;

typedef enum _FuelingMode
{
  NORMAL = 0,
  TRAINING,
  ACTUAL,
  FLOW_RATE_DISPLAY	
	
}eFuelingMode_t;

/* system structure here */
__packed typedef struct _SysConfig
{
  TotalizerValue_t		Totalizer;
  DailyTotalizerValue_t 	DailyTotal;
  UnitPrice_t		        UnitPrice;                      /* code 13                                                */
  uint16_t			UserPassword;                   /* code 15  default 2222                                  */
  double 			FuelingLimit;                   /* code 16                                                */
  ePosMode_t			PosMode;                        /* code 18                                                */
  uint8_t			CommID; 		        /* code 19 ID in rs485 */
  eFuelTypeCode_t		FuelType;                       /* code 20                                                */ 
  KeypadSetting_t		KeypadSetting; 			/* code 24                                                */ 
  uint32_t			CoPassword; 			/* code 29 - default 71777554                             */
  uint8_t 			CountryCode; 			/* code 30                                                */
  uint8_t 			FirstIndication; 		/* code 32 , default = 8                                  */
  uint8_t 			PresetOverRunMasking; 		/* code 33,  default = 5                                   */
  uint8_t 			AmountPresetMethod; 		/* code 36,  default = 0                                   */
  DecimalPlaces_t		DecimalPlace; 			/* code 37,  default = 2, 2, 2                             */
  double 			PresetSlowdownPosition; 	/* code 41,  default 0.8 for all fuel type.                */
  uint8_t 			SlowdownTimeForPulseStop; 	/* code 44                                                */
  uint8_t 			PumpLockTimeAfterUPriceChange;  /* code 45,  after change of displayed unit price          */
  uint8_t 			PumpLockTimeForPulseStop; 	/* code 46				                   */
  eFuelingMode_t		FuelingMode; 			/* code 47                                                */
  uint8_t 			CommTimeout; 		        /* code 48                                                */
  uint32_t			FuelingCount; 			/* code 63                                                */
  uint32_t			PowerOnOffCount;		/* code 64                                                */
  uint8_t 			DisplayTest; 			/* code 80                                                */
  uint32_t			MaintenancePassword; 		/* code 90 , default 71777554                             */
  CalendarSetting_t		Calendar; 			/* code 95	                                          */
	
}SysConfig_t;

typedef struct _SysStatus
{
  eNozzleStatus_t  nzlState;
  uint32_t         unitPrice;
  double           AmountOfLit;
  uint32_t         AmountOfMoney;
  
}SysStatus_t;

extern SysConfig_t    xSysConfig[DISPENSERn];
  
#ifdef __cplusplus
}
#endif

#endif /* __SYS_SETTING_DEFINES_H */
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
