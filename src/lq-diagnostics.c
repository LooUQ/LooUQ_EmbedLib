#include <sam.h>
#include <lq-types.h>
#include <lq-diagnostics.h>
#include <stdio.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))


diagnosticControl_t g_diagControl  __attribute__ ((section (".noinit")));

/* For the diagControl to be preserved across reboots, it has to be protected from Loader initialization
 * The .noinit section assignment is the 1st step. The loader file (.ld extension) used to flash the 
 * MCU memory has to have a matching definion of the section. The LooUQ location for this section is immediately
 * following the .bss section (and before the .heap section). The .noinit loader section is in RAM, so it will 
 * not survive a power cycle, but does survive a reset.
 *
 * The linker scripts are included with every update to hardware packages in Arduino framework systems and the linker
 * script is unique for each MCU target. Example path for Adafruit Feather-M0-Express below.
 * 
 * >> C:\Users\GregTerrell\AppData\Local\Arduino15\packages\adafruit\hardware\samd\1.7.5\variants\feather_m0_express\linker_scripts\gcc
 * 
    // .bss section defined here

    .noinit (NOLOAD):
    {
		. = ALIGN(4);
        *(.noinit*)
        . = ALIGN(4);
    } > RAM

    // .heap section defined here

    
*/


uint8_t S_calcNotifyCbChk(uint32_t notifCbAddr)
{
    uint8_t result =  notifCbAddr & 0xFF;
    result = result ^ (notifCbAddr >> 4) & 0xFF;
    result = result ^ (notifCbAddr >> 12) & 0xFF;
    result = result ^ (notifCbAddr >> 20) & 0xFF;
    return result;
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \param notifyCallback Function pointer to the application's event notification callback function, which may or may not return.
 */
void lqDiag_registerEventCallback(appEventCallback_func appEventCallback)
{
    //g_diagControl.assertMagic = assert__assertControlMagic;
    g_diagControl.eventCB = appEventCallback;
    g_diagControl.notifCBChk = S_calcNotifyCbChk((int32_t)appEventCallback);
}


void lqDiag_setResetCause(uint8_t resetcause)
{
    g_diagControl.diagnosticInfo.rcause = resetcause;
    uint8_t boots = g_diagControl.diagnosticInfo.bootLoops + g_diagControl.diagnosticInfo.bootLoops == diag__bootSafe ? 0 : 1;   // no boot INC if bootSafe

    if (resetcause == diagRcause_watchdog || resetcause == diagRcause_system)
    {
        g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;          // diagnostics reportable start
    }
    else                                                                            // if non-reportable reset, clear diagnostics except bootLoops detection
    {
        memset(&g_diagControl.diagnosticInfo, 0, sizeof(diagnosticInfo_t));
        g_diagControl.diagnosticInfo.bootLoops = boots;
    }
}


void inline lqDiag_setBootSafe()
{
    g_diagControl.diagnosticInfo.bootLoops = diag__bootSafe;
    memset(&g_diagControl.diagnosticInfo, 0, sizeof(diagnosticInfo_t));
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \returns Pointer to the diagnostics information block, or NULL if no valid diagnostics available.
 */
diagnosticInfo_t *lqDiag_getDiagnosticsInfo()
{
    return &g_diagControl.diagnosticInfo;
}


/**
 *  \brief Set application level diagnostic information to report to host.
 */
void lqDiag_setApplicationDiagnosticsInfo(int16_t commState, int16_t ntwkState, int16_t signalState)
{
    g_diagControl.diagnosticInfo.commState = commState;         // indications: TCP/UDP/SSL connected, MQTT state, etc.
    g_diagControl.diagnosticInfo.ntwkState = ntwkState;         // indications: LTE PDP, etc.
    g_diagControl.diagnosticInfo.signalState = signalState;     // indications: rssi, etc.
}


/**
 *  \brief Set application notification information to report to host.
 */
void lqDiag_setApplicationMessage(uint8_t notifCode, const char *notifMsg)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.notifCode = notifCode;
    memcpy(g_diagControl.diagnosticInfo.notifMsg, notifMsg, diag__notifMsgSz);
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
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.pc = pc;
    g_diagControl.diagnosticInfo.lr = lr;
    g_diagControl.diagnosticInfo.line = line;
    g_diagControl.diagnosticInfo.fileId = fileId;

    if (g_diagControl.eventCB != NULL && g_diagControl.notifCBChk == S_calcNotifyCbChk(g_diagControl.eventCB))
    {
        char notifyMsg[40];
        snprintf(notifyMsg, sizeof(notifyMsg), "ASSERT f:%d,l:%d,pc=%d,lr=%d", fileId, line, pc, lr);

        uint8_t assm = (uint8_t)(fileId & 0xFF00) >> 8;
        g_diagControl.eventCB(appEvent_fault_assertFailed, notifyMsg);
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
    if (g_diagControl.eventCB != NULL && g_diagControl.notifCBChk == S_calcNotifyCbChk(g_diagControl.eventCB))
    {
        char notifyMsg[80];
        snprintf(notifyMsg, sizeof(notifyMsg), "f:%d,l:%d -%s\r", fileId, line, faultTxt);
        uint8_t assm = (uint8_t)(fileId & 0xFF00) >> 8;
        g_diagControl.eventCB(appEvent_warn_wassertFailed, notifyMsg);
    }
}

inline void assert_brk()
{
    if (WDT->CTRL.bit.ENABLE == 1)
        while (true) {}
    else
        __asm__("BKPT 9");
}

#pragma endregion


void lqDiag_setProtoState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.commState = pstate;
}


void lqDiag_setNtwkState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.ntwkState = pstate;
}


void lqDiag_setSignalState(int16_t pstate)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.signalState = pstate;
}


const char *lqDiag_setHwVersion(const char *hwVersion)
{
    // no magic set, this is informational and not an error

    memcpy(g_diagControl.diagnosticInfo.hwVersion, hwVersion, diag__hwVerSz);
    return hwVersion;
}


const char *lqDiag_setSwVersion(const char *swVersion)
{
    // no magic set, this is informational and not an error
    memcpy(g_diagControl.diagnosticInfo.swVersion, swVersion, diag__swVerSz);
    return swVersion;
}
