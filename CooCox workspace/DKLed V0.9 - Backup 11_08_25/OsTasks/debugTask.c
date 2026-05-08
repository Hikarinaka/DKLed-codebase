/*************************************************************************************************
 * @file		debugTask.c
 *
 * @brief		Задача чтения файла
 *
 * @version		v0.02
 * @date		25.10.2017
 * @author		Vet
 *
 ************************************************************************************************/

//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"
#include "debugTask.h"
#include "descript_S.h"
//#define FastButtonShortcuts //включение коротких команд на кнопках

//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
//ID задач
//ID задач
/*extern OS_TID debugTaskID, outputTaskID, inputsTaskID;

OS_STK testTaskStk[TEST_TASK_STK_SIZE];	// стек задачи
extern StatusType outputStartFlag;
extern StatusType fileReadStartFlag;
extern StatusType r2; //для проверки результата установики флагов
*/

//uint8_t MainLoop_AFK_Flag = 0;
uint32_t MainLoop_ResetTime = 0;

// для WS
extern uint8_t WS2812_IO_framedata[ WS2812_IO_FRAMEDATA_SIZE ];
extern uint8_t LED_control_type;  //тип управления светодиодов,
//1 = 3х проводные, без часов
//2 = аналоговые сервы
//3 = гибридный (1+2)
//4 = быстрый вывод (флаг допустим только когда остальные опущены)
//8 = постоянное отслеживание позиции серв запустить
//16 = фактический флаг необходимости апдейта позиций серв в процессе работы ШИМ
//128 = работа с файлами (флаг допустим только когда остальные опущены)
extern uint8_t WS2812_IO_High;
extern uint8_t WS2812_IO_Low;
extern uint16_t WS2812_Frame_Length; //количество пикселей в кадре
extern uint16_t WS2812_Frame_Length_Actual; //количество пикселей в кадре
extern uint32_t WS2812_Frame_Byte_Length; //количество байт в кадре (WS2812_Frame_Length*24)
extern uint32_t WS2812_Frame_Byte_Length_Actual;
extern uint16_t WS2812_Frame_Count; //количество кадров в анимации (определяем исходя из размеров массивов - сколько кадров вообще может поместиться при такой длине)
extern uint16_t WS2812_Frame_Count_Actual; //количество кадров в анимации
extern uint16_t WS2812_Frame_Total_Count; // всего кадров надо проиграть, счётчик
extern uint32_t WS2812_Frame_Start_Pointer; // указатель на первый проигрываемый кадр (адрес старта массива)
extern uint32_t WS2812_Frame_Start_Pointer_Actual;// указатель на текущий проигрываемый кадр
//extern uint32_t WS2812_Current_Frame_Start;
extern uint16_t WS2812_Frame_Period; //период в мкс, по сути то же что Servo_Period, только значение должно храниться независимо
extern uint16_t WS2812_Frame_Period_Actual;
extern uint8_t DS_LED_Brightness; //яркость светодиодов

extern U64 nextPauseStartTime; // хранение времени начала предыдущей паузы - для сохранения общего таймлайна несмотря на тормоза
extern U64 FileReadStartTime; //хранение времени начала чтения текущего файла
extern U64 nextAbsolutPauseExpireTime; //хранение времени, когда должна закончиться текущая абсолютная пауза
extern uint32_t need_pause; //временное хранилище значения текущей паузы
extern U64 osTime; //для времени ОС
//uint32_t time; //время в мкс, затраченное на открытие нового файла
extern U64 File_Suspend_start;
extern U64 M25ReverseTime;
extern uint32_t M25ReversePauseValue;

//установка параметров серводвигателя - частота обновления, минимальная и максимальная длительность импульса управления
// в микросекундах
extern uint32_t Servo_Period;// = 20000; //50Гц
extern uint32_t Servo_Actual_Period;// = 20000; //50Гц
extern uint32_t Servo_MinPos;// = 1000; //1 ms
extern uint32_t Servo_MaxPos;// = 2000; //2 ms
extern uint16_t Servo_Pos [9]; //массив для позиций серводвигателей
extern uint16_t Servo_Resolution;//=256 //разрешение

extern uint32_t Servo_Period_default; //20 000
extern uint32_t Servo_MinPos_default; // 1000
extern uint32_t Servo_MaxPos_default; // 2000
extern uint16_t Servo_Resolution_default; // 256 //разрешение


extern uint16_t Servo_Pos_Temp [8]; //массив для хранения следующих позиций серводвигателей (диапазон от 0 до Servo_Resolution)
extern uint16_t Servo_Pos_Mask[10]; //массив значений модулятора на каждом такте
extern uint32_t Servo_time; //таймер для серв
extern uint8_t Servo_Update_Flag; // флаг состояния серв
// 0x01: 1 - не включать
// 0x02: 1 - изменились PosTemp
// 0x04: 1 - изменились параметры
// 0x08: 1 - мы в промежутке между импульсов
// 0x10: 1 - надо обновить светодиоды (в гибридном режиме)
// 0x20: 1 - изменились параметры мультикадра
// 0x80: 1 - надо останавливать ШИМ
extern uint8_t Servo_Action_Mask;
extern uint8_t Servo_step; //шаг, на котором находится прерывание - он же индекс массивов позиций сервов
//extern uint16_t Servo_Dead_Band_Width = 10; //период вызова таймера сервов
extern uint16_t Servo_GPIO_Setting_on; //для ног вкл/выкл
extern uint16_t Servo_GPIO_Setting_off;
extern uint16_t Servo_GPIO_Actual_on; //для ног вкл/выкл
extern uint16_t Servo_GPIO_Actual_off;




// переменные для файловой системы
static FATFS FATFS_Obj;
static FIL file; //, file2;
DIR dir;
FILINFO filinfo;
//static char path;
char path[FILE_PATH_MAX_LENGTH]; // = (char*)malloc(18);
char path2[FILE_PATH_MAX_LENGTH]; // = (char*)malloc(18);
char Dirpath[DIR_PATH_MAX_LENGTH]; // = (char*)malloc(18);
char FileworksDirpath[DIR_PATH_MAX_LENGTH]; // = (char*)malloc(18);

char fname[FILE_NAME_MAX_LENGTH]; //для временного хранения имени файла, куда переходить
//extern uint8_t text_buff[READ_SIZE];
//extern uint32_t timer_ms; //в исходнике было статик
uint16_t Current_File_Num; //хранение кодов текущего файла
uint16_t Next_File_Num; //какой файл открыть следующим
uint16_t Prev_File_Num; //куда возвращаться по команде M98 P FFFF
uint16_t Parent_File_Num; //куда возвращаться из подпрограммы
uint16_t Parent_Prev_File_Num; //сохраняем номер предыдущего файла для выхода из подпрограммы

uint16_t String_in_file_Index=0;
uint16_t String_in_file_Index_Backup=0;
uint16_t Char_in_string_index=0;

uint8_t SD_Volume_Exists; //флаги состояния файловой системы

//0x01 - записываем без оглядки на CRC
//0x02 - разрешить запись
//0x04+0x08 (0x0С) - куда выдаём ответ (0000 - USB, 0100 - Uart1, 1000 - Uart2, 1100 - both UARTs)
//0x10 - отсутствие карты памяти
//0x20 - бросили попытки
//0x40 - находимся в подпрограмме
//0x80 - выходим из подпрограммы в основную

uint16_t SD_Recheck_Countdown=0; //для горячего подсоединения карты
DWORD FilePTR_Backup=0;
DWORD FileClust_Backup=0;
//DWORD FileFsWinsect_Backup=0;
//DWORD FileDsect_Backup=0;


DWORD FileWorksPTR=0;

extern uint32_t CRC_sum; //CRC



//uint32_t File_Error_wait_timeout = 1000;
//U64 Unfreeze_Read_From_Ports_when_SD_failed_At; //время когда контроллер забивает на чление карты и раотает как будто её нет
//если она появится, то, возможно он прочитает первую команду с неё ошибкой
//но на практике пусть перечитает файл с нуля.
uint16_t File_Read_Cycle_Count=0; //0 - без остановки или не определено, 1 - игнорировать M47P<>, 2+ - количество циклов
uint16_t File_Read_Cycle_Count_Backup=0; //сохраняем значение при переходе в подпрограмму
uint8_t File_Open_Flags; //флаги режима открытия файла, описание в ff.h

//uint16_t Tmp_File_Num; //файл типа tmp.tmp имя присваивается когда его сохраняем.



// для descript-------------------------------------------------
//для любого descript
extern uint8_t DS_status; //какую команду ожидаем
//extern uint8_t WS2812_TC; //флаг того, что вывод точек на ws завершен (1 если завершено)
extern uint16_t DS_maxCurrPoints; //переменная для вывода только нужного количества точек
extern uint16_t DS_TotalPoints;
extern uint16_t DS_WSpoint_counter; //текущий принимаемый номер точки WS
extern uint8_t DS_RGB_counter; //счетчик принятых символов для точки RGB
extern uint32_t DS_Param; //32-битный параметр для дешифровщика
extern uint8_t DS_comm_num; //текущий номер кода (который после буквы)
extern uint8_t DS_Flag_Register; //дополнительные флаги работы
//b1 (1) - 0 = читаем число в hex формате, 1 - читаем число в dec формате/ I2C: чтение с параметром (адресом)
//b2 (2) - 0 = ничего, 1 - I2C: читаем из порта/ Sx P: - прыгнуть на пиксель
//b4 b3 (8 + 4) - 00xx - точное, 01xx - добавить, 10xx - отнять, 11xx - взять из переменной
//b5 (16,  0x10) - 0 = ничего, 1 = проверить совпадение с цветом
//b6 (32,  0x20) - 0 = нормальное выполнение команды, 1 = добавить команду в кнопку
//b7 (64,  0x40) - 0 = ничего, 1 - смена позиции пикселя
//b8 (128, 0x80) - 0 = ничего,
uint8_t DS_Math_Flags; //флаги активации разных арифметических операций

// для расшифровки файла
extern uint16_t DS_buf_start; //с какого места расшифровывать, если команда из нескольких знаков
uint16_t DS_buf_start_backup=0; //запомнить позицию в осовном файле перед переходом в подпрограмму
extern uint16_t DS_buf_counter; //счетчик, какое число пришло последним в буфер
extern uint8_t text_buff[READ_SIZE+4]; //буфер из которого происходит чтение команд для расшифровки
extern uint8_t file_to_read; //!!!переменная, которая гооврит, как читать файл. 0 - с начала, 1 - продолжаем как было.

// для работы с USART нужно сделать параллельный набор таких же переменных - на случай, если надо будет продолжить чтение файла

//usart 2
extern uint8_t dbgu_rx_buff[DBGU_RX_SIZE];		//< буфер приёмника uart2
extern uint16_t dbgu_rx_buf_start; //с какого места расшифровывать из буфера RX
extern uint16_t dbgu_rx_buf_counter; //счетчик, какое число пришло последним в буфер RX
uint16_t dbgu_rx_temp_start; //вспомогательная переменная для отработки того, что буфер USART зациклен
uint16_t dbgu_rx_temp_counter; //вспомогательная переменная для отработки того, что буфер USART зациклен
extern uint8_t dbgu_State_of_recieved_Command; //индикатор/ркегистр состояния чтения команды из USART2 нам интересны значения 0x80 - команда целиком и 0x90 - команда не целиком, но пора её читать
//extern uint8_t dbgu_State_of_Commands_undescripted; //индикатор/регистр состояния чтения команды из USART2

uint8_t dbgu_rx_buf_overcount; //вспомогательный флаг, показывающий, что надо продолжить чтение сначала буфера
extern volatile uint32_t dbgu_rx_timeout; //предельное время ожидания конца команды от её начала
extern volatile U64 dbgu_rx_start_time; //записываем время когда мы должны начать дешифровку


extern uint8_t USART1_rx_buff[DBGU_RX_SIZE];		//< буфер приёмника uart2
extern uint16_t USART1_rx_buf_start; //с какого места расшифровывать из буфера RX
extern uint16_t USART1_rx_buf_counter; //счетчик, какое число пришло последним в буфер RX
extern uint8_t USART1_State_of_recieved_Command; //индикатор/ркегистр состояния чтения команды из USART2 нам интересны значения 0x80 - команда целиком и 0x90 - команда не целиком, но пора её читать
//extern uint8_t USART1_State_of_Commands_undescripted; //индикатор/регистр состояния чтения команды из USART2
extern volatile uint32_t USART1_rx_timeout; //предельное время ожидания конца команды от её начала
extern volatile U64 USART1_rx_start_time; //записываем время когда мы должны начать дешифровку

uint8_t* USART_Buf_pointer;
uint16_t* USART_counter_pointer;
volatile U64* USART_ptr_rx_start_time;
uint8_t* USART_ptr_State_of_recieved_Command;
//uint8_t* dbgu_rx_temp_undescripted;

//флаг выбора канала для расшифрофки
extern uint8_t DS_Channel_Select;
//0-любой канал готов,
//1-строго файл,
//2-строго USART1,
//3-строго USART2 (DBGU),
//4-строго USB,
//0x10 - все кроме файла
extern uint16_t DS_Pause_interrupt_Flag; //поднимаеется, если мы только что выполнили паузу
//регистры
//1 - пауза типа G4,
//2 - абсолютная пауза,
//4 - вывод на светодиоды не нужен,
//8 - пауза ожидания вывода
//16 и 32 - режим "ждём конца вывода" на usart1 и usart2 оответственно прежде чем исполняем следующую команду
//64 - ожидание конца анимации
//128 - ожидание перезапуска чтения файла M24
//256 - М25 переключается
//512 - ждём окончания передачи через I2C
extern uint32_t Personal_ID; //личный ID контроллера, нужен для общения с другими, "0000" или 0x30303030 - общий
extern volatile uint8_t DebugInfoOutFlag; //включает и отключает вывод служебной информации на второй USART (DBGU)

//для кнопок и выбора других файлов
//extern char fileForButton[FILE_NAME_MAX_LENGTH][MAX_NUM_BUTTONS]; //массив имен файлов для перехода по кнопке
extern uint16_t numFileForButton[MAX_NUM_BUTTONS_ARR];	//массив имён файлов для перехода по кнопкам (хранятся как цифры)
//extern uint8_t lenFileForButton[MAX_NUM_BUTTONS_ARR]; //длины имён файлов для перехода по кнопкам
#if defined (FastButtonShortcuts)
//	extern uint16_t ParameterForButton[MAX_NUM_BUTTONS_ARR];
	extern uint8_t FastCommandForButton[MAX_NUM_BUTTONS_ARR]; //идентификатор быстрой команды
	extern uint8_t CommandArgForButton[MAX_NUM_BUTTONS_ARR]; //короткий аргумент
	extern uint8_t ButtonNomberCarrier; //омер ячейки в массиве кнопок, то есть учитывает сдвиг по QR.
//	extern uint8_t TextToSendArrayForButton[BUTTONS_TEXT_ARRAY_SIZE];
//	extern uint8_t TextToSendArrayIndexesForButton[BUTTONS_TEXTS_AMOUNT];
//	extern uint8_t TextToSendArrayLengthsForButton[BUTTONS_TEXTS_AMOUNT];
//	extern uint8_t TextToSendArrayPointer;
//	extern uint8_t TextToSend_TargetAddress;
#endif
extern BYTE descrActionFlag; //битовые флаги разных действий в процессе цикла чтения/расшифровки, константы в DS_ACTION_...
//
//extern uint8_t buttonPressed; //какая кнопка была нажата
extern uint16_t buttonsState; //запоминаем состояние кнопок, чтобы правильно ловить нажатие/отпускание (по битам)
extern uint16_t buttonPushInterrupt; //для передачи информации о нажатых/отпущенных кнопках
extern uint16_t buttonRelInterrupt;
//extern uint16_t buttonPushInterruptActive; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
//extern uint16_t buttonRelInterruptActive; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
#ifndef FastButtonShortcuts
extern uint16_t buttonPushWaitStateEndFile; // флаги активности кнопок по типам событий
extern uint16_t buttonPushWaitStateEndMulti; // флаги активности кнопок по типам событий
extern uint16_t buttonRelWaitStateEndFile; // флаги активности кнопок по типам событий
extern uint16_t buttonRelWaitStateEndMulti; // флаги активности кнопок по типам событий
#endif
extern U64 ButtonWaitStart; //когда произошло последнее изменение состояния кнопок
extern U64 ButtonCompleteStart;
extern uint32_t ButtonWaitSetting; //через сколько милисекунд обнулять кнопки (повторное нажатие)
extern uint32_t ButtonCompleteSetting;
extern uint8_t ButtonFlags;
//1 - нажатие по одной кнопке (0) или комбинацией (1)
//2 - (1)= ждём конца нажатия
//4
//8 - (0)= последний раз FastCommand сработал вхолостую
//0x10 - принимаем байт как команду с uart 1
//0x20 - принимаем байт как команду с uart 2
extern int16_t ButtonEncoder;
extern uint16_t ButtonEncorerIndiv[MAX_NUM_ENCODERS];

