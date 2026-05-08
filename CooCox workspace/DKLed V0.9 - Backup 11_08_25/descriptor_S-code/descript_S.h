/* Define to prevent recursive inclusion -------------------------------------*/
// в конце файла - ридми по поводу поддерживаемых команд

#ifndef __DESCRIPT_S_H
#define __DESCRIPT_S_H

#include "stm32f10x.h"




#define DS_Symbol_comment1		0x3B //';'
#define DS_Symbol_comment2		0x28 //'('
#define DS_Symbol_comment_end		0x29 //')'
#define DS_Symbol_comma			0x2C //','
#define Max_Random_File_List	200

//коды состояний ожидания приема (дерево при приеме нового символа) (для DS_status)
#define	DS_Type_command_new		0x20		//принимается новая команда (ожидаем начала приема)
#define	DS_Type_command_S		0x21		//принимается s-команда (светодиоы)
#define	DS_Type_command_F		0x22		//принимается f-команда (файловая система)
#define	DS_Type_command_A		0x23		//принимается a-команда
#define	DS_Type_command_G		0x24		//принимается g-команда (G-команды)
#define	DS_Type_command_M		0x25		//принимается m-команда (M-команды)
#define	DS_Type_command_U		0x26		//принимается U-команда (порты вода-вывода)
#define DS_Type_command_I2C		0x27		//управление I2C
									//до 0x1d зарезервировано под другие группы команд
#define	DS_Type_command_comment	0x2e	//принимаем комментарий

#define	DS_Type_command_WS_S0	0x2f	//принимаем последовательность для ws на ногу S0
// 31/01/2017: используется S0 для всех WS.S-команд
/*
//следующие восемь - строго один за другим
#define	DS_Type_command_WS_S0_short		0x20 //для обработки однобайтовых команд
#define	DS_Type_command_WS_S1_short		0x21
#define	DS_Type_command_WS_S2_short		0x22
#define	DS_Type_command_WS_S3_short		0x23
#define	DS_Type_command_WS_S4_short		0x24
#define	DS_Type_command_WS_S5_short		0x25
#define	DS_Type_command_WS_S6_short		0x26
#define	DS_Type_command_WS_S7_short		0x27
*/
//следующие четыре - строго один за другим
#define	DS_Type_command_Pause			0x30	//принимаем значение паузы G4
#define	DS_Type_command_Pause2			0x31	//принимаем значение паузы G5
#define	DS_Type_command_Pause_Absolut	0x32	//принимаем значение абсолютной паузы G6
#define	DS_Type_command_Pause_Absolut2	0x33	//принимаем значение абсолютной паузы G7

#define	DS_Type_command_GOTO_FILE		0x34	//принимаем имя вызываемого файла (для безусловного перехода)
//следующие два - строго один за другим
//#define	DS_Type_command_BUTTON_COMMAND	0x35 //M95 - расширенный вид команды M96
#define	DS_Type_command_SET_BUTTON		0x35	//M96 принимаем значение файла для перехода по кнопке (для условного перехода по кнопке)
#define DS_Type_command_ACTIVATE_BUTTON	0x36	//M97 принимаем команду на активацию/деактивацию события по кнопке

#define	DS_Type_command_SR				0x37	//принимаем raw-данные для светодиодов, ждем параметр Q
#define	DS_Type_command_RAW				0x38	//принимаем raw-данные для светодиодов - пишем байты в память
#define	DS_Type_command_SET_LED_TYPE	0x39	//принимаем значения для светодиодов
#define DS_Type_command_UID				0x3A	//принимаем значение личного ID
#define DS_Type_command_SWITCH_ONOFF	0x3B	//Простое включение/отключение чего-то, результат через DS_Com_Num
#define DS_Type_command_USART_BAUD		0x3C	//принимаем значение baudrate для USART
#define DS_Type_command_FILEWORKS_CONT	0x3D	//принимаем значение работы с файлами (показать файл/папку)
//следующие два - строго один за другим
#define DS_Type_command_USART1_OUT		0x3E	//принимаем значение вывода
#define DS_Type_command_USART2_OUT		0x3F	//принимаем значение вывода
//следующие два - строго один за другим
#define DS_Type_command_USART1_SEND		0x40	//отправляем байты вывода
#define DS_Type_command_USART2_SEND		0x41	//отправляем байты вывода

