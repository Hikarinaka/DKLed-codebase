#include "includes.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"

uint8_t i2c8; //временная переменная - счетчик
uint16_t i2c16; //временная переменная
uint8_t icc;
uint16_t ic;

uint8_t I2C_TargetAddress;//куда отправлять
uint8_t I2C_stage = 0;
//uint8_t I2C_currentStep = 0;
uint16_t I2C_sended_byte_index = 1;
uint8_t I2C_Bytes_To_Send = 0;
uint32_t I2C_eventListener;
uint8_t I2C_SelfAddress = 0; //собственный адрес контроллера
//uint16_t I2C_FailedAttemptCount = 0;
uint8_t I2C_Bytes_Recieved = 0;
uint8_t I2C_Bytes_to_Recieve = 0;
//uint8_t I2C_ByteInProcess = 0;

uint8_t I2C_tx_buff[I2C_TX_SIZE];		///< буфер передатчика uart
uint8_t I2C_rx_buf[I2C_RX_SIZE+2];

uint32_t I2C_ResetTime; //время сброса порта на случай, если он зависнет
uint32_t event = 0, flag2 = 0;

extern uint8_t ButtonFlags;
extern uint16_t DS_Pause_interrupt_Flag; //поднимаеется 0x200, если мы ждём окончания передачи данных

/* IMPORTAINT
 *
 * It seem that I2C code is sensitive to the location of operations in ROM
 * so it may be nesessary to add some dummy operations
 * like repeating same value assignment
 * */



#if (0)
void (*I2C_act[4])(uint8_t) =
	{
			I2C_START,
			I2C_SetTargetReceiver,
			I2C_SendByte,
			I2C_STOP,
	};

void I2C_START(uint8_t b){
	I2C1->CR1 |= CR1_START_Set;
	I2C_stage = 1;
}

void I2C_SetTargetReceiver(uint8_t b){
	b |= OAR1_ADD0_Set;
	I2C1->DR = b;
	I2C_stage = 2;
}

void I2C_SendByte(uint8_t b){
	I2C_stage = 3;
	if (b) {
		I2C1->DR = b;
		I2C_stage = 2;
	}
}

void I2C_STOP(uint8_t b){
	I2C1->CR1 |= CR1_STOP_Set;
	I2C_stage = 0;
}
#endif


void I2C1_DeInit(void)
{

#ifndef SoftwareI2C
	NVIC->ICER[I2C1_EV_IRQn >> 0x05] =(uint32_t)0x01 << (I2C1_EV_IRQn & (uint8_t)0x1F);
	NVIC->ICER[I2C1_ER_IRQn >> 0x05] =(uint32_t)0x01 << (I2C1_ER_IRQn & (uint8_t)0x1F);

	I2C1->CR1 &= 0xFFFE;
	RCC->APB1ENR &= ~RCC_APB1Periph_I2C1;

    /* Enable I2C1 reset state */
    //RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC->APB1RSTR |= RCC_APB1Periph_I2C1;
    /* Release I2C1 from reset state */
    //RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
    RCC->APB1RSTR &= ~RCC_APB1Periph_I2C1;

	//RCC->APB2ENR |= RCC_APB2Periph_GPIOB;
#endif
    ButtonFlags &=~0x40;
    ButtonsInit();
    DS_Pause_interrupt_Flag &=~0x200;

}


#ifndef SoftwareI2C


