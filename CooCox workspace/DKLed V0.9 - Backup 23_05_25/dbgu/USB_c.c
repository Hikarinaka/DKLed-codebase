#include "includes.h"
#include "stm32f10x_rcc.h"


uint8_t EPindex;
uint16_t BKIstr=0;
uint16_t USB_Command = 0;
//uint16_t SaveRState;
uint16_t SaveTState;
//uint16_t EP_walue;
uint16_t USB_RX_Start_pointer=0;
uint16_t USB_Recieved_bytes=0;
uint16_t USB_Bytes_to_send_left = 0;
volatile uint32_t USB_rx_timeout;

CONTROL_STATE USB_state_flag = WAIT_SETUP;
//0 - ready
//1 - data in
//2 - data out
//4 -
//8 -
//0x10 -
//0x20 -
//0x40 -
//0x80 -
//uint8_t usb_fsm_state = IDLE;
uint8_t USB_DADDR = 0;
uint8_t USB_Feature;
uint8_t USB_Configuration;
uint8_t USB_Interface;

uint8_t USB_Function_flags=0;
//1 - transmit request (IN)
//2 - receive request (OUT)
//4 - we've got something to read and decode
//8 - finished serving current stack of data for EP3 (received text, OUT)

uint16_t USB_Command;
uint16_t USB_wValue;
uint16_t USB_wLength;
//uint16_t wIstr;



uint8_t i;
uint16_t TempEP;
uint8_t Related_Endpoint, Reserved;
uint16_t USB_wIndex;
uint32_t i32;

#if(0) //MSC
//USB descriptor constants
//ДЕСКРИПТОРЫ С САЙТА http://ravenium.ru/stm32-и-usb-hid-это-просто/
/* USB Standard Device Descriptor */
const char DeviceDiscriptor[] =
{
		    0x12,         // общая длина дескриптора устройства в байтах
		    0x01, // bDescriptorType - показывает, что это за дескриптор. В данном случае - Device descriptor
		    0x00, 0x02,
		    //0x10, 0x01,                 // bcdUSB - какую версию стандарта USB поддерживает устройство. 2.0

			// класс, подкласс устройства и протокол, по стандарту USB. У нас нули, означает каждый интерфейс сам за себя
		    0x00,                       //bDeviceClass //mass storage https://www.usb.org/defined-class-codes
		    0x00,                       //bDeviceSubClass
		    0x00,                       //bDeviceProtocol

		    USB_EP0_MAX_PACKET_SIZE,                       //bMaxPacketSize - максимальный размер пакетов для Endpoint 0 (при конфигурировании)

			// те самые пресловутые VID и PID,  по которым и определяется, что же это за устройство.
		    0x83, 0x04,                 //idVendor (0x0483)
		    0x20, 0x57,                 //idProduct (0x5711)
		    //0x0483  0x5720   STMicroelectronics  Mass Storage Device
		    //https://www.the-sz.com/products/usbid/index.php?v=0x0483

		    0x07, 0x00,                 // bcdDevice rel. DEVICE_VER_H.DEVICE_VER_L  номер релиза устройства

			// дальше идут индексы строк, описывающих производителя, устройство и серийный номер.
			// Отображаются в свойствах устройства в диспетчере устройств
			// А по серийному номеру подключенные устройства с одинаковым VID/PID различаются системой.
		    1,                          //Index of string descriptor describing manufacturer
		    2,                          //Index of string descriptor describing product
		    3,                          //Index of string descriptor describing the device serial number
		    0x01                        // bNumConfigurations - количество возможных конфигураций. У нас одна.
}; /* MSC_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const char ConfigDescriptor[] =
{
	0x09,	// bLength: длина дескриптора конфигурации
	0x02,	// bDescriptorType: тип дескриптора - конфигурация
	32,		// wTotalLength: общий размер всего дерева под данной конфигурацией в байтах

	0x00,
	0x01,	// bNumInterfaces: в конфигурации всего один интерфейс
	0x01,	// bConfigurationValue: индекс данной конфигурации
	0x00,	// iConfiguration: индекс строки, которая описывает эту конфигурацию
	0xC0,	//0xE0,         // bmAttributes: признак того, что устройство будет питаться от шины USB
	0x32,	// MaxPower mA*0.5 100 mA=0x32, 510 = 0xff: и ему хватит 100 мА

	//************* Дескриптор интерфейса ****************/
	0x09,	// bLength: размер дескриптора интерфейса																									//10
	0x04,	// bDescriptorType: тип дескриптора - интерфейс
	0x00,	// bInterfaceNumber: порядковый номер интерфейса - 0
	0x00,	// bAlternateSetting: признак альтернативного интерфейса, у нас не используется
	0x02,	// bNumEndpoints - количество эндпоинтов.

	0x08,	// bInterfaceClass: класс интерфеса - Mass storage
	0x06,	// bInterfaceSubClass : подкласс интерфейса. \
		//0x01 - Reduced Block Commands (RBC)
		//0x02 - MMC (ATAPI)
		//0x05 - SFF-8070i
		//0x06 - SCSI transparent command set
	0x50,	// nInterfaceProtocol : протокол интерфейса 0x50 - Bulk-Only (BOT) Transport

	4,	//0,	// iInterface: индекс строки, описывающей интерфейс

	//******************* дескриптор конечных точек (endpoints) ********************/
	0x07,	// bLength: длина дескриптора
	0x05,	// тип дескриптора - endpoints
	0x81,	// bEndpointAddress: адрес конечной точки и направление 1(IN)																		//30
	0x02,	// bmAttributes: тип конечной точки - bulk endpoint
	USB_EP1_MAX_PACKET_SIZE_TX, 0x00,    // wMaxPacketSize:  64 Bytes max
	0x00,	// bInterval: Polling Interval (32 ms)																								//34

	0x07,	// bLength: Endpoint Descriptor size
	0x05,	// bDescriptorType:
			//	Endpoint descriptor type
	0x02,	// bEndpointAddress:
		//	Endpoint Address (OUT)
	0x02,	// bmAttributes: bulk endpoint
	USB_EP2_MAX_PACKET_SIZE_RX,	// wMaxPacketSize:  64 Bytes max
	0x00,																																							//40
	0x00,	// bInterval: Polling Interval (32 ms)			    																										//41***************************************
}; /* MSC_ConfigDescriptor */