extern uint16_t CheckPixel_miscomparations;

extern uint16_t AfterLoop_File_Num; //код файла по отложенному переходу M47
uint16_t AfterLoop_File_Num_Backup; //охранить значение оверрайда номера файла при заходе в подпрограмму
extern uint16_t AfterMulti_File_Num; //код файла по отложенному переходу конец мультикадра
extern uint8_t ChosenNomberInM98; //предварительно одобренный выбор файла для команды M98Q<>P<>

#ifdef VARIABLES_SUPPORT
	extern uint16_t Custom_Variables_List[26];
#endif


uint8_t a8; //временная переменная - счетчик
uint16_t a16; //временная переменная
uint8_t descrResult; //временная переменная, передающая результат текущего "захода" расшифровки
BYTE buttonInterrupt_tmp;
uint16_t* buttonInterrupt;
uint32_t read_size; //количество прочитанных за текущий подход данных
//uint32_t full_size; //общее количество прочитанных из файла данных


uint16_t tempUSBpar=0;


extern uint16_t USB_Bytes_to_send_left;
extern uint16_t USB_Recieved_bytes;
extern uint16_t USB_RX_Start_pointer;
extern uint8_t USB_Function_flags;
extern volatile uint32_t USB_rx_timeout;
extern unsigned char USB_Buff1[64];
extern unsigned char USB_COM_TX_Buff1[64];

extern uint8_t I2C_TargetAddress;//куда отправлять
extern uint8_t I2C_Bytes_to_Recieve;
extern uint8_t I2C_Bytes_To_Send;
//-------------------------------------------------------------------------------------------
//			Локальные фунции
//-------------------------------------------------------------------------------------------


void File_Open_Preparation(void)
{

	if (!(SD_Volume_Exists & 0x10) && LED_control_type < 128) { //если всё хорошо
		//printf("file ");
		a8= filePathByFirstNum(Next_File_Num, LEN_FILE_FOR_BUTTON, path, fname, Dirpath, &dir, &filinfo);
		descrActionFlag &= ~DS_ACTION_GOTO_FILE;
		if (a8){// равно 1 если мы нашли новый файл и обновили путь. Иначе мы просто продолжаем работать с теми файлами, которые есть

			//если мы в подпрограмме, то "предыдущий" (Prev_File_Num) основной файл не меняется, так же как и текущий основной (Parent_File_Num)
			//при работе в подпрограмме наш тайминг не переключается и мы отсчитываем паузы по началу основного файла а не файла с подпрограммой
			//по этому в идеале основные файлы используют абсолютные тайминги а подпрограммы - относительные
			if (~SD_Volume_Exists&0x40){//если мы в открываем основной файл, переход по M98
				FileReadStartTime = CoGetOSTime(); //установка тайминга
				nextAbsolutPauseExpireTime = FileReadStartTime;
				DS_Pause_interrupt_Flag &= ~128; //спячка абсолютна, и продолжается при переходе в подпрограмму, так что её сброс только в этой ветке
				FilePTR_Backup = 0; //сброс указателя
				//FileClust_Backup=0;
				//FileFsWinsect_Backup=0;
				//FileDsect_Backup=0;
				Parent_File_Num = Next_File_Num; //файл, который мы открываем - основной
				Parent_Prev_File_Num = Current_File_Num;//сохраняем номер предыдущего файла на случай возвращения из подпрограммы
				File_Read_Cycle_Count = 0;//количество повторов сбрасывается только для основных файлов
				String_in_file_Index_Backup=1;
				//Message("Fl main\r\n\0",1);
			}
			String_in_file_Index=1;
			Char_in_string_index=0;
			Prev_File_Num = Current_File_Num; //тот файл, который мы только что читали записывается в предыдущий
			Current_File_Num = Next_File_Num;
			AfterLoop_File_Num = 0xFFFF;
			AfterMulti_File_Num = 0xFFFF;

			descrActionFlag = descrActionFlag | DS_ACTION_GOTO_FILE; //установка бита-флага
			SD_Volume_Exists &= ~0x30;
		} else {
			Fail_Message(1,2,4);
			FRESULT result_ = f_opendir (&dir, Dirpath );
			Fail_Message(2,2,result_);
			if (result_ != FR_OK) { //бросаем попытки
				SD_Volume_Exists |= 0x10;
			}
		}
	}
}

void FileBufferEnding (void)
{
	DS_buf_start = 0;
	read_size = 0;
	if (LED_control_type < 128){ //у нас не режим работы с ФС
	text_buff[0] = '\r';
	text_buff[1] = 'M';
	text_buff[2] = '2';
	text_buff[3] = '\n';
	DS_buf_counter = 3;}
}


void Reset_FileworkDirPath (void){
	for (a8 = 0; a8 < DIR_PATH_MAX_LENGTH; ++a8) { //чистим путь
		FileworksDirpath[a8]= '\0';
	}
	memcpy(FileworksDirpath, "0:/hcd", 6); //назначаем корень

}

void Path_FullName_in_Fileworkdirpath(void){
	for (a8 = 0; a8 < FILE_PATH_MAX_LENGTH; ++a8) { //чистим путь
		path[a8]= '\0';
	}
	strcat(path, FileworksDirpath);
	strcat(path, "/");
	strcat(path, fname);
}


void Read_Data_Chunk_From_File(){//(FRESULT *result_){

	FRESULT result_ = f_read(&file, (BYTE *)text_buff, READ_SIZE, (UINT*)&read_size);
	if (result_ != FR_OK){
		Fail_Message(1,1,result_);
		result_ = f_mount(0, &FATFS_Obj);
		if (result_ != FR_OK) { //бросаем попытки
			Fail_Message(3,3,result_);
			SD_Volume_Exists |= 0x10;
		} else {
			result_ = f_opendir (&dir, Dirpath );
			if (result_ != FR_OK) { //бросаем попытки
				Fail_Message(2,2,result_);
				SD_Volume_Exists |= 0x10;
			} else {
				result_ = f_open(&file, path, File_Open_Flags);
				if (result_ != FR_OK) { //бросаем попытки
					SD_Volume_Exists |= 0x30;
					Fail_Message(1,2,result_);
				} else {
					SD_Volume_Exists &= ~0x30;
					result_ = f_read(&file, (BYTE *)text_buff, READ_SIZE, (UINT*)&read_size);
					Fail_Message(1,1,result_);
					if (result_ != FR_OK) { //бросаем попытки
						SD_Volume_Exists |= 0x30;
					}
				}
			}
		}

	}

}





#if defined (FastButtonShortcuts)
//FS_a8 - команда,
//FS_b8 - короткий параметр
//*FS_arg - основной параметр
void FastCommand (uint8_t FS_a8, uint8_t FS_b8, uint32_t *FS_arg)
{
//
	//ButtonFlags |=8; //надеемся что тут что-то произойдёт
	//Message("FC_\0",1);
	DS_Math_Flags= 0;
	switch (FS_a8){
		case FC_EXIT_SUBPROGRAM_M89:

			if (SD_Volume_Exists & 0x40) {
				DS_buf_start=DS_buf_start_backup;
				SD_Volume_Exists |=0x80;
				*FS_arg = Parent_File_Num; //возврат в основной файл
				//File_Read_Cycle_Count = File_Read_Cycle_Count_Backup;
				//String_in_file_Index = String_in_file_Index_Backup;
			} else {
				break;
			}
		case FC_ENTER_SUBPROGRAM_M89:
			if (0 == (SD_Volume_Exists & 0x40)) {
				DS_buf_start_backup=DS_buf_start;
				File_Read_Cycle_Count_Backup = File_Read_Cycle_Count;
				AfterLoop_File_Num_Backup = AfterLoop_File_Num;
				SD_Volume_Exists |=0x40;
				String_in_file_Index_Backup=String_in_file_Index;
			}//сохраняем позицию курсора только кгда заходим из основной программы в подпрограмму
			goto Start_button_file_in_case_of_NoAnim;
		case FC_START_FILE_NOW_M98:
			if (SD_Volume_Exists & 0x40) {
				Current_File_Num = Parent_Prev_File_Num;
			}
			SD_Volume_Exists &= ~0xC0; //0x80+0x40
		case FC_ENTER_NEXTFILE_M88:
Start_button_file_in_case_of_NoAnim:
			Next_File_Num = *FS_arg;
			ResetPause();
			File_Open_Preparation();
			break;
		case FC_NEXT_FILE_M98:
			AfterLoop_File_Num = *FS_arg;
			AfterMulti_File_Num = 0xFFFF;
			break;
		case FC_AFTER_ANIM_FILE_M98:
			if (0 == (LED_control_type & 4)) {goto Start_button_file_in_case_of_NoAnim;}
			AfterMulti_File_Num = *FS_arg;
			AfterLoop_File_Num = 0xFFFF;


			break;
		case FC_REPEAT_FILE_M47:
			if ((*FS_arg == 0) || FS_b8 || ((File_Read_Cycle_Count !=1) && (*FS_arg != 1))){ //было (*FS_arg == 0) || ((File_Read_Cycle_Count !=1) && (*FS_arg != 1))
				//M47 без параметра/параметр 0, или это по кнопке, или это ещё не последний цикл
				//перезапускаем файл
				FileReadStartTime = CoGetOSTime();
				if (LED_control_type < 128) file.fptr=0; // сброс чтения текущего файла на начало
				//if (DS_Channel_Select==1){descrActionFlag = descrActionFlag | DS_ACTION_QUIT_DESCRIPT;}
				if (DS_Channel_Select!=1){ResetPause();}
				descrActionFlag = descrActionFlag | DS_ACTION_QUIT_DESCRIPT;
				//проверка: если параметр не 0, но File_Read_Cycle_Count = 0;,
				//значит мы первый раз в файле втречаем эту команду
				//и надо назначить File_Read_Cycle_Count
				//а если File_Read_Cycle_Count > 0, то надо отсчитывать разы
				if (FS_b8) {File_Read_Cycle_Count = 0;}//если это по кнопке то FS_b8=1, значит цикл начинаем сначала
				if (*FS_arg > 0 || File_Read_Cycle_Count > 1){
					if (File_Read_Cycle_Count == 0) {
						File_Read_Cycle_Count = *FS_arg-1;
					} else {
						File_Read_Cycle_Count --;
					}
				}
			}/* else {//файл не будет перезапущен - обнуляем отложенное событие кнопок
				buttonRelInterruptActive |= buttonRelWaitStateEndFile; // соответствующее прерывание делаем активным, inputsTask на него реагирует
				buttonPushInterruptActive |= buttonPushWaitStateEndFile; // соответствующее прерывание делаем активным, inputsTask на него реагирует
			}/**/
			break;
		case FC_REPEAT_COUNTS_M47:
			File_Read_Cycle_Count = *FS_arg;
			break;
		case FC_RESUME_FILE_M24://выходим из спячки
			if (DS_Pause_interrupt_Flag & 128){//reastart playing
M24_Execution_Path_Label:
				//нужно вычислить паузы
				osTime = CoGetOSTime()-File_Suspend_start;//определяем сдвиг всех пауз
				FileReadStartTime += osTime;//сдвиг времени старта чтения файла
				nextAbsolutPauseExpireTime += osTime;//вдвиг времени срабатывания следующей абсолютной паузы
				nextPauseStartTime += osTime;//вдвиг времени срабатывания следующей относительной паузы
				DS_Pause_interrupt_Flag &= ~128;
			}
			break;
		case FC_SUSPEND_FILE_M25://входим в спячку
			if ((0x180==(DS_Pause_interrupt_Flag & 0x180)) && CoGetOSTime() > M25ReverseTime){
				goto M24_Execution_Path_Label;
			}
			if (0 ==( DS_Pause_interrupt_Flag & 128)) {//suspend playing
				//нужно вычислить паузы
				File_Suspend_start = CoGetOSTime();
				M25ReverseTime = File_Suspend_start + M25ReversePauseValue;
				DS_Pause_interrupt_Flag |= 128;
			}
			break;
		case FC_PAUSE_END_G4:
			nextPauseStartTime += *FS_arg;
			DS_Pause_interrupt_Flag |=1; //поднимаем флаг паузы
			Output_to_WS();
			break;
		case FC_PAUSE_END_G5: //пауза без обновления
			nextPauseStartTime += *FS_arg;
			DS_Pause_interrupt_Flag |=1; //поднимаем флаг паузы
			break;
		case FC_PAUSE_END_G9: //команда G9
			ResetPause(); // сброс ожидания конца текущей паузы
			break;
		case FC_RANDOM_SEED_M90://установка значения старта генератора случайных чисел
			srand(*FS_arg);
			break;
		case FC_RANDOM_SEED_TIMER_M90:// random seed = timer
			srand(CoGetOSTime());
			break;
		case FC_SERVO_ENABLE_M3:
			Servo_Action_Mask |= FS_b8; //включаем только нужный сервомотор
			Servo_Update_Flag |=2; //флаг изменения позиций серв
			Servo_GPIO_Setting_on &= ((~FS_b8) << 8);
			Servo_GPIO_Setting_off &= ((~FS_b8) << 8);
			if ((LED_control_type & 3) == 3){
				WS2812_IO_High &= ~FS_b8;
				WS2812_IO_Low &= ~FS_b8;
				WS2812_framedata_Set_HighLow(WS2812_IO_framedata,DS_maxCurrPoints);
			}
			break;
		case FC_SERVO_DISABLE_M5:
			if (FS_b8 == 0xFF){
				stop_servos();
			} else {
				Servo_Action_Mask &= ~FS_b8; //выключаем не нужный сервомотор
				Servo_Update_Flag |=2; //флаг изменения позиций серв
			}
			break;
		case FC_PIN_ENABLE_M10:
			if (LED_control_type & 2) {
				Servo_GPIO_Setting_on |=   (FS_b8 << 8);
				Servo_GPIO_Setting_off &=   ((~FS_b8) << 8);
				WS2812_IO_High |= FS_b8; //поднимаем светодиодные выводы туда же
				WS2812_IO_Low |= FS_b8;
				Servo_Action_Mask &= ~FS_b8;// соотносим маску включённых серв и маску включённых светодиодов
				Servo_Update_Flag |=2; //флаг изменения позиций серв
				WS2812_framedata_Set_HighLow(WS2812_IO_framedata,DS_maxCurrPoints);

			}
			break;
		case FC_PIN_DISABLE_M11:
			if (LED_control_type & 2) {
				Servo_GPIO_Setting_on &=   ((~FS_b8) << 8);
				Servo_GPIO_Setting_off |=   (FS_b8 << 8);
				WS2812_IO_High &= ~FS_b8; //поднимаем светодиодные выводы туда же
				WS2812_IO_Low &= ~FS_b8;
				Servo_Action_Mask &= ~FS_b8;// соотносим маску включённых серв и маску включённых светодиодов
				Servo_Update_Flag |=2; //флаг изменения позиций серв
				WS2812_framedata_Set_HighLow(WS2812_IO_framedata,DS_maxCurrPoints);
			}
			break;
		case FC_SERVO_REALTIME_G1:
			LED_control_type |= (LED_control_type & 2)*0x0C;//0x0C=12=(0x08 + 0x04)
			Servo_Update_Flag |= (LED_control_type & 2);//поднять флаг смены значенийб если сервами тоже управляем
			break;
		case FC_SERVO_FRAMEBASED_G1:

			LED_control_type &= ~((LED_control_type & 8)*3);
			break;
		case FC_SERVO_POS_SUBST_G0:
			DS_Math_Flags =1;
			//DS_Flag_Register |= 4;
		case FC_SERVO_POS_ADD_G0:
			DS_Math_Flags +=1;
			//DS_Flag_Register |= 8;
		case FC_SERVO_POS_SET_G0:
			for (a8 = 0; a8<8; a8++){
				if (FS_b8 & (1<<a8)){
					Servo_Pos_Temp[a8] =  ModifyParameter(Servo_Pos_Temp[a8], FS_arg, Servo_Resolution);
					//if (Servo_Pos_Temp [a8] > Servo_Resolution) {Servo_Pos_Temp [a8] = Servo_Resolution;}
				}
			}
			Servo_Update_Flag |= 2; //флаг смены значений поднят

			break;
		case FC_SERVO_POS_RAND_G0:
			for (a8 = 0; a8<8; a8++){
				if (FS_b8 & (1<<a8)){
					Servo_Pos_Temp[a8] =  rand()%(Servo_Resolution+1);

				}
			}
			Servo_Update_Flag |= 2; //флаг смены значений поднят
			break;
		case FC_BRIGHTNESS_SUBST_G27:
			DS_Math_Flags =1;
			//DS_Flag_Register |= 4;
		case FC_BRIGHTNESS_ADD_G27:
			DS_Math_Flags +=1;
			//DS_Flag_Register |= 8;
		case FC_BRIGHTNESS_SET_G27:
			DS_LED_Brightness = 1 + ModifyParameter(DS_LED_Brightness-1, FS_arg, MAX_BRIGHTNESS);
			//if (DS_LED_Brightness > MAX_BRIGHTNESS) {DS_LED_Brightness = MAX_BRIGHTNESS;}
			break;
		case FC_CHOSEN_FILE_SUBST_M91:
			DS_Math_Flags =1;
			//DS_Flag_Register |= 4;
		case FC_CHOSEN_FILE_ADD_M91:
			DS_Math_Flags +=1;
			//DS_Flag_Register |= 8;
		case FC_CHOSEN_FILE_SET_M91:
			ChosenNomberInM98 = ModifyParameter(ChosenNomberInM98, FS_arg, Max_Random_File_List);
			//if (ChosenNomberInM98 > Max_Random_File_List) {ChosenNomberInM98 = Max_Random_File_List;}
			break;
		case FC_FAST_ANIM_FREQ_SUBST_G30:
			DS_Math_Flags =1;
			//DS_Flag_Register |= 4;
		case FC_FAST_ANIM_FREQ_ADD_G30:
			DS_Math_Flags +=1;
			//DS_Flag_Register |= 8;
		case FC_FAST_ANIM_FREQ_SET_G30:

			*FS_arg =  ModifyParameter((SERVO_TICK_FREQ/WS2812_Frame_Period), FS_arg, (SERVO_FREQ_MAX << 4)/WS2812_Frame_Length);

			if (*FS_arg < SERVO_FREQ_MIN) {*FS_arg = SERVO_FREQ_MIN;}
			//if (*FS_arg > SERVO_FREQ_MAX) {*FS_arg = SERVO_FREQ_MAX;}//скорость при 16 пикселах
			//if (*FS_arg > ((SERVO_FREQ_MAX << 4)/WS2812_Frame_Length)) {*FS_arg = (SERVO_FREQ_MAX << 4)/WS2812_Frame_Length;}
			WS2812_Frame_Period = SERVO_TICK_FREQ / *FS_arg;
			if (FS_b8 == 0){WS2812_Frame_Period_Actual = WS2812_Frame_Period;} //если это с кнопки, то обновляем тайминг немедленно
			break;
		case FC_MISCOMP_SUBST_M45:
			DS_Math_Flags =1;
			//DS_Flag_Register |= 4;
		case FC_MISCOMP_ADD_M45:
			DS_Math_Flags +=1;
			//DS_Flag_Register |= 8;
		case FC_MISCOMP_SET_M45:
			CheckPixel_miscomparations = ModifyParameter(CheckPixel_miscomparations, FS_arg, 0xFFFF);
			break;
		case FC_FAST_ANIM_PLAY_G35:
			WS2812_Frame_Total_Count = *FS_arg;
			if (FS_b8 == 1) {break;} //если это с кнопки (FS_b8 == 0), то сразу запускаем
		case FC_FAST_ANIM_ON_G36:

			//включается только в режиме вывода светодиодов
			if (LED_control_type == 1){
				//stop_servos();
				LED_control_type &= 128;
				LED_control_type |= 4; //режим мультивывода
				Servo_Update_Flag |=0x20; //параметры поменялись
				if (FS_b8 == 0) {Output_to_WS();}
			}
			break;
		case FC_FAST_ANIM_OFF_G36:
			if ((LED_control_type&127) == 4){
				stop_servos();
			}
			break;
		default:
			ButtonFlags &=~8; //отмечаем, что оно вработало вхолостую
			break;
	}

}




