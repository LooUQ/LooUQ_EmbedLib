/******************************************************************************
 *  \file lqSamd.c
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Thanks to Adafruit Industries for making their SleepyDog library available.
 *  This C99 library for SAMD M0 and M4 only is close port from their C++ version.
 *  https://github.com/adafruit/Adafruit_SleepyDog
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

#ifdef ARDUINO_ARCH_SAMD

#include <lq-embed.h>
#define LOG_LEVEL LOGLEVEL_DBG
//#define DISABLE_ASSERTS                                   // ASSERT/ASSERT_W enabled by default, can be disabled 

#include <sam.h>
#include <stdbool.h>
#include <stdint.h>
#include "platform\lq-arduinoSAMD_util.h"


bool wdInitialized = false;
bool isSleeping = false;

/* Static local functions
------------------------------------------------------------------------------------------------ */
static void S_initialize_wdt(void);


void lqStop()
{
    while(1) {}
}

void lqRestart()
{
    NVIC_SystemReset();      // processor software reset
}

uint16_t lqGetResetCause()
{
    #if defined(__SAMD51__)
    return RSTC->RCAUSE.reg;
    #else
    return PM->RCAUSE.reg;
    #endif
}


uint8_t lqSAMD_getResetCause(void)
{
    #if defined(__SAMD51__)
    return RSTC->RCAUSE.reg;
    #else
    return PM->RCAUSE.reg;
    #endif
}


