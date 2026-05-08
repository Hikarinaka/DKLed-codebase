
#include "stm32F10x.h"




#define REG(x)  (*((volatile unsigned int *)(x)))

#define USB_BASE_ADDR   0x40005C00          /*!<USB Registers Base Address               */
#define USB_PMA_ADDR    0x40006000          /*!<USB Packet Memory Area Address           */

/*****************************************************************************************
 *   Common Registers
 *****************************************************************************************/
#define EP0R	REG(USB_BASE_ADDR + 0x00)   /*!<Buffer Table Address Register            */
#define CNTR	REG(USB_BASE_ADDR + 0x40)   /*!<Control Register                         */
#define ISTR	REG(USB_BASE_ADDR + 0x44)   /*!<Interrupt Status Register                */
#define DADDR	REG(USB_BASE_ADDR + 0x4C)   /*!<Device Address Register                  */
#define BTABLE	REG(USB_BASE_ADDR + 0x50)   /*!<Buffer Table Address Register            */

#define ISTR_CTR 0x8000						// 15ый бит устанавливается после удачной транзакции
#define ISTR_PMAOVR 0x4000
#define ISTR_ERR 0x2000						// 13ый бит ERR
#define ISTR_WKUP 0x1000
#define ISTR_SUSP 0x0800					// 11ой бит SUSP
#define ISTR_RESET 0x0400					// 10ый бит RESET
#define ISTR_SOF 0x0200
#define ISTR_ESOF 0x0100					// 8ой бит ESOF
#define ISTR_DIR 0x010						// 4ый бит DIR



//ДЕСКРИПТОРЫ С САЙТА http://ravenium.ru/stm32-и-usb-hid-это-просто/
/* USB Standard Device Descriptor */
unsigned char DeviceDiscriptor[18] =
{
		    0x12,         // общая длина дескриптора устройства в байтах
		    0x01, // bDescriptorType - показывает, что это за дескриптор. В данном случае - Device descriptor
		    0x10, 0x01,                 // bcdUSB - какую версию стандарта USB поддерживает устройство. 2.0

			// класс, подкласс устройства и протокол, по стандарту USB. У нас нули, означает каждый интерфейс сам за себя
		    0x00,                       //bDeviceClass
		    0x00,                       //bDeviceSubClass
		    0x00,                       //bDeviceProtocol

		    0x40,                       //bMaxPacketSize - максимальный размер пакетов для Endpoint 0 (при конфигурировании)

			// те самые пресловутые VID и PID,  по которым и определяется, что же это за устройство.
		    0x83, 0x04,                 //idVendor (0x0483)
		    0x11, 0x57,                 //idProduct (0x5711)

		    0x01, 0x00,                 // bcdDevice rel. DEVICE_VER_H.DEVICE_VER_L  номер релиза устройства

			// дальше идут индексы строк, описывающих производителя, устройство и серийный номер.
			// Отображаются в свойствах устройства в диспетчере устройств
			// А по серийному номеру подключенные устройства с одинаковым VID/PID различаются системой.
		    1,                          //Index of string descriptor describing manufacturer
		    2,                          //Index of string descriptor describing product
		    3,                          //Index of string descriptor describing the device serial number
		    0x01                        // bNumConfigurations - количество возможных конфигураций. У нас одна.
  }
  ; /* CustomHID_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
unsigned char ConfigDescriptor[41] =
  {
		    0x09, 			// bLength: длина дескриптора конфигурации
		    0x02, // bDescriptorType: тип дескриптора - конфигурация
		    41, 0x00, // wTotalLength: общий размер всего дерева под данной конфигурацией в байтах

		    0x01,         // bNumInterfaces: в конфигурации всего один интерфейс
		    0x01,         // bConfigurationValue: индекс данной конфигурации
		    0x00,         // iConfiguration: индекс строки, которая описывает эту конфигурацию
		    0xE0,         // bmAttributes: признак того, что устройство будет питаться от шины USB
		    0x32,         // MaxPower 100 mA: и ему хватит 100 мА

				/************** Дескриптор интерфейса ****************/
				0x09,         // bLength: размер дескриптора интерфейса																									//10
				0x04, 		  // bDescriptorType: тип дескриптора - интерфейс
				0x00,         // bInterfaceNumber: порядковый номер интерфейса - 0
				0x00,         // bAlternateSetting: признак альтернативного интерфейса, у нас не используется
				0x02,         // bNumEndpoints - количество эндпоинтов.

				0x03,         // bInterfaceClass: класс интерфеса - HID
				// если бы мы косили под стандартное устройство, например клавиатуру или мышь, то надо было бы указать правильно класс и подкласс
				// а так у нас общее HID-устройство
				0x00,         // bInterfaceSubClass : подкласс интерфейса.
				0x00,         // nInterfaceProtocol : протокол интерфейса

				0,            // iInterface: индекс строки, описывающей интерфейс

					// теперь отдельный дескриптор для уточнения того, что данный интерфейс - это HID устройство
					/******************** HID дескриптор ********************/
					0x09,         // bLength: длина HID-дескриптора
					0x21, // bDescriptorType: тип дескриптора - HID																										//20
					0x01, 0x01,   // bcdHID: номер версии HID 1.1
					0x00,         // bCountryCode: код страны (если нужен)
					0x01,         // bNumDescriptors: Сколько дальше будет report дескрипторов
						0x22,         // bDescriptorType: Тип дескриптора - report
						23,	0x00, // wItemLength: длина report-дескриптора																								//27**************************************

