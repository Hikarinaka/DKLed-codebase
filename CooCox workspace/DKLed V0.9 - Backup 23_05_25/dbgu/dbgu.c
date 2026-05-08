/*************************************************************************************************
 * @file		main.c
 *
 * @brief		Интерфейс DBGU
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 * @note		Передача осуществляется через кольцевой буфер, что потенциально позволяет
 * 				уменьшить задержку выполнения основной программы
 ************************************************************************************************/

//*-----------------------------------------------------------------------------------------------
//*			Подключаемые модули
//*-----------------------------------------------------------------------------------------------
#include "includes.h"
#include "stm32f10x_rcc.h"

/*extern OS_TID debugTaskID, outputTaskID, inputsTaskID;

extern StatusType outputStartFlag;
extern StatusType fileReadStartFlag;
extern StatusType r2; //для проверки результата установики флагов
*/


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------
static uint8_t dbgu_tx_buff[DBGU_TX_SIZE];		///< буфер передатчика uart
//volatile static uint8_t dbgu_tx_head;	///< указатель для символа, помещаемого в буфер TX
//volatile static uint8_t dbgu_tx_tail;	///< указатель для символа, выбираемого из буфера TX

static uint8_t USART1_tx_buff[USART1_TX_SIZE];		///< буфер передатчика uart
//volatile static uint8_t USART1_tx_head;	///< указатель для символа, помещаемого в буфер TX
//volatile static uint8_t USART1_tx_tail;	///< указатель для символа, выбираемого из буфера TX

uint8_t *Port_tx_buff[2] = {
	 &USART1_tx_buff,
	 &dbgu_tx_buff,
};

volatile static uint8_t Port_tx_head[2];// = {
//	 &USART1_tx_head,
//	 &dbgu_tx_head,
//};

volatile static uint8_t Port_tx_tail[2];// = {
//	 &USART1_tx_tail,
//	 &dbgu_tx_tail,
//};

uint16_t *Port_tx_CR1[2] = {
	 &(USART1->CR1),
	 &(DBGU_UART->CR1),
};

uint8_t dbgu_rx_buff[DBGU_RX_SIZE];		///< буфер приёмника uart
volatile static uint16_t dbgu_rx_head;	///< указатель для символа, помещаемого в буфер RX
volatile static uint16_t dbgu_rx_tail;	///< указатель для символа, выбираемого из буфера RX
volatile static uint16_t dbgu_rx_tail_warning;	///< служебная переменная, которая соответствует половине буфера RX
volatile uint16_t dbgu_rx_buf_start; //с какого места расшифровывать, если команда из нескольких знаков
volatile uint16_t dbgu_rx_buf_counter; //счетчик, какое число пришло последним в буфер


uint8_t USART1_rx_buff[USART1_RX_SIZE];		///< буфер приёмника uart
volatile static uint16_t USART1_rx_head;	///< указатель для символа, помещаемого в буфер RX
volatile static uint16_t USART1_rx_tail;	///< указатель для символа, выбираемого из буфера RX
volatile static uint16_t USART1_rx_tail_warning;	///< служебная переменная, которая соответствует половине буфера RX
volatile uint16_t USART1_rx_buf_start; //с какого места расшифровывать, если команда из нескольких знаков
volatile uint16_t USART1_rx_buf_counter; //счетчик, какое число пришло последним в буфер


volatile uint8_t USART1_State_of_recieved_Command; //индикатор/регистр состояния чтения команды из USART2
volatile uint8_t dbgu_State_of_recieved_Command; //индикатор/регистр состояния чтения команды из USART2
//1 - мы готовы воспринимать приходящие данные
//0 - мы игнорируем входящие данные
//3,5,7,9 - читаем заголовок (4 байта на ID) - 3-прочитали 1й, 5- прочитали 2й,...,9 - прочитали последний
//16 (0x10) - мы принимаем инфу, записываем её в буфер приёма команд
//32 (0x20) - приём открыт, слушаем порт и процессим байты оттуда
//64 (0x40) - временная приостановка приёма (скобочки)
//16+128 (0x90) - есть что показать, но оно не закончено и половину буфера мы уже заняли
//128 (0x80) - есть что показать, что передать в descriot
//const char util_symbol = 0xFF;
//const char univ_symbol = '*';

