/*************************************************************************************************
 * @file		descriptTask.c
 * @brief		Тестовый модуль
 * @version		v0.1
 * @date		19.02.2017
 * @author		Vet
 ************************************************************************************************/

//*-----------------------------------------------------------------------------------------------
//*			Внешние модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"

//*-----------------------------------------------------------------------------------------------
//*			Константы
//*-----------------------------------------------------------------------------------------------
#define DESCRIPT_TASK_STK_SIZE	160			// размер стека задачи 220

//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
//ID задач
/*extern OS_TID debugTaskID, outputTaskID, inputsTaskID;

//OS_STK descriptTaskStk[DESCRIPT_TASK_STK_SIZE];	// стек задачи
extern StatusType outputStartFlag;
extern StatusType fileReadStartFlag;
extern StatusType r2; //для проверки результата установики флагов
*/
extern BYTE descrActionFlag; //битовые флаги разных действий в процессе цикла чтения/расшифровки, константы в DS_ACTION_...

// для WS
extern uint8_t WS2812_IO_framedata[ WS2812_IO_FRAMEDATA_SIZE ];
extern uint8_t LED_control_type;  //тип управления светодиодов,
//1 = 3х проводные, без часов
//2 = аналоговые сервы
//3 = гибридный
extern uint16_t WS2812_Frame_Length; //количество пикселей в кадре
extern uint16_t WS2812_Frame_Length_Actual; //количество пикселей в кадре
extern uint32_t WS2812_Frame_Byte_Length; //количество байт в кадре (WS2812_Frame_Length*24)
extern uint32_t WS2812_Frame_Byte_Length_Actual;
extern uint16_t WS2812_Frame_Count; //количество кадров в анимации
extern uint16_t WS2812_Frame_Count_Actual; //количество кадров в анимации
extern uint16_t WS2812_Frame_Total_Count; // всего кадров (определяем исходя из размеров массивов - сколько кадров вообще может поместиться при такой длине)
extern uint32_t WS2812_Frame_Start_Pointer; // указатель на первый проигрываемый кадр (адрес старта массива)
extern uint32_t WS2812_Frame_Start_Pointer_Actual;// указатель на текущий проигрываемый кадр
//extern uint32_t WS2812_Current_Frame_Start;
extern uint16_t WS2812_Frame_Period; //период в мкс, по сути то же что Servo_Period, только значение должно храниться независимо
extern uint16_t WS2812_Frame_Period_Actual;
extern uint8_t DS_LED_Brightness; //яркость светодиодов

// для descript
extern uint8_t DS_status; //какую команду ожидаем
extern uint16_t DS_buf_start; //с какого места расшифровывать, если команда из нескольких знаков
extern uint16_t DS_buf_counter; //счетчик, какое число пришло последним в буфер
extern uint8_t DS_comm_num; //текущий номер кода (который после буквы)
extern uint16_t DS_WSpoint_counter; //текущий принимаемый номер точки WS
extern uint8_t DS_RGB_counter; //счетчик принятых символов для точки RGB
extern uint32_t DS_Param; //32-битный параметр для дешифровщика
extern uint8_t text_buff[READ_SIZE+4];
//extern uint8_t WS2812_TC; //флаг того, что вывод точек на ws завершен (1 если завершено)
extern uint8_t file_to_read; //!!!переменная, которая гооврит, как читать файл. 0 - с начала, 1 - продолжаем как было.
extern uint16_t DS_maxCurrPoints; //переменная для вывода только нужного количества точек
extern uint16_t DS_TotalPoints;
uint16_t DS_LastPointsUpdated=1; //переменная на случай повторного вывода всё тех же точек

extern volatile uint8_t DebugInfoOutFlag; //включает и отключает вывод служебной информации на второй USART (DBGU)

//переменные для удержания общего таймлайна (время пауз не зависит от торомзов четния/расшифровки/вывода)
extern U64 osTime; //для времени ОС
extern U64 nextPauseStartTime; // хранение времени начала предыдущей паузы - для сохранения общего таймлайна несмотря на тормоза
extern uint32_t need_pause; //временное хранилище значения текущей паузы
extern uint16_t DS_Pause_interrupt_Flag;


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
// 0x04: 1 - изменились параметры, надо обновить
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

