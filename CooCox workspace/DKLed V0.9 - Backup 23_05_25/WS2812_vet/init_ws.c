//#include <stm32f10x.h>
//#include "stm32f10x.h"
#include "init_ws.h"

extern uint8_t WS2812_IO_High;
extern uint8_t WS2812_IO_Low;

//extern uint8_t WS2812_TC;
extern uint8_t TIM4_overflows;

//extern uint32_t timer_ms;

//здесь была декларация массива uint8_t WS2812_IO_framedata[ WS2812_IO_framedata_size ];
// но он стал глобальным и ушел в main
extern uint16_t Timer4_counter; // 800kHz период вывода битов, можно менять в процессеисполнения программы
extern uint8_t LED_control_type;  //тип управления светодиодов, 1 = 3х проводные, без часов

uint32_t WS2812_IO_framedata_Start_address; //стартовый адрес буфера светодиодов
extern uint16_t WS2812_Frame_Length; //количество пикселей в кадре
extern uint32_t WS2812_Frame_Byte_Length; //количество байт в кадре (WS2812_Frame_Length*24)
extern uint16_t WS2812_Frame_Count; //количество кадров в анимации
extern uint16_t WS2812_Frame_Total_Count; // всего кадров (определяем исходя из размеров массивов - сколько кадров вообще может поместиться при такой длине)
extern uint32_t WS2812_Frame_Start_Pointer; // указатель на первый проигрываемый кадр (адрес старта массива)
extern uint32_t WS2812_Current_Frame_Start; // указатель на текущий проигрываемый кадр
extern uint16_t WS2812_Frame_Period; //период в мкс, по сути то же что Servo_Period, только значение должно храниться независимо

extern volatile uint8_t DebugInfoOutFlag; //включает и отключает вывод служебной информации на второй USART (DBGU)
extern uint8_t DS_LED_Brightness; //яркость светодиодов



void WS2812_GPIO_init( void )
{
  //GPIO_InitTypeDef GPIO_InitStructure;
  // GPIOA Periph clock enable
  //RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );
	RCC->APB2ENR |= RCC_APB2Periph_GPIOB;

  //RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
  // GPIOB pins WS2812 data outputs (PB8...PB15)
  //GPIO_InitStructure.GPIO_Pin = 0xFF00;
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  //GPIO_Init( GPIOB, &GPIO_InitStructure );
  GPIOB->CRH = 0x11111111;

//GPIO_Write(GPIOB, 0);
  GPIOB->BRR = 0xFF00;//GPIO_ResetBits(GPIOB, 0xFF00);


  //GPIO_InitStructure.GPIO_Pin = 0x0003;
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  //GPIO_Init( GPIOA, &GPIO_InitStructure );

}

void WS2812_Timer_init( void )
{
  //TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  //TIM_OCInitTypeDef TIM_OCInitStructure;
 // NVIC_InitTypeDef NVIC_InitStructure;

  uint16_t PrescalerValue;

  // TIM4 Periph clock enable
  //RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );
  RCC->APB1ENR |= RCC_APB1Periph_TIM4;

  PrescalerValue = (uint16_t) ( SystemCoreClock / 24000000 ) - 1;
  /* Time base configuration */
  //TIM_TimeBaseStructure.TIM_Period = 29; // 800kHz период вывода битов, можно менять в процессеисполнения программы
	TIM4->ARR = 29;

  //TIM_TimeBaseStructure.TIM_Period = Timer4_counter; // 800kHz период вывода битов, можно менять в процессеисполнения программы

  //TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM4->PSC = PrescalerValue;

  //TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  //TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM4->CR1 &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD)));
	TIM4->CR1 |= TIM_CounterMode_Up|0;

  //TIM_TimeBaseInit( TIM4, &TIM_TimeBaseStructure );
  TIM4->EGR = 1;
  //TIM_ARRPreloadConfig( TIM4, DISABLE );
  TIM4->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_ARPE);

  /* Timing Mode configuration: Channel 1 */
//  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; //p.380RM; Output compare 1 mode: Frozen
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //external IO pin disconnected
//  TIM_OCInitStructure.TIM_Pulse = 8; //такт на котором выполняется первое прерывание CC1 event
//  TIM_OC1Init( TIM4, &TIM_OCInitStructure );


  TIM4->CCER &= (uint16_t)(~(uint16_t)TIM_CCER_CC1E);
  /* Reset the Output Compare Mode Bits */
  TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_OC1M));
  TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_CC1S));
  /* Select the Output Compare Mode */
  TIM4->CCMR1 |= TIM_OCMode_Timing;
  /* Reset the Output Polarity level */
  TIM4->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC1P));
  /* Set the Output State */
  TIM4->CCER |= TIM_OutputState_Disable;   //external IO pin disconnected
   /* Set the Capture Compare Register value */
  TIM4->CCR1 = 8;//TIM_OCInitStruct->TIM_Pulse; //такт на котором выполняется первое прерывание CC1 event


  //TIM_OC1PreloadConfig( TIM4, TIM_OCPreload_Disable );
  TIM4->CCMR1 &= (uint16_t)~((uint16_t)TIM_CCMR1_OC1PE);
  TIM4->CCMR1 |= TIM_OCPreload_Disable;


  /* Timing Mode configuration: Channel 2 */