uint16_t lqSAMD_wdEnable(uint16_t maxPeriodMS)
{
    // Enable the watchdog with a period up to the specified max period in
    // milliseconds.

    // Review the watchdog section from the SAMD21 datasheet section 17:
    // http://www.atmel.com/images/atmel-42181-sam-d21_datasheet.pdf

    int cycles;
    uint8_t bits;

    if (!wdInitialized)
        S_initialize_wdt();

    #if defined(__SAMD51__)
    WDT->CTRLA.reg = 0;                 // Disable watchdog for config
    while (WDT->SYNCBUSY.reg);
    #else
    WDT->CTRL.reg = 0;                  // Disable watchdog for config
    while (WDT->STATUS.bit.SYNCBUSY);
    #endif

    /* You'll see some occasional conversion here compensating between milliseconds (1000 Hz) and WDT clock cycles (~1024 Hz).  
     * The low-power oscillator used by the WDT ostensibly runs at 32,768 Hz with 1:32 prescale, thus 1024 Hz, though 
      * probably not super precise. */
    if ((maxPeriodMS >= 16000) || !maxPeriodMS) 
    {
        cycles = 16384;
        bits = 0xB;
    }
    else 
    {
        cycles = (maxPeriodMS * 1024L + 500) / 1000; // ms -> WDT cycles
        if (cycles >= 8192) {
        cycles = 8192;
        bits = 0xA;
        } else if (cycles >= 4096) {
        cycles = 4096;
        bits = 0x9;
        } else if (cycles >= 2048) {
        cycles = 2048;
        bits = 0x8;
        } else if (cycles >= 1024) {
        cycles = 1024;
        bits = 0x7;
        } else if (cycles >= 512) {
        cycles = 512;
        bits = 0x6;
        } else if (cycles >= 256) {
        cycles = 256;
        bits = 0x5;
        } else if (cycles >= 128) {
        cycles = 128;
        bits = 0x4;
        } else if (cycles >= 64) {
        cycles = 64;
        bits = 0x3;
        } else if (cycles >= 32) {
        cycles = 32;
        bits = 0x2;
        } else if (cycles >= 16) {
        cycles = 16;
        bits = 0x1;
        } else {
        cycles = 8;
        bits = 0x0;
        }
    }

    // Watchdog timer on SAMD is a slightly different animal than on AVR.
    // On AVR, the WTD timeout is configured in one register and then an
    // interrupt can optionally be enabled to handle the timeout in code
    // (as in waking from sleep) vs resetting the chip.  Easy.
    // On SAMD, when the WDT fires, that's it, the chip's getting reset.
    // Instead, it has an "early warning interrupt" with a different set
    // interval prior to the reset.  For equivalent behavior to the AVR
    // library, this requires a slightly different configuration depending
    // whether we're coming from the sleep() function (which needs the
    // interrupt), or just enable() (no interrupt, we want the chip reset
    // unless the WDT is cleared first).  In the sleep case, 'windowed'
    // mode is used in order to allow access to the longest available
    // sleep interval (about 16 sec); the WDT 'period' (when a reset
    // occurs) follows this and is always just set to the max, since the
    // interrupt will trigger first.  In the enable case, windowed mode
    // is not used, the WDT period is set and that's that.
    // The 'isForSleep' argument determines which behavior is used;
    // this isn't present in the AVR code, just here.  It defaults to
    // 'false' so existing Arduino code works as normal, while the sleep()
    // function (later in this file) explicitly passes 'true' to get the
    // alternate behavior.

    #if defined(__SAMD51__)
    if (isForSleep) 
    {
        WDT->INTFLAG.bit.EW = 1;        // Clear interrupt flag
        WDT->INTENSET.bit.EW = 1;       // Enable early warning interrupt
        WDT->CONFIG.bit.PER = 0xB;      // Period = max
        WDT->CONFIG.bit.WINDOW = bits;  // Set time of interrupt
        WDT->EWCTRL.bit.EWOFFSET = 0x0; // Early warning offset
        WDT->CTRLA.bit.WEN = 1;         // Enable window mode
        while (WDT->SYNCBUSY.reg);      // Sync CTRL write
    }
    else
    {
        WDT->INTENCLR.bit.EW = 1;           // Disable early warning interrupt
        WDT->CONFIG.bit.PER = bits;         // Set period for chip reset
        WDT->CTRLA.bit.WEN = 0;             // Disable window mode
        while (WDT->SYNCBUSY.reg);          // Sync CTRL write
    }

    lqSamdWD_reset();                       // Clear watchdog interval
    WDT->CTRLA.bit.ENABLE = 1;              // Start watchdog now!
    while (WDT->SYNCBUSY.reg);
    #else
    if (isSleeping) 
    {
        WDT->INTENSET.bit.EW = 1;           // Enable early warning interrupt
        WDT->CONFIG.bit.PER = 0xB;          // Period = max
        WDT->CONFIG.bit.WINDOW = bits;      // Set time of interrupt
        WDT->CTRL.bit.WEN = 1;              // Enable window mode
        while (WDT->STATUS.bit.SYNCBUSY);   // Sync CTRL write
    } 
    else 
    {
        WDT->INTENCLR.bit.EW = 1;           // Disable early warning interrupt
        WDT->CONFIG.bit.PER = bits;         // Set period for chip reset
        WDT->CTRL.bit.WEN = 0;              // Disable window mode
        while (WDT->STATUS.bit.SYNCBUSY);   // Sync CTRL write
    }

    lqSAMD_resetWD();                       // Clear watchdog interval
    WDT->CTRL.bit.ENABLE = 1;               // Start watchdog now!
    while (WDT->STATUS.bit.SYNCBUSY);
    #endif

    return (cycles * 1000L + 512) / 1024;   // WDT cycles -> ms
}


void lqSAMD_disableWD(void)
{
    #if defined(__SAMD51__)
    WDT->CTRLA.bit.ENABLE = 0;
    while (WDT->SYNCBUSY.reg);
    #else
    WDT->CTRL.bit.ENABLE = 0;
    while (WDT->STATUS.bit.SYNCBUSY);
    #endif
}


void lqSAMD_reset(void)
{
    // Write the watchdog clear key value (0xA5) to the watchdog
    // clear register to clear the watchdog timer and reset it.
    #if defined(__SAMD51__)
    while (WDT->SYNCBUSY.reg);
    #else
    while (WDT->STATUS.bit.SYNCBUSY);
    #endif
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
}