//extern uint16_t buttonPushInterruptActive; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
//extern uint16_t buttonRelInterruptActive; //флаги, активно ли событие по кнопке: для того, чтобы понять, нужно ли тормозить чтение/дешифровку
#ifndef FastButtonShortcuts
extern uint16_t buttonPushWaitStateEndMulti; // флаги активности кнопок по типам событий
extern uint16_t buttonRelWaitStateEndMulti; // флаги активности кнопок по типам событий
#endif
extern uint16_t AfterMulti_File_Num; //код файла по отложенному переходу конец мультикадра
extern uint16_t numFileForButton[MAX_NUM_BUTTONS_ARR];	//массив имён файлов для перехода по кнопкам (хранятся как цифры)
extern uint8_t FastCommandForButton[MAX_NUM_BUTTONS_ARR]; //идентификатор быстрой команды
extern uint8_t CommandArgForButton[MAX_NUM_BUTTONS_ARR]; //короткий аргумент



extern uint8_t a8; //временная переменная - счетчик
extern uint16_t a16; //временная переменная


//*-----------------------------------------------------------------------------------------------
/**			функции														*/
//*-----------------------------------------------------------------------------------------------


//сброс единого счетчика пауз
/**/
void ResetPause(void){
	nextPauseStartTime = CoGetOSTime();
	//if (DebugInfoOutFlag){printf("(OutputTask) ResetPause()     nextPauseStartTime = 0x%x\r\n", nextPauseStartTime);}

}/**/



//команды запуска серводвигателей
void start_servos()
{

	//Message ("srv on\r\n\0",1);
	Servo_Update_Flag |= 1; //флаг запуска поднят
	TIM2->CNT = 0;
	//TIM_Cmd(TIM2, ENABLE);
	TIM2->CR1 |= TIM_CR1_CEN;
	//Servo_time = 0;
	if ((LED_control_type&127) == 4) { //режим циклического вывода картинки
		TIM2->ARR = (uint16_t) WS2812_Frame_Period_Actual; //шаг анимации
		TIM2->EGR |= TIM_EGR_UG; //обновить актуальные тайминги (иначе они обновляются ПОСЛЕ следующего прерывания)

//		buttonRelInterruptActive &= ~buttonRelWaitStateEndMulti; // Прерывания по кнопкам только в момент нулевого кадра или после окончания проигрывания
//		buttonPushInterruptActive &= ~buttonPushWaitStateEndMulti;

	} else if (LED_control_type & 2) { //режим ШИМ
		GPIOB->BSRR = Servo_Pos_Mask[0];//GPIO_SetBits(GPIOB,Servo_Pos_Mask[0]);
		GPIOB->BSRR = Servo_GPIO_Actual_on;//GPIO_SetBits(GPIOB,Servo_GPIO_Actual_on);// и постоянно включённые
		GPIOB->BRR = Servo_GPIO_Actual_off;//GPIO_ResetBits(GPIOB,Servo_GPIO_Actual_off); // опускаем ноги, которые должны остаться опущены

		Servo_step = 0;
		TIM2->ARR = (uint16_t) Servo_Pos[0]; //первое прерывание
		//TIM_ITConfig(TIM2, TIM_DIER_UIE, ENABLE);
		//GPIO_SetBits(GPIOB,Servo_Pos_Mask[0]);
	}
	TIM2->DIER |= TIM_DIER_UIE;
	//CoEnterISR(); // Enter ISR

	//CoExitISR(); // Exit ISR
}



