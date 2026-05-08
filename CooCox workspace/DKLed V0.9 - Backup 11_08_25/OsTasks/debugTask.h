/*************************************************************************************************
 * @file		debugTask.h
 *
 * @brief		Заголовок отладочного модуля
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/
#ifndef DEBUG_H_
#define DEBUG_H_


//*-----------------------------------------------------------------------------------------------
//*			Константы
//*-----------------------------------------------------------------------------------------------
#define READ_SIZE				512			// размер буфера для чтения из файла
#define TEST_TASK_STK_SIZE		220			// размер стека задачи 220
#define FILE_PATH_MAX_LENGTH	34			// размер строки для пути файла "0:/dirname_/dirname_/filename.hcd" - 33 символа
#define DIR_PATH_MAX_LENGTH		24			// размер строки для пути рабочей папки "0:/dirname_/dirname_/" - 21 символ
#define FILE_NAME_MAX_LENGTH	13			// максимальная длина имени файла

#define MainLoop_Maximum_Wait_ms		500

#define SERVO_DEAD_BAND			100			//период вызова таймера сервов

#define SD_Recheck_Countdown_REVOLVE 512 //цикл подключения карты на горячую


//#if defined (FastButtonShortcuts)
/*
Короткие команды ( байта)
1 байт - сама команда, он же номер ссылки в select case
2 байт - параметр (вкл/выкл; битовая маска пинов/выводовб которые активировать)
3 и 4 байты - параметр (число)
*/
	#define FC_TOTAL_COMMAND_LIST		0x29	//сколько всего команд есть в списке

//следующие 3 - строго в этом порядке и строго в начале
	#define FC_START_FILE_NOW_M98		0x01	//open <file>, instant, M98 P<>, M98 Q<> P<>
	#define FC_NEXT_FILE_M98 			0x02	//open P<file>, after M47, M98
	#define FC_AFTER_ANIM_FILE_M98 		0x03	//open P<file>, after last fast anim frame

//следующие 4 - строго в этом порядке и строго после блока команд M98
	#define FC_ENTER_SUBPROGRAM_M89		0x04	//M89 P<>, M89 Q<> P<> - перейти в подпрограмму
	#define FC_EXIT_SUBPROGRAM_M89		0x05	//M89  - без параметра - вернуться в сновную программу
	//следующие 2 - строго в этом порядке и строго после блока команд M98
	#define FC_REPEAT_FILE_M47 			0x06	//M47 P<repeat>,instant, также является границей переключения параметра команды с имени файла на число
	#define FC_REPEAT_COUNTS_M47 		0x07	//How many times to repeat the M47, affects the M47 exec

//следующие 2 - строго в этом порядке
	#define FC_RESUME_FILE_M24 			0x08	//M24, resume file execution
	#define FC_SUSPEND_FILE_M25 		0x09	//M25, suspend file eecution
//следующие 5 - строго в этом порядке
	#define FC_PAUSE_END_G4	 			0x0A	//G4 (<0><time>), G5 (<1><time>), G9 (<2>), instant
	#define FC_PAUSE_END_G5	 			0x0B	//G4 (<0><time>), G5 (<1><time>), G9 (<2>), instant
	#define FC_PAUSE_END_G9	 			0x0C	//G4 (<0><time>), G5 (<1><time>), G9 (<2>), instant
	#define FC_RANDOM_SEED_M90 			0x0D	//M90  <seed>
	#define FC_RANDOM_SEED_TIMER_M90 	0x0E	//M90 <random> timer
//следующие 4 - строго в этом порядке
	#define FC_SERVO_ENABLE_M3 			0x0F	//M3, instant update
	#define FC_SERVO_DISABLE_M5			0x10	//M5, instant update
	#define FC_PIN_ENABLE_M10 			0x11	//M10, instant update
	#define FC_PIN_DISABLE_M11 			0x12	//M11, instant update

//следующие 2 - строго в этом порядке
	#define FC_SERVO_REALTIME_G1 		0x13	//G1 on, instant update
	#define FC_SERVO_FRAMEBASED_G1 		0x14	//G1 off, instant update

//следующие 4 - строго в этом порядке
	#define FC_SERVO_POS_SET_G0			0x15	//G0 <N> <position>, instant
	#define FC_SERVO_POS_ADD_G0			0x16	//G0 <N> <delta position, signed int>, instant
	#define FC_SERVO_POS_SUBST_G0		0x17	//G0 <N> <delta position, signed int>, instant
	#define FC_SERVO_POS_RAND_G0		0x18	//G0 <N> random position, instant

//следующие 3 - строго в этом порядке
	#define FC_BRIGHTNESS_SET_G27		0x19	//G27 P<>, SI P<> - настройка абсолютной яркости
	#define FC_BRIGHTNESS_ADD_G27		0x1A
	#define FC_BRIGHTNESS_SUBST_G27		0x1B
//следующие 3 - строго в этом порядке
	#define FC_CHOSEN_FILE_SET_M91		0x1C	//M91 P<>
	#define FC_CHOSEN_FILE_ADD_M91		0x1D
	#define FC_CHOSEN_FILE_SUBST_M91	0x1E
//следующие 3 - строго в этом порядке
	#define FC_FAST_ANIM_FREQ_SET_G30	0x1F	//G30 set frequency 0x32...0x640,
	#define FC_FAST_ANIM_FREQ_ADD_G30	0x20	//G30 increase frequency
	#define FC_FAST_ANIM_FREQ_SUBST_G30	0x21	//G30 decrease frequency
	//следующие 1 - строго в этом порядке
	#define FC_FAST_ANIM_PLAY_G35		0x22	//G35 <frames to play> - instant start
	//следующе 2 - строго в этом порядке
	#define FC_FAST_ANIM_ON_G36			0x23	//G36 <1>
	#define FC_FAST_ANIM_OFF_G36		0x24	//G36 <0>
//следующие 3 - строго в этом порядке
	#define FC_MISCOMP_SET_M45			0x26
	#define FC_MISCOMP_ADD_M45			0x27
	#define FC_MISCOMP_SUBST_M45		0x28

	#define FC_ENTER_NEXTFILE_M88		0x29	//open <file>, instant, M98 P<>, M98 Q<> P<>

//#endif

// инициализация модуля
void DebugTaskInit(void);
//void TIM2_IRQHandler(void);
void decFileReadStart(/*FIL* file_, */int32_t dec_);
//void Descript_Incomming_part(viod);
#if defined (FastButtonShortcuts)
	void FastCommand (uint8_t FS_a8, uint8_t FS_b8, uint32_t *FS_arg);
#endif
void Output_to_WS (void);
void Fail_Message (uint8_t a, uint8_t b, uint8_t res);
void DirOpenMessage (char DIRpath_[], uint8_t a, uint8_t b);
void Message (char Str[], uint8_t a);
void Finish_Fileworks_Message(uint8_t port);
uint32_t ModifyParameter (uint32_t Inp, uint32_t *FS_arg, uint32_t Max);
//uint8_t DIRPathByName(uint8_t targetdir);
//*-----------------------------------------------------------------------------------------------
//*			Прототипы
//*-----------------------------------------------------------------------------------------------
void TestTask(void);//* pdata);


//-------прототипы функций----------------------------------------------------
//void Check_If_MainLoop_Is_AFK();

#endif /* DEBUG_H_ */
