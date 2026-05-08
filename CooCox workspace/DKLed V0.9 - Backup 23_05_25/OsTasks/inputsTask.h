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
#ifndef INPUTS_H_
#define INPUTS_H_







//*-----------------------------------------------------------------------------------------------
//*			Прототипы
//*-----------------------------------------------------------------------------------------------
//опрос состояния кнопок
uint16_t RequestButtonsState(uint32_t portB);

void ButtonActionImplement(uint8_t BState);
// инициализация модуля
void InputsTaskInit(void);

// задача ОС
//void InputsTask(void* pdata);

void ButtonsCheckSchedile();
void ExecuteIfButtonStatePrepressed(uint8_t *ButtonNomberCarrier_);
#if defined (FastButtonShortcuts)
//обработчик быстрых команд
#endif

#endif /* LEDBLINK_H_ */
