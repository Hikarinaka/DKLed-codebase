// организация работы кнопок: инициализация, прерывания


// все кнопки подключены на PortB, нажатие на кнопку "сажает" пин на землю
// соответствие пинов и кнопок:
// Button1 => Pin4
// Button2 => Pin5
// Button3 => Pin6
// Button4 => Pin7
// Button5 => Pin0
// Button6 => Pin1

//Для первой версии с кнопками (25/10/17) будут работать кнопки с 1 по 4

#include "buttons.h"
//#include "includes.h"


//*-----------------------------------------------------------------------------------------------
//*			Переменные
//*-----------------------------------------------------------------------------------------------

//extern BYTE descrActionFlag; //битовые флаги разных действий в процессе цикла чтения/расшифровки, константы в DS_ACTION_...
//extern uint8_t buttonPressed; //какая кнопка была нажата только что
//extern BYTE buttonsState; //запоминаем состояние кнопок, чтобы правильно ловить нажатие/отпускание (по битам)
//BYTE buttonsNewState; //текущее состояние нажатости кнопок, чтобы ловить что изменилось
//BYTE buttonsStateTMP;

//uint16_t portBtmp; //временное хранение входа B
//uint32_t buttonInterruptState; //для запоминания, какое прерывание сработало

void ButtonsInit(void)
{
//	  GPIO_InitTypeDef GPIO_InitStructure;
	  //RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );
		RCC->APB2ENR |= RCC_APB2Periph_GPIOB;

	  //RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
	  // GPIOB pins WS2812 data outputs (PB8...PB15)
//	  GPIO_InitStructure.GPIO_Pin = 0b11110011; //было 0b01111000 о факту 456701
//	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	  GPIO_Init( GPIOB, &GPIO_InitStructure );
#if defined (O6I4U2)
		GPIOB->CRL &= 0x0000FFFF;
		GPIOB->CRL |= 0x44440000;

		  //GPIO_Write(GPIOB, 0);
		GPIOB->BRR = 0b11110000;//GPIO_ResetBits(GPIOB, 0b11110011);

#elif defined (O8I5U2)
		GPIOB->CRL &= 0x0000FFF0;
		GPIOB->CRL |= 0x44440004;

		 //GPIO_Write(GPIOB, 0);
		GPIOB->BRR = 0b11110001;//GPIO_ResetBits(GPIOB, 0b11110011);

#elif defined (O8I6U2)
		GPIOB->CRL &= 0x0000FF00;
		GPIOB->CRL |= 0x44440044;

		 //GPIO_Write(GPIOB, 0);
		GPIOB->BRR = 0b11110011;//GPIO_ResetBits(GPIOB, 0b11110011);

#elif defined (O4I3)
		GPIOB->CRL &= 0xF000FFFF;
		GPIOB->CRL |= 0x04440000;

		//GPIO_Write(GPIOB, 0);
		GPIOB->BRR = 0b01110000;//GPIO_ResetBits(GPIOB, 0b11110011);

#endif


}




