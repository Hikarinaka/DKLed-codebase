/*************************************************************************************************
 * @file		inputsTask.c
 *
 * @brief
 *
 * @version
 * @date
 * @author
 *
 ************************************************************************************************/

//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"

//*-----------------------------------------------------------------------------------------------
//*			Константы
//*-----------------------------------------------------------------------------------------------
#define INPUTS_TASK_STK_SIZE	110			// размер стека задачи 192

#define DREBEZG_WAIT 50 //пауза на дребезг контактов
#define REFRESH_BUTTONS_PERIOD  2 //мс, период опроса кнопок
//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
//ID задач
/*extern OS_TID debugTaskID, outputTaskID, inputsTaskID;

//OS_STK inputsTaskStk[INPUTS_TASK_STK_SIZE];	// стек задачи
extern StatusType outputStartFlag;
extern StatusType fileReadStartFlag;
extern StatusType r2; //для проверки результата установики флагов
*/
extern BYTE descrActionFlag; //битовые флаги разных действий в процессе цикла чтения/расшифровки, константы в DS_ACTION_...
//extern uint8_t buttonPressed; //какая кнопка была нажата только что
extern uint16_t buttonsState; //запоминаем состояние кнопок, чтобы правильно ловить нажатие/отпускание (по битам)
extern uint16_t buttonsNewState; //текущее состояние нажатости кнопок, чтобы ловить что изменилось
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
//8 - (0)= последний раз FastCommand сработал вхолостую, либо все кнопки были отпущены, так что мы можем набирать комбинацию
//0x10 - принимаем байт как команду с uart 1
//0x20 - принимаем байт как команду с uart 2
extern int16_t ButtonEncoder;
extern uint16_t ButtonEncorerIndiv[MAX_NUM_ENCODERS];

U64 ButtonReactionTick = 0;

extern uint16_t numFileForButton[MAX_NUM_BUTTONS_ARR];	//массив имён файлов для перехода по кнопкам (хранятся как цифры)
//	extern uint16_t ParameterForButton[MAX_NUM_BUTTONS_ARR];
extern uint8_t FastCommandForButton[MAX_NUM_BUTTONS_ARR]; //идентификатор быстрой команды
extern uint8_t CommandArgForButton[MAX_NUM_BUTTONS_ARR]; //короткий аргумент
extern uint16_t AfterLoop_File_Num; //код файла по отложенному переходу M47
extern uint16_t AfterMulti_File_Num; //код файла по отложенному переходу конец мультикадра

//BYTE buttonsStateTMP;

//uint16_t portBtmp; //временное хранение входа B
//uint32_t buttonInterruptState; //для запоминания, какое прерывание сработало
//uint8_t I_a8; //временная переменная - счетчик
uint32_t I_b16;

extern volatile uint8_t DebugInfoOutFlag; //включает и отключает вывод служебной информации на второй USART (DBGU)


extern uint8_t EPindex;
extern uint16_t BKIstr;
extern uint8_t USB_state_flag;
extern unsigned char USB_Buff1[64];
extern uint8_t usb_fsm_state;
//*-----------------------------------------------------------------------------------------------
/**			функции														*/
//*-----------------------------------------------------------------------------------------------
//опрос состояния кнопок
uint16_t RequestButtonsState(uint32_t portB){
	uint16_t buttons = 0;
	//0 => кнопка нажата, флаг состояния =1
#if defined (O8I6U2)
	if ((~portB & GPIO_BUTTON6)) {buttons |= 0x40;}
	if ((~portB & GPIO_BUTTON5)) {buttons |= 0x20;}
	if ((~portB & GPIO_BUTTON4)) {buttons |= 0x10;}
#endif
#if defined (O8I5U2)
	if ((~portB & GPIO_BUTTON5)) {buttons |= 0x20;}//pin 0, software I2C SDA
	if ((ButtonFlags & 0x40)==0){
		if ((~portB & GPIO_BUTTON4)) {buttons |= 0x10;}//pin 7, software I2C SCL/hardware I2C SDA
		if ((~portB & GPIO_BUTTON3)) {buttons |= 0x08;}//pin 6, hardware I2C SCL
	}
#endif
#if defined (O6I4U2)
	if ((~portB & GPIO_BUTTON4)) {buttons |= 0x10;}
#endif
#if defined (O4I3)
	if ((~portB & GPIO_BUTTON3)) {buttons |= 0x08;}
#endif

	if ((~portB & GPIO_BUTTON2)) {buttons |= 0x04;}
	if ((~portB & GPIO_BUTTON1)) {buttons |= 0x02;}
	//кнопки пронумированы с 1й по 6, 0-я логическая
	//if (ButtonFlags & 0x40) {buttons &= ~0x18;} //сбросить значения если всключён I2C

	return buttons;
}


