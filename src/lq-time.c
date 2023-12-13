/******************************************************************************
 *  \file lq-cBuffer.c
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2022 LooUQ Incorporated.
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
 * LooUQ circular buffer implementation for device streaming
 *****************************************************************************/

#include <lq-embed.h>
#define LOG_LEVEL LOGLEVEL_DBG
//#define DISABLE_ASSERTS                                   // ASSERT/ASSERT_W enabled by default, can be disabled 
#define SRCFILE "LTM"                                       // create SRCFILE (3 char) MACRO for lq-diagnostics ASSERT

#define ENABLE_DIAGPRINT                    // expand DPRINT into debug output
//#define ENABLE_DIAGPRINT_VERBOSE            // expand DPRINT and DPRINT_V into debug output
#define ENABLE_ASSERT


#include "lq-time.h"

// void getTmFromDateTime(const char *iso8601, struct tm *tm)
// {
//     char wrkBffr[16] = {0};
//     memset(tm, 0, sizeof(struct tm));                                           // initialize unused fields in single step

//     char *delimPtr = (char *)memchr(iso8601, 'T', 10);
//     if (delimPtr != NULL && (delimPtr - iso8601 == 6 || delimPtr - iso8601 == 8))
//     {
//         // uint8_t yearSz = (delimPtr > iso8601 + 6) ? 4 : 2;
//         uint8_t copySz = (delimPtr > iso8601 + 6) ? 15 : 13;
//         memcpy(wrkBffr, iso8601, copySz);                                       // copy input dateTime to destructive work buffer
//         char* wrkPtr = wrkBffr;

//         wrkPtr = wrkBffr + copySz - 2;                                          // init wrkPtr start of seconds (to work backwards)
//         tm->tm_sec = strtol(wrkPtr, NULL, 10);
//         *wrkPtr = '\0';                                                         // new end-of-string

//         wrkPtr -= 2;
//         tm->tm_min = strtol(wrkPtr, NULL, 10);
//         *wrkPtr = '\0';

//         wrkPtr -= 2;
//         tm->tm_hour = strtol(wrkPtr, NULL, 10);
//         wrkPtr--;                                                               // skip 'T'
//         *wrkPtr = '\0';

//         wrkPtr -= 2;
//         tm->tm_mday = strtol(wrkPtr, NULL, 10);
//         *wrkPtr = '\0';

//         wrkPtr -= 2;
//         tm->tm_mon = strtol(wrkPtr, NULL, 10);
//         tm->tm_mon -= 1;                                                        // months since January — [0, 11]
//         *wrkPtr = '\0';

//         wrkPtr -= 2;
//         tm->tm_year = strtol(wrkPtr, NULL, 10);
//         tm->tm_year += 100;                                                     // years since 1900 
//         return;
//     }
//     tm = NULL;
// }