#define	DS_Type_command_Set_Absolut		0x42	//установить значениетекущего времени с начала чтения файла (для абсолютной паузы G6)
#define DS_Type_command_USART_TIMEOUT	0x43	//принимаем значение максимальное время на приём команды
#define DS_Type_command_FILE_SETDIR		0x44	//принимаем имя рабочей папки
#define	DS_Type_command_GOTO_RND_FILE	0x45	//принимаем случайное имя вызываемого файла из списка (для безусловного перехода)
#define	DS_Type_command_SET_RND_SEED	0x46	//принимаем значение random seed
#define	DS_Type_command_USART_SPECS		0x47	//отправляем информацию о контроллере
#define	DS_Type_command_SERVO_SET		0x48	//назначить позицию сервам
#define	DS_Type_command_SERVO_SET_ALL	0x49	//назначить позицию всем сервам
#define	DS_Type_command_BUTTON_EMULATE	0x4A	//эмуляция нажатия на кнопку
#define	DS_Type_command_SERVO_ENABLE	0x4B	//подключить/отключить серву (а также светодиодные выводы в гибридном режиме) к управляющей частоте
#define	DS_Type_command_SERVO_PARAMS	0x4C	//назначить сервам минимальную длину импульса ШИМ
#define DS_Type_command_MULTI_SET		0x4D	//Назначить параметры разбивки на кадры
#define DS_Type_command_FILE_READ_SET	0x4E	//F10 [Q<pionter>]P/N<number of bytes> - читать кусок из файла, F11 [Q<pointer>]P/H<data> - писать кусок в файл (определяем Q)
#define DS_Type_command_SWITCH_SHORT	0x4F	//короткая команда вкл/выкл, 2 байта на определение
#define DS_Type_command_REPEAT_FILE		0x50	//повтор файла определёное количество раз
#define DS_Type_command_FILE_SUSPEND	0x51	//приостановка исполнения файла на время
//следующие два - строго один за другим
#define DS_Type_command_USART1_SEND_HEX	0x52	//отправляем байты вывода
#define DS_Type_command_USART2_SEND_HEX	0x53	//отправляем байты вывода
//следующие пять - строго один за другим
#define	DS_Type_command_SERVO_ENABLE_3	0x54	// M3 подключить/отключить серву (а также светодиодные выводы в гибридном режиме) к управляющей частоте
#define	DS_Type_command_SERVO_ENABLE_4	0x55	// M4 подключить/отключить серву (а также светодиодные выводы в гибридном режиме) к управляющей частоте
#define	DS_Type_command_SERVO_ENABLE_5	0x56	// M5 подключить/отключить серву (а также светодиодные выводы в гибридном режиме) к управляющей частоте
#define	DS_Type_command_SERVO_ENABLE_6	0x57	// M10 подключить/отключить серву (а также светодиодные выводы в гибридном режиме) к управляющей частоте
#define	DS_Type_command_SERVO_ENABLE_7	0x58	// M11 подключить/отключить серву (а также светодиодные выводы в гибридном режиме) к управляющей частоте

#define	DS_Type_command_BUTTON_EMULATE2	0x59	//эмуляция отпускания кнопки для короткой записи
#define	DS_Type_command_SERVO_ADD		0x5A	//назначить изменение позиции сервам
//следующие два - строго один за другим
#define DS_Type_command_FILE_READ_PART	0x5B	//F10 [Q<pionter>]P/N<number of bytes> - читать кусок из файла (само чтение)
#define DS_Type_command_FILE_WRITE_PART	0x5C	//F11 [Q<pointer>]P/H<data> - писать кусок в файл (сама запись)

#define DS_Type_command_Set_Q_number	0x5D	//промежуточное действие для M96Q.P., M97Q.P., M98Q.P.
#define DS_Type_command_Set_Brightness	0x5E	//SI <brightness>