//const char StringDescriptor0[] = {4, 3, 9, 4};// LangID = 0x0419 Russian 0x0409 English (United States)
//const char StringDescriptor1[] = {10, 3, 'D', 0, 'K', 0, 'A', 0, 'G', 0}; //Vendor
//const char StringDescriptor2[] = {18, 3, 'D', 0, 'K', 0, 'L', 0, 'e', 0, 'd', 0, '0', 0, '.', 0, '7', 0 }; //product
//char StringDescriptor3[] = {10, 3, '0', 0, '0', 0, '0', 0, '1', 0}; //serial in unicode


#endif
#if(1)//CDC
//USB descriptor constants
/* USB Standard Device Descriptor */
const char DeviceDiscriptor[] =
  {
    0x12,   /* bLength */
    0x01,     /* bDescriptorType */
    0x00,
    0x02,   /* bcdUSB = 2.00 */
    0x02,   /* bDeviceClass: CDC */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    0x40,   /* bMaxPacketSize0 */
    0x83,
    0x04,   /* idVendor = 0x0483 */
    0x40,
    0x57,   /* idProduct = 0x7540 */
    0x00,
    0x02,   /* bcdDevice = 2.00 */
    1,              /* Index of string descriptor describing manufacturer */
    2,              /* Index of string descriptor describing product */
    3,              /* Index of string descriptor describing the device's serial number */
    0x01    /* bNumConfigurations */
  };

const char ConfigDescriptor[] =
  {
    /*Configuration Descriptor*/
    0x09,   /* bLength: Configuration Descriptor size */
    0x02,      /* bDescriptorType: Configuration */
    67,       /* wTotalLength:no of returned bytes */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered */
    0x32,   /* MaxPower 0 mA */
    /*Interface Descriptor*/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
//  };
//const char ConfigDescriptor1[] =
//  {
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */
    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,
    /*Call Management Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x01,   /* bDataInterface: 1 */
    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */
//  };
//  const char ConfigDescriptor2[] =
//    {
    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */
    /*Endpoint 2 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x82,   /* bEndpointAddress: (IN2) */
    0x03,   /* bmAttributes: Interrupt */
    8,      /* wMaxPacketSize: */
    0x00,
    0xFF,   /* bInterval: */
    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    0x04,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
//    };
//    const char ConfigDescriptor3[] =
//      {
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */
    /*Endpoint 3 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x03,   /* bEndpointAddress: (OUT3) */
    0x02,   /* bmAttributes: Bulk */
    64,             /* wMaxPacketSize: */
    0x00,
    0x00,   /* bInterval: ignore for Bulk transfer */
    /*Endpoint 1 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x81,   /* bEndpointAddress: (IN1) */
    0x02,   /* bmAttributes: Bulk */
      };
//      const char ConfigDescriptor5[] =
//       {
//    64,             /* wMaxPacketSize: */
//    0x00,
//    0x00    /* bInterval */
//  };


//const char StringDescriptor0[] = {4, 3, 9, 4};// LangID = 0x0419 Russian 0x0409 English (United States)
//const char StringDescriptor1[] = {10, 3, 'D', 0, 'K', 0, 'A', 0, 'G', 0}; //Vendor
//const char StringDescriptor2[] = {18, 3, 'D', 0, 'K', 0, 'L', 0, 'e', 0, 'd', 0, '0', 0, '.', 0, '7', 0 }; //product
//char StringDescriptor3[] = {10, 3, '0', 0, '0', 0, '0', 0, '1', 0}; //serial in unicode


#endif

unsigned char USB_Buff1[64];
unsigned char USB_COM_TX_Buff1[64];




void USB_com_Init()
{


	SystemInit();



	//GPIO_InitTypeDef  GPIO_InitStructure;

  /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
       	#define RCC_APB2Periph_GPIO_DISCONNECT 		RCC_APB2Periph_GPIOF
		#define USB_DISCONNECT_PIN					GPIO_Pin_11
		#define USB_DISCONNECT  					GPIOF
     */


  /* Enable USB_DISCONNECT GPIO clock */
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);
  RCC->APB2ENR |= RCC_APB2Periph_GPIOF;


  /* Configure USB pull-up pin */

/*  GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;//GPIO_Pin_11 = 0x08 00
  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //GPIO_Mode_Out_OD = 0x14,
  GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
/**/
  GPIOF->CRH &= 0xFFFF0FFF;
  GPIOF->CRH |= 0x00007000; //general output open drain/50MHz



  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
//  RCC->APB2ENR |= RCC_APB2Periph_GPIOF;
//  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;
  /*******************************************************************************
  * Function Name  : Set_USBClock
  * Description    : Configures USB Clock input (48MHz)
  * Input          : None.
  * Return         : None.
  *******************************************************************************/

    /* Select USBCLK source */
    //RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

    //*(__IO uint32_t *) (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (USBPRE_BitNumber * 4)) = RCC_USBCLKSource_PLLCLK_1Div5;
    *(__IO uint32_t *) (0x424200D8) = 0;

    /* Enable the USB clock */
    RCC->APB1ENR |=RCC_APB1Periph_USB;


    /*
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
*/

    NVIC->IP[USB_LP_CAN1_RX0_IRQn] = 0xA0; //низший приоритет из возможных 0xf0, высший 0x10

    //pending - будем проверять в цикле
    NVIC->ISER[(uint32_t)(USB_LP_CAN1_RX0_IRQn) >> 0x05] = (uint32_t)0x01 << (USB_LP_CAN1_RX0_IRQn & (uint8_t)0x1F);
    //disable
    //NVIC->IСER[USB_LP_CAN1_RX0_IRQn >> 0x05] = (uint32_t)0x01 << (USB_LP_CAN1_RX0_IRQn & (uint8_t)0x1F);

    USB_RESET();

}


