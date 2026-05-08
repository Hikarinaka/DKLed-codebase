/*************************************************************************************************
 * @file		ledblinkTask.h
 *
 * @brief		Заголовок отладочного модуля
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/
#ifndef DESCRIPTTASK_H_
#define DESCRIPTTASK_H_


//*-----------------------------------------------------------------------------------------------
//*			Прототипы
//*-----------------------------------------------------------------------------------------------
//сброс единого счетчика пауз
void ResetPause(void);
//#define ResetPause()	nextPauseStartTime = CoGetOSTime()

// инициализация модуля
void OutputTaskInit(void);

// задача ОС
//void OutputTask(void* pdata);


//таймеры серв
void Servo_Set_Pins_and_Next_IRQ_Time (void);
void Multi_Output_Frame_And_Set_Next_Frame_And_Time (void);
void start_servos();
//void start_multi();
void TIM2_IRQHandler(void);
void stop_servos();
void UpdateServos();
void Do_WS2812_Output_Sequence();
void Set_Next_Frame_for_Fast_Animation ();
//void stop_multi();
#endif /* DESCRIPTTASK_H_ */