// Почему-то если не описать две конечные точки, то ничего не работает. Хотя для передачи данных используется только нулевая точка
					/******************** дескриптор конечных точек (endpoints) ********************/
					0x07,          // bLength: длина дескриптора
					0x05, // тип дескриптора - endpoints

					0x81,          // bEndpointAddress: адрес конечной точки и направление 1(IN)																		//30
					0x03,          // bmAttributes: тип конечной точки - Interrupt endpoint
					0x40, 0x00,    // wMaxPacketSize:  Bytes max
					0x20,          // bInterval: Polling Interval (32 ms)																								//34

          0x07,	/* bLength: Endpoint Descriptor size */
          0x05,	/* bDescriptorType: */
            /*	Endpoint descriptor type */
          0x01,	/* bEndpointAddress: */
            /*	Endpoint Address (OUT) */
          0x03,	/* bmAttributes: Interrupt endpoint */
          0x40,	/* wMaxPacketSize:  Bytes max  */
          0x00,																																							//40
          0x20,	/* bInterval: Polling Interval (32 ms) */																												//41***************************************
}
  ; /* RHID_ConfigDescriptor */







//Дескриптор репорта из книги Агурова
unsigned char ReportDescriptor[23] =
  {

		  0x06, 0x00, 0xff,		// USAGE_PAGE (Generic Desktop)
		  0x09, 0x01,			// USAGE (Vendor Usage 1)
		  0xa1, 0x01,			// COLLECTION (Application)
		  0x19, 0x01,			// USAGE_MINIMUM (Vendor Usage 1)
		  0x29, 0x01,			// USAGE_MAXIMUM (Vendor Usage 1)
		  0x15, 0x00,			// LOGICAL_MINIMUM (0)
		  0x26, 0xff, 0x00,		// LOGICAL_MAXIMUM (255)
		  0x75, 0x08,			// REPORT_SIZE (8)
		  0x95, 64,				// REPORT_COUNT(64)
		  0xB1, 0x02,			//FEATURE (Data,Var,Abs)
		  0xc0					//END_COLLECTION
};



unsigned char StringDescriptor0[4] = {4, 3, 9, 13};
unsigned char StringDescriptor1[10] = {10, 3, 49, 0, 49, 0, 49, 0, 49, 0};
unsigned char StringDescriptor2[10] = {10, 3, 50, 0, 50, 0, 50, 0, 50, 0};
unsigned char StringDescriptor3[10] = {10, 3, 51, 0, 51, 0, 51, 0, 51, 0};



int AdrBuff;

unsigned char Buff1[64];


int k;













void TO_WRITE_PMA(unsigned char *P1, int P2, int N){
	unsigned short int *P1_1;
	unsigned int *P2_1;
	int i, buf1;;

	P1_1 = P1;
	P2_1 = P2*2;
	P2_1 = P2_1 + 0x10001800;							// Это 0x40006000 / 4
	for(i=0; i<N; i++){
		buf1 = *P1_1;
		*P2_1 = buf1;
		P1_1++;
		P2_1++;
	}

}