//следующие два - строго один за другим
#define DS_Type_command_USART1_COLOR	0x5F	//отправляем информацию о цвете пиксела в 16-чном формате
#define DS_Type_command_USART2_COLOR	0x60	//отправляем информацию о цвете пиксела в 16-чном формате

#define DS_Type_command_I2C_Send_to		0x61	//указываем адрес i2c получателя строчки
#define	DS_Type_command_SK				0x62	//принимаем данные для палитры цветов светодиодов, ждем номер цветовой ячейки

//------------------коды ошибок и команд от расшифровщика-----------------------
//стандартные ответы функции Descript()
//команды
//									0x00	//case default
#define DS_ANS_WS_S_DONE 			0x01	//можно обновить светодиоды
#define DS_ANS_READ_ON				0x02	//читать файл дальше
//следующие 4 строго друг за другом
#define DS_ANS_PAUSE				0x03	//принята команда паузы G4, значение передается через DS_param
#define DS_ANS_PAUSE2				0x04	//принята команда паузы G5, значение передается через DS_param
#define DS_ANS_PAUSE_ABSOLUT		0x05	//принята команда паузы G6, значение передается через DS_param
#define DS_ANS_PAUSE_ABSOLUT2		0x06	//принята команда паузы G7, значение передается через DS_param

#define DS_ANS_STOP					0x07	//конец файла h-кода (!!!!!или M30), останавливаем работу
//следующие два - строго один за другим
#define DS_ANS_REPEAT_FILE			0x08	//команда повторить воспроизведение с начала M47 or M98P0
#define DS_GOTO_FILE				0x09	//начать исполнение другого файла M98P...

#define DS_ASSIGN_BUTTON			0x0A	//запомнить, какой файл читать по кнопке
#define DS_ACTIVATE_BUTTON			0x0B	//активировать/деактивировать событие по кнопке
#define DS_ANS_SET_LED_TYPE			0x0C	//настроить тип светодиодов
#define DS_ANS_RESET_PAUSE			0x0D	//принята команда отмены паузы G9, выполняется процедура ResetPause()
#define DS_USART_SET_ID				0x0E	//принята команда назначения личного ID
//следующие три - строго один за другим
#define DS_USART_SET_DEBUG			0x0F	//принята команда включить/отключить сервисные сообщения
#define DS_USART_SET_INPUT			0x10	//принята команда включить/отключить приём данных
#define DS_USART_SET_WAIT			0x11	//принята команда включить/отключить ожидание отправки данных

#define DS_USART_SET_BAUD			0x12	//принята команда назначить бодрейт
#define DS_ANS_SET_ABSOLUT_TIME		0x13	//принята команда установить время G8, значение передается через DS_param
#define DS_USART_SET_TIMEOUT		0x14	//принята команда назначить максимальное время на приём команды
//следующие два - строго один за другим
#define DS_ANS_U1T_DONE 			0x15	//данные на usart1 отправлены
#define DS_ANS_U2T_DONE 			0x16	//данные на usart2 отправлены

#define DS_ANS_SERVO_DISABLE		0x17	//принята команда M84, отключения/включения серв
#define DS_ANS_SERVO_SET			0x18	//принята команда G0 , значение передается через DS_param
#define DS_ANS_SERVO_SET_PARAM		0x19	//назначить параметры ШИМ сервы
#define DS_ANS_BUTTON_EMULATE		0x1A	//изобразить нажатие/отпускание кнопки
#define DS_ANS_MULTI_SET_PARAM		0x1B	//назначить параметры мультикадровой анимации
#define DS_ANS_MULTI_PLAY			0x1C	//запустить мультикадровую анимацию
#define DS_ANS_SERVO_WATCH			0x1D	//запустить отслеживание координат серв в реальном времени
#define DS_ANS_FILE_SUSPEND			0x1E	//запустить отслеживание координат серв в реальном времени


