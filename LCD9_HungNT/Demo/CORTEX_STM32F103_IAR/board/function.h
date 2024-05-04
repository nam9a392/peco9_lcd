/**
  ******************************************************************************
  * @file    function.h
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

#ifndef	FUNCTION_H
#define	FUNCTION_H
  #include "pos.h"
   /*switch this define to 0-1 */
#define SUNNYXE_ADMIN_CHANGE_CODE_37       0
   
#define	DATA_LCD_PIN		GPIO_Pin_15
#define	DATA_LCD_PORT		GPIOB

#define	SHIFT_CLOCK_PIN		GPIO_Pin_13
#define	SHIFT_CLOCK_PORT	GPIOB

#define	LATCH_CLOCK_PIN		GPIO_Pin_14
#define	LATCH_CLOCK_PORT	GPIOB

#define	BLANK_PIN		GPIO_Pin_12
#define	BLANK_PORT		GPIOB

#define	TEST_PIN		GPIO_Pin_11
#define	TEST_PORT		GPIOB


#define	DATA_LCD_LOW		GPIO_WriteBit(DATA_LCD_PORT,DATA_LCD_PIN,Bit_RESET);
#define	DATA_LCD_HIGH		GPIO_WriteBit(DATA_LCD_PORT,DATA_LCD_PIN,Bit_SET);
#define	SHIFT_CLOCK_HIGH	GPIO_WriteBit(SHIFT_CLOCK_PORT,SHIFT_CLOCK_PIN,Bit_SET);
#define	SHIFT_CLOCK_LOW		GPIO_WriteBit(SHIFT_CLOCK_PORT,SHIFT_CLOCK_PIN,Bit_RESET);
#define	LATCH_HIGH      	GPIO_WriteBit(LATCH_CLOCK_PORT,LATCH_CLOCK_PIN,Bit_SET);
#define	LATCH_LOW		GPIO_WriteBit(LATCH_CLOCK_PORT,LATCH_CLOCK_PIN,Bit_RESET);
#define	BLANK_PIN_HIGH		GPIO_WriteBit(BLANK_PORT,BLANK_PIN,Bit_SET);
#define	BLANK_PIN_LOW		GPIO_WriteBit(BLANK_PORT,BLANK_PIN,Bit_RESET);
#define	TEST_PIN_HIGH		GPIO_WriteBit(TEST_PORT,TEST_PIN,Bit_SET);
#define	TEST_PIN_LOW		GPIO_WriteBit(TEST_PORT,TEST_PIN,Bit_RESET);

#define Unitprice_MaxLeng       5
#define Volume_MaxLeng          7
#define Amount_MaxLeng          7
typedef enum
{
  Mode_Read=0,
  Mode_Write
}RW_Typedef;
__packed typedef struct
{
  char  cSub[3];
  u8    iSub;
}TypeSubCode_t;
__packed typedef struct
{
  double db;
  u32   _u32;
  uint64_t _u64;
  u16   _u16[5];
  /*
  0: code 12;
  1: code 16
  2-5: code 24
  */
  u8    len_tp[6];              
}TypeValue_t;
__packed typedef struct
{
  u8            minNum;
  u32           maxNum;
  double        maxDbNum;
}RangeData_t;
void    LAPIS_DisplayTest(u8 j );
void 	LAPIS_Init(void);
void 	LAPIS_Clear(void);
void 	LATCH(void);
void 	SHIFT(void);
void 	LAPIS_DisplayNumber(u8 num);
//void	LAPIS_FULLDISPLAY(u8 Chuc_Nghin_A,u8 Nghin_A,u8 Tram_A,u8 Chuc_A,u8 Dv_A,u8 TP_Chuc_A,u8 TP_Dv_A,
//	u8 Nghin_VL,u8 Tram_VL,u8 Chuc_VL,u8 Dv_VL,u8 TP_Dv_VL,u8 TP_Chuc_VL,u8 TP_Tram_VL,u8 Tram_UP,u8 Chuc_UP,u8 Dv_UP,
//	u8	TP_Chuc_UP,u8 TP_Dv_UP,u8 Dot,eFuelingMode_t fuelMode,Errorcode_t errCode);
void    LAPIS_DISPLAY(SysStatus_t *status,eFuelingMode_t fuelMode);//,volatile SysConfig_t *config
//void    LAPIS_DISPLAY_NEW(SysStatus_t *status,SDisplay_t *displayStatus, DecimalPlaces_t *decimal,eFuelingMode_t fuelMode);//u8 Dot,
void    LAPIS_BusyState(void);
void	LAPIS_WaitPasswordState(u8 leng);
void 	LAPIS_WaitCodeState(void);
void    LAPIS_DisplayCode(void);
void 	Dots(u8 r1_dot,u8 r2_dot,u8 r3_dot);
void    sfDots(u8 n);
//void    reset_AmountVolume(void);
void reset_AmountVolume(SysStatus_t *sProcess);
void    sfDisplayValueChange(u8 pcode,TypeSubCode_t tScode,TypeValue_t tValue,u8 dot,bool valueDbOrInt,u8 sizeField);
void    LAPIS_DisplaySetup(eLoginMode_t mode,u8 pcode,u8 cntSub, u8 dot);
void    fDisplay(u8 pcode,TypeSubCode_t tScode,TypeValue_t tValue,u8 dot,bool valueDbOrInt,u8 size);
void    fDisplay24(u8 pcode,TypeSubCode_t tScode,TypeValue_t tValue,u8 dot,u8 cntScode,u8 selectPreset);
void    fDisplay37(u8 row);
void    fDisplay95(u8 pcode,TypeValue_t tValue);
void    LAPIS_ClearSegment(u8 len);
void    display_valueChange(u8 code,u8 num[],u8 len,u8 dot,u8 size,bool havedot);
void    sfEnterNewCode(u8 newcode,u8 len);
void    LAPIS_ChangeValue(u8 pcode,u8 cntSub,TypeValue_t tValue,u8 dot,u8 sizefield);
void    sf1(u8 leng,u8 lengtp,u8 digit[]);
void    sfRow1(u8 code,u8 index);
void    LAPIS_CheckNumberDisplay(u8 *arr,u8 leng,u8 from);
void    LAPIS_CheckDisplay(u8 * arr,u8 num1,u8 num2);
void    Display_ArrayNumber(u8 *arr,u8 from,u8 to,u8 index);
void	Split_Digit(u32 num,u8 *arr,u8 leng);
void    Row3(u8 d1,u8 d2,u8 d3, u8 d4);

u8              Split_Number(uint64_t x,u8 *arr);
u8              sumSubcode(u8 pcode,eLoginMode_t mode);
u8              getSizeFieldDataChange(u8 pcode,u8 cntSub);
TypeValue_t     getValue(u8 pcode,volatile SysConfig_t *config,u8 cntSub);
TypeSubCode_t   getSubcode(u8 pcode,u8 cntScode);
RangeData_t     getRangeDataNeedChange(u8 pcode,u8 dot,u8 cntScode);
TypeValue_t     setValue( eLoginMode_t mode,u8 pcode,u8 cntSub,double value,u8 len,u8 choosePreset);

#endif


