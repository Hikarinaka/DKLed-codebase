/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INIT_WS_H
#define __INIT_WS_H


#include "stm32f10x.h"
//#include <init_ws.c>

/* this define sets the number of TIM2 overflows to append to the data frame for the LEDs to load the received data into their registers */
#define WS2812_DEADPERIOD 19

/* WS2812 framebuffer buffersize = max#LEDs*24 /////(#LEDs / 16) * 24 */
#define WS2812_IO_FRAMEDATA_SIZE 512*24  //количество точек в любой из линий *24


void WS2812_GPIO_init( void );
void WS2812_Timer_init( void );
void WS2812_DMA_init( uint8_t WS2812_IO_framedata[] );
void WS2812_sendbuf( uint32_t buffersize );
//void DMA1_Channel4_IRQHandler( void );
void TIM2_IRQHandler( void );
void WS2812_framedata_setPixel(uint8_t WS2812_IO_framedata[], uint8_t row, uint16_t column, uint8_t red,
  uint8_t green, uint8_t blue );
void WS2812_framedata_setPixel_RGB (uint8_t WS2812_IO_framedata_[], uint8_t row,
		uint16_t column, uint32_t point_rgb);
void WS2812_framedata_setRow(uint8_t WS2812_IO_framedata[], uint8_t row, uint16_t columns, uint8_t red,
  uint8_t green, uint8_t blue );
void WS2812_framedata_setColumn(uint8_t WS2812_IO_framedata[], uint8_t rows, uint16_t column, uint8_t red,
  uint8_t green, uint8_t blue );
void WS2812_clear_buffer (uint8_t WS2812_IO_framedata_[], uint16_t buffersize);

#endif