void TO_READ_PMA(int P1, unsigned char *P2, int N){
	unsigned int *P1_1;
	unsigned short int *P2_1;
	int i, buf1;

	P1_1 = P1 * 2;
	P1_1 = P1_1 + 0x10001800;							// Это 0x40006000 / 4
	P2_1 = P2;
	for(i=0; i<N; i++){
		buf1 = *P1_1;
		*P2_1 = buf1;
		P1_1++;
		P2_1++;
	}

}


void USB_CTR(void){							// Это бесполезный обработчик. По идее, здесь должны обрабатываться транзакции. Однако из datasheet-а
	ISTR &= ~ISTR_CTR;						// совершенно не понятно - как сбросить флаг CTR (или условия его сброса). Так что будем наблюдать за
}											// регистром EP0R из основного цикла


void USB_PMAOVR(void){
	ISTR &= ~ISTR_PMAOVR;
}

void USB_ERR(void){
	ISTR &= ~ISTR_ERR;
}

void USB_WKUP(void){
	ISTR &= ~ISTR_WKUP;
}

void USB_SUSP(void){
	ISTR &= ~ISTR_SUSP;
}

void USB_RESET(void){

	int buf1;

	unsigned short int DISCRIPTOR0[4] = {	64, 			// ADDR_TX = 64 оставлено место для всех 8 дискрипторов перед буфером приема(можно оставить меньше)
			0, 				// COUNT_TX = 0 нужно будет заполнить перед отправкой данных
			128,			// ADDR_RX = 128 пропускаем 64 байта буфера передачи
			33792};			// Указываем размер буфера приема 64 байта


    RCC -> APB1ENR |= RCC_APB1ENR_USBEN;	// Без включения тактирования, пакетная память не работает (возвращаются только нули)

	TO_WRITE_PMA(DISCRIPTOR0, 0, 4);		// Заполняем дискриптор нулевой точки

    CNTR = 0;
    BTABLE = 0;								// Адрес таблицы дискрипторов внутри пакетной памяти
    ISTR = 0;								// Флаги прерываний(событий) USB

    // Инициализация EP0R. Сложность в том, что разные биты имеют разный тип обращения (RW, toggle, и. др.)
    // Разрешен прием данных, установлен тип точки CONTROL, запрещена передача
    buf1 = EP0R;
    buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
    buf1 = buf1 & 12288;					//0011000000000000b (Сбросить все биты кроме STAT_RX)
    buf1 = buf1 | 512;						//0000001000000000b (Один бит нужно просто установить (EP_TYPE) (он просто типа RW)
    EP0R = buf1;

    DADDR = 128;							// Включаем модуль USB, адрес устройства 0

	ISTR &= ~ISTR_RESET;
}

void USB_SOF(void){
	ISTR &= ~ISTR_SOF;
}

void USB_ESOF(void){
	ISTR &= ~ISTR_ESOF;
}