#endif
//*-----------------------------------------------------------------------------------------------
/**			Инициализация тестовой задачи														*/
//*-----------------------------------------------------------------------------------------------
void DebugTaskInit(void)
{
	// инициализация тестовой задачи
/*	debugTaskID = CoCreateTask(TestTask,	// указатель на задачу
				(void *)0,	// указатель на передаваемые в задачу данные, не используем
				10,			// приоритет задачи
				&testTaskStk[TEST_TASK_STK_SIZE-1],		// указатель на конец области стека задачи
				TEST_TASK_STK_SIZE);					// размер стека, слов (4-байтных)
/**/


TestTask();
}


//*-----------------------------------------------------------------------------------------------
/**			Тело задачи
 *
 * @param pdata - указатель на доп. параматры, не используется									*/
//*-----------------------------------------------------------------------------------------------
void TestTask(void)//* pdata)
{


	//int i;
	//FRESULT result;
	//ff_timer_init();//запуск функции RAMFUNC void disk_timerproc (void) каждые 10 мс,
	//функция вытаскивается через extern void disk_timerproc(void) и лежит в sd_spi_stm32.c
	//задача этой функции - внутренние таймеры 100Гц для работы SPI на SD карте.

	//fileReadStartFlag = CoCreateFlag(0, 1);   // Сбрасывается вручную, изначально установлен
	//r2 = CoSetFlag (fileReadStartFlag); //чтение из файла - запускаем
	//if (r2 != E_OK) {
	//	if (r2 == E_INVALID_ID){printf("Invalid fileReadStartFlag ID !\n");}
	//}
	// 0.2 сек. для окончания переходных процессов
	CoTickDelay(200);

	DS_Channel_Select = 0; //выбор канала - любой
	DS_Pause_interrupt_Flag = 0; //паузу не находили
	//установка таймингов
	ResetPause();
	FileReadStartTime = CoGetOSTime();
	nextAbsolutPauseExpireTime = FileReadStartTime;
	File_Read_Cycle_Count = 0;

	//printf("Disk mounting: ");
	//start_timer();
	//do{
	for (a8 = 0; a8 < DIR_PATH_MAX_LENGTH; ++a8) {
		Dirpath[a8] = '\0';
		//FileworksDirpath[a8]= '\0';
	}
	/*for (a8 = 0; a8 < BUTTONS_TEXT_ARRAY_SIZE; ++a8){
		TextToSendArrayForButton[a8] = '\0';
	}
	for (a8 = 0; a8 < BUTTONS_TEXTS_AMOUNT; ++a8){
		TextToSendArrayIndexesForButton[a8] = 0xFF;
	}/**/
	memcpy(Dirpath, "0:/hcd", 6);
	//memcpy(FileworksDirpath, "0:/hcd", 6);
	Reset_FileworkDirPath ();
	File_Open_Flags = FA_OPEN_EXISTING | FA_READ;
	AfterLoop_File_Num = 0xFFFF;
	AfterMulti_File_Num = 0xFFFF;
	Current_File_Num = 0;
	Parent_File_Num = 0;
	Parent_Prev_File_Num = 0;
	Next_File_Num = 0;
	Prev_File_Num = 0;
	SD_Volume_Exists |= 0x10; //мы не находили карточку (0x10) + мы проверяем CRC (0x1)

	//Servo_Update_Flag = 0;

	/*result = f_mount(0, &FATFS_Obj);


	//Message ("Random text\r\n\0");
	//printf(Dirpath); //vet
	//printf("\r\n");
	//}
	//while ((result != FR_OK) && --a8);
	//time = stop_timer();
	if (result != FR_OK)
	{
		Fail_Message(3,3,result);//printf("FAT error %d \r\n", result);
		descrActionFlag = 0;
	} else {
		//(start)поиск в папке /hcd файла, начинающегося с символа "ноль"
		result = f_opendir (&dir, Dirpath );
		result = f_opendir (&dir, Dirpath );//почему-то с первого раза он не видит папку или содержимое массива.
		//result = f_opendir (&dir, "0:/hcd" );
		//if (DebugInfoOutFlag){printf("opening dir 0:/hcd/, result = %d\r\n", result);}
		if (result != FR_OK) {
			descrActionFlag = 0;
			Fail_Message(2,2,result);//printf("DIR error %d \r\n", result);
			//SD_Volume_Exists = 0x10;
		} else {
			filePathByFirstNum(0, 4, path, fname, Dirpath, &dir, &filinfo);
			//File_Open_Message();//if (DebugInfoOutFlag){printf("(debugTask) (startup) executing file ");	printf(path);	printf("\r\n");}
			descrActionFlag = descrActionFlag | DS_ACTION_GOTO_FILE; //установка бита-флага перейти к файлу
			SD_Volume_Exists &= ~0x10; //SD присутствует (открылась с первого раза)
		}
	}/**/
	MainLoop_ResetTime = 0;
	MainLoop();

}
//-------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------(mainloop begin) // цикл "чтение-дешифровка"
void MainLoop(){
	FRESULT result;
	do {

		if (SD_Volume_Exists & 0x10){ //мы пока не находили карту памяти. Но надо проверить, вдруг она там появилась
			//для дескриптора: делаем вид, что в файл пустой
			//FileBufferEnding();
			//проверим, не подсоединили ли нам карточку
			result = f_mount(0, &FATFS_Obj);
			if (result == FR_OK)
			{
				result = f_opendir (&dir, Dirpath );
				if (result == FR_OK) {
					if (LED_control_type < 128) filePathByFirstNum(Current_File_Num, 4, path, fname, Dirpath,&dir, &filinfo);
					//Message ("Openning \0",1);
					//Message (path,1);
					//Message ("\r\n\0",1);
					//descrActionFlag = descrActionFlag | DS_ACTION_GOTO_FILE; //установка бита-флага перейти к файлу
					SD_Volume_Exists &= ~0x10; //SD присутствует (открылась с первого раза)
				}
			}
		}

		if (LED_control_type < 128){ //у нас не режим работы с ФС
#ifndef FastButtonShortcuts
		if	(descrActionFlag & DS_ACTION_BUTTON_INTERRUPT){
			descrActionFlag = descrActionFlag & ~DS_ACTION_BUTTON_INTERRUPT; //сброс флага выхода из дешифровки, чтобы дальше всё работало как раньше
			r2 = CoSetPriority(inputsTaskID, 11); //притормаживаем зачачу опроса кнопок, чтобы у нас на ходу не поменялись переменные
			ResetPause();				 	// сброс ожидания конца текущей паузы


			if (buttonPushInterrupt) {
				buttonInterrupt_tmp = buttonPushInterruptActive;
				buttonPushInterruptActive &= ~buttonPushWaitStateEndFile; // на случай если выход был по отложенному концу файла
				buttonInterrupt = &buttonPushInterrupt;
				a16 = 0;
			} else {//if (buttonRelInterrupt)
				buttonInterrupt_tmp = buttonRelInterruptActive;
				buttonRelInterruptActive &= ~buttonRelWaitStateEndFile; // на случай если выход был по отложенному концу файла
				buttonInterrupt = &buttonRelInterrupt;
				a16 = MAX_NUM_BUTTONS;
			}
			for (a8 = 0; a8 < MAX_REAL_BUTTONS; ++a8) {
				if ( (*buttonInterrupt & 1) && (buttonInterrupt_tmp & 1) ){
					Next_File_Num = numFileForButton[a8+a16];
					r2 = CoClearFlag(outputStartFlag);	//следующий вывод на диоды будет ждать команды от дешифровки
					r2 =  CoResetTaskDelayTick(outputTaskID, 0);		// прерывание ожидания задачи
					a8 = MAX_NUM_BUTTONS+1; // выход из цикла
				}
				*buttonInterrupt = *buttonInterrupt >> 1; // подтаскиваем испытуемые биты к концу
				buttonInterrupt_tmp = buttonInterrupt_tmp >> 1;
			}
			File_Open_Preparation();
			r2 = CoSetPriority(inputsTaskID, 10); //притормаживаем зачачу опроса кнопок, чтобы у нас на ходу не поменялись переменные
		}
#endif

		if	(AfterMulti_File_Num != 0xFFFF){//была нажата кнопка во время мультикадра
			descrActionFlag &= ~DS_ACTION_BUTTON_TO_END_MULTI;
			descrActionFlag &= ~DS_ACTION_BUTTON_INTERRUPT;
			Next_File_Num = AfterMulti_File_Num;
			ResetPause();
			File_Open_Preparation();
		}
		} //варианты определения имени файла для режима анимации закончилсь
		DS_buf_start = 0;
	if ((SD_Volume_Exists & 0x10)==0) { //если есть память в слоте (или, по крайней мере, была прошлый раз)


		if	(descrActionFlag & DS_ACTION_GOTO_FILE){ //
			//запуск тайминга файла переставлен в месте чтения имени нового файла
				result = f_open(&file, path, File_Open_Flags);

			/*if (result != FR_OK){
				Fail_Message(1,2,result);
				a8 = 0; //считаем попытки
				do {
					a8++;
					result = f_mount(0, &FATFS_Obj);
				} while ((a8<5) && (result != FR_OK));
				if (result != FR_OK) { //бросаем попытки
					Fail_Message(3,3,result);
					SD_Volume_Exists |= 0x10;
				} else {
					a8 =0;
					do {//переоткрываем файл
						result = f_open(&file, path, File_Open_Flags);
						a8++;
					} while ((a8<5) && (result != FR_OK));
					if (result != FR_OK) { //бросаем попытки
						SD_Volume_Exists |= 0x30;
						Fail_Message(1,2,result);
					} else {
						SD_Volume_Exists &= ~0x30;
					}

				}

			}*/
			//time = stop_timer();

			read_size = 0;				//сброс указателей на позицию в файле
			//FilePTR_Backup=0;
			DS_status = DS_Type_command_new; 		//сброс состояния дешифровщика - в начале файла должно быть начало расшифровки

			descrActionFlag = descrActionFlag & (~DS_ACTION_GOTO_FILE); //файл открыли - сброс флага
			if ((LED_control_type&127) == 4) stop_servos(); //останавливаем мультикадровую анимацию

		}

		if (SD_Volume_Exists&0x80) { //возврат из подпрограммы в основную программу
			Prev_File_Num = Parent_Prev_File_Num;
			DS_buf_start=DS_buf_start_backup;
			file.fptr = FilePTR_Backup;
			file.clust = FileClust_Backup;
			File_Read_Cycle_Count = File_Read_Cycle_Count_Backup;
			String_in_file_Index = String_in_file_Index_Backup;
			AfterLoop_File_Num = AfterLoop_File_Num_Backup;

			SD_Volume_Exists &= ~0xC0; //0x80+0x40
		} else if (0 == (SD_Volume_Exists&0x40)){//если это основной файл, то сохраняем позицию
			FilePTR_Backup = file.fptr;
			FileClust_Backup = file.clust;
			//FileFsWinsect_Backup=file.fs->winsect;
			//FileDsect_Backup=file.dsect;
		}

		//if ((~descrActionFlag & DS_ACTION_RETURN_FROM_AFK)){//если мы зависали, нам не надо читать следующий кусок файла
			Read_Data_Chunk_From_File();//&result);
		//}
		if ((~SD_Volume_Exists)& 0x10){//мы всё прочитали, проверяем на конец файла
				//full_size += read_size;
				//DS_buf_start = 0;
			DS_buf_counter = read_size-1;
			if (file.fptr >= file.fsize && LED_control_type < 128) { //если оказалось, что файл прочитан до конца,
					// в буфер добавляем два символа: \r\n, чтобы дешифратор не подумал, что файл не дочитался и не запросил ещё кусок
				text_buff[read_size] = '\r';
				text_buff[read_size+1] = 'M';
				text_buff[read_size+2] = '2';
				text_buff[read_size+3] = '\n';
				DS_buf_counter = read_size+3;
			}
		}
	} //(SD_Volume_Exists == 0)

if (SD_Volume_Exists&0x10){//работаем без карты памяти
	//DS_buf_start = 0;
	read_size = 0;
	if (LED_control_type < 128){ //у нас не режим работы с ФС
	text_buff[0] = '\r';
	text_buff[1] = 'M';
	text_buff[2] = '2';
	text_buff[3] = '\n';
	DS_buf_counter = 3;}
}

		//time = stop_timer();

	//-------------------------------- расшифровка прочитанного  ---------------------------------------------------------------------
#if defined (FastButtonShortcuts)
	descrActionFlag = 0;
#endif
#ifndef FastButtonShortcuts
		descrActionFlag &= DS_ACTION_BUTTON_INTERRUPT;
#endif
		//поскольку опрос кнопок и чтение/дешифровка происходят по своим таймингам,
		//иногда descrActionFlag обнуляется не успев быть обработанной.
		//Так что конкретный бит кнопок мы отключаем только в куске обработки кнопок

		//do
		while (  descrActionFlag == 0 ){
			//if (DS_status == DS_Type_command_RAW) {
			//	do {
			//		WS2812_IO_framedata[DS_WSpoint_counter] = text_buff[DS_buf_start];
			//		++DS_WSpoint_counter;
			//		++DS_buf_start;
			//	} while ( (DS_buf_start < DS_buf_counter) && (DS_WSpoint_counter < DS_Param) );
			//}

			//вот тут должен быть выбор из разных дескриптов - если ничего, то дескрипт файла, если сработал флаг usart, то дескрипт его, если сработал флаг usb, то его.
			//и при этом надо, чтобы последняя расшифровываемая команда была полностью прочитана
			//мы можем это понять, когда DS_Channel_Select = 0x10 или 0
			//то есть махаем флагами
			if ((DS_Channel_Select&0x0F)==0){
				descrResult=DS_ANS_WS_S_DONE;
				if (DS_Channel_Select==0 && LED_control_type < 128){DS_Channel_Select = 1;} //предполагаем, что читается и исполняется команда из файла

				if (dbgu_State_of_recieved_Command & 0x80){ //... но если вдруг у нас есть что почитать от USART2, то
					DS_Channel_Select = 3;
					USART_Buf_pointer = dbgu_rx_buff;
					USART_counter_pointer = &dbgu_rx_buf_counter;
					USART_ptr_State_of_recieved_Command = &dbgu_State_of_recieved_Command;
					//dbgu_rx_temp_undescripted = &dbgu_State_of_Commands_undescripted;
					USART_ptr_rx_start_time = &dbgu_rx_start_time;
					dbgu_rx_temp_counter = *USART_counter_pointer;
					//dbgu_rx_temp_counter = dbgu_rx_buf_counter;
					dbgu_rx_temp_start = dbgu_rx_buf_start;
					dbgu_rx_buf_overcount=0;
				} else if(USART1_State_of_recieved_Command & 0x80){
					DS_Channel_Select = 2;
					USART_Buf_pointer = USART1_rx_buff;
					USART_counter_pointer = &USART1_rx_buf_counter;
					USART_ptr_State_of_recieved_Command = &USART1_State_of_recieved_Command;
					//dbgu_rx_temp_undescripted = &USART1_State_of_Commands_undescripted
					USART_ptr_rx_start_time = &USART1_rx_start_time;
					dbgu_rx_temp_counter = *USART_counter_pointer;
					//dbgu_rx_temp_counter = dbgu_rx_buf_counter;
					dbgu_rx_temp_start = USART1_rx_buf_start;
					dbgu_rx_buf_overcount=0;

				} else if (USB_Function_flags & 2){//there is something in USB
					DS_Channel_Select = 4;
				}


			}

			//__disable_irq();
			if (DS_Channel_Select==1){ //читаем из файла, мы сюда не заходим, если у нас режим работы с ФС
				//printf("Descript file, DS_buf_start = %d, DS_buf_counter = %d, max position in buffer 0x%03X\r\n", DS_buf_start, DS_buf_counter, READ_SIZE);

				if ((~SD_Volume_Exists) & 0x10){
					descrResult = Descript(WS2812_IO_framedata, text_buff, &DS_buf_start, &DS_buf_counter,
						&DS_status, &DS_comm_num, &DS_WSpoint_counter, &DS_RGB_counter, &DS_maxCurrPoints, &DS_Param);
//					descrResult = Descript(WS2812_IO_framedata, text_buff, &DS_buf_start, &DS_buf_counter,
//						&DS_status, &DS_comm_num, &DS_WSpoint_counter, &DS_RGB_counter, &DS_maxCurrPoints, &DS_Param);
				} else {
					//if (CoGetOSTime() > Unfreeze_Read_From_Ports_when_SD_failed_At){
						DS_Channel_Select = (SD_Volume_Exists & 0x10);
						file.fptr = 0;
						DS_status=DS_Type_command_new;
						descrResult=DS_ANS_WS_S_DONE;
					//}
				}
				//printf("Descript file \r\n");
			} else if ( (DS_Channel_Select & (~1)) == 2){ //читаем из буфера USART1/2


				//надо сделать проверку на случай если dbgu_State_of_recieved_Command==0x10
				//это значит команда была в буфере не до конца и теперь пришло продолжение
				while (*USART_ptr_State_of_recieved_Command==0x30){
					if (CoGetOSTime() > *USART_ptr_rx_start_time){*USART_ptr_State_of_recieved_Command = 0x21;}
				} //ждём, пока придёт команда или будет таймаут приёма
				//возможно, что тут окажется, что мы ждали и не дождались, то есть прим сброшен и надо выйти из цикла
				//это можно будет понять если dbgu_State_of_recieved_Command=0x21, иначе будет 0x80 (пришло всё) или 0x90 (следующий кусок)

				if ((*USART_ptr_State_of_recieved_Command & 0x80) && (CoGetOSTime() <= *USART_ptr_rx_start_time)){ //мы всё ещё читаем
					if (dbgu_rx_buf_overcount & 2){
						dbgu_rx_temp_counter = *USART_counter_pointer;
						dbgu_rx_buf_overcount &=  ~2;
					}
					if (dbgu_rx_buf_overcount & 1) {//мы прочитали кусок команды, который был в конце буфера, и теперь читаем кусок сначала буфера
						dbgu_rx_temp_start=0;
						dbgu_rx_temp_counter = *USART_counter_pointer;
						dbgu_rx_buf_overcount = 0;
					} else if (dbgu_rx_temp_start > dbgu_rx_temp_counter){ //мы читаем до конца буфера на этом проходе, а на следующем надо будет прочитать ещё кусочек сначала
						//мы сюда попадаем только если нам надо будет на следующем проходе прочитать сначала буфера остаток команды
						//при этом если вдруг буфер был заполнен ровно до конца, то сюда мы не попадаем
						dbgu_rx_temp_counter = DBGU_RX_SIZE-1;
						dbgu_rx_buf_overcount = 1;
					}

					descrResult = Descript(WS2812_IO_framedata, USART_Buf_pointer, &dbgu_rx_temp_start, &dbgu_rx_temp_counter,
						&DS_status, &DS_comm_num, &DS_WSpoint_counter, &DS_RGB_counter, &DS_maxCurrPoints, &DS_Param);

					if (DS_status==DS_Type_command_new || DS_status == DS_Type_command_comment){ //мы полностью прочитали команду
						DS_status=DS_Type_command_new;
						//*dbgu_rx_temp_undescripted --;
						*USART_ptr_State_of_recieved_Command=0x21;
						//if (dbgu_rx_temp_start < dbgu_rx_temp_counter){
						//	*USART_ptr_State_of_recieved_Command|=0x80;
							//dbgu_rx_temp_start=dbgu_rx_temp_counter;
						//}

					} else {
						//надо проверить, мы просто подошли к макс значению буфера или мы узнали, что команда прислана не полностью, а мы просто прочитали половину буфера
						//то есть dbgu_rx_buf_overcount == 1 если подошли к макс значению буфера и ещё не знаем, полностью ли прислана команда
						if (dbgu_rx_buf_overcount != 1){
							*USART_ptr_State_of_recieved_Command=0x30;
						} //ждём когда прочитаем ещё
						dbgu_rx_temp_start=dbgu_rx_temp_counter;
						dbgu_rx_buf_overcount |= 2;
					}
				} else {//if (*USART_ptr_State_of_recieved_Command!=0x30) {
					descrResult = 0;
					//сваливаемся в дефолт
				}
			} else if (DS_Channel_Select == 4 && USB_Recieved_bytes){
				//Message("usb OUT\r\0",2);
				descrResult = 0;
				/*while (USB_Function_flags & 0x10){
					if (CoGetOSTime() > USB_rx_timeout){
						USB_Function_flags = 8;
						descrResult = 0;
						goto USB_Skip_reading_buffer_and_Abort_Label;
					}
				}*/
				USB_Recieved_bytes--;
				//print_0X4(DS_status,2);
//				USB_Function_flags |= 0x10;//not full
				descrResult = Descript(WS2812_IO_framedata, USB_Buff1, &USB_RX_Start_pointer, &USB_Recieved_bytes,
										&DS_status, &DS_comm_num, &DS_WSpoint_counter, &DS_RGB_counter, &DS_maxCurrPoints, &DS_Param);
USB_Skip_reading_buffer_and_Abort_Label:

				if (DS_status==DS_Type_command_new || DS_status == DS_Type_command_comment){ //мы полностью прочитали команду
					DS_status=DS_Type_command_new;
				}


				if (descrResult >= DS_ERR_MIN_ERROR_NUMBER){
					descrResult = DS_ANS_WS_S_DONE;
					DS_status=DS_Type_command_new;
					//USB_Function_flags = 8;
					//Fail_Message(6,4,0);
						while (USB_Buff1[USB_RX_Start_pointer] != 0x0D &&
								USB_Buff1[USB_RX_Start_pointer] != 0x0A &&
								USB_Buff1[USB_RX_Start_pointer] != 0x00 &&
								USB_Buff1[USB_RX_Start_pointer] != DS_Symbol_comment1 &&
								USB_Buff1[USB_RX_Start_pointer] != DS_Symbol_comment2 &&
								USB_RX_Start_pointer <= USB_Recieved_bytes)
						{
							USB_RX_Start_pointer++;
						}

				}/**/

				if (USB_RX_Start_pointer >= USB_Recieved_bytes){
					USB_Recieved_bytes = -1;
					USB_RX_Start_pointer = 0;
					USB_Function_flags &= 0xFD;//data processed
					//USB_Signal_End_Of_Data_Cunsumed;
					EP3R = (EP3R^0x3000)&0xBF8F;
				}
				USB_Recieved_bytes++;

			}
			//__enable_irq();
Jump_To_Action_Label:
			//MainLoop_AFK_Flag &= 0xfe;
			MainLoop_ResetTime = 0;

			switch (descrResult) {
				case DS_ANS_WS_S_DONE:
					//читаем дальше, всё, что нужно, сделали в дескрипте
					break;
				case DS_WS_SET_MAX_LENGTH:
					DS_TotalPoints = (DS_Param < WS2812_IO_FRAMEDATA_PIXELS)? DS_Param : WS2812_IO_FRAMEDATA_PIXELS;
					break;
				case DS_ANS_READ_ON:
					if (DS_Channel_Select==1){descrActionFlag = descrActionFlag | DS_ACTION_QUIT_DESCRIPT;}
					//if (DS_Channel_Select==4) Message("usb OUT\r\0",2);
					// надо просто прочитать следующую порцию данных из файла - данные по статусу текущей принимаемой команды не изменяются
					// все уже прочитанные символы расшифрованы и состояние запомнено
					break;
				case DS_ANS_STOP:
					break;
				case DS_ANS_REPEAT_FILE: //M2, M47, M47 P<>, M98

					//входные параметры - DS_Param
					if (AfterLoop_File_Num == 0xFFFF){


#if defined (FastButtonShortcuts)
						//вся логика просто перенесена в процедуру быстрых команд
						FastCommand(FC_REPEAT_FILE_M47,0,&DS_Param);
#endif
#ifndef FastButtonShortcuts
					if ((DS_Param == 0) || ((File_Read_Cycle_Count !=1) && (DS_Param != 1))){
						//M47 без параметра, или параметр 0, или это ещё не последний цикл
						//перезапускаем файл
						FileReadStartTime = CoGetOSTime();
						if (LED_control_type < 128) file.fptr=0; // сброс чтения текущего файла на начало
						//if (DS_Channel_Select==1){descrActionFlag = descrActionFlag | DS_ACTION_QUIT_DESCRIPT;}
						if (DS_Channel_Select!=1){ResetPause();}
						descrActionFlag = descrActionFlag | DS_ACTION_QUIT_DESCRIPT;
						//проверка: если параметр не 0, но File_Read_Cycle_Count = 0;,
						//значит мы первый раз в файле втречаем эту команду
						//и надо назначить File_Read_Cycle_Count
						//а если File_Read_Cycle_Count > 0, то надо отсчитывать разы
						if (DS_Param > 0){
							if (File_Read_Cycle_Count == 0) {
								File_Read_Cycle_Count = DS_Param-1;
							} else {
								File_Read_Cycle_Count --;
							}
						}
					} else {//файл не будет перезапущен - обнуляем отложенное событие кнопок
						buttonRelInterruptActive |= buttonRelWaitStateEndFile; // соответствующее прерывание делаем активным, inputsTask на него реагирует
						buttonPushInterruptActive |= buttonPushWaitStateEndFile; // соответствующее прерывание делаем активным, inputsTask на него реагирует
					}
#endif

					} else { //отложенная реакция на кнопку
						Next_File_Num = AfterLoop_File_Num;
						if (DS_Channel_Select!=1){ResetPause();}
						File_Open_Preparation();
					}
					break;
#ifndef FastButtonShortcuts
				case DS_ANS_PAUSE:
					nextPauseStartTime += DS_Param;
					DS_Pause_interrupt_Flag |=1; //поднимаем флаг паузы
					Output_to_WS();
					break;
				case DS_ANS_PAUSE2: //пауза без обновления
					nextPauseStartTime += DS_Param;
					DS_Pause_interrupt_Flag |=1; //поднимаем флаг паузы
					break;
				case DS_ANS_RESET_PAUSE: //команда G9
					DS_Param=0;   //сброс значения дополнительной паузы
					ResetPause(); // сброс ожидания конца текущей паузы
					break;
#endif

				case DS_ANS_PAUSE:
				case DS_ANS_PAUSE2: //пауза без обновления
					FastCommand(descrResult+FC_PAUSE_END_G4-DS_ANS_PAUSE,0,&DS_Param);
					break;
				case DS_ANS_RESET_PAUSE: //команда G9
					FastCommand(FC_PAUSE_END_G9,1,&DS_Param);
					break;

				case DS_ANS_PAUSE_ABSOLUT2: //G7, пауза с точным значением времени относительно старта чтения файла
					//пауза без обновления
					//принцип действия как G6
					DS_Pause_interrupt_Flag |=6; //поднимаем флаг абсолютной паузы (2) и запрет обновления светодиодов (4)
					//nextAbsolutPauseExpireTime = FileReadStartTime + DS_Param; //назначаем время
					//break;
				case DS_ANS_PAUSE_ABSOLUT: //G6, пауза с точным значением времени вывода относительно старта чтения файла
					//принцип действия отличается от G4
					//описаниее команды:
					//вывести на светодиоды всё, что подготовлено
					//сразу после определённой милисекунды, после чего продолжить чтение/расшифровку
					//это значительно уменьшит время между идеальным таймингом и реальным
					//т.к. первое, что делает эта команда - это вывод,
					//а чтение и расшифровка продолжаются после вывода
					DS_Pause_interrupt_Flag |=2; //поднимаем флаг абсолютной паузы
					nextAbsolutPauseExpireTime = FileReadStartTime + DS_Param; //назначаем время
					break;
				case DS_ANS_SET_ABSOLUT_TIME: //назначаем время, которое прошло от начала файла
					//нужно прежде всего для синхронизации нескольких контроллеров
					//osTime = CoGetOSTime();
					//определяем, сколько было времени, когда началось чтение файла,
					//если в данный момент с начала чтения прошло DS_Param милисекунд
					osTime = nextAbsolutPauseExpireTime - FileReadStartTime; //выделяем разницу между временем конца паузы и временем начала чтения файла
					FileReadStartTime = CoGetOSTime() - DS_Param; //назначаем новое время начала чтения файла
					nextAbsolutPauseExpireTime = FileReadStartTime + osTime; //рассчитываем время конца паузы заново, но с новым временем старта файла
					break;
				case DS_ANS_SET_M25_SWITCH_TIME:
					DS_Pause_interrupt_Flag |=0x100;
					M25ReversePauseValue = DS_Param;
					if (DS_Param == 0){
						DS_Pause_interrupt_Flag &=~0x100;
					}
					break;
				case DS_ANS_EXIT_SUBPROGRAM:

					//Message("M89\r\n\0",1);
					if (AfterLoop_File_Num == 0xFFFF){
						FastCommand(FC_EXIT_SUBPROGRAM_M89,1,&DS_Param);
					} else if (SD_Volume_Exists & 0x40){
						DS_Param = AfterLoop_File_Num;
						FastCommand(FC_START_FILE_NOW_M98,1,&DS_Param);
					}
					break;
				case  DS_GOTO_FILE: //M98 P<>, M98 Q<>P<>
					//*DS_Param_ - для хранения номера (имени) файла, в который переходить
					Next_File_Num = DS_Param;
					// НЕ используем FastCommand, поскольку нам нужно сохранить длительность относительной паузы
					//DS_comm_num & 0x10 =  0 (M89): 1 (M98);
					//SD_Volume_Exists |= 0x40; если M89
					//SD_Volume_Exists |= (((~DS_comm_num) & 0x10)<<2);
					//DS_comm_num & 0x20 =  0 (M89, M88): 1 (M98) ;
					//DS_comm_num & 0x10 =  0 (M89): 1 (M98, M88) ;
					if(DS_comm_num & 0x10){ //M98, M88 analog for FC_START_FILE_NOW_M98, FC_ENTER_NEXTFILE_M88
						if (DS_comm_num & 0x20){//M98, analog for FC_START_FILE_NOW_M98
							if (SD_Volume_Exists & 0x40) {
								Current_File_Num = Parent_Prev_File_Num;
							}
//							DS_buf_start=DS_buf_start_backup;
							SD_Volume_Exists &= ~0xC0; //0x80+0x40
						}
						if (AfterLoop_File_Num != 0xFFFF){Next_File_Num = AfterLoop_File_Num;} //отложенное событие по кнопке перехватывает управление при M98/88 [Q<>] P<>

					} else {//M89, analog FC_ENTER_SUBPROGRAM_M89
						if ((~SD_Volume_Exists) & 0x40) {
							DS_buf_start_backup=DS_buf_start;
							File_Read_Cycle_Count_Backup = File_Read_Cycle_Count;
							String_in_file_Index_Backup = String_in_file_Index;
							AfterLoop_File_Num_Backup = AfterLoop_File_Num;
						}
						SD_Volume_Exists |=0x40;
					}

					//if ((AfterLoop_File_Num != 0xFFFF) && (DS_comm_num & 0x10)){Next_File_Num = AfterLoop_File_Num;} //отложенное событие по кнопке перехватывает управление при M98/88 [Q<>] P<>
					if (DS_Channel_Select!=1){ResetPause();}
					File_Open_Preparation();
					break; // DS_GOTO_FILE:
				case  DS_ASSIGN_BUTTON: //			0x07	//запомнить, какой файл читать по кнопке
					//uint32_t *DS_Param_ - для хранения номера (имени) файла, в который переходить (для P)
					//uint8_t *DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
					//uint16_t *DS_WSpoint_counter_ - для хранения номера кнопки, которую кодируем (для Q)
					//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2=0b10 = есть P, читаем Q; 3=0b11 = есть Q, читаем P
							//*DS_comm_num_ используется как байт с флагами:
							//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P
							//бит 1 (маска 0x2): 0 = распознаём первый из параметров; 1 = распознаём второй из параметров (какой - по биту 1)
							//бит 2 (маска 0x4): 0 = распознаем Q; 1 = распознаём QR (событие по отпусканию кнопки), этот бит используется также как результат работы дешифр.

					if (DS_WSpoint_counter < FC_TOTAL_COMMAND_LIST+2){
						numFileForButton[ButtonNomberCarrier] = (uint16_t) DS_Param % 0xFFFF;
						FastCommandForButton[ButtonNomberCarrier] = (uint8_t) DS_WSpoint_counter;
						CommandArgForButton[ButtonNomberCarrier]=DS_RGB_counter;
					}

#ifndef FastButtonShortcuts
						buttonRelInterruptActive |= (0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным, inputsTask на него реагирует
						buttonRelWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
						buttonRelWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));
#endif

#ifndef FastButtonShortcuts
						buttonPushInterruptActive |= (0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным, inputsTask на него реагирует
						buttonPushWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
						buttonPushWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));
#endif
					//}
					//}

					break; //DS_ASSIGN_BUTTON:
/*				case DS_ACTIVATE_BUTTON: //M97 - активация/деактивация кнопок M97 Q<R><кнопка> P<0-выкл, 1-вкл.> (символ Q или P уже получен)
					//применение переменных - аналогично команде M96 (на 05/01/2018)
					//uint32_t *DS_Param_ - акивируем или деактивируем прерывание по кнопке
					//uint8_t *DS_RGB_counter_ - используется, но нафиг не нужен
					//uint16_t *DS_WSpoint_counter_ - для хранения номера кнопки, которую кодируем (для Q)
					//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2=0b10 = есть P, читаем Q; 3=0b11 = есть Q, читаем P
							//*DS_comm_num_ используется как байт с флагами:
							//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P
							//бит 1 (маска 0x2): 0 = распознаём первый из параметров; 1 = распознаём второй из параметров (какой - по биту 1)
							//бит 2 (маска 0x4): 0 = распознаем Q; 1 = распознаём QR (событие по отпусканию кнопки), этот бит используется также как результат работы дешифр.


#if defined (FastButtonShortcuts)
					//ButtonNomberCarrier = ButtonNomberCarrier + ((*DS_comm_num_ & 0x4)>>2)*MAX_NUM_BUTTONS
						if (DS_Param == 0){ // деактивация одной из кнопок
							if ((DS_comm_num & 0x4)) {		//бит 3 == 1, значит принимали команду на отпускание кнопки
								buttonRelInterruptActive &= ~(0x1 << (ButtonNomberCarrier - MAX_NUM_BUTTONS)); // соответствующее прерывание делаем НЕактивным
								//DS_WSpoint_counter += MAX_NUM_BUTTONS;
							} else {	 //бит 3 == 0, значит принимали команду на нажатие кнопки
								buttonPushInterruptActive &= ~(0x1 << (ButtonNomberCarrier)); // соответствующее прерывание делаем НЕактивным
							}
//						} else { //активация одной из кнопок, реагирование по DS_Param
//							if ((DS_comm_num & 0x4)) {		//бит 3 == 1, значит принимали команду на отпускание кнопки
//								buttonRelInterruptActive |= (0x1 << (ButtonNomberCarrier - MAX_NUM_BUTTONS)); // соответствующее прерывание делаем активным
//								//DS_WSpoint_counter += MAX_NUM_BUTTONS;
//							} else {	 //бит 3 == 0, значит принимали команду на нажатие кнопки
//								buttonPushInterruptActive |= (0x1 << (ButtonNomberCarrier)); // соответствующее прерывание делаем активным
//							}
						}
						//if (){FastCommandForButton[ButtonNomberCarrier] = DS_Param;}
						FastCommandForButton[ButtonNomberCarrier] = DS_Param; //DS_Param+FC_START_FILE_NOW_M98-1 но т.к. FC_START_FILE_NOW_M98=1 можно сэкономить на операндах
						CommandArgForButton[ButtonNomberCarrier]=DS_RGB_counter;
//						numFileForButton[DS_WSpoint_counter]=;


#endif
#ifndef FastButtonShortcuts
					if (DS_WSpoint_counter < MAX_NUM_BUTTONS){

					if (DS_Param == 0){ // деактивация одной из кнопок
						if ((DS_comm_num & 0x4)) {		//бит 3 == 1, значит принимали команду на отпускание кнопки
							//*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF; обрезание лишних байтов
							buttonRelInterruptActive &= ~(0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем НЕактивным
							buttonRelWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
							buttonRelWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));
						} else {	 //бит 3 == 0, значит принимали команду на нажатие кнопки
							buttonPushInterruptActive &= ~(0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем НЕактивным
							buttonPushWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
							buttonPushWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));
						}
					} else if (DS_Param == 1){ //активация одной из кнопок, реагирование сразу
						if ((DS_comm_num & 0x4)) {		//бит 3 == 1, значит принимали команду на отпускание кнопки
							//*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF; обрезание лишних байтов
							buttonRelInterruptActive |= (0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным
							buttonRelWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
							buttonRelWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));

						} else {	 //бит 3 == 0, значит принимали команду на нажатие кнопки
							buttonPushInterruptActive |= (0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным
							buttonPushWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
							buttonPushWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));
						}
					} else if (DS_Param == 2){ //активация одной из кнопок, реагирование по M47
						if ((DS_comm_num & 0x4)) {		//бит 3 == 1, значит принимали команду на отпускание кнопки
							//*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF; обрезание лишних байтов
							buttonRelInterruptActive &= ~(0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным
							buttonRelWaitStateEndFile |= (0x1 << (DS_WSpoint_counter));
							buttonRelWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));

						} else {	 //бит 3 == 0, значит принимали команду на нажатие кнопки
							buttonPushInterruptActive &= ~(0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным
							buttonPushWaitStateEndFile |= (0x1 << (DS_WSpoint_counter));
							buttonPushWaitStateEndMulti &= ~(0x1 << (DS_WSpoint_counter));
						}
					} else if (DS_Param == 3){ //активация одной из кнопок, реагирование по концу анимации
						if ((DS_comm_num & 0x4)) {		//бит 3 == 1, значит принимали команду на отпускание кнопки
							//*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF; обрезание лишних байтов
							buttonRelInterruptActive |= (0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным
							buttonRelWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
							buttonRelWaitStateEndMulti |= (0x1 << (DS_WSpoint_counter));

						} else {	 //бит 3 == 0, значит принимали команду на нажатие кнопки
							buttonPushInterruptActive |= (0x1 << (DS_WSpoint_counter)); // соответствующее прерывание делаем активным
							buttonPushWaitStateEndFile &= ~(0x1 << (DS_WSpoint_counter));
							buttonPushWaitStateEndMulti |= (0x1 << (DS_WSpoint_counter));
						}
					}}
#endif

					break; //DS_ACTIVATE_BUTTON: */
				case DS_ANS_BUTTON_MULTI_SET: //вкл/выкл режим реакции на 1 байт
					ButtonFlags &=0xFE;//выкл
					if (DS_Param){
						ButtonFlags |= 1;//вкл
						ButtonEncoder = 0;
					}
					break;
				case DS_ANS_BUTTON_EMULATE:


					if (((DS_comm_num & 0x0B)==9) && (CheckPixel_miscomparations != DS_Param)){//символ !, количество несовпадений пикселов больше чем написанное
						DS_comm_num = 0;
					} else if (((DS_comm_num & 0x0B)==0x0B) && (CheckPixel_miscomparations == DS_Param)){//символ =, количество несовпадений пикселов больше чем написанное
						DS_comm_num = 0;
					} else if (((DS_comm_num & 0x0B)==3) && (CheckPixel_miscomparations > DS_Param)){//символ >, количество несовпадений пикселов больше чем написанное
						DS_comm_num = 0;
					} else if (((DS_comm_num & 0x0B)==1) && (CheckPixel_miscomparations < DS_Param)){//символ <, количество несовпадений пикселов меньше чем написанное
						DS_comm_num = 0;
					}

					if (DS_comm_num & 0x10) {//проверяем фактическое состояние кнопки
						ExecuteIfButtonStatePrepressed(&ButtonNomberCarrier);
					} else if ((DS_comm_num & 0x1)==0) {
						DS_Param = numFileForButton[ButtonNomberCarrier];
						FastCommand((FastCommandForButton[ButtonNomberCarrier]&127),CommandArgForButton[ButtonNomberCarrier],&DS_Param);
					}


					break;
				case DS_ANS_SERVO_SET:

					DS_comm_num = (DS_comm_num & 4) ? FC_SERVO_POS_RAND_G0 : (FC_SERVO_POS_SET_G0+((DS_Flag_Register & 0xC)>>2));//((DS_Flag_Register & 8)>>3)+((DS_Flag_Register & 4)>>2));
					if (DS_Flag_Register&0x20){
//						FastCommandForButton[ButtonNomberCarrier]=FC_SERVO_POS_SET_G0+((DS_Flag_Register & 8)>>3)+((DS_Flag_Register & 4)>>2);
						FastCommandForButton[ButtonNomberCarrier]=DS_comm_num;
						numFileForButton[ButtonNomberCarrier]= DS_Param;
						CommandArgForButton[ButtonNomberCarrier]=DS_WSpoint_counter;
						//DS_Flag_Register &= ~0xC;
						//Message("srv set\r\n\0",1);
					} else {
						FastCommand(DS_comm_num,DS_WSpoint_counter,&DS_Param);

					}
					DS_comm_num =0;
					break; //DS_ANS_SERVO_SET
				case DS_ANS_SET_LED_TYPE: //M06
					// в DS_Param находится значение типа светодиодов - 0x122812 для WS или 0x236812 для SK, для среднего
					  if ((DS_Param==0x122812)){
						  //if (~LED_control_type & 1) stop_servos();
						  //LED_control_type &=128;
						  //LED_control_type |= 1;
						  //WS2812_IO_High = 0xFF;
						  //WS2812_IO_Low = 0x00;
						  WS2812_Timer_reinit(29,  8, 17);//ws2812 freq
						  goto SET_LED_Type_LedOnly_Parameters;
					  } else if (DS_Param==0x236812){
						  //if (~LED_control_type & 1) stop_servos();
						  //LED_control_type &=128;
						  //LED_control_type |= 1;
						  //WS2812_IO_High = 0xFF;
						  //WS2812_IO_Low = 0x00;
						  WS2812_Timer_reinit(29,  6, 12); //sk6812 freq
						  goto SET_LED_Type_LedOnly_Parameters;
					  } else if ((DS_Param==0x12813)){
					  						  //if (~LED_control_type & 1) stop_servos();
					  						  //LED_control_type &=128;
					  						  //LED_control_type |= 1;
					  						  //WS2812_IO_High = 0xFF;
					  						  //WS2812_IO_Low = 0x00;
						  WS2812_Timer_reinit(27,  4, 16); //ws2813 freq
						  //goto SET_LED_Type_LedOnly_Parameters;
					  //} else if (DS_Param==0x130012){
						  //WS2812_Timer_reinit(29,  7, 15); //что-то между частотами WS и SK, должны работать оба типа одновременно
SET_LED_Type_LedOnly_Parameters:
						  if (~LED_control_type & 1) stop_servos();
						  LED_control_type &=128;
						  LED_control_type |= 1;
						  WS2812_IO_High = 0xFF;
						  WS2812_IO_Low = 0x00;
					  } else if (DS_Param==0x2d2490){ //аналоговые сервы
						  if (~LED_control_type & 2) {
							  stop_servos();
							  LED_control_type &=128;
							  LED_control_type |= 2;
							  Servo_Period_default = SERVO_PERIOD_DEF; //50 Гц
							  Servo_MinPos_default = SERVO_POS_MIN_DEF; //0.5 ms
							  Servo_MaxPos_default = SERVO_POS_MAX_DEF; //2.5 ms
							  Servo_Resolution_default = 512; //разрешение
							  //UpdateServos();
						  }
					  } else if (DS_Param==0x12002d){ //ws2812+аналоговые сервы
						  WS2812_Timer_reinit(29,  8, 17);//ws2812 freq
						  goto SET_LED_Type_LED_and_Servo_Parameters;
					  } else if (DS_Param==0x12132d){ //ws2813+аналоговые сервы
						  WS2812_Timer_reinit(27,  4, 16);//ws2813 freq
						  goto SET_LED_Type_LED_and_Servo_Parameters;
					  } else if (DS_Param==0x23002d){ //sk6812+аналоговые сервы
						  WS2812_Timer_reinit(29,  6, 12); //sk6812 freq
SET_LED_Type_LED_and_Servo_Parameters:
						  if ((LED_control_type & 3) != 3) {
							  stop_servos();
							  LED_control_type &=128;
							  LED_control_type |= 3;
							  WS2812_IO_High = ~Servo_Action_Mask;
							  WS2812_IO_Low = 0x00;
							  Servo_Period_default = SERVO_PERIOD_DEF; //50 Гц
							  Servo_MinPos_default = SERVO_POS_MIN_DEF; //0.5 ms
							  Servo_MaxPos_default = SERVO_POS_MAX_DEF; //2.5 ms
							  Servo_Resolution_default = 512; //разрешение
							  //UpdateServos();
						  }
					  } else if ((DS_Param & 0xff000000)==0xfc000000){//прямой контроль количества тиков
						  DS_comm_num = (DS_Param>>16)&0xff; //всего тиков (частота 24 МГц или 42 нс на тик
						  DS_RGB_counter = (DS_Param>>8)&0xff; //тик первого перескока
						  a8 = DS_Param&0xff; //тик второго перескока
						  DS_comm_num = (DS_comm_num > 4) ? DS_comm_num : 4;
						  a8 = (a8 > 3) ? ((a8 < DS_comm_num) ? a8 : DS_comm_num - 1) : 3;
						  DS_RGB_counter = (DS_RGB_counter>2) ? ((DS_RGB_counter < a8) ? DS_RGB_counter : a8 - 1) : 2;
						  //a8 = (a8 < DS_comm_num) ? a8 : DS_comm_num - 1;
						  //DS_RGB_counter = (DS_RGB_counter < a8) ? DS_RGB_counter : a8 - 1;
						  WS2812_Timer_reinit(DS_comm_num-1,  DS_RGB_counter-1, a8-1);
					  }
					break;
				case DS_USART_SET_ID: //UD P ****
					Personal_ID = DS_Param;
					break;
				case DS_USART_SET_DEBUG: // turn debug info on/off at USART2
					DebugInfoOutFlag = DS_Param;
					break;

				case DS_USART_SET_BAUD://назначить бодрейт
					if(DS_RGB_counter&1){//назначаем бод для USART1
						USART1_Init(DS_Param);
					}
					if(DS_RGB_counter&2){//назначаем бод для dbgu
						DBGU_Init(DS_Param);
					}

					break;
				case DS_USART_SET_TIMEOUT://назначить максимальное время на приём команды
					//if (DS_RGB_counter == 0){
					//	File_Error_wait_timeout = DS_Param;
					//} else
					if(DS_RGB_counter&1){//назначаем для USART1
						USART1_rx_timeout = DS_Param;
						//print_0X4 (USART1_rx_timeout,0);
					}
					if(DS_RGB_counter&2){//назначаем для dbgu
						dbgu_rx_timeout = DS_Param;
					}

					break;
				case DS_USART_SET_INPUT://вкл/выкл чтение
					if(DS_RGB_counter&1){//для USART1
						if (DS_Param){
							USART1_State_of_recieved_Command |=0x20;//вкл
						} else {
							USART1_State_of_recieved_Command &= ~0x20;//выкл
						}
						Port_send_char('1',DEBUG_PORT_OUT);
						USB_main_COM_react();
					} //else
					if(DS_RGB_counter&2){//для dbgu
						if (DS_Param){
							dbgu_State_of_recieved_Command |=0x20;//вкл
						} else {
							dbgu_State_of_recieved_Command &= ~0x20;//выкл
						}
						Port_send_char('2',DEBUG_PORT_OUT);
						USB_main_COM_react();
					}

					break;
				case DS_ANS_U1T_DONE:
					//только что была завершена отправка на порт, работает сдвиговый регистр
					//надо поднять флаг паузы, если мы ждём окончания вывода
					//if (DS_Pause_interrupt_Flag & 16) {DS_Pause_interrupt_Flag |=8;}
				//case DS_ANS_U2T_DONE:
					//if (DS_Pause_interrupt_Flag & (16 << (descrResult - DS_ANS_U1T_DONE))) {DS_Pause_interrupt_Flag |=8;}
					if (DS_RGB_counter < 2){
						if (DS_Pause_interrupt_Flag & (16 << DS_RGB_counter)) {DS_Pause_interrupt_Flag |=8;}
					} else if (DS_RGB_counter == I2C_OUT_LABEL){
						start_I2C(0);
					//} else if (DS_RGB_counter == BUTTONS_TEXTS_OUT_LABEL){//выводв короткие тексты для кнопок
						//TextToSendArrayPointer содержитпервый свободный байт в массиве быстрых сообщений
					} else {
						USB_main_COM_react();
					}
					break;
				case DS_I2C_GET_FROM_WITH_ADDR:
					I2C_Bytes_To_Send = (I2C_Bytes_To_Send + 1)>>1; //оскольку мы считали количество hex-цифр, т.е. половин байт
					//DS_WSpoint_counter содержит инфу о том, из какого адреса читать байт
				case DS_I2C_GET_FROM://прочитать быйты из I2C
					I2C_Bytes_to_Recieve = (DS_Param < I2C_RX_SIZE) ? DS_Param : I2C_RX_SIZE;
					start_I2C(1);
					break;
				case DS_USART_SET_SHORT: //вкл/выкл режим реакции на 1 байт
					//DS_RGB_counter = 1, 2, 1+2=3 - uart; 4 - I2C
					DS_RGB_counter = ((DS_RGB_counter) << 4) & 0x30; //0x10 + 0x20+0x40

					if (DS_Param){
						//DS_RGB_counter &=0x30;
						ButtonFlags |=DS_RGB_counter;//вкл
					} else {
						ButtonFlags &= ~DS_RGB_counter;//выкл
						//if (DS_RGB_counter & 0x40){ ButtonsInit();}
					}
					break;
				case DS_USART_SET_WAIT: //вкл/выкл ожидание конца передачи
					//if(DS_RGB_counter==1){//для USART1
					//DS_RGB_counter = (8 << (DS_RGB_counter)) & 0x30;
					DS_RGB_counter = ((DS_RGB_counter) << 4) & 0x30;
					if (DS_Param){
							DS_Pause_interrupt_Flag |=DS_RGB_counter;//вкл
						} else {
							DS_Pause_interrupt_Flag &= ~DS_RGB_counter;//выкл
						}
					break;
				case DS_I2C_SET_ADDRESS:
					//ButtonFlags |=0x40;//вкл I2C
					if (DS_Param) {
						I2C1_init( (uint8_t) (DS_Param&0x7f));
					} else {
						I2C1_DeInit();
					}
					break;
				case DS_ANS_SERVO_DISABLE:
					//вход - DS_Param

					if (DS_comm_num == 4) { //включаем все светодиодные выводы в гибридном режиме  M4
						if ((LED_control_type & 3) == 3){ //18...25
							Servo_GPIO_Setting_on &= ((~DS_Param) & 0xFF00);
							Servo_GPIO_Setting_off &= ((~DS_Param) & 0xFF00);
							WS2812_IO_High |= (uint8_t)(DS_Param&0xFF); //включаем светодиодные выводы
							WS2812_IO_Low &= (uint8_t)((~DS_Param) &0xFF );
							Servo_Action_Mask &= (uint8_t)((~DS_Param) &0xFF );// соотносим маску включённых серв и маску включённых светодиодов
							Servo_Update_Flag |=2; //флаг изменения позиций серв
						}

					} else {
						 // *DS_comm_num_ += ((*DS_comm_num_ & 4)>>1) + ((*DS_comm_num_ & 4)<<1) (5(00101)->0F (01111)),
						 // + ((*DS_comm_num_ & 2)>>1) + (*DS_comm_num_ & 2) + ((*DS_comm_num_ & 2)<<2) (3(00011)->0E (01110))
						// 3 -> 3 + 0 + 0 + 1 + 2 + 8 + 1 = F
						// 5 -> 5 + 2 + 8 + 0 + 0 + 0 + 1 = 10
						// 10->10 + 0 + 0 + 0 + 0 + 0 + 1 = 11
						// 11->11 + 0 + 0 + 0 + 0 + 0 + 1 = 12
						// 4 -> 4 + 2 + 8 + 0 + 0 + 0 + 1 = F
						DS_comm_num = DS_comm_num + ((DS_comm_num & 4)>>1) + ((DS_comm_num & 4)<<1)
								+ ((DS_comm_num & 2)>>1) + (DS_comm_num & 2) + ((DS_comm_num & 2)<<2)
								+ FC_SERVO_ENABLE_M3 - 0x0E;
						if (DS_Flag_Register&0x20){
								//ParameterForButton[ButtonNomberCarrier]=0;
								FastCommandForButton[ButtonNomberCarrier]=DS_comm_num;
								CommandArgForButton[ButtonNomberCarrier]=DS_Param & 0xFF;
						} else {
							FastCommand(DS_comm_num,(DS_Param & 0xFF),&DS_Param);
						}
					}



					/*} else if (DS_comm_num == 3){ //all servo ON M3
						FastCommand(FC_SERVO_ENABLE_M3,(DS_Param & 0xFF),&DS_Param);
					/*	Servo_Action_Mask |= (uint8_t)(DS_Param&0xFF); //включаем только нужный сервомотор
						Servo_Update_Flag |=2; //флаг изменения позиций серв
						Servo_GPIO_Setting_on &= ((~DS_Param) &0xFF00);
						Servo_GPIO_Setting_off &= ((~DS_Param) &0xFF00);
						if ((LED_control_type & 3) == 3){
							WS2812_IO_High &= (uint8_t)((~DS_Param) &0xFF );
							WS2812_IO_Low &= (uint8_t)((~DS_Param) &0xFF );
						}/**/
					/*} else if (DS_comm_num == 5) {						//all OFF M5
						FastCommand(FC_SERVO_DISABLE_M5,(DS_Param & 0xFF),&DS_Param);
						/*if (DS_Param == 0xFFFF){
							stop_servos();
						} else {
							Servo_Action_Mask &= (uint8_t)((~DS_Param) &0xFF ); //выключаем не нужный сервомотор
							Servo_Update_Flag |=2; //флаг изменения позиций серв
						}/**/
					/*} else if (DS_comm_num == 0x10){ // просто включить ногу, 27-34 M10
						FastCommand(FC_PIN_ENABLE_M10,(DS_Param & 0xFF),&DS_Param);
						/*if (LED_control_type & 2) {
							//GPIO_SetBits(GPIOB,(uint16_t) DS_a8 <<  8); //поднимаем ноги при условии что сейчас серво-ориентированный режим
							Servo_GPIO_Setting_on |=   (DS_Param &0xFF00);
							Servo_GPIO_Setting_off &=   ((~DS_Param) & 0xFF00);
							WS2812_IO_High |= (uint8_t)(DS_Param&0xFF); //поднимаем светодиодные выводы туда же
							WS2812_IO_Low |= (uint8_t)(DS_Param&0xFF);
							Servo_Action_Mask &= (uint8_t)((~DS_Param) &0xFF );// соотносим маску включённых серв и маску включённых светодиодов
							Servo_Update_Flag |=2; //флаг изменения позиций серв
						}/**/
					/*} else if (DS_comm_num == 0x11){ // просто выключить ногу, 36-43 M11
						FastCommand(FC_PIN_DISABLE_M11,(DS_Param & 0xFF),&DS_Param);
						/*if (LED_control_type & 2) {
							Servo_GPIO_Setting_on &=   ((~DS_Param) & 0xFF00);
							Servo_GPIO_Setting_off |=   (DS_Param &0xFF00);
							WS2812_IO_High &= (uint8_t)((~DS_Param) &0xFF ); //поднимаем светодиодные выводы туда же
							WS2812_IO_Low &= (uint8_t)((~DS_Param) &0xFF );
							Servo_Action_Mask &= (uint8_t)((~DS_Param) &0xFF );// соотносим маску включённых серв и маску включённых светодиодов
							Servo_Update_Flag |=2; //флаг изменения позиций серв
						}/**/
					/*}/**/
					break;
				case DS_ANS_SERVO_SET_PARAM:
					//stop_servos(); //уже не нужно, так как сервы будут остановлены в момент апдейта параметров по вызову из паузы
					Servo_Update_Flag |=2; //флаг изменения позиций серв

					if (DS_comm_num == 0x43){
						Servo_MinPos = DS_Param;
					} else if (DS_comm_num == 0x44){
						Servo_MaxPos = DS_Param;
					} else if (DS_comm_num == 0x45){
						if (DS_Param > 0) Servo_Resolution = DS_Param;
					} else if (DS_comm_num == 0x50){
						if (DS_Param < SERVO_FREQ_MIN) {DS_Param = SERVO_FREQ_MIN;}
						if (DS_Param > SERVO_FREQ_MAX) {DS_Param = SERVO_FREQ_MAX;}
						Servo_Period = SERVO_TICK_FREQ / DS_Param;
					} else {// if (DS_comm_num == 0){ //восстановить умолчания
						Servo_Period = Servo_Period_default; //50Гц
						Servo_MinPos = Servo_MinPos_default; //0.5 ms
						Servo_MaxPos = Servo_MaxPos_default; //2.5 ms
						Servo_Resolution = Servo_Resolution_default; //разрешение
					}
					break;
				case DS_ANS_MULTI_SET_PARAM:

#if (0)
					switch(DS_comm_num ){
					case 0x26: //G26 P<> период сброса кнопки
						ButtonWaitSetting =  ModifyParameter(ButtonWaitSetting, &DS_Param);
						break;
					case 0x35://G35 количество кадров, которое надо проиграть, 0 - пока не остановят принудительно или не сменится файл
						//DS_comm_num = FC_FAST_ANIM_PLAY_G35; //=20. чтобы получить это значение, надо учесть, что дальше мы вычитаем 0x10 и 3
						DS_comm_num = 0x33; // 0x33 - 3 -0x10 = 0x20
						DS_Flag_Register &= ~0xC;
						//WS2812_Frame_Total_Count = DS_Param;
						break;
					case 0x30: //G30 частота смены кадров в мультикадре
						Servo_Update_Flag |=0x20;
						//DS_comm_num = FC_FAST_ANIM_FREQ_SET_G30 - FC_BRIGHTNESS_SET_G27 + 0x27 ; //30->2D
						DS_comm_num -= 3; //30->2D
					case 0x27: //SI P<>
					case 0x2A: //M91 P<>
						//DS_comm_num = DS_comm_num + FC_BRIGHTNESS_SET_G27 - 0x27; //FC_BRIGHTNESS_SET_G27 = 17, FC_CHOSEN_FILE_SET_M91 = 1A
						DS_comm_num -= 0x10;
						if (DS_Flag_Register&0x20){
							FastCommandForButton[ButtonNomberCarrier]=DS_comm_num+((DS_Flag_Register & 8)>>3)+((DS_Flag_Register & 4)>>2);
							numFileForButton[ButtonNomberCarrier]= DS_Param;
							CommandArgForButton[ButtonNomberCarrier]=1; //для быстрого изменения скорости мультикадра
							DS_Flag_Register &= ~0xC;
						} else {
							FastCommand(DS_comm_num,1,&DS_Param);
						}
						break;
					case 0x29://G29 начало первого проигрываемого кадра анимации мультикадра в памяти
						Servo_Update_Flag |=0x20;
						if (DS_Flag_Register & 8){
							if (DS_Flag_Register & 4) {
								DS_Param = ((WS2812_Frame_Start_Pointer/WS2812_Frame_Byte_Length) - DS_Param)%WS2812_Frame_Count;
							} else {
								DS_Param = ((WS2812_Frame_Start_Pointer/WS2812_Frame_Byte_Length) + DS_Param)%WS2812_Frame_Count;
							}
							DS_Flag_Register &= ~0xC;
						} else if (DS_Param >= WS2812_Frame_Count){
							DS_Param = WS2812_Frame_Count - 1;
						}
						WS2812_Frame_Start_Pointer = DS_Param * WS2812_Frame_Byte_Length;
						break;
					case 0x31://G31 количество пикселей в кадре
						Servo_Update_Flag |=0x20;
						//длина фрейма не может быть больше длины массива
						if (DS_Param > WS2812_IO_FRAMEDATA_PIXELS){DS_Param = WS2812_IO_FRAMEDATA_PIXELS;}
						WS2812_Frame_Length = DS_Param;
						//стартовый фрейм не может быть больше чем максимальное количество, даже если есть место в массиве
						WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer / WS2812_Frame_Byte_Length; //определяем номер стартового фрейма, чтобы не вылететь за пределы
						WS2812_Frame_Byte_Length = (uint32_t) WS2812_Frame_Length * 24;
						//и количество кадров должно быть таким, чтобы они помещались в массив
						if ((WS2812_Frame_Count* WS2812_Frame_Length) > WS2812_IO_FRAMEDATA_PIXELS){WS2812_Frame_Count = WS2812_IO_FRAMEDATA_PIXELS/WS2812_Frame_Length;}
						if (WS2812_Frame_Count < 1){WS2812_Frame_Count = 1;}
						if (WS2812_Frame_Start_Pointer  >= WS2812_Frame_Count) {
							WS2812_Frame_Start_Pointer  = WS2812_Frame_Count - 1;
						}
						//восстанавливаем стартовый кадр
						WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer * WS2812_Frame_Byte_Length;
						//ограничиваем максимальную частоту
						if (WS2812_Frame_Period < (SERVO_TICK_FREQ * WS2812_Frame_Length / (SERVO_FREQ_MAX<<4) )){
							WS2812_Frame_Period = SERVO_TICK_FREQ * WS2812_Frame_Length / (SERVO_FREQ_MAX<<4) ;
						}
						break;
					case 0x32://G32 количество кадров в анимации
						Servo_Update_Flag |=0x20;
						if (DS_Param < 1){DS_Param = 1;}
						if ((DS_Param * WS2812_Frame_Length) <= WS2812_IO_FRAMEDATA_PIXELS) {
							WS2812_Frame_Count = DS_Param;
						} else {
							WS2812_Frame_Count = WS2812_IO_FRAMEDATA_PIXELS/WS2812_Frame_Length;
						}
						//стартовый фрейм не может быть больше чем максимальное количество, даже если есть место в массиве
						WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer / WS2812_Frame_Byte_Length; //определяем номер стартового фрейма, чтобы не вылететь за пределы
						if (WS2812_Frame_Start_Pointer  >= WS2812_Frame_Count) {
							WS2812_Frame_Start_Pointer  = WS2812_Frame_Count - 1;
						}
						//восстанавливаем стартовый кадр
						WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer * WS2812_Frame_Byte_Length;
						break;
					default:
						break;
					}
#else

					if (DS_comm_num == 0x26) {//назначить паузу для сброса кнопок
						DS_Math_Flags = (DS_Flag_Register & 0xC)>>2;
						ButtonWaitSetting =  ModifyParameter(ButtonWaitSetting, &DS_Param, BUTTON_WAIT_MAX);
					} else if (DS_comm_num == 0x27){//G27 - время ожидания при нажатии в режме нескольких кнопок
						DS_Math_Flags = (DS_Flag_Register & 0xC)>>2;
						ButtonCompleteSetting = ModifyParameter(ButtonCompleteSetting, &DS_Param, BUTTON_COMPLETE_MAX);
						if (ButtonCompleteSetting < BUTTON_COMPLETE_MIN){ButtonCompleteSetting = BUTTON_COMPLETE_MIN;}
						//else if (ButtonCompleteSetting > BUTTON_COMPLETE_MAX){ButtonCompleteSetting = BUTTON_COMPLETE_MAX;}
					//} else if(DS_comm_num == 0x28 || DS_comm_num == 0x2B || DS_comm_num == 0x16){
						//DS_comm_num = DS_comm_num + FC_BRIGHTNESS_SET_G27 - 0x28; //FC_BRIGHTNESS_SET_G27 = 19, FC_CHOSEN_FILE_SET_M91 = 1С, FC_REPEAT_COUNTS_M47 = 07
					} else if(DS_comm_num == 0x28){//SI P<>=0x28
						DS_comm_num = FC_BRIGHTNESS_SET_G27;
						goto Link_to_Modify_parameters_DS_comm_num_IS_ACTION;
					} else if(DS_comm_num == 0x91){//M91 P<>=0x2B
						DS_comm_num = FC_CHOSEN_FILE_SET_M91;
						goto Link_to_Modify_parameters_DS_comm_num_IS_ACTION;
					} else if(DS_comm_num == 0x45){//M45 P<>=0x2B
						DS_comm_num = FC_MISCOMP_SET_M45;
						goto Link_to_Modify_parameters_DS_comm_num_IS_ACTION;
					} else if(DS_comm_num == 0x46){//M46 P<> = 16
						//SI P<>=0x28, M91 P<>=0x2B, M46 P<> = 16
						DS_comm_num = FC_REPEAT_COUNTS_M47;
						DS_Flag_Register &= ~0xC;  //поскольку у этой команды только 1 вариация
Link_to_Modify_parameters_DS_comm_num_IS_ACTION:
						DS_comm_num += (DS_Flag_Register & 0xC)>>2;
						if (DS_Flag_Register&0x20){
							FastCommandForButton[ButtonNomberCarrier]=DS_comm_num;//+((DS_Flag_Register & 8)>>3)+((DS_Flag_Register & 4)>>2); //командаможетиспольняться в режиме "записать значение" и "изменить значение на X", что описывается битами 8 и 4 в Flag Register
							//if (DS_comm_num == FC_REPEAT_COUNTS_M47) {FastCommandForButton[ButtonNomberCarrier]=DS_comm_num;}
							numFileForButton[ButtonNomberCarrier]= DS_Param;
							CommandArgForButton[ButtonNomberCarrier]=0; //для быстрого изменения скорости мультикадра
							//DS_Flag_Register &= ~0xC;
						} else {
							FastCommand(DS_comm_num,1,&DS_Param);
						}
					} else {

						Servo_Update_Flag |=0x20; //параметры поменялись (применятся в Output_to_WS())
						//stop_servos();
						if (DS_comm_num == 0x29){//G29 начало первого проигрываемого кадра анимации мультикадра в памяти
							if (DS_Flag_Register & 8){
								if (DS_Flag_Register & 4) {
									DS_Param = ((WS2812_Frame_Start_Pointer/WS2812_Frame_Byte_Length) - DS_Param)%WS2812_Frame_Count;
								} else {
									DS_Param = ((WS2812_Frame_Start_Pointer/WS2812_Frame_Byte_Length) + DS_Param)%WS2812_Frame_Count;
								}
								//DS_Flag_Register &= ~0xC;
							} else if (DS_Param >= WS2812_Frame_Count){
								DS_Param = WS2812_Frame_Count - 1;
							}
							WS2812_Frame_Start_Pointer = DS_Param * WS2812_Frame_Byte_Length;
						} else if (DS_comm_num == 0x30){//G30 частота смены кадров в мультикадре

							DS_comm_num = FC_FAST_ANIM_FREQ_SET_G30;
							goto Link_to_Modify_parameters_DS_comm_num_IS_ACTION;

						} else if (DS_comm_num ==0x31){//G31 количество пикселей в кадре
							//длина фрейма не может быть больше длины массива
							if (DS_Param > WS2812_IO_FRAMEDATA_PIXELS){DS_Param = WS2812_IO_FRAMEDATA_PIXELS;}
							WS2812_Frame_Length = DS_Param;
							//стартовый фрейм не может быть больше чем максимальное количество, даже если есть место в массиве
							WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer / WS2812_Frame_Byte_Length; //определяем номер стартового фрейма, чтобы не вылететь за пределы
							WS2812_Frame_Byte_Length = (uint32_t) WS2812_Frame_Length * 24;
							//и количество кадров должно быть таким, чтобы они помещались в массив
							if ((WS2812_Frame_Count* WS2812_Frame_Length) > WS2812_IO_FRAMEDATA_PIXELS){WS2812_Frame_Count = WS2812_IO_FRAMEDATA_PIXELS/WS2812_Frame_Length;}
							if (WS2812_Frame_Count < 1){WS2812_Frame_Count = 1;}
							if (WS2812_Frame_Start_Pointer  >= WS2812_Frame_Count) {
								WS2812_Frame_Start_Pointer  = WS2812_Frame_Count - 1;
							}
							//восстанавливаем стартовый кадр
							WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer * WS2812_Frame_Byte_Length;
							//ограничиваем максимальную частоту
							if (WS2812_Frame_Period < (SERVO_TICK_FREQ * WS2812_Frame_Length / (SERVO_FREQ_MAX<<4) )){
								WS2812_Frame_Period = SERVO_TICK_FREQ * WS2812_Frame_Length / (SERVO_FREQ_MAX<<4) ;
							}
						} else if (DS_comm_num ==0x32){//G32 количество кадров в анимации
							if (DS_Param < 1){DS_Param = 1;}
							if ((DS_Param * WS2812_Frame_Length) <= WS2812_IO_FRAMEDATA_PIXELS) {
								WS2812_Frame_Count = DS_Param;
							} else {
								WS2812_Frame_Count = WS2812_IO_FRAMEDATA_PIXELS/WS2812_Frame_Length;
							}
							//стартовый фрейм не может быть больше чем максимальное количество, даже если есть место в массиве
							WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer / WS2812_Frame_Byte_Length; //определяем номер стартового фрейма, чтобы не вылететь за пределы
							if (WS2812_Frame_Start_Pointer  >= WS2812_Frame_Count) {
								WS2812_Frame_Start_Pointer  = WS2812_Frame_Count - 1;
							}
							//восстанавливаем стартовый кадр
							WS2812_Frame_Start_Pointer = WS2812_Frame_Start_Pointer * WS2812_Frame_Byte_Length;
						} else if (DS_comm_num == 0x35){//G35 количество кадров, которое надо проиграть, 0 - пока не остановят принудительно или не сменится файл
							DS_comm_num = FC_FAST_ANIM_PLAY_G35;
							DS_Flag_Register &= ~0xC; //поскольку у этой команды только 1 вариация, сбрасываем биты Flag Register которые отвечают за вариации
							goto Link_to_Modify_parameters_DS_comm_num_IS_ACTION;
							//WS2812_Frame_Total_Count = DS_Param;
							//фактическим сигналом выхода из цикла анимации будет  WS2812_Frame_Total_Count = 1
							//по этому мы сдвигаем значение на 1 вверх
						}
					}

#endif

					break;
				case DS_ANS_MULTI_PLAY:
					//включается только в режиме вывода светодиодов

						if (DS_Param) {DS_Param = 1;}//1 - вкл, 0 - выкл
					if (DS_Flag_Register&0x20){
						FastCommandForButton[ButtonNomberCarrier]=FC_FAST_ANIM_OFF_G36-DS_Param;
						//ParameterForButton[ButtonNomberCarrier]= DS_Param;
						CommandArgForButton[ButtonNomberCarrier]=0; //индикатор того, что это нажатие, и надо сразу запускать анимацию
					} else {
						FastCommand(FC_FAST_ANIM_OFF_G36-DS_Param,1,0);

					}


					break;
				case DS_ANS_SERVO_WATCH:

					if (DS_Flag_Register&0x20){
						//if (DS_Param) {DS_Param=1;}
						DS_Param = (DS_Param&1) | ((DS_Param&2)>>1) | ((DS_Param&3)>>2) | ((DS_Param&8)>> 3);
						//ParameterForButton[ButtonNomberCarrier]=0;
						FastCommandForButton[ButtonNomberCarrier]=FC_SERVO_FRAMEBASED_G1 - DS_Param;
						//CommandArgForButton[ButtonNomberCarrier]=1;
					} else if (DS_Param && LED_control_type & 2) { //мы управляем сервами
						//включение постоянного отслеживания положения серв
							LED_control_type |= 8;
							Servo_Update_Flag |= 2; //флаг смены значений поднят
					} else if (LED_control_type & 8){//режим отслеживания серв включён
							LED_control_type &= ~ 0x18;//выключаем
					}

					break;
				case DS_ANS_FILE_SUSPEND:
#if defined (FastButtonShortcuts)

					FastCommand(FC_RESUME_FILE_M24+DS_comm_num,1,&DS_Param);
#endif

					break;

#if defined (FS_ENABLED)

				case DS_ANS_FILEMODE_ON: //Переход а режим работы с файлами
					SD_Volume_Exists &= ~0x0F; //сбрасываем порт вывода информации ФС (b1100), разрешение на запись(b0010) и проверку CRC (b0001)
					if (DS_Param) {
						LED_control_type |= 128; //включаем с сохранением информации о текущих режимах
						result = f_opendir (&dir, FileworksDirpath );
						//result = f_opendir (&dir, FileworksDirpath );
						SD_Volume_Exists |= (DS_Channel_Select==4)? 0 : (((DS_Channel_Select) - 1) & 3)<<2;
						//DS_Channel_Select = 0-любой канал готов, мы сюда не попадаем
						//DS_Channel_Select = 1-строго файл, значение b0000, a8 = b00 - 1 = 0xFF - USB
						//DS_Channel_Select = 2-строго USART1, значение b0100, a8 = b01 - 1 = 0, USART1
						//DS_Channel_Select = 3-строго USART2 (DBGU), значение b1000, a8 = b10 - 1 = 1, USART2
						//DS_Channel_Select = 4-строго USB, значение b1100, a8 = b00 - 1 = 0xFF, USB
						//DS_Channel_Select = 0x10 - все кроме файла, мы сюда не попадаем
						//a8=(SD_Volume_Exists>>2)&3 - 1;
						Calculate_Fileworks_Output_Port();
						Port_send_char(DS_Channel_Select+'0', DEBUG_PORT_OUT);
						Port_send_char(a8, DEBUG_PORT_OUT);
						Port_send_char ('\r',DEBUG_PORT_OUT);
						USB_main_COM_react();

						Message ("Open DIR \0",a8);
						Message (FileworksDirpath,a8);
						//Port_send_char ('\r',a8);
						Finish_Fileworks_Message(a8);
						FileWorksPTR = 0;
						path2[0] = '\0';
					} else if (LED_control_type & 128) {
						LED_control_type &= 127; //выключаем с сохранением информации о текущих режимах
						result = f_opendir (&dir, Dirpath ); //мы могли работать с корнем
						ResetPause();
						Next_File_Num = Current_File_Num;
						Current_File_Num = Prev_File_Num;
						File_Open_Preparation(); //мы могли поменять тот самый файл, который читаем сейчас.
					}
					break;
				/*case DS_ANS_FILEMODE_READDIR:
					a8=(SD_Volume_Exists>>2) - 1;
					if (DS_Param <100){
					if (DS_Param == 0){ //читаем первый объект в папке
						result = f_opendir (&dir, FileworksDirpath );
					}

					result = f_readdir (&dir,  &filinfo );
					if (!filinfo.fname[0]) {
						Message ("EOD\0",a8);
						result = f_opendir (&dir, FileworksDirpath );
					} else {
						if (filinfo.fattrib & AM_ARC) {
							Message ("F:\0",a8);
						} else if (filinfo.fattrib & AM_DIR) {
							Message ("D:\0",a8);
						}

						if (result != FR_OK) {
							Fail_Message(2,1,result);
						} else {//открываем
							Message (filinfo.fname,a8);
						}
					}
					} else { //DS_Param  = 100
						Port_send_char ('\r',a8);
					}
					Finish_Fileworks_Message(a8);

					break;/**/
				case DS_ANS_FILEMODE_SETDIR: //открыть папку для работы с файлами F2P<>
						if (fname[0]) {// не пустая строка
							DIRPathByName(fname, FileworksDirpath, &dir, &filinfo);
						} else {
							Reset_FileworkDirPath ();
						}
						result = f_opendir (&dir, FileworksDirpath );
						Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
						Message ("Open DIR \0",a8);
						Message (FileworksDirpath,a8);
						//Port_send_char ('\r',a8);
						Finish_Fileworks_Message(a8);
					break;
				case DS_ANS_NORMALMODE_SETDIR: //назанчить папку для вызова анимаций M23P<>
					//Dirpath[]
					if (fname[0]) {// не пустая строка
						DIRPathByName(fname, Dirpath, &dir, &filinfo);
					} else {
						for (a8 = 0; a8 < DIR_PATH_MAX_LENGTH; ++a8) {//чистим путь
							Dirpath[a8] = '\0';
						}
						memcpy(Dirpath, "0:/hcd", 6);//назначаем корень
					}
					if ((~LED_control_type) & 128) {
						result = f_opendir (&dir, Dirpath ); //мы могли работать с корнем
						ResetPause();
						Next_File_Num = Current_File_Num;
						Current_File_Num = Prev_File_Num;
						//SD_Volume_Exists &= ~0xC0;
						File_Open_Preparation(); //мы могли поменять тот самый файл, который читаем сейчас.
					}
					break;
				case DS_ANS_FILEMODE_NEWDIR: //создать папку F3P<>
					Reset_FileworkDirPath ();
						FileworksDirpath[6]= '/';//strcat(FileworksDirpath, "/");
						strcat(FileworksDirpath, fname);
						result = f_mkdir(FileworksDirpath);
						if (result != FR_OK && result!= FR_EXIST){ //какая-то фигня, возвращаемся в корень
							Reset_FileworkDirPath ();
						}
						result = f_opendir (&dir, FileworksDirpath );
						Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
						Message ("Open DIR \0",a8);
						Message (FileworksDirpath,a8);
						//Port_send_char ('\r',a8);
						Finish_Fileworks_Message(a8);
					break;
				case DS_ANS_FILEMODE_OPENREAD: //F4P<>
					Path_FullName_in_Fileworkdirpath();
					memcpy(path2,path,FILE_PATH_MAX_LENGTH);
						result = f_close(&file);
						result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
						FileWorksPTR = 0;
						if (result == FR_OK){
							Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
							Message ("Open \0",a8);
							Message (path,a8);
							Message (" to read\0",a8);
							Finish_Fileworks_Message(a8);
							//FileWorksPTR = 0;
						} else {
							Fail_Message(1,1,result);
						}
					break;
				case DS_ANS_FILEMODE_DELETE: //F5P<>
					Path_FullName_in_Fileworkdirpath();

						result = f_unlink(path);
						if (result == FR_OK){
							Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
							Message (path,a8);
							Message (" deleted\0",a8);
							Finish_Fileworks_Message(a8);
						} else {
							Fail_Message(1,5,result);
						}
					break;
				case DS_ANS_FILEMODE_ACTUALIZE: //сохранить tmp файл под новым именем F6P<>
					Path_FullName_in_Fileworkdirpath();

						for (a8 = 0; FileworksDirpath[a8]; ++a8) { }//считаем символы
						FileworksDirpath[a8] = '/';
						FileworksDirpath[a8+1] = 'T';
						//имя временного файла 0:/hcd/t или 0:/hcd/<subdir>/t
						result = f_close(&file);

						result = f_rename (FileworksDirpath, path);
						FileWorksPTR = 0;
						FileworksDirpath[a8] = '\0';// "/t" сделал своё дело, "/t" может удалиться
						FileworksDirpath[a8+1] = '\0';
						if (result == FR_OK){
							Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
							Message ("File \0",a8);
							Message (path,a8);
							Message (" saved\0",a8);
							Finish_Fileworks_Message(a8);
						} else {
							Fail_Message(1,3,result);
						}

						//for (a8 = 0; FileworksDirpath[a8]; ++a8) { }//считаем символы
						//FileworksDirpath[a8-1] = '\0'; // "/t" сделал своё дело, "/t" может удалиться
						//FileworksDirpath[a8-2] = '\0';
					break;
				case DS_ANS_FILEMODE_RENAME:
					Path_FullName_in_Fileworkdirpath();
					if (path2[0]){
					result = f_close(&file);
					result = f_rename (path2, path);
					if (result == FR_OK){
						Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
						Message ("File \0",a8);
						Message (path2,a8);
						Message (" renamed to \0",a8);
						Message (path,a8);
						//Port_send_char ('\r',a8);
						Finish_Fileworks_Message(a8);
					} else {
						Fail_Message(1,2,result);
					}
					}
					path2[0] = '\0';
					break;
				case DS_ANS_FILEMODE_NEWFILE: //F9 создаём временный файл для записи в него всякого
						for (a8 = 0; FileworksDirpath[a8]; ++a8) { }//считаем символы
						FileworksDirpath[a8] = '/';
						FileworksDirpath[a8+1] = 'T';
						//имя временного файла 0:/hcd/t или 0:/hcd/<subdir>/t
						result = f_close(&file);
						result = f_unlink(FileworksDirpath);//удалить старый, если он есть
						result = f_open(&file, FileworksDirpath, FA_CREATE_NEW | FA_CREATE_ALWAYS | FA_WRITE |FA_READ);//создать
						result = f_sync(&file);
						FileWorksPTR = 0;
						FileworksDirpath[a8] = '\0';// "/t" сделал своё дело, "/t" может удалиться
						FileworksDirpath[a8+1] = '\0';
						if (result == FR_OK){
							Calculate_Fileworks_Output_Port();////a8=((SD_Volume_Exists>>2) - 1);
							Message ("Temp file created\0",(a8));//(SD_Volume_Exists>>2) - 1));
							Finish_Fileworks_Message(a8);//(SD_Volume_Exists>>2) - 1);
						} else {
							Fail_Message(1,3,result);
						}
						//for (a8 = 0; FileworksDirpath[a8]; ++a8) { }//считаем символы
						//FileworksDirpath[a8-1] = '\0'; // "/t" сделал своё дело, "/t" может удалиться
						//FileworksDirpath[a8-2] = '\0';
						path2[0] = '\0';
					break;
				case DS_ANS_FILEMODE_READFILE: //F10 [Q<pionter>]P/N<number of bytes>
					//DS_Param - указатель на старт чтения
					//DS_WSpoint_counter - сколько читать
					Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
					if ( DS_Param < file.fsize){//читаем
						FileWorksPTR = DS_Param;
						if (file.fptr != DS_Param){
							f_lseek(&file,FileWorksPTR);
						}
						result = f_read(&file, (BYTE *)text_buff, DS_WSpoint_counter, (UINT*)&read_size);
						if (DS_WSpoint_counter>read_size) {DS_WSpoint_counter =  (uint16_t) read_size;} //отправляем только прочитанное!
						if (result == FR_OK){
							//ниже в DS_Param держим текущий кусок данных


							for (a16 = 0; a16 < 8; a16++) {
								fname[a16] = DS_inttochar((uint8_t)((DS_Param >> (28-(a16<<2)))&0x0F));

							}
							fname[8] = '\0';
							Message (fname,a8);
							DS_Param = crc32_byte(CRC32_INIT,fname, 8);
							DS_Param = crc32_byte(DS_Param, text_buff, DS_WSpoint_counter);
							// CRC parameters:
							 // tested on http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
							 // NOT Input reflected, NOT Result reflected
							 // Polynomial:	0x4C11DB7
							 // Initial Value:	0x0
							 // Final Xor Value:	0x0

							text_buff[DS_WSpoint_counter] = '\0';
							Message (text_buff,a8);
							for (a16 = 0; a16 < 4; a16++) {
								fname[a16] =((uint8_t)((DS_Param >> (24-(a16<<3)))&0xFF));
							}
							fname[4] = '\0';
							Message (fname,a8);
						} else {
							Fail_Message(1,1,result);
						}
						FileWorksPTR = file.fptr;
					} else {//мы вне файла
						Message ("END OF FILE\0",a8);
					}
					Finish_Fileworks_Message(a8);
					break;
				case DS_ANS_FILEMODE_WRITEFILE: //F11 [Q<pionter>]P/H<data>
					//DS_Param - указатель на старт записи
					//DS_comm_num - записываемый символ
					//DS_WSpoint_counter - 4 - сколько писать (последние 4 это CRC)
					Calculate_Fileworks_Output_Port();//a8=((SD_Volume_Exists>>2) - 1);
					if (file.flag & FA_WRITE){

						FileWorksPTR = DS_Param;
						if (file.fptr != DS_Param){
							f_lseek(&file,FileWorksPTR);
						}

						SD_Volume_Exists |= 2;

						if ((SD_Volume_Exists & 1) && CRC_sum){//проверяем контрольную сумму и нам не повезло
							SD_Volume_Exists &= ~2;
							Message ("Corrupted data\0" ,a8);
						}


						if (SD_Volume_Exists & 2){
							result = f_write(&file, (BYTE *)text_buff, (DS_WSpoint_counter - 4), (UINT*)&read_size);
							if (result != FR_OK){
								Fail_Message(1,6,result);
							} else {
								Message ("Data saved\0" ,a8);
							}
						}
						FileWorksPTR = file.fptr;
					} else {//мы вне файла
						Message ("FILE WRITE DENIED\0",a8);
					}
					Finish_Fileworks_Message(a8);
					break;
				case DS_ANS_FILEMODE_WRITE_CRC://F12
					SD_Volume_Exists &= ~1;
					if (DS_Param) SD_Volume_Exists |= 1;
					break;
				case DS_ANS_FILEMODE_OUT_TARGET://F13
					SD_Volume_Exists &= ~0xC;//4+8
					SD_Volume_Exists |= ((DS_Param & 3)<<2); //0...3 * 4 = 0, 4 , 8, 12
					break;/**/
#endif



				default:
					DS_status=DS_Type_command_new; //мы обновляем всё
					if (DS_Channel_Select==3){ //мы получили грязь, канал заспамлен, сбился счётчик, не дождались конца приёма
						dbgu_State_of_recieved_Command=0x21; //мы готовы принять новую команду
					} else if (DS_Channel_Select==2){
						USART1_State_of_recieved_Command=0x21; //мы готовы принять новую команду
					} else {
						DS_status=DS_Type_command_comment; //пропускаем остаток строки
					}
			}

			if (DS_status==DS_Type_command_new){


				//если он дочитал до паузы, то теперь он проверяет на конец паузы, а если не дочитал, то ему всё равно
				//osTime = CoGetOSTime();
				//Приоритеты пауз:
				//пауза передачи данных, сбрасывается в прерывании
				//абсолютная пауза - сбрасывает как себя так и относительные паузы
				//относительная пауза - сбрасывает только себя

				//if ((DS_Pause_interrupt_Flag & 8) || (DS_Pause_interrupt_Flag & 64) || (DS_Pause_interrupt_Flag & 128)){//ожидание передачи данных или вывода анимации
				if (0 != (DS_Pause_interrupt_Flag & (0x8 | 0x40 | 0x80 | 0x200))){//ожидание передачи данных или вывода анимации

					DS_Channel_Select = 0x10; //готовы принимать команды только из портов
				} else if ((CoGetOSTime() < nextAbsolutPauseExpireTime) && (DS_Pause_interrupt_Flag & 2)){ //абсолютная пауза в процессе
					//ждём, когда пора будет выполнить абсолютную паузу
					DS_Channel_Select = 0x10; //готовы принимать команды только из портов

				} else if ((CoGetOSTime() >= nextAbsolutPauseExpireTime) && (DS_Pause_interrupt_Flag & 2)){ //абсолютная пауза в только закончилась
					//время выполнения абсолютной паузы наступило
					//отменяем флаги абсолютной и относительной пауз
					//готовы принимать команды откуда угодно
					//осуществляем вывод на светодиоды
					if (~(DS_Pause_interrupt_Flag & 4)){Output_to_WS();} //вывод только если флаг "без вывода" опущен

					DS_Pause_interrupt_Flag &= ~7;//сброс флага паузы (1), абсолютной паузы (2), и флага ввода (4)
					DS_Channel_Select = (SD_Volume_Exists &0x10); //готовы принимать команды откуда угодно

				} else if ((CoGetOSTime() < nextPauseStartTime) && (DS_Pause_interrupt_Flag & 1)){ //относительная пауза в процессе

					DS_Channel_Select = 0x10; //готовы принимать команды только из портов
				} else { //паузы закончились или их нет
					DS_Pause_interrupt_Flag &= ~1;//сброс флага паузы
					DS_Channel_Select = (SD_Volume_Exists &0x10); //готовы принимать команды откуда угодно
					if (SD_Volume_Exists & 0x10){//если карта отмонтировалась
						SD_Recheck_Countdown ++;
						if (SD_Recheck_Countdown>SD_Recheck_Countdown_REVOLVE){
							SD_Recheck_Countdown = 0;
							descrActionFlag = descrActionFlag | DS_ACTION_QUIT_DESCRIPT;
						}
					}
				}

			}
			ButtonsCheckSchedile();




		} //while (  descrActionFlag == 0 ); //конец цикла "расшифровка прочитанного"


