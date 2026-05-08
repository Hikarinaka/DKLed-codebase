/*************************************************************************************************
 * @file		board.h
 *
 * @brief		Модуль описания периферии контроллера
 *
 * @version		v1.0
 * @date		03.09.2013
 * @author		Mike Smith
 *
 *************************************************************************************************/
#ifndef BOARD_H_
#define BOARD_H_


#define TEST_PIN_INIT do{\
						TEST_PIN1_LOW;\
						GPIOD->CRL &= ~GPIO_CRL_CNF2;\
						GPIOD->CRL |= GPIO_CRL_MODE2;\
						TEST_PIN2_LOW;\
						GPIOC->CRH &= ~GPIO_CRH_CNF13;\
						GPIOC->CRH |= GPIO_CRH_MODE13;\
						TEST_PIN3_HIGH;\
						GPIOC->CRH &= ~GPIO_CRH_CNF12;\
						GPIOC->CRH |= GPIO_CRH_MODE12;\
						TEST_PIN4_LOW;\
						GPIOC->CRH &= ~GPIO_CRH_CNF10;\
						GPIOC->CRH |= GPIO_CRH_MODE10;\
						TEST_PIN5_LOW;\
						GPIOC->CRH &= ~GPIO_CRH_CNF9;\
						GPIOC->CRH |= GPIO_CRH_MODE9;\
						TEST_PIN6_LOW;\
						GPIOC->CRH &= ~GPIO_CRH_CNF8;\
						GPIOC->CRH |= GPIO_CRH_MODE8;\
						}while(0)

#define TEST_PIN1_LOW	GPIOD->BSRR = GPIO_BSRR_BR2
#define TEST_PIN1_HIGH	GPIOD->BSRR = GPIO_BSRR_BS2

#define TEST_PIN2_LOW	GPIOC->BSRR = GPIO_BSRR_BR13
#define TEST_PIN2_HIGH	GPIOC->BSRR = GPIO_BSRR_BS13

#define TEST_PIN3_LOW	GPIOC->BSRR = GPIO_BSRR_BR12
#define TEST_PIN3_HIGH	GPIOC->BSRR = GPIO_BSRR_BS12

#define TEST_PIN4_LOW	GPIOC->BSRR = GPIO_BSRR_BR10
#define TEST_PIN4_HIGH	GPIOC->BSRR = GPIO_BSRR_BS10

#define TEST_PIN5_LOW	GPIOC->BSRR = GPIO_BSRR_BR9
#define TEST_PIN5_HIGH	GPIOC->BSRR = GPIO_BSRR_BS9

#define TEST_PIN6_LOW	GPIOC->BSRR = GPIO_BSRR_BR8
#define TEST_PIN6_HIGH	GPIOC->BSRR = GPIO_BSRR_BS8


//================================================================================================

// отображение прерывания DMA DAC
#define DAC_DMA_TEST_LOW	TEST_PIN1_LOW
#define DAC_DMA_TEST_HIGH	TEST_PIN1_HIGH

// отображение декодирования порции сэмплов
#define DECODE_TEST_LOW		TEST_PIN2_LOW
#define DECODE_TEST_HIGH	TEST_PIN2_HIGH

// светодиод
#define LED_ON				TEST_PIN3_LOW
#define LED_OFF				TEST_PIN3_HIGH


#endif
