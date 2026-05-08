//USB
//extern const char DeviceDiscriptor[18];
//extern const char ConfigDescriptor[32];
//extern const char ReportDescriptor[23];
//const char StringDescriptor0[4];
//const char StringDescriptor1[10]; //Vendor
//const char StringDescriptor2[10]; //product
//const char StringDescriptor3[10]; //serial nomber

#define REG(x)  (*((volatile unsigned int *)(x)))
//#define REG(x)  (*((__IO unsigned int *)(x)))

#define USB_BASE_ADDR   0x40005C00          //!<USB Registers Base Address               */
#define USB_PMA_ADDR    0x40006000          //!<USB Packet Memory Area Address           */

/*****************************************************************************************
 *   Common Registers
 *****************************************************************************************/
#define EP0R	REG(USB_BASE_ADDR)   //!<Buffer Table Address Register            */
#define EP1R	REG(0x40005C04)   //!<Buffer Table Address Register            */
#define EP2R	REG(0x40005C08)   //!<Buffer Table Address Register            */
#define EP3R	REG(0x40005C0C)   //!<Buffer Table Address Register            */
#define EP4R	REG(0x40005C10)   //!<Buffer Table Address Register            */
#define EP5R	REG(0x40005C14)   //!<Buffer Table Address Register            */
#define EP6R	REG(0x40005C18)   //!<Buffer Table Address Register            */
#define EP7R	REG(0x40005C1C)   //!<Buffer Table Address Register            */

#define ENDPOINT(bEpNum)        REG(USB_BASE_ADDR + (bEpNum)*4)

#define PMA_BUF(INum)        REG(USB_PMA_ADDR + (INum)*4)
#define PMA_SBUF(SINum)        (*((volatile unsigned short int *)(USB_PMA_ADDR + (SINum)*2)))


#define EP0RX_OFFSET 	0x80//0x58
#define EP0TX_OFFSET 	0x40//0x18

#define EP1RX_OFFSET 	0
#define EP1TX_OFFSET 	0xC0

#define EP2RX_OFFSET 	0x100
#define EP2TX_OFFSET 	0x100

#define EP3RX_OFFSET 	0x110
#define EP3TX_OFFSET 	0

#define EP_NUM			4
#define TOTAL_CONFIG	1

#define CNTR	REG(USB_BASE_ADDR + 0x40)   //!<Control Register                         */
#define ISTR	REG(USB_BASE_ADDR + 0x44)   //!<Interrupt Status Register                */
#define DADDR	REG(USB_BASE_ADDR + 0x4C)   //!<Device Address Register                  */
#define BTABLE	REG(USB_BASE_ADDR + 0x50)   //!<Buffer Table Address Register            */


#define ISTR_CTR    (0x8000) /* Correct TRansfer (clear-only bit) */
#define ISTR_DOVR   (0x4000) /* DMA OVeR/underrun (clear-only bit) */
#define ISTR_ERR    (0x2000) /* ERRor (clear-only bit) */
#define ISTR_WKUP   (0x1000) /* WaKe UP (clear-only bit) */
#define ISTR_SUSP   (0x0800) /* SUSPend (clear-only bit) */
#define ISTR_RESET  (0x0400) /* RESET (clear-only bit) */
#define ISTR_SOF    (0x0200) /* Start Of Frame (clear-only bit) */
#define ISTR_ESOF   (0x0100) /* Expected Start Of Frame (clear-only bit) */


#define ISTR_DIR    (0x0010)  /* DIRection of transaction (read-only bit)  */
#define ISTR_EP_ID  (0x000F)  /* EndPoint IDentifier (read-only bit)  */

#define ISTR_NOT_RESET	0xFB00	//биты с 8 по 15 кроме ресета, чтобы всё в одном прерывании сбросить (мы даже не будем его проверять)

#define CLR_CTR    (~ISTR_CTR)   /* clear Correct TRansfer bit */
#define CLR_DOVR   (~ISTR_DOVR)  /* clear DMA OVeR/underrun bit*/
#define CLR_ERR    (~ISTR_ERR)   /* clear ERRor bit */
#define CLR_WKUP   (~ISTR_WKUP)  /* clear WaKe UP bit     */
#define CLR_SUSP   (~ISTR_SUSP)  /* clear SUSPend bit     */
#define CLR_RESET  (~ISTR_RESET) /* clear RESET bit      */
#define CLR_SOF    (~ISTR_SOF)   /* clear Start Of Frame bit   */
#define CLR_ESOF   (~ISTR_ESOF)  /* clear Expected Start Of Frame bit */


