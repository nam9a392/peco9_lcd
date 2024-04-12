#ifndef __SYS_SETTING_DEFINES_H
#define __SYS_SETTING_DEFINES_H

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @brief Definition for SYSTEM configuration structure
 */ 
   

__packed typedef struct _TotalizerValue
{
  double volume; // L
  uint64_t amount;
	
}TotalizerValue_t;

__packed typedef struct _DailyTotalizerValue
{
  double volume; // L
  uint64_t amount;
	
}DailyTotalizerValue_t;

typedef uint32_t UnitPrice_t;


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
typedef struct _TienLit
{
  u32 uUPLossPower;
  u32 uVolumeLossPower;
  u32 uAmountLossPower;
}UAVLossPower_t;
typedef enum _LANGUAGE
{
  Vietnamese=0,
  English
}eLanguage_t;
typedef enum _CountryCode
{
  Default=0,
  Mianma=222,
  Taiwan=158, 
  India=356,
  Korea=410,
  Indonesia=360,
  HongKong=344,
  Malaysia=458,
  Philippines=608,
  VietNam=704,
  ThaiLand=764
}eCountryCode_t;
/* - setting of units for fueling with rounding-up numbers 
   - position of a digit which is rounded off in case of fueling with rounding-up numbers is set */

typedef enum _KeypadCondition
{
  PRESET_UNAVAILABLE = 0,
  PRESET_AVAILABLE = 3
	
}eKeypadCondition_t;

   __packed typedef struct _PresetValue
{
  uint64_t        PA[5];
  double     PV[5];	
}PresetValue_t;

typedef struct _DataSetup
{
  uint64_t      data64;
  u16           data_calender[5];
  u8            code;
  u8            AmountOrVolume;
  u8            leng_default;
  u8            leng_tp;
  u8            Index[1];
  u8            leng_data;
  u8            dataArr[80];
}DataSetup_t;

__packed typedef struct _KeypadSetting
{
  eKeypadCondition_t    Condition;
  PresetValue_t         OneTouch;   	
}KeypadSetting_t;

typedef struct _DecimalPlaces
{
  u8	Amount;
  u8	Volume;
  u8	UnitPrice;	
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
  NORMAL = 1,
  TRAINING,
  ACTUAL,
  FLOW_RATE_DISPLAY	
	
}eFuelingMode_t;


typedef enum
{
    SUNNYXE_PRESET =0,
    SUNNYXE_CODE,
    SUNNYXE_PRINT,
    SUNNYXE_READ,
    SUNNYXE_ADMIN,
    SUNNYXE_USER,
    SUNNYXE_OILCOMP,
    SUNNYXE_FUELING,
    SUNNYXE_BUSY,
    SUNNYXE_PECO
}eLoginMode_t;

__packed typedef struct _ePresetSlowPos
{
  u8    F[6];
}ePresetSlowPos_t;

__packed typedef struct _Version
{
  u8 versionLcd;
  u8 versionCpu;
}Version_t;
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
   eCountryCode_t        	CountryCode; 			/* code 30                                                */
  uint8_t 			FirstIndication; 		/* code 32 , default = 8                                  */
  uint8_t 			PresetOverRunMasking; 		/* code 33,  default = 5                                   */
  uint8_t 			AmountPresetMethod; 		/* code 36,  default = 0                                   */
  //DecimalPlaces_t		DecimalPlace; 			/* code 37,  default = 2, 2, 2                             */
  DecimalPlaces_t                DecimalPlace;
  //double 			PresetSlowdownPosition; 	/* code 41,  default 0.8 for all fuel type.                */
  ePresetSlowPos_t 	        PresetSlowdownPosition;
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
  Version_t                     Version;                        /*code 99*/
  u8                            PositonRoundUpAmount;           /*code 21, defualt 2*/
  u8                            MonetaryRatio;                   /*code 38*/
  u8                            PosVersion;  
  UAVLossPower_t                UAV;
  u8                            uDisplay_Err;
  u8                            uDisplay_LastData;
}SysConfig_t;   



#ifdef __cplusplus
}
#endif

#endif /* __SYS_SETTING_DEFINES_H */