//следующие 9 должы идти строго в таком порядке
#define DS_ANS_FILEMODE_ON			0x1F	//принята команда F0 , значение передается через DS_param
#define DS_ANS_FILEMODE_READDIR		0x20	//читать содержимое папки
#define DS_ANS_NORMALMODE_SETDIR	0x21	//выбрать папку для чтения
#define DS_ANS_FILEMODE_SETDIR		0x22	//выбрать папку для работы с файлами F2
#define DS_ANS_FILEMODE_NEWDIR		0x23	//создать папку в корне (0:/hcd/) F3
#define DS_ANS_FILEMODE_OPENREAD	0x24	//выбрать файл для чтения F4
#define DS_ANS_FILEMODE_DELETE		0x25	//удалить файл F5
#define DS_ANS_FILEMODE_ACTUALIZE	0x26	//файл tmp переименовать во что-то серьзное F6
#define DS_ANS_FILEMODE_RENAME		0x27	//файл tmp переименовать во что-то серьзное F7

#define DS_ANS_FILEMODE_NEWFILE		0x28	//создать временный файл (0:/.../t) для записи F9
//следующие два - строго один за другим
#define DS_ANS_FILEMODE_READFILE	0x29	//читать кусок содержимого файла и отправить по uart файл F10Q<>P<>
#define DS_ANS_FILEMODE_WRITEFILE	0x2A	//записать в файл (0:/.../t) F11Q<>P<> (принять массив)
//следующие два - строго один за другим
#define DS_ANS_FILEMODE_WRITE_CRC	0x2B	//F12 P<0/1> - проверять или нет чексумму при записи файла
#define DS_ANS_FILEMODE_OUT_TARGET	0x2C	//F13 P<0, 1,2> - куда выводить ответы ФС

#define DS_ANS_EXIT_SUBPROGRAM		0x2D	//M89 без параметра - возврат к основной программе

#define DS_USART_SET_SHORT			0x2E	//UxS P<0/1> - включить юарт как эмулятор кнопок (короткая команда) или нет
#define DS_ANS_BUTTON_MULTI_SET		0x2F	//M86 P<0/1> - включить режим одновременного нажатия кнопок

#define DS_ANS_SET_M25_SWITCH_TIME	0x30	//принята команда установить время реверса команды M25, значение передается через DS_param
#define DS_I2C_SET_ADDRESS			0x31	//назначить собственный адрес
#define DS_I2C_SEND_TO				0x32	//отправить на I2c
//следующие два - строго один за другим
#define DS_I2C_GET_FROM				0x33	//простое получение байт от слейва
#define DS_I2C_GET_FROM_WITH_ADDR	0x34	//получение байт от слейва по адресу

#define DS_WS_SET_MAX_LENGTH		0x35	//установить максимальное количество точек

//ошибки
#define DS_ERR_MIN_ERROR_NUMBER		0x50	//граница между командами и ошибками - для распознавания в первом приближении

#define DS_ERR_FNAME_TOO_LONG		0xF7
#define DS_ERR_M_COMMAND_FAILED		0xF8
#define DS_ERR_CODE_OUT_OF_RANGE	0xF9
#define DS_ERR_TOO_MANY_SYMBOLS		0xFA
#define DS_ERR_UNEXPEXTED_SYMBOL	0xFB
#define DS_ERR_UNKNOWN_SYMBOL		0xFC
#define DS_ERR_DECODE_HEX_ERROR		0xFD
#define DS_ERR_UNKNOWN_ERROR		0xFE
#define DS_ERR_NO_ERROR				0xFF

//------------------флаги команд расшифровщика--------------------------------
//для использования в задаче ОС по чтению/расшифровке, биты в переменной descrActionFlag
#define DS_ACTION_QUIT_DESCRIPT			0x01
#define DS_ACTION_GOTO_FILE				0x02
#define DS_ACTION_BUTTON_INTERRUPT		0x04	//устанавливается по нажатию юкнопки
#define DS_ACTION_BUTTON_TO_END_MULTI	0x08	//устанавливается по нажатию юкнопки сли надо подождать конца анимации
#define DS_ACTION_BUTTON__MULTI			0x10	//устанавливается по нажатию юкнопки сли надо подождать конца анимации
#define DS_ACTION_FILEWORKS				0x20	//Работаем с файлами
#define DS_ACTION_OPENREAD				0x40	//Работаем с файлами
#define DS_ACTION_RETURN_FROM_AFK		0x80	//возврат к работе после зависания






