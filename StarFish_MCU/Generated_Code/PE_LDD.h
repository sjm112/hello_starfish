/** ###################################################################
**     THIS COMPONENT MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename    : PE_LDD.h
**     Project     : ProcessorExpert
**     Processor   : MKL26Z128VLH4
**     Version     : Component 01.006, Driver 01.04, CPU db: 3.00.000
**     Compiler    : Keil ARM C/C++ Compiler
**     Date/Time   : 2013-04-18, 10:22, # CodeGen: 13
**     Abstract    :
**
**     Settings    :
**
**
**     Copyright : 1997 - 2012 Freescale Semiconductor, Inc. All Rights Reserved.
**     
**     http      : www.freescale.com
**     mail      : support@freescale.com
** ###################################################################*/
/*!
** @file PE_LDD.h                                                  
** @version 01.04
** @date 2013-04-18, 10:22, # CodeGen: 13
** @brief
**
*/         
/*!
**  @addtogroup PE_LDD_module PE_LDD module documentation
**  @{
*/         
#ifndef __PE_LDD_H
#define __PE_LDD_H

/* MODULE PE_LDD. */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "MQX1.h"
#include "SystemTimer1.h"
#include "I2C.h"
#include "BlinkRateCounter.h"
#include "TSS1.h"
#include "CsIO1.h"
#include "IO1.h"
#include "PWMTimerRGB.h"


/*
** ===================================================================
** Function prototypes
** ===================================================================
*/

/*
** ===================================================================
**     Method      :  Cpu_PE_FillMemory (component MKL26Z256LH4)
*/
/*!
**     @brief
**         Fills a memory area block by a specified value.
**     @param
**       SourceAddressPtr - Source address pointer.
**     @param
**       c - A value used to fill a memory block.
**     @param
**       len - Length of a memory block to fill.
*/
/* ===================================================================*/
void PE_FillMemory(register void* SourceAddressPtr, register uint8_t c, register uint32_t len);

/*
** ===================================================================
**     Method      :  Cpu_PE_PeripheralUsed (component MKL26Z256LH4)
*/
/*!
**     @brief
**         Returns information whether a peripheral is allocated by PEx 
**         or not.
**     @param
**       PrphBaseAddress - Base address of a peripheral.
**     @return
**       TRUE if a peripheral is used by PEx or FALSE if it isn't used.
*/
/* ===================================================================*/
bool PE_PeripheralUsed(uint32_t PrphBaseAddress);

/*
** ===================================================================
**     Method      :  Cpu_LDD_SetClockConfiguration (component MKL26Z256LH4)
*/
/*!
**     @brief
**         Changes the clock configuration of all LDD components in a 
**         project.
**     @param
**       ClockConfiguration - New CPU clock configuration changed by CPU SetClockConfiguration method.
*/
/* ===================================================================*/
void LDD_SetClockConfiguration(LDD_TClockConfiguration ClockConfiguration);

/* END PE_LDD. */


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
