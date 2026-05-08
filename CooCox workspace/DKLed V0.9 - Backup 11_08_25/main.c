/*************************************************************************************************
 * @file		main.c
 *
 * @brief		Программное обеспечение контроллера управляемых светодиодов (STM32)
 *
 * @version		v0.02
 * @date		29.10.2017
 *
 ************************************************************************************************/
// ВНИМАНИЕ!  описание поддерживаемых кодов из h-файла находится в конце файла "descript_S.h"


//*-----------------------------------------------------------------------------------------------
//*			Подключаемые внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"
//#define FastButtonShortcuts //включение коротких команд на кнопках

//*-----------------------------------------------------------------------------------------------
//*			global variables
//*-----------------------------------------------------------------------------------------------
//ID задач
//OS_TID debugTaskID, outputTaskID, inputsTaskID;

// для модуля descript_S.h
uint8_t DS_status = 0; //какую команду ожидаем
uint16_t DS_buf_start = 0; //с какого места расшифровывать, если команда из нескольких знаков
uint16_t DS_buf_counter;// = 512; //счетчик, какое число пришло последним в буфер
//uint8_t DS_buffer[256] = "S02 P 1f0000 001A00 2c0000 00000f";
//uint8_t	DS_buffer2[256] = "S4 P1f0000 001A00 2c0000"
//		" 00000f 1f0000 001A00 2c0000 00000f 1f0000 001A00 2c0000 00000f  "; //буфер приема команды
uint8_t DS_comm_num = 0; //текущий номер кода (который после буквы)
uint16_t DS_WSpoint_counter = 0; //текущий принимаемый номер точки WS
uint8_t DS_RGB_counter = 0; //счетчик принятых символов для точки RGB
uint32_t DS_Param = 0; //32-битный расшифровываемый параметр для дешифровщика
uint8_t text_buff[READ_SIZE+4]; // чтение из файла, и его же - для дешифровки
uint8_t file_to_read = 0; //!!!переменная, которая гооврит, как читать файл. 0 - с начала, 1 - продолжаем как было.
//uint8_t save_buff[24]; //временный буфер для сохранения команд, если очередное чтение из файла застало на половине команды
uint16_t DS_maxCurrPoints = 0; //переменная для вывода только нужного количества точек
uint16_t DS_TotalPoints = WS2812_IO_FRAMEDATA_PIXELS; //переменная для пределения максимального подключённого количества точек
uint8_t DS_LED_Brightness = MAX_BRIGHTNESS; //яркость светодиодов

//переменная для выбора канала чтения
uint8_t DS_Channel_Select; //0-любой канал готов, 1-строго файл, 2-строго USART1, 3-строго USART2 (DBGU), 4-строго USB



uint32_t Personal_ID; //личный ID контроллера для общения с другими


// для модуля init_ws.h
uint8_t WS2812_IO_framedata[ WS2812_IO_FRAMEDATA_SIZE ];
uint8_t WS2812_IO_High = 0xFF;
uint8_t WS2812_IO_Low = 0x00;
//volatile uint8_t WS2812_TC = 1;
volatile uint8_t TIM4_overflows = 0;
uint16_t Timer4_counter = 29; // 800kHz период вывода битов, можно менять в процессеисполнения программы
uint8_t LED_control_type = 1; //LEDs with 1 signal and no clock cable (like ws2812)

uint16_t WS2812_Frame_Length = 1; //количество пикселей в кадре
uint16_t WS2812_Frame_Length_Actual = 1; //количество пикселей в кадре
uint32_t WS2812_Frame_Byte_Length = 24; //количество байт в кадре (WS2812_Frame_Length*24)
uint32_t WS2812_Frame_Byte_Length_Actual = 24;
uint16_t WS2812_Frame_Count = 1; //количество кадров в анимации
uint16_t WS2812_Frame_Count_Actual = 1; //количество кадров в анимации
uint16_t WS2812_Frame_Total_Count = 1; // всего кадров (определяем исходя из размеров массивов - сколько кадров вообще может поместиться при такой длине)
uint32_t WS2812_Frame_Start_Pointer = 0; // указатель на первый проигрываемый кадр (адрес старта массива)
uint32_t WS2812_Frame_Start_Pointer_Actual = 0;// указатель на текущий проигрываемый кадр
//uint32_t WS2812_Current_Frame_Start = 0; // указатель на текущий проигрываемый кадр
uint16_t WS2812_Frame_Period = 625; //период в мкс, по сути то же что Servo_Period, только значение должно храниться независимо
uint16_t WS2812_Frame_Period_Actual = 625;