void USB_CTR_RX(){
	int buf1;


	TO_READ_PMA(128, Buff1, 32);			// Если завершилась транзакция приема (и при этом количество принятых байт не равно нулю), то читаем буфер приема

	switch(Buff1[0]+Buff1[1]*256){
		case 0x0680:						// Запрос дискриптора

			switch(Buff1[3]){
				case 0x01:							// Запрос дискриптора устройства

					TO_WRITE_PMA(DeviceDiscriptor, 64, 9);		// Заполняем буфер передачи дискриптором устройства
					TO_WRITE_PMA(&Buff1[6], 2, 1);				// Заполняем COUNT_TX количеством байт из принятого запроса

					// Разрешить передачу
					buf1 = EP0R;
					buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
					EP0R = buf1;

				    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
					}

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;

				    while((EP0R & 0x8000) == 0){			// Проверяем CTR_RX (Ждем пустого пакета подтверждения)
					}										// Первые два байта в буфере приема в пакетной памяти, в результате, окажутся заполнены нулями

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;


					break;

				case 0x02:							// Запрос дескриптора конфигурации


					TO_WRITE_PMA(ConfigDescriptor, 64, 21);		// Заполняем буфер передачи дискриптором конфигурации

					buf1=Buff1[6]+Buff1[7]*256;
					if(buf1==9){}else{buf1=41;}				// Без этой строчки не работает
					TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (в запросе требуется 256 байт, поэтому принудительно ставлю 41, т.е. фактическое количество)


					// Разрешить передачу
					buf1 = EP0R;
					buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
					EP0R = buf1;

				    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
					}

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;

				    while((EP0R & 0x8000) == 0){			// Проверяем CTR_RX (Ждем пустого пакета подтверждения)
					}										// Первые два байта в буфере приема в пакетной памяти, в результате, окажутся заполнены нулями

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;




					break;

				case 0x03:								//Запрос дескриптора строки
					switch(Buff1[2]){
						case 0:
							TO_WRITE_PMA(StringDescriptor0, 64, 2);		// Заполняем буфер передачи нулевым строковым дискриптором
							buf1 = 4;
							TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (в запросе требуется 256 байт, поэтому принудительно ставлю 4, т.е. фактическое количество)
							break;

						default:

							TO_WRITE_PMA(StringDescriptor3, 64, 5);		// Заполняем буфер передачи нулевым строковым дискриптором
							buf1 = 10;
							TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (в запросе требуется 256 байт, поэтому принудительно ставлю 10, т.е. фактическое количество)
							break;
					}

					// Разрешить передачу
					buf1 = EP0R;
					buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
					EP0R = buf1;

				    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
					}

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;

				    while((EP0R & 0x8000) == 0){			// Проверяем CTR_RX (Ждем пустого пакета подтверждения)
					}										// Первые два байта в буфере приема в пакетной памяти, в результате, окажутся заполнены нулями

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;

				default:

					break;
			}
			break;

		case 0x0500:						// SET_ADDRESS

			AdrBuff = Buff1[2] | 128;

			buf1 = 0;
			TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (0 байт)

			// Разрешить передачу
			buf1 = EP0R;
			buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
			EP0R = buf1;



		    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
			}




			DADDR = AdrBuff;						// Устанавливаем новый адрес

			// Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;

			break;

		case 0x0900:						//SET_CONFIGURATION
			buf1 = 0;
			TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (0 байт)

			// Разрешить передачу
			buf1 = EP0R;
			buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
			EP0R = buf1;



		    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
			}

			// Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;

			break;

		case 0x0A21:							//SET_IDLE
			buf1 = 0;
			TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (0 байт)

			// Разрешить передачу
			buf1 = EP0R;
			buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
			EP0R = buf1;



		    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
			}

			// Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;

			break;

		case 0x0100:							//CLEAR_FEATURE к устройству (ЭТОТ ЗАПРОС ПОЯВИЛСЯ В WINDOWS 8.1; НА СЕМЕРКЕ И XP ЕГО НЕ БЫЛО)
			buf1 = 0;
			TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (0 байт)

			// Разрешить передачу
			buf1 = EP0R;
			buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
			EP0R = buf1;



		    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
			}

			// Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;

			break;

		case 0x0681:								//Запрос нестандартного дескриптора

			switch(Buff1[3]){
				case 0x22:							//Запрос дескриптора репорта

					TO_WRITE_PMA(ReportDescriptor, 64, 12);		// Заполняем буфер передачи нулевым дискриптором репорта
					buf1 = 23;
					TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (в запросе требуется более 256 байт, поэтому принудительно ставлю 23, т.е. фактическое количество)





					// Разрешить передачу
					buf1 = EP0R;
					buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
					EP0R = buf1;
				    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
					}

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;
				    while((EP0R & 0x8000) == 0){			// Проверяем CTR_RX (Ждем пустого пакета подтверждения)
					}										// Первые два байта в буфере приема в пакетной памяти, в результате, окажутся заполнены нулями

					// Разрешить прием
					buf1 = EP0R;
					buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
					buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
					EP0R = buf1;
					break;



				default:
					break;
			}
			break;


		case 0x0921:								//HID запрос SET_REPORT (Feature)

			// Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;
		    while((EP0R & 0x8000) == 0){			// Проверяем CTR_RX (Ждем окончания приема данных)
			}										//

