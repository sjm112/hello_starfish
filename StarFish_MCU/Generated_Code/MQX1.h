/** ###################################################################
**     THIS COMPONENT MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename    : MQX1.h
**     Project     : ProcessorExpert
**     Processor   : MKL26Z128VLH4
**     Version     : Component 01.098, Driver 01.00, CPU db: 3.00.000
**     Compiler    : Keil ARM C/C++ Compiler
**     Date/Time   : 2013-04-18, 10:22, # CodeGen: 13
**     Abstract    :
**         MQX Lite RTOS Adapter component.
**     Settings    :
**
**     Copyright : 1997 - 2012 Freescale Semiconductor, Inc. All Rights Reserved.
**     
**     http      : www.freescale.com
**     mail      : support@freescale.com
** ###################################################################*/
/*!
** @file MQX1.h
** @version 01.00
** @date 2013-04-18, 10:22, # CodeGen: 13
** @brief
**         MQX Lite RTOS Adapter component.
*/         
/*!
**  @addtogroup MQX1_module MQX1 module documentation
**  @{
*/         

#ifndef __MQX1_H
#define __MQX1_H

/* MODULE MQX1. */

/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited components */
#include "SystemTimer1.h"
#include "task_template_list.h"
/* MQX Lite include files */
#include "mqxlite.h"
#include "mqxlite_prv.h"


/* Path to MQX Lite source files */
#define MQX_PATH   "C:/Program Files/Freescale/PExDrv v10.0/eclipse/ProcessorExpert/lib/mqxlite/V1.0.0/"

void      SystemTimer1_OnCounterRestart(LDD_TUserData *UserDataPtr);
uint32_t  SystemTimer1_GetTicsPerSecond(LDD_TDeviceData *DeviceDataPtr);


/* MQX Lite entrypoint */
void __boot(void);
/* SVC handler - called after SVC instruction */
void _svc_handler(void);
/* PendSV handler - task switch functionality */
void _pend_svc(void);

/* MQX Lite adapter system timer functions */
#define MQXLITE_SYSTEM_TIMER_INIT(param)   \
            SystemTimer1_Init(param)
#define MQXLITE_SYSTEM_TIMER_START(param)  \
            SystemTimer1_Enable(param)
#define MQXLITE_SYSTEM_TIMER_GET_INPUT_FREQUENCY(param)         \
            SystemTimer1_GetInputFrequency(param)
#define MQXLITE_SYSTEM_TIMER_GET_PERIOD_TICKS(param, value)     \
            SystemTimer1_GetPeriodTicks(param, value)
#define MQXLITE_SYSTEM_TIMER_SET_PERIOD_TICKS(param, value)     \
            SystemTimer1_SetPeriodTicks(param, value)
#define MQXLITE_SYSTEM_TIMER_GET_COUNTER_VALUE(param)           \
            SystemTimer1_GetCounterValue(param)
#define MQXLITE_SYSTEM_TIMER_GET_TICKS_PER_SECOND(param)        \
            SystemTimer1_GetTicsPerSecond(param)
#define MQXLITE_SYSTEM_TIMER_SET_HWTICKS_FUNCTION(param)        \
            _time_set_hwtick_function((MQX_GET_HWTICKS_FPTR)SystemTimer1_GetCounterValue, param)



/* Task stacks declarations */
extern uint8_t Init_Task_task_stack[INIT_TASK_TASK_STACK_SIZE];
extern uint8_t shell_task_stack[SHELL_TASK_STACK_SIZE];
/* MQX Lite init structure and task template list */
extern const MQXLITE_INITIALIZATION_STRUCT       MQX_init_struct;
extern const TASK_TEMPLATE_STRUCT                MQX_template_list[];


/* MQX Lite initialization function */
#define PEX_RTOS_INIT()    if (MQX_OK != _mqxlite_init((MQXLITE_INITIALIZATION_STRUCT_PTR)&MQX_init_struct)) while(1)
/* MQX Lite start function */
#define PEX_RTOS_START()    _mqxlite()


/* The first interrupt vector that the application wants to have a 'C' ISR for.    */
#define FIRST_INTERRUPT_VECTOR_USED    (INT_SysTick)
/* The last interrupt vector that the application wants to handle. */
#define LAST_INTERRUPT_VECTOR_USED     (INT_TSI0)
#define MQX_INTERRUPT_TABLE_ITEMS      (LAST_INTERRUPT_VECTOR_USED - FIRST_INTERRUPT_VECTOR_USED + 1)
/* The table of 'C' handlers for interrupts. */
extern INTERRUPT_TABLE_STRUCT          mqx_static_isr_table[MQX_INTERRUPT_TABLE_ITEMS];

/* MQX system stack */
extern uint8_t mqx_system_stack[];

/* Task ready queue */
#define MQX_IDLE_TASK_PRIORITY         (32)
#define MQX_READY_QUEUE_ITEMS          (MQX_IDLE_TASK_PRIORITY + 1)
extern READY_Q_STRUCT                  mqx_static_ready_queue[MQX_READY_QUEUE_ITEMS];

/* Task stacks array of pointers */
extern const uint8_t * mqx_task_stack_pointers[];



/* Backward compatibility */
#define MQXLITE_SYSTEM_TIMER_GET_TICS_PER_SECOND    MQXLITE_SYSTEM_TIMER_GET_TICKS_PER_SECOND

/* END MQX1. */

#endif
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.0.13 [05.05]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
