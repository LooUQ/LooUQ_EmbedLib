/******************************************************************************
 *  \file lq-flashDictionary.h
 *  \author Greg Terrell
 *
 *  \copyright Copyright (c) 2021 LooUQ Incorporated. 
 *
 ******************************************************************************
 * Non-filesystem based structured storage to flash
 *****************************************************************************/

#ifndef __LQ_FLASHDICTIONARY_H__
#define __LQ_FLASHDICTIONARY_H__

#include <Adafruit_SPIFlashBase.h>
#include <Adafruit_FlashTransport.h>

#define FLASHDICT_MAGIC 0x452B
#define FLASHDICT_KEYNOTFOUND 404
#define FLASHDICT_BADREQUEST 400
#define FLASHDICT_SECTOR_SZ 4096


class lq_flashDictionary
{
public:
    lq_flashDictionary(Adafruit_SPIFlashBase *flash, uint32_t baseAddr = 0, uint32_t flashAllocated = 8192, uint32_t pageSz = 256);
    ~lq_flashDictionary();

    uint16_t write(uint16_t pkgKey, void *pkgStruct, uint16_t structSz, bool autoCreate);
    uint16_t readByKey(uint16_t pkgKey, void *pkgStruct, uint16_t structSz);
    uint16_t readByIndex(uint16_t pkgIndx, void *pkgStruct, uint16_t structSz);
    void erase(uint16_t key);
    void eraseAll();

private:
    Adafruit_SPIFlashBase *_flash;
    uint32_t _baseAddr;
    uint16_t _pkgSize;
    uint16_t _pkgCount;
    uint16_t _sectorCount;
    uint16_t _availablePkgIndx;

    uint32_t sectorAddr(uint16_t pkgIndx);
    uint16_t pkgOffset(uint16_t pkgIndx);
};

#endif  /* !__LQ_FLASHDICTIONARY_H__ */