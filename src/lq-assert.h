// Many thanks to the team at Memfault.com
// LTEmC ASSERT Implementation derived from Interrupt article at https://interrupt.memfault.com/blog/asserts-in-embedded-systems#common-usages-of-asserts


#ifndef __LQ_ASSERT_H__
#define __LQ_ASSERT_H__

#include <lq-types.h>

enum 
{
    assert__assertControlMagic = 0xAC,
    assert__diagnosticsMagic = 0xDD
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
#define ASSERT(exp, fileId)                                 \
    do {                                                    \
        if (!(exp)) {                                       \
            PROCESS_ASSERT(fileId, __LINE__);               \
        }                                                   \
    } while (0)
#else
#define ASSERT(exp, fileId)  (0)
#endif


//#define NO_ASSERTWARNINGS
#ifndef NO_ASSERTWARNINGS && NO_ASSERT
#define ASSERT_W(exp, fileId, faultTxt)                     \
    do {                                                    \
        if (!(exp)) {                                       \ 
            assert_warning(fileId, __LINE__, faultTxt);     \
        }                                                   \
    } while (0);    
#else
#define ASSERT_W(exp, faultTxt)  (0)
#endif



typedef enum lqDiagnosticRCause_tag
{
    lqDiagResetCause_powerOn = 1,
    lqDiagResetCause_pwrCore = 2,
    lqDiagResetCause_pwrPeriph = 4,
    lqDiagResetCause_nvm = 8,
    lqDiagResetCause_external = 16,
    lqDiagResetCause_watchdog = 32,
    lqDiagResetCause_system = 64,
    lqDiagResetCause_backup = 128
} lqDiagnosticRCause_t;


typedef struct lqDiagnosticInfo_tag
{
    uint8_t diagMagic;
    uint8_t notifType;                  // code from application notification callback
    char notifMsg[20];                  // message from application notification callback
    uint8_t rptStatus;                  // indication that the diagnostics info has been reported to host\cloud
    lqDiagnosticRCause_t rCause;        // cause of last reset

    /* ASSERT capture info */
    uint32_t pc;
    uint32_t lr;
    uint32_t line;
    uint8_t fileId;
 
    /* Application communications state info */
    int16_t protoState;                 // indications: TCP/UDP/SSL connected, MQTT state, etc.
    int16_t ntwkState;                  // indications: LTE PDP, etc.
    int16_t sgnlState;                  // indications: rssi, etc.

    /* Hardfault capture */
    uint16_t ufsr;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t return_address;
    uint32_t xpsr;
} lqDiagnosticInfo_t;


typedef struct assertControl_tag 
{
    uint8_t assertMagic;
    lqDiagnosticInfo_t diagnosticInfo;
    appNotifyFunc_t notifyCB;
} assertControl_t;


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void assert_registerNotifCallback(appNotifyFunc_t notifyCallback);
lqDiagnosticInfo_t *assert_getDiagInfoPtr();

void assert_init(lqDiagnosticInfo_t *diagInfo, appNotifyFunc_t notifCB);

void assert_invoke(void *pc, const void *lr, uint16_t fileId, uint16_t line);
void assert_warning(uint16_t fileId, uint16_t line, const char *faultTxt); 
void assert_brk();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_ASSERT_H__ */