//  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //p.380RM, мануал по кортексам; Output compare 1 mode:
  	  	  //PWM mode 1 - In upcounting, channel 2 is active as long as TIMx_CNT<TIMx_CCR1 else inactive.
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//external IO pin disconnected
//  TIM_OCInitStructure.TIM_Pulse = 17; //такт на котором выполняется первое прерывание CC2 event
//  TIM_OC2Init( TIM4, &TIM_OCInitStructure );

  /* Disable the Channel 2: Reset the CC2E Bit */
 TIM4->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC2E));
 /* Reset the Output Compare mode and Capture/Compare selection Bits */
 TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_OC2M));
 TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_CC2S));
 /* Select the Output Compare Mode */
 TIM4->CCMR1 |= (uint16_t)(TIM_OCMode_PWM1 << 8);
 /* Reset the Output Polarity level */
 TIM4->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC2P));
 /* Set the Output State */
 TIM4->CCER |= (uint16_t)(TIM_OutputState_Enable << 4);
 /* Set the Capture Compare Register value */
 TIM4->CCR2 = 17; //такт на котором выполняется первое прерывание CC2 event TIM_OCInitStruct->TIM_Pulse;





  //TIM_OC2PreloadConfig( TIM4, TIM_OCPreload_Disable );
  TIM4->CCMR1 &= (uint16_t)~((uint16_t)TIM_CCMR1_OC2PE);
  TIM4->CCMR1 |= (uint16_t)(TIM_OCPreload_Disable << 8);

  /* configure TIM4 interrupt */
 /* NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );*/
  //NVIC_Enable(TIM4_IRQn,0,2);
  NVIC->IP[TIM4_IRQn] = 0x20; //низший приоритет из возможных 0xf0, высший 0x10
  NVIC->ISER[(uint32_t)(TIM4_IRQn) >> 0x05] = (uint32_t)0x01 << (TIM4_IRQn & (uint8_t)0x1F);





}

void WS2812_Timer_reinit( uint16_t Tinit_tc, uint16_t OC1_tick,uint16_t OC2_tick)
{

	  //TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	  //TIM_OCInitTypeDef TIM_OCInitStructure;
//	  NVIC_InitTypeDef NVIC_InitStructure;

	  uint16_t PrescalerValue;
		//ws2812
	    //Tinit_tc=29;
		//OC1_tick=8;
		//OC2_tick=17;
		//sk6812
	    //Tinit_tc=29;
		//OC1_tick=6;
		//OC2_tick=12;
		//ws2812c
	    //Tinit_tc=23...29; = 23
		//OC1_tick=6...8; = 6
		//OC2_tick=14...19; = 15
		Timer4_counter=Tinit_tc;// 800kHz период вывода битов, можно менять в процессеисполнения программы

	  // TIM4 Periph clock enable
	  //RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );
	  RCC->APB1ENR |= RCC_APB1Periph_TIM4;

	  PrescalerValue = (uint16_t) ( SystemCoreClock / 24000000 ) - 1;
	  /* Time base configuration */
	  //TIM_TimeBaseStructure.TIM_Period = Tinit_tc; // 800kHz период вывода битов, можно менять в процессеисполнения программы
	  //TIM_TimeBaseStructure.TIM_Period = Timer4_counter; // 800kHz период вывода битов, можно менять в процессеисполнения программы
		TIM4->ARR = Tinit_tc;

	  //TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
		TIM4->PSC = PrescalerValue;

	  //TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	  //TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM4->CR1 &= (uint16_t)(~((uint16_t)(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD)));
		TIM4->CR1 |= TIM_CounterMode_Up|0;
	  //TIM_TimeBaseInit( TIM4, &TIM_TimeBaseStructure );
	  TIM4->EGR = 1;
	  //TIM_ARRPreloadConfig( TIM4, DISABLE );
	  TIM4->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_ARPE);

	  /* Timing Mode configuration: Channel 1 */
//	  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; //p.380RM; Output compare 1 mode: Frozen
//	  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //external IO pin disconnected
//	  TIM_OCInitStructure.TIM_Pulse = OC1_tick; //такт на котором выполняется первое прерывание CC1 event
//	  TIM_OC1Init( TIM4, &TIM_OCInitStructure );
	  TIM4->CCER &= (uint16_t)(~(uint16_t)TIM_CCER_CC1E);
	  /* Reset the Output Compare Mode Bits */
	  TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_OC1M));
	  TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_CC1S));
	  /* Select the Output Compare Mode */
	  TIM4->CCMR1 |= TIM_OCMode_Timing;
	  /* Reset the Output Polarity level */
	  TIM4->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC1P));
	  /* Set the Output State */
	  TIM4->CCER |= TIM_OutputState_Disable;   //external IO pin disconnected
	   /* Set the Capture Compare Register value */
	  TIM4->CCR1 = OC1_tick;//TIM_OCInitStruct->TIM_Pulse; //такт на котором выполняется первое прерывание CC1 event


	  //TIM_OC1PreloadConfig( TIM4, TIM_OCPreload_Disable );
	  TIM4->CCMR1 &= (uint16_t)~((uint16_t)TIM_CCMR1_OC1PE);
	  TIM4->CCMR1 |= TIM_OCPreload_Disable;

	  /* Timing Mode configuration: Channel 2 */
//	  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //p.380RM, мануал по кортексам; Output compare 1 mode:
	  	  	  //PWM mode 1 - In upcounting, channel 2 is active as long as TIMx_CNT<TIMx_CCR1 else inactive.
