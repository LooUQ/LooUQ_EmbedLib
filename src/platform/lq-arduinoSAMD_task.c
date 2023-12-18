
#ifdef ARDUINO_ARCH_SAMD

#include "lq-platform_task.h"

#include <stdint.h>


static uint8_t _mutexContainer(mutexTableIndex_t indx, int newValue);

#define SAMD_SIGNAL_TAKE (-1)
#define SAMD_SIGNAL_QUERY (0)
#define SAMD_SIGNAL_GIVE (1)

/* Public platform functions
 ----------------------------------------------------------------------------------------------- */

uint8_t lqMutexCount(mutexTableIndex_t indx)
{
    return _mutexContainer(indx, SAMD_SIGNAL_QUERY);
}


bool lqMutexTake(mutexTableIndex_t indx, uint16_t timeout)           // SAMD is single-threaded, no alternate task to release mutex; API parameter timeout is ignored
{
    return _mutexContainer(indx, SAMD_SIGNAL_TAKE);
}


void lqMutexGive(mutexTableIndex_t indx)
{
    _mutexContainer(indx, SAMD_SIGNAL_GIVE);
    //return _mutexContainer(indx, SAMD_SIGNAL_GIVE);
}

uint32_t lqGetTaskHandle()
{
    return 0;
}



/* Static local functions
 ----------------------------------------------------------------------------------------------- */
static uint8_t _mutexContainer(mutexTableIndex_t indx, int newValue)
{
    //static bool initialized = false;
    // static SemaphoreHandle_t _handles[mutexTableSz];
    // static StaticSemaphore_t _mutexTable[mutexTableSz];


    static uint8_t _mutexTable[mutexTableSz] = {0};

    if (newValue > 0 && _mutexTable[indx] < 1)
        _mutexTable[indx]++;
    else if (newValue < 0 && _mutexTable[indx] > 0)
        _mutexTable[indx]--;


    // if (!initialized)
    // {
    //     for (size_t i = 0; i < mutexTableSz; i++)
    //     {
    //         _handles[indx] = xSemaphoreCreateMutexStatic(&_mutexTable[indx]);
    //     }
    //     initialized = true;
    // }

    return _mutexTable[indx];
}

#endif  // ARDUINO_ARCH_SAMD