//конец расшифровки прочитанного
//----------------------------------------------------------------------------------
/**/
	} while (1);



}


//------------------------------------------------------------------------------------------
//инициализация вывода данных на светодиоды
//и ожидание конца вывода

void Output_to_WS (void)
{
	if ((LED_control_type&127) == 1){ //работаем со светодиодами
		//r2 = CoClearFlag (fileReadStartFlag); //флаг текущей задачи - тормозим

		//r2 = CoSetFlag(outputStartFlag); //размораживаем задачу вывода на экран и отсчёта паузы

		//CoWaitForSingleFlag(fileReadStartFlag, 0);
		Do_WS2812_Output_Sequence();

	} else if (LED_control_type & 2){ //серводвигатели или гибрид: 2,3, 10,11
		if (LED_control_type & 1){ //3, 11
			Servo_Update_Flag |= 0x10; //поднять флаг необходимости вывода на светодиоды
		}

		if ((Servo_Update_Flag & 2) && (~LED_control_type & 0x10)) {//изменились позиции или подконтрольные ноги но мы не изменяем их в реальном времени
			//вообще нелогично включать и отключать ноги в процессе когда ШИМ уже работает, но мало ли.
			//LED_control_type |= 16;
			Servo_Update_Flag |= 4;//обновить сервы как только мы будем между циклами
			//stop_servos(); //останавливаем сервы чтобы случайно не поменять параметры на выводе и не получить грязь.
			if (LED_control_type & 8) {//поднимаем фактический флаг на обновление
				LED_control_type |= 0x10;
			}
			//UpdateServos();

		}

		if ( (Servo_Update_Flag & 3)== 2 ) {
			start_servos();
		}


	} else if ((LED_control_type&127) == 4){


		if (Servo_Update_Flag & 0x20) {//изменились параметры мультикадра
			stop_servos();
			LED_control_type &= 128;
			LED_control_type |= 4; //так как в stop_servos оно сбрасывается
			WS2812_Frame_Start_Pointer_Actual = WS2812_Frame_Start_Pointer;
			WS2812_Frame_Length_Actual = WS2812_Frame_Length; //количество пикселей в кадре
			WS2812_Frame_Count_Actual = WS2812_Frame_Count;
			WS2812_Frame_Byte_Length_Actual = WS2812_Frame_Byte_Length;
			WS2812_Frame_Period_Actual = WS2812_Frame_Period;
			DS_maxCurrPoints = WS2812_Frame_Length_Actual;

			if (WS2812_Frame_Total_Count>0) {
				DS_Pause_interrupt_Flag |= 64; //вкл ожидаание конца вывода
			}
		//назначить стартовую позицию
			WS2812_SetStartPixelInBuffer(WS2812_Frame_Start_Pointer_Actual);
			//if (DebugInfoOutFlag){printf ("MULTI upd\r\n");}
			Servo_Update_Flag &= ~0x20; //флаг смены позиций опущен
		}

		if ( (~Servo_Update_Flag) & 1 ) {
			start_servos();
		}

	}


}/**/



