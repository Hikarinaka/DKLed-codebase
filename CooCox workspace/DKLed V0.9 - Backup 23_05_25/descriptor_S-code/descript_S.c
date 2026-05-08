#include "descript_S.h"
#include <stdio.h>
#include <init_ws.h>
#include "debugTask.h"
#include "includes.h"
#include "I2C_c.h"

extern uint8_t DS_Channel_Select;
extern volatile uint8_t DebugInfoOutFlag; //включает и отключает вывод служебной информации на второй USART (DBGU)
extern uint16_t DS_Pause_interrupt_Flag; //поднимаеется, если мы только что выполнили паузу

extern uint16_t Current_File_Num; //хранение кодов текущего и предыдущего файлов
extern uint16_t Prev_File_Num;
extern uint16_t Parent_File_Num;
extern uint16_t Parent_Prev_File_Num;
extern uint16_t File_Read_Cycle_Count;
extern uint32_t Personal_ID; //личный ID контроллера, нужен для общения с другими, "0000" или 0x30303030 - общий
//extern uint32_t time; //время в мкс, затраченное на открытие нового файла
extern uint32_t CRC_sum; //CRC
extern uint8_t I2C_TargetAddress;//куда отправлять
extern uint8_t I2C_Bytes_To_Send;//сколько отправлять
extern uint8_t I2C_Bytes_to_Recieve;
extern uint8_t I2C_stage;


extern uint16_t Servo_Pos_Temp [8]; //массив для хранения следующих позиций серводвигателей (диапазон от 0 до 1024)
extern uint16_t DS_LastPointsUpdated; //переменная на случай повторного вывода всё тех же точек
extern uint8_t LED_control_type;  //тип управления светодиодов,
extern uint8_t WS2812_IO_High;
extern uint8_t WS2812_IO_Low;
extern uint8_t Servo_Action_Mask;
extern uint8_t Servo_Update_Flag; // флаг состояния серв
extern uint16_t Servo_GPIO_Setting_on; //для ног вкл/выкл
extern uint16_t Servo_GPIO_Setting_off;
extern uint16_t Servo_Resolution;//=256 //разрешение
extern uint16_t WS2812_Frame_Length; //количество пикселей в кадре
extern uint16_t WS2812_Frame_Length_Actual;
extern uint32_t WS2812_Frame_Byte_Length_Actual;
extern uint16_t WS2812_Frame_Count; //количество кадров в анимации
extern uint16_t WS2812_Frame_Count_Actual;
extern uint16_t WS2812_Frame_Total_Count; // всего кадров (определяем исходя из размеров массивов - сколько кадров вообще может поместиться при такой длине)
extern uint32_t WS2812_Frame_Start_Pointer; // указатель на первый проигрываемый кадр (адрес старта массива)
extern uint32_t WS2812_Frame_Start_Pointer_Actual; // указатель на текущий проигрываемый кадр
extern uint16_t WS2812_Frame_Period; //период в мкс, по сути то же что Servo_Period, только значение должно храниться независимо
extern uint16_t WS2812_Frame_Period_Actual;
extern uint8_t dbgu_rx_buf_overcount;
extern char fname[FILE_NAME_MAX_LENGTH];
extern DWORD FileWorksPTR;
extern uint8_t text_buff[READ_SIZE+4]; //буфер из которого происходит чтение команд для расшифровки
extern uint8_t DS_LED_Brightness; //яркость светодиодов
extern uint16_t DS_TotalPoints;//переменная для вывода только нужного количества точек

extern char path[FILE_PATH_MAX_LENGTH]; // = (char*)malloc(18);
extern char path2[FILE_PATH_MAX_LENGTH]; // = (char*)malloc(18);
extern char Dirpath[DIR_PATH_MAX_LENGTH]; // = (char*)malloc(18);
extern char FileworksDirpath[DIR_PATH_MAX_LENGTH]; // = (char*)malloc(18);
extern DIR dir;
extern FILINFO filinfo;
extern uint8_t SD_Volume_Exists;

extern U64 M25ReverseTime;
extern uint32_t M25ReversePauseValue;

//для кнопок и выбора других файлов
//extern char fileForButton[FILE_NAME_MAX_LENGTH][MAX_NUM_BUTTONS]; //массив имен файлов для перехода по кнопке
extern uint16_t numFileForButton[MAX_NUM_BUTTONS_ARR];	//массив имён файлов для перехода по кнопкам (хранятся как цифры)
//extern uint8_t lenFileForButton[MAX_NUM_BUTTONS_ARR]; //длины имён файлов для перехода по кнопкам
//	extern uint16_t ParameterForButton[MAX_NUM_BUTTONS_ARR];
	extern uint8_t FastCommandForButton[MAX_NUM_BUTTONS_ARR]; //идентификатор быстрой команды
	extern uint8_t CommandArgForButton[MAX_NUM_BUTTONS_ARR]; //короткий аргумент
	uint8_t ButtonNomberCarrier = 0; //номер ячейки в массиве кнопок, то есть учитывает сдвиг по QR.
//extern uint16_t buttonPushInterruptActive; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
//extern uint16_t buttonRelInterruptActive; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
extern uint8_t ChosenNomberInM98; //предварительно одобренный выбор файла для команды M98Q<>P<>
extern uint8_t ButtonFlags;
//1 - нажатие по одной кнопке (0) или комбинацией (1)
//2 - (1)= ждём конца нажатия
//4
//8
//0x10 - принимаем байт как команду с uart 1
//0x20 - принимаем байт как команду с uart 2

extern int16_t ButtonEncoder;
extern uint16_t ButtonEncorerIndiv[MAX_NUM_ENCODERS];

uint16_t CheckPixel_miscomparations=0; //считаем, скольк пикселов не совпало


uint8_t DS_Flag_Register = 0; //дополнительные флаги работы
//b1 (1) - 0 = читаем число в hex формате, 1 - читаем число в dec формате/ I2C: чтение с параметром (адресом)
//b2 (2) - 0 = ничего, 1 - I2C: читаем из порта
//b3 (4) - 0 = положительный, 1 = отрицательный
//b4 (8) - 0 = точное значение/ничего, 1 = дельта/ прыгнуть на пиксель
//b5 (16,  0x10) - 0 = ничего, 1 = проверить совпадение с цветом
//b6 (32,  0x20) - 0 = нормальное выполнение команды, 1 = добавить команду в кнопку
//b7 (64,  0x40) - 0 = ничего, 1 - смена позиции пикселя
//b8 (128, 0x80) - 0 = ничего,
char 	DS_current_char = 0;  //для считывания одного символа из строки

//const char DS_Symbol_comment1=';';
//const char DS_Symbol_comment2='(';
//const uint8_t Max_Random_File_List = 200;

uint8_t DS_Status_backup;
uint8_t	DS_Repeater_set;
//b1 (1) - 1 - флаг инициации повтора
//= 1 	- простой повтор (цвета, команды)
//= 2+1 - повтор с декрементом (несколько "v")
//= 4+1 - овтор просто (">")
//= 8+1 - повтор с инкрементом (несколько "^")
//b5 (0x10) - флаг что был один из символов v>^

uint8_t DS_Repeat_Count;
//uint8_t  DS_Status_Before_comment;
uint8_t  DS_WS_line_mask;
int Descript_answer;

extern uint16_t USB_Bytes_to_send_left;
extern uint16_t USB_Recieved_bytes;
extern uint8_t USB_Function_flags;
extern unsigned char USB_Buff1[64];
extern unsigned char USB_COM_TX_Buff1[64];
extern uint16_t String_in_file_Index;
extern uint16_t Char_in_string_index;

//=================================================================================
int Descript(
		uint8_t WS2812_IO_framedata_[],
		uint8_t DS_buffer_[],			//ссылка на буфер с текстом команды (или частью команды)
		uint16_t *DS_buf_start_,		//откуда в текстовом буфере начинать анализ
		uint16_t *DS_buf_counter_, 		//где в текстовом буфере заканчивать анализ
		uint8_t *DS_status_,			//какую информацию ждём в начале анализа (!!! переделать на enum)
		uint8_t *DS_comm_num_,   		//текущий номер кода (который после буквы)
		uint16_t *DS_WSpoint_counter_,  //текущий принимаемый номер точки WS (или байта массива при raw-data)
		uint8_t *DS_RGB_counter_,		//счетчик принятых символов для точки RGB
		uint16_t *DS_maxCurrPoints_, 		//максимальное количество точек, обновленных в текущем кадре
		uint32_t *DS_Param_)			//32-битное значение параметра
{
	uint16_t DS_i = 0; // служебный счетчик (для основного прохода по буферной строке)
	uint8_t DS_a8 = 0; // вспомогательная переменная
	uint8_t DS_End_Of_Command_flag;
	//uint16_t DS_a16 = 0; // вспомогательная переменная
	//uint32_t DS_a32 = 0; // вспомогательная переменная
	//	uint8_t DS_b = 0; // вспомогательный счетчик для мелких внутренних циклов
	DS_current_char = 0;  //для считывания одного символа из строки


	for (DS_i = *DS_buf_start_; DS_i <= *DS_buf_counter_; ++DS_i)
	{
		//printf("Descr OSTime()=%d\r\n", (int) CoGetOSTime());

		DS_current_char = DS_buffer_[DS_i];
		*DS_buf_start_ = DS_i + 1;
		if (DS_Channel_Select == 1) {
			Char_in_string_index ++;
			if (DS_current_char == 0x0D){
				String_in_file_Index ++;
			}
		}
		//if (DS_i>130 && DS_i<145){
			//printf("     *DS_status_ = 0x%02X, DS_i = %d, ", *DS_status_, DS_i);
			//printf("DS_current_char = %d\r\n", DS_current_char);//v
			//CoTickDelay(20);
		//}
		if ( (*DS_status_ != DS_Type_command_RAW)
				//&& (*DS_status_ != DS_Type_command_USART2_SEND)
				&& (*DS_status_ != DS_Type_command_USART1_SEND)
				&& (*DS_status_ != DS_Type_command_FILE_WRITE_PART)){
//если нам не важно чтобы в процессе отправлялся именно тот символ, который нужен
			if (DS_current_char == ' '){
				//все пробелы везде игнорируются
				//кроме случаев отправки сырых данных на светодиоды и через usart-ы
				//*DS_buf_start_ = DS_i + 1;
				//printf("         'space' received; *DS_status_ = 0x%02X, DS_i = %d, \r\n ", *DS_status_, DS_i);
				//CoTickDelay(50);
				continue;
			} else if (DS_current_char > 0x60 && DS_current_char <0x7B){ //символы с 61 (a) по 7A (z)
				DS_current_char -= 0x20; // 61->41,... 7A->5A или a->A z->Z
			}

		}
		DS_End_Of_Command_flag = 0;
		if (DS_current_char == 0x0D || DS_current_char == 0x0A ||  DS_current_char == 0){
			DS_End_Of_Command_flag = DS_Type_command_new;
			Char_in_string_index = 0;
		} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
			DS_End_Of_Command_flag = DS_Type_command_comment;
		}
		//DS_Status_Before_comment = (*DS_status_ == DS_Type_command_comment)? DS_Status_Before_comment : *DS_status_;
		switch (*DS_status_) {
			case DS_Type_command_new: //начало принятия команды
				//*DS_Param_ = 0;
				DS_Flag_Register &=~0x7D; //по умолчанию всё в HEX,  снимаем биты 1,4,8, 16, 32 (0x1+0x4+0x8+0x10+0x20+0x40), указывающий на расширенную команду
				*DS_comm_num_=0;
				*DS_RGB_counter_ = 0;
				//DS_Flag_Register &= ~1; //по умолчанию дальнейший приём значений команд - в hex
				if (DS_current_char == 'S'){
					*DS_status_ = DS_Type_command_S;
				}else if (DS_current_char == 'F'){
					*DS_status_ = DS_Type_command_F;
				}else if (DS_current_char == 'A'){
					*DS_status_ = DS_Type_command_A;
				}else if (DS_current_char == 'G'){
					*DS_status_ = DS_Type_command_G;
				}else if (DS_current_char == 'M'){
					*DS_status_ = DS_Type_command_M;
				}else if (DS_current_char == 'U'){
					*DS_status_ = DS_Type_command_U;
				}else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2 ){
					if (DS_Channel_Select==1){
						*DS_status_ = DS_Type_command_comment;
					}
				}else if (DS_current_char == 0x0D || DS_current_char == 0x0A ||  DS_current_char == 0){ //DS_current_char == '/r'
					; //второй символ при переводе строки - игнорируем
				}
				else {
					Error_Message_full(0, 0xFF,0, 0, 1,DS_i, DS_current_char);
					//*DS_buf_start_ = DS_i + 1;
					*DS_status_ = DS_Type_command_comment; //считаем всё ошибкой, пока не дойдём до следующей строки
					return DS_ERR_UNKNOWN_SYMBOL;
				}

				*DS_Param_ = 0;
				//*DS_buf_start_ = DS_i + 1;
				break; //case DS_Type_command_new:
//----------------------------------------------------------------------------------------------------------------------------
//определяем, какой тип S команды будет - запись цветов на один вывод или же сырые данные
			case DS_Type_command_S: // ждем номер для S-команды
				DS_a8 = DS_chartoint (DS_current_char);
				//if (DS_i>505){
				//	printf(" DS_Type_command_S DS_a8r = %d\r\n", DS_a8); //v
				//}

				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					*DS_Param_ = 0;
					if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P),
						//к этому моменту цифра уже должна быть определена в циклах ранее
						//DS_a32 = DS_strtohex(DS_buffer_, *DS_buf_start_, DS_i - *DS_buf_start_); //устаревшая конструкция
						//printf("*DS_Param_ = 0x%03X\r\n", *DS_Param_); //v

						//*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF;



						*DS_status_ =  DS_Type_command_WS_S0;
						DS_Status_backup = DS_Flag_Register;
						DS_Repeater_set = 0;
						DS_Repeat_Count = 0;
						//DS_Flag_Register &= ~1; //дальнейший приём значений команд - в hex
						//CheckPixel_miscomparations = 0;
						//*DS_Param_ = 0; //здесь теперь будет храниться цвет очередной расшифровываемой точки RGB
						//	*DS_buf_start_ = DS_i + 1;
					//	*DS_RGB_counter_ = 0;  // сброс переменных для распознавания WS-последовательности
						*DS_WSpoint_counter_ = 0;
					} //теперь в DS_a32 и *DS_comm_num_ находится значение одно-двух-трехзначного S-кода

					//SR Q<кол-во точек> P<raw></r/n>. После P пробелы не допускаются, строго идут сырые байты в требуемом количестве
					else if (DS_current_char == 'R'){// прием raw-данных, дальше ждём букву Q и сырые данные
						//printf("*DS_Param_ = 0x%03X\r\n", *DS_Param_); //v

						//*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF;
						//if (*DS_comm_num_ > 7) return DS_ERR_CODE_OUT_OF_RANGE;
						//DS_Flag_Register &= ~1; //дальнейший приём значений команд - в hex
						*DS_status_ =  DS_Type_command_SR;
						//*DS_comm_num_ = 4;  //*DS_comm_num_ используется как флаг состояния приема: ==4 - ждём букву Q, ==5 - ждём значение параметра Q
						//*DS_Param_ = 0; //здесь теперь будет храниться количество точек
						//	*DS_buf_start_ = DS_i + 1;
						//*DS_RGB_counter_ = 0;  // сброс переменных для распознавания WS-последовательности
						//*DS_WSpoint_counter_ = 0;
					} //теперь в DS_a32 и *DS_comm_num_ находится значение одно-двух-трехзначного S-кода
					else if (DS_current_char == 'I'){
						*DS_status_ = DS_Type_command_G;
						//	*DS_buf_start_ = DS_i + 1;
						// *DS_comm_num_ = 0x27;
						//goto Setting_Of_Variables; //переход на логику G26,... G36
						*DS_Param_ = 0x28;//переход на логику G26,... G35
					}else if (DS_current_char == 'M'){//setting maximum amount of pixels on WS lines
						*DS_status_ =  DS_Type_command_USART_BAUD;
						*DS_comm_num_ = DS_WS_SET_MAX_LENGTH;
					} else {
						//if (DebugInfoOutFlag){
						Error_Message_full('S', 0xFF,0, 0, 1,DS_i, DS_current_char);
						//Error_Message_Command('S', 0xFF);//printf("DS_Type_command_S,   DS_ERR_UNKNOWN_SYMBOL  "); //v
						//Error_Message_data(1,DS_i, DS_current_char);
						//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char); //v
						//}
						return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
					}
				} else {
					//если символ был цифровой - заносим свежую цифру в сразу в параметр
					if (DS_a8 > 7) {
						Error_Message_full('S', 0xFF,0, 0, 2,DS_i, DS_current_char);
						//Error_Message_Command('S', 0xFF);
						//Error_Message_data(2,DS_i, DS_current_char);
						return DS_ERR_CODE_OUT_OF_RANGE;
					}
					*DS_comm_num_ |= (0x01 << DS_a8);
					//*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
					//printf("*DS_Param_ = 0x%03X\r\n", *DS_Param_); //v
				}
				break; //case DS_Type_command_S:
//S...---------------------------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_WS_S0: //расшифровка цветов точек для WS
//WS_Sx_Dscript_Process: //прямая ссылка для дескриптов по SxP...
				// проверка на начало комментария