//установка параметров серводвигателя - частота обновления, минимальная и максимальная длительность импульса управления
// в микросекундах
uint32_t Servo_Period = SERVO_PERIOD_DEF; //50Гц
uint32_t Servo_Actual_Period = SERVO_PERIOD_DEF;// = 20000; //50Гц
uint32_t Servo_MinPos = SERVO_POS_MIN_DEF; //1 ms
uint32_t Servo_MaxPos = SERVO_POS_MAX_DEF; //2 ms
uint16_t Servo_Resolution = 256; //разрешение

uint32_t Servo_Period_default = SERVO_PERIOD_DEF; //50Гц
uint32_t Servo_MinPos_default = SERVO_POS_MIN_DEF; //1 ms
uint32_t Servo_MaxPos_default = SERVO_POS_MAX_DEF; //2 ms
uint16_t Servo_Resolution_default = 256; //разрешение

uint16_t Servo_Pos [9]; //массив для позиций серводвигателей
uint8_t Servo_Action_Mask = 0xFF; //список активных серв
//в режиме мультиуправления (сервы + светодиоды) WS2812_IO_High = ~Servo_Action_Mask
//uint16_t Servo_Dead_Band_Width = 10; //период вызова таймера сервов
uint16_t Servo_Pos_Temp [8]; //массив для хранения следующих позиций серводвигателей (диапазон от 0 до Servo_Resolution)
uint16_t Servo_Pos_Mask[10]; //массив значений модулятора на каждом такте
uint32_t Servo_time; //таймер для серв
uint8_t Servo_Update_Flag = 0; // флаг состояния серв
// 0x01: 0 - не активен 1 - фктивен
// 0x02: 1 - изменились PosTemp
// 0x04: 1 - изменились параметры
// 0x08: 1 - мы в промежутке между импульсов
uint8_t Servo_step; //шаг, на котором находится прерывание - он же индекс массивов позиций сервов
//extern uint16_t Servo_Dead_Band_Width = 10; //период вызова таймера сервов
uint16_t Servo_GPIO_Setting_on = 0; //для ног вкл/выкл
uint16_t Servo_GPIO_Setting_off = 0;
uint16_t Servo_GPIO_Actual_on = 0; //для ног вкл/выкл
uint16_t Servo_GPIO_Actual_off = 0;


//флаги ОС
/*StatusType outputStartFlag; //флаг - команда на расшифровку текста
StatusType fileReadStartFlag; //флаг - команда на чтение файла
StatusType r2; //для проверки результата установики флагов
*/
//отладка
//uint32_t timer_ms;
volatile uint8_t DebugInfoOutFlag;
extern uint8_t dbgu_State_of_recieved_Command; //индикатор/ркегистр состояния чтения команды из USART2
extern uint8_t USART1_State_of_recieved_Command; //индикатор/ркегистр состояния чтения команды из USART2
//1 - мы готовы воспринимать приходящие данные и ждём заголовок
//0 - мы игнорируем входящие данные
//3,5,7,9 - читаем заголовок (4 байта на ID) - 3-прочитали 1й, 5- прочитали 2й,...,9 - прочитали последний
//0x10 - мы принимаем инфу
//0x10+0x80 - есть команда, но она не закончено и половину буфера мы уже заняли
//0x80 - есть команда
//0x20 - приём открыт, мы читаем данные и куда-то их распределяем
extern volatile uint32_t dbgu_rx_timeout; //предельное время ожидания конца команды от её начала
extern volatile uint32_t USART1_rx_timeout; //предельное время ожидания конца команды от её начала




//для пауз и таймлайнов
U64 osTime; //для времени ОС
U64 nextPauseStartTime = 0; // хранение времени начала предыдущей паузы - для сохранения общего таймлайна несмотря на тормоза
U64 FileReadStartTime=0; //хранение времени начала чтения текущего файла
U64 nextAbsolutPauseExpireTime =0; //хранение времени, когда должна закончиться текущая абсолютная пауза
U64 M25ReverseTime = 0;
uint32_t M25ReversePauseValue = 0;

uint32_t need_pause; //временное хранилище значения текущей паузы
uint16_t DS_Pause_interrupt_Flag; //поднимаеется, если мы только что выполнили паузу
U64 File_Suspend_start;


