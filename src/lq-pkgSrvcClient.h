/******************************************************************************
 *  \file lq-cnfgPkgSrvcClient.h
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

#ifndef __LQ_CNFGPKGCLIENT_H__
#define __LQ_CNFGPKGCLIENT_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <ltemc.h>                      // *** TODO remove this dependency ***
#include <lq-flashDictionary.h>

//#include <src/ tinycrypt/aes.h>
#include <tinycrypt.h>
#include <tinycrypt/aes.h>


typedef enum configPkgSrvcResult_tag
{
    configPkgSrvcResult_packageFault = 0,
    configPkgSrvcResult_packageCurrent = 1,
    configPkgSrvcResult_packageUpdated = 2,
    configPkgSrvcResult_packageAdded = 3
} configPkgSrvcResult_t;


typedef bool pkgStructParser_t(const char *buf, void *dictionaryStruct);


class lq_configPkgSrvc
{
public: 
    lq_configPkgSrvc(const char *pkgSrvcHost, uint16_t pkgSrvcPort, lq_flashDictionary *flashDict);
    ~lq_configPkgSrvc();

    configPkgSrvcResult_t CheckFetch(uint16_t pkgKey, void *pkgStruct, size_t pkgStructSz, pkgStructParser_t parser);

private:
    char _pkgSrvcHost[128];
    uint16_t _pkgSrvcPort;
    lq_flashDictionary *_flashDictionary;
};

#endif  /* !__LQ_CNFGPKGCLIENT_H__ */
