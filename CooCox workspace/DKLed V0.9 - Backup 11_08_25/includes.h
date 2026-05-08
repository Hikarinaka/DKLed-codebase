/*************************************************************************************************
 * @file		includes.h
 *
 * @brief		Список подключаемых программных модулей
 *
 * @version		v1.0
 * @date		03.09.2013
 * @author		Mike Smith
 *
 *************************************************************************************************/

#ifndef INCLUDES_H_
#define INCLUDES_H_


#define O8I5U2 //тип контроллера O8I6U2, O6I4U2, O8I5U2,O4I3
#define DEFAULT_PERSONAL_ID		0x30303031 //"0001"
#define EMPTY_PERSONAL_ID		0x2A2A2A2A //"****"

#define VARIABLES_SUPPORT //включить поддержку переменных


#define FS_ENABLED //включить части кода, связанные с файловлй системой
//#define USB_ENABLED //включить части кода, связанные с USB
#define FastButtonShortcuts //включение коротких команд на кнопках


// стандартные модули
//#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
//#include <core_cm3.h>

// описание контроллера
#include "stm32f10x.h"
//#include "misc.h"
#include "stm32f10x_crc.h"

// операционная система
//#include "coocox.h"
#include "timings.h"

// файловая система
#include "ff.h"

// наши модули
//#include "board.h"
#include "exceptions.h"
#include "I2C_c.h"
#include "dbgu.h"
#include "ff_addons.h"
#include "descript_S.h"
#include "init_ws.h"
#include "buttons.h"
#include "file_functions.h"
#include "USB_c.h"



// задачи
//#include "dacTask.h"
#include "debugTask.h"
#include "inputsTask.h"
#include "outputTask.h"

//#define O6I4U2 //тип контроллера O8I6U2, O6I4U2, O6I5U2,O4I3

#if defined (O6I4U2)
	#define MAX_REAL_BUTTONS		5		//количество реальных кнопок + 0-я
#endif
#if defined (O8I5U2)
	#define MAX_REAL_BUTTONS		6		//количество реальных кнопок + 0-я
#endif
#if defined (O8I6U2)
	#define MAX_REAL_BUTTONS		7		//количество реальных кнопок + 0-я
#endif
#if defined (O4I3)
	#define MAX_REAL_BUTTONS		4		//количество реальных кнопок + 0-я
#endif
#define LEN_FILE_FOR_BUTTON			4//длины имён файлов для перехода по кнопкам
#define FILE_PACKAGE_BYTE_LENGTH	16*4 //количество байт; строго кратно 4


#define PCLK1_Freq		0x02255100	//36 MHz
#define PCLK2_Freq		0x044AA200	//72 MHz
#define SYSCLK_Freq		0x044AA200	//72 MHz

#define DEBUG_PORT_OUT	0xF	//0- USART1, 1 - DBGU USART, 0xF - USB


void Init_SPI(void);

#endif /* INCLUDES_H_ */