void USB_RESET(void){

	//int buf1;
    //адреса эндпойнтов
//	uint16_t DISCRIPTOR0[4] = {	64, 			// ADDR_TX = 64 оставлено место для всех 8 дискрипторов перед буфером приема(можно оставить меньше)
//			0, 				// COUNT_TX = 0 нужно будет заполнить перед отправкой данных
//			128,			// ADDR_RX = 128 пропускаем 64 байта буфера передачи
//			33792};			// Указываем размер буфера приема 64 байта
//	TO_WRITE_PMA(DISCRIPTOR0, 0, 4);		// Заполняем дискриптор нулевой точки

//	uint16_t DISCRIPTOR0[4] = {	EP0TX_OFFSET, 			// ADDR_TX оставлено место для всех 8 дискрипторов перед буфером приема(можно оставить меньше)
//			0, 				// COUNT_TX = 0 нужно будет заполнить перед отправкой данных
//			EP0RX_OFFSET,			// ADDR_RX пропускаем 64 байта буфера передачи
//			0x8400};			// Указываем размер буфера приема 64 байта, BL_size = 1 (32 байта на блок), 2 штуки
//	TO_WRITE_PMA(DISCRIPTOR0, 0, 4);		// Заполняем дискриптор нулевой точки
//	DISCRIPTOR0[0] = 0xD8; 			// ADDR_TX оставлено место для всех 8 дискрипторов перед буфером приема(можно оставить меньше)
//	DISCRIPTOR0[1] = 0; 				// COUNT_TX = 0 нужно будет заполнить перед отправкой данных
//	DISCRIPTOR0[2] = 0x98;			// ADDR_RX пропускаем 64 байта буфера передачи
//	DISCRIPTOR0[3] = 0x8400;			// Указываем размер буфера приема 64 байта, BL_size = 1 (32 байта на блок), 2 штуки

//	TO_WRITE_PMA(DISCRIPTOR0, 8, 4);		// Заполняем дискриптор первой точки

	BTABLE = 0;	// Адрес таблицы дискрипторов внутри пакетной памяти

//	print_0X4(PMA_BUF(0),1);
//EP 0
	PMA_BUF(0) = EP0TX_OFFSET;// ADDR_TX
	PMA_BUF(1) = 0;// COUNT_TX = 0 нужно заполнять перед отправкой данных
	PMA_BUF(2) = EP0RX_OFFSET;// ADDR_RX
	PMA_BUF(3) = 0x8400;// Указываем размер буфера приема 64 байта, BL_size = 1 (32 байта на блок), 2 штуки

	EP0R = ((EP0R ^ 0x3020) & 0x3030) | 0x0200;

/*
//EP 1 MSC TX
	PMA_BUF(4) = EP1TX_OFFSET;
	PMA_BUF(5) = 0;
	PMA_BUF(6) = 0;
	PMA_BUF(7) = 0;

    ENDPOINT(1) = ((ENDPOINT(1) ^ 0x0030) & 0x3030);
//EP 2 MSC RX
	PMA_BUF(8) = 0;
	PMA_BUF(9) = 0;
	PMA_BUF(10) = EP2RX_OFFSET;
	PMA_BUF(11) = 0x8400;

    ENDPOINT(2) = ((ENDPOINT(2) ^ 0x3000) & 0x3030);
*/
	//EP 1 CDC TX
	PMA_BUF(4) = EP1TX_OFFSET;
	PMA_BUF(5) = 0;
	PMA_BUF(6) = 0;
	PMA_BUF(7) = 0;
    ENDPOINT(1) = ((ENDPOINT(1) ^ 0x0020) & 0x3030);
	//EP 2 CDC interrupt
	PMA_BUF(8) = EP2TX_OFFSET;
	PMA_BUF(9) = 0;
	PMA_BUF(10) = 0;
	PMA_BUF(11) = 0;
    ENDPOINT(2) = ((ENDPOINT(2) ^ 0x0020) & 0x3030)|0x0600;
    //EP 3 CDC RX
	PMA_BUF(12) = 0;
	PMA_BUF(13) = 0;
	PMA_BUF(14) = EP3RX_OFFSET;
	PMA_BUF(15) = 0x8400;

    ENDPOINT(3) = ((ENDPOINT(3) ^ 0x3000) & 0x3030);


	USB_Feature = 0xC0;
	USB_Configuration = 0;
	USB_Interface = 0;




//    CNTR = 0x0001;

    ISTR = 0;	// Флаги прерываний(событий) USB
//    CNTR = 0;
    CNTR = 0x8400;


//	Message("Ur\r\0",1);

    DADDR = 0x80;							// Включаем модуль USB, адрес устройства 0
	ISTR &= ~ISTR_RESET;
}


//write data from designated buffer to selected endpoint buffer.
//P1 -
void TO_WRITE_PMA(uint16_t *P1, uint32_t P2, uint16_t N, uint16_t max){

	//uint8_t i;

	N = (N > max)? max : N;
	for(i=0; i<N; i+=2){
		PMA_SBUF(P2+i) = *P1;//_1;
		P1++;
	}

}

void TO_READ_PMA(uint32_t P1, uint16_t *P2, uint16_t N){

	//uint8_t i;

	//N = (N > max)? max : N;
	for(i=0; i<N; i+=2){
		*P2 = PMA_SBUF(P1+i);
		P2++;
	}

}

void USB_Message (char Str[], uint8_t EP){
	uint16_t b1 = PMA_BUF(EP*4);
	//uint8_t i;
	//we need to check if string fits within EPTX buffer boundaries. But we will do it at other places.
	for (i = 0; Str[i]; i++)
	{
		b1 += 2;
		PMA_SBUF(b1) = Str[i];
	}

	i = i * 2 + 2; // надо UO_a16 = (UO_a16 * 2 + 2)&FF, т.к. не больше 256 байт в дескрипторе,
	// но поскольку памяти для сообщений длиннее 20 символов всё равно нет места, не будем обрезать;
	//UO_a16 &= 0xFF;
	b1 = PMA_BUF(EP*4);
	PMA_SBUF(b1) = i | 0x0300; //N, 3 - string descriptor
	PMA_BUF(EP*4+1) = i;

}





