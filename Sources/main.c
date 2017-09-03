#include "includes.h"    

#pragma push

/* this variable definition is to demonstrate how to share data between XGATE and S12X */
#pragma DATA_SEG SHARED_DATA
volatile int shared_counter; /* volatile because both cores are accessing it. */

/* Two stacks in XGATE core3 */ 
#pragma DATA_SEG XGATE_STK_L
word XGATE_STACK_L[1]; 
#pragma DATA_SEG XGATE_STK_H
word XGATE_STACK_H[1];

#pragma pop

#define ROUTE_INTERRUPT(vec_adr, cfdata)                \
  INT_CFADDR= (vec_adr) & 0xF0;                         \
  INT_CFDATA_ARR[((vec_adr) & 0x0F) >> 1]= (cfdata)

#define SOFTWARETRIGGER0_VEC  0x72 /* vector address= 2 * channel id */


static  OS_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];

static  void    AppTaskStart(void *p_arg);

#if (uC_PROBE_OS_PLUGIN > 0) || (uC_PROBE_COM_MODULE > 0)
extern  void    AppProbeInit(void);
#endif



static void SetupXGATE(void) {
  /* initialize the XGATE vector block and
     set the XGVBR register to its start address */
  XGVBR= (unsigned int)(void*__far)(XGATE_VectorTable - XGATE_VECTOR_OFFSET);

  /* switch software trigger 0 interrupt to XGATE */
  ROUTE_INTERRUPT(SOFTWARETRIGGER0_VEC, 0x81); /* RQST=1 and PRIO=1 */

  /* when changing your derivative to non-core3 one please remove next five lines */
  XGISPSEL= 1;
  XGISP31= (unsigned int)(void*__far)(XGATE_STACK_L + 1);
  XGISPSEL= 2;
  XGISP74= (unsigned int)(void*__far)(XGATE_STACK_H + 1);
  XGISPSEL= 0;

  /* enable XGATE mode and interrupts */
  XGMCTL= 0xFBC1; /* XGE | XGFRZ | XGIE */

  /* force execution of software trigger 0 handler */
  XGSWT= 0x0101;
}





void main(void) {

  /* put your own code here */
  CPU_INT08U  err;
    
  SetupXGATE();
  BSP_IntDisAll();                                                    /* Disable ALL interrupts to the interrupt controller       */

  OSInit();                                                           /* Initialize uC/OS-II                                      */

                                                                      /* Create start task                                        */
  OSTaskCreateExt(AppTaskStart,
                  NULL,
                  (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1],
                  APP_TASK_START_PRIO,
                  APP_TASK_START_PRIO,
                  (OS_STK *)&AppTaskStartStk[0],
                  APP_TASK_START_STK_SIZE,
                  NULL,
                  OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

                                                                        /* Assign names to created tasks                            */
#if OS_TASK_NAME_SIZE > 11
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_CHAR *)"Start Task", &err);
#endif

    OSStart();                                                          /* Start uC/OS-II                                           */
}

static  void  AppTaskStart (void *p_arg)
{
    CPU_INT08U  i;

    (void)p_arg;                                                        /* Prevent compiler warning                                 */

    BSP_Init();                                                         /* Initialize the BSP                                       */

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                                       /* Start stats task                                         */
#endif

#if (uC_PROBE_OS_PLUGIN > 0) || (uC_PROBE_COM_MODULE > 0)
    AppProbeInit();                                                     /* Initialize uC/Probe modules                              */
#endif
    
                                                            /* Turn on ALL the LEDs                                     */
    All_LED_Off();
    OSTimeDlyHMSM(0, 0, 0, 1000);
    
    while (DEF_TRUE) {                                                   /* Task body, always written as an infinite loop.           */
        for (i = 0; i < 8; i++) {
            //LED_On(i);
            LED_On(i);
            OSTimeDlyHMSM(0, 0, 0, 100);
            
        }
        
        for (i = 8; i > 0; i--) {
            LED_Off(i-1);
            OSTimeDlyHMSM(0, 0, 0, 100);
            //LED_Off(i);
        }
    }
}


