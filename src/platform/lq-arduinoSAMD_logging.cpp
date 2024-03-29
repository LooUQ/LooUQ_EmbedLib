// Copyright (c) 2020 LooUQ Incorporated.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef ARDUINO_ARCH_SAMD

#include <Arduino.h>
#include <lq-embed.h>
#include <lq-logging.h>

__attribute__((weak)) void lqLog_printf(uint8_t color, const char *msg, ...)
{
    char buf[DBGBUFFER_SZ] = {0};
    va_list args;
    va_start(args, msg);
    vsnprintf(buf, sizeof(buf), msg, args);
    va_end(args);

    Serial.print(buf);
}

#endif