/* Endpoint Addresses (w/direction) */
#define EP0_OUT     ((uint8_t)0x00)
#define EP0_IN      ((uint8_t)0x80)
#define EP1_OUT     ((uint8_t)0x01)
#define EP1_IN      ((uint8_t)0x81)
#define EP2_OUT     ((uint8_t)0x02)
#define EP2_IN      ((uint8_t)0x82)
#define EP3_OUT     ((uint8_t)0x03)
#define EP3_IN      ((uint8_t)0x83)
#define EP4_OUT     ((uint8_t)0x04)
#define EP4_IN      ((uint8_t)0x84)
#define EP5_OUT     ((uint8_t)0x05)
#define EP5_IN      ((uint8_t)0x85)
#define EP6_OUT     ((uint8_t)0x06)
#define EP6_IN      ((uint8_t)0x86)
#define EP7_OUT     ((uint8_t)0x07)
#define EP7_IN      ((uint8_t)0x87)

/* endpoints enumeration */
#define ENDP0       ((uint8_t)0)
#define ENDP1       ((uint8_t)1)
#define ENDP2       ((uint8_t)2)
#define ENDP3       ((uint8_t)3)
#define ENDP4       ((uint8_t)4)
#define ENDP5       ((uint8_t)5)
#define ENDP6       ((uint8_t)6)
#define ENDP7       ((uint8_t)7)


/******************************************************************************/
/*             CNTR control register bits definitions                         */
/******************************************************************************/
#define CNTR_CTRM   (0x8000) /* Correct TRansfer Mask */
#define CNTR_DOVRM  (0x4000) /* DMA OVeR/underrun Mask */
#define CNTR_ERRM   (0x2000) /* ERRor Mask */
#define CNTR_WKUPM  (0x1000) /* WaKe UP Mask */
#define CNTR_SUSPM  (0x0800) /* SUSPend Mask */
#define CNTR_RESETM (0x0400) /* RESET Mask   */
#define CNTR_SOFM   (0x0200) /* Start Of Frame Mask */
#define CNTR_ESOFM  (0x0100) /* Expected Start Of Frame Mask */


#define CNTR_RESUME (0x0010) /* RESUME request */
#define CNTR_FSUSP  (0x0008) /* Force SUSPend */
#define CNTR_LPMODE (0x0004) /* Low-power MODE */
#define CNTR_PDWN   (0x0002) /* Power DoWN */
#define CNTR_FRES   (0x0001) /* Force USB RESet */

/******************************************************************************/
/*                FNR Frame Number Register bit definitions                   */
/******************************************************************************/
#define FNR_RXDP (0x8000) /* status of D+ data line */
#define FNR_RXDM (0x4000) /* status of D- data line */
#define FNR_LCK  (0x2000) /* LoCKed */
#define FNR_LSOF (0x1800) /* Lost SOF */
#define FNR_FN  (0x07FF) /* Frame Number */
/******************************************************************************/
/*               DADDR Device ADDRess bit definitions                         */
/******************************************************************************/
#define DADDR_EF (0x80)
#define DADDR_ADD (0x7F)
/******************************************************************************/
/*                            Endpoint register                               */
/******************************************************************************/
/* bit positions */
#define EP_CTR_RX      (0x8000) /* EndPoint Correct TRansfer RX */
#define EP_DTOG_RX     (0x4000) /* EndPoint Data TOGGLE RX */
#define EPRX_STAT      (0x3000) /* EndPoint RX STATus bit field */
#define EP_SETUP       (0x0800) /* EndPoint SETUP */
#define EP_T_FIELD     (0x0600) /* EndPoint TYPE */
#define EP_KIND        (0x0100) /* EndPoint KIND */
#define EP_CTR_TX      (0x0080) /* EndPoint Correct TRansfer TX */
#define EP_DTOG_TX     (0x0040) /* EndPoint Data TOGGLE TX */
#define EPTX_STAT      (0x0030) /* EndPoint TX STATus bit field */
#define EPADDR_FIELD   (0x000F) /* EndPoint ADDRess FIELD */

/* EndPoint REGister MASK (no toggle fields) */
//#define EPREG_MASK     (EP_CTR_RX|EP_SETUP|EP_T_FIELD|EP_KIND|EP_CTR_TX|EPADDR_FIELD)
#define EPREG_MASK     (0x8F8F)

