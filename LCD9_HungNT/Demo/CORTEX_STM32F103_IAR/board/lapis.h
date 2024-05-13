/**
  ******************************************************************************
  * @file    lapis.h
  * @brief   
  *          
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
  

#ifndef LAPIS_H
#define LAPIS_H
#include  "stm32f10x_gpio.h"
#include  "function.h"
#include  "string.h"
#include  <stdlib.h>
#include  "sysSetting.h"
#include  "utility.h"
#include  "listcode.h"

extern  volatile u8                      aDecimalBuffer[3];
#define		DATA_LOW		GPIO_WriteBit(KB_DATAOUT_PORT,KB_DATAOUT_PIN,Bit_RESET);
#define		DATA_HIGH		GPIO_WriteBit(KB_DATAOUT_PORT,KB_DATAOUT_PIN,Bit_SET);
#define		CLOCK_LOW		GPIO_WriteBit(KB_CLOCK_PORT,KB_CLOCK_PIN,Bit_RESET);
#define		CLOCK_HIGH		GPIO_WriteBit(KB_CLOCK_PORT,KB_CLOCK_PIN,Bit_SET);
#define		KB_LATCH_LOW		GPIO_WriteBit(KB_LATCH_PORT,KB_LATCH_PIN,Bit_RESET);
#define		KB_LATCH_HIGH		GPIO_WriteBit(KB_LATCH_PORT,KB_LATCH_PIN,Bit_SET);

#define         PFL_PIN                 GPIO_Pin_8
#define		PFL_PORT		GPIOB//B

#define LED_SYSTEM_Pin                GPIO_Pin_15
#define LED_SYSTEM_GPIO_Port          GPIOA    

/*Define size field information of printer*/
#define Size_Name               80
#define Size_Address            80
#define Size_KieuCB             30
#define Size_Kyhieu             30
#define Size_Serial             30
#define Size_Nhienlieu          30
#define Size_TenCty             60
#define Size_Dthoai             20
/*Define size field number preset*/
#define Size_Number_Preset      7

/*Version LCD*/
#define LCD_Version             24   //Update version 18/12/2023
#define CODE2_LENGTH            199
void Display_DataChange(u8 code,u8 cntScode,eLoginMode_t mode,uint64_t *intValue,bool bDot, volatile TypeValue_t   *tValue);
bool PRESET_CheckValidVolume(uint64_t value,volatile BOOLEAN *flag,bool havedot);
bool PRESET_CheckValidAmount(uint64_t value,volatile BOOLEAN *flag,bool havedot);
bool SUNNYXE_SaveData(volatile SysConfig_t *config,uint64_t intValue,u8 pcode,u8 cntScode);
bool SUNNYXE_SaveData24(volatile SysConfig_t *config,uint64_t intValue,u8 cntScode);
void LCD_Default(void);
//void TextLcd_Display(bool bl,u32 num,u8 leng);
void TextLcd_Display(bool bl,u32 num,u8 leng,u8 value,bool bDisplay_Donvi);
void PRESET_SendValue(char key);
void Switch_Money(u8 x2,u8 y2);
/*Added new function 22/5/2021*/
void FUELING_SendData(char key);
static char Font_AA[10][4]=
{
  '0','0','0','0',
  '1','A','B','C',
  '2','D','E','F',
  '3','G','H','I',
  '4','J','K','L',
  '5','M','N','O',
  '6','P','R','S',
  '7','T','U','V',
  '8','W','X','Y',
  '9','Q','Q','Q',
};
static char Font_aa[10][4]=
{
  '0','0','0','0',
  '1','a','b','c',
  '2','d','e','f',
  '3','g','h','i',
  '4','j','k','l',
  '5','m','n','o',
  '6','p','r','s',
  '7','t','u','v',
  '8','w','x','y',
  '9','q','q','q',
};
#if (KEYPAD_USE74HC==KP_USE)
static char Keyboard[6][4]={        
        '.','H','R','P',
        '/','-',':',',',
        '0',' ','C','L',
	'1','2','3','$',
	'4','5','6','S',  
        '7','8','9','X'
};
#else
static char KeyboardNew[4][6]={
	'.','-','0','1','4','7', //'E'=>'-', 'F'=>'.'
	'H',',',' ','2','5','8', //'.'=>' '
	'R',':','C','3','6','9', // 'A'=>':'
	'P','/','L','$','S','X',//'B'=>'/','D'=>','
};
#endif
/* The definition of each message sent from tasks and interrupts to the LCD
task. */
typedef struct
{
	//uint64_t cMessageID;	/* << States what the message is. */
	char cMessageValue; /* << States the message value (can be an integer, string pointer, etc. depending on the value of cMessageID). */
} xQueueMessage;

