#include <lq-types.h>
#include <lq-assert.h>
#include <stdio.h>

static assertControl_t g_assertControl  __attribute__ ((section (".noinit")));


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \param notifyCallback Function pointer to the application's event notification callback function, which may or may not return.
 */
void assert_registerNotifCallback(appNotifyFunc_t notifyCallback)
{
    g_assertControl.assertMagic = assert__assertControlMagic;
    g_assertControl.notifyCB = notifyCallback;
}

/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \returns Pointer to the diagnostics information block, or NULL if no valid diagnostics available.
 */
lqDiagnosticInfo_t *assert_getDiagInfoPtr()
{
    if (g_assertControl.diagnosticInfo.diagMagic == assert__diagnosticsMagic)
        return &g_assertControl.diagnosticInfo;
    return NULL;
}


/**
 *  \brief Clear diagnostics information block.
 * 
 *  LQCloud uses this after checking diagnostics at system start. LQCloud will use reset cause and diagnostics magic value
 *  to determine if the diagnostics are relevant and sent.
 */
void assert_clearDiagnosticInfo()
{
    g_assertControl.assertMagic = assert__assertControlMagic;
    memset(&(g_assertControl.diagnosticInfo), 0, sizeof(lqDiagnosticInfo_t));       // clear diagnostics, magic == 0 is no diags available
}


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
    if (g_assertControl.diagnosticInfo.diagMagic != assert__diagnosticsMagic)
    {
        g_assertControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
        g_assertControl.diagnosticInfo.pc = pc;
        g_assertControl.diagnosticInfo.lr = lr;
        g_assertControl.diagnosticInfo.line = line;
        g_assertControl.diagnosticInfo.fileId = fileId;
    }
    if (g_assertControl.assertMagic == assert__assertControlMagic && g_assertControl.notifyCB != NULL)
    {
        char notifyMsg[40];
        snprintf(notifyMsg, sizeof(notifyMsg), "ASSERT f:%d,l:%d-pc=%d,lr=%d", fileId, line, pc, lr);
        g_assertControl.notifyCB(lqNotificationType_assertFailed, notifyMsg);
    }
    g_assertControl.assertMagic = assert__assertControlMagic;
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
    if (g_assertControl.assertMagic == 0xAC && g_assertControl.notifyCB != NULL)
    {
        char notifyMsg[80];
        snprintf(notifyMsg, sizeof(notifyMsg), "WARN f:%X,l:%d-%s\r", fileId, line, faultTxt);
        g_assertControl.notifyCB(lqNotificationType_assertWarning, notifyMsg);
    }
}


inline void assert_brk()
{
    __asm__("BKPT 9");
}