/* EP_TYPE[1:0] EndPoint TYPE */
#define EP_TYPE_MASK   (0x0600) /* EndPoint TYPE Mask */
#define EP_BULK        (0x0000) /* EndPoint BULK */
#define EP_CONTROL     (0x0200) /* EndPoint CONTROL */
#define EP_ISOCHRONOUS (0x0400) /* EndPoint ISOCHRONOUS */
#define EP_INTERRUPT   (0x0600) /* EndPoint INTERRUPT */
//#define EP_T_MASK      (~EP_T_FIELD & EPREG_MASK)
#define EP_T_MASK      (0x898F)


/* EP_KIND EndPoint KIND */
//#define EPKIND_MASK    (~EP_KIND & EPREG_MASK)
#define EPKIND_MASK    (0x8E8F)


/* STAT_TX[1:0] STATus for TX transfer */
#define EP_TX_DIS      (0x0000) /* EndPoint TX DISabled */
#define EP_TX_STALL    (0x0010) /* EndPoint TX STALLed */
#define EP_TX_NAK      (0x0020) /* EndPoint TX NAKed */
#define EP_TX_VALID    (0x0030) /* EndPoint TX VALID */
#define EPTX_DTOG1     (0x0010) /* EndPoint TX Data TOGgle bit1 */
#define EPTX_DTOG2     (0x0020) /* EndPoint TX Data TOGgle bit2 */
//#define EPTX_DTOGMASK  (EPTX_STAT|EPREG_MASK)
#define EPTX_DTOGMASK  (0x8FBF)

/* STAT_RX[1:0] STATus for RX transfer */
#define EP_RX_DIS      (0x0000) /* EndPoint RX DISabled */
#define EP_RX_STALL    (0x1000) /* EndPoint RX STALLed */
#define EP_RX_NAK      (0x2000) /* EndPoint RX NAKed */
#define EP_RX_VALID    (0x3000) /* EndPoint RX VALID */
#define EPRX_DTOG1     (0x1000) /* EndPoint RX Data TOGgle bit1 */
#define EPRX_DTOG2     (0x2000) /* EndPoint RX Data TOGgle bit1 */
//#define EPRX_DTOGMASK  (EPRX_STAT|EPREG_MASK)
#define EPRX_DTOGMASK  (0xBF8F)



#define  USB_REQ_GET_STATUS  			0
#define  USB_REQ_CLEAR_FEATURE 			1
#define  USB_REQ_RESERVED1				2
#define  USB_REQ_SET_FEATURE			3
#define  USB_REQ_RESERVED2				4
#define  USB_REQ_SET_ADDRESS			5
#define  USB_REQ_GET_DESCRIPTOR			6
#define  USB_REQ_SET_DESCRIPTOR			7
#define  USB_REQ_GET_CONFIGURATION		8
#define  USB_REQ_SET_CONFIGURATION		9
#define  USB_REQ_GET_INTERFACE			a
#define  USB_REQ_SET_INTERFACE			b
#define  USB_REQ_TOTAL_sREQUEST  		c
#define  USB_REQ_SYNCH_FRAME 	 		12

#define USB_EP0_MAX_PACKET_SIZE			0x40
#define USB_EP1_MAX_PACKET_SIZE_TX		0x40
#define USB_EP2_MAX_PACKET_SIZE_TX		0x08//CDC - 0x08, MSC - 0x40
#define USB_EP2_MAX_PACKET_SIZE_RX		0x40//CDC - 0x08, MSC - 0x40
#define USB_EP3_MAX_PACKET_SIZE_RX		0x40

#define USB_EP3_RECEIVE_TIMEOUT			1000


typedef enum _CONTROL_STATE
{
  WAIT_SETUP,       /* 0 */
  SETTING_UP,       /* 1 */
  IN_DATA,          /* 2 */
  OUT_DATA,         /* 3 */
  LAST_IN_DATA,     /* 4 */
  LAST_OUT_DATA,    /* 5 */
  WAIT_STATUS_IN,   /* 7 */
  WAIT_STATUS_OUT,  /* 8 */
  STALLED,          /* 9 */
  PAUSE             /* 10 */
} CONTROL_STATE;    /* The state machine states of a control pipe */



//*-----------------------------------------------------------------------------------------------
//*			Функции модуля
//*-----------------------------------------------------------------------------------------------

void USB_com_Init();
void USB_RESET(void);
void TO_WRITE_PMA(uint16_t *P1, uint32_t P2, uint16_t N, uint16_t max);
void TO_READ_PMA(uint32_t P1, uint16_t *P2, uint16_t N);
void USB_Message (char Str[], uint8_t EP);
void USB_main_COM_react();
void Check_If_USB_Is_AFK();
//void USB_CTR_RX();