//	  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//external IO pin disconnected
//	  TIM_OCInitStructure.TIM_Pulse = OC2_tick; //такт на котором выполняется первое прерывание CC2 event
//	  TIM_OC2Init( TIM4, &TIM_OCInitStructure );


	  /* Disable the Channel 2: Reset the CC2E Bit */
	 TIM4->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC2E));
	 /* Reset the Output Compare mode and Capture/Compare selection Bits */
	 TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_OC2M));
	 TIM4->CCMR1 &= (uint16_t)(~((uint16_t)TIM_CCMR1_CC2S));
	 /* Select the Output Compare Mode */
	 TIM4->CCMR1 |= (uint16_t)(TIM_OCMode_PWM1 << 8);
	 /* Reset the Output Polarity level */
	 TIM4->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC2P));
	 /* Set the Output State */
	 TIM4->CCER |= (uint16_t)(TIM_OutputState_Enable << 4);
	 /* Set the Capture Compare Register value */
	 TIM4->CCR2 = OC2_tick; //такт на котором выполняется первое прерывание CC2 event TIM_OCInitStruct->TIM_Pulse;


	  //TIM_OC2PreloadConfig( TIM4, TIM_OCPreload_Disable );
	  TIM4->CCMR1 &= (uint16_t)~((uint16_t)TIM_CCMR1_OC2PE);
	  TIM4->CCMR1 |= (uint16_t)(TIM_OCPreload_Disable << 8);

	  /* configure TIM4 interrupt */
	 /* NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init( &NVIC_InitStructure );*/
	  //NVIC_Enable(TIM4_IRQn,0,2);
	  NVIC->IP[TIM4_IRQn] = 0x20; //низший приоритет из возможных 0xf0, высший 0x10
	  NVIC->ISER[(uint32_t)(TIM4_IRQn) >> 0x05] = (uint32_t)0x01 << (TIM4_IRQn & (uint8_t)0x1F);

}



void WS2812_DMA_init( uint8_t WS2812_IO_framedata_[] )
{
//  DMA_InitTypeDef DMA_InitStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;

  //RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
  RCC->AHBENR |= RCC_AHBPeriph_DMA1;

  // TIM4 Update event
  /* DMA1 Channel7 configuration ----------------------------------------------*/
  // вывод всего вывода в 1
  //DMA_DeInit( DMA1_Channel7 );
  DMA1_Channel7->CCR &= (uint16_t)(~DMA_CCR1_EN);
  DMA1_Channel7->CCR  = 0;
  DMA1_Channel7->CNDTR = 0;
  DMA1_Channel7->CPAR  = 0;
  DMA1_Channel7->CMAR = 0;
  DMA1->IFCR |= DMA1_Channel7_IT_Mask;
  /*DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &GPIOB->ODR + 1 ;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &WS2812_IO_High;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init( DMA1_Channel7, &DMA_InitStructure );*/
  DMA1_Channel7->CCR &= CCR_CLEAR_Mask;
  DMA1_Channel7->CCR |= (uint32_t)0x00002010;/*
		  DMA_DIR_PeripheralDST | DMA_Mode_Normal |
		  DMA_PeripheralInc_Disable | DMA_MemoryInc_Disable |
		  DMA_PeripheralDataSize_Byte | DMA_MemoryDataSize_Byte |
		  DMA_Priority_High | DMA_M2M_Disable;*/
  DMA1_Channel7->CNDTR = 0;
  DMA1_Channel7->CPAR = (uint32_t) &GPIOB->ODR + 1;
  DMA1_Channel7->CMAR = (uint32_t) &WS2812_IO_High;

  // TIM4 CC1 event
  /* DMA1 Channel1 configuration ----------------------------------------------*/
  //значения нужных битов в high
//  DMA_DeInit( DMA1_Channel1 );
  DMA1_Channel1->CCR &= (uint16_t)(~DMA_CCR1_EN);
  DMA1_Channel1->CCR  = 0;
  DMA1_Channel1->CNDTR = 0;
  DMA1_Channel1->CPAR  = 0;
  DMA1_Channel1->CMAR = 0;
  DMA1->IFCR |= DMA1_Channel1_IT_Mask;

  WS2812_IO_framedata_Start_address = (uint32_t) WS2812_IO_framedata_;// - записываем стартовую позицию
/*  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &GPIOB->ODR + 1;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) WS2812_IO_framedata_;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init( DMA1_Channel1, &DMA_InitStructure );*/
	DMA1_Channel1->CCR &= CCR_CLEAR_Mask;
    DMA1_Channel1->CCR |= 0x00002090 ;
    		/*DMA_DIR_PeripheralDST | DMA_Mode_Normal |
    		DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
    		 DMA_PeripheralDataSize_Byte | DMA_MemoryDataSize_Byte |
    		 DMA_Priority_High | DMA_M2M_Disable;*/
    DMA1_Channel1->CNDTR = 0;
    DMA1_Channel1->CPAR = (uint32_t) &GPIOB->ODR + 1;
    DMA1_Channel1->CMAR = (uint32_t) WS2812_IO_framedata_;


  // TIM4 CC2 event
  /* DMA1 Channel4 configuration ----------------------------------------------*/
  //вывод всего вывода на 0
  //DMA_DeInit( DMA1_Channel4 );
  DMA1_Channel4->CCR &= (uint16_t)(~DMA_CCR1_EN);
  DMA1_Channel4->CCR  = 0;
  DMA1_Channel4->CNDTR = 0;
  DMA1_Channel4->CPAR  = 0;
  DMA1_Channel4->CMAR = 0;
  DMA1->IFCR |= DMA1_Channel4_IT_Mask;

 /* DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &GPIOB->ODR +1;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &WS2812_IO_Low;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init( DMA1_Channel4, &DMA_InitStructure );*/
    DMA1_Channel4->CCR &= CCR_CLEAR_Mask;
    DMA1_Channel4->CCR |= 0x00002010 ;
    		/*DMA_DIR_PeripheralDST | DMA_Mode_Normal |
    		DMA_PeripheralInc_Disable | DMA_MemoryInc_Disable |
    		DMA_PeripheralDataSize_Byte | DMA_MemoryDataSize_Byte |
    		DMA_Priority_High | DMA_M2M_Disable;*/
    DMA1_Channel4->CNDTR = 0;
    DMA1_Channel4->CPAR = (uint32_t) &GPIOB->ODR +1;
    DMA1_Channel4->CMAR = (uint32_t) &WS2812_IO_Low;





  /* configure DMA1 Channel4 interrupt */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  SCB->AIRCR = AIRCR_VECTKEY_MASK | NVIC_PriorityGroup_0;

  /*NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );*/
  //NVIC_Enable(DMA1_Channel4_IRQn,0,0);
  NVIC->IP[DMA1_Channel4_IRQn] = 0x00; //низший приоритет из возможных 0xf0, высший 0x10
  NVIC->ISER[(uint32_t)(DMA1_Channel4_IRQn) >> 0x05] = (uint32_t)0x01 << (DMA1_Channel4_IRQn & (uint8_t)0x1F);

  /* enable DMA1 Channel7 transfer complete interrupt */
  //DMA_ITConfig( DMA1_Channel4, DMA_IT_TC, ENABLE );
  DMA1_Channel4->CCR |= DMA_IT_TC;
}
//==============================================================================================

