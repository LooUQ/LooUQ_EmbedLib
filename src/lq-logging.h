/******************************************************************************
 *  \file lq-logging.h
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2023 LooUQ Incorporated.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 * LooUQ Diagnostics 
 * - Debug logging via Segger RTT or native platform serial output.
 * - ASSERT signalling (incl ASSERT_W) with application notification and
 *   platform specific diagnostic information capture
 * 
 *****************************************************************************/

#ifndef __LQLOGGING_H__
#define __LQLOGGING_H__

#include <stdint.h>
#include "lq-embed.h"


/* Logging and PRINTF macro support
 * ================================================================================================ 
 
 Standard Logging Levels
-------------------------------------------------
FATAL [not implemented]
ERROR (ASSERT)  
WARN (ASSET_W) 
INFO Start/stop, initialization 
DEBUG (DPRINT) 
TRACE [not implemented]
ALL [not implemented, set level to TRACE]
OFF Intrinsict, if LOG_LEVEL not set in source

LOG_LEVEL
-------------------------------------------------
LOGLEVEL_ERR 	4	RED
LOGLEVEL_WARN	3	YELLOW
LOGLEVEL_INFO	2	WHITE
LOGLEVEL_DBG	1	<specified>
LOGLEVEL_OFF	0

Capture/Log Prototype Statements
-------------------------------------------------
LOG_ERR(msg_format, variadicParams);
LOG_WARN(msg_format, variadicParams);
LOG_INFO(msg_format, variadicParams);
LOG_DBG(color, msg_format, variadicParams);

================================================================================================ */

// default Cortex targets to JLink-RTT
#ifdef ARDUINO_ARCH_SAMD
    #define ENABLE_JLINKRTT
#endif

#ifdef ENABLE_JLINKRTT
    #include <jlinkRtt.h>               // will set _TERMINAL and define macro expansion for DPRINT()
#endif

// default logging level (when not specified in source)
#ifndef LOG_LEVEL                               
    #define LOG_LEVEL LOGLEVEL_OFF
#endif


#ifndef PRNT_DEFAULT
    #define PRNT_DEFAULT 13
    #define PRNT_INFO 12
    #define PRNT_WARN 17
    #define PRNT_ERROR 16

    #define PRNT_CYAN 10
    #define PRNT_MAGENTA 11
    #define PRNT_WHITE 12
    #define PRNT_GRAY 13
    #define PRNT_BLUE 14
    #define PRNT_GREEN 15
    #define PRNT_RED 16
    #define PRNT_YELLOW 17

    #define PRNT_dCYAN 20
    #define PRNT_dMAGENTA 21
    #define PRNT_dBLUE 24
    #define PRNT_dGREEN 25
    #define PRNT_dRED 26
    #define PRNT_dYELLOW 27
#endif


/* Define LOG_x support
 * ============================================================================================= */
#ifdef LOG_ERR
    #undef LOG_ERR
#endif
#ifdef LOG_WARN
    #undef LOG_WARN
#endif
#ifdef LOG_INFO
    #undef LOG_INFO
#endif 
#ifdef LOG_DBG
    #undef LOG_DBG
#endif

// forces build to link in float support for printf
#if (defined(LOG_LEVEL) && LOG_LEVEL > LOGLEVEL_OFF) || defined(ENABLE_DIAGPRINT) || defined(ENABLE_DIAGPRINT_VERBOSE)
    asm(".global _printf_float");
#endif

#if defined(LOG_LEVEL) && LOG_LEVEL > LOGLEVEL_OFF

    // dbg_print is the serial port output, color info from macro is dropped
    #if LOG_LEVEL > LOG_DBG
        #define LOG_DBG(c_, f_, ...) log_printf((f_), ##__VA_ARGS__)
    #else
        #define LOG_DBG(c_, f_, ...)
    #endif

    #if LOG_LEVEL > LOG_INFO
        #define LOG_INFO(f_, ...) log_print(PRNT_INFO, (f_), ##__VA_ARGS__)
    #else
        #define LOG_INFO(f_, ...)
    #endif

    #if LOG_LEVEL > LOG_WARN
        #define LOG_WARN(f_, ...) log_print(PRNT_WARN, (f_), ##__VA_ARGS__)
    #else
        #define LOG_WARN(f_, ...)
    #endif

    #if LOG_LEVEL > LOG_ERR
        #define LOG_ERR(f_, ...) log_print(PRNT_ERROR, (f_), ##__VA_ARGS__)
    #else
        #define LOG_ERR(f_, ...)
    #endif

#else  // No logging, empty macro expansion
    #define LOG_DBG(c_, f_, ...)
    #define LOG_INFO(f_, ...)
    #define LOG_WARN(f_, ...)
    #define LOG_ERR(f_, ...)
#endif 


/* LEGACY DPRINT
 * --------------------------------------------------------------------------------------------- */

// configure diagnostic logging output
#if defined(ENABLE_DIAGPRINT)

    #ifdef DPRINT
        #undef DPRINT
    #endif
    #ifdef DPRINT_V
        #undef DPRINT_V
    #endif

    #define DPRINT(c_, f_, ...) log_print(c_, (f_), ##__VA_ARGS__)          // log_print is the serial port output, color info from macro is dropped

    #if defined(ENABLE_DIAGPRINT_VERBOSE)
        #define DPRINT_V(c_, f_, ...) log_print(c_, (f_), ##__VA_ARGS__)
    #else
        #define DPRINT_V(c_, f_, ...)
    #endif

#else   // No DPRINT() expansion, statements in source ignored
    #undef DPRINT
    #undef DPRINT_V
    #define DPRINT(c_, f_, ...)
    #define DPRINT_V(c_, f_, ...)
#endif


#ifndef _TERMINAL                                                       // if NO _TERMINAL defined yet, default to platform serial ouput
    #define _TERMINAL
    #define _TERMINAL_SERIAL
    #define LOG_SERIAL
    #define DIAGPRINT_SERIAL                                            // LEGACY: remove in future version
#endif


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void log_print(uint8_t color, const char *msg, ...);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __LQLOGGING_H__