//uint8_t USART1_State_of_Commands_undescripted=0; //индикатор/регистр состояния чтения команды из USART2
//uint8_t dbgu_State_of_Commands_undescripted=0; //индикатор/регистр состояния чтения команды из USART2


volatile U64 dbgu_rx_start_time; //записываем время когда мы должны начать дешифровку
volatile uint32_t dbgu_rx_timeout; //предельное время ожидания конца команды от её начала
volatile U64 USART1_rx_start_time; //записываем время когда мы должны начать дешифровку
volatile uint32_t USART1_rx_timeout; //предельное время ожидания конца команды от её начала

//volatile static uint8_t dbgu_Pay_Attention_RX_flag; //индикатор того, что мы
volatile static uint32_t dbgu_temp_header; //заголовок, его мы будем сравнивать с Personal_ID
volatile static uint32_t USART1_temp_header; //заголовок, его мы будем сравнивать с Personal_ID
//volatile uint16_t dbgu_rx_expected_length;//ожидаемая длина приёма

extern uint32_t Personal_ID; //"0000" или 0x30303030 - общий
extern uint16_t DS_Pause_interrupt_Flag; //поднимаеется 8, если мы ждём окончания передачи данных

extern uint8_t ButtonFlags;
//1 - нажатие по одной кнопке (0) или комбинацией (1)
//2 - (1)= ждём конца нажатия
//4
//8
//0x10 - принимаем байт как команду с uart 1
//0x20 - принимаем байт как команду с uart 2
extern uint16_t numFileForButton[MAX_NUM_BUTTONS_ARR];	//массив имён файлов для перехода по кнопкам (хранятся как цифры)
//extern uint16_t ParameterForButton[MAX_NUM_BUTTONS_ARR];
extern uint8_t FastCommandForButton[MAX_NUM_BUTTONS_ARR]; //идентификатор быстрой команды
extern uint8_t CommandArgForButton[MAX_NUM_BUTTONS_ARR]; //короткий аргумент

extern uint16_t USB_Bytes_to_send_left;
extern uint16_t USB_Recieved_bytes;
extern uint8_t USB_Function_flags;
extern unsigned char USB_Buff1[64];
extern unsigned char USB_COM_TX_Buff1[64];

extern uint8_t I2C_Bytes_To_Send;
extern uint8_t I2C_tx_buff[I2C_TX_SIZE];
extern I2C_stage;


uint32_t i;
uint8_t temp_tx_head;
uint8_t j ; //0 if 0,2 ;1 if 1
uint8_t k ; //0 if 0, 1 if 1, 2
//-------------------------------------------------------------------------------------------
//локальные функции