//прямое обращение к массиву действий по номеру
//номер получается исходя из нажатых кнопок, с 1 по 5
//то есть всего 32 варианта комбинаций, кодируемых битами 0011 1110
void ButtonActionImplement(uint8_t BState){

	//BState &= MAX_NUM_BUTTONS_MASK;
	I_b16 = numFileForButton[BState];

	//if (temp_buttonstate) {
		ButtonFlags |=8;
		FastCommand(FastCommandForButton[BState],CommandArgForButton[BState],&I_b16);
	//}//мы что-то делаем, а значит надо отпустить кнопки прежде чем набирать новую комбинацию

}



//*-----------------------------------------------------------------------------------------------
/**			Инициализация тестовой задачи														*/
//*-----------------------------------------------------------------------------------------------
void InputsTaskInit(void)
{
	// инициализация тестовой задачи
/*	inputsTaskID = CoCreateTask(InputsTask,	// указатель на задачу
				(void *)0,	// указатель на передаваемые в задачу данные, не используем
				10,			// приоритет задачи, максимум
				&inputsTaskStk[INPUTS_TASK_STK_SIZE-1],		// указатель на конец области стека задачи
				INPUTS_TASK_STK_SIZE);					// размер стека, слов (4-байтных)
/**/
	//ButtonEncoder = 0x2 + 0x8;
	buttonsState = RequestButtonsState(GPIOB->IDR);
}

#if (0)
//*-----------------------------------------------------------------------------------------------
/**			Тело задачи
 *
 * @param pdata - указатель на доп. параматры, не используется									*/
