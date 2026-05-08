/**
  ******************************************************************************
  * @file    stm32f10x_crc.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    11-March-2011
  * @brief   This file provides all the CRC firmware functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_crc.h"

uint32_t CRC_sum; //CRC

//uint32_t CRC32_Polynomial = CRC32_POLY;
/** @addtogroup STM32F10x_StdPeriph_Driver
  * @{
  */

/** @defgroup CRC 
  * @brief CRC driver modules
  * @{
  */

/** @defgroup CRC_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @defgroup CRC_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup CRC_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup CRC_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup CRC_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

//вычисление CRC дл€ массива
//init_crc - CRC предыдущего куска
//*buf - указатель на 1 байт массива
//len - количество байт
uint32_t crc32_byte(uint32_t init_crc, uint8_t *buf, uint16_t len)
{
      /*  uint32_t v;
        uint32_t crc, cr;
        uint8_t j;
        crc = ~init_crc;
        while(len > 0) {
                v = *buf++;
                cr = ( crc ^ (v ) ) & 0xff;
                for (j = 8; j > 0; --j) {
                	cr = cr & 0x00000001 ? (cr >> 1) ^ CRC32_POLY_R : (cr >> 1);
                }
                crc = ( crc >> 8 ) ^ cr;
                len --;
        }
        return ~crc;*/

	uint32_t crc, cr;
	uint8_t v;
	crc = init_crc;
	while(len > 0){
		v = *buf++;

		CRC32_singleByte (&crc, &v);
		/*crc ^= (uint32_t)(v << 24); // move byte into MSB of 32bit CRC
		for (i = 0; i < 8; i++){
			if ((crc & 0x80000000) != 0) {// test for MSB = bit 31
				crc = (uint32_t)((crc << 1) ^ CRC32_POLY);
			} else {
				crc <<= 1;
			}
		}*/
		len -= 1;
	}
	return crc;
}

//добавление нового байта в CRC к предыдущему
//crc_ - указатель на текущую €чейку
//v - байт
void CRC32_singleByte (uint32_t *crc_, uint8_t *v_){
*crc_ ^= (uint32_t)(*v_ << 24); /* move byte into MSB of 32bit CRC */
uint8_t i;
for (i = 0; i < 8; i++){
	if ((*crc_ & 0x80000000) != 0) {/* test for MSB = bit 31 */
		*crc_ = (uint32_t)((*crc_ << 1) ^ CRC32_POLY);
	} else {
		*crc_ <<= 1;
	}
}
}
/** @defgroup CRC_Private_Functions
  * @{
  */

/**
  * @brief  Resets the CRC Data register (DR).
  * @param  None
  * @retval None
  */
void CRC_ResetDR(void)
{
  /* Reset CRC generator */
  CRC->CR = CRC_CR_RESET;
}

/**
  * @brief  Computes the 32-bit CRC of a given data word(32-bit).
  * @param  Data: data word(32-bit) to compute its CRC
  * @retval 32-bit CRC
  */
uint32_t CRC_CalcCRC(uint32_t Data)
{
  CRC->DR = Data;
  
  return (CRC->DR);
}

/**
  * @brief  Computes the 32-bit CRC of a given buffer of data word(32-bit).
  * @param  pBuffer: pointer to the buffer containing the data to be computed
  * @param  BufferLength: length of the buffer to be computed					
  * @retval 32-bit CRC
  */
uint32_t CRC_CalcBlockCRC(uint32_t pBuffer[], uint32_t BufferLength)
{
  uint32_t index = 0;
  
  for(index = 0; index < BufferLength; index++)
  {
    CRC->DR = pBuffer[index];
  }
  return (CRC->DR);
}

/**
  * @brief  Returns the current CRC value.
  * @param  None
  * @retval 32-bit CRC
  */
uint32_t CRC_GetCRC(void)
{
  return (CRC->DR);
}

/**
  * @brief  Stores a 8-bit data in the Independent Data(ID) register.
  * @param  IDValue: 8-bit value to be stored in the ID register 					
  * @retval None
  */
void CRC_SetIDRegister(uint8_t IDValue)
{
  CRC->IDR = IDValue;
}

/**
  * @brief  Returns the 8-bit data stored in the Independent Data(ID) register
  * @param  None
  * @retval 8-bit value of the ID register 
  */
uint8_t CRC_GetIDRegister(void)
{
  return (CRC->IDR);
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

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
