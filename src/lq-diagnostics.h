/******************************************************************************
 *  \file lq-diagnostics.c
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2021-2022 LooUQ Incorporated.
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
 * Many thanks to the team at Memfault.com!
 * LTEmC ASSERT Implementation derived from Interrupt article at 
 * 
 * https://interrupt.memfault.com/blog/asserts-in-embedded-systems#common-usages-of-asserts
 * 
 *****************************************************************************/





#ifndef __LQ_DIAGNOSTICS_H__
#define __LQ_DIAGNOSTICS_H__

#include "lq-types.h"
#include "lq-logging.h"
//#include <platform/lq-arduinoSAMD_util.h>

enum 
{
    assert__diagnosticsMagic = 0x186F,                      // Kepler's first find
    assert__fileTagSz = 5,

    diag__notifMsgSz = 20,
    diag__hwVerSz = 40,
    diag__swVerSz = 12,

    diag__bootSafe = 0xFF
};


// default product and source file macro values if the parent did not define them
#ifndef LQ_PRODUCT
    #define LQ_PRODUCT "LQ"
#endif
#ifndef LQ_SRCFILE
    #define LQ_SRCFILE "APP"
#endif 

/* Combine product and source file values into composite SRCFILE_TAG.  
 * SRCFILE_TAG is used for ASSERT location specification
 * --------------------------------------------------------------------------------------------- */
#define SRCFILE_TAG LQ_PRODUCT LQ_SRCFILE


// #define DISABLE_ASSERTS
// #define ASSERT_ACTION_STOP 

// #define LOG_LEVEL LOGLEVEL_ERR

#ifdef ARDUINO_ARCH_SAMD 
    #define GRAB_MCU_STATE(_a)  do { __asm volatile ("mov %0, pc" : "=r" (_a)); lr = __builtin_return_address(0); }while(0)

// This is ARM and GCC specific syntax
// #define GET_LR() __builtin_return_address(0)
// #define GET_PC(_a) __asm volatile ("mov %0, pc" : "=r" (_a))
#endif

#ifdef ARDUINO_ARCH_ESP32
    #define GRAB_MCU_STATE(_a)
#endif


#ifndef DISABLE_ASSERT
    #ifdef ASSERT_ACTION_STOP 
        #define ASSERT(exp)                                                             \
            do {                                                                        \
                if (!(exp)) {                                                           \
                    LOG_ERROR("ASSERT(stop) at %s, ln=%d\r\n", SRCFILE_TAG, __LINE__);  \
                    while(1){}                                                          \
                }                                                                       \
            } while(0)

        #define ASSERT_W(exp, faultTxt)                                                 \
            do {                                                                        \
                if (!(exp)) {                                                           \
                    LOG_WARN("ASSERT(stop) at %s, ln=%d\r\n", SRCFILE_TAG, __LINE__);   \
                    while(1){}                                                          \
                }                                                                       \
            } while(0)

    #else 
        #define ASSERT(exp)                                                             \
            do {                                                                        \
                if (!(exp)) {                                                           \
                    void *pc = 0;                                                       \
                    const void *lr = 0;                                                 \
                    GRAB_MCU_STATE(pc);                                                 \
                    lqASSERT_invoke(SRCFILE_TAG, __LINE__, pc, lr);                     \
                }                                                                       \
            } while (0)

        #define ASSERT_W(exp, faultTxt)                                                 \
            do {                                                                        \
                if (!(exp)) {                                                           \
                    lqASSERT_warning(SRCFILE_TAG, __LINE__, faultTxt);                  \
                }                                                                       \
            } while (0);    
    #endif
#else
    #define ASSERT(exp)
    #define ASSERT_W(exp, faultTxt)
#endif


typedef enum diagRcause_tag
{
    diagRcause_serviced = 0,
    diagRcause_powerOn = 1,
    diagRcause_pwrCore = 2,
    diagRcause_pwrPeriph = 4,
    diagRcause_nvm = 8,
    diagRcause_external = 16,
    diagRcause_watchdog = 32,
    diagRcause_system = 64,
    diagRcause_backup = 128
} diagRcause_t;


/**
 * @brief Diagnostic capture structure for an event.
 */
