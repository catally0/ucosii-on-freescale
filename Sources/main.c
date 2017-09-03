/*
*********************************************************************************************************
*  
*  Ӧ���������ۺϿ���ƽ̨V3.0 
*  Designed by Chiu Sir
*  E-mail:chiusir@163.com
*  ����汾:V1.1
*  kernal:uC/OS-II V2.86                                      
*  target:MC9S12XEP100                                      
*  crystal:16M 
*  PLL:32M
*  ������:2009��9��30��
*  �����Ϣ�ο����е�ַ:
*  ��վ:  http://www.lqist.cn
*  �Ա���:http://shop36265907.taobao.com
---------------------------------------------------
��A����������P&E����ֱ�����غ�����

��B����LQ_USBDM V1.3����LQ_USBDM_CF V2.0���صķ����Ͳ���:
  1,���뱾P&E����û�д���,����.abs/.s19�ļ�
  2,��ԭ�еĹ��̻����½�һ����򵥵Ĺ���
  3,��hiwave.exe�ļ�
  4,��TBDML HCS12�����˵�ѡ��RESET
  5,��TBDML HCS12�����˵�ѡ��FLASH
  6,�ӵ������ڵ���"select all"
  7,�ӵ������ڵ���"erase"
  8,�ӵ������ڵ���"load",�ҵ�LQXEP100ucos286PE\bin\LQXEPucos.abs/.s19,Ȼ��ȴ��������
  9,����OK�Ϳ�����,�������л��߸�λ,Ӧ�þͿ���������,������LED��ˮ����,�Ƚ�����:)

��˵������ʵ����P&E������,�������Խ���P&E�Ĺ���,Ȼ����HIWAVE����,    

*********************************************************************************************************
*/
      
#include "includes.h"    

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

static  OS_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void    AppTaskStart(void *p_arg);

#if (uC_PROBE_OS_PLUGIN > 0) || (uC_PROBE_COM_MODULE > 0)
extern  void    AppProbeInit(void);
#endif

/*
*********************************************************************************************************
*                                             C ENTRY POINT
*********************************************************************************************************
*/

int  main (void)
{
    CPU_INT08U  err;

  
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

/*$PAGE*/
/*
*********************************************************************************************************
*                                              STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
* Arguments   : p_arg is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*               2) Interrupts are enabled once the task start because the I-bit of the CCR register was
*                  set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/

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
    
    LED_Off(0);                                                         /* Turn on ALL the LEDs                                     */

    while (DEF_TRUE) {                                                  /* Task body, always written as an infinite loop.           */
        for (i = 1; i <= 8; i++) {
            LED_On(i);
            OSTimeDlyHMSM(0, 0, 0, 100);
            LED_Off(i);
        }
        
        for (i = 8; i >=1; i--) {
            LED_On(i);
            OSTimeDlyHMSM(0, 0, 0, 100);
            LED_Off(i);
        }
    }
}

