/******************************************************************************
 *  \file lq-cnfgPkgSrvcClient.cpp
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2021 LooUQ Incorporated.
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
 * Integrated flash and cloud service for management of configuration packages.
 *****************************************************************************/

#include <lq-pkgSrvcClient.h>


lq_configPkgSrvc::lq_configPkgSrvc(const char *pkgSrvcHost, uint16_t pkgSrvcPort, lq_flashDictionary *flashDictionary)
{
    strncpy(_pkgSrvcHost, pkgSrvcHost, 128);
    _pkgSrvcPort = pkgSrvcPort;
    _flashDictionary = flashDictionary;
}


lq_configPkgSrvc::~lq_configPkgSrvc() {}


/* Public Member Methods
 ----------------------------------------------------------------------------------------------- */

 /*
    tinycrypt Usage:      1) call tc_aes128_set_encrypt/decrypt_key to set the key.
 *                        2) call tc_aes_encrypt/decrypt to process the data. */

configPkgSrvcResult_t lq_configPkgSrvc::CheckFetch(uint16_t pkgKey, void *pkgStruct, size_t pkgStructSz, pkgStructParser_t parser)
{
    // fetch config pkg locally to check for: existance, version

    // configPkgSrvcResult_t result = {0};

    // if (_flashDictionary->find(LOOUQ_FLASHDICTKEY__LQCDEVICE, pkgStruct, pkgStructSz) == FLASHDICT_KEYNOTFOUND)
    // {


    // }
    // else
    // {


    // }
    // return result;
}

// static function to perform pkgr command with service, decrypt package, verify magic 