void I2C1_init(uint8_t address)
{
    I2C_tx_buff[I2C_TX_MAX] = 0;
	//I2C_InitTypeDef  I2C_InitStructure;
    //GPIO_InitTypeDef  GPIO_InitStructure;
    ButtonFlags |=0x40;
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC->APB1ENR &= ~RCC_APB1Periph_I2C1;
    I2C1->CR1 = 0;
    I2C1->CR2 = 0;
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC->APB2ENR |= RCC_APB2Periph_GPIOB;
    RCC->APB2ENR |= RCC_APB2Periph_AFIO;
    RCC->APB1ENR |= RCC_APB1Periph_I2C1;

    /* Configure I2C_EE pins: SCL and SDA */
    //GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;//0x00C0
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//3
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//0x1C
    //GPIO_Init(GPIOB, &GPIO_InitStructure);

     /* I2C configuration */
    //I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;//0
    //I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;//0xBFFF
    //I2C_InitStructure.I2C_OwnAddress1 = 0x38;//11st device own address
    //I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;//0x0400
    //I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;//0x4000
    //I2C_InitStructure.I2C_ClockSpeed = 100000;//clock freq

  	NVIC->IP[I2C1_EV_IRQn] = 0xC0;///низший приоритет из возможных 0xf0, высший 0x10
  	NVIC->ISER[((uint32_t)(I2C1_EV_IRQn) >> 5)] = (1 << ((uint32_t)(I2C1_EV_IRQn) & 0x1F)); /* enable interrupt */
  	NVIC->IP[I2C1_ER_IRQn] = 0xC0;///низший приоритет из возможных 0xf0, высший 0x10
  	NVIC->ISER[((uint32_t)(I2C1_ER_IRQn) >> 5)] = (1 << ((uint32_t)(I2C1_ER_IRQn) & 0x1F)); /* enable interrupt */

    /* I2C Peripheral Enable */
    uint16_t freqrange = (PCLK1_Freq / I2C_Clock_Speed);//360
	I2C1->CR2 |= (PCLK1_Freq/1000000);

    I2C1->CCR = (freqrange<0x08) ? 0x04 : (freqrange>>1);//rcc_clocks.PCLK1_Frequency/(100000<<1)
	//I2C1->CCR = I2C_Halffreqrange;
	I2C1->TRISE = (PCLK1_Freq/1000000)+1;
    I2C1->CR1 |= I2C_CR1_ACK_Set | 0x0000;// Ack enable, Mode: SMB = Device, SMBus = I2C


    //I2C_Cmd(I2C1, ENABLE);

    /* Apply I2C configuration after enabling it */
    //I2C_Init(I2C1, &I2C_InitStructure);
    //I2C1->CR2 &= 0xFFC0;
    //I2Cx->CR2 |= 0x000A; //set freq range to 10 MHz
    //RCC_ClocksTypeDef  rcc_clocks;
    //RCC_GetClocksFreq(&rcc_clocks);


    //I2C1->CR1 &= 0xFBF5;
    /*---------------------------- GPIO Mode Configuration -----------------------*/
      //currentmode = 0x1C & ((uint32_t)0x0F);//0x0C
      //currentmode |= GPIO_Speed_50MHz;//0xC | 0x3 = 0xF
    /*---------------------------- GPIO CRL Configuration ------------------------*/
      /* Configure the eight low port pins */
        GPIOB->CRL &=0x00FFFFFF;
        GPIOB->CRL |=0xFF000000;
    I2C_SelfAddress = address;
    address = address<<1;
    I2C1->OAR1 = 0x4000 | address;//1-st device own address
    I2C1->OAR2 = 0x40 | 0x00;// 0x00 - only OAR1 is valid, 0x01 - both OAR1 and OAR2 are valid

    I2C1->CR2 |= 0x0700;//0x0100 - IT ERR enable, 0x0200 - IT Event enable
    //I2C_eventListener = ~I2C_FLAG_Mask;
    I2C1->CR1 |= 0x0001;//0x0001 - 1 enables the I2C

    Message("I2C on\r\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
		USB_main_COM_react();
#endif
	DS_Pause_interrupt_Flag &=~0x200;
	I2C_Bytes_To_Send = 0;
    I2C_Bytes_to_Recieve = 0;
    I2C_stage = 0x7f;
}



void I2C1_Send()
{

		//I2C_Working
	//next changes speed to 80 000
	//I2C still working
	  //I2C still working
	//print_0X4(flag2,15);
	  //print_0X4(event,15);
	  //Port_send_char(',',15);
	  //print_0X4(I2C1->CR1,15);
	  //Port_send_char(';',15);
	  //USB_main_COM_react();
	  //for (i2c16 = 0;i2c16<1000;i2c16++){}
	  //cutting off is it in IRQ check
	//now it is working again

	//simple receive is also done and finally do not hang the controller
	//now we need to create a way to turn I2C expand board into a keyboard
	switch (I2C_stage){
		case 8:
			//I2C1->CR2 |= 0x1000;
		case 0:
			//Port_send_char(0x0D,15);
			I2C1->CR1 &= ~(I2C_CR1_START_Set | I2C_CR1_STOP_Set | I2C_CR1_PEC_Set );//wait for clear START, STOP and PEC bits
			I2C1->CR1 &= I2C_CR1_PEC_Reset;
			I2C1->CR2 |= 0x0700;
			//for (i2c16 = 0;i2c16<3000;i2c16++){}
			I2C1->CR1 |= I2C_CR1_ACK_Set | 0x0001;
			I2C_eventListener = I2C_EVENT_MASTER_MODE_SELECT;//not through: 1(SB in SR1), 1 (MSL, SR2); through - 2(Busy in SR2)
			I2C_stage ++;//0 to 1 or 8 to 9
			//Port_send_char('0',15);

			I2C1->CR1 |= I2C_CR1_START_Set;
			//I2C1->CR1;
			//print_0X4(I2C1->CR1,15);
			break;
		case 1:
			I2C_eventListener = I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED;
			I2C_stage = 2;//1 to 2 or 9 to 10
			I2C_sended_byte_index = 0;
			//Port_send_char('1',15);
			I2C1->DR = (I2C_TargetAddress<<1);//I2C_tx_buff[0];
			break;
		case 9:
			I2C_eventListener = I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED;
			I2C_stage = 10;
			I2C_Bytes_Recieved = 0;
			I2C1->DR = (I2C_TargetAddress<<1) | 1;//I2C_tx_buff[0];
			break;
		case 2:
			//I2C_stage = 3;
			I2C_eventListener = I2C_EVENT_MASTER_BYTE_TRANSMITTED;
			if ((I2C_sended_byte_index<I2C_Bytes_To_Send) && (I2C_sended_byte_index<I2C_TX_SIZE)) {
				I2C_stage = 2;
				//I2C_eventListener = I2C_EVENT_MASTER_BYTE_TRANSMITTED;
				//Port_send_char('2',15);
				I2C_sended_byte_index++;
				I2C1->DR = I2C_tx_buff[I2C_sended_byte_index-1];
				break;
			}
		case 3:
			//I2C_eventListener = I2C_EVENT_MASTER_BYTE_TRANSMITTED;
			//I2C_Bytes_To_Send = I2C_sended_byte_index;
			//Message("I2C:\0",15);
			//for (I2C_sended_byte_index = 0; I2C_sended_byte_index<I2C_Bytes_To_Send; I2C_sended_byte_index){Port_send_char(I2C_tx_buff[I2C_sended_byte_index],0xf);}
			//Port_send_char('3',15);
			//Port_send_char(0x0D,15);
			//USB_main_COM_react();
			I2C_Bytes_To_Send=0;
			I2C_stage = 0x7f;
			DS_Pause_interrupt_Flag &=~0x200;
			I2C1->CR1 |= I2C_CR1_STOP_Set;

			break;
		case 10:
			I2C_stage = 11;
			I2C_eventListener = I2C_EVENT_MASTER_BYTE_RECEIVED;
			break;
		case 11:


			I2C_eventListener = I2C_EVENT_MASTER_BYTE_RECEIVED;
			I2C_Bytes_Recieved ++;
			I2C_rx_buf[I2C_Bytes_Recieved-1] = I2C1->DR;

			if (I2C_Bytes_Recieved >= I2C_Bytes_to_Recieve){
				I2C_stage = 12;
				I2C1->CR1 |= I2C_CR1_STOP_Set;
				I2C1->CR1 &= I2C_CR1_ACK_Reset;
			}
			break;
		case 12:
			//if (I2C_stage == 0x7f){
			//I2C_Bytes_Recieved ++;
			I2C_rx_buf[I2C_Bytes_Recieved] = I2C1->DR;
			I2C_stage = 0x7f;
			DS_Pause_interrupt_Flag &=~0x200;
				Message("I2C: \0",15);
				//Port_send_char(DS_inttochar((I2C_Bytes_Recieved>>4)&0x0F),15);
				//Port_send_char(DS_inttochar(I2C_Bytes_Recieved&0xF),15);
				//Port_send_char(':',15);
				//I2C_Bytes_Recieved --;
				for (icc = 0; icc<=I2C_Bytes_Recieved; icc++){
					Port_send_char(DS_inttochar((I2C_rx_buf[icc]>>4)&0x0F),15);
					Port_send_char(DS_inttochar(I2C_rx_buf[icc]&0xF),15);
					Port_send_char(' ',15);
				}
				Port_send_char(0x0D,15);
				USB_main_COM_react();
				I2C_Bytes_to_Recieve = 0;

			break;
		default:
			I2C_Bytes_To_Send=0;
			I2C_Bytes_to_Recieve = 0;
			DS_Pause_interrupt_Flag &=~0x200;
			I2C_stage = 0x7f;
			break;
	}

}



uint8_t I2C_Check(uint32_t I2C_EVENT)
{
  uint32_t flag1 = 0, flag2 = 0;

  /* Read the I2Cx status register */
  flag1 = I2C1->SR1;
  flag2 = I2C1->SR2;

  /* Get the last event value from I2C status register */
  flag1 = (flag1 | (flag2<<16)) & I2C_FLAG_Mask;

  /* Check whether the last event contains the I2C_EVENT */
	if ((flag1 & I2C_EVENT) == I2C_EVENT)
	{
    /* SUCCESS: last event is equal to I2C_EVENT */
		return 0xff;
  	}
    return 0;//return ERROR
}


void I2C_Not_Enabled_Message(){
	Message ("I2C disabled, use UCA<address> to enable\r\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
		USB_main_COM_react();
#endif
}


uint8_t I2C1_ForceSend(){
	if ((ButtonFlags & 0x40) == 0){
		I2C_Not_Enabled_Message();
		return 0;
	}

	//I2C_stage = 0;

	Message ("I2C scan..\r\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
	USB_main_COM_react();
#endif
	//i2c8=0x76;
	I2C_tx_buff[0]=0;
	for (i2c8=1; i2c8<128; i2c8++){
		I2C_TargetAddress = i2c8;
		ic = 8000;
		I2C_Bytes_To_Send = 0;
		start_I2C(0);
		while((I2C_stage<0x70) && ic){
			ic--;
		}

		if (ic || (I2C_stage > 2)){//странно но работает только когда ОБА эти условия присутствуют
			Port_send_char(DS_inttochar((i2c8>>4)&0x0F),DEBUG_PORT_OUT);
			Port_send_char(DS_inttochar(i2c8&0xF),DEBUG_PORT_OUT);
			Port_send_char(0x0D,DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
			USB_main_COM_react();
			for (ic = 0;ic<1000;ic++){}//a short delay to allow port for transmission
#endif
		}
		//for (ic = 0;ic<10000;ic++){}
		//if (I2C_FailedAttemptCount & 1) {Port_send_char('_',15);}
		//if (I2C_FailedAttemptCount & 2) {Port_send_char('-',15);}
		//Port_send_char(0x0D,15);
		//I2C_FailedAttemptCount = 0;
		//USB_main_COM_react();

	}
	Message ("..finished\r\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
	USB_main_COM_react();
#endif
	DS_Pause_interrupt_Flag &=~0x200;
return 0xff;


}


//Start transmitting or recieving data through I2C
//dir - Read from slave (1) or Write to slave (0)
//I2C_TargetAddress - slave address to interact with
//I2C_Bytes_to_Recieve - amount of bytes to read from slave
//I2C_Bytes_To_Send - amount of bytes in the buffer
void start_I2C(uint8_t dir)
{
	if (ButtonFlags & 0x40){
		//dir =  0; //I2C_OAR1_ADD0_Reset;
		//I2C_currentStep = 0;
		I2C_stage = (dir & 1)<<3;
		I2C_sended_byte_index = 0;
		//I2C_FailedAttemptCount = 0;
		//I2C_Bytes_To_Send++;
		//I2C_eventListener = ~I2C_FLAG_Mask;
		//I2C_tx_buff[0]=(I2C_TargetAddress<<1) | (dir & 1);
		I2C_ResetTime = 0; //TGetOSTime() + I2C_Maximum_Wait_ms;
/*		I2C1->CR1 &= I2C_CR1_PE_Reset;

		I2C1->CR2 |= 0x0700;
		I2C1->CR1 &= ~(I2C_CR1_START_Set | I2C_CR1_STOP_Set | I2C_CR1_PEC_Set );

		I2C1->CR1 |= I2C_CR1_ACK_Set| I2C_CR1_PE_Set;
		//for (i2c16 = 0;i2c16<3000;i2c16++){}
		I2C1->CR1 |= I2C_CR1_START_Set;
		//while((I2C1->SR2 & I2C_SR2_MSL) || (I2C1->CR1 & I2C_CR1_STOP)){} //Stop bit is checked because of the problem I have
/**/
		//I2C1->CR1;
		//I2C1->SR2;
		DS_Pause_interrupt_Flag |=0x200;
		I2C_sended_byte_index = 0;
		I2C1_Send();
	} else {
		I2C_Not_Enabled_Message();
	}
}


void I2C1_EV_IRQHandler(void) {
  // Reading last event
  event = I2C1->SR1;
  flag2 = I2C1->SR2;
  // Get the last event value from I2C status register
  //print_0X4(flag2,15);
  //print_0X4(event,15);
  //USB_main_COM_react();
  event = (event | (flag2<<16)) & I2C_FLAG_Mask;
  //I2C_FailedAttemptCount |= 1;
  if((event & I2C_eventListener) == I2C_eventListener) {I2C1_Send();}
/*
  //uint8_t buff_sr1=I2C1->SR1;
      if(event & I2C_SR1_SB){ //SB bit is set(unsetting by read SR1 and writing adress to DR)
          I2C1->DR=(I2C_TargetAddress<<1);
      }else if(event & I2C_SR1_ADDR){ //ADDR bit is set(unsetting by read SR1 and read SR2)
          (void)I2C1->SR2;
      }
      if ((event & I2C_SR1_TXE)&& (I2C_sended_byte_index<I2C_Bytes_To_Send)){  //Checking TxE( clearing by writting to DR)
          I2C1->DR=I2C_tx_buff[I2C_sended_byte_index];
          I2C_sended_byte_index++;
      } else if((event & I2C_SR1_TXE)&&(event & I2C_SR1_BTF)){ //Checking stop
        //condition(TxE=1,BTF=1,counter=0)
          (void)I2C1->SR1; //Dont know why, but it works(just read some I2C1 register)
          I2C1->CR1 |= I2C_CR1_STOP;  //Generate stop condition
			I2C_Bytes_To_Send=0;
			I2C_Bytes_to_Recieve = 0;
			I2C_stage = 0x7f;
      }/**/

}



void I2C1_ER_IRQHandler(void) {
	//if (I2C_GetITStatus(I2C1, 0x01000400)) {
    //I2C_ClearITPendingBit(I2C1, 0x01000400);}
	//uint32_t event = 0, flag2 = 0;
	event =	I2C1->SR1;
	flag2 =	I2C1->SR2;
	//event = (event | (flag2<<16)) & I2C_FLAG_Mask;
	DS_Pause_interrupt_Flag &=~0x200;
	//I2C_FailedAttemptCount |= 2;
	if ((I2C1->SR1 & 0x0400)){
		I2C1->SR1 &= ~0x0400;
	}

}




void Check_If_I2C_Is_AFK(){
	if (ButtonFlags & 0x40){
		if (I2C_Bytes_To_Send | I2C_Bytes_to_Recieve){// && (I2C_ResetTime < TGetOSTime())){
			I2C_ResetTime++;
			if (I2C_ResetTime > I2C_Maximum_Wait_ms){
				I2C1_init(I2C_SelfAddress);
			}
		}
	}
}


#else


if (I2C_Bytes_Recieved >= I2C_Bytes_to_Recieve){
	I2C_stage = 0x7f;
	I2C_Bytes_to_Recieve = 0;
	I2C1->CR1 |= I2C_CR1_STOP_Set;
	I2C1->CR1 &= I2C_CR1_ACK_Reset;
}
I2C_rx_buf[I2C_Bytes_Recieved] = I2C1->DR;
I2C_eventListener = I2C_EVENT_MASTER_BYTE_RECEIVED;
I2C_Bytes_Recieved ++;

if (I2C_stage == 0x7f){
	uint8_t icc;
	Message("I2C \0",15);
	Port_send_char(DS_inttochar((I2C_Bytes_Recieved>>4)&0x0F),15);
						Port_send_char(DS_inttochar(I2C_Bytes_Recieved&0xF),15);
	Port_send_char(':',15);
	for (icc = 0; icc<I2C_Bytes_Recieved; icc++){
		Port_send_char(DS_inttochar((I2C_rx_buf[icc]>>4)&0x0F),15);
		Port_send_char(DS_inttochar(I2C_rx_buf[icc]&0xF),15);
		Port_send_char(' ',15);
	}
	Port_send_char(0x0D,15);
	USB_main_COM_react();
}

break;

/*
case 11:
I2C_rx_buf[I2C_Bytes_Recieved] = I2C1->DR;
if (I2C_Bytes_Recieved >= I2C_Bytes_to_Recieve){
	I2C_stage = 12;
	I2C1->CR1 |= I2C_CR1_STOP_Set;
	I2C1->CR1 &= I2C_CR1_ACK_Reset;
}
I2C_eventListener = I2C_EVENT_MASTER_BYTE_RECEIVED;
I2C_Bytes_Recieved ++;
break;
case 12:
I2C_stage = 0x7f;
I2C_Bytes_to_Recieve = 0;
I2C_rx_buf[I2C_Bytes_Recieved] = I2C1->DR;
if (I2C_stage == 0x7f){
	uint8_t icc;
	Message("I2C \0",15);
	Port_send_char(DS_inttochar((I2C_Bytes_Recieved>>4)&0x0F),15);
	Port_send_char(DS_inttochar(I2C_Bytes_Recieved&0xF),15);
	Port_send_char(':',15);
	for (icc = 0; icc<I2C_Bytes_Recieved; icc++){
		Port_send_char(DS_inttochar((I2C_rx_buf[icc]>>4)&0x0F),15);
		Port_send_char(DS_inttochar(I2C_rx_buf[icc]&0xF),15);
		Port_send_char(' ',15);
	}
	Port_send_char(0x0D,15);
	USB_main_COM_react();
}

break;
*/



















//software I2C implementation based on timers


uint8_t Perform_I2C_Step(){
	//TIM3->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
	I2C_eventListener = 0;
//I2C_currentStep correspondence to stages:
//	0...3 - Send START
//	4...30  - send byte: 4 - set MSB (bit 7),
//		5,8,11,14,17,20,23,26-clock up,
//		6,9,12,15,18,21,24,27...-clock down,
//		7 - set bit 6, 10-bit 5, 13(4),16(3),19(2),22(1),25(0)
//		28(SDA to listen, next bit is ACK from slave)
//		29 - clock up, check for SDA to be LOW
//		30 - clock down, prepare next byte to send
//	31... - send STOP
	switch (I2C_currentStep){
		case 0:
			//Port_send_char('1',15);
			//USB_main_COM_react();
			I2C_eventListener = I2C_Event_SDA_IsUp;
		case 28://prepare for ACK in Stage 2
			I2C_SDA_SetFree;
			goto End_Step_Label;
		case 1:
			I2C_SCL_SetFree;
			I2C_eventListener = I2C_Event_SDA_IsUp | I2C_Event_SCL_IsUp;
			goto End_Step_Label;
		case 31:
			Port_send_char('3',15);
			Port_send_char(0x0D,DEBUG_PORT_OUT);
			USB_main_COM_react();
		case 2:
			I2C_SDA_SetLow;
			goto End_Step_Label;
		case 3:
			I2C_stage = 2;
			I2C_SCL_SetLow;
			goto End_Step_Label;
		case 4://send MSBit (bit 7)
			Port_send_char('2',15);
			USB_main_COM_react();
			I2C_ByteInProcess = I2C_tx_buff[I2C_sended_byte_index];
			I2C_sended_byte_index++;
		case 7:	//send bit 6
		case 10: //send bit 5
		case 13: //send bit 4
		case 16: //send bit 3
		case 19: //send bit 2
		case 22: //send bit 1
		case 25: //send bit 0
			if (I2C_ByteInProcess & 0x80){
				I2C_SDA_SetFree;
				I2C_eventListener = I2C_Event_SDA_IsUp;
			} else {
				I2C_SDA_SetLow;
			}
			I2C_ByteInProcess <<=1;
			goto End_Step_Label;
		case 5: //clock up to allow slave to read certain bit
		case 8:
		case 11:
		case 14:
		case 17:
		case 20:
		case 23:
		case 26:
		case 32: //Generate STOP (SCL Up before SDA Up)
			I2C_SCL_SetFree;
			I2C_eventListener = I2C_Event_SCL_IsUp;
			goto End_Step_Label;
		case 30:
			I2C_stage = 3;
			if ((I2C_sended_byte_index<I2C_Bytes_To_Send) && (I2C_sended_byte_index<I2C_TX_SIZE-1)) {
				I2C_currentStep = 3; //4-1
				I2C_stage = 2;
			}
		case 6:
		case 9:
		case 12:
		case 15:
		case 18:
		case 21:
		case 24:
		case 27:
			I2C_SCL_SetLow;
			goto End_Step_Label;
		case 29: //Wait for ACK at stage 2
			I2C_eventListener = I2C_Event_SDA_IsDown;
			I2C_SCL_SetFree;
			goto End_Step_Label;
		case 33://generate STOP, last step
			I2C_SDA_SetFree;
			I2C_stage = 0;
			stop_I2C();
			return 2;//success, finished
		default:
			I2C_SDA_SetFree;
			I2C_SCL_SetFree;
			I2C_stage = 0;
			stop_I2C();
			Message ("I2C Failed\r\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
			USB_main_COM_react();
#endif
			return 0;
	}

End_Step_Label:
	I2C_currentStep++;
	//TIM3->CR1 |= TIM_CR1_CEN;//запуск
	return 1;//success, continue
}



void I2C_preInit(){
	// запуск таймера для генерации тиков (каждые 10 мкс)
	RCC->APB1ENR |= RCC_APB1Periph_TIM3;

	//для сервы
	//TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//TIM_TimeBaseStructure.TIM_Period = 1000;//SERVO_DEAD_BAND; //10 мкс, == dead band width
	TIM3->ARR = I2C_TIMER_TicksPerStep;
	//TIM_TimeBaseStructure.TIM_Prescaler = SERVO_TIMER_PRESC-1;	// 1 MГц тактирование
	TIM3->PSC = I2C_TIMER_PRESC-1;	// 100 KГц тактирование
	//TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	//TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM3->CR1 &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD)));
	TIM3->CR1 |= TIM_CounterMode_Up|0;
	//TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	//TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM3->EGR = 1;
	//TIM_ARRPreloadConfig(TIM5, ENABLE);
	TIM3->CR1 |= TIM_CR1_ARPE;
	// событие при переполнении
	//TIM_ITConfig(TIM5, TIM_DIER_UIE, ENABLE);
	TIM3->DIER |= TIM_DIER_UIE;
	TIM3->EGR |= TIM_EGR_UG;
	//TIM5->EGR = TIM_PSCReloadMode_Immediate;
	// прерывание
	NVIC->IP[TIM3_IRQn] = (0xB0);
	//NVIC_EnableIRQ(TIM2_IRQn);
	NVIC->ISER[((uint32_t)(TIM3_IRQn) >> 5)] = (1 << ((uint32_t)(TIM3_IRQn) & 0x1F)); /* enable interrupt */
}

void I2C1_init(uint8_t address){
	//I2C_InitTypeDef  I2C_InitStructure;
    //GPIO_InitTypeDef  GPIO_InitStructure;
    ButtonFlags |=0x40;

    // Configure the eight low port pins
    // Configure I2C_EE pins: SCL (Pin7) and SDA (pin0)
      //GPIOB->CRL &=0x0FFFFFF0;//не нужно так как мы поднимаем все биты и так
    GPIOB->CRL |=I2C_GPIO_CRL_Mask;



	I2C_SelfAddress = address<<1;

//    Message("I2C on\r\0",15);
//	USB_main_COM_react();
    I2C_stage = 0;

}



void I2C1_Send(){
    Message("empty\r\0",15);
	USB_main_COM_react();
}




uint8_t I2C1_ForceSend(){
	Message ("Listing I2S..\r\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
	USB_main_COM_react();
#endif
	i2c8=0x76;
	for (i2c8=0; i2c8<128; i2c8++){
		I2C_stage = 0;
		I2C_Bytes_To_Send=1;
		I2C_TargetAddress = i2c8;
		start_I2C(0);

		while (I2C_stage){}

		if (I2C_FailedAttemptCount <= I2C_Maximum_ACK_waits){
			Port_send_char(DS_inttochar((i2c8>>4)&0x0F),DEBUG_PORT_OUT);
			Port_send_char(DS_inttochar(i2c8&0xF),DEBUG_PORT_OUT);
			Port_send_char(0x0D,DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
			USB_main_COM_react();
#endif
		}

	}
	Message ("..finished\r\0",DEBUG_PORT_OUT);
#if (DEBUG_PORT_OUT == 0xf)
	USB_main_COM_react();
#endif

return 0xff;

}

//команды запуска I2C
void start_I2C(uint8_t dir)
{

	dir =  0; //I2C_OAR1_ADD0_Reset;
	I2C_stage = 1;
	I2C_currentStep = 0;
	I2C_sended_byte_index = 0;
	I2C_FailedAttemptCount = 0;
	I2C_eventListener = 0;
	I2C_tx_buff[0]=(I2C_TargetAddress<<1) | (dir & 1);
	//Port_send_char('0',15);
	//USB_main_COM_react();

	TIM3->CNT = 0;
	TIM3->CR1 |= TIM_CR1_CEN;//запуск
	TIM3->ARR = I2C_TIMER_TicksPerStep; //шаг
	TIM3->EGR |= TIM_EGR_UG; //обновить актуальные тайминги (иначе они обновляются ПОСЛЕ следующего прерывания)

	TIM3->DIER |= TIM_DIER_UIE;



}

//остановка I2C
void stop_I2C()
{
	TIM3->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
	I2C_stage = 0;
    //Message("I2C stop\r\0",15);
	//USB_main_COM_react();
	//GPIOB->BRR = 0x00A0;//GPIO_ResetBits(GPIOB,0xFF00);

}



// обработчик прерывания счётного таймера
void TIM3_IRQHandler(void)
{
	CoEnterISR(); // Enter ISR

	if ((I2C_eventListener == I2C_Event_SDA_IsDown) && ((I2C_Status_Port & I2C_SDA_Pin)==0)) I2C_eventListener = 0;
	if ((I2C_eventListener == I2C_Event_SDA_IsUp) && ((I2C_Status_Port & I2C_SDA_Pin))) I2C_eventListener = 0;
	if ((I2C_eventListener == I2C_Event_SCL_IsUp) && ((I2C_Status_Port & I2C_SCL_Pin))) I2C_eventListener = 0;
	if ((I2C_eventListener == (I2C_Event_SDA_IsUp | I2C_Event_SCL_IsUp)) && ((I2C_Status_Port & (I2C_SDA_Pin | I2C_SCL_Pin))==(I2C_SDA_Pin | I2C_SCL_Pin))) I2C_eventListener = 0;

	if (I2C_eventListener == 0) {
		I2C_FailedAttemptCount = 0;
		Perform_I2C_Step();
	} else {
		I2C_FailedAttemptCount ++;
	}
	if (I2C_FailedAttemptCount > I2C_Maximum_ACK_waits) {

		stop_I2C();
	}

	TIM3->ARR = I2C_TIMER_TicksPerStep; //шаг
	TIM3->SR &= ~TIM_SR_UIF; //если сделать это раньше, то прерывание наступит сразу по обновлению таймингов

	CoExitISR(); // Exit ISR

}




#endif
