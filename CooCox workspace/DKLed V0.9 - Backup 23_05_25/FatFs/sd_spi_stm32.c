/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module for STM32 Version 1.1.6    */
/* (C) Martin Thomas, 2010 - based on the AVR MMC module (C)ChaN, 2007   */
/*-----------------------------------------------------------------------*/

/* Copyright (c) 2010, Martin Thomas, ChaN
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */


#include "stm32f10x.h"
#include "ffconf.h"
#include "diskio.h"
#include "stm32f10x_spi.h"
//*===============================================================================================
//*			допиливаем модуль
//*===============================================================================================
#include <stdint.h>
#include <stdbool.h>
#include "coocox.h"
#include "board.h"
#define TRUE	true
#define FALSE	false
#define BOOL	_Bool
#define	RAMFUNC

/*
 *	ƒоработаны функции socket_is_empty() и socket_is_write_protected():
 *  - изменена пол€рность контролируемого логического уровн€ дл€ Olimex STM32-P103л
 */

// demo uses a command line option to define this (see Makefile):
#define STM32_SD_USE_DMA


#ifdef STM32_SD_USE_DMA
// #warning "Information only: using DMA"
#pragma message "*** Using DMA ***"
#endif

/* set to 1 to provide a disk_ioctrl function even if not needed by the FatFs */
#define STM32_SD_DISK_IOCTRL_FORCE      0

// demo uses a command line option to define this (see Makefile):
//#define USE_EK_STM32F
#define USE_STM32_P103
//#define USE_MINI_STM32


#if defined(USE_EK_STM32F)
 #define CARD_SUPPLY_SWITCHABLE   1
 #define GPIO_PWR                 GPIOD
 #define RCC_APB2Periph_GPIO_PWR  RCC_APB2Periph_GPIOD
 #define GPIO_Pin_PWR             GPIO_Pin_10
 #define GPIO_Mode_PWR            GPIO_Mode_Out_OD /* pull-up resistor at power FET */
 #define SOCKET_WP_CONNECTED      0
 #define SOCKET_CP_CONNECTED      0
 #define SPI_SD                   SPI1
 #define GPIO_CS                  GPIOD
 #define RCC_APB2Periph_GPIO_CS   RCC_APB2Periph_GPIOD
 #define GPIO_Pin_CS              GPIO_Pin_9
 #define DMA_Channel_SPI_SD_RX    DMA1_Channel2
 #define DMA_Channel_SPI_SD_TX    DMA1_Channel3
 #define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC2
 #define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC3
 #define GPIO_SPI_SD              GPIOA
 #define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_5
 #define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_6
 #define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_7
 #define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB2PeriphClockCmd
 #define RCC_APBPeriph_SPI_SD     RCC_APB2Periph_SPI1
 /* - for SPI1 and full-speed APB2: 72MHz/4 */
 #define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_4

#elif defined(USE_STM32_P103)
 // Olimex STM32-P103 not tested!
 #define CARD_SUPPLY_SWITCHABLE   0
 #define SOCKET_WP_CONNECTED      0 /* write-protect socket-switch */
 #define SOCKET_CP_CONNECTED      0 /* card-present socket-switch */
 #define GPIO_WP                  GPIOA
 #define GPIO_CP                  GPIOC
 #define RCC_APBxPeriph_GPIO_WP   RCC_APB2Periph_GPIOA
 #define RCC_APBxPeriph_GPIO_CP   RCC_APB2Periph_GPIOC
 #define GPIO_Pin_WP              GPIO_Pin_1 //ещЄ один пин, конфликтующий с кнопками.
 #define GPIO_Pin_CP              GPIO_Pin_13
 #define GPIO_Mode_WP             GPIO_Mode_IN_FLOATING /* external resistor */
 #define GPIO_Mode_CP             GPIO_Mode_IN_FLOATING /* external resistor */
 #define SPI_SD                   SPI1
 #define GPIO_CS                  GPIOB
 #define RCC_APB2Periph_GPIO_CS   RCC_APB2Periph_GPIOB
 #define GPIO_Pin_CS              GPIO_Pin_2 //было 0, но 0 это кнопка
 #define DMA_Channel_SPI_SD_RX    DMA1_Channel2
 #define DMA_Channel_SPI_SD_TX    DMA1_Channel3
 #define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC2
 #define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC3
 #define GPIO_SPI_SD              GPIOA
 #define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_5 //5,6,7 - пины управлени€ картой пам€ти
 #define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_6
 #define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_7
 #define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB2PeriphClockCmd
 #define RCC_APBPeriph_SPI_SD     RCC_APB2Periph_SPI1
 /* for SPI2 and full-speed APB1: 36MHz/2 */
 /* !! PRESCALE 4 used here - 2 does not work, maybe because
       of the poor wiring on the HELI_V1 prototype hardware */
 #define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_4
