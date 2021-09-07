
#include <lq-flashDictionary.h>


lq_flashDictionary::lq_flashDictionary(Adafruit_SPIFlashBase *flash, uint32_t baseAddr, uint32_t flashAllocated, uint32_t pkgSz)
{
    _flash = flash;
    _baseAddr = baseAddr;
    _sectorCount = flashAllocated / FLASHDICT_SECTOR_SZ;
    _pkgCount = _sectorCount * FLASHDICT_SECTOR_SZ / pkgSz;         // maximum number of config packages (cnfgPkg) slots available (based on int _sectorCount)
    _pkgSize = pkgSz;
}

lq_flashDictionary::~lq_flashDictionary() {}


/* Public Member Methods
 ----------------------------------------------------------------------------------------------- */

uint16_t lq_flashDictionary::write(uint16_t pkgKey, void *pkgStruct, uint16_t structSz, bool autoCreate)
{
    if (structSz > _pkgSize)
        return 400;

    uint8_t flashBuf[FLASHDICT_SECTOR_SZ] = {1};
    uint16_t pkgIndx = readByKey(pkgKey, pkgStruct, structSz);

    if (pkgIndx != FLASHDICT_KEYNOTFOUND)                                                   // pkg key found
    {
        _flash->readBuffer(sectorAddr(pkgIndx), flashBuf, FLASHDICT_SECTOR_SZ);                 // get prior full sector contents
        _flash->eraseSector(sectorAddr(pkgIndx));
        memcpy(flashBuf + pkgOffset(pkgIndx), pkgStruct, structSz);                             // update sector buffer prior+write for flash 
        _flash->writeBuffer(sectorAddr(pkgIndx), flashBuf, FLASHDICT_SECTOR_SZ);
        return pkgIndx;
    }
    else if (autoCreate && _availablePkgIndx != FLASHDICT_KEYNOTFOUND)                      // not found, but auto-create
    {
        _flash->readBuffer(sectorAddr(_availablePkgIndx), flashBuf, FLASHDICT_SECTOR_SZ);       // read prior sector contents. Area under page should be all ones, no erase needed
        memcpy(flashBuf + pkgOffset(_availablePkgIndx), pkgStruct, structSz);
        _flash->writeBuffer(sectorAddr(_availablePkgIndx), flashBuf, FLASHDICT_SECTOR_SZ);
        return _availablePkgIndx;
    }
    return FLASHDICT_KEYNOTFOUND;
}


uint16_t lq_flashDictionary::readByKey(uint16_t key, void *pkgStruct, uint16_t structSz)
{
    uint8_t flashBuf[FLASHDICT_SECTOR_SZ];
    uint16_t sectorIndx = 0;
    _availablePkgIndx = FLASHDICT_KEYNOTFOUND;
    uint8_t pkgsPerSector = FLASHDICT_SECTOR_SZ / _pkgSize;

    do
    {
        _flash->readBuffer(_baseAddr + (sectorIndx * FLASHDICT_SECTOR_SZ), flashBuf, FLASHDICT_SECTOR_SZ);

        for (size_t i = 0; i < pkgsPerSector; i++)
        {
            if (*(uint16_t *)(flashBuf + (i * _pkgSize)) == FLASHDICT_MAGIC &&                                      // test for header found (magic + ID)
                *(uint16_t *)(flashBuf + (i * _pkgSize) + 2) == key)
            {
                if (pkgStruct != NULL)
                    memcpy(pkgStruct, flashBuf + (i * _pkgSize), structSz);
                return sectorIndx * pkgsPerSector + i;
            }
            if (_availablePkgIndx == FLASHDICT_KEYNOTFOUND && *(uint16_t*)(flashBuf + (i * _pkgSize)) == 0xFFFF)    // simultaneously look for "open" pkg slots
                _availablePkgIndx = sectorIndx * (FLASHDICT_SECTOR_SZ / _pkgSize) + i;
        }
        sectorIndx++;
    } while (sectorIndx < _sectorCount);

    return FLASHDICT_KEYNOTFOUND;
}


uint16_t lq_flashDictionary::readByIndex(uint16_t pkgIndx, void *pkgStruct, uint16_t structSz)
{
    if (pkgIndx > _pkgCount)
        return FLASHDICT_BADREQUEST;

    uint32_t readAddr = _baseAddr + pkgIndx * _pkgSize ;
    return  (uint16_t)_flash->readBuffer(readAddr, (uint8_t *)pkgStruct, structSz);
}


void lq_flashDictionary::erase(uint16_t pkgKey)
{
    uint8_t emptyPage[_pkgSize] = {0};
    write(pkgKey, emptyPage, _pkgSize, false);
}


void lq_flashDictionary::eraseAll()
{
    for (size_t i = 0; i < _sectorCount; i++)
    {
        _flash->eraseSector(i);
    }
}


/* Private Member Methods
 ----------------------------------------------------------------------------------------------- */

uint32_t lq_flashDictionary::sectorAddr(uint16_t pkgIndx)
{
    uint32_t sectorIndx = (_pkgSize * pkgIndx) / FLASHDICT_SECTOR_SZ;
    return sectorIndx * FLASHDICT_SECTOR_SZ;
}


uint16_t lq_flashDictionary::pkgOffset(uint16_t pkgIndx)
{
    uint16_t offsetIndx = pkgIndx % (FLASHDICT_SECTOR_SZ / _pkgSize);
    return offsetIndx * _pkgSize;
}
