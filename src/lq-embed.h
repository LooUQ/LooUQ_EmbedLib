/******************************************************************************
 *  \file lq-embed.h
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
 *****************************************************************************/


#define LOGLEVEL_DBG	(4)
#define LOGLEVEL_INFO	(3)
#define LOGLEVEL_WARN	(2)
#define LOGLEVEL_ERROR 	(1)
#define LOGLEVEL_OFF	(0)

#ifndef LOGBUFFER_SZ
     #define LOGBUFFER_SZ 180
#endif
#ifndef DBGBUFFER_SZ
     #define DBGBUFFER_SZ 180
#endif


// #ifndef LOG_DEFAULT
//     #define LOG_DEFAULT 13
//     #define LOG_INFO 12
//     #define LOG_WARN 17
//     #define LOG_ERROR 16

//     #define LOG_CYAN 10
//     #define LOG_MAGENTA 11
//     #define LOG_WHITE 12
//     #define LOG_GRAY 13
//     #define LOG_BLUE 14
//     #define LOG_GREEN 15
//     #define LOG_RED 16
//     #define LOG_YELLOW 17

//     #define LOG_dCYAN 20
//     #define LOG_dMAGENTA 21
//     #define LOG_dBLUE 24
//     #define LOG_dGREEN 25
//     #define LOG_dRED 26
//     #define LOG_dYELLOW 27
// #endif


