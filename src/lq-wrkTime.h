/******************************************************************************
 *  \file lq-wrkTime.h
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2020, 2021 LooUQ Incorporated.
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
 * LooUQ LQCloud Client Work-Time Services
 *****************************************************************************/

#ifndef __LQ_WRKTIME_H__
#define __LQ_WRKTIME_H__

#include <stdint.h>
#include <stdbool.h>


/* Work Time 
------------------------------------------------------------------------------------------------ */

#define PERIOD_FROM_SECONDS(period)  (period * 1000)
#define PERIOD_FROM_MINUTES(period)  (period * 1000 * 60)
#define PERIOD_FROM_HOURS(period)  (period * 1000 * 60 * 60)

typedef unsigned long millisTime_t, millisDuration_t;   // aka uint32_t 


/** 
 *  \brief typedef of workSchedule object to coordinate the timing of application work items.
*/
typedef struct wrkTime_tag
{
    millisTime_t period;            ///< Time period in milliseconds 
    millisTime_t lastMillis;        ///< Tick (system millis()) when the workSchedule timer last signaled in workSched_doNow()
    millisTime_t elapsedAtPaused;   ///< Duration (within interval period) when timer was paused, used to calc partial period on resume
    uint8_t enabled;                ///< Reset on timer sched objects, when doNow() (ie: timer is queried) following experation.
} wrkTime_t;



#ifdef __cplusplus
extern "C"
{
#endif


wrkTime_t wrkTime_create(unsigned long intervalMillis);

void wrkTime_start(wrkTime_t *schedObj);
void wrkTime_stop(wrkTime_t *schedObj);
void wrkTime_reset(wrkTime_t *schedObj, unsigned long intervalMillis);
void wrkTime_pause(wrkTime_t *schedObj);
void wrkTime_resume(wrkTime_t *schedObj);

bool wrkTime_isRunning(wrkTime_t *schedObj);
bool wrkTime_doNow(wrkTime_t *schedObj);
bool wrkTime_isElapsed(millisTime_t startTime, millisDuration_t reqdDuration);

#ifdef __cplusplus
}
#endif // !__cplusplus


#endif  /* !__LQ_WRKTIME_H__ */
