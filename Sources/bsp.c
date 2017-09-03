/*
*********************************************************************************************************
*  应用于龙丘综合开发平台V3.0 
*  Designed by Chiu Sir
*  E-mail:chiusir@163.com
*  软件版本:V1.1
*  kernal:uC/OS-II V2.86                                      
*  target:MC9S12XEP100                                      
*  crystal:16M 
*  PLL:32M
*  最后更新:2009年9月30日
*  相关信息参考下列地址:
*  网站:  http://www.lqist.cn
*  淘宝店:http://shop36265907.taobao.com
*********************************************************************************************************
*/

#include "includes.h"


/*
*********************************************************************************************************
*                                        CONSTANTS
*********************************************************************************************************
*/

#define  SOFTWARETRIGGER0_VEC  0x72                             /* Vector address = (2 * channel id)                    */

/*
*********************************************************************************************************
*                                        MACROS
*********************************************************************************************************
*/
         
#define ROUTE_INTERRUPT(vec_adr, cfdata) {                                                          \
                                              INT_CFADDR = (vec_adr) & 0xF0;                        \
                                              INT_CFDATA_ARR[((vec_adr) & 0x0F) >> 1] = (cfdata);   \
                                         }

/*
*********************************************************************************************************
*                                        PROTOTYPES
*********************************************************************************************************
*/

static  void  XGATE_Init(void);                                 /* Initialize the XGATE co-processor                    */
static  void  PLL_Init(void);                                   /* Initialize the CPU's PLL                             */
static  void  OSTickISR_Init(void);                             /* Initialize the OS Ticker                             */
static  void  LED_Init(void);                                   /* Initialize the LED hardware                          */

/*
*********************************************************************************************************
*                                        DATATYPES
*
* Notes : 1) The CPU_ISR_FNCT_PTR data type is used to declare pointers to ISR functions in 
*            a more simplistic manner. It is used when setting the Vector Table in BSP_VectSet()
*********************************************************************************************************
*/

typedef  void  near  (*CPU_ISR_FNCT_PTR)(void *); 

/*
*********************************************************************************************************
*                                        GLOBALS
*********************************************************************************************************
*/

static  CPU_INT16U  OSTickCnts;

/*
*********************************************************************************************************
*                                        BSP_Init()
*
* Description: Initialize BSP, called from app.c instead of calling all of the internal bsp init functions
*********************************************************************************************************
*/

void BSP_Init (void)
{
    XGATE_Init();                                               /* Initialize the XGATE co-processor                    */
    
    PLL_Init();                                                 /* Initialize the PLL.                                  */ 
    BSP_SetECT_Prescaler(4);                                    /* Set ECT prescaler to prevent OSTickCnts overrflow    */    
   
    OSTickISR_Init();
    LED_Init();   
}

/*
*********************************************************************************************************
*                                        XGATE_Init()
*
* Description: XGATE co-processor initialization.
*********************************************************************************************************
*/

void  XGATE_Init (void) 
{
                                                                /* Initialize the XGATE vector block and set the   ...  */
                                                                /* ... XGVBR register to it's start address             */
    XGVBR  = (unsigned int)(void*__far)(XGATE_VectorTable - XGATE_VECTOR_OFFSET);

                                                                /* Switch software trigger 0 interrupt to XGATE  ...    */
                                                                /* ... RQST=1 and PRIO=1                                */
    ROUTE_INTERRUPT(SOFTWARETRIGGER0_VEC, 0x81); 
  
    XGMCTL = 0xFBC1;                                            /* Enable XGATE mode and XGE, XGFRZ, and XGIE Int's.    */  
    XGSWT  = 0x0101;                                            /* Force execution of software trigger 0 handler        */
}

/*
*********************************************************************************************************
*                                        PLL_Init()
*
* Description: Initialize on-chip PLL. CPU clock = 32MHz, Bus clock = 16MHz.
*
* Arguments  : none.
*
* Returns    : none.
*
* Note(s)    : (1) This function assumes the presence of an onboard 16MHz crystal used as the 
*                  Fref oscillator frequency.
*********************************************************************************************************
*/