//#define SPI_BaudRatePrescaler_SPI_SD	SPI_BaudRatePrescaler_2

#elif defined(USE_MINI_STM32)
 #define CARD_SUPPLY_SWITCHABLE   0
 #define SOCKET_WP_CONNECTED      0
 #define SOCKET_CP_CONNECTED      0
 #define SPI_SD                   SPI1
 #define GPIO_CS                  GPIOB
 #define RCC_APB2Periph_GPIO_CS   RCC_APB2Periph_GPIOB
 #define GPIO_Pin_CS              GPIO_Pin_6
 #define DMA_Channel_SPI_SD_RX    DMA1_Channel2
 #define DMA_Channel_SPI_SD_TX    DMA1_Channel3
 #define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC2
 #define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC3
 #define GPIO_SPI_SD              GPIOA
 #define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_5
 #define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_6
 #define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_7
 #define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB2PeriphClockCmd
 #define RCC_APBPeriph_SPI_SD     RCC_APB2Periph_SPI1
 /* - for SPI1 and full-speed APB2: 72MHz/4 */
 #define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_4

#else
#error "unsupported board"
#endif


/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */

/* Card-Select Controls  (Platform dependent) */
#define SELECT()        GPIO_CS->BRR = GPIO_Pin_CS;//GPIO_ResetBits(GPIO_CS, GPIO_Pin_CS)    /* MMC CS = L */
#define DESELECT()      GPIO_CS->BSRR = GPIO_Pin_CS;//GPIO_SetBits(GPIO_CS, GPIO_Pin_CS)      /* MMC CS = H */

/* Manley EK-STM32F board does not offer socket contacts -> dummy values: */
#define SOCKPORT	1			/* Socket contact port */
#define SOCKWP		0			/* Write protect switch (PB5) */
#define SOCKINS		0			/* Card detect switch (PB4) */

#if (_MAX_SS != 512) || (_FS_READONLY == 0) || (STM32_SD_DISK_IOCTRL_FORCE == 1)
#define STM32_SD_DISK_IOCTRL   1
#else
#define STM32_SD_DISK_IOCTRL   0
#endif

/*--------------------------------------------------------------------------

   Module Private Functions and Variables

---------------------------------------------------------------------------*/

static const DWORD socket_state_mask_cp = 1;//(1 << 0);
static const DWORD socket_state_mask_wp = 2;//(1 << 1);

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
DWORD Timer1, Timer2;	/* 100Hz decrement timers */

static
BYTE CardType;			/* Card type flags */

enum speed_setting { INTERFACE_SLOW, INTERFACE_FAST };

static void interface_speed( enum speed_setting speed )
{
	DWORD tmp;

	tmp = SPI_SD->CR1;
	if ( speed == INTERFACE_SLOW ) {
		/* Set slow clock (100k-400k) */
		//printf("INTERFACE_SLOW\r\n"); //vet
		tmp = ( tmp | SPI_BaudRatePrescaler_256 );
	} else {
		/* Set fast clock (depends on the CSD) */
		//printf("INTERFACE_FAST\r\n"); //vet
		tmp = ( tmp & ~SPI_BaudRatePrescaler_256 ) | SPI_BaudRatePrescaler_SPI_SD;
	}
	SPI_SD->CR1 = tmp;
}