/* Transmit the frambuffer with buffersize number of bytes to the LEDs
 * buffersize = (#LEDs / 16) * 24 */
/* до вызова обязательно настроить DMA и задать буфер через WS2812_DMA_init */

extern void WS2812_sendbuf( uint32_t buffersize )
{
  // transmission complete flag, indicate that transmission is taking place
  //WS2812_TC = 0;

  // clear all relevant DMA flags
  //DMA_ClearFlag( DMA1_FLAG_TC7 | DMA1_FLAG_HT7 | DMA1_FLAG_GL7 | DMA1_FLAG_TE7 );
  //DMA_ClearFlag( DMA1_FLAG_TC1 | DMA1_FLAG_HT1 | DMA1_FLAG_GL1 | DMA1_FLAG_TE1 );
  //DMA_ClearFlag( DMA1_FLAG_HT4 | DMA1_FLAG_GL4 | DMA1_FLAG_TE4 );
  DMA1->IFCR = 0x0F00D00F;//DMA1_FLAG_TC7 | DMA1_FLAG_HT7 | DMA1_FLAG_GL7 | DMA1_FLAG_TE7 | DMA1_FLAG_TC1 | DMA1_FLAG_HT1 | DMA1_FLAG_GL1 | DMA1_FLAG_TE1 |DMA1_FLAG_HT4 | DMA1_FLAG_GL4 | DMA1_FLAG_TE4;

  // configure the number of bytes to be transferred by the DMA controller
//  DMA_SetCurrDataCounter( DMA1_Channel7, buffersize );
  DMA1_Channel7->CNDTR = buffersize;
//  DMA_SetCurrDataCounter( DMA1_Channel1, buffersize );
  DMA1_Channel1->CNDTR = buffersize;
//  DMA_SetCurrDataCounter( DMA1_Channel4, buffersize );
  DMA1_Channel4->CNDTR = buffersize;

  // clear all TIM4 flags
  TIM4->SR = 0;

  // enable the corresponding DMA channels
  //DMA_Cmd( DMA1_Channel7, ENABLE );
  DMA1_Channel7->CCR |= DMA_CCR1_EN;
  //DMA_Cmd( DMA1_Channel1, ENABLE );
  DMA1_Channel1->CCR |= DMA_CCR1_EN;
  //DMA_Cmd( DMA1_Channel4, ENABLE );
  DMA1_Channel4->CCR |= DMA_CCR1_EN;

  // IMPORTANT: enable the TIM4 DMA requests AFTER enabling the DMA channels!
//  TIM_DMACmd( TIM4, TIM_DMA_CC1, ENABLE );
  TIM4->DIER |= TIM_DMA_CC1;
//  TIM_DMACmd( TIM4, TIM_DMA_CC2, ENABLE );
  TIM4->DIER |= TIM_DMA_CC2;
//  TIM_DMACmd( TIM4, TIM_DMA_Update, ENABLE );
  TIM4->DIER |= TIM_DMA_Update;


  // preload counter with 29 so TIM4 generates UEV directly to start DMA transfer
  //установить таймер на конец, чтобы сработало прерывание, чтобы когда насчитает 29, начинает считать с 0 и даёт прерывание
  //TIM_SetCounter( TIM4, 29 );
  //то же самое, но с переменной вместо константы
  //TIM_SetCounter( TIM4, Timer4_counter );
  TIM4->CNT = Timer4_counter;
  /*
  	  // раскрытие процедуры SetCounter (то же самое но без вызова процедуры)
  	  // Check the parameters
  	  assert_param(IS_TIM_ALL_PERIPH(TIM4));
  	  // Set the Counter Register value
  	  TIM4->CNT = Timer4_counter;
  /**/


  // start TIM4
  //TIM_Cmd( TIM4, ENABLE );
  TIM4->CR1 |= TIM_CR1_CEN;
}





//установка сдвига буфера чтения светодиодов
//применять только если WS2812_TC == 1
void WS2812_SetStartPixelInBuffer( uint32_t buffersize ){
	 if (buffersize<WS2812_IO_FRAMEDATA_SIZE){
		 //buffersize *=24;
		 //DMA_SetCurrDataStart(DMA1_Channel1,(WS2812_IO_framedata_Start_address + buffersize));
		 DMA1_Channel1->CMAR = (WS2812_IO_framedata_Start_address + buffersize);
	 }

}
//установка сдвига буфера чтения светодиодов на начало
//применять только если WS2812_TC == 1
void WS2812_ResetStartPixelInBuffer( void ){
	//DMA_SetCurrDataStart(DMA1_Channel1,WS2812_IO_framedata_Start_address);
	DMA1_Channel1->CMAR = WS2812_IO_framedata_Start_address;

}