static void PLL_Init (void)
{  
    CLKSEL=0X00;				// disengage PLL to system
    PLLCTL_PLLON=1;			// turn on PLL
    SYNR=0x00 | 0x01; 	// VCOFRQ[7:6];SYNDIV[5:0]
                        // fVCO= 2*fOSC*(SYNDIV + 1)/(REFDIV + 1)
                        // fPLL= fVCO/(2 × POSTDIV) 
                        // fBUS= fPLL/2 
                        // VCOCLK Frequency Ranges  VCOFRQ[7:6]
                        // 32MHz <= fVCO <= 48MHz    00
                        // 48MHz <  fVCO <= 80MHz    01
                        // Reserved                  10
                        // 80MHz <  fVCO <= 120MHz   11				
    REFDV=0x80 | 0x01;  // REFFRQ[7:6];REFDIV[5:0]
                        // fREF=fOSC/(REFDIV + 1)
                        // REFCLK Frequency Ranges  REFFRQ[7:6]
                        // 1MHz <= fREF <=  2MHz       00
                        // 2MHz <  fREF <=  6MHz       01
                        // 6MHz <  fREF <= 12MHz       10
                        // fREF >  12MHz               11                         
                        // pllclock=2*osc*(1+SYNR)/(1+REFDV)=32MHz;
    POSTDIV=0x00;       // 4:0, fPLL= fVCO/(2xPOSTDIV)
                        // If POSTDIV = $00 then fPLL is identical to fVCO (divide by one).
    _asm(nop);          // BUS CLOCK=16M
    _asm(nop);
    while(!(CRGFLG_LOCK==1));	  //when pll is steady ,then use it;
    CLKSEL_PLLSEL =1;		        //engage PLL to system;                                 */
}


/*
*********************************************************************************************************
*                                        BSP_CPU_ClkFreq()
*
* Description : Returns the CPU operating frequency in Hz. (This is NOT the CPU BUS frequency)
*               However, the bus frequency is (clk_frq / 2)
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    /*
    CPU_INT32U  clk_freq;
    CPU_INT08U  mul;
    CPU_INT08U  div;
    
    
    mul = (SYNR  & 0x3F) + 1;
    div = (REFDV & 0x3F) + 1; 
    
    if ((CLKSEL & CLKSEL_PLLSEL_MASK) > 0) {
        clk_freq =  ((OSCFREQ * 2) * mul) / div;
    } else {
        clk_freq =    OSCFREQ;
    */
        
    return  (32000000);
}

/*
*********************************************************************************************************
*                                        BSP_IntDisAll()
*
* Description : Disable global interrupts.
*********************************************************************************************************
*/

void  BSP_IntDisAll (void) 
{
    CPU_SR  cpu_sr;
    
    
    CPU_CRITICAL_ENTER();    
}

/*
*********************************************************************************************************
*                                        LED_Init()
*
* Description : Initialize LED support hardware.
*
* Arguments   : none.
*********************************************************************************************************
*/

static void LED_Init (void) 
{  
    DDRB |= 0xFF;
}

/*
*********************************************************************************************************
*                                        LED_Toggle()
*
* Description : These functions are included to encapsulate the control of LEDs.
*
* Arguments   : led    0 = Toggle all LEDs
*                      1 = Toggle LED 1
*                      2 = Toggle LED 2
*                      3 = Toggle LED 3
*                      4 = Toggle LED 4
*********************************************************************************************************
*/

void LED_Toggle (CPU_INT08U led)
{
    switch (led) {
        case 0:
             PORTB ^= 0x0F;
             break;

        case 1:
             PORTB ^= 0x01;
             break;
             
        case 2:
             PORTB ^= 0x02;
             break;
             
        case 3:
             PORTB ^= 0x04;
             break;
             
        case 4:
             PORTB ^= 0x08;
             break;                                       
        case 5:
             PORTB ^= 0x10;
             break;
             
        case 6:
             PORTB ^= 0x20;
             break;
             
        case 7:
             PORTB ^= 0x40;
             break;
             
        case 8:
             PORTB ^= 0x80;
             break;
        default:
             break;
    }
}

/*
*********************************************************************************************************
*                                        LED_Off()
*
* Description : Shut off an onboard LED.
*
* Arguments   : led    0 = Toggle all LEDs
*                      1 = Toggle LED 1
*                      2 = Toggle LED 2
*                      3 = Toggle LED 3
*                      4 = Toggle LED 4
*********************************************************************************************************
*/