//==============================================================================================
#if SOCKET_WP_CONNECTED
/* Socket's Write-Protection Pin: high = write-protected, low = writable */

void socket_wp_init(void)
//static void socket_wp_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure I/O for write-protect */
	RCC_APB2PeriphClockCmd(RCC_APBxPeriph_GPIO_WP, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_WP;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_WP;
	GPIO_Init(GPIO_WP, &GPIO_InitStructure);
}

static DWORD socket_is_write_protected(void)
{
	//return ( GPIO_ReadInputData(GPIO_WP) & GPIO_Pin_WP ) ? socket_state_mask_wp : 0;
	return ( GPIO_ReadInputData(GPIO_WP) & GPIO_Pin_WP ) ? 0: socket_state_mask_wp;
}

#else

static void socket_wp_init(void)
{
	return;
}

static inline DWORD socket_is_write_protected(void)
{
	return 0; /* fake not protected */
}

#endif /* SOCKET_WP_CONNECTED */


//==========================================================================================
#if SOCKET_CP_CONNECTED
/* Socket's Card-Present Pin: high = socket empty, low = card inserted */

//static void socket_cp_init(void)
void socket_cp_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure I/O for card-present */
	RCC_APB2PeriphClockCmd(RCC_APBxPeriph_GPIO_CP, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_CP;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_CP;
	GPIO_Init(GPIO_CP, &GPIO_InitStructure);
}

static inline DWORD socket_is_empty(void)
{
	//если карта есть - на входе в порт низкое напр€жение
	//return ( GPIO_ReadInputData(GPIO_CP) & GPIO_Pin_CP ) ? socket_state_mask_cp : FALSE;
	//на некоторых бордах (например Olimex) сигнал инверсный, вот дл€ него:
	return ( GPIO_ReadInputData(GPIO_CP) & GPIO_Pin_CP ) ? FALSE: socket_state_mask_cp;
}

#else

static void socket_cp_init(void)
{
	return;
}

static inline DWORD socket_is_empty(void)
{
	return 0; /* fake inserted */
}

#endif /* SOCKET_CP_CONNECTED */


#if CARD_SUPPLY_SWITCHABLE

static void card_power(BOOL on)		/* switch FET for card-socket VCC */
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Turn on GPIO for power-control pin connected to FET's gate */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_PWR, ENABLE);
	/* Configure I/O for Power FET */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_PWR;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_PWR;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_PWR, &GPIO_InitStructure);
	if (on) {
		GPIO_ResetBits(GPIO_PWR, GPIO_Pin_PWR);
	} else {
		/* Chip select internal pull-down (to avoid parasite powering) */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS;
		GPIO_Init(GPIO_CS, &GPIO_InitStructure);

		GPIO_SetBits(GPIO_PWR, GPIO_Pin_PWR);
	}
}

#if (STM32_SD_DISK_IOCTRL == 1)
static int chk_power(void)		/* Socket power state: 0=off, 1=on */
{
	if ( GPIO_ReadOutputDataBit(GPIO_PWR, GPIO_Pin_PWR) == Bit_SET ) {
		return 0;
	} else {
		return 1;
	}
}
#endif

#else

static void card_power(BYTE on)
{
	//on=on;	парсер ошибок недоволен такой конструкцией, закомментарим
}

#if (STM32_SD_DISK_IOCTRL == 1)
static int chk_power(void)
{
	return 1; /* fake powered */
}
#endif

#endif /* CARD_SUPPLY_SWITCHABLE */


/*-----------------------------------------------------------------------*/
/* Transmit/Receive a byte to MMC via SPI  (Platform dependent)          */
/*-----------------------------------------------------------------------*/
static BYTE stm32_spi_rw( BYTE out )
{
	/* Loop while DR register in not empty */
	/// not needed: while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_TXE) == RESET) { ; }

	/* Send byte through the SPI peripheral */
	//SPI_I2S_SendData(SPI_SD, out);
	SPI_SD->DR = (uint16_t) out;

	/* Wait to receive a byte */
	//while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_RXNE) == RESET) { ; }
	while (!(SPI_SD->SR & SPI_I2S_FLAG_RXNE)) { ; }

	/* Return the byte read from the SPI bus */
	//return SPI_I2S_ReceiveData(SPI_SD);
	return (BYTE) SPI_SD->DR;
}



