/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INIT_WS_H
#define __INIT_WS_H


#include "stm32f10x.h"
#include "includes.h"
//#include <init_ws.c>

/* this define sets the number of TIM2 overflows to append to the data frame for the LEDs to load the received data into their registers */
#define WS2812_DEADPERIOD 19

/* WS2812 framebuffer buffersize = max#LEDs*24 /////(#LEDs / 16) * 24 */
#define WS2812_IO_FRAMEDATA_PIXELS 576  //количество точек в любой из линий
#define WS2812_IO_FRAMEDATA_SIZE 576*24  //количество точек в любой из линий *24
#define BRIGHTNESS_SHIFT	5	//1=2, 2=4, 3=8, 4=0x10, 5=0x20, 6=0x40
#define MAX_BRIGHTNESS		0x20	//максимальная яркость
#define MAX_PALLETTE_SIZE	256 //аксимальный размер палитры


#define SERVO_TIMER_PRESC	72 //72 000 000 / 1 000 000 = 1 MHz		* scale
#define SERVO_FREQ_MIN		20 //Hz, 								/ scale
//50 is a minimum possible to stable compute with current prescaler value cause of a limit within TIM2->Arr that is only 2 byte long
#define SERVO_FREQ_MAX		1600 //Hz, 								/ scale for 16 pixels
#define SERVO_TICK_FREQ		1000000 // 1 000 000 = 1 MHz			/ scale
#define SERVO_PERIOD_DEF	20000 //1 000 000 / 50 Hz				/ scale
#define SERVO_POS_MIN_DEF	500 // 500 uS							/ scale
#define SERVO_POS_MAX_DEF	2500 // 2500 uS							/ scale

/* backup values
#define SERVO_TIMER_PRESC	72 //72 000 000 / 1 000 000 = 1 MHz
#define SERVO_FREQ_MIN		50 //Hz
#define SERVO_TICK_FREQ		1000000 // 1 000 000 = 1 MHz
#define SERVO_PERIOD_DEF	20000 //50 Hz
#define SERVO_POS_MIN_DEF	500 // 500 uS
#define SERVO_POS_MAX_DEF	2500 // 2500 uS
*/

void WS2812_GPIO_init( void );
void WS2812_Timer_init( void );
void WS2812_DMA_init( uint8_t WS2812_IO_framedata[] );
void WS2812_sendbuf( uint32_t buffersize );
void WS2812_SetStartPixelInBuffer( uint32_t buffersize );
void WS2812_ResetStartPixelInBuffer( void );
//void DMA1_Channel4_IRQHandler( void );
//void TIM2_IRQHandler( void );
void WS2812_framedata_setPixel(uint8_t WS2812_IO_framedata[], uint8_t row, uint16_t column, uint8_t red,
  uint8_t green, uint8_t blue );
void WS2812_framedata_setPixel_RGB (uint8_t WS2812_IO_framedata_[], uint8_t row,
		uint16_t column, uint32_t point_rgb, uint8_t Repeats);
void WS2812_framedata_SwapPixel (uint8_t WS2812_IO_framedata_[], uint8_t rowSRC, uint8_t rowDST,
		uint16_t columnSRC, uint16_t columnDST, uint8_t Swapmode, uint8_t Repeats, uint8_t Rep_Set);
void WS2812_framedata_MultiplyPixel_RGB (uint8_t WS2812_IO_framedata_[], uint8_t rowSRC, uint8_t rowDST,
		uint16_t columnSRC, uint16_t columnDST, uint8_t Repeats, uint8_t Rep_Set);
uint8_t WS2812_framedata_CheckPixel(uint8_t WS2812_IO_framedata_[], uint8_t row_bitmask,
		uint16_t column, uint32_t point_rgb, uint8_t Repeats);
void WS2812_framedata_setRow(uint8_t WS2812_IO_framedata[], uint8_t row, uint16_t columns, uint8_t red,
  uint8_t green, uint8_t blue );
void WS2812_framedata_setColumn(uint8_t WS2812_IO_framedata[], uint8_t rows, uint16_t column, uint8_t red,
  uint8_t green, uint8_t blue );
void WS2812_clear_buffer (uint8_t WS2812_IO_framedata_[], uint16_t buffersize, uint8_t row_bitmask);
void WS2812_framedata_Set_HighLow(uint8_t WS2812_IO_framedata_[], uint16_t buffersize);
#endif
