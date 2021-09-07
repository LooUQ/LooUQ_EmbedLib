# 1 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
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
# 29 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
// debugging output options             // LTEm1c will satisfy PRINTF references with empty definition if not already resolved

    asm(".global _printf_float"); // forces build to link in float support for printf



# 36 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 2





# 42 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 2
# 43 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 2
# 44 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 2


// Flash Services
# 48 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 2
# 49 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 2
# 50 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 2




Adafruit_FlashTransport_SPI flashTransport(SS1, SPI1);



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
    0x100C, // .magicFlag
    100, // .pageKey
    "https:\\LooUQ.com", // .url
    "1234567890", // .dvcId
    "myLooUQdvc" // .dvcLabel
};

testStruct_t readStruct;
uint16_t pageKey = 100;
uint16_t pageIndx;
uint8_t loopcnt = 0;


void setup()
{
# 97 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
    rtt_printf(16, ("LQFlash getJEDECID Test\r\n"));
    pinMode((13u), (0x1));

    // Initialize flash library and check its chip ID.
    if (flash.begin())
    {
        rtt_printf(0, ("Flash JEDEC ID: 0x%x\r"), flash.getJEDECID());
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
    if (pageIndx == 0xFFFF)
    {
        rtt_printf(17, ("Device config key=%d was not found. Creating now...\r"), pageKey);
        writeStruct.pageKey = pageKey;
        if (flashDict.write(pageKey, &writeStruct, sizeof(testStruct_t), true) == 0xFFFF)
        {
            rtt_printf(16, ("Error writing settings for key=%d.\r"), pageKey);
            while (1) {}
        }
    }

    rtt_printf(11, ("Write settings for key=%d\r"), pageKey);
    rtt_printf(10, ("   Device ID = %s\r"), writeStruct.dvcId);
    rtt_printf(10, ("Device Label = %s\r"), writeStruct.dvcLabel);
    rtt_printf(10, ("       Magic = %x\r"), writeStruct.magicFlag);
    rtt_printf(10, ("         Key = %d\r"), writeStruct.pageKey);


    pageIndx = flashDict.find(pageKey, 
# 135 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino" 3 4
                                      __null
# 135 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-FlashDictionary\\examples\\BasicWriteRead\\basicWriteRead.ino"
                                          , 0);
    if (pageIndx != 0xFFFF && flashDict.read(pageIndx, &readStruct, sizeof(testStruct_t)))
    {
        rtt_printf(11, ("Read back settings for key=%d.\r"), pageKey);
        rtt_printf(10, ("   Device ID = %s\r"), readStruct.dvcId);
        rtt_printf(10, ("Device Label = %s\r"), readStruct.dvcLabel);
        rtt_printf(10, ("       Magic = %x\r"), readStruct.magicFlag);
        rtt_printf(10, ("         Key = %d\r"), readStruct.pageKey);
    }

    if (flashDict.read(pageIndx-1, &readStruct, sizeof(testStruct_t)))
    {
        rtt_printf(21, ("Index=%d, expecting previous key=%d\r"), pageIndx-1, pageKey-1);
        rtt_printf(20, ("Device ID (prev) = %s\r"), readStruct.dvcId);
        rtt_printf(20, ("    Device Label = %s\r"), readStruct.dvcLabel);
        rtt_printf(20, ("           Magic = %x\r"), readStruct.magicFlag);
        rtt_printf(20, ("             Key = %d\r"), readStruct.pageKey);
    }

    delay(2000);
    loopcnt++;
    if (loopcnt > 20)
        while (true) {}
    pageKey++;
    rtt_printf(15, ("\rNext page key will be %d at address: %d\r"), pageKey, pageIndx*256);
}