//*-----------------------------------------------------------------------------------------------
void InputsTask(void* pdata)
{
//uint8_t II_b8;
uint8_t I_a8; //временная переменная - счетчик
uint8_t Drebezg_flags=1;

	CoTickDelay (300);
	//buttonsState = RequestButtonsState(GPIOB->IDR);

	do {
		CoTickDelay (REFRESH_BUTTONS_PERIOD); // =5 (опрос 200 раз в секунду)

		buttonsNewState = RequestButtonsState(GPIOB->IDR);


		if ((ButtonFlags & 2)&&(ButtonCompleteStart < CoGetOSTime())) { //пора нажимать на кнопку
			r2 = CoSetFlag (fileReadStartFlag);
			ButtonFlags &= 0xFD;// ~2
			//print_0X4(buttonsNewState,1);
			//Message("...ex\r\n\0",1);
			I_a8 = (buttonsNewState >>1) & MAX_NUM_BUTTONS_ACIONS_MASK;
			ButtonActionImplement( I_a8);
//			if (ButtonFlags & 8){//команда реально выполнена
//			}

		} else if (ButtonWaitSetting){// && (buttonPushInterruptActive | buttonRelInterruptActive)){
			if (ButtonWaitStart < CoGetOSTime()){
				//Message("B_exp\r\n\0",1);
				buttonsState &= ButtonEncoder;//на энкодер не влияет
				ButtonFlags &=~8;
			}//если прошёл таймаут на кнопку, то делаем вид, что они все только что были отпущены
		}

		// ButtonFlags & 8 == 1 - мы исполнили какую-то реакцию
		//теперь нам надо чтобы все кнопки были отпущены, buttonsNewState==0
		//тогда мы имеем право опять начать набирать новую комбинацию
		if (buttonsNewState == 0){ButtonFlags &=~8;}

		if (buttonsNewState ^ buttonsState) { //если состояние кнопок поменялось
			buttonPushInterrupt = (buttonsState ^ buttonsNewState) & buttonsNewState;
			buttonRelInterrupt = (buttonsState ^ buttonsNewState) & buttonsState;




			//пробуждаем задачу по чтению/дешифровке (если она ждала окончания паузы), чтобы она смогла отработать открытие нового файла и сброс паузы
			if (ButtonFlags & 1){//реакция на комбинацию кнопок
				if (buttonsNewState && (0 == (ButtonFlags & 0x0A))){
					//Message("mBtn: \0",1);
					ButtonWaitStart = CoGetOSTime() + ButtonWaitSetting; //когда сбрасывать кнопки
					ButtonCompleteStart = CoGetOSTime() + ButtonCompleteSetting - REFRESH_BUTTONS_PERIOD;//когда завершится набор
					ButtonFlags |= 2;//флаг ожидания конца одновременного нажатия
					//CoTickDelay (BUTTON_COMPLETE_MIN);
				}
//			} else if ((buttonPushInterrupt & buttonPushInterruptActive) | (buttonRelInterrupt & buttonRelInterruptActive)) {//реакция на отдельное нажатие/отпускание
			} else {//if (buttonPushInterrupt | buttonRelInterrupt) {//реакция на отдельное нажатие/отпускание

				Drebezg_flags=1;
				if (ButtonEncoder){
					for (I_a8 = 0; I_a8 < MAX_NUM_ENCODERS; ++I_a8){
						buttonPushInterrupt &= ~ButtonEncorerIndiv[I_a8];
						buttonRelInterrupt &= ~ButtonEncorerIndiv[I_a8];
						//01->00, 10->11 =>2
						//10->00, 01->11 =>1
						if ((buttonPushInterrupt|buttonRelInterrupt)==0 ){
							Drebezg_flags=0;
							if (ButtonEncorerIndiv[I_a8] == (buttonsNewState & ButtonEncorerIndiv[I_a8])){ //11
								buttonPushInterrupt |= (buttonsState & ButtonEncorerIndiv[I_a8]);//10->11 = 2, 01->11 = 1
							} else if (ButtonEncorerIndiv[I_a8] == ((~buttonsNewState) & ButtonEncorerIndiv[I_a8])){ //00
								buttonPushInterrupt |= ((~buttonsState) & ButtonEncorerIndiv[I_a8]);//10->00 = 1, 01->00 = 2
							}
						}
					}

				}/**/
				r2 = CoSetFlag (fileReadStartFlag);
				ButtonFlags &=0xFD;//на всякий сбросить 0x02
				//Message("Btn\r\n\0",1);
				ButtonWaitStart = CoGetOSTime() + ButtonWaitSetting; //когда сбрасывать кнопки
				for (I_a8 = 0; I_a8 < MAX_REAL_BUTTONS; ++I_a8){
					if (((FastCommandForButton[I_a8]-1) < 127) && (buttonPushInterrupt&(1<<I_a8))){//(buttonPushInterrupt & buttonPushInterruptActive)&(1<<I_a8)){
						//FastCommandForButton больше 0 и меньше 0x80(=128)
						I_b16 = numFileForButton[I_a8];
						//if (FastCommandForButton[I_a8]>FC_EXIT_SUBPROGRAM_M89) {I_b16 = ParameterForButton[I_a8];}
						FastCommand(FastCommandForButton[I_a8],CommandArgForButton[I_a8],&I_b16);
						//print_0X4(I_a8,1);
						//Message("p\r\n\0",1);
						I_a8 = MAX_REAL_BUTTONS+2;
					} else if (((FastCommandForButton[I_a8+ MAX_NUM_BUTTONS]-1) < 127) && (buttonRelInterrupt&(1<<I_a8))){//(buttonRelInterrupt & buttonRelInterruptActive)&(1<<I_a8)){
						I_b16 = numFileForButton[I_a8+ MAX_NUM_BUTTONS];
						//if (FastCommandForButton[I_a8+ MAX_NUM_BUTTONS]>FC_EXIT_SUBPROGRAM_M89) {I_b16 = ParameterForButton[I_a8+ MAX_NUM_BUTTONS];}
						FastCommand(FastCommandForButton[I_a8+ MAX_NUM_BUTTONS],CommandArgForButton[I_a8+ MAX_NUM_BUTTONS],&I_b16);
						//print_0X4(I_a8,1);
						//Message("r\r\n\0",1);
						I_a8 = MAX_REAL_BUTTONS+2;
					}
				}

				if (Drebezg_flags) CoTickDelay (DREBEZG_WAIT); //пауза на дребезг контактов
			}
			buttonsState = buttonsNewState;

			/*
#ifndef FastButtonShortcuts
			if ((buttonPushInterrupt & buttonPushInterruptActive) | (buttonRelInterrupt & buttonRelInterruptActive)) { //если произошедшее событие требует действия
				//тут будет проверка на тип команды (если по файлам - то делаем как делали, иначе сваливаемся в обработчик быстрых команд)
				descrActionFlag |= DS_ACTION_BUTTON_INTERRUPT; //добавляем флаг, что кнопка была нажата;
				//пробуждаем задачу по чтению/дешифровке (если она ждала окончания паузы), чтобы она смогла отработать открытие нового файла и сброс паузы
				r2 = CoSetFlag (fileReadStartFlag);
				CoTickDelay (DREBEZG_WAIT); //пауза на дребезг контактов
			} else if ((buttonPushInterrupt & buttonPushWaitStateEndFile) | (buttonRelInterrupt & buttonRelWaitStateEndFile)) {//отработка отложенного нажатия (M47)
				for (I_a8 = 0; I_a8 < MAX_REAL_BUTTONS; ++I_a8){
					if ((buttonPushInterrupt & buttonPushWaitStateEndFile)&(1<<I_a8)){
						AfterLoop_File_Num = numFileForButton[I_a8];
						AfterMulti_File_Num = 0xFFFF;
					} else if ((buttonRelInterrupt & buttonRelWaitStateEndFile)&(1<<I_a8)){
						AfterLoop_File_Num = numFileForButton[I_a8 + MAX_NUM_BUTTONS];
						AfterMulti_File_Num = 0xFFFF;
					}
				}
				CoTickDelay (DREBEZG_WAIT); //пауза на дребезг контактов
			} else if ((buttonPushInterrupt & buttonPushWaitStateEndMulti) | (buttonRelInterrupt & buttonRelWaitStateEndMulti)) {//отработка отложенного нажатия (конец мультикадра)
				for (I_a8 = 0; I_a8 < MAX_REAL_BUTTONS; ++I_a8){
					if ((buttonPushInterrupt & buttonPushWaitStateEndMulti)&(1<<I_a8)){
						AfterLoop_File_Num = 0xFFFF;
						AfterMulti_File_Num = numFileForButton[I_a8];
					} else if ((buttonRelInterrupt & buttonRelWaitStateEndMulti)&(1<<I_a8)){
						AfterLoop_File_Num = 0xFFFF;
						AfterMulti_File_Num = numFileForButton[I_a8 + MAX_NUM_BUTTONS];
					}
				}
				CoTickDelay (DREBEZG_WAIT); //пауза на дребезг контактов
			}
#endif/**/
		}



/*
		for (II_b8 = 0; II_b8 < REFRESH_BUTTONS_PERIOD; II_b8++){//REFRESH_BUTTONS_PERIOD
			 if (usb_fsm_state){
				USB_CTR_RX();
			};
			CoTickDelay (1);
		}
					/**/




	} while(1);

}
#endif


