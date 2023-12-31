/** ***************************************************************************
  @file lq-arduinoSAMD_diagnostics.cpp
  @brief LooUQ application diagnostics for SAMD (ARM Cortex+) under Arduino framework.

  @author Greg Terrell, LooUQ Incorporated

  \loouq

  @warning Internal dependencies, changes only as directed by LooUQ staff.

-------------------------------------------------------------------------------

Copyright (C) 2017-2023 LooUQ Incorporated

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
Also add information on how to contact you by electronic and paper mail.

**************************************************************************** */

#ifdef ARDUINO_ARCH_SAMD

#include <stdio.h>
#include <string.h>

#include <sam.h>

#include <lq-types.h>
#include <lq-diagnostics.h>

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


static uint8_t S__calcNotifyCbChk(appEvntNotify_func notifCbAddr)
{
    uint32_t validator = (uint32_t)notifCbAddr;

    uint8_t result =  validator & 0xFF;
    result = result ^ (validator >> 4) & 0xFF;
    result = result ^ (validator >> 12) & 0xFF;
    result = result ^ (validator >> 20) & 0xFF;
    return result;
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \param notifyCallback Function pointer to the application's event notification callback function, which may or may not return.
 */
void lqDiag_setNotifyCallback(appEvntNotify_func applNotifyCallback)
{
    //g_diagControl.assertMagic = assert__assertControlMagic;
    g_diagControl.notifyCB = applNotifyCallback;
    g_diagControl.notifyCBChk = S__calcNotifyCbChk(applNotifyCallback);      // since this is in a non-initialized block, calculate a validation check
}


void lqDiag_setResetCause(uint8_t core, uint32_t resetcause)
{
    g_diagControl.diagnosticInfo.rcause[core] = resetcause;
    
    g_diagControl.diagnosticInfo.boots += diag__bootSafe ? 0 : 1;   // no boot INC if bootSafe

    //if (resetcause == diagRcause_watchdog || resetcause == diagRcause_system)
    if (resetcause == diagRcause_watchdog)
    {
        g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;          // diagnostics reportable start
    }
    else                                                                            // if non-reportable reset, clear diagnostics except bootLoops detection
    {
        memset(&g_diagControl.diagnosticInfo, 0, sizeof(diagnosticInfo_t));
    }
}


void inline lqDiag_setBootSafe()
{
    memset(&g_diagControl.diagnosticInfo, 0, sizeof(diagnosticInfo_t));
    g_diagControl.diagnosticInfo.boots = diag__bootSafe;
}


/**
 *  \brief Get diagnostic information from the device.
 * 
 *  \returns Pointer to the diagnostics information block, or NULL if no valid diagnostics available.
 */
diagnosticInfo_t *lqDiag_getDiagnosticsBlock()
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
 * @brief ASSERT function; FATAL asserts that suggest a problem in the running firmware.
 */
void lqASSERT_invoke(const char *fileTag, uint16_t line, void *pc, const void *lr)
{
    g_diagControl.diagnosticInfo.diagMagic = assert__diagnosticsMagic;
    g_diagControl.diagnosticInfo.pc = (uint32_t)pc;
    g_diagControl.diagnosticInfo.lr = (uint32_t)lr;
    g_diagControl.diagnosticInfo.line = line;
    memcpy(g_diagControl.diagnosticInfo.fileTag, fileTag, assert__fileTagSz);

    if (g_diagControl.notifyCB != NULL && g_diagControl.notifyCBChk == S__calcNotifyCbChk(g_diagControl.notifyCB))
    {
        char notifyMsg[40];
        snprintf(notifyMsg, sizeof(notifyMsg), "ASSERT f:%s,l:%d,pc=%d,lr=%d", fileTag, line, pc, lr);
        g_diagControl.notifyCB(appEvent_fault_assertFailed, notifyMsg);
    }
    lqASSERT_brk();                                                               // stop here if notify callback returned
}


/**
 * @brief ASSERT warning function; non-fatal asserts that suggest a problem in the running firmware
 * 
 * @param fileTag 
 * @param line 
 * @param faultTxt 
 */
void lqASSERT_warning(const char *fileTag, uint16_t line, const char *faultTxt)
{
    if (g_diagControl.notifyCB != NULL && g_diagControl.notifyCBChk == S__calcNotifyCbChk(g_diagControl.notifyCB))
    {
        char notifyMsg[80];
        snprintf(notifyMsg, sizeof(notifyMsg), "f:%s,l:%d -%s\r", fileTag, line, faultTxt);
        //uint8_t assm = (uint8_t)(fileTag & 0xFF00) >> 8;
        g_diagControl.notifyCB(appEvent_warn_wassertFailed, notifyMsg);
    }
}

inline void lqASSERT_brk()
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

#endif // #ifdef ARDUINO_ARCH_SAMD
