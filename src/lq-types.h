/******************************************************************************
 *  \file lq-types.h
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
 * Global/base defines and typedefs
 *****************************************************************************/

#ifndef __LQ_TYPES_H__
#define __LQ_TYPES_H__

// #include <stddef.h>
// #include <stdint.h>
// #include <stdbool.h>

#ifdef __cplusplus
#include <cstdint>
#include <cstdlib>
#include <cstdbool>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif // __cplusplus


#ifndef ASCII_sOK
#define ASCII_sOK "OK\r\n"

#define ASCII_cCR '\r'
#define ASCII_sCR "\r"
#define ASCII_cCOMMA ','
#define ASCII_cNULL '\0'
#define ASCII_cESC (char)0x1B
#define ASCII_cSPACE (char)0x20
#define ASCII_cDBLQUOTE (char)0x22
#define ASCII_cHYPHEN (char)0x2D
#define ASCII_sCTRLZ "\032"
#define ASCII_sCRLF "\r\n"
#define ASCII_sMQTTTERM "\"\r\n"
#define ASCII_szCRLF 2
#define NOT_NULL 1
#endif


#define PROVISION_MAGIC "LQCP"
#define PROVISION_LQCCONFIG "LQCC"


// to allow for BGxx error codes starting at 900 to be passed back to application
// ltem1c uses macros and the action_result_t typedef

#ifndef RESULTCODE_T
#define RESULTCODE_T
enum lqTypes__resultCodes
{
    resultCode__success = 200,
    resultCode__accepted = 202,
    resultCode__noContent = 204,
    // resultCode__previouslyOpened = 208,

    resultCode__badRequest = 400,
    resultCode__unauthorized = 401,
    resultCode__forbidden = 403,
    resultCode__notFound = 404,
    resultCode__methodNotAllowed = 405,
    resultCode__timeout = 408,
    resultCode__conflict = 409,
    resultCode__gone = 410,
    resultCode__preConditionFailed = 412,
    resultCode__locked = 423,
    resultCode__tooManyRequests = 429,
    resultCode__cancelled = 498,
    resultCode__cmError = 499,

    resultCode__internalError = 500,
    resultCode__unavailable = 503,
    resultCode__gtwyTimeout = 504,              /// signals for a background (doWork) process timeout

    resultCode__extendedBase = 1000,

    // convenience values for processing result values
    resultCode__pending = 0,                    /// Value returned from response parsers indicating a pattern match has not yet been detected
    resultCode__unknown = 0,
    resultCode__anyError = 400,
    resultCode__successMax = 299
};
#endif

// action_result_t should be populated with RESULT_CODE_x constant values or an errorCode (uint >= 400)
typedef uint16_t resultCode_t;

/* LooUQ Common Macros
 =============================================================================================== */
 
#define LQC_PROVISIONING_MAGICFLAG "LQCP"
#define LQC_DEVICECONFIG_PACKAGEID "LQCC"

#define SEC_TO_MS(period) (period * 1000)
#define MIN_TO_MS(period) (period * 1000 * 60)
#define IS_ELAPSED(start, timeout) ((start == 0) ? 0 : pMillis() - start > timeout)
#define ELAPSED_DURATION(start) ((start == 0) ? 0 : pMillis() - start)
#define IS_CYCLE(i,c)  (i % c == 0 && i >= c)

#define IS_SUCCESS(r)  (r == resultCode__success)
#define IS_SUCCESS_RSLT(r) (r == resultCode__success)
#define IS_SUCCESS__RSLT(r) ({ _rslt = r; _rslt == resultCode__success; })

#define IS_NOTSUCCESS(r)  (r != resultCode__success)
#define IS_NOTSUCCESS_RSLT(r)  (r != resultCode__success)
#define IS_NOTSUCCESS__RSLT(r) ({ _rslt = r; _rslt != resultCode__success; })

#define DO_ONCE (0)
#define DO_FOREVER (1)

#define PSZ(SZ) (SZ + 1)
#define STREMPTY(charvar)  (charvar == NULL || charvar[0] == 0 )
#define STRCMP(X, Y) (strcmp(X, Y) == 0)
#define STRNCMP(X, Y, N) (strncmp(X, Y, N) == 0)