typedef struct diagnosticInfo_tag
{
    uint16_t diagMagic;                                     ///< Magic value to signal this is a valid diagnostics struct
    uint32_t rcause[4];                                     ///< cause of last reset 
    uint8_t boots;                                          ///< boot-loop detection
    uint8_t notifCode;                                      ///< code from application notification callback
    char notifMsg[20];                                      ///< message from application notification callback
    char hwVersion[40];                                     ///< device HW version that may/may not be recorded
    char swVersion[12];                                     ///< embedded app version that may/may not be recorded

    /* ASSERT capture info */
    uint32_t pc;                                            ///< Program counter register at event
    uint32_t lr;                                            ///< Link register at event
    uint32_t line;                                          ///< Line number from source
    char fileTag[assert__fileTagSz];                        ///< file tag (NOTE: not /0 terminated)
 
    /* Application communications state info */
    int16_t commState;                                      ///< indications: TCP/UDP/SSL connected, MQTT state, etc.
    int16_t ntwkState;                                      ///< indications: LTE PDP, etc.
    int16_t signalState;                                    ///< indications: rssi, etc.

    /* Hardfault capture */
    uint16_t ufsr;                                          ///< registers: UFSR
    uint32_t r0;                                            ///< registers: R0
    uint32_t r1;                                            ///< registers: R1
    uint32_t r2;                                            ///< registers: R2
    uint32_t r3;                                            ///< registers: R3
    uint32_t r12;                                           ///< registers: R12
    uint32_t return_address;                                ///< registers: return address
    uint32_t xpsr;                                          ///< registers: XPSR
} diagnosticInfo_t;


/**
 * @brief Control structure for reporting a diagnostics event 
 */
typedef struct diagnosticControl_tag 
{
    appEvntNotify_func notifyCB;                            ///< Address of a registered notify callback handler
    uint8_t notifyCBChk;                                    ///< Check digit for callback registered correctly
    diagnosticInfo_t diagnosticInfo;                        ///< Diagnostics collection to report to application
} diagnosticControl_t;


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/**
 * @brief Set application callback function destination for diagnostics to inform application of problems
 * 
 * @param [in] notifCallback Function pointer for the application callback.
 */
void lqDiag_setNotifyCallback(appEvntNotify_func notifCallback);


/* Create/update diagnostic info object
 * --------------------------------------------------------------------------------------------- */

/**
 * @brief Set the reset cause 
 * 
 * @param [in] core The MCU core the cause is applicable to. Single core MCU should be 0.
 * @param [in] resetcause The cause (gathered elsewhere) of the last reset
 */
void lqDiag_setResetCause(uint8_t core, uint32_t resetcause);


diagnosticInfo_t *lqDiag_getDiagnosticsBlock();

void lqDiag_setApplicationMessage(uint8_t notifCode, const char *notifMsg);

void lqDiag_setProtoState(int16_t pstate);

void lqDiag_setNtwkState(int16_t pstate);

void lqDiag_setSignalState(int16_t pstate);

const char *lqDiag_setHwVersion(const char *hwVersion);

const char *lqDiag_setSwVersion(const char *swVersion);

void lqDiag_setBootSafe();


/* ASSERT/ASSERT_W implementation functions
 * --------------------------------------------------------------------------------------------- */
/**
 * @brief ASSERT function; FATAL asserts that suggest a problem in the running firmware.
 * 
 * @param [in] fileTag Short text phrase describing the project and source where ASSERT failed.
 * @param [in] line Line number where ASSERT failed.
 * @param [in] pc Program counter register value
 * @param [in] lr Link register value
 */
void lqASSERT_invoke(const char *fileTag, uint16_t line, void *pc, const void *lr);

/**
 * @brief ASSERT warning function; non-fatal asserts that suggest a problem in the running firmware
 * 
 * @param [in] fileTag Short text phrase describing the project and source where ASSERT failed.
 * @param [in] line Line number where ASSERT (warning) failed.
 * @param [in] faultTxt Description from source of warning concern.
 */
void lqASSERT_warning(const char *fileTag, uint16_t line, const char *faultTxt); 


/**
 * @brief ASSERT destination function, allows for customization 
 */
void lqASSERT_brk();


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_DIAGNOSTICS_H__ */
