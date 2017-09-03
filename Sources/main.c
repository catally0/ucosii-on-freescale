/*
*********************************************************************************************************
*  
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
---------------------------------------------------
【A】本工程用P&E可以直接下载和运行

【B】用LQ_USBDM V1.3或者LQ_USBDM_CF V2.0下载的方法和步骤:
  1,编译本P&E工程没有错误,生成.abs/.s19文件
  2,打开原有的工程或者新建一个最简单的工程
  3,打开hiwave.exe文件
  4,从TBDML HCS12下来菜单选择RESET
  5,从TBDML HCS12下来菜单选择FLASH
  6,从弹出窗口单击"select all"
  7,从弹出窗口单击"erase"
  8,从弹出窗口单击"load",找到LQXEP100ucos286PE\bin\LQXEPucos.abs/.s19,然后等待下载完成
  9,单击OK就可以了,按下运行或者复位,应该就可以跑起来,现象是LED流水点亮,比较美观:)

【说明】其实不用P&E下载器,照样可以建立P&E的工程,然后用HIWAVE下载,    

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