/* DMA1 Channel7 Interrupt Handler gets executed once the complete framebuffer has been transmitted to the LEDs */
void DMA1_Channel4_IRQHandler( void )
{
	//__disable_irq ();
	//TIM_Cmd( TIM4, DISABLE );
	//__enable_irq ();
	CoEnterISR(); // Enter ISR
	//CoSchedLock();
	//TIM_Cmd( TIM4, DISABLE );
	//printf("DMA start\r\n");
		//GPIO_Write(GPIOB, 0);
	GPIOB->BRR = 0xFF00;//GPIO_ResetBits(GPIOB,0xFF00); //сброс в ноль только нужных битов, чтобы кнопки не срабатывали

	// clear DMA4 transfer complete interrupt flag
	//DMA_ClearITPendingBit( DMA1_IT_TC4 );
	DMA1->IFCR =  DMA1_IT_TC4;
	// enable TIM4 Update interrupt to append 50us dead period
	TIM_ITConfig( TIM4, TIM_IT_Update, ENABLE );
	// disable the DMA channels
	//DMA_Cmd( DMA1_Channel7, DISABLE );
	DMA1_Channel7->CCR &= (uint16_t)(~DMA_CCR1_EN);
	//DMA_Cmd( DMA1_Channel1, DISABLE );
	DMA1_Channel1->CCR &= (uint16_t)(~DMA_CCR1_EN);
	//DMA_Cmd( DMA1_Channel4, DISABLE );
	DMA1_Channel4->CCR &= (uint16_t)(~DMA_CCR1_EN);

	// IMPORTANT: disable the DMA requests, too!
	//TIM_DMACmd( TIM4, TIM_DMA_CC1, DISABLE );
	TIM4->DIER &= (uint16_t)~TIM_DMA_CC1;
	//TIM_DMACmd( TIM4, TIM_DMA_CC2, DISABLE );
	TIM4->DIER &= (uint16_t)~TIM_DMA_CC2;
	//TIM_DMACmd( TIM4, TIM_DMA_Update, DISABLE );
	TIM4->DIER &= (uint16_t)~TIM_DMA_Update;

	//printf("!!/r/n");
	//CoSchedUnlock();
	CoExitISR(); // Exit ISR

}

/* TIM4 Interrupt Handler gets executed on every TIM4 Update if enabled */
//TIM4 - отработка паузы в конце передачи точек WS
void TIM4_IRQHandler( void )
{
	CoEnterISR(); // Enter ISR, запрет на выполнение других функций и процедур параллельно с текущей
  // Clear TIM4 Interrupt Flag
//  TIM_ClearITPendingBit( TIM4, TIM_IT_Update );
  TIM4->SR = (uint16_t)~TIM_IT_Update;

  /* check if certain number of overflows has occured yet
   * this ISR is used to guarantee a 50us dead time on the data lines
   * before another frame is transmitted */
  if ( TIM4_overflows < (uint8_t) WS2812_DEADPERIOD )
  {
    // count the number of occured overflows
    TIM4_overflows++;
  }
  else
  {
    // clear the number of overflows
    TIM4_overflows = 0;
    // stop TIM4 now because dead period has been reached
    //TIM_Cmd( TIM4, DISABLE );
    TIM4->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
    /* disable the TIM4 Update interrupt again
     * so it doesn't occur while transmitting data */
    TIM_ITConfig( TIM4, TIM_IT_Update, DISABLE );
    // finally indicate that the data frame has been transmitted
    //вставить сюда вызов процедуры обновления номера фрейма для мультикадра
    //WS2812_TC = 1;
    if ((LED_control_type&127) == 4){Set_Next_Frame_for_Fast_Animation ();}
  }
	CoExitISR(); // Exit ISR

}

/* This function sets the color of a single pixel in the framebuffer
 *
 * Arguments:
 * row = the channel number/LED strip the pixel is in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixel should display
 */
void WS2812_framedata_setPixel(uint8_t WS2812_IO_framedata_[], uint8_t row, uint16_t column, uint8_t red,
  uint8_t green, uint8_t blue )
{
	uint8_t i;
  uint8_t row_bitmask;
  row_bitmask = (uint8_t) 0x01 << row;
  uint8_t c;

  for ( i = 0; i < 8; i++ )
  {
    // clear the data for pixel
    c= (column*24 ) + i;
	WS2812_IO_framedata_[c] &= ~row_bitmask;
    WS2812_IO_framedata_[c+8] &= ~row_bitmask;
    WS2812_IO_framedata_[c+16] &= ~row_bitmask;
    // write new data for pixel
    WS2812_IO_framedata_[c] |= ( ((green << i) & 0x80) >> 7 ) << row ;
    WS2812_IO_framedata_[c+8] |= ( ((red << i) & 0x80) >> 7 ) << row ;
    WS2812_IO_framedata_[c+16] |= ( ((blue << i) & 0x80) >> 7 ) << row;
  }
}

