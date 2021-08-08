#include <lq-types.h>
#include <lq-diagnostics.h>
#include <stdio.h>

diagnosticControl_t g_diagControl  __attribute__ ((section (".noinit")));


uint8_t S_calcNotifyCbChk(uint32_t notifCbAddr)
{
    uint8_t result = notifCbAddr & 0xFF;
    result = result ^ (notifCbAddr >> 8) & 0xFF;
    result = result ^ (notifCbAddr >> 16) & 0xFF;
    result = result ^ (notifCbAddr >> 24) & 0xFF;
    return result;
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \param notifyCallback Function pointer to the application's event notification callback function, which may or may not return.
 */
void lqDiag_registerNotifCallback(appNotifyFunc_t notifyCallback)
{
    //g_diagControl.assertMagic = assert__assertControlMagic;
    g_diagControl.notifyCB = notifyCallback;
    g_diagControl.notifyCbChk = S_calcNotifyCbChk((int32_t)notifyCallback);
}


void lqDiag_setResetCause(uint8_t resetcause)
{
    g_diagControl.diagnosticInfo.rcause = resetcause;
    if (resetcause != diagRcause_watchdog && resetcause != diagRcause_system)           // if non-error reset, clear diagnostics
    {
        memset(&g_diagControl.diagnosticInfo + 1, 0, sizeof(diagnosticInfo_t) - 1);
    }
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \returns Pointer to the diagnostics information block, or NULL if no valid diagnostics available.
 */
diagnosticInfo_t *lqDiag_getDiagnosticsInfo()
{
    if (g_diagControl.diagnosticInfo.diagMagic == assert__diagnosticsMagic)
        return &g_diagControl.diagnosticInfo;
    return NULL;
}


/**
 *  \brief Clear diagnostics information block.
 * 
 *  LQCloud uses this after checking diagnostics at system start. LQCloud will use reset cause and diagnostics magic value
 *  to determine if the diagnostics are relevant and sent.
 */
void lqDiag_clearDiagnosticInfo()
{
    memset(&(g_diagControl.diagnosticInfo), 0, sizeof(diagnosticInfo_t));       // clear diagnostics, magic == 0 is no diags available
}

#pragma region ASSERT Functionality
/* --------------------------------------------------------------------------------------------- */

/**
 *  \brief Function to service an application failed ASSERT().
 * 
 *  \param pc
 *  \param lr
 *  \param fileIn [in] The numeric ID of the file containing the triggered ASSERT_W()
 *  \param line [in] The line number of the ASSERT_W() triggering the invoke.
 */
void assert_invoke(void *pc, const void *lr, uint16_t fileId, uint16_t line)
{
    if (g_diagControl.diagnosticInfo.diagMagic != assert__diagnosticsMagic)
    {
        g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
        g_diagControl.diagnosticInfo.pc = pc;
        g_diagControl.diagnosticInfo.lr = lr;
        g_diagControl.diagnosticInfo.line = line;
        g_diagControl.diagnosticInfo.fileId = fileId;
    }
    if (g_diagControl.notifyCB != NULL && g_diagControl.notifyCbChk == S_calcNotifyCbChk(g_diagControl.notifyCB))
    {
        char notifyMsg[40];
        snprintf(notifyMsg, sizeof(notifyMsg), "ASSERT f:%d,l:%d-pc=%d,lr=%d", fileId, line, pc, lr);
        g_diagControl.notifyCB(lqNotifType_assertFailed, notifyMsg);
    }
    assert_brk();                                                               // stop here if notify callback returned
}


/**
 *  \brief Function to service an application failed ASSERT_W().
 * 
 *  Assert warnings are intended to be used during active development to signal back non-optimal
 *  code patterns.
 * 
 *  \param fileIn [in] The numeric ID of the file containing the triggered ASSERT_W()
 *  \param line [in] The line number of the ASSERT_W() triggering the invoke.
 *  \param faultTxt [in] Error message
 */
void assert_warning(uint16_t fileId, uint16_t line, const char *faultTxt)
{
    if (g_diagControl.notifyCB != NULL && g_diagControl.notifyCbChk == S_calcNotifyCbChk(g_diagControl.notifyCB))
    {
        char notifyMsg[80];
        snprintf(notifyMsg, sizeof(notifyMsg), "WARN f:%X,l:%d-%s\r", fileId, line, faultTxt);
        g_diagControl.notifyCB(lqNotifType_assertWarning, notifyMsg);
    }
}


inline void assert_brk()
{
    __asm__("BKPT 9");
}
 #pragma endregion


void lqDiag_setFaultNotif(uint16_t notifCd, const char *notifMsg)
{
    g_diagControl.diagnosticInfo.notifType = notifCd;
    memcpy(g_diagControl.diagnosticInfo.notifMsg, notifMsg, 20);
}


void lqDiag_setApplPState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.commState = pstate;
}


void lqDiag_setNtwkPState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.ntwkState = pstate;
}

void lqDiag_setPhysPState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.signalState = pstate;
}