//для кнопок и выбора других файлов
//static char fileForButton[FILE_NAME_MAX_LENGTH][MAX_NUM_BUTTONS]; //массив имен файлов для перехода по кнопке
uint16_t numFileForButton[MAX_NUM_BUTTONS_ARR];	//массив имён файлов для перехода по кнопкам (хранятся как цифры), так же тут будут жить параметры коротких команд
// от 0 до (MAX_NUM_BUTTONS-1) - переход по нажатию кнопки, дальше - по отпусканию
#if defined (FastButtonShortcuts)
//	uint16_t ParameterForButton[MAX_NUM_BUTTONS_ARR];
	uint8_t FastCommandForButton[MAX_NUM_BUTTONS_ARR]; //идентификатор быстрой команды
	uint8_t CommandArgForButton[MAX_NUM_BUTTONS_ARR]; //короткий аргумент
//	uint8_t TextToSendArrayForButton[BUTTONS_TEXT_ARRAY_SIZE];
//	uint8_t TextToSendArrayIndexesForButton[BUTTONS_TEXTS_AMOUNT];
//	uint8_t TextToSendArrayLengthsForButton[BUTTONS_TEXTS_AMOUNT];
//	uint8_t TextToSendArrayPointer;
//	uint8_t TextToSend_TargetAddress;
#endif

BYTE descrActionFlag = 0; //битовые флаги разных действий в процессе цикла чтения/расшифровки, константы в DS_ACTION_...
//uint8_t buttonPressed; //какая кнопка была нажата
uint16_t buttonPushInterrupt; //для передачи информации о нажатых/отпущенных кнопках
uint16_t buttonRelInterrupt;
uint16_t buttonsState = 0 ; //запоминаем состояние кнопок, чтобы правильно ловить нажатие/отпускание (по битам)
uint16_t buttonsNewState; //текущее состояние нажатости кнопок, чтобы ловить что изменилось
//uint16_t buttonPushInterruptActive = 0; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
//uint16_t buttonRelInterruptActive = 0; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
#ifndef FastButtonShortcuts
uint16_t buttonPushWaitStateEndFile=0; // флаги активности кнопок по типам событий
uint16_t buttonPushWaitStateEndMulti=0; // флаги активности кнопок по типам событий
uint16_t buttonRelWaitStateEndFile=0; // флаги активности кнопок по типам событий
uint16_t buttonRelWaitStateEndMulti=0; // флаги активности кнопок по типам событий
#endif
U64 ButtonWaitStart=0; //когда произошло последнее изменение состояния кнопок
U64 ButtonCompleteStart=0; //когда произошло последнее изменение состояния кнопок при мультинажатии

uint32_t ButtonWaitSetting=0; //через сколько милисекунд обнулять кнопки (повторное нажатие)
uint32_t ButtonCompleteSetting=500; //сколько времени (мс) на набор комбинации кнопок (многонажатная система)
uint8_t ButtonFlags = 0;
//1 - нажатие по одной кнопке (0) или комбинацией (1)
//2 - (1)= ждём конца нажатия
//4
//8 - (0)= последний раз FastCommand сработал вхолостую,
//0x10 - принимаем байт как команду с uart 1
//0x20 - принимаем байт как команду с uart 2
//0x40 - включение I2C вместо кнопок 4 и 5

uint16_t ButtonEncoder = 0;//биты этой штуки указывают, какие кнопки работают как энкодер
uint16_t ButtonEncorerIndiv[MAX_NUM_ENCODERS];

uint16_t AfterLoop_File_Num = 0xFFFF; //код файла по отложенному переходу M47
uint16_t AfterMulti_File_Num = 0xFFFF; //код файла по отложенному переходу конец мультикадра

uint8_t ChosenNomberInM98 = 0; //предварительно одобренный выбор файла для команды M98Q<>P<>

#ifdef VARIABLES_SUPPORT
	uint16_t Custom_Variables_List[26];
#endif
//переменные для USB_FS



//обработчмк файловой системы заниммает в ПЗУ 7 500 байт
//максимум, который может занимать всё остальное (text)- 58 000байт
//*-----------------------------------------------------------------------------------------------
//*			Время компиляции и версия программы
//*-----------------------------------------------------------------------------------------------
//const char version[4] = "0.2\0";