// обработчик прерывания счётного таймера
void TIM2_IRQHandler(void)
{
	CoEnterISR(); // Enter ISR
	if (LED_control_type & 2) { // режим ШИМ
		Servo_Set_Pins_and_Next_IRQ_Time ();
	} else if ((LED_control_type&127) == 4){
		TIM2->ARR = (uint16_t) WS2812_Frame_Period_Actual; //шаг анимации поскольку теперь это значениие может измениться
		Multi_Output_Frame_And_Set_Next_Frame_And_Time ();
	}





	TIM2->SR &= ~TIM_SR_UIF; //если сделать это раньше, то прерывание наступит сразу по обновлению таймингов

	CoExitISR(); // Exit ISR

}
//остановка контроля серводвигателей
void stop_servos()
{
//	Servo_Update_Flag |= 0x80;

	//if (DebugInfoOutFlag){printf("stop servos \r\n");}
	//TIM_Cmd(TIM2, DISABLE);
	TIM2->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
	//Servo_time = 0;
	//CoEnterISR(); // Enter ISR
	GPIOB->BRR = 0xFF00;//GPIO_ResetBits(GPIOB,0xFF00);
	if ((LED_control_type&127) == 4) {
		DS_Pause_interrupt_Flag &= ~64; //выкл ожидание конца вывода
		LED_control_type &= 128;
		LED_control_type |= 1;
		//сбросить стартовую позицию на 0
		//WS2812_ResetStartPixelInBuffer();
		//DMA1_Channel1->CMAR = WS2812_IO_framedata;
		WS2812_SetStartPixelInBuffer(0);
		if (AfterMulti_File_Num != 0xFFFF){
			descrActionFlag = descrActionFlag | DS_ACTION_BUTTON_TO_END_MULTI; //прерывание что надо открыть файл
			//descrActionFlag = descrActionFlag | DS_ACTION_BUTTON_TO_END_MULTI; //прерывание что надо открыть файл
		}
//		buttonRelInterruptActive |= buttonRelWaitStateEndMulti; // прерывания по кнопкам снова обычные
//		buttonPushInterruptActive |= buttonPushWaitStateEndMulti;

	} //возврат к обычной работе светодиодов
	Servo_Update_Flag &= ~1; //флаг запуска опущен
	//CoExitISR(); // Exit ISR
}




//Обновление позиций серв
void UpdateServos()
{
	Servo_GPIO_Actual_off = Servo_GPIO_Setting_off;
	Servo_GPIO_Actual_on = Servo_GPIO_Setting_on;
	for (a8 = 0; a8 <8; a8++){ //вычисляем время смены значения для каждого сервы
		Servo_Pos [a8] = ((Servo_Pos_Temp [a8] * (Servo_MaxPos - Servo_MinPos)) /Servo_Resolution ) +  Servo_MinPos; //читываем старт
		if (Servo_Pos[a8] > Servo_MaxPos){Servo_Pos[a8] = Servo_MaxPos;}//обрезаем максимум
		Servo_Pos_Mask [a8+1] = 1<<(a8+8);
	}
	for (a16 = 7; a16>0; a16--){ //сортируем массив по Servo_Pos
	for (a8 = 0; a8 < a16; a8++){
		if (Servo_Pos [a8]>Servo_Pos [a8+1]) {
			Servo_time = Servo_Pos [a8];
			Servo_Pos [a8] = Servo_Pos [a8+1];
			Servo_Pos [a8+1] = Servo_time;
			Servo_Pos_Mask [9] = Servo_Pos_Mask [a8+1];
			Servo_Pos_Mask [a8+1] = Servo_Pos_Mask [a8+2];
			Servo_Pos_Mask [a8+2] = Servo_Pos_Mask [9];
		}
	}}
	Servo_Pos [8] = Servo_Period;
	Servo_Pos_Mask [9] = 0xFF00;
	for (a8 = 7; a8 >0; a8--) { //схлопываем массив - определяем сколько всего реально надо прерываний
		if (Servo_Pos [a8-1]==Servo_Pos [a8]){
			//printf("\r\n");
			Servo_Pos_Mask [a8] += Servo_Pos_Mask [a8+1];// сохранить это значение после цикла ниже
			for (a16 = a8; a16<8; a16++){
				//printf("%d, ",a16);
				Servo_Pos [a16] = Servo_Pos [a16+1];
				Servo_Pos_Mask [a16+1] = Servo_Pos_Mask [a16+2];
			}
		}
	}
	//выключаем неактивные двигатели
	Servo_Pos_Mask[0] = ((uint16_t) Servo_Action_Mask) << 8;
	for (a8=1; a8<9; a8++){//применяем только к активным битам
		Servo_Pos_Mask [a8] &= Servo_Pos_Mask [0];
	}/**/
	Servo_Actual_Period = Servo_Period; //применяем длительность цикла ШИМ
	Servo_Update_Flag &= ~6; //флаг смены позиций опущен

}



