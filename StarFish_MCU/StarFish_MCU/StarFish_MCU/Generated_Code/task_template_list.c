/** ###################################################################
**     THIS COMPONENT MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename    : task_template_list.c
**     Project     : ProcessorExpert
**     Processor   : MKL26Z128VLH4
**     Version     : Component 01.098, Driver 01.00, CPU db: 3.00.000
**     Compiler    : Keil ARM C/C++ Compiler
**     Date/Time   : 2013-04-18, 10:22, # CodeGen: 13
**
**     Copyright : 1997 - 2012 Freescale Semiconductor, Inc. All Rights Reserved.
**     
**     http      : www.freescale.com
**     mail      : support@freescale.com
** ###################################################################*/
/*!
** @file task_template_list.c                                                  
** @version 01.00
** @date 2013-04-18, 10:22, # CodeGen: 13
*/         
/*!
**  @addtogroup task_template_list_module task_template_list module documentation
**  @{
*/         

#include "task_template_list.h"
#include "mqx_tasks.h"

#define TASK_TEMPLATE_LIST_END         {0, 0, 0, 0, 0, 0, 0}

/* MQX task template list */
const TASK_TEMPLATE_STRUCT MQX_template_list[] =
{
  /* Task: Init_Task */
  {
  /* Task number                      */  INIT_TASK_TASK,
  /* Entry point                      */  (TASK_FPTR)Init_Task,
  /* Stack size                       */  INIT_TASK_TASK_STACK_SIZE,
  /* Task priority                    */  9U,
  /* Task name                        */  "init_task",
  /* Task attributes                  */  (MQX_AUTO_START_TASK),
  /* Task parameter                   */  (uint32_t)(0)
  },
  /* Task: ColorTask */
  {
  /* Task number                      */  COLORTASK_TASK,
  /* Entry point                      */  (TASK_FPTR)ColorTask,
  /* Stack size                       */  COLORTASK_TASK_STACK_SIZE,
  /* Task priority                    */  9U,
  /* Task name                        */  "colortask",
  /* Task attributes                  */  (0),
  /* Task parameter                   */  (uint32_t)(0)
  },
  TASK_TEMPLATE_LIST_END
};

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