void USB_main_COM_react(){
//if (USB_Function_flags){
	// Check to see if we have data yet
//	if (USB_Function_flags & 0x08) {//EP3 is ready to get new bunch of information
//		EP3R = (EP3R^0x3000)&0xBF8F;
//		USB_Function_flags &= 0xF1;//clear 2, 4 and 8 flags
//		USB_Recieved_bytes = 0;
//	}

	if (USB_Bytes_to_send_left){
		//send pack back to the Host
		//while((EP1R & 0x80) == 0){}				// Проверяем CTR_TX (Ждем завершения передачи)
		USB_Function_flags &= 0xFE;
		TO_WRITE_PMA(USB_COM_TX_Buff1 ,EP1TX_OFFSET, 64, USB_EP1_MAX_PACKET_SIZE_TX);
		PMA_BUF(5) = USB_Bytes_to_send_left;
		EP1R = (EP1R^EP_TX_VALID)&0x8FBF;//_SetEPTxStatus(ENDP1, EP_TX_VALID);
		USB_Bytes_to_send_left = 0;

		//uint32_t
		i32 = 5000;
		while (((EP1R & 0x80) == 0) && (i32))
			i32--;
    		  //}
    		  //CDC_Send_DATA ((uint8_t*)Receive_Buffer,Receive_length);
	}


//}
}



void Check_If_USB_Is_AFK(){
	if (USB_Function_flags & 2){
		USB_rx_timeout++;
		if ((USB_rx_timeout > USB_EP3_RECEIVE_TIMEOUT)){
			USB_rx_timeout = 0;
			USB_Recieved_bytes = 0;
			USB_RX_Start_pointer = 0;
			USB_Function_flags &= 0xFD;//data processed
			EP3R = (EP3R^0x3000)&0xBF8F;
			//USB_RESET();
		}
	}
}