Do_WS2812_Output_Sequence(){ //следующие 20 строк кода вынесены в процедуру т.к. она встретится ещё раз

if (DS_maxCurrPoints > 0){
	DS_LastPointsUpdated=DS_maxCurrPoints;
	WS2812_sendbuf( DS_LastPointsUpdated * 24 );     //  WS2812_IO_FRAMEDATA_SIZE );
	DS_maxCurrPoints = 0;
}

}



//*-----------------------------------------------------------------------------------------------
/**			Инициализация тестовой задачи														*/
//*-----------------------------------------------------------------------------------------------
void OutputTaskInit(void)
{
	// инициализация тестовой задачи
/*	outputTaskID = CoCreateTask(OutputTask,	// указатель на задачу
				(void *)0,	// указатель на передаваемые в задачу данные, не используем
				8,			// приоритет задачи, максимум
				&descriptTaskStk[DESCRIPT_TASK_STK_SIZE-1],		// указатель на конец области стека задачи
				DESCRIPT_TASK_STK_SIZE);					// размер стека, слов (4-байтных)
*/
	  WS2812_GPIO_init();
	  WS2812_DMA_init(WS2812_IO_framedata);
	  WS2812_Timer_reinit(29,  8, 17); //ws2812 freq
	  //WS2812_Timer_reinit(29,  6, 12); //sk6812 freq
	  //WS2812_Timer_reinit(29,  7, 15); //что-то между частотами WS и SK, должны работать оба типа одновременно
	  WS2812_clear_buffer(WS2812_IO_framedata, WS2812_IO_FRAMEDATA_SIZE, 0xFF);
	  WS2812_sendbuf( WS2812_IO_FRAMEDATA_SIZE );

	  DS_LastPointsUpdated=1;

	// запуск таймера для отсчёта времени
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC->APB1ENR |= RCC_APB1Periph_TIM2;

	//для сервы
	//TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//TIM_TimeBaseStructure.TIM_Period = 1000;//SERVO_DEAD_BAND; //10 мкс, == dead band width
	TIM2->ARR = 1000;
	//TIM_TimeBaseStructure.TIM_Prescaler = SERVO_TIMER_PRESC-1;	// 1 MГц тактирование
	TIM2->PSC = SERVO_TIMER_PRESC-1;	// 1 MГц тактирование
	//TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	//TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM2->CR1 &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD)));
	TIM2->CR1 |= TIM_CounterMode_Up|0;
	//TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	//TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM2->EGR = 1;
	//TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM2->CR1 |= TIM_CR1_ARPE;
	// событие при переполнении
	//TIM_ITConfig(TIM2, TIM_DIER_UIE, ENABLE);
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->EGR |= TIM_EGR_UG;
	//TIM2->EGR = TIM_PSCReloadMode_Immediate;
	// прерывание
	//NVIC_SetPriority(TIM2_IRQn, 8);
	NVIC->IP[TIM2_IRQn] = (0x80);

	//NVIC_EnableIRQ(TIM2_IRQn);
	NVIC->ISER[((uint32_t)(TIM2_IRQn) >> 5)] = (1 << ((uint32_t)(TIM2_IRQn) & 0x1F)); /* enable interrupt */


	Servo_Period = SERVO_PERIOD_DEF; //50Гц
	Servo_MinPos = SERVO_POS_MIN_DEF; //1 ms
	Servo_MaxPos = SERVO_POS_MAX_DEF; //2 ms
	Servo_Resolution = 512; //разрешение
	UpdateServos();

}