//СЛЕДУЮЩИЕ ЧЕТЫРЕ СТРОКИ НА РАБОТОСПОСОБНОСТЬ USB КАК ТОКОВОГО НЕ ВЛИЯЮТ ЭТО ТЕСТ ЧТЕНИЯ ДАННЫХ
		    TO_READ_PMA(128, Buff1, 32);			// читаем буфер приема
			k=Buff1[0]+Buff1[1]*256+Buff1[2]*65536+Buff1[3]*16777216;
		    k++;
			if(Buff1[4]==0){GPIOC->BSRR |= GPIO_BSRR_BR13;}else{GPIOC->BSRR |= GPIO_BSRR_BS13;}



			buf1 = 0;
			TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт (0 байт)

			// Разрешить передачу
			buf1 = EP0R;
			buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
			EP0R = buf1;



		    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи пустого пакета)
			}

//			TO_READ_PMA(128, Buff1, 32);			// читаем буфер приема (В ДАЛЬНЕЙШЕМ ЭТО НЕ НАДО, ТАМ ПРОСТО ДВА ПЕРВЫХ НУЛЯ)

		    // Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;

			break;

		case 0x01A1:								//HID запрос GET_REPORT (Feature)

//СЛЕДУЮЩАЯ СТРОКА НА РАБОТОСПОСОБНОСТЬ USB КАК ТОКОВОГО НЕ ВЛИЯЕТ ЭТО ТЕСТ ПЕРЕДАЧИ ДАННЫХ
			Buff1[3]=k/16777216; k=k-Buff1[3]*16777216; Buff1[2]=k/65536; k=k-Buff1[2]*65536; Buff1[1]=k/256; k=k-Buff1[1]*256; Buff1[0]=k;



			TO_WRITE_PMA(Buff1, 64, 4);					// Заполняем буфер передачи
			buf1=64;
			TO_WRITE_PMA(&buf1, 2, 1);				// Заполняем COUNT_TX количеством байт

			// Разрешить передачу
			buf1 = EP0R;
			buf1 = buf1 ^ 48;						//0000000000110000b ( ^ - это XOR ) (Включить два бита STAT_TX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 36671;					//1000111100111111b (Сбрасываем все биты типа toggle, кроме STAT_TX, чтобы не менять их; Сбрасываем бит CTR_TX)
			EP0R = buf1;

		    while((EP0R & 0x80) == 0){				// Проверяем CTR_TX (Ждем завершения передачи)
			}

			// Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;

		    while((EP0R & 0x8000) == 0){			// Проверяем CTR_RX (Ждем пустого пакета подтверждения)
			}										// Первые два байта в буфере приема в пакетной памяти, в результате, окажутся заполнены нулями

			// Разрешить прием
			buf1 = EP0R;
			buf1 = buf1 ^ 12288;					//0011000000000000b ( ^ - это XOR ) (Включить два бита STAT_RX с учетом того, что они имеют тип toggle)
			buf1 = buf1 & 16271;					//0011111110001111b (Сбрасываем все биты типа toggle, кроме STAT_RX, чтобы не менять их; Сбрасываем бит CTR_RX)
			EP0R = buf1;

			break;






		default:

//			WriteBuff1();

			while(1){
			}

		    break;

	}

//	WriteBuff1();
}





int main(void){
	int i;


	SystemInit();

	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;	// Разрешить тактирование портов A B и C.

	// Инициализируем единственный светодиод на плате PC13
	GPIOC->CRH |= GPIO_CRH_MODE13;								// Порт PC13 на выход 50 MHz.
	GPIOC->CRH &= ~GPIO_CRH_CNF13;								// Двухтактный выход на PC13.

//	TO_INIT_THE_LCD();
//	TO_CLEAR_THE_SCREEN();


	USB_RESET();


	while(1){

		if(ISTR & ISTR_CTR){
			USB_CTR();
		}

		if(ISTR & ISTR_PMAOVR){
			USB_PMAOVR();
		}

		if(ISTR & ISTR_ERR){
			USB_ERR();
		}

		if(ISTR & ISTR_WKUP){
			USB_WKUP();
		}

		if(ISTR & ISTR_SUSP){
			USB_SUSP();
		}

		if(ISTR & ISTR_RESET){
			USB_RESET();
			//return;
		}

		if(ISTR & ISTR_SOF){
			USB_SOF();
		}

		if(ISTR & ISTR_ESOF){
			USB_ESOF();
		}

		if(EP0R & 32768){											// 1000000000000000b (CTR_RX)
			USB_CTR_RX();
		}

	}

}