void  LED_Off (CPU_INT08U led)
{
    switch (led) {
        case 0:
             PORTB &= ~0x0F;
             break;

        case 1:
             PORTB &= ~0x01;
             break;
             
        case 2:
             PORTB &= ~0x02;
             break;
             
        case 3:
             PORTB &= ~0x04;
             break;
             
        case 4:
             PORTB &= ~0x08;
             break; 
              
        case 5:
             PORTB &= ~0x10;
             break;

        case 6:
             PORTB &= ~0x20;
             break;
             
        case 7:
             PORTB &= ~0x40;
             break;
             
        case 8:
             PORTB &= ~0x80;
             break;             
        
        default:
             break;                                                               
    }
}

/*
*********************************************************************************************************
*                                        LED_On()
*
* Description : Turn on an onboard LED.
*
* Arguments   : led    0 = Toggle all LEDs
*                      1 = Toggle LED 1
*                      2 = Toggle LED 2
*                      3 = Toggle LED 3
*                      4 = Toggle LED 4
*********************************************************************************************************
*/

void  LED_On (CPU_INT08U led)
{
    switch (led) {
         case 0:
             PORTB |= 0x0F;
             break;

        case 1:
             PORTB |= 0x01;
             break;
             
        case 2:
             PORTB |= 0x02;
             break;
             
        case 3:
             PORTB |= 0x04;
             break;
             
        case 4:
             PORTB |= 0x10;
             break;
              
        case 5:
             PORTB |= 0x20;
             break;
             
        case 6:
             PORTB |= 0x40;
             break; 
             
        case 7:
             PORTB |= 0x80;
             break;            
             
        default:
             break;                                                  
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                     uC/OS-II TICK ISR INITIALIZATION
*
* Description : This function is used to initialize one of the eight output compares to generate an
*               interrupt at the desired tick rate.  You must decide which output compare you will be
*               using by setting the configuration variable OS_TICK_OC (see OS_CFG.H and also OS_CPU_A.S) 
*               to 0..7 depending on which output compare to use.
*                   OS_TICK_OC set to 4 chooses output compare #4 as the ticker source
*                   OS_TICK_OC set to 5 chooses output compare #5 as the ticker source
*                   OS_TICK_OC set to 6 chooses output compare #6 as the ticker source
*                   OS_TICK_OC set to 7 chooses output compare #7 as the ticker source
* Arguments   : none
* Notes       : 1) It is assumed that you have set the prescaler rate of the free running timer within
*                  the first 64 E clock cycles of the 68HC12.
*               2) CPU registers are define in IO.H (see COSMIC compiler) and in OS_CPU_A.S.
*********************************************************************************************************
*/

static void  OSTickISR_Init (void)
{
    CPU_INT32U  cpu_frq;
    CPU_INT32U  bus_frq;
    CPU_INT08U  ect_prescaler;    


    cpu_frq = BSP_CPU_ClkFreq();            /* Get the current CPU frequency                           */
    bus_frq = cpu_frq / 2;                  /* Derive the BUS frequency from the CPU frequency         */
      
    ect_prescaler = ECT_TSCR2 & 0x07;       /* Get the prescaler value in the control register         */
    
    ect_prescaler = (1 << ect_prescaler);   /* Calculate the correct prescaler value from the reg val  */
    
                                            /* Calculate the nbr of ticks for the interrupt period     */ 
    OSTickCnts    = (CPU_INT16U)((bus_frq / (ect_prescaler * OS_TICKS_PER_SEC)) - 1); 

#if OS_TICK_OC == 4
    ECT_TIOS  |= 0x10;                      /* Make channel an output compare                          */
    ECT_TC4    = ECT_TCNT + OSTickCnts;     /* Set TC4 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE   |= 0x10;                      /* Enable OC4 interrupt.                                   */
#endif

#if OS_TICK_OC == 5
    ECT_TIOS  |= 0x20;                      /* Make channel an output compare                          */
    ECT_TC5    = ECT_TCNT + OSTickCnts;     /* Set TC5 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE   |= 0x20;                      /* Enable OC5 interrupt.                                   */
#endif

#if OS_TICK_OC == 6
    ECT_TIOS |= 0x40;                       /* Make channel an output compare                          */
    ECT_TC6   = ECT_TCNT + OSTickCnts;      /* Set TC6 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE  |= 0x40;                       /* Enable OC6 interrupt.                                   */
#endif

#if OS_TICK_OC == 7
    ECT_TIOS |= 0x80;                       /* Make channel an output compare                          */
    ECT_TC7   = ECT_TCNT + OSTickCnts;      /* Set TC7 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE  |= 0x80;                       /* Enable OC7 interrupt.                                   */
#endif

    ECT_TSCR1 = 0xC0;                       /* Enable counter & disable counter in background mode     */
}