/*				if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
					*DS_status_ = DS_Type_command_comment;
					//*DS_maxCurrPoints_ = *DS_WSpoint_counter_; //количество обновляемых точек берем из счетчика точек
//					if (*DS_maxCurrPoints_ < *DS_WSpoint_counter_){*DS_maxCurrPoints_ = *DS_WSpoint_counter_;} //количество обновляемых точек берем из счетчика точек
					return DS_ANS_WS_S_DONE;
				} else if (DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){   //0x0D){ //сравниваем его с концом строки
					*DS_status_ = DS_Type_command_new;
					//	*DS_buf_start_ = DS_i + 1;
					//printf("return DS_ANS_WS_S_DONE \r\n"); //v
					//*DS_maxCurrPoints_ = *DS_WSpoint_counter_; //количество обновляемых точек берем из счетчика точек
//					if (*DS_maxCurrPoints_ < *DS_WSpoint_counter_){*DS_maxCurrPoints_ = *DS_WSpoint_counter_;} //количество обновляемых точек берем из счетчика точек

					//Message("Chk",1);
					//print_0X4(CheckPixel_miscomparations,1);

					return DS_ANS_WS_S_DONE;  // расшифровка закончена
					//continue;
				}*/

				if (DS_End_Of_Command_flag){
					*DS_status_ = DS_End_Of_Command_flag;
					return DS_ANS_WS_S_DONE;
				}

				/*if (*DS_WSpoint_counter_ >= WS2812_IO_FRAMEDATA_PIXELS){
					*DS_status_ = DS_Type_command_new;
					*DS_buf_start_ = DS_i + 1;
					Error_Message_full('S', 'x',0, 0, 3,DS_i, DS_current_char);
					return DS_ERR_TOO_MANY_SYMBOLS;
				}*/

				DS_a8 = DS_chartoint (DS_current_char);
				//printf(" DS_Type_command_WS_S0 DS_a8 = %d\r\n", DS_a8); //v
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					//DS_current_char = (*DS_RGB_counter_)? ' ': DS_current_char;
					//*DS_RGB_counter_ = 2;
					if (*DS_RGB_counter_){
						goto S0P_Error_Unexpected_Symbol;
					} else if (DS_current_char == 'G') { //если команда на пропуск точки
						*DS_WSpoint_counter_ += 1;	//текущий принимаемый номер точки WS
						goto S0P_Next_Loop_label;
					//	*DS_RGB_counter_ = 0;		//номер символа в точке
					//	continue;//выход на новый виток
					} else if(DS_current_char == '>'){ //повторить предыдущее действие, исключая пропуск
						*DS_RGB_counter_ = 5;
						DS_Flag_Register = DS_Status_backup;
						DS_Repeater_set = 4;
					} else if (DS_current_char == '^'){//повторить предыдущее действие, исключая пропуск, если это SPTMR - то со сдвигом адресации на 1 к концу
						*DS_RGB_counter_ = 5;
						if (DS_Status_backup & 0x40){
							*DS_Param_ +=1;
						}
						DS_Flag_Register = DS_Status_backup;
						DS_Repeater_set = 8;
					} else if (DS_current_char == 'V'){//повторить предыдущее действие, исключая пропуск, если это SPTMR - то со сдвигом адресации на 1 к началу
						*DS_RGB_counter_ = 5;
						if ((DS_Status_backup & 0x40) && (*DS_Param_ & 0x0FFF)>1){
							*DS_Param_ -=1;
						}
						DS_Flag_Register = DS_Status_backup;
						DS_Repeater_set = 2;
					} else if (DS_current_char == 'H') { //если команда на все единицы для точки (белый)
						*DS_RGB_counter_ = 5;		//номер символа в точке
						*DS_Param_ = 0x00FFFFFF;				//значение цвета точки в RGB - временное хранилище
						//continue;//выход на новый виток
					} else if (DS_current_char == 'L') { //если команда на все нули для точки (чёрный)
						*DS_RGB_counter_ = 5;		//номер символа в точке
						*DS_Param_ = 0;				//значение цвета точки в RGB - временное хранилище
						//continue;//выход на новый виток
					} else if(DS_current_char == 'S'){ //обменяться цветами
						*DS_Param_ = 3;			//поменять текущий и целевой
						goto S0P_Next_Loop_Swap_Pixel_label;
					//	DS_Flag_Register |= 0x40;
					//	*DS_RGB_counter_ = 2;		//номер символа в точке
					//	continue;//выход на новый виток
					} else if(DS_current_char == 'P'){ //скопировать цвет в целевой пиксель
						*DS_Param_ = 2;			//скопировать из текущего в целевой
						goto S0P_Next_Loop_Swap_Pixel_label;
					//	DS_Flag_Register |= 0x40;
					//	*DS_RGB_counter_ = 2;		//номер символа в точке
					//	continue;//выход на новый виток
					} else if(DS_current_char == 'T'){ //скопировать цвет из целевого пикселя
						*DS_Param_ = 1;			//скопировать из целевого в текущий
						goto S0P_Next_Loop_Swap_Pixel_label;
					//	DS_Flag_Register |= 0x40;
					//	*DS_RGB_counter_ = 2;		//номер символа в точке
					//	continue;//выход на новый виток
					} else if(DS_current_char == 'R'){ //скопировать цвет из целевого пикселя
						*DS_Param_ = 8;			//умножить цвета
						goto S0P_Next_Loop_Swap_Pixel_label;
					//	DS_Flag_Register |= 0x40;
					//	*DS_RGB_counter_ = 2;		//номер символа в точке
					//	continue;//выход на новый виток
					} else if(DS_current_char == 'M'){ //объединить значения цветов пикселей по принципу OR
						*DS_Param_ = 5;			//скопировать из целевого в текущий
S0P_Next_Loop_Swap_Pixel_label:
						DS_Flag_Register |= 0x40;
						*DS_RGB_counter_ = 2;		//номер символа в точке
						continue;//выход на новый виток
					} else if (DS_current_char == 'O') { //перепрыгнуть на новый пиксел
						DS_Flag_Register |= 0x08;
						*DS_RGB_counter_ = 3;		//номер символа в точке
						CRC_sum = *DS_Param_;		//сохраняем значение параметра
						*DS_Param_ = 0;
						continue;//выход на новый виток
					} else if (DS_current_char == '*') { //Многоповтор
						DS_Flag_Register = DS_Status_backup;
						*DS_RGB_counter_ = 4;		//номер символа в точке
						DS_Repeater_set = (DS_Repeater_set & 0x10)? 1: DS_Repeater_set|1;
						//DS_Repeater_set |= 1; //поднимаем флаг многоповтора
						continue;//выход на новый виток
					} else if (DS_current_char == '?') { //проверить совпадение с цветом
						DS_Flag_Register |= 0x10;
S0P_Next_Loop_label:
						*DS_RGB_counter_ = 0;		//номер символа в точке
						continue;//выход на новый виток
					} else if (DS_current_char == ',') { //просто запятая, чтобы разделять цвета
						goto S0P_Next_Loop_label;
					//	*DS_RGB_counter_ = 0;		//номер символа в точке
					//	continue;//выход на новый виток
					} else {
S0P_Error_Unexpected_Symbol:
						Error_Message_full('S', 'x',0, 0, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
				} else { //(DS_a8 != 0xFF) //конец "если принятый символ не цифровой": если тру, дальше прохода не дожно быть

						//основная расшифровка
						//добавляем свежеполученный символ в параметр
						*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);

				}

					//отсчитываем 6 цифровых символов после P
					*DS_RGB_counter_ += 1;

					if (*DS_RGB_counter_ >= 6){ //когда случилось 6 цифровых символов - распознавание и помещение в память буфера
						//теперь в *DS_Param_ находится значение цвета точки WS
						if (*DS_WSpoint_counter_ >= WS2812_IO_FRAMEDATA_PIXELS){
							*DS_status_ = DS_Type_command_new;
							//	*DS_buf_start_ = DS_i + 1;
							Error_Message_full('S', 'x',0, 0, 3,DS_i, DS_current_char);
							return DS_ERR_TOO_MANY_SYMBOLS;
						}/**/
						if (DS_Flag_Register & 8){
							*DS_WSpoint_counter_ = *DS_Param_;
							*DS_Param_ = CRC_sum;
							DS_Flag_Register &= ~0x08;
						} else {
							DS_Status_backup = DS_Flag_Register;
							if (DS_Repeater_set & 1) {
								DS_Repeat_Count = *DS_Param_ & 0xFF;
								*DS_Param_ >>= 8;
								DS_Repeater_set &= ~1;
								if (DS_Repeat_Count < 2) {goto S0P_Skip_Any_Pixel_Action_Label;}
								if (DS_Repeater_set > 1) {// значит, перед этой штукой было ">", "^" или "v"
									DS_Repeater_set |= 0x10;//в следующий раз сбрасываем звёздочку
									//чтобы запись ">*05" читалась как "повторить ЕЩЁ 5 раз", а "*05" как "всего 5 копий действия"
									//DS_Repeat_Count = DS_Repeat_Count - 1;
									if ((DS_Repeater_set & 2) && (DS_Flag_Register & 0x40) && (*DS_Param_ & 0x0FFF)>1){
										*DS_Param_ -=1;
									} else if ((DS_Repeater_set & 8) && (DS_Flag_Register & 0x40) && ((*DS_Param_ & 0x0FFF)<WS2812_IO_FRAMEDATA_PIXELS)){
										*DS_Param_ +=1;
									} //декремент/инкремент параметра осуществляется после первого прохода функции, так что надо это сделать заранее
								}
								//Port_send_char(0x0d,DEBUG_PORT_OUT);
								//USB_main_COM_react();
								DS_Repeat_Count = ((*DS_WSpoint_counter_ + DS_Repeat_Count - 1) <= DS_TotalPoints)? (DS_Repeat_Count - 2): (DS_TotalPoints - *DS_WSpoint_counter_ - 1);
								//DS_Repeat_Count -= 1;// поскольку функции работы с массивом цветов осуществляют один проход в любом случае, мы записываем сколько раз дополнительно повторить этот действие помимо самого первого
								// запись "*00" не осуществляет повтор, а запись ">*00" один повтор осуществит, т.к. в момент чтения ">" система ещё не знает, сколько раз ей повторять
							}
							if (DS_Flag_Register & 0x40){
								if ((*DS_Param_ & 0x0FFF)>= WS2812_IO_FRAMEDATA_PIXELS){
									*DS_Param_ = (*DS_Param_ &	0xFFF000) + WS2812_IO_FRAMEDATA_PIXELS-1;
								}
								//DS_Param & 0x00000FFF - номер пиксела
								//DS_Param & 0x0000F000 - номер вывода
								//DS_Param & 0x00070000 - режим работы функции
								if (*DS_Param_ & 0x00080000){
									WS2812_framedata_MultiplyPixel_RGB(WS2812_IO_framedata_,*DS_comm_num_,(*DS_Param_ >> 12)& 0x07,*DS_WSpoint_counter_, *DS_Param_ & 0x0FFF, DS_Repeat_Count, DS_Repeater_set);
								} else {
									WS2812_framedata_SwapPixel(WS2812_IO_framedata_,*DS_comm_num_,(*DS_Param_ >> 12)& 0x07,*DS_WSpoint_counter_, *DS_Param_ & 0x0FFF,*DS_Param_ >> 16, DS_Repeat_Count, DS_Repeater_set);
								}
								DS_Flag_Register &= ~0x40;  //сброс флага смены пикселов
							} else if (DS_Flag_Register & 0x10){
								CheckPixel_miscomparations += WS2812_framedata_CheckPixel(WS2812_IO_framedata_, *DS_comm_num_, *DS_WSpoint_counter_, *DS_Param_, DS_Repeat_Count);
								DS_Flag_Register &= ~0x10;
							} else {
								WS2812_framedata_setPixel_RGB (WS2812_IO_framedata_, *DS_comm_num_, *DS_WSpoint_counter_, *DS_Param_, DS_Repeat_Count);
								//DS_Flag_Register &= ~0x10;
							}/**/
							*DS_WSpoint_counter_ += (1 + DS_Repeat_Count);	//текущий принимаемый номер точки WS
							//DS_Repeater_set = 0;
							if ((*DS_maxCurrPoints_ < *DS_WSpoint_counter_) &&  (*DS_WSpoint_counter_ <= DS_TotalPoints)){*DS_maxCurrPoints_ = *DS_WSpoint_counter_;} //количество обновляемых точек берем из счетчика точек
						}
S0P_Skip_Any_Pixel_Action_Label:
						DS_Repeat_Count = 0;
						*DS_RGB_counter_ = 0;		//номер символа в точке
						//*DS_Param_ = 0;				//значение цвета точки в RGB - временное хранилище

					}
					//*DS_buf_start_ = DS_i + 1; //дальше начало анализа со следующего символа
				break; //DS_Type_command_WS_S0:

//SR---------------------------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_SR: //принято "SR", ожидаем букву Q, если пришла не она - ошибка
								//*DS_comm_num_ используется как флаг состояния приема: ==4 - ждём букву Q, ==5 - ждём значение параметра Q
								//*DS_Param_ - значение параметра Q, сколько точек принимаем
								//*DS_WSpoint_counter_ - номер текущей принимаемой точки, счетчик для расшифровки параметра P

								/*if (*DS_comm_num_ == 4) { //должна придти буква Q

									if(DS_current_char == 'Q'){
										*DS_comm_num_ = 5; //см. описание в начале case DS_Type_command_SR:
										*DS_Param_ = 0;				//*DS_Param_ - значение параметра Q, сколько точек принимаем
									} else { //если не Q - значит ошибка
										Error_Message_full('S', 'R',0, 0, 1,DS_i, DS_current_char);
										return DS_ERR_UNEXPEXTED_SYMBOL;
									}
								} else if (*DS_comm_num_ == 5)  { //должна придти цифра-значение параметра Q - количество принимаемых точек
								*/	DS_a8 = DS_chartoint (DS_current_char);
									if (DS_a8 == 0xFF){ //если принятый символ не цифровой
										if (DS_current_char == 'P'){
											if (*DS_Param_ > WS2812_IO_FRAMEDATA_PIXELS){
												Error_Message_full('S', 'R',0, 0, 2,DS_i, DS_current_char);
												*DS_Param_ = WS2812_IO_FRAMEDATA_PIXELS;
											}
											*DS_WSpoint_counter_ = 0;		//*DS_WSpoint_counter_ - номер текущей принимаемой точки, счетчик для расшифровки параметра P
											*DS_maxCurrPoints_ = *DS_Param_; //количество обновляемых точек берем из счетчика точек
											*DS_Param_ = *DS_Param_ * 24; //теперь в параметре количество байт в массиве, а не число точек
											*DS_status_ = DS_Type_command_RAW;
											goto RAW_Processing;
										}
										else if(DS_current_char != 'Q') { //ждем только P или цифру для Q, если не P - значит ошибка
											//Q проглатываем, всё равно после SR у нас дожно идти значение параметра P
											Error_Message_full('S', 'R',0, 0, 1,DS_i, DS_current_char);
											return DS_ERR_UNEXPEXTED_SYMBOL;
										}
									} else{  //если символ цифровой
										//если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
										*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
									}
								//}

				break; //DS_Type_command_SR:

//RAW---------------------------------------------------------------------------------------------------------------------------------------------

			case DS_Type_command_RAW: //принято "SR", значение Q, умноженное на 24, записано в *DS_Param_,
				// дальше просто пихаем полученные байты в массив, начиная с нуля
				//*DS_WSpoint_counter_ используется как счетчик текущего принимаемого байта
				//потенциал для ускорения: вытащить это в отдельный цикл из общего большого цикла ++DS_i


RAW_Processing:

				while ((*DS_WSpoint_counter_ < *DS_Param_)&&(DS_i <= *DS_buf_counter_)){
					DS_i +=1;
					WS2812_IO_framedata_[*DS_WSpoint_counter_] = DS_buffer_[DS_i];
					*DS_WSpoint_counter_ +=1;
				}

//				WS2812_IO_framedata_[*DS_WSpoint_counter_] = DS_current_char;
//				++*DS_WSpoint_counter_;
				if (*DS_WSpoint_counter_ >= *DS_Param_) {
					*DS_status_ = DS_Type_command_new;
					//	*DS_buf_start_ = DS_i + 1;
					return DS_ANS_WS_S_DONE;  // расшифровка закончена
				}

				break; //DS_Type_command_RAW:
//-------------------------------------------------------------------------------------------------------------------------------
//file system functions
#if defined (FS_ENABLED)
			case DS_Type_command_F:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF; //запоминаем номер текущей G-команды
					*DS_Param_ = 0;
					//	*DS_RGB_counter_ = 0;
					if (*DS_comm_num_ == 0){ //войти/выйти из режима чтения / записи файлов
						if (DS_current_char == 'P'){// если прием номера s-кода закончен (буква P), определяем цифру
							*DS_comm_num_ = DS_ANS_FILEMODE_ON;
							*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
							//	*DS_buf_start_ = DS_i + 1;
						//	*DS_Param_ = 0;
						} else {
							Error_Message_full('F', 0,0, 0, 1,DS_i, DS_current_char);;
							return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
						}
					} else if (LED_control_type & 128) {//работает только в режиме файлов
						switch (*DS_comm_num_) {

							case 1:	//список папок/файлов в корне 0 - первая, 1 - следующая по порядку в памяти
								if (DS_current_char == 'P'){// если прием номера s-кода закончен (буква P), определяем цифру
									*DS_comm_num_ = 0;
									*DS_status_ =  DS_Type_command_FILEWORKS_CONT;
									//	*DS_buf_start_ = DS_i + 1;
								//	*DS_Param_ = 0;
									*DS_RGB_counter_ = (SD_Volume_Exists>>2) - 1;
								} else {
									Error_Message_full('F', 1,0, 0, 1,DS_i, DS_current_char);
									return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
								}
								break;
							case 2: //работать в подпапке
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
							//	*DS_RGB_counter_ = 0;
								*DS_comm_num_ = DS_ANS_FILEMODE_SETDIR;
								for (DS_a8 = 0; DS_a8 < FILE_NAME_MAX_LENGTH; DS_a8++) { //очистка fname - корневая папка (0:/hcd)
									fname[DS_a8] = '\0';
								}
								/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
									*DS_status_ =  DS_Type_command_new;
									return DS_ANS_FILEMODE_SETDIR;
								} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
									*DS_status_ = DS_Type_command_comment;*/
								if (DS_End_Of_Command_flag){
									*DS_status_ = DS_End_Of_Command_flag;

									return DS_ANS_FILEMODE_SETDIR;
								} else if (DS_current_char == 'P') {
									*DS_status_ =  DS_Type_command_FILE_SETDIR;
								} else { //если не конец строки, файла или комментарий - значит ошибка
									Error_Message_full('F', 0x2,0, 0, 1,DS_i, DS_current_char);
									return DS_ERR_UNEXPEXTED_SYMBOL;
								}
								break;
							case 3:
							case 4:
							case 5:
							case 6:
							case 7:
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
							//	*DS_RGB_counter_ = 0;

								for (DS_a8 = 0; DS_a8 < FILE_NAME_MAX_LENGTH; DS_a8++) { //очистка fname - корневая папка (0:/hcd)
									fname[DS_a8] = '\0';
								}
								if (DS_current_char == 'P') {
									*DS_status_ =  DS_Type_command_FILE_SETDIR;
									*DS_comm_num_ = *DS_comm_num_ + DS_ANS_FILEMODE_SETDIR - 2; //сдвиг указателя
								} else { //если не конец строки, файла или комментарий - значит ошибка
									Error_Message_full('F', *DS_comm_num_,0, 0, 1,DS_i, DS_current_char);
									return DS_ERR_UNEXPEXTED_SYMBOL;
								}
								break;
							case 9: //создать временный файл
								*DS_status_ =  DS_Type_command_new;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								return DS_ANS_FILEMODE_NEWFILE;

								break;
							case 0x10://прочитать кусок файла F10 [Q<pionter>]P/N<number of bytes>
							case 0x11://записать кусок в файл F11 [Q<pointer>]P/H<data>
								*DS_comm_num_ = *DS_comm_num_ - 0x10;//сдвиг указателя
								//	*DS_buf_start_ = DS_i + 1;
								CRC_sum = CRC32_INIT;
								DS_Status_backup=0;
								*DS_WSpoint_counter_ = 0; //количество символов
								if (DS_current_char == 'Q') {
									*DS_status_ =  DS_Type_command_FILE_READ_SET;
								//	*DS_Param_ = 0; //назначим в следующей части
									*DS_comm_num_ += DS_Type_command_FILE_READ_PART; //указатель на следующий сегмент
								} else if (DS_current_char == 'P') {
									*DS_status_ =  DS_Type_command_FILE_READ_PART + *DS_comm_num_;
									//DS_Flag_Register &=~1; //in HEX
								//	*DS_RGB_counter_ = 0; //строчка в байтах
									*DS_Param_ = FileWorksPTR;//продолжаем читать/писать с прошлого места
								} else if (DS_current_char == 'N' && *DS_comm_num_==0) { //только для чтения
									*DS_status_ =  DS_Type_command_FILE_READ_PART;
									DS_Flag_Register |=1; //in DEC
									*DS_Param_ = FileWorksPTR;//продолжаем читать с прошлого места
								} else if (DS_current_char == 'H') {
									*DS_status_ =  DS_Type_command_FILE_READ_PART + *DS_comm_num_;
									//DS_Flag_Register &=~1; //in HEX
									*DS_RGB_counter_ = 1; //строчка в HEXкодах
									*DS_Param_ = FileWorksPTR;//продолжаем читать/писать с прошлого места
								} else { //если не конец строки, файла или комментарий - значит ошибка
									Error_Message_full('F', (8+*DS_comm_num_),0, 0, 1,DS_i, DS_current_char);
									return DS_ERR_UNEXPEXTED_SYMBOL;
								}
								break;
							case 0x12:
							case 0x13:
								if (DS_current_char == 'P'){// если прием номера s-кода закончен (буква P), определяем цифру
									*DS_comm_num_ = (*DS_comm_num_ - 0x12) + DS_ANS_FILEMODE_WRITE_CRC;
									*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
									//	*DS_buf_start_ = DS_i + 1;
								//	*DS_Param_ = 0;
								} else {
									Error_Message_full('F', *DS_comm_num_,0, 0, 1,DS_i, DS_current_char);
									return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
								}
								break;
							default:
								Error_Message_full('F', 0xFF,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
								break;
						}
					} else {
						//Message ("Working with files N/A\r\0",DS_Channel_Select - 1);
						Error_Message_full('F', 0xFF,0, 0, 5,0, 0);
						*DS_status_ = DS_Type_command_comment;
					//	*DS_RGB_counter_ = 0;		//номер символа в точке
						//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
						return DS_ANS_WS_S_DONE;

					}

				} else {
					//если символ был цифровой - заносим свежую цифру в сразу в параметр
					*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
				}
				break;/*DS_Type_command_F*/