//сообщение о неудачной попытке
//a - где проблема
//1 - File, 2 - Dir, 3 - FAT, (4 - usart1, 5 - usart2, 6-USB), остальное - line
//b - действие
//1 - read , 2- open, 3- mount, (4- descript), 5 - delete, остальное - write
void Fail_Message (uint8_t a, uint8_t b, uint8_t res)
{
	if (DebugInfoOutFlag){
		switch (a){
		case 1:
			Message ("File \0",DEBUG_PORT_OUT);
			break;
		case 2:
			Message ("Dir \0",DEBUG_PORT_OUT);
			break;
		case 3:
			Message ("FAT \0",DEBUG_PORT_OUT);
			break;
		/*case 4:
			Message ("USART 1 \0",DEBUG_PORT_OUT);
			break;
		case 5:
			Message ("USART 2 \0",DEBUG_PORT_OUT);
			break;
		case 6:
			Message ("USB \0",DEBUG_PORT_OUT);
			break;*/
		default:
			Message ("Line \0",DEBUG_PORT_OUT);
		}

		switch (b){
		case 1:
			Message ("read \0",DEBUG_PORT_OUT);
			break;
		case 2:
			Message ("open \0",DEBUG_PORT_OUT);
			break;
		case 3:
			Message ("mount \0",DEBUG_PORT_OUT);
			break;
		case 4:
			//Message ("descript \0",DEBUG_PORT_OUT);
			break;
		case 5:
			Message ("delete \0",DEBUG_PORT_OUT);
			break;
		default:
			Message ("write \0",DEBUG_PORT_OUT);
		}



		if (res){
			Message ("failed (\0",DEBUG_PORT_OUT);

			//Port_send_char('(',DEBUG_PORT_OUT);
			Port_send_char(DS_inttochar((res>>4)&0x0F),DEBUG_PORT_OUT);
			Port_send_char(DS_inttochar(res&0xF),DEBUG_PORT_OUT);
			Port_send_char(')',DEBUG_PORT_OUT);
		}
		//dbgu_send_char(0x0D);
		//dbgu_send_char(0x0A);
		Port_send_char(0x0D,DEBUG_PORT_OUT);
		//Port_send_char(0x0A,1);
#if (DEBUG_PORT_OUT == 0xf)
		USB_main_COM_react();
#endif
	}
}


