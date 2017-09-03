/*
*********************************************************************************************************
*                                    APPLICATION SPECIFIC RTOS SETUP
*
*                             (c) Copyright 2005, Micrium, Inc., Weston, FL
*                                          All Rights Reserved
*
*                                          CONFIGURATION FILE
*
* File : app_cfg.h
* By   : Eric Shufro
*********************************************************************************************************
*/

#ifndef  APP_CFG_H
#define  APP_CFG_H


/*
*********************************************************************************************************
*                                       INCLUDES
*********************************************************************************************************
*/

#include "lib_def.h"


/*
*********************************************************************************************************
*                                      APPLICATION CONFIGURATION
*
* Note(s) : (1) Configure the product's/application's desired product configuration values.
*********************************************************************************************************
*/

#define  LIB_STR_CFG_FP_EN              DEF_ENABLED
#define  uC_PROBE_OS_PLUGIN             DEF_ENABLED                  
#define  uC_PROBE_COM_MODULE            DEF_ENABLED


/*$PAGE*/
/*
*********************************************************************************************************
*                                            TASKS PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                   1
#define  OS_PROBE_TASK_PRIO                    4
#define  PROBE_COMM_RS232_PRIO_RESERVED        9                        /* See probe_com_cfg.h, for Probe Parse Task priority       */
#define  OS_TASK_TMR_PRIO                     18


/*
*********************************************************************************************************
*                                              STACK SIZES
*                            Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE             512
#define  OS_PROBE_TASK_STK_SIZE              256


/*
*********************************************************************************************************
*                                       uC/Probe plug-in for uC/OS-II CONFIGURATION
*
* Note(s):  1) Changing OS_PROBE_TIMER_SEL will not change the timer that uC/Probe uses to make time
*              measurements. In order to change the timer used by uC/Probe, modifications to
*              OSProbe_TmrInit() within bsp.c will need to be made.
*********************************************************************************************************
*/

#define  OS_PROBE_TASK                         1                        /* Task will be created for uC/Probe OS Plug-In             */
#define  OS_PROBE_TMR_32_BITS                  0                        /* Timer is 32 bits                                         */
#define  OS_PROBE_HOOKS_EN                     1                        /* Enable uC/Probe application hooks                        */
#define  OS_PROBE_USE_FP                       1                        /* Override uC/Probe floating point support, use integers   */
#define  OS_PROBE_TASK_ID               OS_PROBE_TASK_PRIO              /* Current version of uC/OS-II does not use ID field        */


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION*                            
*********************************************************************************************************
*/

#define  TRACE_LEVEL_OFF                       0
#define  TRACE_LEVEL_INFO                      1
#define  TRACE_LEVEL_DEBUG                     2

#define  APP_TRACE_LEVEL                TRACE_LEVEL_DEBUG
#define  APP_TRACE                        printf

#define  APP_TRACE_INFO(x)            ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DEBUG(x)           ((APP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG) ? (void)(APP_TRACE x) : (void)0)

#endif                                                                  /* End of file                                              */




