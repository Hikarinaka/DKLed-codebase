//I2C lib
//#define SoftwareI2C



#define I2C_Clock_Speed	80000 //100000
#define I2C_Freqrange	360 //PCLK1_Freq / I2C_Clock_Speed = 36 000 000 / I2C_Clock_Speed
#define I2C_Halffreqrange	180 //I2C_Freqrange/2
#define I2C_TIMER_PRESC	720 //72 000 000 / 100 000 = 100 KHz		* scale
#define I2C_TIMER_TicksPerStep	2

#define I2C_TX_SIZE		130	//internal adress + 128 + 1 empty
#define I2C_TX_MAX		I2C_TX_SIZE-1
#define I2C_RX_SIZE		8


#define I2C_OUT_LABEL	0xC	//позиция I2C в списке портов ввода-вывода (0, 1 и 2 - это UART, без указания - USB)

#define I2C_SCL_Pin			GPIO_Pin_7	//button 4
#define I2C_SDA_Pin			GPIO_Pin_0	//button 5
#define I2C_pins			GPIO_Pin_0 | GPIO_Pin_7
#define I2C_GPIO_CRL_Mask	0xF000000F
#define I2C_SCL_SetLow		GPIOB->BRR = I2C_SCL_Pin
#define I2C_SDA_SetLow		GPIOB->BRR = I2C_SDA_Pin
#define I2C_SCL_SetFree		GPIOB->BSRR = I2C_SCL_Pin
#define I2C_SDA_SetFree		GPIOB->BSRR = I2C_SDA_Pin
#define I2C_Status_Port		GPIOB->IDR

#define I2C_Event_SCL_IsUp	0x0001 //SCL line was freed by line/slave and got up to 3V
#define I2C_Event_SDA_IsUp	0x0002 //SDA line was set to 1 by slave or got restored
#define I2C_Event_SCL_IsDown	0x0004 //SCL line was forced to 0 by slave
#define I2C_Event_SDA_IsDown	0x0008 //SDA line was set to 0 by slave


#define I2C_Maximum_ACK_waits	25 //amount of timer cycles to wait correct ACK before giving up on communication

#define I2C_Maximum_Wait_ms		3000//timeframe in millisecs to reset I2C after start of operation

/* I2C SPE mask */
#define I2C_CR1_PE_Set              ((uint16_t)0x0001)
#define I2C_CR1_PE_Reset            ((uint16_t)0xFFFE)

/* I2C START mask */
#define I2C_CR1_START_Set           ((uint16_t)0x0100)
#define I2C_CR1_START_Reset         ((uint16_t)0xFEFF)

/* I2C STOP mask */
#define I2C_CR1_STOP_Set            ((uint16_t)0x0200)
#define I2C_CR1_STOP_Reset          ((uint16_t)0xFDFF)

/* I2C ACK mask */
#define I2C_CR1_ACK_Set             ((uint16_t)0x0400)
#define I2C_CR1_ACK_Reset           ((uint16_t)0xFBFF)

/* I2C ENGC mask */
#define I2C_CR1_ENGC_Set            ((uint16_t)0x0040)
#define I2C_CR1_ENGC_Reset          ((uint16_t)0xFFBF)

/* I2C SWRST mask */
#define I2C_CR1_SWRST_Set           ((uint16_t)0x8000)
#define I2C_CR1_SWRST_Reset         ((uint16_t)0x7FFF)

/* I2C PEC mask */
#define I2C_CR1_PEC_Set             ((uint16_t)0x1000)
#define I2C_CR1_PEC_Reset           ((uint16_t)0xEFFF)

/* I2C ENPEC mask */
#define I2C_CR1_ENPEC_Set           ((uint16_t)0x0020)
#define I2C_CR1_ENPEC_Reset         ((uint16_t)0xFFDF)

/* I2C ENARP mask */
#define I2C_CR1_ENARP_Set           ((uint16_t)0x0010)
#define I2C_CR1_ENARP_Reset         ((uint16_t)0xFFEF)

/* I2C NOSTRETCH mask */
#define I2C_CR1_NOSTRETCH_Set       ((uint16_t)0x0080)
#define I2C_CR1_NOSTRETCH_Reset     ((uint16_t)0xFF7F)

/* I2C registers Masks */
#define I2C_CR1_CLEAR_Mask          ((uint16_t)0xFBF5)

/* I2C DMAEN mask */
#define I2C_CR2_DMAEN_Set           ((uint16_t)0x0800)
#define I2C_CR2_DMAEN_Reset         ((uint16_t)0xF7FF)

/* I2C LAST mask */
#define I2C_CR2_LAST_Set            ((uint16_t)0x1000)
#define I2C_CR2_LAST_Reset          ((uint16_t)0xEFFF)

/* I2C FREQ mask */
#define I2C_CR2_FREQ_Reset          ((uint16_t)0xFFC0)

/* I2C ADD0 mask */
#define I2C_OAR1_ADD0_Set           ((uint16_t)0x0001)
#define I2C_OAR1_ADD0_Reset         ((uint16_t)0xFFFE)

/* I2C ENDUAL mask */
#define I2C_OAR2_ENDUAL_Set         ((uint16_t)0x0001)
#define I2C_OAR2_ENDUAL_Reset       ((uint16_t)0xFFFE)

/* I2C ADD2 mask */
#define I2C_OAR2_ADD2_Reset         ((uint16_t)0xFF01)

/* I2C F/S mask */
#define I2C_CCR_FS_Set              ((uint16_t)0x8000)

/* I2C CCR mask */
#define I2C_CCR_CCR_Set             ((uint16_t)0x0FFF)

/* I2C FLAG mask */
#define I2C_FLAG_Mask               ((uint32_t)0x00FFFFFF)

/* I2C Interrupt Enable mask */
#define I2C_ITEN_Mask               ((uint32_t)0x07000000)



//*-----------------------------------------------------------------------------------------------
//*			Функции модуля
//*-----------------------------------------------------------------------------------------------

void I2C1_init(uint8_t address);
void I2C1_DeInit(void);
//void I2C1_Send();
uint8_t I2C1_ForceSend();
void start_I2C(uint8_t dir);
void Check_If_I2C_Is_AFK();
#ifdef SoftwareI2C
void TIM3_IRQHandler(void);
void I2C_preInit();
#else

#endif
