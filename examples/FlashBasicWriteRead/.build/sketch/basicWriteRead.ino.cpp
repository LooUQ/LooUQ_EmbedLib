#include <Arduino.h>
#line 1 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
/******************************************************************************
 *  \file basicWriteRead.ino
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
 *
 ******************************************************************************
 * Simple SPI to flash chip I/O test using LQ InfoBlocks.
 *****************************************************************************/

#define _DEBUG 2                        // set to non-zero value for PRINTF debugging output, 
// debugging output options             // LTEm1c will satisfy PRINTF references with empty definition if not already resolved
#if defined(_DEBUG) && _DEBUG > 0
    asm(".global _printf_float");       // forces build to link in float support for printf
    #if _DEBUG == 1
    #define SERIAL_DBG 1                // enable serial port output using devl host platform serial, 1=wait for port
    #elif _DEBUG == 2
    #include <jlinkRtt.h>               // output debug PRINTF macros to J-Link RTT channel
    #endif
#else
#define PRINTF(c_, f_, ...) ;
#endif

#include <lq-types.h>
#include <lqc-types.h>              // for sas compose\decompose
#include <lqc-azure.h>


// Flash Services
#include <SPI.h>                        
#include <Adafruit_SPIFlashBase.h>      // Adafruit SPI Flash extensions
#include <lq-flashDictionary.h>

#if defined(EXTERNAL_FLASH_USE_QSPI)
Adafruit_FlashTransport_QSPI flashTransport;
#elif defined(EXTERNAL_FLASH_USE_SPI)
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);
#else
#error No QSPI/SPI flash are defined on your board variant.h !
#endif
Adafruit_SPIFlashBase flash(&flashTransport);
lq_flashDictionary flashDict(&flash);


typedef struct testStruct_tag
{
    magicFlag_t magicFlag;
    fdictKey_t pageKey;
    char url[41];
    char dvcId[37];
    char dvcLabel[13];
} testStruct_t;


testStruct_t writeStruct = {
    LOOUQ_MAGIC,                        // .magicFlag
    100,                                // .pageKey
    "https:\\LooUQ.com",                // .url
    "1234567890",                       // .dvcId
    "myLooUQdvc"                        // .dvcLabel
};

testStruct_t readStruct;
uint16_t pageKey = 100;
uint16_t pageIndx;
uint8_t loopcnt = 0;


#line 86 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
void setup();
#line 114 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
void loop();
#line 86 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
void setup() 
{
    #ifdef SERIAL_DBG
        Serial.begin(115200);
        #if (SERIAL_DBG > 0)
        while (!Serial) {}      // force wait for serial ready
        #else
        delay(5000);            // just give it some time
        #endif
    #endif

    PRINTF(DBGCOLOR_red, "LQFlash getJEDECID Test\r\n");
    pinMode(LED_BUILTIN, OUTPUT);

    // Initialize flash library and check its chip ID.
    if (flash.begin()) 
    {
        PRINTF(0, "Flash JEDEC ID: 0x%x\r", flash.getJEDECID());
    }

    uint8_t eraseFirst = 1;
    if (eraseFirst)
    {
        for (size_t i = 0; i < 2; i++)
            flash.eraseSector(i);
    }
}

void loop() 
{
    pageIndx = flashDict.find(pageKey, &writeStruct, sizeof(testStruct_t));
    if (pageIndx == FLASHDICT_KEYNOTFOUND) 
    {
        PRINTF(DBGCOLOR_warn, "Device config key=%d was not found. Creating now...\r", pageKey);
        writeStruct.pageKey = pageKey;
        if (flashDict.write(pageKey, &writeStruct, sizeof(testStruct_t), true) == FLASHDICT_KEYNOTFOUND)
        {
            PRINTF(DBGCOLOR_error, "Error writing settings for key=%d.\r", pageKey);
            while (1) {}
        }
    }

    PRINTF(DBGCOLOR_magenta, "Write settings for key=%d\r", pageKey);
    PRINTF(DBGCOLOR_cyan, "   Device ID = %s\r", writeStruct.dvcId);
    PRINTF(DBGCOLOR_cyan, "Device Label = %s\r", writeStruct.dvcLabel);
    PRINTF(DBGCOLOR_cyan, "       Magic = %x\r", writeStruct.magicFlag);
    PRINTF(DBGCOLOR_cyan, "         Key = %d\r", writeStruct.pageKey);


    pageIndx = flashDict.find(pageKey, NULL, 0);
    if (pageIndx != FLASHDICT_KEYNOTFOUND && flashDict.read(pageIndx, &readStruct, sizeof(testStruct_t)))
    {
        PRINTF(DBGCOLOR_magenta, "Read back settings for key=%d.\r", pageKey);
        PRINTF(DBGCOLOR_cyan, "   Device ID = %s\r", readStruct.dvcId);
        PRINTF(DBGCOLOR_cyan, "Device Label = %s\r", readStruct.dvcLabel);
        PRINTF(DBGCOLOR_cyan, "       Magic = %x\r", readStruct.magicFlag);
        PRINTF(DBGCOLOR_cyan, "         Key = %d\r", readStruct.pageKey);
    }

    if (flashDict.read(pageIndx-1, &readStruct, sizeof(testStruct_t)))
    {
        PRINTF(DBGCOLOR_dMagenta, "Index=%d, expecting previous key=%d\r", pageIndx-1, pageKey-1);
        PRINTF(DBGCOLOR_dCyan, "Device ID (prev) = %s\r", readStruct.dvcId);
        PRINTF(DBGCOLOR_dCyan, "    Device Label = %s\r", readStruct.dvcLabel);
        PRINTF(DBGCOLOR_dCyan, "           Magic = %x\r", readStruct.magicFlag);
        PRINTF(DBGCOLOR_dCyan, "             Key = %d\r", readStruct.pageKey);
    }

    delay(2000);
    loopcnt++;
    if (loopcnt > 20)
        while (true) {}
    pageKey++;
    PRINTF(DBGCOLOR_green, "\rNext page key will be %d at address: %d\r", pageKey, pageIndx*256);
}

