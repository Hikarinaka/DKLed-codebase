#ifndef TIMINGS_H_
#define TIMINGS_H_

typedef signed   char      S8;
typedef unsigned char      U8;
typedef short              S16;
typedef unsigned short     U16;
typedef int                S32;
typedef unsigned int       U32;
typedef long long          S64;
typedef unsigned long long U64;

/*!<
System frequency (Hz).
*/
#define CFG_CPU_FREQ            (72000000)

/*!<
systick frequency (Hz).
*/
#define CFG_SYSTICK_FREQ        (1000)


#define NVIC_ST_CTRLa    (*((volatile U32 *)0xE000E010))
#define NVIC_ST_RELOADa  (*((volatile U32 *)0xE000E014))
#define RELOAD_VALa      ((U32)(( (U32)CFG_CPU_FREQ) / (U32)CFG_SYSTICK_FREQ) -1)

/*!< Initial System tick.	*/
#define TInitSysTick()   NVIC_ST_RELOADa =  RELOAD_VALa; \
                        NVIC_ST_CTRLa   =  0x0007

#define NVIC_SYS_PRI2a   (*((volatile U32 *)0xE000ED1C))
#define NVIC_SYS_PRI3a   (*((volatile U32 *)0xE000ED20))

/*!< Initialize PendSV,SVC and SysTick interrupt priority to lowest.          */
#define TInitInt()       NVIC_SYS_PRI2a |=  0xFF000000;\
                        NVIC_SYS_PRI3a |=  0xFFFF0000

void TInitOS(void);
U64 TGetOSTime(void);
void TEnterISR(void);
void TExitISR(void);
void TTickDelay(U32 ticks);

#define CoGetOSTime() TGetOSTime()
#define CoEnterISR() TEnterISR()
#define CoExitISR() TExitISR()
#define CoTickDelay(ticks)	TTickDelay(ticks)


#endif /* TIMINGS_H_ */
