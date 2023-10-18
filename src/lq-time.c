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

#define SRCFILE "LTM"                       // create SRCFILE (3 char) MACRO for lq-diagnostics ASSERT
#define ENABLE_DIAGPRINT                    // expand DPRINT into debug output
//#define ENABLE_DIAGPRINT_VERBOSE            // expand DPRINT and DPRINT_V into debug output
#define ENABLE_ASSERT
#include <lqdiag.h>


#include "lq-time.h"

void getTmFromDateTime(const char* dateTime, struct tm* tm)
{
    char wrkBffr[5];
    memset(tm, 0, sizeof(tm));                                  // initialize unused fields in single step

    char* delimPtr = (char*)memchr(dateTime, 'T', 10);
    if (delimPtr != NULL && (delimPtr - dateTime == 6 || delimPtr - dateTime == 8))
    {
        uint8_t yearSz = (delimPtr > dateTime + 6) ? 4 : 2;
        const char* wrkPtr = dateTime;

        strncpy(wrkBffr, wrkPtr, yearSz);
        tm->tm_year = strtol(wrkBffr, NULL, 10);
        tm->tm_year += (yearSz == 2) ? 2000 : 0;                // if after 2099 this program in not in use

        wrkPtr += yearSz;
        strncpy(wrkBffr, wrkPtr, 2);
        tm->tm_mon = strtol(wrkBffr, NULL, 10);

        wrkPtr += 2;
        strncpy(wrkBffr, wrkPtr, 2);
        tm->tm_mday = strtol(wrkBffr, NULL, 10);

        wrkPtr += 3;                                            // skip past 'T'
        strncpy(wrkBffr, wrkPtr, 2);
        tm->tm_hour = strtol(wrkBffr, NULL, 10);

        wrkPtr += 2;
        strncpy(wrkBffr, wrkPtr, 2);
        tm->tm_min = strtol(wrkBffr, NULL, 10);

        wrkPtr += 2;
        strncpy(wrkBffr, wrkPtr, 2);
        tm->tm_sec = strtol(wrkBffr, NULL, 10);
    }
}
