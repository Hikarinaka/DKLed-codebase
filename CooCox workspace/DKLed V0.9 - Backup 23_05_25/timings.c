#include "includes.h"
#include "timings.h"

#define DISK_TIMEPROC_PERIOD	10

volatile U64 TickCnt = 0;
uint8_t ISRFlag = 0;
U64 WokeUpTick = 0;

//SD spi timers (100 Hz) control stuff
U64 disc_timeproc_repeater = 0;
extern void disk_timerproc(void);
/**
 *******************************************************************************
 * @brief      Initialize OS
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @par Description
 * @details   This function is called to initialize OS.
 *
 * @note      You must call this function first,before any other OS API function
 *
 * @code      There is a example for useage of this function,as follows:
 *        e.g.
 *            ...                   // Your target initial code.
 *
 *            OsInit();             // Initial OS.
 *            CreateTask(...);      // Create tasks.
 *            ...
 *            OsStart();            // Start multitask.
 * @endcode
 *******************************************************************************
 */
void TInitOS(void)
{
    TInitSysTick();                /* Initialize system tick.                  */
    TInitInt();                    /* Initialize PendSV,SVC,SysTick interrupt  */

}


U64 TGetOSTime(void)
{
    return TickCnt;                   /* Get system time(tick)              */
}

/**
 *******************************************************************************
 * @brief      Enter a ISR.
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @par Description
 * @details    This function is called to notify OS when enter to an ISR.
 *
 * @note       When you call API in ISR,you must call CoEnterISR() before your
 *             interrupt handler code,and call CoExitISR() after your handler
 *             code and before exiting from ISR.
 *******************************************************************************
 */
void TEnterISR(void)
{
	ISRFlag +=1;
}


/**
 *******************************************************************************
 * @brief      Exit a ISR.
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @par Description
 * @details    This function is called when exit from a ISR.
 *
 * @note
 *******************************************************************************
 */
void TExitISR(void)
{
	if (ISRFlag) ISRFlag -=1;
	if (ISRFlag == 0) StandardSceduleTaskList();
}


void StandardSceduleTaskList(){
	//repeaters
	if (disc_timeproc_repeater<=TickCnt){
		disc_timeproc_repeater = TickCnt + DISK_TIMEPROC_PERIOD;
		disk_timerproc();
	}
	//functions
	//Check_If_I2C_Is_AFK();
	//Check_If_USB_Is_AFK();
	//Check_If_MainLoop_Is_AFK();
	//ButtonsCheckSchedile();
}

void TTickDelay(U32 ticks){
	WokeUpTick = TickCnt + ticks;
	while (WokeUpTick<TickCnt){}
}

/**
 *******************************************************************************
 * @brief      System tick interrupt handler.
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @par Description
 * @details    This is system tick interrupt headler.
 * @note       CoOS may schedule when exiting this ISR.
 *******************************************************************************
 */
void SysTick_Handler(void)
{
    TickCnt++;                    /* Increment systerm time.                */
    if (ISRFlag == 0) StandardSceduleTaskList();
}