void CTR_LP_CTRX() {
//  volatile uint16_t wEPVal = 0;
  /* stay in loop while pending interrupts */
	//uint16_t TempEP;
            	//uint8_t Related_Endpoint, Reserved;
            	//uint16_t USB_wIndex;

  while (((BKIstr = ISTR) & ISTR_CTR)) {
    /* extract highest priority endpoint number */
    EPindex = (uint8_t)(BKIstr & ISTR_EP_ID);


    if (EPindex == 1 && (EP1R & EP_CTR_TX)){ //CDC Transmit
        	EP1R &= 0x8F0F;
        	USB_Function_flags |= 1;
	} else if (EPindex == 3 && (EP3R & EP_CTR_RX)){ //CDC Receive
		USB_Function_flags &= 0xEF;
		USB_RX_Start_pointer = 0;

		USB_rx_timeout = 0;//CoGetOSTime()+USB_EP3_RECEIVE_TIMEOUT;
            EP3R = (EP3R ^ 0x2000) & 0x3F8F;
            USB_Recieved_bytes = (PMA_BUF(15)&0x3FF);
        	TO_READ_PMA(EP3RX_OFFSET, USB_Buff1, 64);
		USB_Function_flags |= 2;


    } else if(EPindex == 0){/*  */

        /* Decode and service control endpoint interrupt */
        /* calling related service routine */
        /* (Setup0_Process, In0_Process, Out0_Process) */

        /* save RX & TX status */
  	    SaveTState = EP0R & 0x3030;
        /* and set both to NAK */
  	    EP0R = (EP0R^0x2020)&0xBFBF;
        /* DIR bit = origin of the interrupt */
        if (BKIstr & ISTR_DIR) {
            /* DIR = 1 */
            /* DIR = 1 & CTR_RX       => SETUP or OUT int */
            /* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */
            	EP0R &=0x0F8F;/* SETUP bit kept frozen while CTR_RX = 1 */
            	//USB_state_flag = STALLED;
            if (EP0R & EP_SETUP) {
            	//Setup0_Process();
              USB_Command = PMA_SBUF(EP0RX_OFFSET);
              USB_wValue  = PMA_SBUF(EP0RX_OFFSET+2);
              USB_wIndex  = PMA_SBUF(EP0RX_OFFSET+4);
              USB_wLength = PMA_SBUF(EP0RX_OFFSET+6);
              if (USB_wLength == 0)
              {
                /* Setup with no data stage */
                //NoData_Setup0();
            	  USB_state_flag = WAIT_STATUS_IN;/* After no data stage SETUP */
            	  PMA_BUF(1) = 0;//_SetEPTxCount(ENDP0, 0);

            	  if (USB_Command == 0x0900){    /* SET_CONFIGURATION*/
            		  //if ((USB_wValue > TOTAL_CONFIG) || USB_wIndex){
            			//  USB_state_flag = STALLED;
            		  //} else {
            			  USB_Configuration = USB_wValue;
            		  //}
            	  } else if (USB_Command == 0x0500){/*SET ADDRESS*/
            		  USB_DADDR = USB_wValue | DADDR_EF;
            		  //if ((USB_wValue > 127) || USB_wIndex || (USB_Configuration)){
            		//	  USB_state_flag = STALLED;
            	    //	  USB_DADDR = 0;
            	     // }
            	  } else if ((USB_Command &0xFDFF) == 0x0100){/*SET FEATURE for Device 0300*//*CLEAR FEATURE for Device 0100*/
            	   //   if ((USB_wValue == 1)//DEVICE_REMOTE_WAKEUP) //1
            	   //       && USB_wIndex == 0)
            	    //  {
            	    //	  USB_Feature &= ~0x20;
            	   // 	  USB_Feature |= ((USB_Command&0x0200)>>4);//0x20;
            	   //   }
            	  } else if (USB_Command == 0x0B01 && (USB_Configuration) && ((USB_wIndex & 0xFF) <= 1)){/*SET INTERFACE*/
            		  //if((USB_wValue == 0) && (USB_Configuration) && ((USB_wIndex & 0xFF) <= 1)){
            			  USB_Interface = USB_wIndex;
            		  //}
            	  } else if (USB_Command == 0x0102){
            	      //Standard_ClearFeature();
            		  /*EndPoint Clear Feature*/
            		  Related_Endpoint = USB_wIndex & 0x7F;

            	      if ((USB_wValue == 0)//ENDPOINT_STALL)
            	          && (USB_wIndex > 0x100) && USB_Configuration)// && (Related_Endpoint < EP_NUM))
            	      {
            		        /*Get Status of endpoint & stall the request if the related_ENdpoint
            		        is Disabled*/
            	    	  TempEP = ENDPOINT(Related_Endpoint);
            	    	  if (USB_wIndex & 0x80) {
            	    		  /* IN endpoint */
            			      if ((TempEP&0x0030)==0x0010)
            			      {
            			          ENDPOINT(Related_Endpoint) = (TempEP ^ 0x0030)&0x8FFF;
            			      }
            		      } else {
            		    	  /* OUT endpoint */
            		    	  if ((TempEP&0x3000)==0x1000)
            		    	  {
            		    		  if (Related_Endpoint == 0)
            			          {
            			            /* After clear the STALL, enable the default endpoint receiver */
            			        	  PMA_BUF(3) = 0x8400;//SetEPRxCount(Related_Endpoint, Device_Property.MaxPacketSize);
            			        	  EP0R = (EP0R ^ 0x3000)&0xBF8F;
            			          }
            			          else
            			          {
            			            ENDPOINT(Related_Endpoint) = (TempEP ^ 0x3000)&0xFF8F;
            			          }
            		    	  }
            		      }
            	      }
            	  } else if (USB_Command == 0x0302){
            	      //Standard_SetEndPointFeature();
            	      Related_Endpoint = USB_wIndex & 0x7F;
            	      if ((USB_wValue==0) && USB_Configuration){//(Related_Endpoint < EP_NUM) &&
            	    	  TempEP = ENDPOINT(Related_Endpoint);
            	    	  if (USB_wIndex & 0x80) {
            				    /* get Status of endpoint & stall the request if the related_ENdpoint
            				    is Disabled*/
            				    if(TempEP&0x0030){
            				    	ENDPOINT(Related_Endpoint) = (TempEP ^ 0x0010)&0x8FBF;
            				    }
            			  } else {
            				    if(TempEP&0x3000){
            				    	ENDPOINT(Related_Endpoint) = (TempEP ^ 0x1000)&0xBF8F;
            				    }
            			  }
            	      }

            	  } else if ((USB_Command & 0xDDFF) == 0x0021){//SET_COMM_FEATURE 0221//SET_CONTROL_LINE_STATE 2221//SET Linecoding 2021
            		  Message("DKLed connected\r\0",DEBUG_PORT_OUT);
            		  USB_main_COM_react();

            	  } else {
            		  USB_state_flag = STALLED;
            	  }

            	  SaveTState |= 0x0030;//(EP_TX_VALID);
              } else {
                /* Setup with data stage */
            	  USB_state_flag =  LAST_IN_DATA;
            	  PMA_BUF(1) = USB_wLength;
            		if (USB_Command == 0x0680){/*GET DESCRIPTOR*/
            		      if (USB_wValue == 0x0100){
            		    	  TO_WRITE_PMA(DeviceDiscriptor,EP0TX_OFFSET,0x12,USB_EP0_MAX_PACKET_SIZE);
            		    	  PMA_BUF(1)= 0x12;
            		      } else if (USB_wValue == 0x0200){
            		    	  TO_WRITE_PMA(ConfigDescriptor,EP0TX_OFFSET,64,USB_EP0_MAX_PACKET_SIZE);
        		    		  //USB_state_flag =  LAST_IN_DATA;
            		    	  if (USB_wLength>9){
            		    		  //pInformation->Ctrl_Info.Usb_wLength = USB_wLength;
            		    		  PMA_BUF(1)= USB_EP0_MAX_PACKET_SIZE;
            		    		  USB_state_flag = IN_DATA;
            		    	  }
            		      } else if (USB_wValue == 0x0300){
            		    	  PMA_SBUF(EP0TX_OFFSET) = 0x0304; // 3 - string descriptor, N=4
            		    	  PMA_SBUF(EP0TX_OFFSET + 2) = 0x0409;
            		    	  PMA_BUF(1)= 4;
            		      } else if (USB_wValue == 0x0301){
            		    	  USB_Message("DKAG\0",0);
            		      } else if (USB_wValue == 0x0302){
            		    	  USB_Message("DKLed V0.7\0",0);
            		      } else if (USB_wValue == 0x0303){
            		    	  uint32_t buf1 = *(__IO uint32_t*)(0x1FFFF7F0);//chip itself
            		    	  for (Reserved = 2; Reserved<24; Reserved+=2){
            		    		  PMA_SBUF(EP0TX_OFFSET + Reserved)= (buf1 & 0x7) + '0';
            		    		  buf1 >>= 3;
            		    	  }
            		    	  PMA_SBUF(EP0TX_OFFSET)= 0x0318;
            		    	  PMA_BUF(1)=24;
            		      }// End of GET_DESCRIPTOR
            		} else if ((USB_Command&0xFFFE) == 0x0080){/* GET STATUS for Device*/ /* GET STATUS for Interface*/
            			Reserved = 0;
            			if (USB_Command == 0x0080){
            			    //if (USB_Feature & 0x20){/* Remote Wakeup enabled */
            			    //	Reserved |= 2;
            			    //}
            			    if (USB_Feature & 0x40){/* Bus-powered */
            			    	Reserved |= 1;
            			    }
            			}
            			PMA_SBUF(EP0TX_OFFSET) = Reserved; // 3 - string descriptor, N=4
            			//PMA_BUF(1)= 2;
            		} else if (USB_Command == 0x0082){/* GET STATUS for EndPoint*/
            			Related_Endpoint = USB_wIndex & 0x7F;
            			PMA_SBUF(EP0TX_OFFSET) =0;
            			TempEP = ENDPOINT(Related_Endpoint);
            			//Reserved = 0;
            			//if (Related_Endpoint < EP_NUM){
            				if (USB_wIndex & 0x80) {
            					/* get Status of endpoint & stall the request if the related_ENdpoint is Disabled*/
            					if(TempEP&0x0030 == 0x0010){//_GetEPTxStatus(Related_Endpoint);
            						PMA_SBUF(EP0TX_OFFSET) =1;//Reserved = 1; /* IN Endpoint stalled */
            					}
            				} else {
            					if(TempEP&0x3000 == 0x1000){//_GetEPRxStatus(Related_Endpoint);
            						PMA_SBUF(EP0TX_OFFSET) =1;//Reserved = 1; /* OUT Endpoint stalled */
            					}
            				}
            			//}
            			//PMA_SBUF(EP0TX_OFFSET) = Reserved; // 3 - string descriptor, N=4
            			//PMA_BUF(1)= 2;
            		} else if (USB_Command == 0x0880){/*GET CONFIGURATION*/
            			PMA_SBUF(EP0TX_OFFSET) = USB_Configuration;
            			//PMA_BUF(1)= 1;
            		//} else if (USB_Command == 0x0080){/*GET INTERFACE*/
            		//	PMA_SBUF(EP0TX_OFFSET) = 0;//pInformation->Current_AlternateSetting;
            			//PMA_BUF(1)= 1;
            		} else if ((USB_Command & 0xFEFF) == 0x20A1){//Get (21A1)/set (20A1) Line Coding
            			PMA_SBUF(EP0TX_OFFSET) = 0xc200;//115200 (49664)
            			PMA_SBUF(EP0TX_OFFSET+2) = 0x1;//115200 (65536)
            			//PMA_BUF(EP0TX_OFFSET) = 9600;//baudrate bytes 1
            			PMA_SBUF(EP0TX_OFFSET+4) = 0;//parity*256+format
            			PMA_SBUF(EP0TX_OFFSET+6) = 8;//datatype
            			//Message("Get line\r\0",2);
            			//PMA_BUF(1)= 7;
            		} else if (USB_Command == 0x2021){//(2021) set baudrate, stop, parity, length
            			USB_state_flag = OUT_DATA;
            		} else {
            			USB_state_flag = STALLED;
            		}
            	    SaveTState = (0x3030);
               }

            } else if (USB_state_flag == OUT_DATA){
            	//save baudrate, stop bits, parity, length
            	//baudrate = PMA_SBUF(EP0RX_OFFSET) + PMA_SBUF(EP0RX_OFFSET+2)*0x10000;
            	//parity = (PMA_SBUF(EP0RX_OFFSET+4))>>8;
            	//format = (PMA_SBUF(EP0RX_OFFSET+4))&0xff;
            	//datatype = (PMA_SBUF(EP0RX_OFFSET+6))&0xff;
            	SaveTState |= 0x0030;
            	PMA_BUF(1)= 0;
            	USB_state_flag = LAST_OUT_DATA;
            } else {
            	USB_state_flag = STALLED;
            }

        } else {
            /* DIR = 0 */
            /* DIR = 0      => IN  int */
            /* DIR = 0 implies that (EP_CTR_TX = 1) always  */
        	EP0R &=0x8F0F;

        	if (USB_state_flag == IN_DATA){
		    	PMA_SBUF(EP0TX_OFFSET) = 0x0040;
		    	PMA_SBUF(EP0TX_OFFSET + 2) = 0;
		    	USB_state_flag =  LAST_IN_DATA;
		    	PMA_BUF(1)= 3;
		    	SaveTState = (0x3030);//vSetEPTxStatus(EP_TX_VALID);
        	} else if (USB_state_flag == LAST_IN_DATA){
        		USB_state_flag = WAIT_STATUS_OUT;
      			SaveTState = (0x3010);
        	} else {//if (USB_state_flag == WAIT_STATUS_IN && USB_DADDR){
        		if (USB_DADDR){//SetDeviceAddress
        			for (Reserved = 0; Reserved < EP_NUM; Reserved++) {
        				ENDPOINT(Reserved) = (ENDPOINT(Reserved) & 0x8F80) | Reserved ;//_SetEPAddress(i, i);
        			}
        			DADDR = USB_DADDR; // set device address and enable function
        			USB_DADDR = 0;
        		}
        		USB_state_flag = STALLED;
        	}/**/


        }
        PMA_BUF(3) = 0x8400;//SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
        //SaveTState = (USB_state_flag == STALLED)?  0x1010:SaveTState;
        if (USB_state_flag == STALLED) SaveTState = 0x1010;

        EP0R = (EP0R^(SaveTState))&0xBFBF;// _SetEPRxTxStatus(ENDP0,SaveRState,SaveTState);
        return;
    }

  }/* while(...) */
}