void Incomming_Byte_Processing (
		char *temp_tx_tail_,
		uint8_t *dbgu_State_of_recieved_Command_,
		uint32_t *dbgu_temp_header_,
		uint8_t dbgu_rx_buff_[],
		uint16_t *dbgu_rx_tail_,
		uint16_t RX_SIZE,
		uint16_t *dbgu_rx_buf_counter_,
		uint16_t *dbgu_rx_tail_warning_,
		U64 *dbgu_rx_start_time_,
		uint32_t *dbgu_rx_timeout_,
		uint16_t *dbgu_rx_head_,
		uint16_t *dbgu_rx_buf_start_//,
		//uint8_t *dbgu_State_of_Commands_undescripted_
		){

	if (*dbgu_State_of_recieved_Command_ & 0x20){ //проверка на чтение
		if ((*temp_tx_tail_ == util_symbol || *temp_tx_tail_ == DS_Symbol_comment1 ||  *temp_tx_tail_ == 0x0A || *temp_tx_tail_ == 0) && ((*dbgu_State_of_recieved_Command_ & 0x40) == 0)){//завершение приёма
			//так же, как и любая команда в файле, т.е. по \r, \n, коммент (;), \0
			//dbgu_State_of_recieved_Command равно или 0 (надо в 1), или 0x10, или 0x90 (надо в 0x80)

				*dbgu_temp_header_ = 0; //на случай, если нам недавно прилетало 0xFF, и прерывание начало заполнять заголовок, но не заполнило его до конца
				*dbgu_State_of_recieved_Command_ &=0xB1; //очищаем биты 2,4,8, 64 (та же ситуация, что и строкой выше)
				*dbgu_State_of_recieved_Command_ |=1; //мы снова готовы принять заголовок
				if (*dbgu_State_of_recieved_Command_ >0x30){//отдаём на расшифровку
					dbgu_rx_buff_[*dbgu_rx_tail_&(RX_SIZE-1)]='\r';
					//dbgu_rx_buff_[(*dbgu_rx_tail_+1)&(RX_SIZE-1)]='\n';
					*dbgu_rx_tail_+=1;
					*dbgu_rx_buf_counter_ =  *dbgu_rx_tail_&(RX_SIZE-1);
					*dbgu_State_of_recieved_Command_ = 0xA1; //0x20 + 0x80, всё готово к расшифровке
					//*dbgu_State_of_Commands_undescripted_ = *dbgu_State_of_Commands_undescripted_ + 1; //команда добавлена в очередь
				}

		} else {
			if ((*dbgu_rx_start_time_ < CoGetOSTime()) && (*dbgu_State_of_recieved_Command_ != 0x21)){//команда просрочена
				*dbgu_State_of_recieved_Command_ = 0x21;
			}
			//это выполняется после того, как выполнилось Сначала и Затем ниже
			if (*dbgu_State_of_recieved_Command_ & 0x10) {//записываем данные в буфер (на предыдущем вызове прерывания мы выяснили, что нам это надо)
				if (*dbgu_State_of_recieved_Command_ & 0x40) {//ранее прилетел служебный символ и мы "в скобочках"
					if (*temp_tx_tail_ == DS_Symbol_comment_end){//символ ")"
						*dbgu_State_of_recieved_Command_ &= ~0x40;
					}
				} else {/**/
					if (*temp_tx_tail_ == DS_Symbol_comment2){//символ "("
						*dbgu_State_of_recieved_Command_ |= 0x40;
					} else {
						dbgu_rx_buff_[*dbgu_rx_tail_&(RX_SIZE-1)] = *temp_tx_tail_; //значение указателя dbgu_rx_tail обрезаем так, чтобы он был в рамках диапазона DBGU_RX_SIZE
						*dbgu_rx_buf_counter_ = *dbgu_rx_tail_&(RX_SIZE-1); //запоминаем, до какого символа будем читать команду в дескрипте
						*dbgu_rx_tail_+=1; //увеличиваем указатель куда принимать байт на 1
						//нужно проверить, не заполнили ли мы буфер наполовину
						if (*dbgu_rx_tail_ == *dbgu_rx_tail_warning_ ){//tail больше чем head на половину размера буфера, даже с учётом переполнения
							*dbgu_State_of_recieved_Command_ |= 0x80;
							*dbgu_rx_tail_warning_ = *dbgu_rx_tail_ + (RX_SIZE>>1);
						}
					}
				}
			} else {
				//Сначала выполняется это (чтение заголовка)
				if (*dbgu_State_of_recieved_Command_ & 0x1){//мы ждём заголовок
					*dbgu_rx_start_time_ = CoGetOSTime()  + *dbgu_rx_timeout_; //записываем время, когда мы должны закончить расшифровывать команду

					if (*temp_tx_tail_ == univ_symbol) {
						*dbgu_temp_header_ = (*dbgu_temp_header_<<8) + *temp_tx_tail_;
						*dbgu_State_of_recieved_Command_ +=2;
						//((dbgu_State_of_recieved_Command>>1)&3)<<3
					} else if (*temp_tx_tail_ == (uint8_t)(Personal_ID >>((3-((*dbgu_State_of_recieved_Command_>>1)&3))<<3))){
						*dbgu_temp_header_ = (*dbgu_temp_header_<<8) + *temp_tx_tail_;
						*dbgu_State_of_recieved_Command_ +=2;/**/
					} else {
						*dbgu_temp_header_ = 0;
						*dbgu_State_of_recieved_Command_ &= 0xA0;//сбрасываем до 0x20 или  0x20+0x80
						//*dbgu_State_of_recieved_Command_ = 0x20;//просто читаем всё и ждём util_symbol (0x0D), но не добавляем в буфер - для этого надо чтобы стало 0x30 (ранее было 0x21 - ждём заголовок)
					}/**/
					//0x23 - принят 1 символ, 0x25 - 2, 0x27 - 3...
				}
				//Затем выполняется это (установка флага на заполнение буфера команды)
				if ((*dbgu_State_of_recieved_Command_ &0x7F) == 0x29){//заголовок принят!
					//*dbgu_State_of_recieved_Command_ = 0x20;//не нужно так как снизу уже есть =0x30
					//if (dbgu_temp_header == 0x30303030 || dbgu_temp_header == Personal_ID){ //значит нам интересно воспринимать данные
					*dbgu_State_of_recieved_Command_ &= 0x80;//0x29 -> 0x30
					*dbgu_State_of_recieved_Command_ |= 0x30;//0x10+0x20
					*dbgu_rx_head_ = *dbgu_rx_tail_; //указатель на начало очередной записи устанавливаем на текущую позицию
					*dbgu_rx_tail_warning_ = *dbgu_rx_tail_ + (RX_SIZE>>1); //определяем, когда будет заполнена половина буфера
					*dbgu_rx_buf_start_ = *dbgu_rx_tail_&(RX_SIZE-1); //запоминиаем, откуда будем читать команду расшифровщиком descript
					//}
					*dbgu_temp_header_ = 0;
				}
			}

		}
	}
}