//*-----------------------------------------------------------------------------------------------
/**			Тело задачи
 *
 * @param pdata - указатель на доп. параматры, не используется									*/
//*-----------------------------------------------------------------------------------------------
/*void OutputTask(void* pdata)
{
	//uint32_t a32; //вспомогательная
	//uint16_t cntr, cntr2, cntr3, h;
	outputStartFlag = CoCreateFlag(0, 0);   // Сбрасывается вручную, изначально не установлен
	StatusType r2; //вспомогательная переменная для выставления флагов ОС
	//CoWaitForSingleFlag(outputStartFlag, 0);

	nextPauseStartTime = CoGetOSTime();


	do{
		CoWaitForSingleFlag(outputStartFlag, 0);
		//Do_WS2812_Output_Sequence(); //следующие 20 строк кода вынесены в процедуру т.к. она встретится ещё раз

		//если у нас мультикадр, надо обновить параметры
		//запихнуть эту часть в TIM4_IRQHandler из init_ws.c

		//if ((LED_control_type&127) == 4){
		//	while (WS2812_TC != 1){ ; } //при переносе в процедуру этот цикл выкинуть, так как процедура будет вызываться в том месте, в котором WS2812_TC присваивается 1
			//вычисляем номер текущего фрейма
		//	Set_Next_Frame_for_Fast_Animation ();
		//}//обработка мультикадра завершена.
		 //точки обновлены, чтение/расшифровку - запускаем
		r2 = CoSetFlag (fileReadStartFlag);
		//сброс флага паузы - после окончания паузы ждёт флага (на случай если расшифровка дольше паузы)
		r2 = CoClearFlag(outputStartFlag);

	} while(1);


}/**/


void Set_Next_Frame_for_Fast_Animation (){
	//while (WS2812_TC != 1){ ; }
	WS2812_Frame_Start_Pointer_Actual = 1 + WS2812_Frame_Start_Pointer_Actual / WS2812_Frame_Byte_Length_Actual;

	if (WS2812_Frame_Start_Pointer_Actual  >= WS2812_Frame_Count_Actual) {
		WS2812_Frame_Start_Pointer_Actual  = 0;
		//if (AfterMulti_File_Num != 0xFFFF){	WS2812_Frame_Total_Count = 1;}
	}
	//восстанавливаем новый номер кадра
	WS2812_Frame_Start_Pointer_Actual = WS2812_Frame_Start_Pointer_Actual * WS2812_Frame_Byte_Length_Actual;
	WS2812_SetStartPixelInBuffer(WS2812_Frame_Start_Pointer_Actual);


		//если за время проигрывания была нажата кнопка, выходим из анимации
	if ((WS2812_Frame_Total_Count == 1) || ((AfterMulti_File_Num != 0xFFFF) && (WS2812_Frame_Start_Pointer_Actual == 0))) {//цикл закончен
		//DS_Pause... поднимать не нужно - в Stop_Servos он сбрасывается
		//разделить последний фрейм и событие по кнопке, вытащить из Stop_servos этот кусок сюда.
		DS_Pause_interrupt_Flag |= 64;//вкл ожидаание конца вывода
		stop_servos();
		if (((FastCommandForButton[AFTER_FAST_ANIM_ACIONS_ADRESS]-1) < 127) && (AfterMulti_File_Num == 0xFFFF)){
			uint16_t I_b16 = numFileForButton[AFTER_FAST_ANIM_ACIONS_ADRESS];
			FastCommand(FastCommandForButton[AFTER_FAST_ANIM_ACIONS_ADRESS],CommandArgForButton[AFTER_FAST_ANIM_ACIONS_ADRESS],&I_b16);
		}/**/
	} else if (WS2812_Frame_Total_Count > 1) {
		//DS_Pause_interrupt_Flag |= 64;
		WS2812_Frame_Total_Count --; //уменьшаем количество оставшихся кадров на 1
	}
}


