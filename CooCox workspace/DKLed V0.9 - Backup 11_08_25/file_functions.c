// вспомогательные функции для чтения файлов

#include "file_functions.h"
#include "descript_S.h"
//#include "includes.h"
extern volatile uint8_t DebugInfoOutFlag; //включает и отключает вывод служебной информации на второй USART (DBGU)


void filehextofname(char* fname_, uint32_t hex, uint8_t len)
{

	uint8_t c_num; //преобразованное числовое значение этого символа
	uint8_t i; //counter

	for (i=len; i > 0 ; --i){
		c_num = (uint8_t)hex &0x0F;
		fname_[i-1] = DS_inttochar(c_num);
		hex >>= 4;
	}

	//return c_num;
}

//--------------------------------------------------------------------------------------------------------------------------------------
// путь и имя файла в папке /hcd, имя которого равно "цифры.hcd"
void filePathByNum(
		uint16_t fnumber_,  		// главный параметр - число, с которого должно начинаться имя файла
		uint8_t letter_counter_,	// сколько символов имени файла учитывать (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
		char path[],				// результат работы функции, здесь формируется путь к файлу
		char fname[]){				// вспомогательный массив, временное хранение имени файла
	//if (DebugInfoOutFlag){printf("filePathByNum...\r\n");}
	uint8_t a, b; //счетчики
	for (a = 0; a < FILE_NAME_MAX_LENGTH-1; ++a) {
		fname[a] = '\0';
	}
	//CoTickDelay(200);

	//printf("(filePathByNum) *fnumber_=%x,  *letter_counter_=%x, len=x\r\n", fnumber_, letter_counter_);
	//CoTickDelay(50);

	filehextofname(fname, fnumber_, letter_counter_);

	//printf("fname = "); printf(fname); printf("\r\n");
	//printf("path = "); printf(path); printf("\r\n");
	//CoTickDelay(200);

	for (a = 0; a < FILE_PATH_MAX_LENGTH; ++a) {
		path[a] = '\0';
	}
	//printf("path = "); printf(path); printf("\r\n");
	//CoTickDelay(200);
	memcpy(path, "0:/hcd/", 7);

	a = 0;
	while(path[a]) {
		//printf("a=%x, path[a]=%x\r\n", a, path[a]);
		a++;
	} // ищем конец строки path
	b = 0; //запоминаем конец строки path
	while(fname[b]) {
		path[a] = fname[b];
		a++;
		b++;
	}
	//strcat(path, fname);
	strcat(path, ".hcd");
}


//------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------
// путь и имя файла в папке /hcd, имя которого начинается с "цифры...."
uint8_t filePathByFirstNum(
		uint16_t fnumber_,  		// главный параметр - число, с которого должно начинаться имя файла
		uint8_t letter_counter_,	// сколько символов имени файла учитывать (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
		char path_[],				// результат работы функции, здесь формируется путь к файлу, длина фиксированная FILE_PATH_MAX_LENGTH
		char fname_[],				// вспомогательный массив, временное хранение имени файла, длина фиксированная FILE_NAME_MAX_LENGTH-1
		char DIRpath_[],			//массив с именем директории
		DIR* dir_,					// папка, в которой всё безобразие происходит
		FILINFO* filinfo_){			// для временного хранения инфо о файле
	//if (DebugInfoOutFlag){printf("filePathByFirstNum...\r\n");}
	uint8_t a; //, b; //счетчики
	FRESULT result;

	for (a = 0; a < FILE_NAME_MAX_LENGTH; ++a) { //очистка fname
		fname_[a] = '\0';
	}
	filehextofname(fname_, fnumber_, letter_counter_); //перевод цифр в буквы fname
//(start)поиск в папке /hcd файла, начинающегося с символов "ноль"
		result = f_opendir (dir_, DIRpath_ );
		if (result == FR_OK){

		// далее - поиск нужного файла
		// a - счетчик количества совпавших букв, если он стал больше letter_counter_, значит пробежали по всем совпавшим буквам
		// здесь и далее то, что (a >= letter_counter_) является признаком того, что нужный файл найден
		do {
			result = f_readdir (dir_,  filinfo_ ); //читаем следующий файл
			a = 0;
			while ((filinfo_->fname[a]==fname_[a]) && (a< FILE_NAME_MAX_LENGTH-1)) { a++; }

		} while ( ! ( (filinfo_->fattrib & AM_ARC)	//объект является файлом
				&& (a >= letter_counter_)) //количество совпадающих символов достаточно
				&& filinfo_->fname[0]); //и при этом имя файла не пустое

		//файл с нужного начала встретился - готовимся его читать и переваривать
		if (a == letter_counter_) {
				//printf("   (filePathByFirstNum) fname begins with"); printf(fname_); printf("\r\n");

				for (a = 0; a < FILE_PATH_MAX_LENGTH; ++a) {
					path_[a] = '\0';
				}

				//for (a = 0; a < FILE_NAME_MAX_LENGTH; ++a) { //очистка fname
				//	fname_[a] = filinfo_->fname[a];
				//}
				//memcpy(fname_, filinfo_->fname, FILE_NAME_MAX_LENGTH);
				//CoTickDelay(100);
				//memcpy(path_, "0:/hcd/", 7);
//				for (a=0; ((a<DIR_PATH_MAX_LENGTH) && DIRpath_[a]); a++){
//					path_[a] = DIRpath_[a];
//				}
				strcat(path_, DIRpath_);
				strcat(path_, "/");
				strcat(path_, filinfo_->fname);
				return 1;
			//} else {
			//
			}

		}
		return 0;//else {
}