typedef enum
{
  READ=0,
  SELECT,
  //CHANGE
}eTypePressX_t;
 typedef struct 
{
  bool bNumber;
  bool bP1_4;
  bool bNzzlHang;
}EnablePreset_t;

typedef struct _Number
{
  u8 leng;
  u8 leng_tp;
  uint64_t value;
  bool Dot;
}Number_t;
typedef struct
{
  u8 data[360];
}ReceivePrinterData_t;
typedef struct __STATUSLCD
{
    bool bHANGStatusLCD;
    bool bTakeOffStatusLCD;
    bool bFuelingStatusLCD;
    bool bEndedStatusLCD;
}StatusLCD_Typedef;
//void REFUELING_UpdateData(void);
void Clear_Error(void);
void LAPIS_DetectError(void);
void LAPIS_CheckCCC(void);
bool PRESET_CheckValid(u32 value,u8 lengtp,bool AV);
void PRESET_SendP1234(u8 key);
bool PRESET_Valid(u8 P);
void Send_CmdPrinter(void);
void PRESET_SendSelect(bool AorV);
void Reset_SomeValues(void);
void PRINTER_WaitForSendDone(DataSetup_t *data);
void LCD_DisplayFollowLanguage(u8 xv,u8 yv,u8 xe,u8 ye,int8_t *vietnamese,int8_t *english);
void Change_Values(eLoginMode_t mode,u8 arr[]);
void TIMER4_ENABLE(u32 Period);
void TM_TIMER4_Init(u32 Period);
void TIMER4_DISABLE(void);
void vLcdTask(void * pvParameters);
bool WaitTransmitDone(DataSetup_t *dt,bool ReceiverOrSend);
bool WaitReceiverDone(void);
bool Read_Total(void);
bool Ask_Log(void);
void LCD_MsgPresetInvalid(bool AorV);
void PRINTER_WaitPassword(u8 lengPW);
void xTimers_Init(u8 i,u16 period,TimerCallbackFunction_t callback);
void vTimerCallback_BlinkErr( TimerHandle_t xTimer );
void vTimerCallback_Displaytest( TimerHandle_t xTimer );
void vTimerCallback_Printer( TimerHandle_t xTimer);
void vTimerCallback_ClearCode( TimerHandle_t xTimer );
void vTimerCallback_UpdateState( TimerHandle_t xTimer );
//void vTimerCallback_PosLcd( TimerHandle_t xTimer );
void vTimerCallback_BlinkLedSys( TimerHandle_t xTimer );
bool SUNNYXE_ReadTotal_Logs(DataSetup_t dt);
bool SUNNYXE_CheckValidEnterCode(eLoginMode_t mode,u8 arr[],u8 code);
//bool WaitReceivedNzzlHang(void);
u8 SUNNYXE_FindCode(u8 lengArr,u8 arr[],u8 code);
u8 ID_P(u8 key);
void GPIO_TogglePin(GPIO_TypeDef* GPIOx, u16 GPIO_Pin);
void QueueSend_Err(SysStatus_t *sys,u8 code_chuc,u8 code_dv);
void Reset_Buffer_Keypad(void);
void Disable_SWD(AFIO_TypeDef *AFIO);
void Enable_SWD(AFIO_TypeDef *AFIO);
void LCD_SenFalseMessage(void);
void LCD_StateMessage(SysStatus_t *sys);
uint8_t GetNumberAfterDotVolume(SysStatus_t *sys);
void LCD_NozzleFromCPU(SysStatus_t *sysFromCPU);
//bool WaitPresetSendDone(TickType_t timeout);
#endif 