// obsolete - delete when reference fully removed
#define PERIOD_FROM_SECONDS(period)  (period * 1000)
#define PERIOD_FROM_MINUTES(period)  (period * 1000 * 60)
#define SET_PROPLEN(SZ) (SZ + 1)



enum LooUQ_constants
{
    APPEVENT_MESSAGE_SZ = 25
};


/* LooUQ standard event notification callback codes.
 * Notification callbacks are the standardize mechanism for LooUQ libraries to update the embedded application of 
 * events or issues. The notification callback function is registered by the application at startup, the signature
 * is shown below. 
 * 
 * void (*appNotify_func)(uint8_t notifType, const char *notifMsg);
 * 
 * To use the notify framework for your application code you can extend with your own enum (uint8_t compatible)
 * or simply pass in a uint8_t value. The enum below shows LooUQ prescribed use. The values above __CATASTROPHIC
 * are expected to end with a breakpoint or application termination in the hard fault handler, where a watchdog
 * timeout is expected to reset the system.
*/


typedef enum appEvent_tag
{
    appEvent_info = 200,

    appEvent_env_getPwr = 210,
    appEvent_env_getBatt,
    appEvent_env_getMem,
    appEvent_env_getSignal,

    appEvent_ntwk_connected = 220,
    appEvent_ntwk_disconnected,

    appEvent_msg_sendDiscard,
    appEvent_msg_sendQueued,

    appEvent_proto_sendFault,
    appEvent_proto_recvFault,

    /*  Warnings/Faults
     --------------------------------------------------- */
    appEvent__WARNINGS = 240,
    appEvent_warn_wassertFailed = 240,  /// code warnings failed (like an assertion, but your call on recovery)

    appEvent__FAULTS = 248,             /// serious problems: recommended that only simple diagnostic logging be attempted, system is likely unstable
    appEvent_fault_softLogic,           /// soft logical state failure, possible recovery via subsystem initialization
    appEvent_fault_hardLogic,           /// hard/hardware state failure, possible recovery via hardware reset/power
    appEvent_fault_assertFailed,        /// code assertion failed, automatically initiated by LooUQ lq_diagnostics subsystem
    appEvent_fault_codeFault,           /// unrecoverable code fault detected
    appEvent_fault_hardFault = 255      /// potential IRQ hardFault signal redirect to UNR handler for logging
} appEvent_t;


typedef enum resetAction_tag
{
    resetAction_skipIfOn,
    resetAction_swReset,
    resetAction_hwReset,
    resetAction_powerReset
} resetAction_t;


typedef struct appEventResponse_tag
{
    uint8_t requestCode;
    uint16_t resultCode;
    char message[APPEVENT_MESSAGE_SZ];
} appEventResponse_t;



/*  Callbacks into application
 =============================================================================================== */

/* yield callback allows host application to be signalled when the LTEm1 is awaiting network events
 * --------------------------------------------------------------------------------------------- */
typedef void (*yield_func)();


/* callback to notify application of an event, events can be simple notifications or a notification that information is needed
 * --------------------------------------------------------------------------------------------- */
typedef void (*appEvntNotify_func)(appEvent_t eventType, const char *notifyMsg);                        // application event notification and action/info request callback


/* callback to application specified diagnostic service with a description of the LTEm location
 * --------------------------------------------------------------------------------------------- */
typedef void (*appDiagCallback_func)(const char *diagPointDescription);                                 // application diagnostics callback


// /* callback to request environment information from app
//  * --------------------------------------------------------------------------------------------- */

// /**
//  * @brief Callback into app to request environment information
//  * 
//  * @returns A int32_t (signed) with the value requested.
//  */
// typedef int32_t (*appInfoRequest_func)(uint8_t infoRqst);


/* callback to allow for app background processingfor extended duration operations to allow for 
 * --------------------------------------------------------------------------------------------- */
typedef void (*yield_func)();


#endif  /* !__LQ_TYPES_H__ */