void ButtonsCheckSchedile(){
	uint8_t I_a8; //временная переменная - счетчик
	uint8_t Drebezg_flags=1;
	if (ButtonReactionTick <= CoGetOSTime()){
		ButtonReactionTick = CoGetOSTime() + REFRESH_BUTTONS_PERIOD;
		//CoTickDelay (REFRESH_BUTTONS_PERIOD); // =5 (опрос 200 раз в секунду)

		buttonsNewState = RequestButtonsState(GPIOB->IDR);


		if ((ButtonFlags & 2)&&(ButtonCompleteStart < CoGetOSTime())) { //пора нажимать на кнопку
//			r2 = CoSetFlag (fileReadStartFlag);
			ButtonFlags &= 0xFD;// ~2
			I_a8 = (buttonsNewState >>1) & MAX_NUM_BUTTONS_ACIONS_MASK;
			ButtonActionImplement( I_a8);

		} else if (ButtonWaitSetting){
			if (ButtonWaitStart < CoGetOSTime()){
				buttonsState &= ButtonEncoder;//на энкодер не влияет
				ButtonFlags &=~8;
			}//если прошёл таймаут на кнопку, то делаем вид, что они все только что были отпущены
		}

		// ButtonFlags & 8 == 1 - мы исполнили какую-то реакцию
		//теперь нам надо чтобы все кнопки были отпущены, buttonsNewState==0
		//тогда мы имеем право опять начать набирать новую комбинацию
		if (buttonsNewState == 0){ButtonFlags &=~8;}

		if (buttonsNewState ^ buttonsState) { //если состояние кнопок поменялось
			buttonPushInterrupt = (buttonsState ^ buttonsNewState) & buttonsNewState;
			buttonRelInterrupt = (buttonsState ^ buttonsNewState) & buttonsState;




			//пробуждаем задачу по чтению/дешифровке (если она ждала окончания паузы), чтобы она смогла отработать открытие нового файла и сброс паузы
			if (ButtonFlags & 1){//реакция на комбинацию кнопок
				if (buttonsNewState && (0 == (ButtonFlags & 0x0A))){
					ButtonWaitStart = CoGetOSTime() + ButtonWaitSetting; //когда сбрасывать кнопки
					ButtonCompleteStart = CoGetOSTime() + ButtonCompleteSetting - REFRESH_BUTTONS_PERIOD;//когда завершится набор
					ButtonFlags |= 2;//флаг ожидания конца одновременного нажатия
				}
			} else {//реакция на отдельное нажатие/отпускание

				Drebezg_flags=1;
				if (ButtonEncoder){
					for (I_a8 = 0; I_a8 < MAX_NUM_ENCODERS; ++I_a8){
						buttonPushInterrupt &= ~ButtonEncorerIndiv[I_a8];
						buttonRelInterrupt &= ~ButtonEncorerIndiv[I_a8];
						//01->00, 10->11 =>2
						//10->00, 01->11 =>1
						if ((buttonPushInterrupt|buttonRelInterrupt)==0 ){
							Drebezg_flags=0;
							if (ButtonEncorerIndiv[I_a8] == (buttonsNewState & ButtonEncorerIndiv[I_a8])){ //11
								buttonPushInterrupt |= (buttonsState & ButtonEncorerIndiv[I_a8]);//10->11 = 2, 01->11 = 1
							} else if (ButtonEncorerIndiv[I_a8] == ((~buttonsNewState) & ButtonEncorerIndiv[I_a8])){ //00
								buttonPushInterrupt |= ((~buttonsState) & ButtonEncorerIndiv[I_a8]);//10->00 = 1, 01->00 = 2
							}
						}
					}

				}/**/
//				r2 = CoSetFlag (fileReadStartFlag);
				ButtonFlags &=0xFD;//на всякий сбросить 0x02
				ButtonWaitStart = CoGetOSTime() + ButtonWaitSetting; //когда сбрасывать кнопки
				for (I_a8 = 0; I_a8 < MAX_REAL_BUTTONS; ++I_a8){
					if (((FastCommandForButton[I_a8]-1) < 127) && (buttonPushInterrupt&(1<<I_a8))){//(buttonPushInterrupt & buttonPushInterruptActive)&(1<<I_a8)){
						//FastCommandForButton больше 0 и меньше 0x80(=128)
						I_b16 = numFileForButton[I_a8];
						FastCommand(FastCommandForButton[I_a8],CommandArgForButton[I_a8],&I_b16);
						goto Jump_Out_Of_Button_Action_FindindCycle_Label;
						//I_a8 = MAX_REAL_BUTTONS+2;
					} else if (((FastCommandForButton[I_a8+ MAX_NUM_BUTTONS]-1) < 127) && (buttonRelInterrupt&(1<<I_a8))){//(buttonRelInterrupt & buttonRelInterruptActive)&(1<<I_a8)){
						I_b16 = numFileForButton[I_a8+ MAX_NUM_BUTTONS];
						FastCommand(FastCommandForButton[I_a8+ MAX_NUM_BUTTONS],CommandArgForButton[I_a8+ MAX_NUM_BUTTONS],&I_b16);
						goto Jump_Out_Of_Button_Action_FindindCycle_Label;
						//I_a8 = MAX_REAL_BUTTONS+2;
					}
				}
Jump_Out_Of_Button_Action_FindindCycle_Label:
				if (Drebezg_flags) ButtonReactionTick = CoGetOSTime() + DREBEZG_WAIT; //пауза на дребезг контактов
			}
			buttonsState = buttonsNewState;
		}

	}

}