/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

#define xmit_spi(dat)  stm32_spi_rw(dat)

/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static
BYTE rcvr_spi (void)
{
	return stm32_spi_rw(0xff);
}

/* Alternative macro to receive data fast */
#define rcvr_spi_m(dst)  *(dst)=stm32_spi_rw(0xff)



/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE wait_ready (void)
{
	BYTE res;


	Timer2 = 50;	/* Wait for ready in timeout of 500ms */
	rcvr_spi();
	do
		res = rcvr_spi();
	while ((res != 0xFF) && Timer2);

	return res;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	//DESELECT();
	rcvr_spi();
}

#ifdef STM32_SD_USE_DMA
/*-----------------------------------------------------------------------*/
/* Transmit/Receive Block using DMA (Platform dependent. STM32 here)     */
/*-----------------------------------------------------------------------*/
static
void stm32_dma_transfer(
	BOOL receive,		/* FALSE for buff->SPI, TRUE for SPI->buff               */
	const BYTE *buff,	/* receive TRUE  : 512 byte data block to be transmitted
						   receive FALSE : Data buffer to store received data    */
	UINT btr 			/* receive TRUE  : Byte count (must be multiple of 2)
						   receive FALSE : Byte count (must be 512)              */
)
{
	//printf("DMAstart/r/n");  //v
//	DMA_InitTypeDef DMA_InitStructure;
	WORD rw_workbyte[] = { 0xffff };

	/* shared DMA configuration values */
/*	DMA_InitStructure.DMA_PeripheralBaseAddr = (DWORD)(&(SPI_SD->DR));
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//0
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//0
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//0
	DMA_InitStructure.DMA_BufferSize = btr;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //0
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //00003000
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;// 0*/

	//DMA_DeInit(DMA_Channel_SPI_SD_RX);
	DMA_Channel_SPI_SD_RX->CCR &= (uint16_t)(~DMA_CCR1_EN);
	DMA_Channel_SPI_SD_RX->CCR  = 0;
	DMA_Channel_SPI_SD_RX->CNDTR = 0;
	DMA_Channel_SPI_SD_RX->CPAR  = 0;
	DMA_Channel_SPI_SD_RX->CMAR = 0;
	DMA1->IFCR |= DMA1_Channel2_IT_Mask;

	//DMA_DeInit(DMA_Channel_SPI_SD_TX);
	DMA_Channel_SPI_SD_TX->CCR &= (uint16_t)(~DMA_CCR1_EN);
	DMA_Channel_SPI_SD_TX->CCR  = 0;
	DMA_Channel_SPI_SD_TX->CNDTR = 0;
	DMA_Channel_SPI_SD_TX->CPAR  = 0;
	DMA_Channel_SPI_SD_TX->CMAR = 0;
	DMA1->IFCR |= DMA1_Channel3_IT_Mask;

	if ( receive ) {

		/* DMA1 channel2 configuration SPI1 RX ---------------------------------------------*/
		/* DMA1 channel4 configuration SPI2 RX ---------------------------------------------*/
		/*DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)buff;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//0
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//00000080
		DMA_Init(DMA_Channel_SPI_SD_RX, &DMA_InitStructure);*/
		DMA_Channel_SPI_SD_RX->CCR &= CCR_CLEAR_Mask;
		DMA_Channel_SPI_SD_RX->CCR |=  0x00003080;
		DMA_Channel_SPI_SD_RX->CNDTR = btr;
		DMA_Channel_SPI_SD_RX->CPAR = (DWORD)(&(SPI_SD->DR));
		DMA_Channel_SPI_SD_RX->CMAR = (DWORD)buff;

		/* DMA1 channel3 configuration SPI1 TX ---------------------------------------------*/
		/* DMA1 channel5 configuration SPI2 TX ---------------------------------------------*/
		/*DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)rw_workbyte;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//00000010
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;//0
		DMA_Init(DMA_Channel_SPI_SD_TX, &DMA_InitStructure);*/
		DMA_Channel_SPI_SD_TX->CCR &= CCR_CLEAR_Mask;
		DMA_Channel_SPI_SD_TX->CCR |=  0x00003010;
		DMA_Channel_SPI_SD_TX->CNDTR = btr;
		DMA_Channel_SPI_SD_TX->CPAR = (DWORD)(&(SPI_SD->DR));
		DMA_Channel_SPI_SD_TX->CMAR = (DWORD)rw_workbyte;

	} else {

#if _FS_READONLY == 0
		/* DMA1 channel2 configuration SPI1 RX ---------------------------------------------*/
		/* DMA1 channel4 configuration SPI2 RX ---------------------------------------------*/
		/*DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)rw_workbyte;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//0
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;//0
		DMA_Init(DMA_Channel_SPI_SD_RX, &DMA_InitStructure);*/

		DMA_Channel_SPI_SD_RX->CCR &= CCR_CLEAR_Mask;
		DMA_Channel_SPI_SD_RX->CCR |=  0x00003000;
		DMA_Channel_SPI_SD_RX->CNDTR = btr;
		DMA_Channel_SPI_SD_RX->CPAR = (DWORD)(&(SPI_SD->DR));
		DMA_Channel_SPI_SD_RX->CMAR = (DWORD)rw_workbyte;

		/* DMA1 channel3 configuration SPI1 TX ---------------------------------------------*/
		/* DMA1 channel5 configuration SPI2 TX ---------------------------------------------*/
		/*DMA_InitStructure.DMA_MemoryBaseAddr = (DWORD)buff;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//00000010
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//00000080
		DMA_Init(DMA_Channel_SPI_SD_TX, &DMA_InitStructure);*/

		DMA_Channel_SPI_SD_TX->CCR &= CCR_CLEAR_Mask;
		DMA_Channel_SPI_SD_TX->CCR |=  0x00003090;
		DMA_Channel_SPI_SD_TX->CNDTR = btr;
		DMA_Channel_SPI_SD_TX->CPAR = (DWORD)(&(SPI_SD->DR));
		DMA_Channel_SPI_SD_TX->CMAR = (DWORD)buff;

#endif

	}

	/* Enable DMA RX Channel */
	//DMA_Cmd(DMA_Channel_SPI_SD_RX, ENABLE);
	DMA_Channel_SPI_SD_RX->CCR |= DMA_CCR1_EN;
	/* Enable DMA TX Channel */
	//DMA_Cmd(DMA_Channel_SPI_SD_TX, ENABLE);
	DMA_Channel_SPI_SD_TX->CCR |= DMA_CCR1_EN;


	/* Enable SPI TX/RX request */
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

	/* Wait until DMA1_Channel 3 Transfer Complete */
	/// not needed: while (DMA_GetFlagStatus(DMA_FLAG_SPI_SD_TC_TX) == RESET) { ; }
	/* Wait until DMA1_Channel 2 Receive Complete */
	//while (DMA_GetFlagStatus(DMA_FLAG_SPI_SD_TC_RX) == RESET) { ; }
	while (!(DMA1->ISR & DMA_FLAG_SPI_SD_TC_RX)) { ; }



	// same w/o function-call:
	// while ( ( ( DMA1->ISR ) & DMA_FLAG_SPI_SD_TC_RX ) == RESET ) { ; }

	/* Disable DMA RX Channel */
	//DMA_Cmd(DMA_Channel_SPI_SD_RX, DISABLE);
	DMA_Channel_SPI_SD_RX->CCR &= (uint16_t)(~DMA_CCR1_EN);
	/* Disable DMA TX Channel */
	//DMA_Cmd(DMA_Channel_SPI_SD_TX, DISABLE);
	DMA_Channel_SPI_SD_TX->CCR &= (uint16_t)(~DMA_CCR1_EN);

	/* Disable SPI RX/TX request */
	SPI_I2S_DMACmd(SPI_SD, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);
}
#endif /* STM32_SD_USE_DMA */


