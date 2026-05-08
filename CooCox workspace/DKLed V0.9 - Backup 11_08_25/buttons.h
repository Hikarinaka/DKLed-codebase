/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __BUTTONS_H
#define __BUTTONS_H


#include "stm32f10x.h"
#include "includes.h"


//#define BUTTONS_PORT		PORTB
#define MAX_NUM_BUTTONS			16		//количество кнопок

#if defined (O8I6U2)
#define MAX_NUM_ENCODERS		3
#endif
#if defined (O8I5U2)
#define MAX_NUM_ENCODERS		2
#endif
#if defined (O6I4U2)
#define MAX_NUM_ENCODERS		2
#endif
#ifndef MAX_NUM_ENCODERS
#define MAX_NUM_ENCODERS		1
#endif
#define MAX_NUM_BUTTONS_ARR		0x41		//количество ячеек в массиве действий
#define MAX_NUM_BUTTONS_MASK	0x1F	//=31, обрезка переменной
#define MAX_NUM_BUTTONS_ACIONS_MASK		0x3F	//обрезка байта действия при обращении к массиву кнопок (действий)
#define AFTER_FAST_ANIM_ACIONS_ADRESS		0x40	//адрес "кнопки", в которой событие по окончанию быстрой анимации
#define BUTTONS_TEXT_ARRAY_SIZE		0x40	//количество байт в массиве для отправки текста по кнопке
#define BUTTONS_TEXTS_AMOUNT		8		//количество разных текстов для отправки (максимально может быть 128 потому что действует обрезка, см. descript ветка DS_Type_command_I2C_Send_to, номер текстового поля назначается через указатьель *I2C_TargetAddress_Pointer
#define BUTTONS_TEXTS_OUT_LABEL		0xB			//позиция буфера текста в списке портов ввода-вывода (0, 1 и 2 - это UART, 0xC - I2C, без указания - USB)


//минимальное и максимальное время,нужное чтобы набрать комбинацию из кнопок, мс
#define BUTTON_COMPLETE_MIN		20
#define BUTTON_COMPLETE_MAX		10000
#define BUTTON_WAIT_MAX			0x1000000

//битовые маски кнопок для разных регистров:
#define BUTTONS_MASK_FTSR			(EXTI_FTSR_TR4 | EXTI_FTSR_TR5 | EXTI_FTSR_TR6)		// | EXTI_FTSR_TR7)
#define BUTTONS_MASK_PR				(EXTI_PR_PR4 | EXTI_PR_PR5 | EXTI_PR_PR6)			// | EXTI_PR_PR7)
#define BUTTONS_MASK_IMR			(EXTI_IMR_MR4 | EXTI_IMR_MR5 | EXTI_IMR_MR6)		// | EXTI_IMR_MR7)
//28/10/17 пин7 отключён, потому что на этот пин отключён, не имеет резистора подтяжки и при старте контроллера ловит прерывание

//битовые маски для запоминания нажатых кнопок buttonsState (1=нажата, 0=отпущена)
#define BUTTONS_STATE_1				0x01
#define BUTTONS_STATE_2				0x02
#define BUTTONS_STATE_3				0x04
#define BUTTONS_STATE_4				0x08
#define BUTTONS_STATE_5				0x10
#define BUTTONS_STATE_6				0x20
#define BUTTONS_STATE_7				0x40
#define BUTTONS_STATE_8				0x80

//битовые маски для положения бита кнопки в порте ввода

#if defined (O6I4U2)
#define GPIO_BUTTON1				GPIO_Pin_7
#define GPIO_BUTTON2				GPIO_Pin_6
#define GPIO_BUTTON3				GPIO_Pin_5
#define GPIO_BUTTON4				GPIO_Pin_4
#define GPIO_BUTTON5				GPIO_Pin_0
#define GPIO_BUTTON6				GPIO_Pin_1

#elif defined (O8I5U2)
#define GPIO_BUTTON1				GPIO_Pin_4
#define GPIO_BUTTON2				GPIO_Pin_5
#define GPIO_BUTTON3				GPIO_Pin_6
#define GPIO_BUTTON4				GPIO_Pin_7
#define GPIO_BUTTON5				GPIO_Pin_0
#define GPIO_BUTTON6				GPIO_Pin_1

#elif defined (O8I6U2)
#define GPIO_BUTTON1				GPIO_Pin_7
#define GPIO_BUTTON2				GPIO_Pin_6
#define GPIO_BUTTON3				GPIO_Pin_5
#define GPIO_BUTTON4				GPIO_Pin_4
#define GPIO_BUTTON5				GPIO_Pin_0
#define GPIO_BUTTON6				GPIO_Pin_1

#elif defined (O4I3)
#define GPIO_BUTTON1				GPIO_Pin_4
#define GPIO_BUTTON2				GPIO_Pin_5
#define GPIO_BUTTON3				GPIO_Pin_6
#define GPIO_BUTTON4				GPIO_Pin_7
#define GPIO_BUTTON5				GPIO_Pin_0
#define GPIO_BUTTON6				GPIO_Pin_1

#endif

//*-----------------------------------------------------------------------------------------------
//*			Прототипы
//*-----------------------------------------------------------------------------------------------

//инициализация портов и прерываний
void ButtonsInit(void);

//общая процедура на прерывания по разным кнопкам
//void ButtonPressed_IRQHandler(void);

//анализ нажатых кнопок в конце паузы против дребезга контактов
//void ButtonPressedAnalyse(void);

//пауза
//void tupnyak(void);

#endif