//проверка конкретной кнопки/комбинации на то, нажата ли она в данный момент
void ExecuteIfButtonStatePrepressed(uint8_t *ButtonNomberCarrier_){
	uint16_t BS = RequestButtonsState(GPIOB->IDR);
	if (ButtonFlags & 1){//реакция на комбинацию кнопок
		BS = (buttonsNewState >>1) & MAX_NUM_BUTTONS_ACIONS_MASK;
		if (BS == *ButtonNomberCarrier_) {
			goto Implement_Button_Action_Immediatelly_Label;
			//ButtonActionImplement(BS);
		}
	} else {//реакция на отдельное наличие нажатия/отпускания
		uint16_t BP = 0;
		//в *ButtonNomberCarrier_ находится номер кнопки
		if (*ButtonNomberCarrier_< MAX_NUM_BUTTONS){//проверяем нажатость
			BP = 1 << *ButtonNomberCarrier_;
		} else {//проверяем отпущенность
			BS = ~BS;
			BP = 1 << (*ButtonNomberCarrier_ - MAX_NUM_BUTTONS);
		}
#if defined (O8I5U2)//сброс I2C
		if ((ButtonFlags & 0x40)){
			BS &= ~0x18;
		}
#endif
		BS &= ~ButtonEncoder;//сброс энкодеров
		//теперь в BS подняты только биты, относящиеся к нажатым/отпущенным кнопкам, которые не размечены под энкодеры и I2C
		if (BS & BP){
Implement_Button_Action_Immediatelly_Label:
			I_b16 = numFileForButton[*ButtonNomberCarrier_];
			ButtonFlags |=8;
			FastCommand((FastCommandForButton[*ButtonNomberCarrier_]&127),CommandArgForButton[*ButtonNomberCarrier_],&I_b16);
			//ButtonActionImplement(*ButtonNomberCarrier_);
		}
	}
}