/*-----------------------------------------------------------------------*/
/* Power Control and interface-initialization (Platform dependent)       */
/*-----------------------------------------------------------------------*/

static
void power_on (void)
{
//	SPI_InitTypeDef  SPI_InitStructure;
//	GPIO_InitTypeDef GPIO_InitStructure;
	//volatile BYTE dummyread;
	volatile BYTE dummyread __attribute__ ((unused));	// убираем предупреждение компил€тора

	/* Enable GPIO clock for CS */
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CS, ENABLE);
	RCC->APB2ENR |= RCC_APB2Periph_GPIO_CS;


	/* Enable SPI clock, SPI1: APB2, SPI2: APB1 */
	//RCC_APBPeriphClockCmd_SPI_SD(RCC_APBPeriph_SPI_SD, ENABLE);
	RCC->APB2ENR |= RCC_APBPeriph_SPI_SD;

	//card_power(1);
	//socket_cp_init();//там всЄ равно пусто
	//socket_wp_init();

	for (Timer1 = 25; Timer1; );	/* Wait for 250ms */

	/* Configure I/O for Flash Chip select */
//	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_CS;
//	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIO_CS, &GPIO_InitStructure);
	GPIO_CS->CRL &= 0xFFFFF0FF; //переместились на пин 2
	GPIO_CS->CRL |= 0x00000300;

	/* De-select the Card: Chip Select high */
	//DESELECT();

	/* Configure SPI pins: SCK and MOSI with default alternate function (not re-mapped) push-pull */