void Servo_Set_Pins_and_Next_IRQ_Time (void){
	if ((Servo_Pos[Servo_step] < Servo_Actual_Period) && (Servo_step < 8 )) { //дальше ещё есть куда двигаться

		GPIOB->BRR = Servo_Pos_Mask[Servo_step+1];//сброс битов, которые сбрасываем на этом шаге

		TIM2->ARR = (uint16_t) (Servo_Pos[Servo_step+1] - Servo_Pos[Servo_step]);
		Servo_step++; //следующий шаг
		if ((Servo_Pos[Servo_step] >= Servo_Actual_Period) || Servo_step >7) {
			//это значит, что на следующем вызове мы будем поднимать ноги
			//и сейчас все ноги должны быть опущены
			//а длительность до следуюшего вызова нам не особенно важна
			Servo_Update_Flag |= 8;
			//if (Servo_Update_Flag & 0x80){//открытие файла по событию

			//	Servo_Update_Flag &= ~0x80;

			//}

			if (Servo_Update_Flag & 0x10){//флаг необходимости вывода на светодиоды был поднят
				//разрешаем вывод на светодиоды
				Do_WS2812_Output_Sequence();
				/*if (DS_maxCurrPoints > 0){
					DS_LastPointsUpdated=DS_maxCurrPoints;
					DS_maxCurrPoints = 0;
					WS2812_sendbuf( DS_LastPointsUpdated * 24 );
					//while (WS2812_TC != 1){ ; }
				}*/

				Servo_Update_Flag &= (uint8_t)(~0x10);//мы запустили процесс, флаг нужно опустить.
			}//Servo_Update_Flag & 0x10


			if (((LED_control_type & 0x10) && (Servo_Update_Flag & 2)) || (Servo_Update_Flag & 4)){//надо обновить позиции серв не дожидаясь команды из паузы
				// повторяем всё то же самое, что и в процедуре output_to_WS(), но процесс выполняется в рамках этой задачи
				//if (~LED_control_type & 8) {LED_control_type &= ~16;}
				//LED_control_type &= (uint8_t)(~0x10) + ((LED_control_type & 8)<<1);
				//Servo_Update_Flag = Servo_Update_Flag | 4;
				UpdateServos();
			}
			//if ((Servo_Update_Flag & 4)) UpdateServos();

		}
	} else { //мы дошли до времени границы цикла, т.к. максимальное время - это период цикла вывода
		//Servo_time = 0;
		Servo_step = 0;
		GPIOB->BSRR = Servo_Pos_Mask[0];//поднимаем ноги (ШИМ)
		GPIOB->BSRR = Servo_GPIO_Actual_on;// и постоянно включённые
		GPIOB->BRR = Servo_GPIO_Actual_off; // опускаем ноги, которые должны остаться опущены
		TIM2->ARR = (uint16_t) Servo_Pos[0]; //первое прерывание
		Servo_Update_Flag &= ~8;
	}/**/
	TIM2->EGR |= TIM_EGR_UG; //обновить актуальные тайминги (иначе они обновляются ПОСЛЕ следующего прерывания)
	//TIM2->EGR = TIM_PSCReloadMode_Immediate;
	/*TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->CR1 |= TIM_CR1_CEN;*/
}


void Multi_Output_Frame_And_Set_Next_Frame_And_Time (void){

	DS_maxCurrPoints = WS2812_Frame_Length_Actual;

	//r2 = CoClearFlag (fileReadStartFlag); //флаг текущей задачи - тормозим

	Do_WS2812_Output_Sequence(); //следующие 20 строк кода вынесены в процедуру т.к. она встретится ещё раз

	//if ((LED_control_type&127) == 4){
	//while (WS2812_TC != 1){ ; } //при переносе в процедуру этот цикл выкинуть, так как процедура будет вызываться в том месте, в котором WS2812_TC присваивается 1
		//вычисляем номер текущего фрейма
	//Set_Next_Frame_for_Fast_Animation ();

	//}//обреботка мультикадра завершена.
	//r2 = CoSetFlag (fileReadStartFlag);
	//r2 = CoSetFlag(outputStartFlag); //размораживаем задачу вывода на экран

}

