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
LOGLEVEL_DBG	4	<specified>
LOGLEVEL_INFO	3	WHITE
LOGLEVEL_WARN	2	YELLOW
LOGLEVEL_ERR 	1	RED
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
    #undef _LOGGER
    #define _LOGGER 1
    #define LOG_RTT
    #define DIAGPRINT_RTT                                           // LEGACY: remove in future version
    #include <jlinkRtt.h>                                           // will set _LOGGER and define macro expansion for DPRINT()
#endif

#ifndef LOG_LEVEL                               
    #define LOG_LEVEL LOGLEVEL_OFF                                  // default is no logging (when not specified in source)
#endif

#ifndef _LOGGER                                                     // if NO _LOGGER defined yet, default to platform serial ouput
    #define _LOGGER 0
    #define LOG_SERIAL
    #define DIAGPRINT_SERIAL                                        // LEGACY: remove in future version
#endif


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void log_printf(uint8_t color, const char *msg, ...);

#ifdef __cplusplus
}
#endif // __cplusplus


#ifndef PRNT_DEFAULT                // PRNT_ constants will be deprecated when DPRINT() and DPRINT_V() macros are eliminated
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

#ifndef logDEFAULT
    #define logDEFAULT 13
    #define logINFO 12
    #define logWARN 17
    #define logERROR 16

    #define logCYAN 10
    #define logMAGENTA 11
    #define logWHITE 12
    #define logGRAY 13
    #define logBLUE 14
    #define logGREEN 15
    #define logRED 16
    #define logYELLOW 17

    #define logDARKCYAN 20
    #define logDARKMAGENTA 21
    #define logDARKBLUE 24
    #define logDARKGREEN 25
    #define logDARKRED 26
    #define logDARKYELLOW 27
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

// #if LOG_LEVEL == LOGLEVEL_OFF                                // potentially add back once DIAGPRINT is gone
// #pragma message ("LooUQ embedLib logging is diabled.")
// #endif 


// force build to link in float support for printf
#if (defined(LOG_LEVEL) && LOG_LEVEL > LOGLEVEL_OFF) || defined(ENABLE_DIAGPRINT) || defined(ENABLE_DIAGPRINT_VERBOSE)
    asm(".global _printf_float");
#endif

#if defined(LOG_LEVEL) && LOG_LEVEL > LOGLEVEL_OFF

    // dbg_print is the serial port output, color info from macro is dropped
    #if LOG_LEVEL >= LOGLEVEL_DBG
        #define LOG_DBG(c_, f_, ...) log_printf(c_, f_, ##__VA_ARGS__)
    #else
        #define LOG_DBG(c_, f_, ...)
    #endif

    #if LOG_LEVEL >= LOGLEVEL_INFO
        #define LOG_INFO(f_, ...) log_printf(logINFO, f_, ##__VA_ARGS__)
    #else
        #define LOG_INFO(f_, ...)
    #endif

    #if LOG_LEVEL >= LOGLEVEL_WARN
        #define LOG_WARN(f_, ...) log_printf(logWARN, f_, ##__VA_ARGS__)
    #else
        #define LOG_WARN(f_, ...)
    #endif

    #if LOG_LEVEL >= LOGLEVEL_ERROR
        #define LOG_ERROR(f_, ...) log_printf(logERROR, f_, ##__VA_ARGS__)
        #define LOG_NOTICE(f_, ...) log_printf(logERROR, f_, ##__VA_ARGS__)
    #else
        #define LOG_ERROR(f_, ...)
        #define LOG_NOTICE(f_, ...)
    #endif

#else  // No logging, empty macro expansion
    #define LOG_DBG(c_, f_, ...)
    #define LOG_INFO(f_, ...)
    #define LOG_WARN(f_, ...)
    #define LOG_ERROR(f_, ...)
    #define LOG_NOTICE(f_, ...)
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

    #define DPRINT(c_, f_, ...) log_printf(c_, f_, ##__VA_ARGS__)            // log_print is the serial port output, color info from macro is dropped

    #if defined(ENABLE_DIAGPRINT_VERBOSE)
        #define DPRINT_V(c_, f_, ...) log_printf(c_, f_, ##__VA_ARGS__)
    #else
        #define DPRINT_V(c_, f_, ...)
    #endif

#else   // No DPRINT() expansion, statements in source ignored
    #undef DPRINT
    #undef DPRINT_V
    #define DPRINT(c_, f_, ...)
    #define DPRINT_V(c_, f_, ...)
#endif

#endif // __LQLOGGING_H__