void USART_params_set (USART_TypeDef* USARTx, uint32_t* integerdivider_){


	  //uint16_t tmpreg = 0x00; //работало с 32 битами
	  //uint32_t integerdivider = 0x00;
	  //uint32_t fractionaldivider = 0x00;

	    //integerdivider_ = ((25 * PCLK2_Freq) / (4 * (BaudRate_)));
	  uint16_t  tmpreg_ = ( *integerdivider_ / 100) << 4;
	  uint32_t  fractionaldivider = *integerdivider_ - (100 * (tmpreg_ >> 4));
		    tmpreg_ |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);


	USARTx->CR2 &= CR2_STOP_CLEAR_Mask;
	USARTx->CR1 &= CR1_CLEAR_Mask;
	USARTx->CR1 |= 0x000C;
	USARTx->CR3 &= CR3_CLEAR_Mask;
	USARTx->BRR = (uint16_t) tmpreg_;

	USARTx->CR1 &= ~((uint32_t) USART_FLAG_TXE);
	USARTx->CR1 |= (uint32_t) USART_FLAG_RXNE;

}


//*-----------------------------------------------------------------------------------------------
/**			Инициализация отладочного интерфейса
 *
 * @param BaudRate - скорость работы интерфейса, бит/сек										*/
//*-----------------------------------------------------------------------------------------------
void DBGU_Init(uint32_t BaudRate)
{
	//dbgu_tx_head = dbgu_tx_tail = 0;
	Port_tx_head[1]=Port_tx_tail[1] = 0;
	dbgu_rx_head = dbgu_rx_tail = 0;
	dbgu_rx_buf_start = dbgu_rx_buf_counter = 0; //сброс указателей буфера чтения команд в начало
	//dbgu_State_of_recieved_Command = 0x21; //перенесено в main
	dbgu_temp_header = 0;


	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;

//	GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure USART Tx as alternate function push-pull */
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Pin = DBGU_UART_TX_PIN;//0x0004
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(DBGU_UART_PORT, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructure.GPIO_Pin = DBGU_UART_RX_PIN;//0x0008
//	GPIO_Init(DBGU_UART_PORT, &GPIO_InitStructure);

	DBGU_UART_PORT->CRL &= 0xFFFF00FF;
	DBGU_UART_PORT->CRL |= 0x00004B00;

	/* USARTx configured as follow:
	        - Word Length = 8 Bits
	        - One Stop Bit
	        - No parity
	        - Hardware flow control disabled (RTS and CTS signals)
	        - Receive and transmit enabled
	*/
/*	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;*/

	/* Enable GPIO clock */
	//DBGU_UART_CLK_ENABLE;
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)
	RCC->APB1ENR |= RCC_APB1Periph_USART2;

	/* USART configuration */
	//USART_Init(DBGU_UART, &USART_InitStructure);
//	DBGU_UART->CR2 &= CR2_STOP_CLEAR_Mask;
	  //USART1->CR2 |= USART_StopBits_1; //= 0000//(uint32_t)USART_InitStruct->USART_StopBits;
//	DBGU_UART->CR1 &= CR1_CLEAR_Mask;
//	DBGU_UART->CR1 |= 0x000C; //USART_WordLength_8b|USART_Parity_No|USART_Mode_Rx | USART_Mode_Tx;
	  //(uint32_t)USART_InitStruct->USART_WordLength | USART_InitStruct->USART_Parity | USART_InitStruct->USART_Mode;
//	DBGU_UART->CR3 &= CR3_CLEAR_Mask;
	  //USART1->CR3 |= USART_HardwareFlowControl_None;//=0000 //USART_InitStruct->USART_HardwareFlowControl;
		//	  RCC_ClocksTypeDef RCC_ClocksStatus;

//	  uint16_t tmpreg = 0x00; //работало с 32 битами
//	  uint32_t integerdivider = 0x00;
//	  uint32_t fractionaldivider = 0x00;

	  	  //	  RCC_GetClocksFreq(&RCC_ClocksStatus);
	uint32_t  integerdivider = ((25 * PCLK1_Freq) / (4 * (BaudRate)));
//		  tmpreg = (integerdivider / 100) << 4;
//		  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
//		    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
//	  DBGU_UART->BRR = (uint16_t)tmpreg;

	  USART_params_set (DBGU_UART, &integerdivider);
	  	  //	USART_ITConfig(DBGU_UART, USART_IT_TXE, DISABLE);
//	DBGU_UART->CR1 &= ~((uint32_t) USART_FLAG_TXE);
		//	USART_ITConfig(DBGU_UART, USART_IT_RXNE, ENABLE); //включение приёма символов
//	DBGU_UART->CR1 |= (uint32_t) USART_FLAG_RXNE;


	//NVIC_SetPriority(DBGU_UART_IRQn, IRQ_UART_DBGU_PRIORITY);
	NVIC->IP[DBGU_UART_IRQn] = 0x40;//((IRQ_UART_DBGU_PRIORITY << 4) & 0xff);

	//NVIC_EnableIRQ(DBGU_UART_IRQn);
	NVIC->ISER[((uint32_t)(DBGU_UART_IRQn) >> 5)] = (1 << ((uint32_t)(DBGU_UART_IRQn) & 0x1F)); /* enable interrupt */

	/* Enable USART */
//	USART_Cmd(DBGU_UART, ENABLE);
	DBGU_UART->CR1 |= CR1_UE_Set;
}




