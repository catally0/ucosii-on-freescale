/*
*********************************************************************************************************
*                                     uC/Probe uC/OS-II Plug-in
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              uC/Probe
*
*                                         Plug-in for uC/OS-II
*
* Filename      : os_probe.c
* Version       : V1.50
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#define  OS_PROBE_MODULE
#include "os_probe.h"
   

/*
*********************************************************************************************************
*                                             LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  OS_STK  OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE];
#endif


/*
*********************************************************************************************************
*                                        LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  void  OSProbe_InitOS(void);
static  void  OSProbe_Task  (void  *p_arg);
#endif


/*
*********************************************************************************************************
*                                       LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             OSProbe_Init()
*
* Description : Initialize the uC/Probe Plug-In for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  OSProbe_Init (void)
{
#if (OS_PROBE_TASK > 0)
    OSProbe_SetDelay(100);
    OSProbe_SetCallback((void (*)(void))0);                     /* Force terminal callback function to 'nothing'.       */

    (void)OSProbe_TaskCPUUsage;
    (void)OSProbe_TaskStkUsage;

    OSProbe_InitOS();
#endif

#if (OS_PROBE_HOOKS_EN > 0)
    OSProbe_TmrInit();

    OSProbe_CyclesCtr   = 0;
    OSProbe_TmrCntsPrev = 0;
#endif
}


/*
*********************************************************************************************************
*                                            OSProbe_InitOS()
*
* Description : Create the task for the uC/Probe Plug-In for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  void  OSProbe_InitOS (void)
{
    INT8U  err;


#if (OS_TASK_CREATE_EXT_EN > 0)
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreateExt( OSProbe_Task,
                           (void *)0,
                          &OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE - 1], /* Set Top-Of-Stack.                            */
                           OS_PROBE_TASK_PRIO,
                           OS_PROBE_TASK_PRIO,
                          &OSProbe_TaskStk[0],                          /* Set Bottom-Of-Stack.                         */
                           OS_PROBE_TASK_STK_SIZE,
                           (void *)0,                                   /* No TCB extension.                            */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);  /* Enable stack checking + clear stack.         */
    #else
    err = OSTaskCreateExt( OSProbe_Task,
                           (void *)0,
                          &OSProbe_TaskStk[0],                          /* Set Top-Of-Stack.                            */
                           OS_PROBE_TASK_PRIO,
                           OS_PROBE_TASK_PRIO,
                          &OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE - 1], /* Set Bottom-Of-Stack.                         */
                           OS_PROBE_TASK_STK_SIZE,
                           (void *)0,                                   /* No TCB extension.                            */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);  /* Enable stack checking + clear stack.         */
    #endif
#else
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreate( OSProbe_Task,
                        (void *)0,
                       &OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE - 1],
                        OS_PROBE_TASK_PRIO);
    #else
    err = OSTaskCreate( OSProbe_Task,
                        (void *)0,
                       &OSProbe_TaskStk[0],
                        OS_PROBE_TASK_PRIO);
    #endif
#endif

#if   (OS_TASK_NAME_SIZE > 15)
    OSTaskNameSet(OS_PROBE_TASK_PRIO, (INT8U *)"Probe OS PlugIn", &err);
#endif
}
#endif


/*
*********************************************************************************************************
*                                          OSProbe_SetCallback()
*
* Description : Set the callback function which will be invoked in OSProbe_Task().
*
* Argument(s) : call_back   Pointer to the callback function.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
void  OSProbe_SetCallback (void (*call_back)(void))
{
    OSProbe_CallbackFnct = call_back;
}
#endif


/*
*********************************************************************************************************
*                                           OSProbe_SetDelay()
*
* Description : Set the delay used in OSProbe_Task().
*
* Argument(s) : delay       Delay, in milliseconds.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
void    OSProbe_SetDelay (INT16U  delay)
{
    OSProbe_Delay = delay;
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               TASK HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        OSProbe_TaskCreateHook()
*
* Description : This function is called when a task is created.
*
* Argument(s) : ptcb        Pointer to the task control block of the task being created.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*
*               (2) This MUST be called from applications's task create hook function App_TaskCreateHook().
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void  OSProbe_TaskCreateHook (OS_TCB *ptcb)
{
    ptcb->OSTCBCyclesStart = OSProbe_TimeGetCycles();           /* Get the current start time for this task.            */
    ptcb->OSTCBCyclesTot   = 0;                                 /* Update the task's total execution time.              */
}
#endif


/*
*********************************************************************************************************
*                                          OSProbe_TaskSwHook()
*
* Description : This function is called when a task switch is performed.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*
*               (2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                   task being switched out (i.e. the preempted task).
*
*               (3) This MUST be called from application's task switch hook function App_TaskSwHook().
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void  OSProbe_TaskSwHook (void)
{
    INT32U  cycles;


    cycles                         = OSProbe_TimeGetCycles();   /* This task is done.                                   */
    OSTCBCur->OSTCBCyclesTot      += cycles - OSTCBCur->OSTCBCyclesStart;
    OSTCBHighRdy->OSTCBCyclesStart = cycles;                    /* Save absolute #cycles at task activation.            */
}
#endif


/*
*********************************************************************************************************
*                                           OSProbe_TickHook()
*
* Description : This function is called every tick.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*
*               (2) This MUST be called from user's time tick hook function App_TimeTickHook().
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void  OSProbe_TickHook (void)
{
    (void)OSProbe_TimeGetCycles();
}
#endif


/*
*********************************************************************************************************
*                                         OSProbe_TimeGetCycles()
*
* Description : Get time as accurately as possible, stored in a 32-bit variable.
*
* Argument(s) : none.
*
* Return(s)   : A 32-bit representation of time.
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
INT32U   OSProbe_TimeGetCycles(void)
{
    INT32U     cycles;
#if (OS_PROBE_TMR_32_BITS > 0)
    INT32U     cnts32;
    INT32U     cnts32_delta;
#else
    INT16U     cnts16;
    INT16U     cnts16_delta;
#endif
#if (OS_CRITICAL_METHOD == 3)                                   /* Allocate storage for CPU status register.            */
    OS_CPU_SR  cpu_sr = 0;