void Calculate_Fileworks_Output_Port(){
	a8=((SD_Volume_Exists>>2)&3) - 1;
}

void Finish_Fileworks_Message(uint8_t port){
	Port_send_char ('\r',port);
	if (port>1){
		USB_main_COM_react();
		// если буфер занят, подождать освобождения
		//	uint32_t i = 5000;
		//	while (((EP1R & 0x80) == 0) && (i))
		//		i--;
		}
}



//str - строчка для вывода. должна оканчиваться на '\0'
//a - вывод (1 - dbgu, 0 - USART1)
void Message (char Str[], uint8_t a){
	for (a16 = 0; Str[a16]; a16++)
	{
		Port_send_char(Str[a16],a);
		//if ( a ) {dbgu_send_char(Str[a16]);}
		//else {USART1_send_char(Str[a16]);}
	}
}


//мат операция для команд где может быть + или -
//Inp - значение, к которому прибавить или вычесть параметр
//arg - вычитаемое или прибавляемое число
//Max - наибольшее разрешённое значение
uint32_t ModifyParameter (uint32_t Inp, uint32_t *arg, uint32_t Max){

	if (DS_Math_Flags == 2){//((DS_Flag_Register & 0xC)==0xC){
		return (Inp > *arg) ? (Inp - *arg) : 0;
	}
	if (DS_Math_Flags == 1){//(DS_Flag_Register & 8){
		return ((Inp + *arg) < Max) ? (Inp + *arg) : Max;
	}
	/*
	if (DS_Flag_Register & 8){
		if (DS_Flag_Register & 4) {
			return (Inp > *arg) ? (Inp - *arg) : 0;
		} else {
			return (Inp + *arg);
		}
	}/**/
	return (*arg < Max) ? *arg : Max;
}

void Check_If_MainLoop_Is_AFK(){
	if (DS_Pause_interrupt_Flag & 0x8000){
		MainLoop_ResetTime ++;
		if (MainLoop_Maximum_Wait_ms < MainLoop_ResetTime){
			MainLoop_ResetTime = 0;
			descrActionFlag = DS_ACTION_RETURN_FROM_AFK;
			MainLoop();
		}
	}
}