void USART1_Init(uint32_t BaudRate)
{
	//USART1_tx_head = USART1_tx_tail = 0;
	Port_tx_head[0]=Port_tx_tail[0] = 0;
	USART1_rx_head = USART1_rx_tail = 0;
	USART1_rx_buf_start = USART1_rx_buf_counter = 0; //сброс указателей буфера чтения команд в начало
	USART1_temp_header = 0;


	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;

	USART1_PORT->CRH &= 0xFFFFF00F;
	USART1_PORT->CRH |= 0x000004B0;

	/* USARTx configured as follow:
	        - Word Length = 8 Bits
	        - One Stop Bit
	        - No parity
	        - Hardware flow control disabled (RTS and CTS signals)
	        - Receive and transmit enabled
	*/
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)
	RCC->APB2ENR |= RCC_APB2Periph_USART1;


//	  uint16_t tmpreg = 0x00; //работало с 32 битами
//	  uint32_t integerdivider = 0x00;
//	  uint32_t fractionaldivider = 0x00;

	uint32_t  integerdivider = ((25 * PCLK2_Freq) / (4 * (BaudRate)));
//		  tmpreg = (integerdivider / 100) << 4;
//		  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
//		    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
	  /* Write to USART BRR */
//	  USART1->BRR = (uint16_t)tmpreg;

	  USART_params_set (USART1, &integerdivider);



	//NVIC_SetPriority(USART1_IRQn, IRQ_USART1_PRIORITY);
	NVIC->IP[USART1_IRQn] = 0x40;//((IRQ_USART1_PRIORITY << 4) & 0xff);
	//NVIC_EnableIRQ(USART1_IRQn);
	NVIC->ISER[((uint32_t)(USART1_IRQn) >> 5)] = (1 << ((uint32_t)(USART1_IRQn) & 0x1F)); /* enable interrupt */

	/* Enable USART */