#endif
//-------------------------------------------------------------------------------------------------------------------------------
//тут у нас паузы
			case DS_Type_command_G: // ждем номер для G-команды
				DS_a8 = DS_chartoint (DS_current_char);
				//printf ("DS_Type_command_G, DS_current_char = %d, DS_a8 = %d\r\n", DS_current_char, DS_a8);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой

					*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF; //запоминаем номер текущей G-команды
					*DS_Param_ = 0;
					switch (*DS_comm_num_) {
						case 0: //положение сервомашинок
							//DS_Flag_Register &= ~(0xD); //второй параметр положительный, абсолютный 1+(4+8)
							//	*DS_buf_start_ = DS_i + 1;
						//	*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
							*DS_WSpoint_counter_ = 0;
						//	*DS_Param_ = 0;
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_SET_ALL; //дальше ждём строчку из 8 блоков по 4 hex цифры
							} else if (DS_current_char == 'Q'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_SET; //дальше будет установка конкретного сервы
								*DS_comm_num_ = 0;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							} else {
								//if (DebugInfoOutFlag){
								Error_Message_full('G', 0,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 0 );//printf("DS_Type_command_G0,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//	Error_Message_data(1,DS_i, DS_current_char);
									//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char); //v
								//}
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P

							}
							break;
						case 1:
							if (DS_current_char == 'P'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = DS_ANS_SERVO_WATCH;
								*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
							} else {
								Error_Message_full('G', 1,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 1 );//printf("DS_Type_command_G1,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//Error_Message_data(1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
						/*case 2:
							if (DS_current_char == 'Q'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_ADD; //дальше будет установка конкретного сервы
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
								*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
								*DS_WSpoint_counter_ = 0;
								*DS_comm_num_ = 0;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							} else {
								Error_Message_full('G', 2,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 2 );//printf("DS_Type_command_G0,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//Error_Message_data(1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;*/
						case 4: //пауза с обновлением
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру (hex)
								*DS_comm_num_ = 0;
								*DS_status_ =  DS_Type_command_Pause;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								//DS_Flag_Register &=~1; //in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру (dec)
								*DS_comm_num_ = 0;
								*DS_status_ =  DS_Type_command_Pause;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC

							} else if (DS_End_Of_Command_flag){
								*DS_status_ = DS_End_Of_Command_flag;

								if (DS_Flag_Register&0x20){
									numFileForButton[ButtonNomberCarrier]=0;
									FastCommandForButton[ButtonNomberCarrier]=FC_PAUSE_END_G4;
									return DS_ANS_WS_S_DONE;
								}
								return DS_ANS_PAUSE; //исполняем паузу
							} else {
								Error_Message_full('G', 4,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
						break;
						case 5: //пауза без обновления
						case 6: //пауза до определнного момента с начала файла
						case 7: // case 7 - то же, что case 6, но без вывода светодиодов
							*DS_comm_num_ = *DS_comm_num_ - 4; //G4->0...G7->3
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_Pause;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								//DS_Flag_Register &=~1; //in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_Pause;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', (4 + *DS_comm_num_),0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', (4 + *DS_comm_num_) );//printf("DS_Type_command_G5,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//Error_Message_data(1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
						break;

						case 8: //установка нового значения текущего времени от начала чтения файла
							//или принудительный сдвиг тайминга файла к определнному авремени
							//но конечно файл назад не отматывается, если откатить тайминг назад мы просто удлинним паузу
							*DS_comm_num_ = 0;
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_Set_Absolut;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								//DS_Flag_Register &=~1; // parameter will be in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_Set_Absolut;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC
							} else {
								//if (DebugInfoOutFlag){
								Error_Message_full('G', 8,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 8 );//printf("DS_Type_command_G8,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//	Error_Message_data(1,DS_i, DS_current_char);
									//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char); //v
								//}
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}

						break;
						case 9://принудительный выход из паузы
							*DS_status_ =  DS_Type_command_new;
							//	*DS_buf_start_ = DS_i + 1;
						//	*DS_Param_ = 0;
							if (DS_Flag_Register&0x20){
								FastCommandForButton[ButtonNomberCarrier]=FC_PAUSE_END_G9;
								return DS_ANS_WS_S_DONE;
							}
							return DS_ANS_RESET_PAUSE;
						break;
						case 0xA:
						case 0x25: //эмуляция кнопок
							//G25 - перейти к файлу по кнопке, если без P и Q - ошибка
							if(DS_current_char == 'Q'){
								*DS_status_ =  DS_Type_command_BUTTON_EMULATE; //_GOTO_FILE;
								ButtonNomberCarrier = 0;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
								//*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
							//	*DS_WSpoint_counter_ = 0;
								*DS_comm_num_ = 0;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							} else { //если не P или Q - значит ошибка
								Error_Message_full('G', 0x25,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;
						case 0x26: //назначить паузу для сброса кнопок
						case 0x27: //G27 - время ожидания при нажатии в режме нескольких кнопок
						case 0x28: //команда SI P<>/ SI N<>
						case 0x29: //начало текущего кадра анимации в мультикадре
						case 0x30://частота кадров в мультикадре
						case 0x31://количество пикселей в кадре мультикадра
						case 0x32://количество кадров в анимации мультикадра
						case 0x35: //G35 количество кадров для проигрывания, 0 - бесконечно
Setting_Of_Variables:	//case 0x2B = M91, 0x16 = M46

							//DS_Flag_Register &= ~(0xD);

							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_MULTI_SET;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								//DS_Flag_Register &=~1; //in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_MULTI_SET;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', *DS_comm_num_,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
						/*case 0x30://частота кадров в мультикадре
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 1;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register &=~1; //in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 1;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', 0x30,0, 0, 1,DS_i, DS_current_char);
							//	Error_Message_Command('G', 0x30);//	printf("DS_Type_command_G30,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//Error_Message_data(1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
						case 0x31://количество пикселей в кадре мультикадра
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 2;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register &=~1; //in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 2;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', 0x31,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 0x31 );//	printf("DS_Type_command_G31,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//Error_Message_data(1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
						case 0x32://количество кадров в анимации мультикадра
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 3;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register &=~1; //in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 3;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', 0x32,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 0x32 );//printf("DS_Type_command_G32,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//Error_Message_data(1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
						case 0x35: //G35 количество кадров для проигрывания, 0 - бесконечно
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 4;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register &=~1; //in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = 4;
								*DS_status_ =  DS_Type_command_MULTI_SET;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								DS_Flag_Register |=1; //in DEC
							} else {
								//if (DebugInfoOutFlag){
								Error_Message_full('G', 0x35,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 0x35 );//printf("DS_Type_command_G35,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//	Error_Message_data(1,DS_i, DS_current_char);
									//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char); //v
								//}
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
							*/
						case 0x36: //G36 запуск или остановка проигрывания
							if (DS_current_char == 'P'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = DS_ANS_MULTI_PLAY;
								*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
							} else {
								Error_Message_full('G', 0x36,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
						case 0x37:
							ButtonNomberCarrier = AFTER_FAST_ANIM_ACIONS_ADRESS;
							*DS_WSpoint_counter_ = 0;
							*DS_comm_num_ = 0;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							goto Set_Button_Parameter_Type_Label;
							break;
						case 0x43: //параметры серв; Servo_MinPos
						case 0x44: //параметры серв; Servo_MaxPos
						case 0x45: //параметры серв; Servo_Resolution
						case 0x50: //параметры серв; Servo_Period
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								//*DS_comm_num_ = 1; //флаг Servo_MinPos
								//DS_Flag_Register &=~1; // parameter will be in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
								//*DS_comm_num_ = 1; //флаг Servo_MinPos
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', *DS_comm_num_,0, 0, 1,DS_i, DS_current_char);
								//Error_Message_Command('G', 0x43 );//printf("DS_Type_command_G43,   DS_ERR_UNKNOWN_SYMBOL  "); //v
								//	Error_Message_data(1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
/*						case 0x44: //параметры серв; Servo_MaxPos
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								//*DS_comm_num_ = 2; //флаг Servo_MaxPos
								DS_Flag_Register &=~1; // parameter will be in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								//*DS_comm_num_ = 2; //флаг Servo_MaxPos
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', 0x44,0, 0, 1,DS_i, DS_current_char);
								Error_Message_Command('G', 0x44 );//printf("DS_Type_command_G44,   DS_ERR_UNKNOWN_SYMBOL  "); //v
									Error_Message_data(1,DS_i, DS_current_char);
									//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char); //v
								//}
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
						case 0x45: //параметры серв; Servo_Resolution
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								//*DS_comm_num_ = 3; //флаг Servo_Resolution
								DS_Flag_Register &=~1; // parameter will be in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								//*DS_comm_num_ = 3; //флаг Servo_Resolution
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', 0x45,0, 0, 1,DS_i, DS_current_char);
								Error_Message_Command('G', 0x45 );//printf("DS_Type_command_G45,   DS_ERR_UNKNOWN_SYMBOL  "); //v
									Error_Message_data(1,DS_i, DS_current_char);
									//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char); //v
								//}
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;*/
						case 0x49: //параметры ШИМ серв по умлчанию
							*DS_status_ =  DS_Type_command_new;
							//	*DS_buf_start_ = DS_i + 1;
						//	*DS_Param_ = 0;
							*DS_comm_num_ = 0; //флаг умолчания
							return DS_ANS_SERVO_SET_PARAM;

							break;
/*						case 0x50: //параметры серв; Servo_Period
							if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								//*DS_comm_num_ = 4; //флаг Servo_Period
								DS_Flag_Register &=~1; // parameter will be in HEX
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_status_ =  DS_Type_command_SERVO_PARAMS;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;
								//*DS_comm_num_ = 4; //флаг Servo_Period
								DS_Flag_Register |=1; //in DEC
							} else {
								Error_Message_full('G', 0x50,0, 0, 1,DS_i, DS_current_char);
								Error_Message_Command('G', 0x50 );//printf("DS_Type_command_G50,   DS_ERR_UNKNOWN_SYMBOL  "); //v
									Error_Message_data(1,DS_i, DS_current_char);
									//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char); //v
								//}
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;*/
						default:
							Error_Message_full('G', 0xFF,0, 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P

						break;
					}



				} else {
					//если символ был цифровой - заносим свежую цифру в сразу в параметр
					*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
				}

				break; //DS_Type_command_G:
//---------------------------------------------------------------------------------------------------------------------------------------
//M...
			case DS_Type_command_M:
				DS_a8 = DS_chartoint (DS_current_char);
				//printf ("DS_Type_command_M, DS_current_char = %x, DS_a8 = %d\r\n", DS_current_char, DS_a8);

				if (DS_a8 == 0xFF){ //если принятый символ не цифровой

					*DS_comm_num_  = (uint8_t) *DS_Param_ % 0xFF; //запоминаем номер текущей M-команды
					//printf("(M...) DS_i= %d, *DS_comm_num_ = 0x%02X\r\n", DS_i, *DS_comm_num_); //v
				//	*DS_RGB_counter_ = 0;
					*DS_Param_ = 0;
					switch (*DS_comm_num_) {
					case 0x02:	//М2 - конец файла

						/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
							*DS_status_ =  DS_Type_command_new;
						} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;*/
						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
						}  else { //если не конец строки, файла или комментарий - значит ошибка
							Error_Message_full('M', 0x2,0, 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL;
						}
						//*DS_buf_start_ = DS_i + 1;
						*DS_Param_ = 1;
						return DS_ANS_REPEAT_FILE;
						break;
					case 0x03: //M3 включение серв (всех вместе или по одному) 0000 0011
					case 0x04: //M4 - включение светодиодных выводов в гибридном режиме, всех или по одному 0000 0100
					case 0x05: //M5 - выключить серводвигатели 0000 0101
					case 0x10: //M10 - включить отдельную ногу на постоянку 0001 0000
					case 0x11: //M11 - выключить отдельную ногу на постоянке 0001 0001
						//DS com Num хранит номер команды
						//*DS_comm_num_ = 0x03;
					//	*DS_RGB_counter_ = 0;
						//	*DS_buf_start_ = DS_i + 1;
						/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
							*DS_status_ =  DS_Type_command_new;
							*DS_Param_ = 0xFFFF; //индикатор, что все ноги участвуют
							return DS_ANS_SERVO_DISABLE;
						} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;*/
						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
							*DS_Param_ = 0xFFFF; //индикатор, что все ноги участвуют
							return DS_ANS_SERVO_DISABLE;
						} else if (DS_current_char == 'P') {
							*DS_status_ =  DS_Type_command_SERVO_ENABLE;
						//	*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
						} else { //если не конец строки, файла или комментарий - значит ошибка
							Error_Message_full('M', *DS_comm_num_,0, 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL;
						}
						break;

						case 0x06: //M6 - назначить тип светодиодов

							if(DS_current_char == 'P'){
								*DS_status_ =  DS_Type_command_SET_LED_TYPE; //_set LEDs;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							//	*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
								*DS_comm_num_ = 1;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							}
							else { //если не P  значит ошибка
								Error_Message_full('M', 0x6,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;
/*						case 0x10: //M10 - включить отдельную ногу на постоянку
							*DS_comm_num_ = 0x10;
							*DS_RGB_counter_ = 0;
							*DS_buf_start_ = DS_i + 1;
							if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
								//printf ("case M47; *DS_status_ = DS_Type_command_new;\r\n");
								*DS_status_ =  DS_Type_command_new;
								*DS_Param_ = 0xFFFF; //индикатор, что все ноги участвуют
								return DS_ANS_SERVO_DISABLE;
							} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
								*DS_status_ = DS_Type_command_comment;
								*DS_Param_ = 0xFFFF; //индикатор, что все ноги участвуют
								return DS_ANS_SERVO_DISABLE;
							} else if (DS_current_char == 'P') {
								*DS_status_ =  DS_Type_command_SERVO_ENABLE;
								*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							} else { //если не конец строки, файла или комментарий - значит ошибка
								Error_Message_full('M', 0x10,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}

							break;
						case 0x11: //M11 - выключить отдельную ногу на постоянке
							*DS_comm_num_ = 0x11;
							*DS_RGB_counter_ = 0;
							*DS_buf_start_ = DS_i + 1;
							if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
								*DS_status_ =  DS_Type_command_new;
								*DS_Param_ = 0xFFFF; //индикатор, что все ноги участвуют
								return DS_ANS_SERVO_DISABLE;
							} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
								*DS_status_ = DS_Type_command_comment;
								*DS_Param_ = 0xFFFF; //индикатор, что все ноги участвуют
								return DS_ANS_SERVO_DISABLE;
							} else if (DS_current_char == 'P') {
								*DS_status_ =  DS_Type_command_SERVO_ENABLE;
								*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							} else { //если не конец строки, файла или комментарий - значит ошибка
								Error_Message_full('M', 0x11,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}

							break;*/
						case 0x23: //выбрать рабочую папку
							//	*DS_buf_start_ = DS_i + 1;
						//	*DS_Param_ = 0;
						//	*DS_RGB_counter_ = 0; //позиция в имени
							for (DS_a8 = 0; DS_a8 < FILE_NAME_MAX_LENGTH; DS_a8++) { //очистка fname - корневая папка (0:/hcd)
								fname[DS_a8] = '\0';
							}
							/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
								*DS_status_ =  DS_Type_command_new;
								*DS_comm_num_ = 0;
								return DS_ANS_NORMALMODE_SETDIR;
							} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
								*DS_status_ = DS_Type_command_comment;*/
							if (DS_End_Of_Command_flag){
								*DS_status_ = DS_End_Of_Command_flag;
								*DS_comm_num_ = 0;
								return DS_ANS_NORMALMODE_SETDIR;
							} else if (DS_current_char == 'P') {
								*DS_status_ =  DS_Type_command_FILE_SETDIR;
								*DS_comm_num_ = DS_ANS_NORMALMODE_SETDIR; //сдвиг указателя
							} else { //если не конец строки, файла или комментарий - значит ошибка
								Error_Message_full('M', 0x23,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}

							break;
						case 0x24: //возобновить выполнение файла, паузы расконсервировать
						case 0x25: //приостановить выполнение файла, паузы законсервировать
							*DS_comm_num_ -= 0x24;// 0 - ON, 1 - off
							/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
								*DS_status_ =  DS_Type_command_new;
								if (DS_Flag_Register&0x20){
									FastCommandForButton[ButtonNomberCarrier]=FC_RESUME_FILE_M24+*DS_comm_num_;
									return DS_ANS_WS_S_DONE;
								}
								return DS_ANS_FILE_SUSPEND;
							} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
								*DS_status_ = DS_Type_command_comment;*/
							if (DS_End_Of_Command_flag){
								*DS_status_ = DS_End_Of_Command_flag;
								if (DS_Flag_Register&0x20){
									FastCommandForButton[ButtonNomberCarrier]=FC_RESUME_FILE_M24+*DS_comm_num_;
									return DS_ANS_WS_S_DONE;
								}
								return DS_ANS_FILE_SUSPEND;
								*DS_comm_num_ = 0;
							} else if (DS_current_char == 'P' || DS_current_char == 'H'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = DS_ANS_SET_M25_SWITCH_TIME - DS_ANS_SET_ABSOLUT_TIME;
								*DS_status_ =  DS_Type_command_Set_Absolut;
							} else if (DS_current_char == 'N'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = DS_ANS_SET_M25_SWITCH_TIME - DS_ANS_SET_ABSOLUT_TIME;
								*DS_status_ =  DS_Type_command_Set_Absolut;
								DS_Flag_Register |=1; //in DEC
							} else { //если не конец строки, файла или комментарий - значит ошибка
								Error_Message_full('M', *DS_comm_num_,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;
						case 0x45:
							CheckPixel_miscomparations = 0;

Finish_Command_On_The_Spot_Without_Reading_All_Line_Label:
							*DS_status_ = DS_Type_command_comment;
						/*	if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
								*DS_status_ =  DS_Type_command_new;*/
							if (DS_End_Of_Command_flag){
								*DS_status_ = DS_End_Of_Command_flag;
							}
							return DS_ANS_WS_S_DONE;
							break;/**/
						case 0x46: //M46 P<> - переназначить количество повторов текущего файла
							*DS_comm_num_ = 0x16;//отстраиваемся от G26...G35
							goto Setting_Of_Variables;
							break;
						case 0x47:	//М47 - повторить текущий файл с начала, после неё - конец строки, файла или комментарий
							/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
								*DS_status_ =  DS_Type_command_new;
								if (DS_Flag_Register&0x20){
									numFileForButton[ButtonNomberCarrier]=0;
									FastCommandForButton[ButtonNomberCarrier]=FC_REPEAT_FILE_M47;
									CommandArgForButton[ButtonNomberCarrier]=1;
									return DS_ANS_WS_S_DONE;
								}
								return DS_ANS_REPEAT_FILE;
							} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
								*DS_status_ = DS_Type_command_comment;*/
							if (DS_End_Of_Command_flag){
								*DS_status_ = DS_End_Of_Command_flag;
								if (DS_Flag_Register&0x20){
									numFileForButton[ButtonNomberCarrier]=0;
									FastCommandForButton[ButtonNomberCarrier]=FC_REPEAT_FILE_M47;
									CommandArgForButton[ButtonNomberCarrier]=1;
									return DS_ANS_WS_S_DONE;
								}
								return DS_ANS_REPEAT_FILE;
							} else if(DS_current_char == 'P' || DS_current_char == 'H') {
								*DS_status_ =  DS_Type_command_REPEAT_FILE;
								//DS_Flag_Register &=~1;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
							//	*DS_RGB_counter_ = 0;
							}else if(DS_current_char == 'N'){
								*DS_status_ =  DS_Type_command_REPEAT_FILE;
								DS_Flag_Register |=1;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
							//	*DS_RGB_counter_ = 0;
							} else { //если не конец строки, файла или комментарий - значит ошибка
								Error_Message_full('M', 0x47,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;
						case 0x90: //назначить стартовое значение для генератора псевдослучайных чисел
							if(DS_current_char == 'P' || DS_current_char == 'H'){
								*DS_status_ =  DS_Type_command_SET_RND_SEED;
								//DS_Flag_Register &=~1;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							//	*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
							}else if(DS_current_char == 'N'){
								*DS_status_ =  DS_Type_command_SET_RND_SEED;
								DS_Flag_Register |=1;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							//	*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
							/*} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
								*DS_status_ = DS_Type_command_comment;
								if (DS_Flag_Register&0x20){
									FastCommandForButton[ButtonNomberCarrier]=FC_RANDOM_SEED_TIMER_M90;
								} else {
									srand(CoGetOSTime());
								}
								return DS_ANS_WS_S_DONE;
							} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
								*DS_status_ = DS_Type_command_new;*/

							} else if (DS_End_Of_Command_flag){
								*DS_status_ = DS_End_Of_Command_flag;
								if (DS_Flag_Register&0x20){
									FastCommandForButton[ButtonNomberCarrier]=FC_RANDOM_SEED_TIMER_M90;
								} else {
									srand(CoGetOSTime());
								}
								return DS_ANS_WS_S_DONE;
							} else { //если не P, коммент или конец строки - значит ошибка
								Error_Message_full('M', 0x90,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;

						case 0x91: //M91 - заранее выбрать файл по M98 Q<>P<>
							*DS_comm_num_ = 0x2B;//отстраиваемся от G26...G35
							goto Setting_Of_Variables;
							break;
						case 0x86: //M86 P<> - вкл/выкл реагирование на комбинацию.
							if (DS_current_char == 'P'){// если прием номера s-кода закончен (буква P), определяем цифру
								*DS_comm_num_ = DS_ANS_BUTTON_MULTI_SET;
								*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;
							} else if(DS_current_char == 'Q'){
								*DS_status_ = DS_Type_command_ACTIVATE_BUTTON;
								*DS_comm_num_ = 3;
								//ButtonEncoder = 0;
								//*DS_Param_ - кнопки
							} else {
								Error_Message_full('M', 0x86,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNKNOWN_SYMBOL; //символ не цифровой и не P
							}
							break;
					//	case 0x93: //M93 - заглушка для M96, параметр кнопок чтобы отстроиться от файлов
					//		DS_Flag_Register |=0x10; //указываем, что команда расширенная
					//		*DS_comm_num_ = 0x96; //прикидываемся M96
						case 0x96:  //M96 - перейти к другому файлу по кнопке, если без P и Q - ошибка
					//	case 0x97:  //M97 - активация/деактивация кнопок M97 Q<кнопка> P<0-выкл, 1-вкл.>
							//uint32_t *DS_Param_ - для хранения номера (имени) файла, в который переходить
							//uint8_t *DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
							//uint16_t *DS_WSpoint_counter_ - для хранения номера кнопки, которую кодируем
							//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2 = есть P, читаем Q; 3 = есть Q, читаем P

							/*if(DS_current_char == 'P'){
								*DS_status_ =  DS_Type_command_SET_BUTTON;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							//	*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
								*DS_WSpoint_counter_ = 0;
								*DS_comm_num_ = 1;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							}
							else*/ if(DS_current_char == 'Q'){
								*DS_status_ =  DS_Type_command_Set_Q_number;
								ButtonNomberCarrier = 0;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							//	*DS_RGB_counter_ = 0;
							//	*DS_RGB_counter_ = DS_Type_command_SET_BUTTON + *DS_comm_num_ - 0x96;		//*DS_RGB_counter_ - ссылка
								*DS_WSpoint_counter_ = 0;
								*DS_comm_num_ = 0;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							}
							else { //если не P или Q - значит ошибка
								Error_Message_full('M', *DS_comm_num_,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;


						/*case 0x97:  //M97 - активация/деактивация кнопок M97 Q<кнопка> P<0-выкл, 1-вкл.>
							//uint32_t *DS_Param_ - акивируем или деактивируем прерывание по кнопке
							//uint8_t *DS_RGB_counter_ -
							//uint16_t *DS_WSpoint_counter_ - для хранения номера кнопки, которую кодируем
							//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2 = есть P, читаем Q; 3 = есть Q, читаем P
								//*DS_comm_num_ используется как байт с флагами:
								//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P
								//бит 1 (маска 0x2): 0 = распознаём первый из параметров; 1 = распознаём второй из параметров (какой - по биту 1)
								//бит 2 (маска 0x4): 0 = распознаем Q; 1 = распознаём QR (событие по отпусканию кнопки), этот бит используется также как результат работы дешифр.

							if(DS_current_char == 'P'){
								//printf("M97; (DS_current_char == 'P'); DS_Type_command_SET_BUTTON\r\n"); //v
								*DS_status_ =  DS_Type_command_ACTIVATE_BUTTON; //_GOTO_FILE;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
								*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
								*DS_WSpoint_counter_ = 0;
								*DS_comm_num_ = 1;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							}
							else if(DS_current_char == 'Q'){
								*DS_status_ =  DS_Type_command_Set_Q_number; //_GOTO_FILE;
								*DS_buf_start_ = DS_i + 1;
								*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
								*DS_RGB_counter_ = DS_Type_command_ACTIVATE_BUTTON;
								*DS_WSpoint_counter_ = 0;
								*DS_comm_num_ = 0;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
							}
							else { //если не P или Q - значит ошибка
								Error_Message_full('M', 0x97,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;*/
						case 0x88: //1000 1000
							*DS_comm_num_ |= 0x10; // 0xB8 = 1001 1000
						case 0x89:	//M89 - переход в подпрограмму или выход из неё.
							//0x89 = 1000 1001, 0x88-> 0x98 = 1001 1000
							//Message("M89\r\n\0",1);
							*DS_comm_num_ |= 0x20; //0x89-> 1010 1001, 0x88-> 1011 1000
						case 0x98:  //M98 - перейти к другому файлу сразу же, если без P - то аналогично M47
									//проверка на конец номера команды: это должен быть конец строки или P
							*DS_comm_num_ = *DS_comm_num_ ^ 0x20; // M98=1011 1000 M89 = 1000 1001 M88=1001 1000
							/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){
								*DS_status_ =  DS_Type_command_new;
								if (DS_Flag_Register&0x20){
									numFileForButton[ButtonNomberCarrier]=0;
									FastCommandForButton[ButtonNomberCarrier]=FC_REPEAT_FILE_M47 - (*DS_comm_num_& 0x01);//FC_EXIT_SUBPROGRAM_M89 = FC_REPEAT_FILE_M47 - 1
									CommandArgForButton[ButtonNomberCarrier]=1;
									return DS_ANS_WS_S_DONE;
								}

								if (*DS_comm_num_& 0x01) {
									return	DS_ANS_EXIT_SUBPROGRAM;
								}
								return DS_ANS_REPEAT_FILE; //*DS_comm_num_& 0x01 = 1 для M89 и 0 для M98:	DS_ANS_REPEAT_FILE=8, DS_GOTO_FILE = 9
							} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
								*DS_status_ = DS_Type_command_comment;*/

							if (DS_End_Of_Command_flag){
								*DS_status_ = DS_End_Of_Command_flag;

								if (DS_Flag_Register&0x20){
									numFileForButton[ButtonNomberCarrier]=0;
									FastCommandForButton[ButtonNomberCarrier]=FC_REPEAT_FILE_M47 - (*DS_comm_num_& 0x01);
									CommandArgForButton[ButtonNomberCarrier]=1;
									return DS_ANS_WS_S_DONE;
								}
								if (*DS_comm_num_& 0x01) {
									return	DS_ANS_EXIT_SUBPROGRAM;
								}
								return DS_ANS_REPEAT_FILE;// + (*DS_comm_num_& 0x01)*(DS_ANS_EXIT_SUBPROGRAM - DS_ANS_REPEAT_FILE);
							} else if(DS_current_char == 'P'){
								*DS_status_ =  DS_Type_command_GOTO_FILE; //_GOTO_FILE;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;			//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							//	*DS_RGB_counter_ = 0;	//*DS_RGB_counter_ - сколько символов считано (разрешается не больше 8)
							} else if(DS_current_char == 'Q'){
								*DS_status_ =  DS_Type_command_GOTO_RND_FILE; //_GOTO_FILE;
								//	*DS_buf_start_ = DS_i + 1;
							//	*DS_Param_ = 0;			//*DS_Param_ - для хранения номера (имени) файла, в который переходить
							//	*DS_RGB_counter_ = 0;	//*DS_RGB_counter_ - сколько символов считано (разрешается не больше 8)
								*DS_WSpoint_counter_ = 0;
								*DS_comm_num_ &= 0x30;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
														//бит 4 (0x10) - сохраняем, чтобы отличать M98 (1) от M89 (0)
							} else { //если не конец строки, файла или комментарий - значит ошибка
								Error_Message_full('M', *DS_comm_num_ ,0, 0, 1,DS_i, DS_current_char);
								return DS_ERR_UNEXPEXTED_SYMBOL;
							}
							break;

						default:
							Error_Message_full('M', 0xFF,0, 0, 2,DS_i, DS_current_char);
							return DS_ERR_CODE_OUT_OF_RANGE;

							break;

					} //switch (*DS_comm_num_)

				} else  //if (DS_a8 == 0xFF)
				{ //если символ цифровой - добавляем циферку в копилку и ждем следующего символа
					*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
				}


				break; //case DS_Type_command_M:

//-------------------------------------------------------------------------------------------------------------------------------
//команды управления usart
			case DS_Type_command_U:
				DS_a8 = DS_chartoint (DS_current_char);
				//printf(" DS_Type_command_WS_S0 DS_a8 = %d\r\n", DS_a8); //v
				if (DS_a8 >2){ //если принятый символ не 0, 1 или 2
					*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
					*DS_Param_ = 0; //смысл этой переменной в каждом случае см в соответствующих ифах
					if (DS_current_char == 'I'){ //присвоение ID
						*DS_status_ =  DS_Type_command_UID;
						*DS_Param_ = EMPTY_PERSONAL_ID;
						//*DS_Param_ = 0; //здесь теперь будет храниться цвет очередной расшифровываемой точки RGB
						//*DS_buf_start_ = DS_i + 1;
						*DS_RGB_counter_ = 0;  // сброс переменных для распознавания ID
					} else if (DS_current_char == 'D'){//включить/отключить дебаггер
						//*DS_RGB_counter_ = *DS_comm_num_;
						*DS_comm_num_ = DS_USART_SET_DEBUG;
						//*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
						//*DS_Param_ = 0; //здесь теперь будет храниться 1(вкл) или 0 (выкл)
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'B'){//настройка baudrate
						*DS_status_ =  DS_Type_command_USART_BAUD;
						*DS_comm_num_ = DS_USART_SET_BAUD;
						//DS_Flag_Register |= 1; //читаем в dec формате
						//*DS_Param_ = 0; //здесь теперь будет храниться baudrate
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'R'){//включение/отключение приёма
						//*DS_RGB_counter_ = *DS_comm_num_;
						*DS_comm_num_ = DS_USART_SET_INPUT;
						//*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
						//*DS_Param_ = 0; //здесь теперь будет храниться вкл/выкл
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'T'){//вывод того безобразия, которое будет дальше; аналогично SR Q...P...
						*DS_RGB_counter_ = (*DS_RGB_counter_ - 1)&0x0F; //порту 1 соответствует значение 0, порту 2 - 1; USB - все остальные
						*DS_status_ = DS_Type_command_USART1_OUT;// + *DS_RGB_counter_;// :DS_Type_command_USART2_OUT;
						*DS_comm_num_ = 4;  //*DS_comm_num_ используется как флаг состояния приема: ==4 - ждём букву Q, ==5 - ждём значение параметра Q
						//*DS_Param_ = 0;
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'W'){//установка максимального времени приёма команды
						//*DS_status_ =  DS_Type_command_USART_TIMEOUT;
						*DS_status_ =  DS_Type_command_USART_BAUD;
						*DS_comm_num_ = DS_USART_SET_TIMEOUT;
						//DS_Flag_Register &= ~1; //hex формат числа
						//*DS_Param_ = 0; //здесь теперь будет храниться время
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'F'){//установка флага ожидания конца передачи
						//*DS_RGB_counter_ = *DS_comm_num_;
						*DS_comm_num_ = DS_USART_SET_WAIT;
						//*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
						//*DS_Param_ = 0; //здесь теперь будет храниться вкл/выкл
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'A'){//вывод данных
						*DS_status_ =  DS_Type_command_USART_SPECS;
						DS_Status_backup = DS_Type_command_USART_SPECS;
						*DS_RGB_counter_ = (*DS_RGB_counter_ - 1)&0x0F ; //порту 1 соответствует значение 0, порту 2 - 1; USB - все остальные
						//*DS_Param_ = 0; //здесь теперь будет храниться выбранный ответ
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'S'){//установка флага переключения в приём по байтам
						//*DS_RGB_counter_ = *DS_comm_num_;
						*DS_comm_num_ = DS_USART_SET_SHORT;
						//*DS_status_ =  DS_Type_command_SWITCH_ONOFF;
						//*DS_Param_ = 0; //здесь теперь будет храниться вкл/выкл
						//*DS_buf_start_ = DS_i + 1;
					} else if (DS_current_char == 'C'){//управление I2C
						*DS_status_ =  DS_Type_command_I2C;

					} else { //какая-то неправильная буква
						Error_Message_full('U', 0xFF,0, 0, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL;
					}

					//	*DS_buf_start_ = DS_i + 1;
				} else {
					//если символ был цифровой это был номер порта, 0,1 или 2 - заносим свежую цифру в сразу в параметр
					*DS_RGB_counter_ |= DS_a8;
					//*DS_comm_num_ = DS_a8;
				}

				break;
//I2C control-------------------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_I2C:
				I2C_Bytes_To_Send = 0;
				/*if (DS_current_char == '0' || DS_End_Of_Command_flag){//выключить
					*DS_status_ = DS_End_Of_Command_flag;
					*DS_RGB_counter_ = 4;
					return DS_USART_SET_SHORT;
				} else*/
				if (DS_current_char == 'I'){//включить и назначить адрес
					*DS_status_ =  DS_Type_command_USART_BAUD;
					I2C_stage = 0x7f;
					//I2C_Bytes_To_Send = 0;
					I2C_Bytes_to_Recieve = 0;
					*DS_comm_num_ = DS_I2C_SET_ADDRESS;
				} else if (DS_current_char == 'T'){//управление I2C - отправить текс
					*DS_RGB_counter_ = I2C_OUT_LABEL;
					*DS_status_ =  DS_Type_command_I2C_Send_to;
					//I2C_Bytes_To_Send = 0;
					//I2C_stage = I2C_stage;
					//I2C_Bytes_To_Send = I2C_Bytes_To_Send;
				} else if (DS_current_char == 'R'){
					DS_Flag_Register |=2;
					*DS_RGB_counter_ = I2C_OUT_LABEL;
					*DS_status_ =  DS_Type_command_I2C_Send_to;
				} else if (DS_current_char == 'S'){//прозвонить все адреса и выдать результат в USB
					I2C1_ForceSend();

					goto Finish_Command_On_The_Spot_Without_Reading_All_Line_Label;
					//*DS_status_ = DS_Type_command_comment;
					//if (DS_End_Of_Command_flag){*DS_status_ = DS_End_Of_Command_flag;}
					//return DS_ANS_WS_S_DONE;

				} else { //какая-то неправильная буква
					Error_Message_full('U', 'C',0, 0, 1,DS_i, DS_current_char);
					return DS_ERR_UNEXPEXTED_SYMBOL;
				}
				break;
//-------------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_A: //действие по кнопке
/*				*DS_status_ =  DS_Type_command_BUTTON_EMULATE; //_GOTO_FILE;
				ButtonNomberCarrier = 0;
				*DS_Param_ = 0;				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
				//	*DS_RGB_counter_ = 0;		//*DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8)
				//	*DS_WSpoint_counter_ = 0;
				*DS_comm_num_ = 0x80;			//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P, остальные биты - инициализация нулями
*/
				//break;
//comment-----------------------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_comment:
				if (DS_current_char == EOF || DS_current_char == '\n' || DS_current_char == '\r'){ // ждем конца строки или файла
					*DS_status_ = DS_Type_command_new;
					//	*DS_buf_start_ = DS_i + 1;
				//} else if (DS_current_char == DS_Symbol_comment_end){
				//	*DS_status_ = DS_Status_Before_comment;
				}
				break; // DS_Type_command_comment:
//G4 P...--------------------------------------------------------------------------------------------------------------------------
//G4 P..., G5 P..., G6 P..., G7 P...
			case DS_Type_command_Pause_Absolut2: //ждем значение параметра для G7
			case DS_Type_command_Pause_Absolut: //ждем значение параметра для G6
			case DS_Type_command_Pause2: //ждем значение параметра для G5
			case DS_Type_command_Pause: //ждем значение параметра для G4
				// *DS_comm_num_ = *DS_status_ - DS_Type_command_Pause;
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					// проверка на начало комментария
					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('G', (4 + *DS_comm_num_),0, 1, 1,DS_i, DS_current_char);/*
						Error_Message_Command('G', (4 + *DS_comm_num_) );
						Error_Meaasge_Param (0,1);
						Error_Message_data(1,DS_i, DS_current_char);*/
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

					// если новый символ был не цифровой, но и не ошибочный
					//*DS_Param_ = DS_a32; // значение паузы передаем через эту переменную
					//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					if ((DS_Flag_Register&0x20) && (*DS_comm_num_ < 2)){
						//numFileForButton[ButtonNomberCarrier]=*DS_Param_;
						numFileForButton[ButtonNomberCarrier]=*DS_Param_;
						FastCommandForButton[ButtonNomberCarrier]=FC_PAUSE_END_G4+*DS_comm_num_;
						//CommandArgForButton[ButtonNomberCarrier]=1;
						return DS_ANS_WS_S_DONE;
					} //else {
					//	return DS_ANS_REPEAT_FILE;
					//}*/
					return DS_ANS_PAUSE + *DS_comm_num_; //исполняем паузу
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}
				break;
//G5 P...--------------------------------------------------------------------------------------------------------------------------
/*			case DS_Type_command_Pause2: //ждем значение параметра для G5
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					// проверка на начало комментария
					if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_Command('G', 5 );
						Error_Meaasge_Param (0,1);
						Error_Message_data(1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

					// если новый символ был не цифровой, но и не ошибочный
					//*DS_Param_ = DS_a32; // значение паузы передаем через эту переменную
					*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					return DS_ANS_PAUSE2; //исполняем паузу
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}
				break;*/
//G6 P... G7 P...--------------------------------------------------------------------------------------------------------------------------
/*			case DS_Type_command_Pause_Absolut2:
				*DS_comm_num_ = 1;
			case DS_Type_command_Pause_Absolut: //ждем значение параметра для G6
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					// проверка на начало комментария
					if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					}
					// проверка на конец строки
					else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_Command('G', (6+*DS_comm_num_) );
						Error_Meaasge_Param (0,1);
						Error_Message_data(1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

					// если новый символ был не цифровой, но и не ошибочный
					//*DS_Param_ = DS_a32; // значение паузы передаем через эту переменную
					*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					//if (*DS_comm_num_) {return DS_ANS_PAUSE_ABSOLUT2;} //исполняем паузу
					//else {return DS_ANS_PAUSE_ABSOLUT;} //исполняем паузу
					return (DS_ANS_PAUSE_ABSOLUT + (*DS_comm_num_)*(DS_ANS_PAUSE_ABSOLUT2 - DS_ANS_PAUSE_ABSOLUT));
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}
				break;*/
//G8 P...------------------------------------------------------------------------------------------------
			case DS_Type_command_Set_Absolut: //ждем значение параметра для G7
				DS_a8 = DS_chartoint (DS_current_char);
				//printf ("DS_i = %d, DS_current_char = %d, DS_a8 = %d\r\n", DS_i, DS_current_char, DS_a8);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					// проверка на начало комментария
					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else if(*DS_comm_num_ > 0) { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('M', 25,0, 1, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('G', 8,0, 1, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

					// если новый символ был не цифровой, но и не ошибочный
					//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					return DS_ANS_SET_ABSOLUT_TIME + *DS_comm_num_; //назначаем время
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}
				break;
//M98 P...---------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_GOTO_FILE: //M98 P... (символ P уже получен)
				//*DS_Param_ - для хранения номера (имени) файла, в который переходить
				//*DS_RGB_counter_ - сколько символов считано (разрешается не больше 8) - нужно для правильного формирования текстового имени файла

				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой

					// проверка на начало комментария
					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/

					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('M', *DS_comm_num_ ,0, 3, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
					if ((*DS_RGB_counter_ == 0) || (*DS_Param_ == 0xFFFF)) {
						//*DS_Param_ = Prev_File_Num;
						//*DS_comm_num_ & 0x20 =  0 (M89, M88): 1 (M98) ;
						*DS_Param_ = (*DS_comm_num_ & 0x20) ?  Parent_Prev_File_Num : Prev_File_Num;
					} //если было пусто, то назначаем предыдущий файл
					// если новый символ был не цифровой, но и не ошибочный
					if (DS_Flag_Register&0x20){

						//if ((*DS_RGB_counter_ == 0) || (*DS_Param_ == 0xFFFF)) {
						//	numFileForButton[ButtonNomberCarrier]=(*DS_comm_num_ & 0x20) ? Parent_Prev_File_Num : Prev_File_Num;
						//} else {
							numFileForButton[ButtonNomberCarrier]= *DS_Param_;
						//}
						// M98=1011 1000 M89 = 1000 1001 M88=1001 1000
						//numFileForButton[ButtonNomberCarrier]=((*DS_RGB_counter_ == 0) || (*DS_Param_ == 0xFFFF)) ? Parent_Prev_File_Num : *DS_Param_;
						//ParameterForButton[ButtonNomberCarrier]=*DS_Param_;
						//FastCommandForButton[ButtonNomberCarrier]=FC_START_FILE_NOW_M98 + (*DS_comm_num_ & 0x1)*(FC_ENTER_SUBPROGRAM_M89 - FC_START_FILE_NOW_M98) ;
						//FastCommandForButton[ButtonNomberCarrier]=(*DS_comm_num_ & 0x20) ? FC_START_FILE_NOW_M98 : FC_ENTER_SUBPROGRAM_M89;
						FastCommandForButton[ButtonNomberCarrier]=(*DS_comm_num_ & 0x20) ? FC_START_FILE_NOW_M98 : ((*DS_comm_num_ & 0x10) ? FC_ENTER_NEXTFILE_M88 : FC_ENTER_SUBPROGRAM_M89);
						//*DS_comm_num_ & 0x1 =  1 (M89): 0 (M98) ;
						//CommandArgForButton[ButtonNomberCarrier]=1;
						return DS_ANS_WS_S_DONE;
					}

//перенести эту проверку вверх, чтобы сэкономить один ИФ

					*DS_RGB_counter_=LEN_FILE_FOR_BUTTON;  //меньше 4 символов - добавляем leading zeroes
					//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					return DS_GOTO_FILE; //исполняем переход в файл, определяемый *DS_Param_
				} else {
					//если принятый символ - цифра
					//если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_RGB_counter_ += 1;

					if (*DS_RGB_counter_>LEN_FILE_FOR_BUTTON) return DS_ERR_FNAME_TOO_LONG; //больше 8 символов запрещено - не влезет в имя файла
					*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
				}

				break; // DS_Type_command_GOTO_FILE:
//M98 Q...P...-------------------------------------------------------------------------------------------------
			case DS_Type_command_GOTO_RND_FILE:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					 if (*DS_comm_num_ & 0x2) { //если  распознавали второй из двух параметров, ждём только конца строки/файла/коммента

						/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;
						} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r')  {
							*DS_status_ = DS_Type_command_new;*/

						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
						} else {
							Error_Message_full('M', 0x98,1, 3, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}
						if ((DS_Flag_Register&0x20) ){
							if(*DS_comm_num_ & 4){
								if (*DS_Param_ == 0xFFFF) {
									numFileForButton[ButtonNomberCarrier]=(*DS_comm_num_ & 0x20) ? Parent_Prev_File_Num : Prev_File_Num;
								} else {
									numFileForButton[ButtonNomberCarrier]= *DS_Param_;
								}
								//ParameterForButton[ButtonNomberCarrier]=*DS_Param_;
								//*DS_comm_num_ & 0x10 =  0 (M89, M88): 1 (M98) ;
								//FastCommandForButton[ButtonNomberCarrier]=FC_START_FILE_NOW_M98;
								FastCommandForButton[ButtonNomberCarrier]=(*DS_comm_num_ & 0x20) ? FC_START_FILE_NOW_M98 : ((*DS_comm_num_ & 0x10) ? FC_ENTER_NEXTFILE_M88 : FC_ENTER_SUBPROGRAM_M89);

								//FastCommandForButton[ButtonNomberCarrier]=(*DS_comm_num_ & 0x20) ? FC_START_FILE_NOW_M98 : FC_ENTER_SUBPROGRAM_M89;
								//CommandArgForButton[ButtonNomberCarrier]=1;
							} else { //не выбрано файла
								FastCommandForButton[ButtonNomberCarrier]=0;
							}
							return DS_ANS_WS_S_DONE;
						} //else {
						//FastCommandForButton[ButtonNomberCarrier]=0; //на случай если мы не выберем файл, кнопка выключается
						//if (*DS_WSpoint_counter_ < 4 || *DS_WSpoint_counter_ > (4 + (Max_Random_File_List<<2))){
						if ( ~(*DS_comm_num_) & 4){
							//мы прошлись по всему списку и вылетели из него
							//Error_Message_full('M', 0x98,1, 3, 4,DS_i, DS_current_char);
							//return DS_ERR_UNEXPEXTED_SYMBOL;
							return DS_ANS_WS_S_DONE;
						}
						//*DS_RGB_counter_= LEN_FILE_FOR_BUTTON;  //меньше 4 символов - добавляем leading zeroes
						//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
						//	return DS_ANS_REPEAT_FILE;
						//}*/
						if (*DS_Param_ == 0xFFFF) {
							//*DS_Param_ = Prev_File_Num;
							//*DS_comm_num_ & 0x20 =  0 (M89, M88): 1 (M98) ;
							*DS_Param_ = (*DS_comm_num_ & 0x20) ?  Parent_Prev_File_Num : Prev_File_Num;
						}//если было FFFF, то назначаем предыдущий файл
						return DS_GOTO_FILE; //исполняем переход в файл, определяемый *DS_Param_

					} else { //(*DS_comm_num_ & 0x2) == 0, распознавали первый из двух параметров, ждём имя второго
						if ( DS_current_char == 'P' && ( ~*DS_comm_num_ & 0x1) ) { //байт0 == 0, значит принимали Q

							*DS_comm_num_ |= 0x3;  //принимаем второй параметр, и этот параметр P
							if (*DS_RGB_counter_ > 0){
								//пределяем символьный сдвиг начала присвоения значения
								*DS_WSpoint_counter_ = 0-((rand()%*DS_RGB_counter_)<<2);
							} else {
								*DS_WSpoint_counter_ = 4 - (ChosenNomberInM98 << 2);
								//Error_Message_full('M', 0x98,1, 3, 2,DS_i, DS_current_char);
								//return DS_ERR_CODE_OUT_OF_RANGE;
							}
						} else if (DS_current_char == 'N' && ( ~*DS_comm_num_ & 0x1)){ //QN - количество файлов в DEC
							DS_Flag_Register |=1;
						}
						//else if ( DS_current_char == 'Q' && ( *DS_comm_num_ & 0x1) ) { //бит0 == 1, значит принимали P

						//	*DS_comm_num_ |= 0x2; //теперь принимаем второй параметр (бит1 = 1)
						//	*DS_comm_num_ &= ~0x1; // принимаем параметр Q (бит0 = 0)
						//}
						else {
							Error_Message_full('M', 0x98,1, 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}
					}
				} else {//если принятый символ - цифра
					//если сюда дошли - добавляем свежеполученный символ в нужный параметр и продолжаем расшифровку
					if (*DS_comm_num_ & 0x1) { //если младший бит =1, то записываем символ в P, иначе - в Q
						*DS_WSpoint_counter_ += 1;
						//если попали в диапазон от 1 до 4
						if ((*DS_WSpoint_counter_ > 0) && (*DS_WSpoint_counter_ < 5)) {
							*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);//добавляем символ в P
							if (*DS_WSpoint_counter_ == 4){*DS_comm_num_ |= 0x4;}//сигнал что мы действительно выбрали файл и нам есть что добавлять в кнопки
						}  //добавляем символ в P
					} else { //определяем длину списка
						//*DS_RGB_counter_ = DS_hex_add_symbol(DS_a8, *DS_RGB_counter_); //добавляем символ в Q
						*DS_RGB_counter_ = DS_num_add_symbol(DS_a8, *DS_RGB_counter_); //добавляем символ в Q
							if (*DS_RGB_counter_ > Max_Random_File_List){
								*DS_RGB_counter_ = Max_Random_File_List;
								//Error_Message_full('M', 0x98,1, 0, 2,DS_i, DS_current_char);
								//return DS_ERR_CODE_OUT_OF_RANGE;
							}
					}
				}
				break; //DS_Type_command_GOTO_RND_FILE
//------------------------------------------------------------------------------------------------------------------------------------
//M96 Q...P..., M97 Q...P...
			case DS_Type_command_Set_Q_number:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					if ( DS_current_char == 'R'){
						*DS_comm_num_ = 0x4;
					} else if ( DS_current_char == 'H'){//обращение по номеру действия
						*DS_comm_num_ = 0x8;
					} else {
Set_Button_Parameter_Type_Label:
						DS_Flag_Register &=~0x0D; //in HEX, 1+4+8
						*DS_status_ = DS_Type_command_SET_BUTTON;
						if ((*DS_comm_num_ & 0x4) && (~ButtonFlags & 1)){ButtonNomberCarrier += MAX_NUM_BUTTONS;}
						FastCommandForButton[ButtonNomberCarrier] &= 127;//включить кнопку
						if ( DS_current_char == 'T') { //Включить команду с предыдущими настройками
							*DS_status_ =  DS_Type_command_comment;
							*DS_WSpoint_counter_ = FC_TOTAL_COMMAND_LIST+3;
						} else if ( DS_current_char == 'P') { //по нажатию перейти в файл
							*DS_WSpoint_counter_ = FC_START_FILE_NOW_M98;
						} else if ( DS_current_char == 'L') { //по нажатию переназначить команду M47, M89, M88, M98 на переход в файл
							*DS_WSpoint_counter_ = FC_NEXT_FILE_M98;
						} else if ( DS_current_char == 'J') { //по нажатию перейти в файл в конце анимации
							*DS_WSpoint_counter_ = FC_AFTER_ANIM_FILE_M98;
						} else if ( DS_current_char == 'I') { //по нажатию вызвать подпрограмму
							*DS_WSpoint_counter_ = FC_ENTER_SUBPROGRAM_M89;
						} else if ( DS_current_char == 'O') { //по нажатию выйти из подпрограммы
							*DS_WSpoint_counter_ = FC_EXIT_SUBPROGRAM_M89;
						/*} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r')  {
							*DS_status_ = DS_Type_command_new;
							return DS_ASSIGN_BUTTON;
						} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;*/
						} else if (DS_End_Of_Command_flag){//конец команды, выключить кнопку
							*DS_status_ = DS_End_Of_Command_flag;
							//*DS_WSpoint_counter_ = FC_TOTAL_COMMAND_LIST+3;
							FastCommandForButton[ButtonNomberCarrier] |= 128;
							return DS_ANS_WS_S_DONE;
						} else if (DS_current_char == 'M'){ //запимываем команду в кнопку
							DS_Flag_Register |= 0x20; //b5 (32) - поднять
							*DS_status_ = DS_Type_command_M;
						} else if (DS_current_char == 'G'){ //запимываем команду в кнопку
							DS_Flag_Register |= 0x20; //b5 (32) - поднять
							*DS_status_ = DS_Type_command_G;
							/**/
						} else {
							if (ButtonNomberCarrier < AFTER_FAST_ANIM_ACIONS_ADRESS){
								Error_Message_full('M', 0x96,1, 0, 1,DS_i, DS_current_char);
							}else{
								Error_Message_full('G', 0x37,1, 0, 1,DS_i, DS_current_char);
							}
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}

						/*if (ButtonNomberCarrier <= MAX_NUM_BUTTONS_MASK){//действие относится к кнопкам
							if ((ButtonNomberCarrier >= MAX_NUM_BUTTONS)) {		//бит 3 == 1, значит принимали команду на отпускание кнопки
								buttonRelInterruptActive |= (0x1 << (ButtonNomberCarrier & 0xF)); // соответствующее прерывание делаем активным
							} else {	 //бит 3 == 0, значит принимали команду на нажатие кнопки
								buttonPushInterruptActive |= (0x1 << ButtonNomberCarrier); // соответствующее прерывание делаем активным
							}
						}/**/

					}
				} else {//если принятый символ - цифра
					if (*DS_comm_num_ == 0x8){
						ButtonNomberCarrier <<= 4;
						ButtonNomberCarrier |= DS_a8;
						ButtonNomberCarrier &= MAX_NUM_BUTTONS_ACIONS_MASK;
					} else if (ButtonFlags&1){ //режим комбинации кнопок, DS_a8 должно быть 1...5
						DS_a8 = (DS_a8 - 1) & 7; //1...8 -> 0...7
						ButtonNomberCarrier |= (1<<DS_a8) & MAX_NUM_BUTTONS_ACIONS_MASK; //0...31
					} else {
						ButtonNomberCarrier = DS_a8; //поскольку сейчас MAX_NUM_BUTTONS = 16 и номер кнопки умещается в доин символ
					}


				}

				break;
//M96 Q... P...------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_SET_BUTTON:		//M96 Q... P... (символ Q уже получен)
				//uint32_t *DS_Param_ - для хранения номера (имени) файла, в который переходить (для P)
				//uint8_t *DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
				//uint16_t *DS_WSpoint_counter_ - для хранения номера кнопки, которую кодируем (для Q)
				//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2=0b10 = есть P, читаем Q; 3=0b11 = есть Q, читаем P
						//*DS_comm_num_ используется как байт с флагами:
						//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P
						//бит 1 (маска 0x2): 0 = распознаём первый из параметров; 1 = распознаём второй из параметров (какой - по биту 1)
						//бит 2 (маска 0x4): 0 = распознаем Q; 1 = распознаём QR (событие по отпусканию кнопки), этот бит используется также как результат работы дешифр.

				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					/*if ( DS_current_char == 'R' && ( ~*DS_comm_num_ & 0x1) ) { //ждём нуля в байте 0 и одновременно символа R
															//это значит - принимали значение Q, теперь будем принимать QR
						*DS_comm_num_ |= 0x4; //ставим 1 в бит3 ( = распознаём QR) и выходим из дерева
					}
					else if (*DS_comm_num_ & 0x2) { //если  распознавали второй из двух параметров, ждём только конца строки/файла/коммента
					 */
						/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r')  {
							*DS_status_ = DS_Type_command_new;
						} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;*/
						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
						} else {
							Error_Message_full('M', 0x96,(1+((*DS_comm_num_&4)>>2)), 3, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}
						//if (((*DS_RGB_counter_ == 0) || (*DS_Param_ == 0xFFFF)) && ((DS_Flag_Register & 0x10) == 0) ) {*DS_Param_ = Parent_Prev_File_Num;} //если было пусто или FFFF, то назначаем предыдущий файл
						if ((*DS_RGB_counter_ == 0) || (*DS_Param_ == 0xFFFF))  {*DS_Param_ = Parent_Prev_File_Num;} //если было пусто или FFFF, то назначаем предыдущий файл

						//*DS_RGB_counter_=LEN_FILE_FOR_BUTTON;  //меньше 4 символов в P - добавляем leading zeroes
						//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
						return DS_ASSIGN_BUTTON;

					/*} else { //(*DS_comm_num_ & 0x2) == 0, распознавали первый из двух параметров, ждём имя второго
						if ( DS_current_char == 'P' && ( ~*DS_comm_num_ & 0x1) ) { //байт0 == 0, значит принимали Q

							*DS_comm_num_ |= 0x3;  //принимаем второй параметр, и этот параметр P
						} else if ( DS_current_char == 'Q' && ( *DS_comm_num_ & 0x1) ) { //бит0 == 1, значит принимали P

							*DS_comm_num_ |= 0x2; //теперь принимаем второй параметр (бит1 = 1)
							*DS_comm_num_ &= ~0x1; // принимаем параметр Q (бит0 = 0)
						}
						else {
							Error_Message_full('M', 0x96,(1+(*DS_comm_num_>>2)), 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}
					}*/
				} else {//если принятый символ - цифра
					//если сюда дошли - добавляем свежеполученный символ в нужный параметр и продолжаем расшифровку
					//if (*DS_comm_num_ & 0x1) { //если младший бит =1, то записываем символ в P, иначе - в Q
						*DS_RGB_counter_ += 1;
//						if (*DS_RGB_counter_>LEN_FILE_FOR_BUTTON) {
//							Error_Message_full('M', 0x96,1, 0, 2,DS_i, DS_current_char);
//							return DS_ERR_CODE_OUT_OF_RANGE;} //больше 6 символов запрещено - не влезет в имя файла
						*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);  //добавляем символ в P
						//*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
					/*} else {
						*DS_WSpoint_counter_ = DS_hex_add_symbol(DS_a8, *DS_WSpoint_counter_); //добавляем символ в Q
					}*/
				}
				break; // DS_Type_command_SET_BUTTON:

//G25 Q... ------------------------------------------------------------------------------------------------------------------------
//			case DS_Type_command_BUTTON_EMULATE2:
//				*DS_comm_num_ |= 0x4; //ставим 1 в бит3 ( = распознаём QR) и продолжаем читать
			case DS_Type_command_BUTTON_EMULATE:		//G25 Q... P... (символ Q уже получен)
				//uint32_t *DS_Param_ - для хранения номера (имени) файла, в который переходить (для P)
				//uint8_t *DS_RGB_counter_ - сколько символов имени файла считано (разрешается не больше 8) - нужно для правильного формирования текстового имени файла
				//uint16_t *DS_WSpoint_counter_ - для хранения номера кнопки, которую кодируем (для Q)
				//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2=0b10 = есть P, читаем Q; 3=0b11 = есть Q, читаем P
						//*DS_comm_num_ используется как байт с флагами:
						//бит 0 (маска 0x1) : 0 = распознаём Q; 1 = распознаём P
						//бит 1 (маска 0x2) : 0 = '<', 1 = '>'
						//бит 2 (маска 0x4) : 0 = распознаем Q; 1 = распознаём QR (событие по отпусканию кнопки), этот бит используется также как результат работы дешифр.
						//бит 3 (маска 0x8) : ситуация "=" или "!="
						//бит 4 (маска 0x10): проверяем, фактически нажато или отпущено
						//бит 7 (маска 0x80): прямое значение короткой команды
						// т.е. значения параметра указывают на состояния 1 => "<", 1+2 => ">", 1+8 => "!=", 1+2+8 => "="
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
// перед входом в функцию у нас и так было *DS_comm_num_ = 0. так что в ветках "R" и "H" можно вместо |= ставить просто =
					if ( DS_current_char == 'R' && ((*DS_comm_num_ & 0x1)==0)) { //ждём нуля в байте 0 и одновременно символа R
															//это значит - принимали значение Q, теперь будем принимать QR
						*DS_comm_num_ = 0x4; //ставим 1 в бит3 ( = распознаём QR) и выходим из дерева
					} else if ( DS_current_char == 'H' && ((*DS_comm_num_ & 0x1)==0)) {
						//это значит - принимали значение Q, теперь будем принимать QH
					//	*DS_comm_num_ &=~4;
						*DS_comm_num_ = 0x80; //ставим 1 в бит7 - будем работать с командой по её номеру
					} else if ( DS_current_char == 'P' && ((*DS_comm_num_ & 0x1)==0)){
						*DS_comm_num_ |=1;
						//DS_Flag_Register &=~1; //второй параметр в hex, положительный, абсолютный 1 + (4+8)
					} else if ( DS_current_char == 'N' && ((*DS_comm_num_ & 0x1)==0)){
						*DS_comm_num_ |=1;
						DS_Flag_Register |=1; //второй параметр в dec
					} else if ( DS_current_char == '>' && (*DS_comm_num_ & 0x1)){
						*DS_comm_num_ |=2;
					} else if ( DS_current_char == '<' && (*DS_comm_num_ & 0x1)){
						*DS_comm_num_ &=~2;
					} else if ( DS_current_char == '=' && (*DS_comm_num_ & 0x1)){
						*DS_comm_num_ |= 0x0A; //8+2
					} else if ( DS_current_char == '!' && (*DS_comm_num_ & 0x1)){
						*DS_comm_num_ |= 8; //8
						*DS_comm_num_ &=~2;
					} else { //если  распознавали параметр, ждём только конца строки/файла/коммента


						/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r')  {
							*DS_status_ = DS_Type_command_new;
						} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;*/
						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
						} else if ( DS_current_char == '?'){
							*DS_comm_num_ |=0x10;
							*DS_status_ = DS_Type_command_comment;
						} else {
							Error_Message_full('G', 0x25,(1+((*DS_comm_num_&4)>>2)), 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}
						if ((*DS_comm_num_ & 0x4) && (~ButtonFlags & 1)){ButtonNomberCarrier += MAX_NUM_BUTTONS;}

						//		*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
						return DS_ANS_BUTTON_EMULATE;

					}
				} else {//если принятый символ - цифра
					//если сюда дошли - добавляем свежеполученный символ в нужный параметр и продолжаем расшифровку

					if (*DS_comm_num_ & 0x1) { //если младший бит =1, то записываем символ в P, иначе - в Q
						*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);  //добавляем символ в P

					} else {
						if (*DS_comm_num_ == 0x80){//будем работать с командой по её номеру
							ButtonNomberCarrier <<= 4;
							ButtonNomberCarrier |= DS_a8;
							ButtonNomberCarrier &= MAX_NUM_BUTTONS_ACIONS_MASK;
						} else if (ButtonFlags&1){ //DS_a8 должно быть 1...5, комбинация
							DS_a8 = (DS_a8 - 1) & 7; //1...8 -> 0...7
							ButtonNomberCarrier |= (1<<DS_a8) & MAX_NUM_BUTTONS_ACIONS_MASK; //0...31
						} else {
							ButtonNomberCarrier = DS_a8; //поскольку сейчас MAX_NUM_BUTTONS = 16 и номер кнопки умещается в доин символ
						}
					}
				}
				break; // DS_Type_command_EMULATE_BUTTON:

//M97 Q... P...------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_ACTIVATE_BUTTON:		//M97 - активация/деактивация кнопок M97 Q<кнопка> P<0-выкл, 1-вкл.> (символ Q или P уже получен)
/*				//uint32_t *DS_Param_ - акивируем или деактивируем прерывание по кнопке
				//uint8_t *DS_RGB_counter_
				//uint16_t *DS_WSpoint_counter_ - для хранения номера кнопки, которую кодируем (для Q)
				//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2=0b10 = есть P, читаем Q; 3=0b11 = есть Q, читаем P
						//*DS_comm_num_ используется как байт с флагами:
						//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P
						//бит 1 (маска 0x2): 0 = распознаём первый из параметров; 1 = распознаём второй из параметров (какой - по биту 1)
						//бит 2 (маска 0x4): 0 = распознаем Q; 1 = распознаём QR (событие по отпусканию кнопки), этот бит используется также как результат работы дешифр.

				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
//					if ( DS_current_char == 'R' && ( ~*DS_comm_num_ & 0x1) ) { //ждём нуля в байте 0 и одновременно символа R
//															//это значит - принимали значение Q, теперь будем принимать QR
//						*DS_comm_num_ |= 0x4; //ставим 1 в бит3 ( = распознаём QR) и выходим из дерева
//					}
//					else if (*DS_comm_num_ & 0x2) { //если  распознавали второй из двух параметров, ждём только конца строки/файла/коммента


						if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r')  {
							*DS_status_ = DS_Type_command_new;
							*DS_comm_num_ |= 0x1;
						} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;
							*DS_comm_num_ |= 0x1;
						} else if(DS_current_char == 'O'){ //параметр для команды - битовая маска
							*DS_comm_num_ |= 0x2;
						} else {
							Error_Message_full('M', 0x97,(1+(*DS_comm_num_>>2)), 3, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}

						//Мы передаём *DS_Param_ (команда), *DS_comm_num_ (Q/QR), *DS_WSpoint_counter_ (кнопка)
						if ((*DS_comm_num_ & 0x1)) {
							//if (*DS_RGB_counter_==0){*DS_RGB_counter_ = 1;}//это для всех, нули там только если оно вызвано из основного цикла
							*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
							return DS_ACTIVATE_BUTTON;
						}

//					} else { //(*DS_comm_num_ & 0x2) == 0, распознавали первый из двух параметров, ждём имя второго
//						if ( DS_current_char == 'P' && ( ~*DS_comm_num_ & 0x1) ) { //байт0 == 0, значит принимали Q
//
//							*DS_comm_num_ |= 0x3;  //принимаем второй параметр, и этот параметр P
//						} else if ( DS_current_char == 'Q' && ( *DS_comm_num_ & 0x1) ) { //бит0 == 1, значит принимали P
//
//							*DS_comm_num_ |= 0x2; //теперь принимаем второй параметр (бит1 = 1)
//							*DS_comm_num_ &= ~0x1; // принимаем параметр Q (бит0 = 0)
//						}
//						else {
//							Error_Message_full('M', 0x97,1, 0, 1,DS_i, DS_current_char);
//							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
//						}
//					}
				} else {//если принятый символ - цифра
					//если сюда дошли - добавляем свежеполученный символ в нужный параметр и продолжаем расшифровку
#if defined (FastButtonShortcuts)
					if (*DS_comm_num_ & 0x2) { //если младший бит =1, то записываем символ в P, иначе - в Q
						*DS_RGB_counter_ |= (0x01<<DS_a8); //битовая маска если надо
					} else {
						*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
						if (*DS_Param_ > FC_TOTAL_COMMAND_LIST) {*DS_Param_ = 0;} //незнакомая команда - выключение кнопки
					}
#endif
				}/**/


				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
						if (*DS_comm_num_ == 3){//у нас сброс всех энкодеров
							ButtonEncoder = 0;
							for (DS_a8 = 0; DS_a8 < MAX_NUM_ENCODERS; ++DS_a8){
								ButtonEncorerIndiv[DS_a8] = 0;
							}
							goto Clear_All_Encoders_Label;
						}
					} else if ( DS_current_char == 'P' && ( *DS_comm_num_ == 3) ) {
						*DS_comm_num_ = 0;
					} else {
						Error_Message_full('M', 0x86,1, 0, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
					ButtonEncoder &= ~ButtonEncorerIndiv[*DS_RGB_counter_]; //очистим от предыдущего значения этого энкодера
					if ((ButtonEncoder & *DS_Param_) == 0) {
						ButtonEncoder |= *DS_Param_;
						ButtonEncorerIndiv[*DS_RGB_counter_] = *DS_Param_;
					} else {//error
						Error_Message_full('M', 0x86,1, 3, 2,DS_i, DS_current_char);
						return DS_ERR_CODE_OUT_OF_RANGE; // если это не конец строки
					}
Clear_All_Encoders_Label:
					return DS_ANS_WS_S_DONE;
				} else {//если принятый символ - цифра

					if (*DS_comm_num_ < 2){
						DS_a8 = (DS_a8) & 7; //1...8 -> 0...7
						*DS_Param_ |= (1<<DS_a8);
						*DS_comm_num_ +=1;
					} else if (*DS_comm_num_ == 3){
						if (DS_a8 >= MAX_NUM_ENCODERS){
							Error_Message_full('M', 0x86,1, 0, 2,DS_i, DS_current_char);
							return DS_ERR_CODE_OUT_OF_RANGE;
						}
						*DS_RGB_counter_ = DS_a8;
					} else {//error
						Error_Message_full('M', 0x86,1, 3, 3,DS_i, DS_current_char);
						return DS_ERR_TOO_MANY_SYMBOLS; // если это не конец строки
					}
				}

				break; // DS_Type_command_ACTIVATE_BUTTON:



//G0------------------------------------------------------------------------------------------------------------------------------------------
//G0 P..	по 4 hex символа на серву
			case DS_Type_command_SERVO_SET_ALL:
				// проверка на начало комментария

				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					/*if (DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r'){   //0x0D){ //сравниваем его с концом строки
						*DS_status_ = DS_Type_command_new;
						//	*DS_buf_start_ = DS_i + 1;
						return DS_ANS_WS_S_DONE;  // расшифровка закончена
					} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;*/
						//	*DS_buf_start_ = DS_i + 1;
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
						return DS_ANS_WS_S_DONE;  // расшифровка закончена
					} else if (DS_current_char == '-'){//отрицательное число
						DS_Flag_Register |=0xC;
					} else if (DS_current_char == '+'){//положительное число
						DS_Flag_Register |=8;
						DS_Flag_Register &=~4;
					} else if (DS_current_char == 'R' || DS_current_char == 'r'){//делаем случайное число
						*DS_RGB_counter_ = 4;
						*DS_Param_ = rand()%(Servo_Resolution+1);
						goto Servo_Set_Value; //перескакивем к записи числа в массив
					} else {
						Error_Message_full('G', 0,0, 3, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
				} else { //(DS_a8 != 0xFF) //конец "если принятый символ не цифровой": если тру, дальше прохода не дожно быть
						//основная расшифровка
						//добавляем свежеполученный символ в параметр
						*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
						//отсчитываем 4 цифровых символов после P
						*DS_RGB_counter_ += 1;
Servo_Set_Value:
						if (*DS_RGB_counter_ >= 4){ //когда случилось 4 цифровых символов - распознавание и помещение в память буфера
							//теперь в *DS_Param_ находится значение положения сервы
							if (*DS_WSpoint_counter_ >= 8){
								*DS_status_ = DS_Type_command_new;
								//	*DS_buf_start_ = DS_i + 1;
								Error_Message_full('G', 0,0, 3, 3,DS_i, DS_current_char);
								return DS_ERR_TOO_MANY_SYMBOLS;
							}
							if (*DS_Param_> Servo_Resolution) {*DS_Param_ = Servo_Resolution;}
							if (~DS_Flag_Register & 8) { //абсолютное значение
								Servo_Pos_Temp[*DS_WSpoint_counter_] = *DS_Param_;
							} else if (DS_Flag_Register & 4){ //минус
								Servo_Pos_Temp [*DS_WSpoint_counter_] = (Servo_Pos_Temp [*DS_WSpoint_counter_] > *DS_Param_) ? Servo_Pos_Temp [*DS_WSpoint_counter_] -= (uint16_t) *DS_Param_: 0;
							} else { //плюс
								Servo_Pos_Temp [*DS_WSpoint_counter_] += (uint16_t) *DS_Param_;
								if (Servo_Pos_Temp [*DS_WSpoint_counter_] > Servo_Resolution) {
									Servo_Pos_Temp [*DS_WSpoint_counter_] = Servo_Resolution;
								}
							}
							DS_Flag_Register &= ~0xC; //второй параметр положительный, абсолютный (4+8)
							Servo_Update_Flag |= 2; //флаг смены значений поднят
							*DS_WSpoint_counter_ += 1;	//текущий принимаемый номер точки WS
							*DS_RGB_counter_ = 0;		//номер символа в точке
							*DS_Param_ = 0;				//значение положения сервы - временное хранилище
						}
				}
				//	*DS_buf_start_ = DS_i + 1; //дальше начало анализа со следующего символа
				break; //DS_Type_command_SERVO_SET_ALL
//G0 Q... P...------------------------------------------------------------------------------------------------
			case DS_Type_command_SERVO_SET:
				DS_a8 = DS_chartoint (DS_current_char);
				//uint8_t *DS_comm_num_,  -	какой параметр сейчас распознается: 0 = Q; 1 = P;  2=0b10 = есть P, читаем Q; 3=0b11 = есть Q, читаем P
						//*DS_comm_num_ используется как байт с флагами:
						//бит 0 (маска 0x1): 0 = распознаём Q; 1 = распознаём P
						//бит 1 (маска 0x2): 0 = распознаём первый из параметров; 1 = распознаём второй из параметров (какой - по биту 1)

				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					if (*DS_comm_num_ & 0x2) { //если  распознавали второй из двух параметров, ждём только конца строки/файла/коммента

						/*if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == '\r')  {
							*DS_status_ = DS_Type_command_new;
							*DS_RGB_counter_ = 0;		//номер символа в точке
							return DS_ANS_SERVO_SET;
						} else if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;*/
						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
							*DS_RGB_counter_ = 0;		//номер символа в точке
							return DS_ANS_SERVO_SET;
						} else if (DS_current_char == '-'){//отрицательное число
							DS_Flag_Register |=0xC;
						} else if (DS_current_char == '+'){//положительное число
							DS_Flag_Register |=8;
							DS_Flag_Register &=~4;
						} else if (DS_current_char == 'R'){
							*DS_Param_ = rand()%(Servo_Resolution+1);
						} else {
							Error_Message_full('G', 0,1, 1, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}

					} else { //(*DS_comm_num_ & 0x2) == 0, распознавали первый из двух параметров, ждём имя второго
						//DS_Flag_Register &=~0xD; //параметр в hex, положительный, абсолютный 1 + (4+8)

						if ( (DS_current_char == 'P' || DS_current_char == 'H') && ( ~*DS_comm_num_ & 0x1) ) { //байт0 == 0, значит принимали Q

							*DS_comm_num_ = 0x3;  //принимаем второй параметр, и этот параметр P
							//DS_Flag_Register &=~0xD; //второй параметр в hex, положительный, абсолютный 1 + (4+8)

						} else if ( DS_current_char == 'N' && ( ~*DS_comm_num_ & 0x1) ) { //байт0 == 0, значит принимали Q

							*DS_comm_num_ = 0x3;  //принимаем второй параметр, и этот параметр P
						//	DS_Flag_Register &= ~0xC; //второй параметр положительный, абсолютный (4+8)
							DS_Flag_Register |=1; //второй параметр в dec
						} else if ( DS_current_char == 'R' && ( ~*DS_comm_num_ & 0x1) ) { //байт0 == 0, значит принимали Q

							*DS_comm_num_ |= 0x7;  //принимаем второй параметр, и этот параметр P, и мы рандомизируем
						//	DS_Flag_Register &=~0xD; //второй параметр в hex, положительный, абсолютный 1 + (4+8)

						} //else if ( DS_current_char == 'Q' && ( *DS_comm_num_ & 0x1) ) { //бит0 == 1, значит принимали P
						//	*DS_Param_ = 0;
						//	DS_Flag_Register &=~0xD; //параметр в hex, положительный, абсолютный 1 + (4+8)
						//	*DS_comm_num_ = 0x2; //теперь принимаем второй параметр (бит1 = 1)
						//	*DS_comm_num_ &= ~0x1; // принимаем параметр Q (бит0 = 0)
						//}
						else {
							Error_Message_full('G', 0,1, 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}

					}
				} else {//если принятый символ - цифра
					//если сюда дошли - добавляем свежеполученный символ в нужный параметр и продолжаем расшифровку
					if (*DS_comm_num_ & 0x1) { //если младший бит =1, то записываем символ в P, иначе - в Q
						*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);  //добавляем символ в P
					//	if (*DS_Param_> Servo_Resolution){
					//		*DS_Param_ = Servo_Resolution;
						//}
					} else {
						*DS_WSpoint_counter_ |= (1<<DS_a8);//DS_hex_add_symbol(DS_a8, *DS_WSpoint_counter_); //добавляем символ в Q
					}
				}
				break; //DS_Type_command_SERVO_SET

//---------------------------------------------------------------------------------------------------------
// M03 P... (включение серв), M04 P... (включение светодиодов в гибридном режиме), M05 P... (установка выключения серв)
			/*case DS_Type_command_SERVO_ENABLE_3: //проваливаемся вниз; подробнее алгоритм описан в обработчике S0 P
			case DS_Type_command_SERVO_ENABLE_4:
			case DS_Type_command_SERVO_ENABLE_5:
			case DS_Type_command_SERVO_ENABLE_6: //M10
			case DS_Type_command_SERVO_ENABLE_7: //M11

			//11(10001)->11, 10(10000)->10, 5(00101)->0F (01111), 3(00011)->0E (01110);
			 // *DS_comm_num_ += ((*DS_comm_num_ & 4)>>1) + ((*DS_comm_num_ & 4)<<1) (5(00101)->0F (01111)),
			 // + ((*DS_comm_num_ & 2)>>1) + (*DS_comm_num_ & 2) + ((*DS_comm_num_ & 2)<<2) (3(00011)->0E (01110))
			 // 4(00100)->0E (01110)!!

				 *DS_comm_num_ = *DS_status_ - DS_Type_command_SERVO_ENABLE_3 + 3;//вычисление короткого номера
				 if (*DS_status_ > DS_Type_command_SERVO_ENABLE_5) *DS_comm_num_ += 0x0A;/**/
			case DS_Type_command_SERVO_ENABLE:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой

					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else { //ошибка
						Error_Message_full('M', *DS_comm_num_,0, 3, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
					// если мы попали сюда, то значит пора назначить
					*DS_RGB_counter_ = 0;		//номер символа в точке
					//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
//*DS_comm_num_ хранит номер команды, что после M
					return DS_ANS_SERVO_DISABLE;

				} else {//если цифра
					if (DS_a8<8) {
						*DS_Param_ |= (0x101 << DS_a8);

					} else {
						Error_Message_full('M', *DS_comm_num_,0, 3, 2,DS_i, DS_current_char);
						return DS_ERR_CODE_OUT_OF_RANGE;
					}

				}
				break; //DS_Type_command_SERVO_DISABLE
//M06 P...------------------------------------------------------------------------------------------------------------------------
			case DS_Type_command_SET_LED_TYPE:		//M06 - назначение типа светодиодов M06 P<WS2812, SK6812> (P уже получен)
				DS_a8 = DS_chartoint (DS_current_char);

				if (DS_a8 == 0xFF){ //если принятый символ не цифровой

					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						Error_Message_full('M', 0x6 ,0, 3, 4,DS_i, DS_current_char);
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {// если мы попали сюда, то значит команда не дописана
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
						//Error_Message_full('M', 0x6 ,0, 3, 4,DS_i, DS_current_char);
						return DS_ANS_SET_LED_TYPE;
					} else if(DS_current_char == 'W'){ //добавляем числа, которые обозначат W, S и K в переменной DS_Param, так как она цифровая
						//*DS_RGB_counter_ += 1;
						*DS_Param_ = DS_hex_add_symbol(1, *DS_Param_);

					} else if(DS_current_char == 'S'){
						//*DS_RGB_counter_ += 1;
						*DS_Param_ = DS_hex_add_symbol(2, *DS_Param_);

					} else if(DS_current_char == 'K'){
						//*DS_RGB_counter_ += 1;
						*DS_Param_ = DS_hex_add_symbol(3, *DS_Param_);

					} else if(DS_current_char == 'G'){
						//*DS_RGB_counter_ += 1;
						*DS_Param_ = DS_hex_add_symbol(4, *DS_Param_);

					}else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('M', 0x6 ,0, 3, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

				} else {//если принятый символ - цифра
					//*DS_RGB_counter_ += 1;
					*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
				}
				/*if (*DS_RGB_counter_ >= 6){ //когда случилось 6 цифровых/псевдоцифровых символов - распознавание и помещение в память буфера
					//теперь в *DS_Param_ находится значение типа светодиодов
					//0x122812 для WS или 0x236812 для SK
					//0x122812B для WS2812b
					//или 0x130012 для промеежуточного (WK0012)
					//или 0x2d2490 (SDSG90)
					//или 0x12002d гибридный (WS00SD)
					//или 0x23002d гибридный (SK00SD)
					//printf("*DS_Param_ = 0x%06X\r\n", *DS_Param_); //v
					*DS_RGB_counter_ = 0;		//номер символа в точке
					*DS_status_ = DS_Type_command_new;
					//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
					return DS_ANS_SET_LED_TYPE;
				}*/
				break; // DS_Type_command_SET_LED_TYPE:
//M90 P... --------------------------------------------------------------------------------------------
//установка генератора случайных чисел
			case DS_Type_command_SET_RND_SEED:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой

					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else { //ошибка
						Error_Message_full('M', 0x90 ,0, 1, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки

					}
					// если мы попали сюда, то значит пора назначить
					*DS_RGB_counter_ = 0;		//номер символа в точке
					//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
					if (DS_Flag_Register&0x20){
						numFileForButton[ButtonNomberCarrier]=*DS_Param_;
						FastCommandForButton[ButtonNomberCarrier]=FC_RANDOM_SEED_M90;
						//CommandArgForButton[ButtonNomberCarrier]=1;
					} else {
						srand(*DS_Param_);
					}/**/
					srand(*DS_Param_);
					return DS_ANS_WS_S_DONE;

				} else {//если цифра
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);

				}

				break;
//-----------------------------------------------------------------------------------------------------------
//присвоение личного ID UI ****\n
			case DS_Type_command_UID:
				// проверка на начало комментария
				/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
					*DS_status_ = DS_Type_command_comment;
					goto UID_set_finish;
//					*DS_RGB_counter_ = 0;		//номер символа в точке
//					return DS_USART_SET_ID;

				} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D){ //если мы попали сюда, значит пора выходить назначив имя
					*DS_status_ = DS_Type_command_new;*/
				if (DS_End_Of_Command_flag){
					*DS_status_ = DS_End_Of_Command_flag;
UID_set_finish:
					*DS_RGB_counter_ = 0;		//номер символа в точке
					return DS_USART_SET_ID;
				} else {
					if (*DS_RGB_counter_>=4){ //уже 4 символа есть и больше не надо
						Error_Message_full('U', 'I' ,0, 0, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
					*DS_RGB_counter_ += 1;
					*DS_Param_=(*DS_Param_ << 8) +DS_current_char;
				}
				break; //DS_Type_command_UID

//-----------------------------------------------------------------------------------------------------------
//назначение бодрейта UxB *****\n
// UxW P... установка максимального времени на приём команды
			case DS_Type_command_USART_BAUD:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
Set_Numeric_Value_To_A_Control_Variable_Label:
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
UxB_set_finish_Label:
						return *DS_comm_num_;
						//return DS_USART_SET_BAUD;
					} else if (DS_current_char == 'P' || DS_current_char == 'H'){ //число в hex формате
						//DS_Flag_Register &=~1;
					} else if (DS_current_char == 'N'){ //число в dec формате
						DS_Flag_Register |=1;
					} else { //ошибка
						if (*DS_comm_num_ == DS_USART_SET_BAUD) {
							DS_a8 = 'B';
						} else if (*DS_comm_num_ == DS_USART_SET_TIMEOUT) {
							DS_a8 = 'W';
						} else if (*DS_comm_num_ == DS_WS_SET_MAX_LENGTH){
							Error_Message_full('S', 'M', 0, 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						} else {//if (*DS_comm_num_ == DS_I2C_SET_ADDRESS) {
							DS_a8 = 'C';
						}
						Error_Message_full('U', DS_a8, 0, 0, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки

					}
				} else {//если цифра
						*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}

				break; //DS_Type_command_USART_BAUD
//--------------------------------------------------------------------------------------------------------------------
//ответы контроллера UA ...
			case DS_Type_command_USART_SPECS:

				if (DS_End_Of_Command_flag){
					*DS_status_ = DS_End_Of_Command_flag;
				//	if (*DS_RGB_counter_ > 1){
				//		USB_main_COM_react();
				//	}
				//	*DS_RGB_counter_ = 0;		//номер символа в точке
					//return DS_ANS_WS_S_DONE;
					return DS_ANS_U1T_DONE;
				} else if (*DS_Param_ == 0){ //первый символ кода
					switch (DS_current_char){
					case 'V'://версия прошивки
							Message ("DKLed v0.7\0",(*DS_RGB_counter_ ));
							goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'T'://Вид контроллера
	#if defined (O6I4U2)
							Message ("I4O6U2\0",(*DS_RGB_counter_ ));
	#endif
	#if defined (O8I5U2)
							Message ("I5O8U2\0",(*DS_RGB_counter_ ));
	#endif
	#if defined (O8I6U2)
							Message ("I6O8U2\0",(*DS_RGB_counter_ ));
	#endif
							goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'F'://текущий файл (0 - текущий, 1 - предыдущий)
						*DS_Param_ = 3;
						break;
					case 'B':
						print_0X4 (DS_LED_Brightness, *DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'C'://color of pixel
						*DS_Param_ = 0;
						DS_WS_line_mask = 0;
						*DS_WSpoint_counter_ = 0;
						*DS_comm_num_ = 0;
						*DS_status_ = DS_Type_command_USART1_COLOR; //будем читать макрос

						break;
					case 'P'://какой по счёту будет выбран файл в команде M98 Q0 P<>
						print_0X4 (ChosenNomberInM98, *DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'U'://сколько отличий насчитали
						print_0X4 (CheckPixel_miscomparations, *DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'S'://количество точек, обновлённое последний раз
						print_0X4 (DS_LastPointsUpdated, *DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'I'://ID
						Port_send_char(DS_Number_to_ASCII(Personal_ID,3),*DS_RGB_counter_);
						Port_send_char(DS_Number_to_ASCII(Personal_ID,2),*DS_RGB_counter_);
						Port_send_char(DS_Number_to_ASCII(Personal_ID,1),*DS_RGB_counter_);
						Port_send_char(DS_Number_to_ASCII(Personal_ID,0),*DS_RGB_counter_);

						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case '_':
						Port_send_char(' ',*DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case '0':
						Port_send_char(0x00,*DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'N':
					//case 'n':
						Port_send_char(0x0A,*DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'R':
					//case 'r':
						Port_send_char(0x0D,*DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 0x5C: //символ "\"
					//case '\':
						Port_send_char(0x5C,*DS_RGB_counter_);
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'M': //servo
						*DS_Param_ = 1;
						break;
					case 'D': //device type
						if ((LED_control_type&127) == 1) {
								Message ("LED\0",(*DS_RGB_counter_ ));
						} else if ((LED_control_type & 3) == 2) {
								Message ("SRV\0",(*DS_RGB_counter_ ));
						} else if ((LED_control_type & 3) == 3) {
								Message ("HBR\0",(*DS_RGB_counter_ ));
						} else if ((LED_control_type&127) == 4) {
								Message ("FST\0",(*DS_RGB_counter_ ));
						}
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					case 'O': //output regime
						*DS_Param_ = 2;
						break;
					case 'A': //animation parameters
						*DS_Param_ = 4;
						break;
					case 'H'://hex to символ
						*DS_Param_ = 5;
						break;
					default:
						Message ("(O.o)\r\0",(*DS_RGB_counter_ ));
						goto DS_Type_command_USART_SPECS_answer_complete;//break;
					} //switch DS_current_char

				} else if (*DS_Param_ == 1) { //второй символ кода - для сервов
					DS_a8 = DS_chartoint (DS_current_char);
					if (DS_a8 < 8){
						*DS_Param_ = Servo_Pos_Temp[DS_a8];
						print_0X4 ((uint16_t)*DS_Param_, *DS_RGB_counter_ );
					}
					goto DS_Type_command_USART_SPECS_answer_complete;
				} else if (*DS_Param_ == 2) { //второй символ кода - для режима выводов
					DS_a8 = DS_chartoint (DS_current_char);
					if (DS_a8 < 8){
						if ((LED_control_type & 3) == 3){

								if ((WS2812_IO_High & (1 << DS_a8)) && ((~WS2812_IO_Low) & (1 << DS_a8))) {
									Port_send_char('L',*DS_RGB_counter_);
								} else if (Servo_Action_Mask & (1 << DS_a8)) {
									Port_send_char('S',*DS_RGB_counter_);
								} else {
									Port_send_char('N',*DS_RGB_counter_);
								}

						} else if ((LED_control_type & 3) == 2){
							if (Servo_Action_Mask & (1 << DS_a8)) {
								Port_send_char('S',*DS_RGB_counter_);
							} else {
								Port_send_char('N',*DS_RGB_counter_);
							}
						} else {// if (LED_control_type == 1 || LED_control_type == 4){
							Port_send_char('L',*DS_RGB_counter_);
						}
					}
					goto DS_Type_command_USART_SPECS_answer_complete;
				} else if (*DS_Param_ == 3) { //второй символ кода - для файлов
					if (DS_current_char == '0'){
						print_0X4 (Current_File_Num, *DS_RGB_counter_ );
					} else if (DS_current_char == '1') {
						print_0X4 (Prev_File_Num, *DS_RGB_counter_ );
					} else {
						print_0X4 (Parent_File_Num, *DS_RGB_counter_ );
					}
					goto DS_Type_command_USART_SPECS_answer_complete;
				} else if (*DS_Param_ == 4) {//второй символ кода - для параметров анимации
					if (DS_current_char == 'P'){//period
						print_0X4 (WS2812_Frame_Period_Actual, *DS_RGB_counter_ );
					} else if (DS_current_char == 'U') {//update frequency
						print_0X4 (((uint16_t) (SERVO_TICK_FREQ / WS2812_Frame_Period_Actual)), *DS_RGB_counter_ );
					} else if (DS_current_char == 'F') {//number of frames
						print_0X4 (WS2812_Frame_Count_Actual, *DS_RGB_counter_ );
					} else if (DS_current_char == 'L') {//length in pixels per output
						print_0X4 (WS2812_Frame_Length_Actual, *DS_RGB_counter_ );
					} else if (DS_current_char == 'C') {//current frame
						print_0X4 (((uint16_t)(WS2812_Frame_Start_Pointer_Actual / WS2812_Frame_Byte_Length_Actual)), *DS_RGB_counter_ );
					} else if (DS_current_char == 'T') {//total frames left
						print_0X4 ((WS2812_Frame_Total_Count-1), *DS_RGB_counter_ );
					}

					goto DS_Type_command_USART_SPECS_answer_complete;
				} else if (*DS_Param_ == 6) {//2nd num of HEX symbol
					DS_WS_line_mask = (DS_WS_line_mask<<4) + DS_chartoint (DS_current_char);
					Port_send_char(DS_WS_line_mask,*DS_RGB_counter_);
					goto DS_Type_command_USART_SPECS_answer_complete;
				} else if (*DS_Param_ == 5) {//1 st num of HEX symbol
					DS_WS_line_mask = DS_chartoint (DS_current_char);
					*DS_Param_ = 6;
				}
			break; //DS_Type_command_USART_SPECS


DS_Type_command_USART_SPECS_answer_complete:
			*DS_Param_ = 0;
			*DS_status_ = DS_Status_backup;
			break;
//---------------------------------------------------------------------------------------------
// UxW P... установка максимального времени на приём команды
/*			case DS_Type_command_USART_TIMEOUT:

				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
						return DS_USART_SET_TIMEOUT;
					} else if (DS_current_char == 'P' || DS_current_char == 'H'){ //число в hex формате
						//DS_Flag_Register &=~1;
					} else if (DS_current_char == 'N'){ //число в dec формате
						DS_Flag_Register |=1;
					} else { //ошибка
						Error_Message_full('U', 'W',0, 1, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

				} else {//если цифра
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);

				}
				break;/**/
//-----------------------------------------------------------------------------------------------------------
//вывод данных через USART1
//вывод данных через DBGU U2T P***\n - определение типа P/H
			case DS_Type_command_USART1_OUT:
			//case DS_Type_command_USART2_OUT:
				//принято "SR", ожидаем букву Q, если пришла не она - ошибка
				//*DS_comm_num_ используется как флаг состояния приема: ==4 - ждём букву Q, ==5 - ждём значение параметра Q
				//*DS_Param_ - значение параметра Q, сколько символов отправляем
				//*DS_WSpoint_counter_ - номер текущей принимаемой точки, счетчик для расшифровки параметра P
Data_Out_Trough_Ports_Format_Choice_Label:

					// DS_a8 = DS_chartoint (DS_current_char);
						if (DS_current_char == 'P'){
							*DS_WSpoint_counter_=0;
							*DS_status_ = DS_Type_command_USART1_SEND;// +(*DS_status_ - DS_Type_command_USART1_OUT);
							DS_Status_backup = *DS_status_;
						} else if (DS_current_char == 'H'){
							*DS_comm_num_=0;
							*DS_WSpoint_counter_=0;
							*DS_status_ = DS_Type_command_USART1_SEND_HEX;// + (*DS_status_ - DS_Type_command_USART1_OUT);//отправка в ХЕКС

						} else { //ждем только P или цифру для Q, если не P - значит ошибка
							DS_a8 = (*DS_RGB_counter_ == I2C_OUT_LABEL)?'C':'T';
							Error_Message_full('U', DS_a8,1, 0, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL;
						}
				break; //DS_Type_command_USART2_OUT
//-------------------------------------------------------------------------------------------
//вывод данных через DBGU U2T P***\n - сама отправка
			case DS_Type_command_USART1_SEND:
			//case DS_Type_command_USART2_SEND:
				// проверка на конец строки
				//else
				if(DS_current_char == '\n' || DS_current_char == 0x0D || DS_current_char == 0)  {
					//DS_a8 = DS_ANS_U1T_DONE;// + *DS_RGB_counter_;//*DS_status_ - DS_Type_command_USART1_SEND;
					*DS_status_ = DS_Type_command_new;
					//	*DS_buf_start_ = DS_i + 1;
					//return DS_a8;
					return DS_ANS_U1T_DONE;
				}
				if (DS_current_char == 0x5C){//спецсимвол '\'
					//*DS_buf_start_ = DS_i + 1;
					//DS_i ++;
					//DS_current_char = DS_buffer_[DS_i];
					DS_Status_backup = *DS_status_;
					*DS_status_ = DS_Type_command_USART_SPECS;
					*DS_Param_ = 0;
					break;
					/*if (DS_current_char == 'n' || DS_current_char == 'N' ){
						//*DS_comm_num_ = 0x0A;
						DS_current_char = 0x0A;
					} else if (DS_current_char == 'r' || DS_current_char == 'R' ){
						//*DS_comm_num_ = 0x0D;
						DS_current_char = 0x0D;
					} else if (DS_current_char == 'c' || DS_current_char == 'C' ){ //C<line - 0...F><position 000...200>
						//*DS_comm_num_ = 0x0D;
						*DS_Param_ = 0;
						DS_WS_line_mask = 0;
						*DS_WSpoint_counter_ = 0;
						*DS_comm_num_ = 0;
						*DS_status_ += DS_Type_command_USART1_COLOR - DS_Type_command_USART1_SEND; //будем читать макрос
						break;
					} /*else if (DS_current_char == 'f' || DS_current_char == 'F' ){ //Filename
						//*DS_comm_num_ = 0x0D;
						Message (fname,(*DS_status_ - DS_Type_command_USART1_SEND));
						break;
					} else if (DS_current_char == 'd' || DS_current_char == 'D' ){ //Filename
						//*DS_comm_num_ = 0x0D;
						Message (Dirpath,(*DS_status_ - DS_Type_command_USART1_SEND));
						break;
					}/*else {;
						*DS_comm_num_ = 0x5C;
					}*/
				}

				Port_send_char(DS_current_char,*DS_RGB_counter_);
				break;

			case DS_Type_command_USART1_COLOR: //дешифруем операнд C<line - 0...F><position 000...200>
			//case DS_Type_command_USART2_COLOR:
				//надо убрать отсюда использование *DS_comm_num_ и *DS_RGB_counter_


				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 != 0xFF){

					if (*DS_comm_num_){

						*DS_WSpoint_counter_ = DS_hex_add_symbol(DS_a8,*DS_WSpoint_counter_);
						*DS_comm_num_+=1;

						if (*DS_comm_num_ > 3){
							//Message("p",1);

							*DS_status_ = DS_Status_backup;//*DS_status_ = *DS_status_ + DS_Type_command_USART1_SEND - DS_Type_command_USART1_COLOR; //макрос дочитан
							if (*DS_WSpoint_counter_ >= WS2812_IO_FRAMEDATA_PIXELS){*DS_WSpoint_counter_ = WS2812_IO_FRAMEDATA_PIXELS - 1;}
							*DS_WSpoint_counter_ *= 24;
							for (DS_a8 = 0; DS_a8 < 24; DS_a8++  ){
								*DS_Param_ <<= 1;
								if (WS2812_IO_framedata_[*DS_WSpoint_counter_] & DS_WS_line_mask){
									*DS_Param_ |=1;
								}
								*DS_WSpoint_counter_+=1;
							}
							//print_0X4(*DS_Param_>>16,1);
							//print_0X4(*DS_Param_,1);
							//
							*DS_comm_num_ = (*DS_Param_ & 0xFF00)>>8;
							*DS_Param_ = (*DS_Param_&0xFF)+((*DS_Param_ & 0xFF0000)>>8); //swap back red and green

							Port_send_char(DS_inttochar((*DS_comm_num_>>4)&0x0F),*DS_RGB_counter_);
							Port_send_char(DS_inttochar(*DS_comm_num_&0x0F),*DS_RGB_counter_);


							//Message("c",1);
							print_0X4(*DS_Param_,*DS_RGB_counter_);
							*DS_Param_ = 0;
							*DS_comm_num_ = 0;
						}

					} else { //*DS_comm_num_ = 0 - первая цифра - номер вывода
						//Message("l",1);
						//print_0X4(*DS_RGB_counter_,1);
						if (DS_a8<8){DS_WS_line_mask = 1<<DS_a8;} //маска выводов

						*DS_comm_num_ = 1;
					}

				} else {//преждевременный выход, чтобы случайно не зависнуть и не выдать следующую команду
					if(DS_current_char == '\n' || DS_current_char == 0x0D || DS_current_char == 0)  {
						//DS_a8 = DS_ANS_U1T_DONE;// + *DS_RGB_counter_;//*DS_status_ - DS_Type_command_USART1_COLOR;
						*DS_status_ = DS_Type_command_new;
						//	*DS_buf_start_ = DS_i + 1;
						return DS_ANS_U1T_DONE;
					}
				}

				break;
//------------------------------------------------------------------------------------------------------------
//вывод данных через DBGU U2T H***\n - сама отправка
			case DS_Type_command_USART1_SEND_HEX:
			//case DS_Type_command_USART2_SEND_HEX:
				DS_a8 = DS_chartoint (DS_current_char);
					//printf ("DS_i = %d, DS_current_char = %d, DS_a8 = %d\r\n", DS_i, DS_current_char, DS_a8);
					if (DS_a8 == 0xFF){ //если принятый символ не цифровой
						// проверка на начало комментария
						//DS_a8 = DS_ANS_U1T_DONE;// + *DS_RGB_counter_;//*DS_status_ - DS_Type_command_USART1_SEND_HEX;
						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
						} else {
							DS_a8 = (*DS_RGB_counter_ == I2C_OUT_LABEL)?'C':'T';
							Error_Message_full('U', DS_a8,1, 2, 1,DS_i, DS_current_char);
							return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
						}
						//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
						return DS_ANS_U1T_DONE; //DS_ANS_U1T_DONE + *DS_status_ - DS_Type_command_USART1_SEND_HEX
					} else {
						*DS_comm_num_ = DS_hex_add_symbol(DS_a8, *DS_comm_num_);
						if (*DS_WSpoint_counter_ & 1) {
							Port_send_char(*DS_comm_num_,*DS_RGB_counter_);
						}
						++*DS_WSpoint_counter_;
					}
				break;
//-----------------------------------------------------------------------------------------------------------
//setting the address of I2C receiver UC Q<address> P/H/N
			case DS_Type_command_I2C_Send_to:
				DS_a8 = DS_chartoint (DS_current_char);
				//DS_Type_command_USART1_OUT
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					// если буфер занят, подождать освобождения
					//uint16_t ic = 50000;
					//while ((I2C_stage <0x7f) && (ic)) ic--;
					//I2C_stage = I2C_stage;
					//I2C_Bytes_To_Send = I2C_Bytes_To_Send;
					I2C_TargetAddress = ((uint8_t) *DS_Param_)&0x7F; //записываем адрес устройства-получателя
					if (DS_current_char == 'Q'){//будем осуществлять сложное чтение по определённому адресу из I2C
						//I2C_Bytes_To_Send = 0;
						DS_Flag_Register|=3;

					} else if (DS_Flag_Register & 2){//DS_current_char == 'R') {//Будем читать из 2C
						*DS_Param_ = 0;
						*DS_status_ =  DS_Type_command_USART_BAUD;
						*DS_comm_num_ = DS_I2C_GET_FROM + (DS_Flag_Register&1);//простое или сложное чтение
						DS_Flag_Register &=~3;
						goto Set_Numeric_Value_To_A_Control_Variable_Label;
					} else {
						goto Data_Out_Trough_Ports_Format_Choice_Label;//и дфльше у нас действия как при обычной отправке двнных
					}
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					if (DS_Flag_Register&1){
						*DS_WSpoint_counter_ = DS_hex_add_symbol(DS_a8, *DS_WSpoint_counter_);
						I2C_Bytes_To_Send ++;
					}else{
						*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
					}
				}
				break;
//--------------------------------------------------------------------------------------------
//G43 P... Set MinPos, G44 P... Set MaxPos, G45 P... Set Resolution, G50 P... Set Period
			case DS_Type_command_SERVO_PARAMS:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					// проверка на начало комментария
					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('G', *DS_comm_num_ ,0, 1, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

					// если новый символ был не цифровой, но и не ошибочный

					// значение  передаем через *DS_Param_, какой параметр изменяем - через *DS_comm_num_
					//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					return DS_ANS_SERVO_SET_PARAM; //назначаем параметр
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}
				break;
//----------------------------------------------------------------------------------------------------------
//G29 P... start frame G30 P...  frame frequaency G31 P... frame length G32 P... frames in anim; G26
			case DS_Type_command_MULTI_SET:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
						//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
						return DS_ANS_MULTI_SET_PARAM; //назначаем параметр
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
						//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
						return DS_ANS_MULTI_SET_PARAM; //назначаем параметр
					} else if (DS_current_char == '-'){//отрицательное число
						DS_Flag_Register |=0xC; //8+4
					} else if (DS_current_char == '+'){//положительное число
						DS_Flag_Register |=8;
						DS_Flag_Register &=~4;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('G', *DS_comm_num_ ,0, 1, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

					// если новый символ был не цифровой, но и не ошибочный

					// значение  передаем через *DS_Param_, какой параметр изменяем - через *DS_comm_num_
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}
				break;

//---------------------------------------------------------------------------------------------------------------
//M47P... set the number of repetitions of the file
			case DS_Type_command_REPEAT_FILE:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой
					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('M', 0x47 ,0, 3, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
					// если новый символ был не цифровой, но и не ошибочный
					// значение  передаем через *DS_Param_
					//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					if (DS_Flag_Register&0x20){
						numFileForButton[ButtonNomberCarrier]=*DS_Param_;
						FastCommandForButton[ButtonNomberCarrier]=FC_REPEAT_FILE_M47;
						CommandArgForButton[ButtonNomberCarrier]=1;
						return DS_ANS_WS_S_DONE;
					} //else {
					//	return DS_ANS_REPEAT_FILE;
					//}/**/
					return DS_ANS_REPEAT_FILE; //назначаем параметр
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_num_add_symbol(DS_a8, *DS_Param_);
				}
				break;
//-------------------------------------------------------------------------------------------------------------
//F1 P...
			case DS_Type_command_FILEWORKS_CONT:
				//у нас 2 варианта: первый объект и следующий объект по кругу
				//начала проверка на конец процесса
				/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
					*DS_status_ = DS_Type_command_comment;
					*DS_RGB_counter_ = 0;
					return DS_ANS_WS_S_DONE;
				} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
					*DS_status_ = DS_Type_command_new;*/
				if (DS_End_Of_Command_flag){
					*DS_status_ = DS_End_Of_Command_flag;
					*DS_RGB_counter_ = 0;
					return DS_ANS_WS_S_DONE;
				}
				//фактический вывод инфы
//				if (LED_control_type&128){
					if (DS_current_char=='0'){ //читаем первый объект в папке
						//*DS_Param_ = 0;
						DS_current_char = '1';
						f_opendir (&dir, FileworksDirpath );
					}
					if (DS_current_char=='1'){//читаем следующий объект в папке
						//*DS_Param_ = 1;

						FRESULT result = f_readdir (&dir,  &filinfo );
						if (!filinfo.fname[0]) {
							Message ("EOD\0",*DS_RGB_counter_);
							f_opendir (&dir, FileworksDirpath );
						} else {
							if (result != FR_OK) {
								Fail_Message(2,1,result);
							} else {//открываем
								if (filinfo.fattrib & AM_ARC) {
									Message ("F:\0",*DS_RGB_counter_);
								} else if (filinfo.fattrib & AM_DIR) {
									Message ("D:\0",*DS_RGB_counter_);
								}
								Message (filinfo.fname,*DS_RGB_counter_);
							}
						}
					} else if (DS_current_char!='N'){
						*DS_status_ = DS_Type_command_comment;
						//значит ошибка
						Error_Message_full('F', 1,0, 3, 1,DS_i, DS_current_char);

						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}

					Finish_Fileworks_Message(*DS_RGB_counter_);
					//мы не изменяем *DS_status_ потому что мы можем ещё раз сюда попасть
					//увы, за пределами debugtask переменные файловой системы ведут мебя странно
					//где-то они переопределяются
					//*DS_buf_start_ = DS_i + 1;

					//dbgu_rx_buf_overcount |=4;


					//return DS_ANS_FILEMODE_READDIR;

				break;	//DS_Type_command_FILEWORKS_CONT

				//break;
//-------------------------------------------------------------------------------------------------------------------------
//M23 P<dir>, F2 P<dir>, F3 P <dir>, F4 P <dir>,F5 P <dir>
			case DS_Type_command_FILE_SETDIR:
				/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
					*DS_status_ = DS_Type_command_comment;
					return *DS_comm_num_; //отправляем результат в соответствующую ветку
				} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
					*DS_status_ = DS_Type_command_new;*/
				if (DS_End_Of_Command_flag){
					*DS_status_ = DS_End_Of_Command_flag;
					//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
					return *DS_comm_num_;//отправляем результат в соответствующую ветку
				} else {//всё, что сюда попало добавляем
					if (*DS_RGB_counter_ >= FILE_NAME_MAX_LENGTH) { //короткое имя папки, предел - символы с 0 по 7
						if (*DS_comm_num_ == DS_ANS_NORMALMODE_SETDIR) {
							Error_Message_full('M', 0x23,0, 3, 3,DS_i, DS_current_char);
						} else {
							Error_Message_full('F', (*DS_comm_num_ + 2 - DS_ANS_FILEMODE_SETDIR),0, 3, 3,DS_i, DS_current_char);
						}
						return DS_ERR_TOO_MANY_SYMBOLS;
					}
					fname[*DS_RGB_counter_] = DS_current_char;
					*DS_RGB_counter_ +=1;
				}
				break;
//----------------------------------------------------------------------------------------------------------------------------
//F10 [Q<pionter>]P/N<number of bytes>, F11 [Q<pointer>]P/H<data> читаем Q
			case DS_Type_command_FILE_READ_SET:
				//*DS_status_ =  DS_Type_command_FILE_READ_PART + *DS_comm_num_;
				//определить DS_Param
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){ //если принятый символ не цифровой, пора перейти в следующий сегмент команды
					if (DS_current_char == 'P') {
						//DS_Flag_Register &=~1; //in HEX
						*DS_RGB_counter_ = 0; //строчка в байтах
					} else if (DS_current_char == 'N') { //только для чтения
						DS_Flag_Register |=1; //in DEC
						*DS_RGB_counter_ = 1; //строчка в байтах
					} else if (DS_current_char == 'H') {
						//DS_Flag_Register &=~1; //in HEX
						*DS_RGB_counter_ = 1; //строчка в HEXкодах
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('F', (*DS_comm_num_ - DS_Type_command_FILE_READ_PART + 8),1, 0, 1,DS_i, DS_current_char);

						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
					// если новый символ был не цифровой, но и не ошибочный
					// значение  передаем через *DS_Param_
					*DS_status_ = *DS_comm_num_;
					//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места

				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_Param_ = DS_hex_add_symbol(DS_a8, *DS_Param_);
				}

				break; //DS_Type_command_FILE_READ_SET
//-----------------------------------------------------------------------------------------------------------------------------
//F10 [Q<pionter>]P/N<number of bytes> читаем P/N
			case DS_Type_command_FILE_READ_PART:
				DS_a8 = DS_chartoint (DS_current_char);
				if (DS_a8 == 0xFF){
					/*if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
						*DS_status_ = DS_Type_command_comment;
					} else if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
						*DS_status_ = DS_Type_command_new;*/
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
					} else { //если не конец строки и не коммент - значит ошибка
						Error_Message_full('F', 8,1, 1, 1,DS_i, DS_current_char);
						return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
					}
					// если новый символ был не цифровой, но и не ошибочный
					// значение  передаем через *DS_WSpoint_counter_
					//	*DS_buf_start_ = DS_i + 1; // начало следующей дешифровки - с текущего места
					return DS_ANS_FILEMODE_READFILE; //отправляемся читать
				} else {  //если сюда дошли - добавляем свежеполученный символ в параметр и продолжаем расшифровку
					*DS_WSpoint_counter_ = DS_num_add_symbol(DS_a8, *DS_WSpoint_counter_);
					if (*DS_WSpoint_counter_ > READ_SIZE) {
						Error_Message_full('F', 8,0, 1, 2,DS_i, DS_current_char);
						return DS_ERR_CODE_OUT_OF_RANGE;
					}
				}

				break; //DS_Type_command_FILE_READ_PART
//-----------------------------------------------------------------------------------------------------------------------------
//F11 [Q<pointer>]P/H<data> читаем P/H
			case DS_Type_command_FILE_WRITE_PART:

				if (*DS_RGB_counter_ && (~DS_Flag_Register & 1)){ //строчка в HEXкодах
					DS_a8 = DS_chartoint (DS_current_char);
					if (DS_a8 == 0xFF){
						//if (DS_current_char == DS_Symbol_comment1 || DS_current_char == DS_Symbol_comment2){
							*DS_status_ = DS_Type_command_comment;
						/*} else*//* if(DS_current_char == 0 || DS_current_char == '\n' || DS_current_char == 0x0D)  {
							*DS_status_ = DS_Type_command_new;
						}*/
						if (DS_End_Of_Command_flag){
							*DS_status_ = DS_End_Of_Command_flag;
						}
						//*DS_RGB_counter_ = 0;		//номер символа в точке
						//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
						*DS_WSpoint_counter_ = *DS_WSpoint_counter_>>1;
						CRC_sum = crc32_byte(CRC32_INIT, text_buff, *DS_WSpoint_counter_);
						return DS_ANS_FILEMODE_WRITEFILE;

					} else { //байты
						*DS_comm_num_ = (uint8_t) DS_hex_add_symbol(DS_a8, *DS_comm_num_);
							text_buff[*DS_WSpoint_counter_>>1] = *DS_comm_num_;
							*DS_WSpoint_counter_ +=1; //номер в массиве
							if ((*DS_WSpoint_counter_>>1) >= READ_SIZE) {
								Error_Message_full('F', 9,1, 2, 3,DS_i, DS_current_char);
								return DS_ERR_TOO_MANY_SYMBOLS;
							}
					}

				} else if ((*DS_RGB_counter_ == 0) && (~DS_Flag_Register & 1)){ //строчка в байтах
					// символы \r \n \0- конец строки
					//если надо их ввести, то пишем '\' + 'r', '\'+'n', '\'+'0' и '\'+'\' для простого слеша
					if (DS_End_Of_Command_flag){
						*DS_status_ = DS_End_Of_Command_flag;
						return DS_ANS_FILEMODE_WRITEFILE;
					} else if (DS_Status_backup == 3){//'\hxx', вторая цифра
						*DS_comm_num_ = (*DS_comm_num_<<4) + DS_chartoint(DS_current_char);
						DS_Status_backup=0;
						*DS_WSpoint_counter_ -=1;
					} else if (DS_Status_backup == 2){//'\hxx', первая цифра
						*DS_comm_num_ = DS_chartoint(DS_current_char);
						DS_Status_backup=3;
						*DS_WSpoint_counter_ -=1;
					} else if (DS_Status_backup == 1){//второй символ после спецсимвола '\'
						DS_Status_backup=0;
						DS_current_char = DS_buffer_[DS_i];
						*DS_comm_num_ = DS_current_char;
						//CRC32_singleByte (&CRC_sum, &DS_current_char);
						if (DS_current_char == 'n'){
							*DS_comm_num_ = 0x0A;
						} else if (DS_current_char == 'r'){
							*DS_comm_num_ = 0x0D;
						} else if (DS_current_char == '0'){
							*DS_comm_num_ = 0x00;
						} else if (DS_current_char == 'h'){
							DS_Status_backup=2;
						}
						*DS_WSpoint_counter_ -=1;
					} else if (DS_current_char == 0x5C){//спецсимвол '\'
						//CRC32_singleByte (&CRC_sum, &DS_current_char);
						DS_Status_backup=1;
					} else {
						*DS_comm_num_ = DS_current_char;
					}
					CRC32_singleByte (&CRC_sum, &DS_current_char);
					text_buff[*DS_WSpoint_counter_] = *DS_comm_num_;
					*DS_WSpoint_counter_ +=1; //номер в массиве
					if (*DS_WSpoint_counter_ >= READ_SIZE) {
						Error_Message_full('F', 9,1, 3, 3,DS_i, DS_current_char);
						return DS_ERR_TOO_MANY_SYMBOLS;
					}

				} else { //ошибка
					Error_Message_full('F',9,1, 0, 1,DS_i, DS_current_char);
					return DS_ERR_UNEXPEXTED_SYMBOL; // если это не конец строки
				}



				break; //DS_Type_command_FILE_WRITE_PART
//---------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//простые включения/выключения

//включение/отключение дебаггера на USART2 (DBGU) UD *\n
//включение/отключение приёма UxR <1/0>\n
//включение/отключение ожидания конца вывода UxF ... *\n
//G36 P <1/0> запуск/остановка анимации
//G1 P<1/0> запуск отслеживания позиций серв в реальном времени
//F0 P...
			case DS_Type_command_SWITCH_ONOFF:
				//*DS_comm_num_ - в какую ветку дебагтаска идти
				//*DS_Param_ от 0 до 0xF, или 0xFF


				if (DS_End_Of_Command_flag){
					*DS_status_ = DS_End_Of_Command_flag;
					//	*DS_buf_start_ = DS_i + 1;  //начало следующего прохода дешифровки
					return *DS_comm_num_;
				}
				*DS_Param_=DS_chartoint (DS_current_char); //всё кроме "0" - это вкл

				 break;
//--------------------------------------------------------------------------------------------------------------------
			default:
				return DS_ERR_UNKNOWN_ERROR;
				break;

		}

	} //DS_i++;

	// если сюда дошло - значит расшифрован буфер до конца, но команда еще не кончилась
	// требуется чтение новой порции из файла и расшифровка дальше
	//printf("Descript return DS_ANS_READ_ON;");

	return DS_ANS_READ_ON;
}

// преобразование текста 0123...abcdef в число
uint8_t DS_chartoint (char c)
{
	if ('0'<=c && c<='9') return (uint8_t)(c-'0');
	if ('A'<=c && c<='F') return (uint8_t)(c-'A'+10);
	if ('a'<=c && c<='f') return (uint8_t)(c-'a'+10);
	return 0xFF;
}

//преобразование цифры в текст
char DS_inttochar (uint8_t c)
{
	if (0<=c && c<=9) return (uint8_t)(c+'0');
	if (0xA<=c && c<=0xF) return (uint8_t)(c-10+'A');
	//if ('a'<=c && c<='f') return (uint8_t)(c-'a'+10);
	return 0xFF;
}

//вывод 16-чного числа из 4 цифр
//par - выводимое число
//c_hex ==0 на 1 порт, ==1 на 2 порт
void print_0X4 (uint16_t par, uint8_t c_hex)
{
	Port_send_char(DS_inttochar((par>>12)&0x0F),c_hex);
	Port_send_char(DS_inttochar((par>>8)&0x0F),c_hex);
	Port_send_char(DS_inttochar((par>>4)&0x0F),c_hex);
	Port_send_char(DS_inttochar(par&0xF),c_hex);

}

void print_DEC_4 (uint16_t par, uint8_t c_hex)
{
	uint16_t temp=par/1000;

	if (temp>0){
		Port_send_char(('0'+temp),c_hex);
	}
	par -= temp*1000;
	temp = par/100;
	if (temp>0){
		Port_send_char(('0'+temp),c_hex);
	}
	par -= temp*100;
	temp = par/10;
	if (temp>0){
		Port_send_char(('0'+temp),c_hex);
	}
	par -= temp*10;
	if (par>0){
		Port_send_char(('0'+par),c_hex);
	}

}

//Вывод информации об ошибке
//a - буква
//	0 новая команда
//	A...Z - буквы
//b - номер команды (Current_char)
//	для с='U':
//	0xFF=..; >0= символ
//	для с='M', 'G':
//	0xFF=..; >0= число
//c - показывать 1-Q; 2-QR
//d - показывать 1-P/N; 2-H; 3-P
//c_hex - код типа ошибки
//	1 Unexpected symbol
//	2 out of range
//	3 too many symbols
//	4 command not finished
//par - номер символа в строке (DS_i)
//c - проблемный символ (Current_char)
void Error_Message_full(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t c_hex, uint16_t par, uint8_t e){
	if (DebugInfoOutFlag){

		if (a == 0){
			Message ("New command\0",DEBUG_PORT_OUT);
		} else {
			Port_send_char(a,DEBUG_PORT_OUT);
			if (b == 0xFF){
				//dbgu_send_char('.');
				//dbgu_send_char('.');
				Port_send_char('.',DEBUG_PORT_OUT);
				Port_send_char('.',DEBUG_PORT_OUT);
			} else if (a == 'U'){
				//dbgu_send_char('x');
				//dbgu_send_char(b);
				Port_send_char('x',DEBUG_PORT_OUT);
				Port_send_char(b,DEBUG_PORT_OUT);
			} else if (a == 'S'){
				Port_send_char(b,DEBUG_PORT_OUT);
			} else {
				Port_send_char(DS_inttochar((b>>4)&0x0F),DEBUG_PORT_OUT);
				Port_send_char(DS_inttochar(b&0xF),DEBUG_PORT_OUT);
			}

		}

		//dbgu_send_char(' ');
		Port_send_char(' ',DEBUG_PORT_OUT);

		if (c==1){
			Message ("Q \0",DEBUG_PORT_OUT);
		} else if (c==2){
			Message ("QR \0",DEBUG_PORT_OUT);
		}

		if ( (d == 1) && ((DS_Flag_Register & 3)== 1) ) {
			//dbgu_send_char('N');
			Port_send_char('N',DEBUG_PORT_OUT);
		} else if (d == 2){
			//dbgu_send_char('H');
			Port_send_char('H',DEBUG_PORT_OUT);
		} else if (d > 0) {
			//dbgu_send_char('P');
			Port_send_char('P',DEBUG_PORT_OUT);
		}

		if (c_hex == 1){
			Message (" Unexpected symbol\0",DEBUG_PORT_OUT);
		} else if (c_hex == 2){
			Message (" Code out of range\0",DEBUG_PORT_OUT);
		} else if (c_hex == 3){
			Message (" Too many Symbols\0",DEBUG_PORT_OUT);
		} else if (c_hex == 4){
			Message (" Unfinished\0",DEBUG_PORT_OUT);
		} else if (c_hex == 5){
			Message (" Not ready\0",DEBUG_PORT_OUT);
		}
		if (e){
			Message (": #\0",DEBUG_PORT_OUT);
			Port_send_char(DS_inttochar((e>>4)&0x0F),DEBUG_PORT_OUT);
			Port_send_char(DS_inttochar(e&0xF),DEBUG_PORT_OUT);
			Message (" (\0",DEBUG_PORT_OUT);
			Port_send_char(e,DEBUG_PORT_OUT);
			par++;
			if (DS_Channel_Select == 1){
				Message ("), file \0",DEBUG_PORT_OUT);
				print_0X4 (Current_File_Num,DEBUG_PORT_OUT);
				Message (", string 0x\0",DEBUG_PORT_OUT);
				print_0X4 (String_in_file_Index,DEBUG_PORT_OUT);
				par = Char_in_string_index;
				//print_0X4 (Char_in_string_index,DEBUG_PORT_OUT);
			} else if (DS_Channel_Select==2) {//UART 1
				Message ("), UART 1\0",DEBUG_PORT_OUT);
			} else if (DS_Channel_Select==3) {//UART 2
				Message ("), UART 2\0",DEBUG_PORT_OUT);
			} else if (DS_Channel_Select==4) {//USB
				Message ("), USB\0",DEBUG_PORT_OUT);
			}
			Message (", pos 0x\0",DEBUG_PORT_OUT);
			print_0X4 (par,DEBUG_PORT_OUT);		//позиция проблемного символа

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

/*
//Вывод информации об ошибке
//c_hex - код типа ошибки
//1 Unexpected symbol
//2 out of range
//3 too many symbols
//4 command not finished
//par - номер символа в строке (DS_i)
//c - проблемный символ (Current_char)
void Error_Message_data(uint8_t c_hex, uint16_t par, uint8_t c)
{
	if (DebugInfoOutFlag){

		if (c_hex == 1){
			Message ("Unexpected symbol\0",DEBUG_PORT_OUT);
		} else if (c_hex == 2){
			Message ("Code out of range\0",DEBUG_PORT_OUT);
		} else if (c_hex == 3){
			Message ("Too many Symbols\0",DEBUG_PORT_OUT);
		} else if (c_hex == 4){
			Message ("Command unfinished\0",DEBUG_PORT_OUT);
		}
		//dbgu_send_char(':');
		Port_send_char(':',DEBUG_PORT_OUT);
		//dbgu_send_char(' ');
		Port_send_char(' ',DEBUG_PORT_OUT);
		//dbgu_send_char('#');//код проблемного символа
		Port_send_char('#',DEBUG_PORT_OUT);
		//dbgu_send_char(DS_inttochar((c>>4)&0x0F));
		//dbgu_send_char(DS_inttochar(c&0xF));
		//dbgu_send_char(' ');
		Port_send_char(DS_inttochar((c>>4)&0x0F),DEBUG_PORT_OUT);
		Port_send_char(DS_inttochar(c&0xF),DEBUG_PORT_OUT);
		Port_send_char(' ',DEBUG_PORT_OUT);
		//dbgu_send_char('(');
		//dbgu_send_char(c);			//сам проблемный символ
		Port_send_char('(',DEBUG_PORT_OUT);
		Port_send_char(c,DEBUG_PORT_OUT);
		Message (") at 0x\0",DEBUG_PORT_OUT);
		print_0X4 (par,DEBUG_PORT_OUT);		//позиция проблемного символа
		//dbgu_send_char(0x0D);
		//dbgu_send_char(0x0A);
		Port_send_char(0x0D,DEBUG_PORT_OUT);
		//Port_send_char(0x0A,1);

		//printf("DS_i= %d, DS_current_char = 0x%02X\r\n", DS_i, DS_current_char);
	}

}
*/

//Вывод информации об ошибке
//c - буква
//0 новая команда
//A...Z - буквы
//c_hex - номер команды (Current_char)
//для с='U':
//0xFF=..; >0= символ
//для с='M', 'G':
//0xFF=..; >0= число
void Error_Message_Command(uint8_t c, uint8_t c_hex)
{
	if (DebugInfoOutFlag){

		if (c == 0){
			Message ("New command\0",DEBUG_PORT_OUT);
		} else if (c_hex == 'U'){
			//dbgu_send_char('U');
			Port_send_char('U',DEBUG_PORT_OUT);
			if (c_hex == 0xFF){
				//dbgu_send_char('.');
				//dbgu_send_char('.');
				Port_send_char('.',DEBUG_PORT_OUT);
				Port_send_char('.',DEBUG_PORT_OUT);
			} else {
				//dbgu_send_char('x');
				Port_send_char('x',DEBUG_PORT_OUT);
				//dbgu_send_char(c_hex);
				Port_send_char(c_hex,DEBUG_PORT_OUT);
			}
		} else if (c_hex == 'S'){
			//dbgu_send_char('S');
			Port_send_char('S',DEBUG_PORT_OUT);
			if (c_hex == 0xFF){
				//dbgu_send_char('.');
				//dbgu_send_char('.');
				Port_send_char('.',DEBUG_PORT_OUT);
				Port_send_char('.',DEBUG_PORT_OUT);
			} else {
				//dbgu_send_char(c_hex);
				Port_send_char(c_hex,DEBUG_PORT_OUT);
			}
		} else {
			dbgu_send_char(c);
			if (c_hex == 0xFF){
				//dbgu_send_char('.');
				//dbgu_send_char('.');
				Port_send_char('.',DEBUG_PORT_OUT);
				Port_send_char('.',DEBUG_PORT_OUT);
			} else {
				//dbgu_send_char(DS_inttochar((c_hex>>4)&0x0F));
				//dbgu_send_char(DS_inttochar(c_hex&0xF));
				Port_send_char(DS_inttochar((c_hex>>4)&0x0F),DEBUG_PORT_OUT);
				Port_send_char(DS_inttochar(c_hex&0xF),DEBUG_PORT_OUT);

			}
		}
		//dbgu_send_char(' ');
		Port_send_char(' ',DEBUG_PORT_OUT);

	}
}

//Вывод информации об ошибке
//c - показывать 1-Q; 2-QR
//c_hex - показывать 1-P/N; 2-H; 3-P
void Error_Meaasge_Param (uint8_t c, uint8_t c_hex)
{
	if (DebugInfoOutFlag){
		if (c==1){
			Port_send_char('Q',DEBUG_PORT_OUT);//dbgu_send_char('Q');
			Port_send_char(' ',DEBUG_PORT_OUT);//dbgu_send_char(' ');
		} else if (c==2){
			Port_send_char('Q',DEBUG_PORT_OUT);//dbgu_send_char('Q');
			Port_send_char('R',DEBUG_PORT_OUT);//dbgu_send_char('R');
			Port_send_char(' ',DEBUG_PORT_OUT);//dbgu_send_char(' ');
		}

		if ( (c_hex == 1) && ((DS_Flag_Register & 3)== 1) ) {
			Port_send_char('N',DEBUG_PORT_OUT);//dbgu_send_char('N');
		} else if (c_hex == 2){
			Port_send_char('H',DEBUG_PORT_OUT);//dbgu_send_char('H');
		} else if (c_hex > 0) {
			Port_send_char('P',DEBUG_PORT_OUT);//dbgu_send_char('P');
		}
		Port_send_char(' ',DEBUG_PORT_OUT);//dbgu_send_char(' ');

	}


}
// преобразование части строки в целое число (считаем, что формат шестнадцетеричный)
// сюда доходим только с 0123...cdef и пробелами, пробелы игнорируются, буквы тоже
// uint8_t start_pos - позиция в строке, откуда начинать (первая из рассматриваемых)
// uint8_t len - общая длина рассматриваемой строки
// строка формата "RRGGBB" возвращается как 0x00RRGGBB
//!!! добавить проверку на излишне длинный номер с выводом ошибки DS_ERR_TOO_MANY_SYMBOLS
uint32_t DS_strtohex(uint8_t DS_tmp_string_[], uint16_t start_pos, uint16_t len)
{
	char c_char; //один символ из заданной для расшифровки строки
	uint8_t c_num; //преобразованное числовое значение этого символа
	uint32_t i32 = 0; //промежуточное хранение результата
	for (uint16_t i=start_pos; i < start_pos+len ; ++i){
		c_char = DS_tmp_string_[i]; 	// для соблюдений условия "i > ..." для беззнаковых чисел, где
										// start_pos может принимать значение = 0
		if (c_char != ' ') {   //пробелы игнорируются
			c_num = DS_chartoint(c_char);
			i32 <<= 4;
			if (c_num ^ 0xFF)	//нечисловые символы тоже игнорируются (на всякий случай)
				i32 += c_num;
		}
	}
	return i32;
}


// посимвольное добавление символа (полбайта) в целое число
// uint8_t c_hex - число, которое должно попасть в последние полбайта (УЖЕ ПРЕОБРАЗОВАННОЕ В ЧИСЛО)
// uint32_t par - переменная на входе
// возвращает переменную на выходе: входная с добавленным в конец символом
// строка формата "RRGGBB" возвращается как 0x00RRGGBB
uint32_t DS_hex_add_symbol(uint8_t c_hex, uint32_t par)
{
	//uint32_t i32 = 0;
//	if (DS_Flag_Register & 1){
//		par *= 10;
//		par += c_hex;
//		return par;
//	} else {
		par <<= 4;
		par += c_hex;
		return par;
//	}

}

uint32_t DS_dec_add_symbol(uint8_t c_hex, uint32_t par)
{
	//uint32_t i32 = 0;
	par *= 10;
	par += c_hex;
	return par;
}

uint32_t DS_num_add_symbol(uint8_t c_hex, uint32_t par)
{
	//uint32_t i32 = 0;
	if ((DS_Flag_Register & 1)){ // байт 1 поднят, байт 2 опущен
		par *= 10;
//		par += c_hex;
//		return par;
	}/*else if (DS_Flag_Register & 2) { //байт 2 поднят, короткий режим
		par <<= 8;
		par += c_hex;
	}/**/
	else { //байт 1 и 2 опущены
		par <<= 4;

	}
		par += c_hex;
		return par;
}

char DS_Number_to_ASCII (uint32_t par, uint8_t c_hex)
{
	c_hex <<=3;
	char c = par >> c_hex;
	return c;
}


//================================================================================================
// возведение в степень (степень от 0 до 2) - для ускорения(?) сделана чере if
/*uint8_t DS_math_power(uint8_t c, uint8_t pow)
{
	switch (pow) {   // через switch бинарник получился на 4 байта меньше, чем через иф
		case 0: return 1; break;
		case 1: return c; break;
		case 2: return c*c; break;
		default: return 0; break;
	}
	//if (0==pow) return 1;
	//if (1==pow) return c;
	//if (2==pow) return c*c;
	//return 0;
}*/

