/** ***************************************************************************
  @file lq-arduinoESP32_diagnostics.cpp
  @brief LooUQ application diagnostics for SAMD (ARM Cortex+) under Arduino framework.

  @author Greg Terrell, LooUQ Incorporated

  \loouq

  @warning Internal dependencies, changes only as directed by LooUQ staff.

-------------------------------------------------------------------------------

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

#ifdef ARDUINO_ARCH_ESP32

#include <FreeRTOS.h>
#include <rom\rtc.h>
#include <task.h>

#include "lq-embed.h"
#define LOG_LEVEL LOGLEVEL_INFO

#include "lq-types.h"
#include "lq-logging.h"
#include "lq-diagnostics.h"
#include <stdio.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * @brief ASSERT function; FATAL asserts that suggest a problem in the running firmware.
 */
void assert_invoke(const char *fileTag, uint16_t line, void *pc, const void *lr)
{
    LOG_ERROR("*** ASSERT in %s at line %d\r\n", fileTag, line);
    vTaskDelay(SEC_TO_MS(10)); 
    assert_brk();
}


/**
 * @brief ASSERT warning function; non-fatal asserts that suggest the likelihood of a problem in the running firmware
 * 
 * @param fileTag 
 * @param line 
 * @param faultTxt 
 */
void assert_warning(const char *fileTag, uint16_t line, const char *faultTxt)
{
    LOG_WARN("*** ASSERT Warning in %s at line %d\r\n", fileTag, line, faultTxt);
    vTaskDelay(SEC_TO_MS(10)); 
}



inline void assert_brk()                                // future debug breakpoint destination possible
{
    #ifdef ASSERT_ACTION_STOP
        LOG_ERR("*** ASSERT_ACTION_STOP Specified in Source - HALTED.\r\n");
        while(1){}                                      // stop here
    #else
    abort();
    #endif
}


#endif // #ifdef ARDUINO_ARCH_ESP32