void WS2812_framedata_setPixel_RGB (uint8_t WS2812_IO_framedata_[], uint8_t row_bitmask,
		uint16_t column, uint32_t point_rgb, uint8_t Repeats)//попробовать point_rgb заменить на указатель
{
	uint8_t i;
	//uint8_t tmp;
	//uint8_t row_bitmask; // для обнуления нужных битов в буфере WS
	uint16_t c;
	//uint32_t pix = point_rgb; //для хранения постепенно сдвигающегося вправо числа
	//uint8_t pix_one_bit = (uint8_t) pix & 0x1; // обнуляем все биты кроме последнего с обрезанием байта
	//почему не перевести point_rgb сдвигами в red, green и blue и не скопировать WS2812_framedata_setPixel ?
	 uint8_t blue = (uint8_t) point_rgb;
	 uint8_t green = (uint8_t) (point_rgb >> 8);
	 uint8_t red = (uint8_t) (point_rgb >> 16);

	 blue = (blue*DS_LED_Brightness)>>BRIGHTNESS_SHIFT;
	 green =  (green*DS_LED_Brightness)>>BRIGHTNESS_SHIFT;
	 red = (red*DS_LED_Brightness)>>BRIGHTNESS_SHIFT;

	// тут можно регулировать яркость blue = (blue*lightness) >> 2  или blue = blue*((lightness&0x4)>>2) + (blue>>1)*((lightness&0x2)>>1) + (blue>>2)*(lightness&0x1)
	//проврка кода - задать цвет 10000000 10000000 10000000 и lightness 4, 3,2,1
	 //printf("Set pix RGB 0x%06X=0x%02X;0x%02X;0x%02X.\r\n",point_rgb,red,green,blue);

	//row_bitmask = (uint8_t) 0x01 << row;
	/*  for ( i = 0; i < 8; i++ )
  	  {
    	// clear the data for pixel
    	c= (column*24 ) + i;
		WS2812_IO_framedata_[c] &= ~row_bitmask;
    	WS2812_IO_framedata_[c+8] &= ~row_bitmask;
    	WS2812_IO_framedata_[c+16] &= ~row_bitmask;
    	// write new data for pixel
    	WS2812_IO_framedata_[c] |= ( ((green << i) & 0x80) >> 7 ) * row_bitmask;//<< row ;
    	WS2812_IO_framedata_[c+8] |= ( ((red << i) & 0x80) >> 7 ) * row_bitmask;//<< row ;
    	WS2812_IO_framedata_[c+16] |= ( ((blue << i) & 0x80) >> 7 ) * row_bitmask;//<< row;
  	  }
	 /**/
	 	 c= column*24;
	 uint8_t row_bitmask_high = (~row_bitmask) & WS2812_IO_High;
	 row_bitmask &= WS2812_IO_High;

WS2812_framedata_Fill_Color_Array_Label:
	  for ( i = 0; i < 8; i++ )
  	  {
    	// clear the data for pixel
    	//c= (column*24 ) + i;
		WS2812_IO_framedata_[c] &= row_bitmask_high;
    	WS2812_IO_framedata_[c+8] &= row_bitmask_high;
    	WS2812_IO_framedata_[c+16] &= row_bitmask_high;
    	// write new data for pixel
    	WS2812_IO_framedata_[c] |= ((((green << i) & 0x80) >> 7 ) * row_bitmask) | WS2812_IO_Low;//<< row ;
    	WS2812_IO_framedata_[c+8] |= ((((red << i) & 0x80) >> 7 ) * row_bitmask) | WS2812_IO_Low;//<< row ;
    	WS2812_IO_framedata_[c+16] |= ((((blue << i) & 0x80) >> 7 ) * row_bitmask) | WS2812_IO_Low;//<< row;
    	c++;
  	  }
	 /**/

	  if (Repeats == 0){return;}
	  Repeats -=1;
	  c += 16;
	  goto WS2812_framedata_Fill_Color_Array_Label;



} //end of WS2812_framedata_setPixel_RGB

void WS2812_framedata_MultiplyPixel_RGB (uint8_t WS2812_IO_framedata_[], uint8_t rowSRC, uint8_t rowDST,
		uint16_t columnSRC, uint16_t columnDST, uint8_t Repeats, uint8_t Rep_Set)//попробовать point_rgb заменить на указатель
{
	uint8_t i;
	uint8_t row_in=0;
	uint8_t row_in_shift;
	uint16_t blue=0;
	uint16_t green=0;
	uint16_t red=0;
	uint8_t blueDST=0;
	uint8_t greenDST=0;
	uint8_t redDST=0;

	for (i=1;i<8;i++){
		if (rowSRC & (1<<i)){
			row_in = i;
			goto MultiplyPixel_RGB_Row_Chosen;
		}
	}
MultiplyPixel_RGB_Row_Chosen:
/*
if (rowSRC & 2)  {
	row_in = 1;
} else if (rowSRC & 4)  {
	row_in = 2;
} else if (rowSRC & 8)  {
	row_in = 3;
} else if (rowSRC & 16)  {
	row_in = 4;
} else if (rowSRC & 32)  {
	row_in = 5;
} else if (rowSRC & 64)  {
	row_in = 6;
} else if (rowSRC & 128)  {
	row_in = 7;
}
*/
	rowSRC = (1<<row_in); //оставляем одну строчку
	row_in_shift = 15 - row_in;//8+7-row_in
	columnSRC = columnSRC*24;
	columnDST = columnDST*24;

WS2812_framedata_MultiplyPixel_Action_Label:
	for (i = 0; i < 8; i++){
		green |=(((WS2812_IO_framedata_[columnSRC] >> row_in) & 1)<<i);
		red   |=(((WS2812_IO_framedata_[columnSRC+8] >> row_in) & 1)<<i);
		blue  |=(((WS2812_IO_framedata_[columnSRC+16] >> row_in) & 1)<<i);
		greenDST |=(((WS2812_IO_framedata_[columnDST] >> rowDST) & 1)<<i);
		redDST   |=(((WS2812_IO_framedata_[columnDST+8] >> rowDST) & 1)<<i);
		blueDST  |=(((WS2812_IO_framedata_[columnDST+16] >> rowDST) & 1)<<i);
		columnSRC++;
		columnDST++;
	}
	//columnSRC -=8;
	columnDST -=8;
	green = green*(greenDST+1);
	red = red*(redDST+1);
	blue = blue*(blueDST+1);
	rowSRC = (~rowSRC) & WS2812_IO_High;
	for ( i = 0; i < 8; i++ ) {
		WS2812_IO_framedata_[columnSRC-8] &= rowSRC;
	  	WS2812_IO_framedata_[columnSRC] &= rowSRC;
	  	WS2812_IO_framedata_[columnSRC+8] &= rowSRC;
	  	// write new data for pixel
	  	WS2812_IO_framedata_[columnSRC-8] |= ((((green << i) & 0x8000) >> row_in_shift )& WS2812_IO_High) | WS2812_IO_Low;//<< row ;
	  	WS2812_IO_framedata_[columnSRC] |= ((((red << i) & 0x8000) >> row_in_shift )& WS2812_IO_High) | WS2812_IO_Low;//<< row ;
	  	WS2812_IO_framedata_[columnSRC+8] |= ((((blue << i) & 0x8000) >> row_in_shift )& WS2812_IO_High) | WS2812_IO_Low;//<< row;
	  	columnSRC++;
	}


	if (Repeats == 0){return;}
	Repeats -=1;
	columnSRC +=8;
	  //columnDST не меняется, т.е. надо её менять принудительно
	  //Rep_Set - 4 - обычный повтор, 8 - с инкрементом (ничего не делаем), 2 - с декрементом
	if (Rep_Set & 8){
		columnDST = (columnDST > (WS2812_IO_FRAMEDATA_PIXELS*24 - 24)) ? (WS2812_IO_FRAMEDATA_PIXELS*24) : columnDST + 24;
	} else if ((Rep_Set & 2) && (columnDST > 23)) {
		columnDST -=24; //сдвиг назад
	}
	goto WS2812_framedata_MultiplyPixel_Action_Label;

} //end of WS2812_framedata_MultiplyPixel_RGB