//	USART_Cmd(USART1, ENABLE);
	USART1->CR1 |= CR1_UE_Set;
}




//*-----------------------------------------------------------------------------------------------
/**			Функция отправки символа в интерфейс
 *
 * @param data - отправляемый символ															*/
//*-----------------------------------------------------------------------------------------------
#if (0)
void dbgu_send_char(signed int data)
{
#ifdef _COOCOX_H
	CoSchedLock();
#endif

	uint8_t temp_tx_head = dbgu_tx_head;
	//uint8_t temp_tx_head = *Port_tx_head[1];

	dbgu_tx_buff[temp_tx_head++] = data;
	//*Port_tx_buff[1][temp_tx_head++] = data;
	// (*((uint8_t *)(Port_tx_buff[1] + temp_tx_head++)))= data;

	// если буфер занят, подождать освобождения
	uint32_t i = 50000;
	while ((temp_tx_head == dbgu_tx_tail) && (i))
//	while ((temp_tx_head == *Port_tx_tail[1]) && (i))
		i--;
	/*
	if (i == 0)	// защита от зависания
	{
		DBG_HALT(0);
		// перезапустить передатчик
		__disable_irq();
		DBGU_UART->CR1 |= USART_FLAG_TXE;
		__enable_irq();
	}
	*/

//	if (*Port_tx_head[1] == *Port_tx_tail[1])
	if (dbgu_tx_head == dbgu_tx_tail)
	{
	//	*Port_tx_head[1] = temp_tx_head;
		dbgu_tx_head = temp_tx_head;

		// запустить передачу
		__disable_irq();

		//*Port_tx_CR1[1] |= USART_FLAG_TXE;

		DBGU_UART->CR1 |= USART_FLAG_TXE;
		__enable_irq();
	}
	else
		dbgu_tx_head = temp_tx_head;
	//	*Port_tx_head[1] = temp_tx_head;

#ifdef _COOCOX_H
	CoSchedUnlock();
#endif
}

void USART1_send_char(signed int data)
{
#ifdef _COOCOX_H
	CoSchedLock();
#endif

	uint8_t temp_tx_head = USART1_tx_head;
	USART1_tx_buff[temp_tx_head++] = data;

	// если буфер занят, подождать освобождения
	uint32_t i = 50000;
	while ((temp_tx_head == USART1_tx_tail) && (i))
		i--;
	/*
	if (i == 0)	// защита от зависания
	{
		DBG_HALT(0);
		// перезапустить передатчик
		__disable_irq();
		DBGU_UART->CR1 |= USART_FLAG_TXE;
		__enable_irq();
	}
	*/

	if (USART1_tx_head == USART1_tx_tail)
	{
		USART1_tx_head = temp_tx_head;

		// запустить передачу
		__disable_irq();
		USART1->CR1 |= USART_FLAG_TXE;
		__enable_irq();
	}
	else
		USART1_tx_head = temp_tx_head;

#ifdef _COOCOX_H
	CoSchedUnlock();
#endif
}

#endif