//------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------
// путь и папки в папке /hcd, имя которого начинается с fname_
void DIRPathByName(
		char fname_[],				// имя папки, длина фиксированная FILE_NAME_MAX_LENGTH-1
		char DIRpath_[],			// результат работы функции, здесь формируется путь к файлу, длина фиксированная
		DIR* dir_,					// папка, в которой всё безобразие происходит
		FILINFO* filinfo_){			// для временного хранения инфо о файле
	uint8_t a, b; //счетчики
	FRESULT result;

		//result = f_opendir (dir_, DIRpath_ );
		result = f_opendir (dir_, "0:/hcd" );//ищем папку в "корне"
		if (result == FR_OK){
		do {
			result = f_readdir (dir_,  filinfo_ ); //читаем следующий файл
			//перебор и сравнение символов в имени файла
			a = 1;
			for (b = 0; (b < FILE_NAME_MAX_LENGTH) && filinfo_->fname[b]; b++) { //считаем сколько символов в fname
				if (filinfo_->fname[b] != fname_[b]) a = 0;
				//printf(fname_[b]);
			}
			//printf(" ");
			//printf(filinfo_->fname);
			//printf(" a= %d\r\n", a);

		} while ( ! ( (filinfo_->fattrib & AM_DIR)	//объект является папкой
				&& a) //количество совпадающих символов совпадает
				&& filinfo_->fname[0]); //и при этом имя файла не пустое
		//printf("a= %d\r\n", a);
		//папка встретилась - готовимся её читать и переваривать
		for (b = 0; b < DIR_PATH_MAX_LENGTH; ++b) {
			DIRpath_[b] = '\0';
		}
		memcpy(DIRpath_, "0:/hcd", 6);
		if (a == 1) {
				//printf("   (filePathByFirstNum) fname begins with"); printf(fname_); printf("\r\n");
			strcat(DIRpath_, "/");
			strcat(DIRpath_, filinfo_->fname);

		}
		//if (DebugInfoOutFlag){printf("\r\n");}
		} //else {
		//	printf("DIR error %d", result);
		//}
}




// преобразование текста 0123...abcdef в число
char  debugTask_inttochar (uint8_t c)
{
	if (0<=c && c<=9)		return (char)(c+'0');
	if (0xA<=c && c<=0xF)	return (char)(c-10+'A');
	return 0xFF;
}


/*
//открытие файла в папке /hcd, начинающегося с заданной цифры
FRESULT openFileByFirstNum(
		uint32_t *fnumber_,  		// главный параметр - число, с которого должно начинаться имя файла
		uint8_t *letter_counter_,	// сколько символов имени файла учитывать (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
		char path[],				// вспомогательный массив, здесь формируется путь к файлу
		FIL *file, DIR *dir, FILINFO *filinfo,
		FRESULT *result){1


	*result = f_opendir (&dir, "0:/hcd" );
	printf("opening dir 0:/hcd/, result = %d\r\n", *result);

	//перебор объектов в папке, пока не встретится файл начинающийся с нуля
	do {
		result = f_readdir (&dir,  &filinfo );
		//printf("reading filinfo in 0:/hcd/, result = %d;  ", result);
		//printf("file name = ");  printf(filinfo.fname); printf("\r\n");
		//CoTickDelay(200);
	} while ( !( (filinfo.fattrib & AM_ARC)&&(filinfo.fname[0]=='0') && (filinfo.fname[0]!=0) ) ); //

	//файл с нуля встретился - готовимся его читать и переваривать
	if ((filinfo.fattrib & AM_ARC) && (filinfo.fname[0]=='0')) {
			printf("fname begins with '0'\r\n");
			memcpy(&path, "0:/hcd/", 7);
			strcat(path, filinfo.fname);
			printf("executing file ");
			printf(&path);
			printf("\r\n");
		} else {
			printf("file not found\r\n");
			while(1);
		}
		printf("\r\n");
}
*/