//WS2812_IO_framedata_[] - массив точек
//rowSRC, rowDST - текущий читаемая строка и целевая строка, rowSRC - битовая маска, rowDST - номер строки!
//columnSRC, columnDST - текущий (DS_WSpoint_counter) и целевой (DS_Param_ & 0x0FFF) пиксели
//Swapmode 3- поменять, 2 - скопировать из текущего в целевой, 1 - скопировать из целевого в текущий
//Swapmode 4+1 - текущий = текущий OR целевой
//Repeats - сколько раз повторить операцию
//Rep_Set - 4 - обычный повтор, 8 - с инкрементом, 2 - с декрементом
void WS2812_framedata_SwapPixel (uint8_t WS2812_IO_framedata_[], uint8_t rowSRC, uint8_t rowDST,
		uint16_t columnSRC, uint16_t columnDST, uint8_t Swapmode, uint8_t Repeats, uint8_t Rep_Set)
{
	uint8_t i;
	uint8_t tmp;
	uint8_t row_in=0; // номер входной строки
	uint8_t Swap1 = (Swapmode & 1);
	uint8_t Swap2 = (Swapmode & 2) >> 1;
	//rowDST = 1<<rowDST;
	uint8_t DST_mask = ~((1<<rowDST) * Swap2);
	uint8_t SRC_mask;

	//выбираем номер линии, который наименьший из перечисленных в команда S<>P<>
/*	for (i=1;i<8;i++){
		if (rowSRC & (1<<i)){
			row_in = i;
			goto framedata_SwapPixel_row_chosen_label;
		}
	}/**/
framedata_SwapPixel_row_chosen_label:

if (rowSRC & 2)  {
	row_in = 1;
} else if (rowSRC & 4)  {
	row_in = 2;
} else if (rowSRC & 8)  {
	row_in = 3;
} else if (rowSRC & 16)  {
	row_in = 4;
} else if (rowSRC & 32)  {
	row_in = 5;
} else if (rowSRC & 64)  {
	row_in = 6;
} else if (rowSRC & 128)  {
	row_in = 7;
}
/**/
	//rowSRC = 1<<row_in; //оставляем одну строчку
	if (((~WS2812_IO_High)| WS2812_IO_Low) & (1<<row_in & 1<<rowDST)) return; //если каналы работают для светодиодов, то IO_High = FF и IO_Low = 00
	SRC_mask = (Swapmode & 4) ? 0xFF : (~((1<<row_in) * Swap1));
	SRC_mask &= WS2812_IO_High;
	DST_mask &= WS2812_IO_High;
	columnSRC = columnSRC*24;
	columnDST = columnDST*24;


WS2812_framedata_SwapPixel_Action_Label:
	for ( i = 0; i < 24; i++ ) {
		//tmpSRC = (Swap2 ? (WS2812_IO_framedata_[columnSRC] & rowSRC & WS2812_IO_High):0) | WS2812_IO_Low;
		//tmpDST = (Swap1 ? (WS2812_IO_framedata_[columnDST] & rowDST & WS2812_IO_High):0) | WS2812_IO_Low;
/*
    	tmp = (WS2812_IO_framedata_[columnSRC] >> row_in) & 1; //сохраняем во временное хранилище бит цвета


    	WS2812_IO_framedata_[columnSRC] &= SRC_mask; //записываем из целевого пикселя в текущий если Swapmode = 1 или 3
    	WS2812_IO_framedata_[columnSRC] |= (((((WS2812_IO_framedata_[columnDST]>>rowDST) & 1) << row_in) * Swap1)& WS2812_IO_High) | WS2812_IO_Low;
    	WS2812_IO_framedata_[columnDST] &= DST_mask; //записываем сохранённое от текущего в целевой если Swapmode = 2 или 3
    	WS2812_IO_framedata_[columnDST] |= (((tmp<<rowDST) * Swap2)& WS2812_IO_High) | WS2812_IO_Low;
*/
/*    	tmp = WS2812_IO_framedata_[columnSRC]; //сохраняем во временное хранилище бит цвета
    	if (Swap1){
    		WS2812_IO_framedata_[columnSRC] &= SRC_mask; //записываем из целевого пикселя в текущий если Swapmode = 1 или 3
    		WS2812_IO_framedata_[columnSRC] |= (((((WS2812_IO_framedata_[columnDST]>>rowDST)&1) << row_in))& WS2812_IO_High) | WS2812_IO_Low;
    	}
    	if (Swap2) {
    		WS2812_IO_framedata_[columnDST] &= DST_mask; //записываем сохранённое от текущего в целевой если Swapmode = 2 или 3
    		WS2812_IO_framedata_[columnDST] |= (((((tmp >> row_in) & 1)<<rowDST))& WS2812_IO_High) | WS2812_IO_Low;
    	}/**/
    	tmp = WS2812_IO_framedata_[columnSRC]; //сохраняем во временное хранилище бит цвета
    		WS2812_IO_framedata_[columnSRC] &= SRC_mask; //записываем из целевого пикселя в текущий если Swapmode = 1 или 3
    		WS2812_IO_framedata_[columnSRC] |= (((((WS2812_IO_framedata_[columnDST]>>rowDST) & Swap1) << row_in)));//& WS2812_IO_High) | WS2812_IO_Low;
    		WS2812_IO_framedata_[columnDST] &= DST_mask; //записываем сохранённое от текущего в целевой если Swapmode = 2 или 3
    		WS2812_IO_framedata_[columnDST] |= (((((tmp >> row_in) & Swap2)<<rowDST)));//& WS2812_IO_High) | WS2812_IO_Low;

    	columnSRC ++;
		columnDST ++;
	}

	  if (Repeats == 0){return;}
	  Repeats -=1;
	  //columnSRC не трогаем, оно само
	  // columnDST по дефолту мы в режиме "^"
	  //Rep_Set - 4 - обычный повтор, 8 - с инкрементом (ничего не делаем), 2 - с декрементом
	  //if ((Rep_Set & (4+2)) || (Rep_Set == 0)) { //(Rep_Set & (4+2)) || (Rep_Set == 0) //Rep_Set < 8
	  columnDST = (Rep_Set & 8) ? columnDST : (columnDST-24);
	  //if (!(Rep_Set & 8)) {
	  //	columnDST -=24; //возврат на исходную
	  //}
	  columnDST = ((Rep_Set & 2) && (columnDST > 23)) ? (columnDST -24) : columnDST;
	  //if ((Rep_Set & 2) && (columnDST > 23)) {
	  //	columnDST -=24; //сдвиг назад
	  //}
	  columnDST = (columnDST > (WS2812_IO_FRAMEDATA_PIXELS*24)) ? (WS2812_IO_FRAMEDATA_PIXELS*24) : columnDST;
	  goto WS2812_framedata_SwapPixel_Action_Label;

} //end of WS2812_framedata_SwapPixel