//	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_SPI_SD_SCK | GPIO_Pin_SPI_SD_MOSI;//5,7
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
//	GPIO_Init(GPIO_SPI_SD, &GPIO_InitStructure);
	/* Configure MISO as Input with internal pull-up */
//	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_SPI_SD_MISO;//6
//	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
//	GPIO_Init(GPIO_SPI_SD, &GPIO_InitStructure);
	GPIO_SPI_SD->CRL &= 0x000FFFFF;
	GPIO_SPI_SD->CRL |= 0xB8B00000;
	GPIO_SPI_SD->BSRR = 0x00000040;

	/* SPI configuration */
//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; // 72000kHz/256=281kHz < 400kHz
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//	SPI_InitStructure.SPI_CRCPolynomial = 7;

	/*---------------------------- SPIx CR1 Configuration ------------------------*/
	  /* Get the SPIx CR1 value */
	  uint16_t tmpreg = SPI_SD->CR1;
	  /* Clear BIDIMode, BIDIOE, RxONLY, SSM, SSI, LSBFirst, BR, MSTR, CPOL and CPHA bits */
	  tmpreg &= CR1_CLEAR_Mask;
	  //tmpreg |= (uint16_t)((uint32_t)SPI_InitStruct->SPI_Direction | SPI_InitStruct->SPI_Mode |
	  //                SPI_InitStruct->SPI_DataSize | SPI_InitStruct->SPI_CPOL |
	   //               SPI_InitStruct->SPI_CPHA | SPI_InitStruct->SPI_NSS |
	   //               SPI_InitStruct->SPI_BaudRatePrescaler | SPI_InitStruct->SPI_FirstBit);

	  //tmpreg |= 0x0000 |  0x0104 |0x0000|0x0000|0x0000|0x0200 |0x0038 |0x0000;

	  tmpreg |= 0x033C;//0x0104|0x0200|0x0038
	  SPI_SD->CR1 = tmpreg;
	  SPI_SD->I2SCFGR &= SPI_Mode_Select;

	/*---------------------------- SPIx CRCPOLY Configuration --------------------*/
	  /* Write to SPIx CRCPOLY */
	  //SPI_SD->CRCPR = SPI_InitStruct->SPI_CRCPolynomial;
	  SPI_SD->CRCPR =7;

	//SPI_Init(SPI_SD, &SPI_InitStructure);
	//SPI_CalculateCRC(SPI_SD, DISABLE);
	SPI_SD->CR1 &= CR1_CRCEN_Reset;
	//SPI_Cmd(SPI_SD, ENABLE);
	SPI_SD->CR1 |= CR1_SPE_Set;

	/* drain SPI */
	while (SPI_I2S_GetFlagStatus(SPI_SD, SPI_I2S_FLAG_TXE) == RESET) { ; }
	dummyread = SPI_I2S_ReceiveData(SPI_SD);