//----------------------------------------------------------------------------
//-------прототипы функций----------------------------------------------------
int Descript(uint8_t WS2812_IO_framedata_[], uint8_t DS_buffer[],
		uint16_t *DS_buf_start,uint16_t *DS_buf_counter,
		uint8_t *DS_status, uint8_t *DS_comm_num_, uint16_t *DS_WSpoint_counter,
		uint8_t *DS_RGB_counter, uint16_t *DS_maxCurrPoints_, uint32_t *DS_Param);
uint8_t DS_chartoint (char c);
char DS_inttochar (uint8_t c);
void print_0X4 (uint16_t par, uint8_t c_hex);
void Error_Message_full(uint8_t a, uint8_t b,uint8_t c, uint8_t d, uint8_t c_hex, uint16_t par, uint8_t e);
//void Error_Message_data(uint8_t c_hex, uint16_t par, uint8_t c);
void Error_Message_Command(uint8_t c, uint8_t c_hex);
void Error_Meaasge_Param (uint8_t c, uint8_t c_hex);
uint32_t DS_strtohex(uint8_t DS_tmp_string_[], uint16_t start_pos, uint16_t len);
uint32_t DS_hex_add_symbol(uint8_t c_hex, uint32_t par);
uint32_t DS_dec_add_symbol(uint8_t c_hex, uint32_t par);
uint32_t DS_num_add_symbol(uint8_t c_hex, uint32_t par);
char DS_Number_to_ASCII (uint32_t par, uint8_t c_hex);


//void DS_tmpstringInit(uint8_t DS_tmp_string_[], uint8_t len);
//uint8_t DS_math_power(uint8_t c, uint8_t pow);

#endif

//--------------------------------------------------------------------------------
//README.TXT
/* Программа начинает первым читать файл, начинающийся с символа "0" (ноль) в папке /hcd/ в корне SD-карты. Если таких файлов несколько,
 * то берётся тот файл, который записан в FAT раньше.
 *
 * Список поддерживаемых кодов
 	 Все пробелы игнорируются. Знаки <...> не часть кода, а обозначение диапазона или опущенных символов в команде.
 	 Все символы - латиница.
 	 (29/10/17 работает криво)
 	 (29/8/17 работает для WS и SK, для SK должно лучше работать 29/6/12)
 	 Комментарии в h-файле - в конце строки после символа ';' или в круглых скобках
 	 Знаком к тому, что запись команды закончена является "перевод строки \r\n", начало комментария или конец файла.

 	 S<0...7> P<RRGGBB><...> - вывод на светодиодную линейку точек, зашифрованных после P. Коды шестнадцатеричные, символы считаются по 6 штук.
 	 	 Если в последней точке до 6 штук не хватило символов, эта точка игнорируется. Совсем.
 	 	 Если в тексте после P появляется символ G или g, то точка, которую дожны были прочитать, пропускается. Чтение цвета следующей точки начинается с первого символа.
 	 	 Эта команда не является сигналом к физическому обновлению соответствующей светодиодной линейки. Будут читаться и расшифровываться следующие команды.
	G4 P<...>  - пауза, в миллисекундах. Число воспринимается как шестнадцатеричное. Команда паузы является сигналом к физическому обновлению всех точек.
	M47 - повторить текущий файл с начала
	M98 P<...> - перейти к другому файлу. Поддерживаются только шестнадцатеричные цифры до 4 символов. Незначащие нули в начале (сразу после P)
		учитываются, но крайне не рекомендуются, потому что могут быть проблемы с первым читаемым файлом.
		M98 без параметра P - работает аналогично M47
	В разработке:
	М96 Q<R><button> P<file> - по кнопке <button> перейти на <file>, <R> - если кодируется действие по отпусканию соответствующей кнопки

 */

