#ifdef ARDUINO_ARCH_ESP32

#include "lq-platform_task.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include "freertos/semphr.h"

SemaphoreHandle_t _ltemCmd_mutexHandle;
static StaticSemaphore_t _ltemCmd_mutexStruct;

typedef struct esp32_mutex_tag
{
    SemaphoreHandle_t handle;
    StaticSemaphore_t mutex;
} esp32_mutex_t;

static SemaphoreHandle_t _mutexContainer(mutexTableIndex_t indx);



/* Public platform functions
 ----------------------------------------------------------------------------------------------- */

uint8_t lqMutexCount(mutexTableIndex_t indx)
{
    return uxSemaphoreGetCount(_mutexContainer(indx));
}


bool lqMutexTake(mutexTableIndex_t indx, uint16_t timeout)
{

    return xSemaphoreTake(_mutexContainer(indx), timeout);
}


void lqMutexGive(mutexTableIndex_t indx)
{
    xSemaphoreGive(_mutexContainer(indx));
}

uint32_t lqGetTaskHandle()
{
    return 0;
}


/* Static local functions
 ----------------------------------------------------------------------------------------------- */
static SemaphoreHandle_t _mutexContainer(mutexTableIndex_t indx)
{
    static bool initialized = false;
    static SemaphoreHandle_t _handles[mutexTableSz];
    static StaticSemaphore_t _mutexTable[mutexTableSz];

    if (!initialized)
    {
        for (size_t i = 0; i < mutexTableSz; i++)
        {
            _handles[indx] = xSemaphoreCreateMutexStatic(&_mutexTable[indx]);
        }
        initialized = true;
    }
    return _handles[indx];
}

#endif  // ARDUINO_ARCH_ESP32
