// Many thanks to the team at Memfault.com
// LTEmC ASSERT Implementation derived from Interrupt article at https://interrupt.memfault.com/blog/asserts-in-embedded-systems#common-usages-of-asserts


#ifndef __LQ_DIAGNOSTICS_H__
#define __LQ_DIAGNOSTICS_H__

#include <lq-types.h>

enum 
{
    assert__diagnosticsMagic = 0xD1AC,

    diag__notifMsgSz = 20,
    diag__hwVerSz = 40,
    diag__swVerSz = 12
};


#define GET_LR() __builtin_return_address(0)
// This is ARM and GCC specific syntax
#define GET_PC(_a) __asm volatile ("mov %0, pc" : "=r" (_a))


#define PROCESS_ASSERT(fileId, lineNm)                      \
  do {                                                      \
    void *pc;                                               \
    GET_PC(pc);                                             \
    const void *lr = GET_LR();                              \
    assert_invoke(pc, lr, fileId, lineNm);                  \
  } while (0)


//#define NO_ASSERTS
#ifndef NO_ASSERTS
#define ASSERT(exp, fileId)                                     \
    do {                                                        \
        if (!(exp)) {                                           \
            PROCESS_ASSERT(fileId, __LINE__);                   \
        }                                                       \
    } while (0)
#else
#define ASSERT(exp, fileId)  (0)
#endif


//#define NO_ASSERTWARNINGS
#ifndef NO_ASSERTWARNINGS && NO_ASSERT
#define ASSERT_W(exp, fileId, faultTxt)                         \
    do {                                                        \
        if (!(exp)) {                                           \ 
            assert_warning(fileId, __LINE__, faultTxt);         \
        }                                                       \
    } while (0);    
#else
#define ASSERT_W(exp, faultTxt)  (0)
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


typedef struct diagnosticInfo_tag
{
    uint16_t diagMagic;
    diagRcause_t rcause;                // cause of last reset
    uint8_t bootFlag;                   // boot-loop detection 
    uint8_t notifCode;                  // code from application notification callback
    char notifMsg[20];                  // message from application notification callback
    char hwVersion[40];                 // device HW version that may\may not be recorded
    char swVersion[12];                 // embedded app version that may\may not be recorded

    /* ASSERT capture info */
    uint32_t pc;
    uint32_t lr;
    uint32_t line;
    uint8_t fileId;
 
    /* Application communications state info */
    int16_t commState;                  // indications: TCP/UDP/SSL connected, MQTT state, etc.
    int16_t ntwkState;                  // indications: LTE PDP, etc.
    int16_t signalState;                // indications: rssi, etc.

    /* Hardfault capture */
    uint16_t ufsr;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t return_address;
    uint32_t xpsr;
} diagnosticInfo_t;


typedef struct diagnosticControl_tag 
{
    eventNotifFunc_t notifCB;
    uint8_t notifCBChk;
    diagnosticInfo_t diagnosticInfo;
} diagnosticControl_t;


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void lqDiag_registerNotifCallback(eventNotifFunc_t notifCallback);
void lqDiag_setResetCause(uint8_t resetcause);
diagnosticInfo_t *lqDiag_getDiagnosticsInfo();

void lqDiag_setApplicationMessage(uint8_t notifCode, const char *notifMsg);
void lqDiag_setProtoState(int16_t pstate);
void lqDiag_setNtwkState(int16_t pstate);
void lqDiag_setSignalState(int16_t pstate);
const char *lqDiag_setHwVersion(const char *hwVersion);
const char *lqDiag_setSwVersion(const char *swVersion);

void assert_invoke(void *pc, const void *lr, uint16_t fileId, uint16_t line);
void assert_warning(uint16_t fileId, uint16_t line, const char *faultTxt); 
void assert_brk();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_DIAGNOSTICS_H__ */
