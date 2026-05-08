/*************************************************************************************************
 * @file		main.c
 *
 * @brief		Заголовок интерфейса DBGU
 *
 * @version		v1.0
 * @date		05.09.2013
 * @author		Mike Smith
 *
 * @note		Передача осуществляется через кольцевой буфер, что существенно уменьшает
 * 				задержку
 ************************************************************************************************/
#ifndef DBGU_H_
#define DBGU_H_

//*-----------------------------------------------------------------------------------------------
//*			Константы
//*-----------------------------------------------------------------------------------------------
#define UART_TX_WRAPMASK			0x1F	//буфер передачи закольцован, и чтобы не выйти за его границы нужно обрезать переменную - счётчик

#define DBGU_RX_SIZE				256			// размер буфера для чтения usart2
#define DBGU_TX_SIZE				UART_TX_WRAPMASK+1			// размер буфера для отправки usart2

#define USART1_RX_SIZE				256			// размер буфера для чтения usart2
#define USART1_TX_SIZE				UART_TX_WRAPMASK+1			// размер буфера для отправки usart1

//*-----------------------------------------------------------------------------------------------
//*			Описание отладочного UART
//*-----------------------------------------------------------------------------------------------
// DBG_TXD PA2 mode = 11 (50 MHz), CNF = 10 (alternate puhs-pull)
// DBG_RXD PA3 mode = 0, cnf=10 (pull up input)
/*#define DBG_PORT_INIT do{ GPIOA->CRH &= ~(GPIO_CRH_CNF9 + GPIO_CRH_CNF10 + GPIO_CRH_MODE9 + GPIO_CRH_MODE10;\
						GPIOA->CRH |= GPIO_CRH_MODE9 + GPIO_CRH_CNF9_1 + GPIO_CRH_CNF10_1;\
						} while(0)
*/

#define DBGU_UART			USART2
#define DBGU_UART_CLK_ENABLE	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)
#define USART1_CLK_ENABLE	RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, ENABLE);

#define DBGU_UART_PORT		GPIOA
#define DBGU_UART_TX_PIN	GPIO_Pin_2
#define DBGU_UART_RX_PIN	GPIO_Pin_3
#define DBGU_IRQHandler		USART2_IRQHandler
#define DBGU_UART_IRQn		USART2_IRQn
#define IRQ_UART_DBGU_PRIORITY		4

#define USART1_PORT		GPIOA
#define USART1_TX_PIN	GPIO_Pin_9
#define USART1_RX_PIN	GPIO_Pin_10
#define IRQ_USART1_PRIORITY		4


#define util_symbol	0x0D	//'я'
#define univ_symbol	0x2A	//'*'


#define USART_Default_timeout 	1000 //in ms

//*-----------------------------------------------------------------------------------------------
//*			Функции модуля
//*-----------------------------------------------------------------------------------------------
void DBGU_Init(uint32_t BaudRate);
void USART1_Init(uint32_t BaudRate);
//void dbgu_send_char(signed int data);
//void USART1_send_char(signed int data);
void Port_send_char(char data,uint8_t P);
void DBGU_Recieve_finish (uint32_t *dbgu_temp_header_,
		uint8_t *dbgu_State_of_recieved_Command_,
		uint8_t dbgu_rx_buff_[],
		uint16_t *dbgu_rx_tail_,
		uint16_t RX_SIZE,
		uint16_t *dbgu_rx_buf_counter_);

#endif /* DBGU_H_ */