//возвращает 1, если есть разница между цветом (цветами) в массиве и цветом, который проверяется
//таким образом при серии несовпадений можно определить, сколько цветов отличается
//если всё совпало, то возвращает 0
uint8_t WS2812_framedata_CheckPixel(uint8_t WS2812_IO_framedata_[], uint8_t row_bitmask,
		uint16_t column, uint32_t point_rgb, uint8_t Repeats)
{
	uint8_t i;
	uint16_t c;
	uint32_t pix; //для хранения постепенно сдвигающегося вправо числа
	uint8_t compar = 0;

	 uint8_t blue = (uint8_t) point_rgb;
	 uint8_t green = (uint8_t) (point_rgb >> 8);
	 uint8_t red = (uint8_t) (point_rgb >> 16);

	 blue = (blue*DS_LED_Brightness)>>BRIGHTNESS_SHIFT;
	 green =  (green*DS_LED_Brightness)>>BRIGHTNESS_SHIFT;
	 red = (red*DS_LED_Brightness)>>BRIGHTNESS_SHIFT;
	//RGB->GRB
	//point_rgb = (point_rgb&0xFF)+((point_rgb & 0xFF0000)>>8)+((point_rgb & 0x00FF00)<<8);
	 point_rgb = blue+green*65536+red*256;

	  c = column*24;
WS2812_framedata_CheckPixel_Comparison:
	  pix = 0;
	  for ( i = 0; i < 24; i++ )
  	  {
		  pix <<= 1;
		if (WS2812_IO_framedata_[c] & row_bitmask){
			pix |= 1;
		}
    	c++;
  	  }
	 if (point_rgb != pix){compar++;}//point == pixel => return 0

	 if (Repeats == 0){return compar;}
	 Repeats -=1;
	 goto WS2812_framedata_CheckPixel_Comparison;
	 return 1;
}

/* This function is a wrapper function to set all LEDs in the complete row to the specified color
 *
 * Arguments:
 * row = the channel number/LED strip to set the color of from 0 to 15
 * columns = the number of LEDs in the strip to set to the color from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display
 */
void WS2812_framedata_setRow(uint8_t WS2812_IO_framedata_[], uint8_t row, uint16_t columns, uint8_t red,
  uint8_t green, uint8_t blue )
{
  uint8_t i;
  for ( i = 0; i < columns; i++ )
  {
    WS2812_framedata_setPixel(WS2812_IO_framedata_, row, i, red, green, blue );
  }
}

/* This function is a wrapper function to set all the LEDs in the column to the specified color
 *
 * Arguments:
 * rows = the number of channels/LED strips to set the row in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixels should display
 */
void WS2812_framedata_setColumn(uint8_t WS2812_IO_framedata_[], uint8_t rows, uint16_t column, uint8_t red,
  uint8_t green, uint8_t blue )
{
  uint8_t i;
  for ( i = 0; i < rows; i++ )
  {
    WS2812_framedata_setPixel(WS2812_IO_framedata_, i, column, red, green, blue );
  }
}


// очистка буфера точек

void WS2812_clear_buffer (uint8_t WS2812_IO_framedata_[], uint16_t buffersize, uint8_t row_bitmask)
{
	uint16_t i;

	for ( i = 0; i < buffersize; i++ )
  {
    // clear the data
	WS2812_IO_framedata_[i] &= ~row_bitmask;
  }
}


void WS2812_framedata_Set_HighLow(uint8_t WS2812_IO_framedata_[], uint16_t buffersize){
	if ((LED_control_type & 3) == 3){
		uint16_t i;
		buffersize = buffersize*24;
		for ( i = 0; i <buffersize; i++ ){
			WS2812_IO_framedata_[i] &= WS2812_IO_High;
			WS2812_IO_framedata_[i] |= WS2812_IO_Low;
		}
	}
}
