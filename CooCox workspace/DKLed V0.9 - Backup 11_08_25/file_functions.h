

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FILE_FUNCTIONS_H
#define __FILE_FUNCTIONS_H


#include "stm32f10x.h"
#include "includes.h"

void filehextofname(char fname_[], uint32_t hex, uint8_t len);
void filePathByNum(
		uint16_t fnumber_,  		// главный параметр - число, с которого должно начинаться имя файла
		uint8_t letter_counter_,	// сколько символов имени файла учитывать (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
		char path[],				// результат работы функции, здесь формируется путь к файлу
		char fname[]);
uint8_t filePathByFirstNum(
		uint16_t fnumber_,  		// главный параметр - число, с которого должно начинаться имя файла
		uint8_t letter_counter_,	// сколько символов имени файла учитывать (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
		char path[],				// результат работы функции, здесь формируется путь к файлу, длина фиксированная FILE_PATH_MAX_LENGTH
		char fname[],				// вспомогательный массив, временное хранение имени файла, длина фиксированная FILE_NAME_MAX_LENGTH-1
		char DIRpath[],			//массив с именем директории
		DIR* dir_,					// папка, в которой всё безобразие происходит
		FILINFO* filinfo_);			// для временного хранение инфо о файле
void DIRPathByName(
		char fname[],				// имя папки, длина фиксированная FILE_NAME_MAX_LENGTH-1
		char DIRpath[],			// результат работы функции, здесь формируется путь к файлу, длина фиксированная
		DIR* dir_,					// папка, в которой всё безобразие происходит
		FILINFO* filinfo_);

char  debugTask_inttochar (uint8_t c);


#endif