void USB_CTR_RX(){

	uint8_t UO_a8; //временная переменная - счетчик
	uint16_t UO_a16; //временная переменная

	uint32_t buf1;

	EPindex = (uint8_t)(BKIstr & ISTR_EP_ID);
    if (EPindex == 0){
    	//_ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
    	TO_READ_PMA(EP0RX_OFFSET, USB_Buff1, 10);
    	EP0R = (EP0R ^ 0x2020) & 0x3FBF; //переводим EP0 в состояние RX_NAK TX_NAK
        		PMA_BUF(1)=0;
        		USB_Command = USB_Buff1[0]+USB_Buff1[1]*256;
        		if (USB_Command == 0x0680){
    			// Запрос дискриптора
        			//Port_send_char('d',1);
        			switch(USB_Buff1[3]){
        				case 0x01:							// Запрос дискриптора устройства

        					TO_WRITE_PMA(DeviceDiscriptor, EP0TX_OFFSET, 18, USB_EP0_MAX_PACKET_SIZE);		// Заполняем буфер передачи дискриптором устройства
        					PMA_BUF(1)=0x12;
        					//Port_send_char('1',1);
        					break;
        				case 0x02:							// Запрос дескриптора конфигурации
        					//MSC:
        					//TO_WRITE_PMA(ConfigDescriptor, EP0TX_OFFSET, 32, USB_EP0_MAX_PACKET_SIZE);		// Заполняем буфер передачи дискриптором конфигурации
        					//CDC:
        					TO_WRITE_PMA(ConfigDescriptor, EP0TX_OFFSET, 64, USB_EP0_MAX_PACKET_SIZE);		// Заполняем буфер передачи дискриптором конфигурации

        					buf1=USB_Buff1[6]+USB_Buff1[7]*256;
        					if (buf1!=9){ //если дескриптор слишком длинный, то сюда записывается отправка всех частей дескриптора кроме последней (последняя должна быть СТРОГО МЕНЬШЕ размера буфера отправки, но может быть нулевой)
        						//MSC:
        						//buf1=32;
        						//CDC:
        						PMA_BUF(1)=64;
        	        			// Разрешить передачу
        	        			PMA_BUF(3) = 0x8400;
        	        			EP0R = (EP0R ^ 0x3030) & 0xBF3F;
        	        		    UO_a16 = 25000;
        	        			while(((EP0R & 0x80) == 0)&& UO_a16){				// Проверяем CTR_TX (Ждем завершения передачи)
        	        			UO_a16--;
        	        		    }
        	        			//PMA_SBUF(EP0TX_OFFSET) = 0x0040; // Last 3 bytes of descriptor
        	        			//PMA_SBUF(EP0TX_OFFSET + 2) = 0;
        	        			//TO_WRITE_PMA(ConfigDescriptor5, EP0TX_OFFSET, 3, USB_EP0_MAX_PACKET_SIZE);
        	        			PMA_SBUF(EP0TX_OFFSET) = 0x0040;
        	        			PMA_SBUF(EP0TX_OFFSET + 2) = 0;
        						buf1=3;
        						//end for CDC
        					}
        					PMA_BUF(1)=buf1;
        					//Port_send_char('2',1);
        					break;
        				case 0x03:								//Запрос дескриптора строки
        					//Port_send_char('3',1);
        					switch(USB_Buff1[2]){
        						case 0://язык
        							PMA_SBUF(EP0TX_OFFSET) = 0x0304; // 3 - string descriptor, N=4
        							PMA_SBUF(EP0TX_OFFSET + 2) = 0x0409;
        							PMA_BUF(1)=4;// Заполняем COUNT_TX количеством байт (в запросе требуется 256 байт, поэтому принудительно ставлю 4, т.е. фактическое количество)
        							//Port_send_char('0',1);
        							break;
        						case 1://Vendor
        							USB_Message("DKAG\0",0);
        							//Port_send_char('1',1);
        							break;
        						case 2://Product
        							USB_Message("DKLed V0.7\0",0);//10 символов, 10*2+2=22 байта
        							//Port_send_char('2',1);
        							break;
        						default: //case 3  - device serial number
        							//buf1 = *(__IO uint32_t*)(0x1FFFF7E8);//Lot
        							//buf1 ^= *(__IO uint32_t*)(0x1FFFF7EC);//Silicon wafer
        							buf1 = *(__IO uint32_t*)(0x1FFFF7F0);//chip itself
        							//buf1 = 0;
        							for (UO_a8 = 2; UO_a8<24; UO_a8+=2){
        								PMA_SBUF(EP0TX_OFFSET + UO_a8)= (buf1 & 0x7) + '0';
        								buf1 >>= 3;
        							}
        							PMA_SBUF(EP0TX_OFFSET)= 0x0318;
        							PMA_BUF(1)=24;
        							//Port_send_char('3',1);
        							break;
        					}
        				default:
        					break;
        			}

USB_SEND_SETUP_lable:
        			// Разрешить передачу
        			PMA_BUF(3) = 0x8400;
        			EP0R = (EP0R ^ 0x3030) & 0xBF3F;
        		    UO_a16 = 25000;
        			while(((EP0R & 0x80) == 0)&& UO_a16){				// Проверяем CTR_TX (Ждем завершения передачи)
        			UO_a16--;
        		    }
        			// Разрешить прием
        			PMA_BUF(3) = 0x8400;
        			EP0R = (EP0R ^ 0x3010) & 0x3FBF;
        			UO_a16 = 25000;
        			while(((EP0R & 0x8000) == 0)&& UO_a16){				// Проверяем CTR_RX (Ждем пустого пакета подтверждения)
        			UO_a16--;
        		    }
        			PMA_BUF(3) = 0x8400;
        			EP0R = (EP0R ^ 0x3020) & 0x3F3F;
//end of USB_SEND_SETUP_lable (it takes ~50 bytes less if called through goto instead of function)


        		} else if (USB_Command == 0x0500){// SET_ADDRESS
        			USB_DADDR = USB_Buff1[2] | 128;
        			//Port_send_char('a',1);

USB_NO_SEND_SETUP_lable:
					// Разрешить передачу
					PMA_BUF(1)=0;
					PMA_BUF(3) = 0x8400;
					EP0R = (EP0R ^ 0x3030) & 0xBF3F;
					UO_a16 = 25000;
					while(((EP0R & 0x80) == 0)&& UO_a16){				// Проверяем CTR_TX (Ждем завершения передачи)
					UO_a16--;
					}
					// Разрешить прием
					PMA_BUF(3) = 0x8400;
					EP0R = (EP0R ^ 0x3020) & 0x3F3F;

					EP0R = EP0R & 0x8F80;
					if (USB_DADDR){
						for (UO_a8 = 0; UO_a8 < EP_NUM; UO_a8++) {
							ENDPOINT(UO_a8) = (ENDPOINT(UO_a8) & 0x8F80) | UO_a8 ;//_SetEPAddress(i, i);
						}
						//ENDPOINT(1) = (ENDPOINT(1) & 0x8F80) | 1;
						//ENDPOINT(2) = (ENDPOINT(2) & 0x8F80) | 2;
						//for CDC only:
						//ENDPOINT(3) = (ENDPOINT(3) & 0x8F80) | 3;
						DADDR = USB_DADDR;
						USB_DADDR = 0;
					}
//end of USB_NO_SEND_SETUP_lable (it takes ~50 bytes less if called through goto instead of function)


        		} else if ((USB_Command & 0xFFFC)== 0x0080 ){//0080, 0081, 0082
        		//send device status (так как у нас питается от УСБ и при этом не переходит в айдл, то возвращаем 1, если бы переходило то 2 - самопитается или 3 - переходит в айдл и питается от усб. См DeviceDescriptor [7])
        			UO_a16 = (USB_Buff1[0] == 0x81) ? 0:1;
        			if (USB_Buff1[0] & 2){
        				//send endpoint status (Stall = 1, other = 0)
        				buf1 = ENDPOINT((USB_Buff1[4] & 0x07));
        				if ((USB_Buff1[4] & 0x80)){ //TX stall check
        					UO_a16 = ((buf1 & 0x30) == 0x10) ? 1 : 0;
        				} else {
        					UO_a16 = ((buf1 & 0x3000) == 0x1000) ? 1 : 0;
        				}
        			}
        			PMA_SBUF(EP0TX_OFFSET) = UO_a16;
        			PMA_BUF(1)=2;// Заполняем COUNT_TX количеством байт
        			//Port_send_char('s',1);
					goto USB_SEND_SETUP_lable;
        		} else if ((USB_Command & 0xFDFF)== 0x0102 ){//0302, 0102
        			UO_a8 = (USB_Buff1[4] & 0x07);
        			UO_a16 = ENDPOINT(UO_a8)&0x3030;
        			ENDPOINT(UO_a8) &= 0x8F8F;
        			if ((USB_Buff1[4] & 0x80)){ //TX stall check
    	   				UO_a16 &= 0x30;
    	   				UO_a16 = (USB_Buff1[1] == 3) ? 0x10 : ( ((UO_a16 == 0x10)) ? 0x30 : UO_a16 );
        			} else {
        				UO_a16 &= 0x3000;
        				UO_a16 = (USB_Buff1[1] == 3) ? 0x1000 : ( ((UO_a16 == 0x1000)) ? 0x3000 : UO_a16 );
        			}
        			ENDPOINT(UO_a8) |= UO_a16;//set STALL or VALID if needed
        			if (UO_a8==0){
        				PMA_BUF(3) = 0x8400; //clear EP0 RX
        			}
        			//Port_send_char('e',1);
        			goto USB_NO_SEND_SETUP_lable;
        		} else if ((USB_Command & 0xFDFE) == 0x0880) {//0880,0A81
        			//Port_send_char('f',1);
        			PMA_SBUF(EP0TX_OFFSET) = 1;
        			PMA_BUF(1)=1;// Заполняем COUNT_TX количеством байт
        			goto USB_SEND_SETUP_lable;
        		} else if ((USB_Command & 0xFDFE) == 0x0900) {//0900,0B01
        			//Port_send_char('k',1);
        			goto USB_NO_SEND_SETUP_lable;
/*        		} else if (USB_Command == 0xFF21){ //Mass Storage reset
        			ENDPOINT(1) = (ENDPOINT(1) ^ 0x0040) & 0x8FCF;
        			ENDPOINT(2) = (ENDPOINT(2) ^ 0x4000) & 0xCF8F;
        			Port_send_char('M',1);
        			goto USB_NO_SEND_SETUP_lable;
        		} else if (USB_Command == 0xFEA1){ //Mass Storage return max LUN
        			PMA_SBUF(EP0TX_OFFSET) = 0;
        			PMA_BUF(1)=1;// Заполняем COUNT_TX количеством байт
        			Port_send_char('L',1);
					goto USB_SEND_SETUP_lable;
*/
        		} else if ((USB_Command & 0xDFFF) == 0x0221){ //Set COM feature (0221)/control line state (2221)
        			//Port_send_char('C',1);
        			goto USB_NO_SEND_SETUP_lable;
        		} else if ((USB_Command & 0xFE7F) == 0x2021){ //return COM line coding (2121,2021, 20A1, 21A1)

        			PMA_BUF(EP0TX_OFFSET) = 9600;//baudrate bytes 1
        			PMA_SBUF(EP0TX_OFFSET+4) = 0;//parity*256+format
        			PMA_SBUF(EP0TX_OFFSET+6) = 8;//datatype

        			PMA_BUF(1)=7;// Заполняем COUNT_TX количеством байт
        			//Port_send_char('D',1);
					goto USB_SEND_SETUP_lable;

        		} else { //default,
        			Port_send_char('-',1);
        			EP0R = ((EP0R^0x1010)&0x3F3F)| 0x8080;
        			ISTR &= ~ISTR_CTR;
        			//Message("stl\r\n\0",1);
        			return;
        		}/**/


    //} //EPindex=0
/*    } else if (EPindex == 1){ //MSC Transmit
    	//buf1 = PMA_BUF(EPindex*4 + 2); //адрес RX для актуальной конечной точки
    	Port_send_char('T',1);
    	ENDPOINT(1) &= 0x0F8F;
    } else if (EPindex == 2){ //MSC Receive
    	//buf1 = PMA_BUF(EPindex*4 + 2); //адрес RX для актуальной конечной точки
    	Port_send_char('R',1);
    	ENDPOINT(2) &= 0x8F0F;
*/
    } else if (EPindex == 1 && (EP1R & EP_CTR_TX)){ //CDC Transmit
    	//buf1 = PMA_BUF(EPindex*4 + 2); //адрес RX для актуальной конечной точки
//    	Port_send_char('T',1);
    	EP1R &= 0x8F0F;
    	USB_Function_flags |= 1;
//    	USB_Message("Transmit \0",1);
        //EP1R = (EP1R ^ 0x0030) & 0xBF3F;

//    } else if (EPindex == 2){ //CDC interrupt
    	//buf1 = PMA_BUF(EPindex*4 + 2); //адрес RX для актуальной конечной точки
//    	Port_send_char('I',1);
//    	EP2R &= 0x8F3F;
    } else if (EPindex == 3 && (EP1R & EP_CTR_RX)){ //CDC Receive
    	//buf1 = PMA_BUF(EPindex*4 + 2); //адрес RX для актуальной конечной точки
//    	Port_send_char('R',1);
    	//ENDPOINT(3) &= 0x8F0F;
    	USB_Function_flags |= 2;
        EP3R = (EP3R ^ 0x2000) & 0x3F8F;
        USB_Recieved_bytes = PMA_BUF(15)&0x3F;
    	TO_READ_PMA(EP3RX_OFFSET, USB_Buff1, 64);

    	EP3R = ((EP3R ^ 0x3000) & 0x3F8F);
    }






	ISTR &= ~ISTR_CTR;
//	Port_send_char('u',1);
//	Port_send_char('\r',1);

}




void USB_LP_CAN1_RX0_IRQHandler(void){
	//CoEnterISR();
	BKIstr = ISTR;

	if(BKIstr & ISTR_CTR){
		//USB_CTR_RX();
		CTR_LP_CTRX();
	}/**/
	if(BKIstr & ISTR_RESET){
		USB_RESET();
	}/**/	
	//CoExitISR();
}//USB_LP_CAN1_RX0_IRQHandler