/*
*********************************************************************************************************
*                                      uC/OS-II TICK ISR HANDLER
*
* Description : This function is called by OSTickISR() when a tick interrupt occurs.
*
* Arguments   : none
*********************************************************************************************************
*/

void  OSTickISR_Handler (void)
{
#if OS_TICK_OC == 4
    ECT_TFLG1 |= 0x10;                      /* Clear interrupt                                         */
    ECT_TC4   += OSTickCnts;                /* Set TC4 to present time + OS_TICK_OC_CNTS               */
#endif

#if OS_TICK_OC == 5
    ECT_TFLG1 |= 0x20;                      /* Clear interrupt                                         */
    ECT_TC5   += OSTickCnts;                /* Set TC5 to present time + OS_TICK_OC_CNTS               */
#endif

#if OS_TICK_OC == 6
    ECT_TFLG1 |= 0x40;                      /* Clear interrupt                                         */
    ECT_TC6   += OSTickCnts;                /* Set TC6 to present time + OS_TICK_OC_CNTS               */
#endif

#if OS_TICK_OC == 7
    ECT_TFLG1 |= 0x80;                      /* Clear interrupt                                         */
    ECT_TC7   += OSTickCnts;                /* Set TC7 to present time + OS_TICK_OC_CNTS               */
#endif

    OSTimeTick();                           /* Notify uC/OS-II that a tick has occurred                */
}

/*
*********************************************************************************************************
*                                 Set the ECT Prescaler
*
* Description : This function configures the ECT prescaler during SYSTEM initialization.
*
* Callers     : BSP_Init()
*
* Notes       : This function should be called during system init, ideally fro BSP_Init().
*               Changing the Prescaler during run-time could impact several modules. Be
*               sure to use extreme caution when calling this function.
*********************************************************************************************************
*/

static  void  BSP_SetECT_Prescaler (CPU_INT08U prescaler)
{
    ECT_TSCR2 &= ~ECT_TSCR2_PR_MASK;       /* Clear all prescaler bits                                 */
    
    switch (prescaler) {
        case 1:                            /* Set a prescaler of 1                                     */     
             ECT_TSCR2 &= ~ECT_TSCR2_PR_MASK;  
             break;

        case 2:
             ECT_TSCR2 |= 0x01;            /* Set a prescaler of 2                                     */        
             break;

        case 4:
             ECT_TSCR2 |= 0x02;            /* Set a prescaler of 4                                     */        
             break;

        case 8:
             ECT_TSCR2 |= 0x03;            /* Set a prescaler of 8                                     */        
             break;

        case 16:
             ECT_TSCR2 |= 0x04;            /* Set a prescaler of 16                                    */        
             break;

        case 32:
             ECT_TSCR2 |= 0x05;            /* Set a prescaler of 32                                    */        
             break;

        case 64:
             ECT_TSCR2 |= 0x06;            /* Set a prescaler of 64                                    */        
             break;

        case 128:
             ECT_TSCR2 |= 0x07;            /* Set a prescaler of 128                                   */        
             break;

        default:
             ECT_TSCR2 |= 0x02;            /* Set a prescaler of 4 if the passed value is invalid      */        
             break;                          
    }
}


/*
*********************************************************************************************************
*                                   OSProbe_TmrInit()
*
* Description : This function is called to by uC/Probe Plug-In for uC/OS-II to initialize the
*               free running timer that is used to make time measurements.
*
* Arguments   : none
*
* Returns     : none
*
* Note(s)     : (1) This function has intentionally been left empty. uC/Probe may be configured to
*                   share a timer with uC/OS-II on the MC9S12X platform.
*********************************************************************************************************
*/

#if (uC_PROBE_OS_PLUGIN > 0) && (OS_PROBE_HOOKS_EN == 1)
void  OSProbe_TmrInit (void)
{
}
#endif 

/*
*********************************************************************************************************
*                              READ TIMER FOR uC/Probe Plug-In for uC/OS-II
*
* Description : This function is called to read the current counts of a 16 bit free running timer.
*
* Arguments   : none
*
* Returns     ; The 16 bit count (in a 32 bit variable) of the timer assuming the timer is an UP counter.
*********************************************************************************************************
*/

#if (uC_PROBE_OS_PLUGIN > 0) && (OS_PROBE_HOOKS_EN == 1)
CPU_INT32U  OSProbe_TmrRd (void)
{
    return (ECT_TCNT);
}
#endif