#ifdef STM32_SD_USE_DMA
	/* enable DMA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#endif
}

static
void power_off (void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;

	if (!(Stat & STA_NOINIT)) {
		//SELECT();
		wait_ready();
		release_spi();
	}

	//SPI_I2S_DeInit(SPI_SD);//SPI_SD = SPI 1
    /* Enable SPI1 reset state */
    //RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC->APB2RSTR |= RCC_APB2Periph_SPI1;
    /* Release SPI1 from reset state */
    //RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);
    RCC->APB2RSTR &= ~RCC_APB2Periph_SPI1;

	//SPI_Cmd(SPI_SD, DISABLE);
	SPI_SD->CR1 &= CR1_SPE_Reset;
	//RCC_APBPeriphClockCmd_SPI_SD(RCC_APBPeriph_SPI_SD, DISABLE);
    RCC->APB2ENR &= ~RCC_APBPeriph_SPI_SD;

	/* All SPI-Pins to input with weak internal pull-downs */
//	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_SPI_SD_SCK | GPIO_Pin_SPI_SD_MISO | GPIO_Pin_SPI_SD_MOSI;//5,6,7
//	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
//	GPIO_Init(GPIO_SPI_SD, &GPIO_InitStructure);
	GPIO_SPI_SD->CRL &= 0x000FFFFF;
	GPIO_SPI_SD->CRL |= 0x88800000;
	GPIO_SPI_SD->BRR = 0x000000E0;

	//card_power(0);

	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
}


/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
BOOL rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;


	Timer1 = 10;
	do {							/* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);
	if(token != 0xFE) return FALSE;	/* If not valid data token, return with error */

#ifdef STM32_SD_USE_DMA
	stm32_dma_transfer( TRUE, buff, btr );
#else
	do {							/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 4);
#endif /* STM32_SD_USE_DMA */

	rcvr_spi();						/* Discard CRC */
	rcvr_spi();

	return TRUE;					/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _FS_READONLY == 0
static
BOOL xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE resp;
#ifndef STM32_SD_USE_DMA
	BYTE wc;
#endif

	if (wait_ready() != 0xFF) return FALSE;

	xmit_spi(token);					/* transmit data token */
	if (token != 0xFD) {	/* Is data token */

#ifdef STM32_SD_USE_DMA
		stm32_dma_transfer( FALSE, buff, 512 );
#else
		wc = 0;
		do {							/* transmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (--wc);
#endif /* STM32_SD_USE_DMA */

		xmit_spi(0xFF);					/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();				/* Receive data response */
		if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
			return FALSE;
	}

	return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	//DESELECT();
	//SELECT();
	if (wait_ready() != 0xFF) {
		return 0xFF;
	}

	/* Send command packet */
	xmit_spi(cmd);						/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);
	//printf("CRC,%X ,", n); //vet

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */

	n = 10;								/* Wait for a valid response in timeout of 10 attempts */

	//printf("\r\n"); //vet
	do{
		res = rcvr_spi();
		//printf("res=,%X ,", res); //vet
		}
	while ((res & 0x80) && --n);
	//printf("\r\n"); //vet
	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
	BYTE n, cmd, ty, ocr[4];

	//if (drv) return STA_NOINIT;			/* Supports only single drive */

	/*if (Stat & STA_NODISK)
		{
		return Stat;	// No card in the socket
		}*/

	power_on();							/* Force socket power on and initialize interface */
	//interface_speed(INTERFACE_SLOW);
	SPI_SD->CR1 = ( SPI_SD->CR1 | SPI_BaudRatePrescaler_256 );

	for (n = 10; n; n--)
		/*printf("rcvr_spi()=%x\r\n",*/ rcvr_spi();	/* 80 dummy clocks */ //vet

	ty = 0;
	//printf("send_cmd(CMD0, 0) = %d\n\r", send_cmd(CMD0, 0)); //vet
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		//printf("send_cmd(CMD0, 0) == 1\r\n"); //vet
		Timer1 = 100;						/* Initialization timeout of 1000 milliseconds */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 response */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at VDD range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
			//printf("/* SDHC */, ty = %d\n\r", ty); //vet
		} else {							/* SDSC or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDSC */
				//printf("send_cmd(ACMD41, 0) <= 1,  ty = %d\n\r", ty); //vet
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMC */
				//printf("send_cmd(ACMD41, 0) <= 1  ---> else,  ty = %d\n\r", ty); //vet
			}
			while (Timer1 && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
			//printf("???, ty = %d\n\r", ty); //vet

		}
	}
	CardType = ty;
	release_spi();

	if (ty) {			/* Initialization succeeded */
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
		//printf("Stat &= ~STA_NOINIT, ty = %d", ty); //vet
		//interface_speed(INTERFACE_FAST);
		SPI_SD->CR1 = ( SPI_SD->CR1 & ~SPI_BaudRatePrescaler_256 ) | SPI_BaudRatePrescaler_SPI_SD;

	} else {			/* Initialization failed */
		//printf("power_off(), ty = %d\n\r", ty); //vet
		power_off();
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block read */
		if (send_cmd(CMD17, sector) == 0)	{ /* READ_SINGLE_BLOCK */
			if (rcvr_datablock(buff, 512)) {
				count = 0;
			}
		}
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) {
					break;
				}
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _FS_READONLY == 0

DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	//if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if (STM32_SD_DISK_IOCTRL == 1)
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;

	if (drv) return RES_PARERR;

	res = RES_ERROR;

	if (ctrl == CTRL_POWER) {
		switch (*ptr) {
		case 0:		/* Sub control code == 0 (POWER_OFF) */
			//if (chk_power())
				power_off();		/* Power off */
			res = RES_OK;
			break;
		case 1:		/* Sub control code == 1 (POWER_ON) */
			power_on();				/* Power on */
			res = RES_OK;
			break;
		case 2:		/* Sub control code == 2 (POWER_GET) */
			*(ptr+1) = 1;//(BYTE)chk_power();
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
	}
	else {
		if (Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			//SELECT();
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC version 2.00 */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* SDC version 1.XX or MMC*/
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			if (CardType & CT_SD2) {	/* SDC version 2.00 */
				if (send_cmd(ACMD13, 0) == 0) {	/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDC version 1.XX or MMC */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {	/* SDC version 1.XX */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMC */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0		/* READ_CSD */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0		/* READ_CID */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 4; n; n--) *ptr++ = rcvr_spi();
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD status as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(ptr, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
		}

		release_spi();
	}

	return res;
}
#endif /* _USE_IOCTL != 0 */


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

RAMFUNC void disk_timerproc (void)
{
//	static DWORD pv;
//	DWORD ns;
	BYTE n;//, s;

	n = Timer1;                // 100Hz decrement timers
	if (n) Timer1 = --n;	//уменьшаем таймеры на 1
	n = Timer2;				//через кэш, поскольку к таймерам есть доступ отовсюду
	if (n) Timer2 = --n;	//иначе можно случайно вылететь за пределы если между проверкой (таймер>0) и декрементом произойдЄт прерывание

/*	ns = pv;
	pv = 0; //socket_is_empty() | socket_is_write_protected();	// Sample socket switch они у нас не мен€ютс€, эти каоманды возвращают нули

//	if (ns == pv) {                         // Have contacts stabled?
		s = Stat;

//		if (pv & socket_state_mask_wp)      // WP is H (write protected)
//			s |= STA_PROTECT;
//		else                                // WP is L (write enabled) мы всегда тут
			s &= ~STA_PROTECT;

//		if (pv & socket_state_mask_cp)      // INS = H (Socket empty)
//			s |= (STA_NODISK | STA_NOINIT);
//		else                                // INS = L (Card inserted) мы всегда тут
			s &= ~STA_NODISK;

		Stat = s;
//	}
		/**/

	//Stat &= ~(STA_NODISK | STA_PROTECT);//на вс€кий случай, хот€ эти биты и так не торгаютс€
}
