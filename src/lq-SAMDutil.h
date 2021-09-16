/******************************************************************************
 *  \file lqSAMD-core.h
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Thanks to Adafruit Industries for making their SleepyDog library available.
 *
 *  Copyright (c) 2020,2021 LooUQ Incorporated.
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
 ******************************************************************************
 * SAMD chip function support 
 *****************************************************************************/

#ifndef __LQSAMD_CORE_H__
#define __LQSAMD_CORE_H__

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t lqSAMD_getResetCause();

uint16_t lqSAMD_wdEnable(uint16_t maxPeriodMS);
void lqSAMD_wdDisable();
void lqSAMD_wdReset();

uint16_t lqSAMD_sleep(uint16_t maxPeriodMS);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQSAMD_CORE_H__ */
