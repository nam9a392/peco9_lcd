/**/
/* Includes ------------------------------------------------------------------*/

#include "stm32f10x_conf.h"

#include "utility.h"
#include "math.h"

/** 
  * @{
  */ 
void ArrayCoppy(u8 *des, u8 *src, u16 len)
{
    u16 cnt = 0;
    while(cnt < len)
    {
        *(des++) = *(src++);
        cnt++;
    }
}

u8 LengthOfInt(u32 num)
{
  if(num==0) return 1;
 return (u8)floor(log10(num)) + 1;
}
void DigitsExtraction(u8 *array, u8 size, uint64_t num)
{

  for (int i = 0; i < size; ++i, num /= 10)
  {
    array[(size - 1) - i] = (num % 10) + 0x30;/* convert to char */
  }
}
void IntergerDigitsExtraction(u8 *array, u8 size, uint64_t num)
{
  
  for (int i = 0; i < size; ++i, num /= 10)
  {
    array[(size - 1) - i] = (num % 10) ;
  }
}
 uint64_t    stringToInt(u8* str,u8 len)
 {
   uint64_t result=0;
   u8 i;
    for(i=0; i<len; i++){

    result = result * 10 + ( *(str+i) - '0' );
}
return result;

 }
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

/**
  * @}
  */    

/**
  * @}
  */ 


  
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