#endif


    OS_ENTER_CRITICAL();
#if (OS_PROBE_TMR_32_BITS > 0)
    cnts32               = OSProbe_TmrRd();                     /* Read current counts of the free running counter.     */
    cnts32_delta         = cnts32 - OSProbe_TmrCntsPrev;
    OSProbe_TmrCntsPrev  = cnts32;                              /* Save current counts for next time.                   */
    OSProbe_CyclesCtr   += cnts32_delta;
#else
    cnts16               = (INT16U)OSProbe_TmrRd();             /* Read current counts of the free running counter.     */
    cnts16_delta         = cnts16 - OSProbe_TmrCntsPrev;
    OSProbe_TmrCntsPrev  = cnts16;                              /* Save current counts for next time.                   */
    OSProbe_CyclesCtr   += (INT32U)cnts16_delta;
#endif
    cycles               = OSProbe_CyclesCtr;
    OS_EXIT_CRITICAL();

    return (cycles);
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL FUNCTION
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             OSProbe_Task()
*
* Description : Updates OSProbe_TaskStkUsage[] and OSProbe_TaskCPUUsage[] and calls
*               a user-specified callback functions (OSProbe_CallbackFnct()), if the user sets this
*               function.
*
* Argument(s) : p_arg       Argument passed to OSProbe_Task() by 'OSTaskCreate()'.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  void  OSProbe_Task (void *p_arg)
{
            OS_TCB  *ptcb;
            INT16U   i;
            INT32U   cycles_tot;
    static  INT32U   cycles_dif[OS_MAX_TASKS];
    static  INT32U   cycles_tot_last[OS_MAX_TASKS];
#if (OS_PROBE_USE_FP == 0)
            INT32U   max;
#endif


    (void)p_arg;

                                                                /* Initialize stored CyclesTot values.                  */
    for (i = 0; i < OS_MAX_TASKS; i++) {
        cycles_tot_last[i]      = 0;
        OSProbe_TaskStkUsage[i] = 0;
        OSProbe_TaskCPUUsage[i] = 0;
    }

    while (1) {
        OSTimeDlyHMSM(0, 0, 0, OSProbe_Delay);
        if ((INT32U)OSProbe_CallbackFnct != 0) {
            OSProbe_CallbackFnct();
        }

                                                                /* Update task CPU usage                                */
        i          = 0;
        cycles_tot = 0;
        ptcb       = &OSTCBTbl[0];                              /*  ... Get pointer to first TCB ...                    */

        while ((i    < OS_MAX_TASKS) &&
               (ptcb != (OS_TCB *)0) &&
               (ptcb != (OS_TCB *)1)) {
                                                                /*  ... Calculate new CyclesDif, the number of cycles   */
                                                                /*  ... used by the task since the last reading.  Half  */
                                                                /*  ... the previous value is added to provide some     */
                                                                /*  ... hysteresis, thereby reducing the natural        */
                                                                /*  ... "jitter" in the data.                           */
            cycles_dif[i]       = (ptcb->OSTCBCyclesTot - cycles_tot_last[i]) / 2 + (cycles_dif[i] / 2);
            cycles_tot_last[i]  = ptcb->OSTCBCyclesTot;
            cycles_tot         += cycles_dif[i];

            if (ptcb->OSTCBStkSize == 0) {
                OSProbe_TaskStkUsage[i] = 0;
            } else {
#if (OS_PROBE_USE_FP > 0)
#if (OS_STK_GROWTH == 1)
                OSProbe_TaskStkUsage[i] = (FP32)(((INT32U)(ptcb->OSTCBStkBase) - (INT32U)(ptcb->OSTCBStkPtr)) * 100)
                                        / ((ptcb->OSTCBStkSize) * sizeof (OS_STK));
#else
                OSProbe_TaskStkUsage[i] = (FP32)(((INT32U)(ptcb->OSTCBStkPtr) - (INT32U)(ptcb->OSTCBStkBase)) * 100)
                                        / ((ptcb->OSTCBStkSize) * sizeof (OS_STK));
#endif
#else
                max = (ptcb->OSTCBStkSize) * sizeof (OS_STK) / 100L;

#if (OS_STK_GROWTH == 1)
                OSProbe_TaskStkUsage[i] = (INT8U)(((INT32U)(ptcb->OSTCBStkBase) - (INT32U)(ptcb->OSTCBStkPtr)) / max);
#else
                OSProbe_TaskStkUsage[i] = (INT8U)(((INT32U)(ptcb->OSTCBStkPtr)  - (INT32U)(ptcb->OSTCBStkBase)) / max);
#endif
#endif
            }

            ptcb = ptcb->OSTCBPrev;

            i++;
        }

#if (OS_PROBE_USE_FP == 0)
        max = cycles_tot / 100L;
#endif
                                                                /*  ... For each task, calculate percent CPU usage.     */
        for (i = 0; i < OS_MAX_TASKS; i++) {
#if (OS_PROBE_USE_FP > 0)
            OSProbe_TaskCPUUsage[i] = (FP32)(cycles_dif[i] * 100) / cycles_tot;
#else
            OSProbe_TaskCPUUsage[i] = (INT8U)(cycles_dif[i] / max);
#endif
        }
    }
}
#endif
