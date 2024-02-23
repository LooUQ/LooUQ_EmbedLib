/** ***************************************************************************
  @file 
  @brief LTEm timing abstraction declarations.

  @author Greg Terrell/Jensen Miller, LooUQ Incorporated

  \loouq

  @warning Internal dependencies, changes only as directed by LooUQ staff.

-------------------------------------------------------------------------------

LooUQ-LTEmC // Software driver for the LooUQ LTEm series cellular modems.
Copyright (C) 2017-2023 LooUQ Incorporated

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
Also add information on how to contact you by electronic and paper mail.

**************************************************************************** */


#ifndef __PLATFORM_TIMING_H__
#define __PLATFORM_TIMING_H__


/**
 * @brief yield callback allows host application to be signalled when the LTEm1 is awaiting network events
 */
typedef void (*platform_yieldCB_func_t)();

/**
 * @brief External reference to yield callback
 */
extern platform_yieldCB_func_t platform_yieldCB_func;


// typedef struct lTiming_tag
// {
//     yieldCB_func_t yieldCB_func;        ///< Callback (CB) to host application when driver code is waiting for network events, allows for background\watchdog\etc.
//     bool cancellationRequest;           ///< For RTOS implementations, token to request cancellation of background operation.
// } lTiming_t;


#ifdef __cplusplus
extern "C"
{
#include <cstdint>
#else
#include <stdint.h>
#endif // __cplusplus

/* transition to new names 

// uint32_t lMillis();
// void lYield();

// // platform implementation should support task switching here
// void lDelay(uint32_t delay_ms);

// bool lTimerExpired(uint32_t timerBase, uint32_t timerTimeout);
*/

/**
 * @brief (LooUQ) millis function like Arduino and other frameworks. 
 * 
 * @return uint32_t Framework "tick" counters, usually reset at device start.
 */
uint32_t lqMillis();


/**
 * @brief (LooUQ) yield function to give program flow to system scheduler/dispatcher
 */
void lqYield();

// 

/**
 * @brief Platform implementation should support task switching here
 * 
 * @param delay_ms Period in milliseconds to pause/delay
 */
void lqDelay(uint32_t delay_ms);


/* DEPRECATED - To be removed in embedLib v2.1.0 
 ------------------------------------------------------- */
// uint32_t pMillis();
// void pYield();

// // platform implementation should support task switching here
// void pDelay(uint32_t delay_ms);

// bool pElapsed(uint32_t timerBase, uint32_t timerTimeout);
/* --------------------------------------------------------
 */

#ifdef __cplusplus
}
#endif // __cplusplus



#endif  /* !__PLATFORM_TIMING_H__ */