void Port_send_char(char data,uint8_t P)
{
#ifdef _COOCOX_H
	CoSchedLock();
#endif

if (P < 3){//если P=2, то надо отправить на оба

	//uint8_t temp_tx_head;
	//uint8_t
	j = P & 1; //0 if 0,2 ;1 if 1
	//uint8_t
	k = (P>0)?1:0; //0 if 0, 1 if 1, 2
	//P = 1;
	for (;j<=k;j++){
	//P=0: j=0,j<=0,j++
	//P=1: j=1,j<=1,j++
	//P=2: j=0,j<=2,j++

	temp_tx_head = Port_tx_head[j];

	(*((uint8_t *)(Port_tx_buff[j] + temp_tx_head++)))= data;
	temp_tx_head &= UART_TX_WRAPMASK;
	// если буфер занят, подождать освобождения
	i = 50000;
	while ((temp_tx_head == Port_tx_tail[j]) && (i))
		i--;

	Port_tx_head[j] = temp_tx_head;
	// запустить передачу
	__disable_irq();
	*Port_tx_CR1[j] |= USART_FLAG_TXE;
	__enable_irq();
	}
} else if (P == I2C_OUT_LABEL){
	if ((I2C_Bytes_To_Send)<(I2C_TX_SIZE-1)){
		// если буфер занят, подождать освобождения
		i = 50000;
		while ((I2C_stage <0x7f) && (i)) i--;

		I2C_tx_buff[I2C_Bytes_To_Send] = data;
		I2C_Bytes_To_Send++;
	} else {
		Message ("UC Q..: too much to send\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
		USB_main_COM_react();
#endif
	}
} else {//to USB

	USB_COM_TX_Buff1[USB_Bytes_to_send_left++] = data;
	//sending packet in case of data == '\r' or buffer is full.
	if ((USB_Bytes_to_send_left >= USB_EP1_MAX_PACKET_SIZE_TX)){
		USB_main_COM_react();
		// если буфер занят, подождать освобождения
//		uint32_t i = 5000;
//		while (((EP1R & 0x80) == 0) && (i))
//			i--;
	}
}
#ifdef _COOCOX_H
	CoSchedUnlock();
#endif
}
//*-----------------------------------------------------------------------------------------------
/**			Обработчик прерывания UART															*/
//*-----------------------------------------------------------------------------------------------
void DBGU_IRQHandler(void)
{
	//CoEnterISR(); // Enter ISR
	uint32_t status = DBGU_UART->SR;
	//if (status & USART_FLAG_RXNE) {printf("s");}
	if (status & USART_FLAG_RXNE){ //заготовка на приём
    	//printf("s\r\n");
    	char temp_tx_tail = DBGU_UART->DR; //обязательно надо прочитать, иначе будет спамить в прерывание
// отсюда надо будет всё лишнее перенести в отдельную процедуру
    	CoEnterISR();
    	if (ButtonFlags & 0x20){
    		temp_tx_tail &= MAX_NUM_BUTTONS_ACIONS_MASK;
    		ButtonActionImplement(temp_tx_tail);
    	} else {
    	Incomming_Byte_Processing (
    			&temp_tx_tail,
    			&dbgu_State_of_recieved_Command,
    			&dbgu_temp_header,
    			dbgu_rx_buff,
    			&dbgu_rx_tail,
    			DBGU_RX_SIZE,
    			&dbgu_rx_buf_counter,
    			&dbgu_rx_tail_warning,
    			&dbgu_rx_start_time,
    			&dbgu_rx_timeout,
    			&dbgu_rx_head,
    			&dbgu_rx_buf_start//,
    			//&dbgu_State_of_Commands_undescripted
    			);
    	}
    	CoExitISR();

	} else if (status & USART_FLAG_TXE) //передача
	{
		uint8_t temp_tx_tail = Port_tx_tail[1];//dbgu_tx_tail;
		if (temp_tx_tail == Port_tx_head[1])//dbgu_tx_head)//мы уже всё отправили
		{
			DBGU_UART->CR1 &= ~USART_FLAG_TXE;
			DS_Pause_interrupt_Flag &= ~8; //сбрасываем флаг передачи данных на случай если он установлен
			//r2 = CoSetFlag (fileReadStartFlag);
		}
		else
		{ //мы ещё не всё содержимое буфера отправили
			DBGU_UART->DR = dbgu_tx_buff[temp_tx_tail++];
			Port_tx_tail[1] = temp_tx_tail & UART_TX_WRAPMASK;
			//dbgu_tx_tail = temp_tx_tail;
		}
	}
} // DBGU_IRQHandler





void USART1_IRQHandler(void)
{
	//CoEnterISR(); // Enter ISR
	uint32_t status = USART1->SR;
	//if (status & USART_FLAG_RXNE) {printf("s");}
	if (status & USART_FLAG_RXNE){ //заготовка на приём
    	//printf("s\r\n");

    	char temp_tx_tail = USART1->DR; //обязательно надо прочитать, иначе будет спамить в прерывание
    	//DBGU_UART->DR =temp_tx_tail;
    	CoEnterISR();
    	if (ButtonFlags & 0x10){
    		temp_tx_tail &= MAX_NUM_BUTTONS_ACIONS_MASK;
    		ButtonActionImplement(temp_tx_tail);
    	} else {
    	Incomming_Byte_Processing (
    			&temp_tx_tail,
    			&USART1_State_of_recieved_Command,
    			&USART1_temp_header,
    			USART1_rx_buff,
    			&USART1_rx_tail,
    			USART1_RX_SIZE,
    			&USART1_rx_buf_counter,
    			&USART1_rx_tail_warning,
    			&USART1_rx_start_time,
    			&USART1_rx_timeout,
    			&USART1_rx_head,
    			&USART1_rx_buf_start//,
    			//&USART1_State_of_Commands_undescripted
    			);
    	}
    	CoExitISR();

	} else if (status & USART_FLAG_TXE) //передача
	{
		uint8_t temp_tx_tail = Port_tx_tail[0];//USART1_tx_tail;
		if (temp_tx_tail == Port_tx_head[0])//USART1_tx_head)//мы уже всё отправили
		{
			USART1->CR1 &= ~USART_FLAG_TXE;
			DS_Pause_interrupt_Flag &= ~8; //сбрасываем флаг передачи данных на случай если он установлен
			//r2 = CoSetFlag (fileReadStartFlag);
		}
		else
		{ //мы ещё не всё содержимое буфера отправили
			USART1->DR = USART1_tx_buff[temp_tx_tail++];
			Port_tx_tail[0] = temp_tx_tail & UART_TX_WRAPMASK;
			//USART1_tx_tail = temp_tx_tail;
		}
	}
} //USART1_IRQHandler




//*--------------------------------------------------------------------------------------------
/**завершение приёма, уже вне прерывания
 * добавляем на всякий случай два байта в конец, чтобы команда точно была завершена
 * ставим флаг что команда принята
----------------------------------------------------------------------------------------*/
void DBGU_Recieve_finish (uint32_t *dbgu_temp_header_,
		uint8_t *dbgu_State_of_recieved_Command_,
		uint8_t dbgu_rx_buff_[],
		uint16_t *dbgu_rx_tail_,
		uint16_t RX_SIZE,
		uint16_t *dbgu_rx_buf_counter_){
	//dbgu_State_of_recieved_Command равно или 0 (надо в 1), или 0x10, или 0x90 (надо в 0x80)
	*dbgu_temp_header_ = 0; //на случай, если нам недавно прилетало 0xFF, и прерывание начало заполнять заголовок, но не заполнило его до конца
	*dbgu_State_of_recieved_Command_ &=0xF1; //очищаем биты 2,4,8 (та же ситуация, что и строкой выше)
	*dbgu_State_of_recieved_Command_ |=1;
	if (*dbgu_State_of_recieved_Command_ >0x30){//отдаём на расшифровку
		dbgu_rx_buff_[*dbgu_rx_tail_&(RX_SIZE-1)]='\r';
		dbgu_rx_buff_[(*dbgu_rx_tail_&(RX_SIZE-1))+1]='\n';
		*dbgu_rx_tail_+=2;
		*dbgu_rx_buf_counter_ =  *dbgu_rx_tail_&(RX_SIZE-1);
		*dbgu_State_of_recieved_Command_ = 0xA0; //0x20 + 0x80, всё готово к расшифровке
	}

}


//*-----------------------------------------------------------------------------------------------
/**			Отображение ошибки ПО
 *
 * @param file - указатель на строку с названием файла-исходником
 * @param line - номер строки, в которой возникла ошибка										*/
//*-----------------------------------------------------------------------------------------------
void assert_failed(uint8_t * file, uint32_t line)
{
	//printf("\r\nAssert Error in %s line %u\r\n",file, (unsigned int)line);
}