//*-----------------------------------------------------------------------------------------------
//*			Главная функция
//*-----------------------------------------------------------------------------------------------
int main(void)
{

#if (0) //protection (250 bytes)
#define FLASH_KEYR    (*(volatile uint32_t*)0x40022004U)
#define FLASH_OPTKEYR (*(volatile uint32_t*)0x40022008U)
#define FLASH_CR      (*(volatile uint32_t*)0x40022010U)
#define FLASH_SR      (*(volatile uint32_t*)0x4002200CU)


	volatile uint32_t *_flash_obr = (uint32_t*)0x4002201CU;
	if (!((*_flash_obr) & 0x2)) {
		// Read protection NOT enabled ->

		// Unlock option bytes
		//_flash_unlock(1);
		// Clear the unlock state.
		FLASH_CR |= FLASH_CR_LOCK;

		// Authorize the FPEC access.
		FLASH_KEYR = 0x45670123U;
		FLASH_KEYR = 0xcdef89abU;
			// F1 uses same keys for flash and option
			FLASH_OPTKEYR = 0x45670123U;
			FLASH_OPTKEYR = 0xcdef89abU;
		// Delete them all
		//_flash_erase_option_bytes();
			//_flash_wait_for_last_operation();
			while (FLASH_SR & FLASH_SR_BSY);
			FLASH_CR |= FLASH_CR_OPTER;
			FLASH_CR |= FLASH_CR_STRT;
			//_flash_wait_for_last_operation();
			while (FLASH_SR & FLASH_SR_BSY);
			FLASH_CR &= ~FLASH_CR_OPTER;

		// Now write a pair of bytes that are complentary [RDP, nRDP]
		//_flash_program_option_bytes(0x1FFFF800U, 0x33CC);
			//_flash_wait_for_last_operation();
			while (FLASH_SR & FLASH_SR_BSY);

			FLASH_CR |= FLASH_CR_OPTPG;  // Enable option byte programming.
			volatile uint16_t *addr_ptr = (uint16_t*)0x1FFFF800;
			*addr_ptr = 0x33CC;
			//_flash_wait_for_last_operation();
			while (FLASH_SR & FLASH_SR_BSY);
			FLASH_CR &= ~FLASH_CR_OPTPG;  // Disable option byte programming.

		//_flash_lock();
		FLASH_CR |= FLASH_CR_LOCK;
	}

	// Disable JTAG and SWD to prevent debugging/readout
	volatile uint32_t *_AFIO_MAPR = (uint32_t*)0x40010004U;
	*_AFIO_MAPR = (*_AFIO_MAPR & ~(0x7 << 24)) | (0x4 << 24);
#endif

	DBGU_Init(128000);
	USART1_Init(128000);
/*
// режим теста новых моторов
	USART1_Init(1000000);
	USART1->CR2 &= CR2_LINEN_Reset;
	USART1->CR2 &= CR2_CLOCK_CLEAR_Mask;
	USART1->CR3 &= CR3_IREN_Reset;
	USART1->CR3 &= CR3_SCEN_Reset;
	USART1->CR3 |= CR3_HDSEL_Set;
*/

	dbgu_State_of_recieved_Command = 0x21;
	USART1_State_of_recieved_Command = 0x21;
	dbgu_rx_timeout = USART_Default_timeout; //время на приём команды - 1 с
	USART1_rx_timeout = USART_Default_timeout; //время на приём команды - 1 с
	//usb
	USB_com_Init();


	// кнопки
	ButtonsInit();
#ifdef SoftwareI2C
	I2C_preInit();
#endif
	srand(0);
	//I2C1_init(5);

	Personal_ID = DEFAULT_PERSONAL_ID;
	DebugInfoOutFlag=1;
	// приветствие

	//printf("");
	//printf("===============================================\r\n");
	//Message ("\rDKLed v0.7\r\0", 0);
	//Message ("\rDKLed v0.7\r\0", 1);
	//Message ("\r\nDKLed v0.5\r\0", 2);
/*	  RCC_ClocksTypeDef RCC_ClocksStatus;
	  RCC_GetClocksFreq(&RCC_ClocksStatus);

		Message ("SYSCLK_Freq: 0x\0", 1);
uint8_t i;
	  for (i = 0;i < 8;i++) {
			dbgu_send_char(DS_inttochar((uint8_t)((RCC_ClocksStatus.SYSCLK_Frequency >> (28-(i<<2)))&0x0F)));
		}
		Message ("\r\nDPCLK2_Freq:\0", 1);
		  for (i = 0;i < 8;i++) {
				dbgu_send_char(DS_inttochar((uint8_t)((RCC_ClocksStatus.PCLK2_Frequency >> (28-(i<<2)))&0x0F)));
			}
		Message ("\r\n\0", 1);/**/

    // инициализация ОС
	//CoInitOS();
	TInitOS();

	InputsTaskInit();
	OutputTaskInit();
	DebugTaskInit();

	// запуск системы
	//CoStartOS();


}
