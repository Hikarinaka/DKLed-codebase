/*************************************************************************************************
 * @file		exceptions.h
 *
 * @brief		«аголовок модул€ обработки исключений
 *
 * @version		v1.0
 * @date		03.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/
#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include "stm32f10x.h"

// макрос отладочной остановки €дра
#define DBG_HALT(numb) \
	do { \
		if (CoreDebug->DHCSR & 1) \
		{ \
			__asm( "BKPT %0\n"::"M"(numb) ); \
		} \
	} while(0)

// инициализаци€ модул€
void hard_fault_ini(void);


#endif /* EXCEPTIONS_H_ */