uint16_t lqSAMD_sleep(uint16_t maxPeriodMS)
{
    isSleeping = true;
    int actualPeriodMS = lqSAMD_enableWD(maxPeriodMS); // true = for sleep

    // Enable standby sleep mode (deepest sleep) and activate.
    // Insights from Atmel ASF library.
    #if (SAMD20 || SAMD21)
    // Don't fully power down flash when in sleep
    NVMCTRL->CTRLB.bit.SLEEPPRM = NVMCTRL_CTRLB_SLEEPPRM_DISABLED_Val;
    #endif

    #if defined(__SAMD51__)
    PM->SLEEPCFG.bit.SLEEPMODE = 0x4;           // Standby sleep mode
    while (PM->SLEEPCFG.bit.SLEEPMODE != 0x4);  // Wait for it to take
    #else
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    #endif

    __DSB();                                    // Data sync to ensure outgoing memory accesses complete
    __WFI();                                    // Wait for interrupt (places device in sleep mode)

    // Code resumes here on wake (WDT early warning interrupt).
    // Bug: the return value assumes the WDT has run its course;
    // incorrect if the device woke due to an external interrupt.
    // Without an external RTC there's no way to provide a correct
    // sleep period in the latter case...but at the very least,
    // might indicate said condition occurred by returning 0 instead
    // (assuming we can pin down which interrupt caused the wake).

    return actualPeriodMS;
}


/* ================================================================================== */
/* ================================================================================== */

/**
 *	\brief Supply watchdog signal handler to be linked into SAMD vector table.
 *
 *  Note: The SAMD's vector/interrupt table is defined with weak references that can be
 *  overridden. The override is by symbol name so signature MUST match: "void WDT_Handler(void)"
 * 
 *  \param void [in] Must be referenced as void to fully match cortex handlers weak reference.
 *  \return Signal handlers return void.
 */
void WDT_Handler(void)
{
    // ISR for watchdog early warning, DO NOT RENAME!
    #if defined(__SAMD51__)
    WDT->CTRLA.bit.ENABLE = 0;              // Disable watchdog
    while (WDT->SYNCBUSY.reg);
    #else
    WDT->CTRL.bit.ENABLE = 0;               // Disable watchdog
    while (WDT->STATUS.bit.SYNCBUSY);       // Sync CTRL write
    #endif
    // WDT->INTFLAG.bit.EW = 1;                // Clear early warning interrupt flag
}


static void S_initialize_wdt(void)
{
    // One-time initialization of watchdog timer.
    // Insights from rickrlh and rbrucemtl in Arduino forum!

    #if defined(__SAMD51__)
    // SAMD51 WDT uses OSCULP32k as input clock now
    // section: 20.5.3
    OSC32KCTRL->OSCULP32K.bit.EN1K = 1;  // Enable out 1K (for WDT)
    OSC32KCTRL->OSCULP32K.bit.EN32K = 0; // Disable out 32K

    // Enable WDT early-warning interrupt
    NVIC_DisableIRQ(WDT_IRQn);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0); // Top priority
    NVIC_EnableIRQ(WDT_IRQn);

    while (WDT->SYNCBUSY.reg)
    ;

    USB->DEVICE.CTRLA.bit.ENABLE = 0; // Disable the USB peripheral
    while (USB->DEVICE.SYNCBUSY.bit.ENABLE)
    ;                                 // Wait for synchronization
    USB->DEVICE.CTRLA.bit.RUNSTDBY = 0; // Deactivate run on standby
    USB->DEVICE.CTRLA.bit.ENABLE = 1;   // Enable the USB peripheral
    while (USB->DEVICE.SYNCBUSY.bit.ENABLE)
    ; // Wait for synchronization
    #else
    // Generic clock generator 2, divisor = 32 (2^(DIV+1))
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
    // Enable clock generator 2 using low-power 32KHz oscillator.
    // With /32 divisor above, this yields 1024Hz(ish) clock.
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_GENEN |
                        GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
    while (GCLK->STATUS.bit.SYNCBUSY)
    ;
    // WDT clock = clock gen 2
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2;

    // Enable WDT early-warning interrupt
    NVIC_DisableIRQ(WDT_IRQn);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0); // Top priority
    NVIC_EnableIRQ(WDT_IRQn);
    #endif

    wdInitialized = true;
}


/* Check free memory (stack-heap) 
 * - Remove if not needed for production
--------------------------------------------------------------------------------- */

// #ifdef __arm__
// // should use uinstd.h to define sbrk but Due causes a conflict
// extern "C" char* sbrk(int incr);
// #else  // __ARM__
// extern char *__brkval;
// #endif  // __arm__

// uint32_t lqSAMD_getMemAvailable();
// {
//     char top;
//     #ifdef __arm__
//     return &top - ((char*)sbrk(0));
//     #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
//     return &top - __brkval;
//     #else  // __arm__
//     return __brkval ? &top - __brkval : &top - __malloc_heap_start;
//     #endif  // __arm__
// }


#endif

