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
OFF Intrinsict, if lqLOG_LEVEL not set in source

lqLOG_LEVEL
-------------------------------------------------
LOGLEVEL_DBG	4	<specified>
LOGLEVEL_INFO	3	WHITE
LOGLEVEL_WARN	2	YELLOW
LOGLEVEL_ERR 	1	RED
LOGLEVEL_OFF	0

Capture/Log Prototype Statements
-------------------------------------------------
lqLOG_ERR(msg_format, variadicParams);
lqLOG_WARN(msg_format, variadicParams);
lqLOG_INFO(msg_format, variadicParams);
lqLOG_DBG(color, msg_format, variadicParams);

================================================================================================ */

// default Cortex targets to JLink-RTT
#ifdef ARDUINO_ARCH_SAMD
    #define ENABLE_JLINKRTT
#endif

#ifdef ENABLE_JLINKRTT
    #undef _lqLOGGER
    #define _lqLOGGER 1
    #define lqLOG_RTT
    #include <jlinkRtt.h>                                          // will set _LOGGER and define macro expansion for DPRINT()
#endif

// #ifndef lqLOG_LEVEL                               
//     #define lqLOG_LEVEL LOGLEVEL_DBG                            // default is no logging (when not specified in source)
// #endif

#ifndef _lqLOGGER                                                   // if NO _LOGGER defined yet, default to platform serial ouput
    #define _lqLOGGER 0
    #define lqLOG_SERIAL
    #define DIAGPRINT_SERIAL                                        // LEGACY: remove in future version
#endif


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void lqLog_printf(uint8_t color, const char *msg, ...);

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
    #define lqCYAN 10
    #define lqMAGENTA 11
    #define lqWHITE 12
    #define lqGRAY 13
    #define lqBLUE 14
    #define lqGREEN 15
    #define lqRED 16
    #define lqYELLOW 17

    #define lqDARKCYAN 20
    #define lqDARKMAGENTA 21
    #define lqDARKBLUE 24
    #define lqDARKGREEN 25
    #define lqDARKRED 26
    #define lqDARKYELLOW 27

    #define lqDEFAULT lqGRAY
    #define lqVRBS lqWHITE
    #define lqINFO lqGRAY
    #define lqWARN lqYELLOW
    #define lqERROR lqRED


    #define lqcCYAN 10
    #define lqcMAGENTA 11
    #define lqcWHITE 12
    #define lqcGRAY 13
    #define lqcBLUE 14
    #define lqcGREEN 15
    #define lqcRED 16
    #define lqcYELLOW 17

    #define lqcDARKCYAN 20
    #define lqcDARKMAGENTA 21
    #define lqcDARKBLUE 24
    #define lqcDARKGREEN 25
    #define lqcDARKRED 26
    #define lqcDARKYELLOW 27

    #define lqcDEFAULT lqcGRAY
    #define lqcVRBS lqcWHITE
    #define lqcINFO lqcGRAY
    #define lqcWARN lqcYELLOW
    #define lqcERROR lqcRED
#endif


/* Define lqLOG_x support
 * ============================================================================================= */
#ifdef lqLOG_ERR
    #undef lqLOG_ERR
#endif
#ifdef lqLOG_WARN
    #undef lqLOG_WARN
#endif
#ifdef lqLOG_INFO
    #undef lqLOG_INFO
#endif 
#ifdef lqLOG_DBG
    #undef lqLOG_DBG
#endif
#ifdef lqlqLOG_VRBS
    #undef lqlqLOG_VRBS
#endif


// #if lqLOG_LEVEL == LOGLEVEL_OFF                                // potentially add back once DIAGPRINT is gone
// #pragma message ("LooUQ embedLib logging is disbled.")
// #endif 


// force build to link in float support for printf
#if (defined(lqLOG_LEVEL) && lqLOG_LEVEL > lqLOGLEVEL_OFF) || defined(ENABLE_DIAGPRINT) || defined(ENABLE_DIAGPRINT_VERBOSE)
    asm(".global _printf_float");
#endif

#if defined(lqLOG_LEVEL) && lqLOG_LEVEL > lqLOGLEVEL_OFF

    #if lqLOG_LEVEL >= lqLOGLEVEL_VRBS
        #define lqLOG_VRBS(f_, ...) lqLog_printf(lqcVRBS, f_, ##__VA_ARGS__)
    #else
        #define lqLOG_VRBS(f_, ...)
    #endif

    #if lqLOG_LEVEL >= lqLOGLEVEL_DBG
        #define lqLOG_DBG(c_, f_, ...) lqLog_printf(c_, f_, ##__VA_ARGS__)
    #else
        #define lqLOG_DBG(c_, f_, ...)
    #endif

    #if lqLOG_LEVEL >= lqLOGLEVEL_INFO
        #define lqLOG_INFO(f_, ...) lqLog_printf(lqcINFO, f_, ##__VA_ARGS__)
    #else
        #define lqLOG_INFO(f_, ...)
    #endif

    #if lqLOG_LEVEL >= lqLOGLEVEL_WARN
        #define lqLOG_WARN(f_, ...) lqLog_printf(lqcWARN, f_, ##__VA_ARGS__)
    #else
        #define lqLOG_WARN(f_, ...)
    #endif

    #if lqLOG_LEVEL >= lqLOGLEVEL_ERROR
        #define lqLOG_ERROR(f_, ...) lqLog_printf(lqcERROR, f_, ##__VA_ARGS__)
        #define lqLOG_NOTICE(f_, ...) lqLog_printf(lqcERROR, f_, ##__VA_ARGS__)
    #else
        #define lqLOG_ERROR(f_, ...)
        #define lqLOG_NOTICE(f_, ...)
    #endif

#else  // No logging, empty macro expansion
    #define lqLOG_VRBS(f_, ...)
    #define lqLOG_DBG(c_, f_, ...)
    #define lqLOG_INFO(f_, ...)
    #define lqLOG_WARN(f_, ...)
    #define lqLOG_ERROR(f_, ...)
    #define lqLOG_NOTICE(f_, ...)
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

    #define DPRINT(c_, f_, ...) lqLog_printf(c_, f_, ##__VA_ARGS__)            // log_print is the serial port output, color info from macro is dropped

    #if defined(ENABLE_DIAGPRINT_VERBOSE)
        #define DPRINT_V(c_, f_, ...) lqLog_printf(c_, f_, ##__VA_ARGS__)
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
