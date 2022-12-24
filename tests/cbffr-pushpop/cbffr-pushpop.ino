/******************************************************************************
 *  \file cbffr-pushpop.ino
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2022 LooUQ Incorporated.
 *  www.loouq.com
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
 * Test for basic operation of the cbffr block circular buffer. 
 *****************************************************************************/

#define _DEBUG 2                        // set to non-zero value for PRINTF debugging output, 
// debugging output options             // LTEm1c will satisfy PRINTF references with empty definition if not already resolved
#if defined(_DEBUG)
    asm(".global _printf_float");       // forces build to link in float support for printf
    #if _DEBUG == 2
    #include <jlinkRtt.h>               // output debug PRINTF macros to J-Link RTT channel
    #define PRINTF(c_,f_,__VA_ARGS__...) do { rtt_printf(c_, (f_), ## __VA_ARGS__); } while(0)
    #else
    #define SERIAL_DBG _DEBUG           // enable serial port output using devl host platform serial, _DEBUG 0=start immediately, 1=wait for port
    #endif
#else
#define PRINTF(c_, f_, ...) ;
#endif

#include <lq-cBffr.h>

cBffr_t cBuffer;
char rawBuffer[32];

char *test_1 = "LooUQ in TC.";
char *test_2 = "1234567890";
char *test = test_1;
char *grabAt;
char bmap[33];
char *legend = "01234567890123456789012345678901";
char outBffr[40];

void setup() {
    #ifdef SERIAL_OPT
        Serial.begin(115200);
        #if (SERIAL_OPT > 0)
        while (!Serial) {}      // force wait for serial ready
        #else
        delay(5000);            // just give it some time
        #endif
    #endif

    cbffr_init(&cBuffer, rawBuffer, sizeof(rawBuffer));
    cbffr_push(&cBuffer, test_1, strlen(test_1));

    grabAt = cbffr_find(&cBuffer, "in", 0, 0);
    cbffr_grab(&cBuffer, grabAt, outBffr, sizeof(outBffr));

    // overflow disallowed
    PRINTF(0, "Verify overflow disallowed (push #3 rslt=0)\r");
    for (size_t i = 2; i < 4; i++)
    {
        bool rslt = cbffr_push(&cBuffer, test_1, strlen(test_1));
        PRINTF(0, "push #%d rslt=%d \r", i, rslt);
        if (!rslt && i < 3)
        {
            PRINTF(dbgColor__red, "Bad, push failed!");
            while (1){}
        }
    }
    PRINTF(0, "\r");


    // can't pop an empty buffer
    PRINTF(0, "Verify can't pop an empty buffer (pop#1 empty)\r");
    memset(outBffr, 0, sizeof(outBffr));                        // clear for visibility
    for (size_t i = 0; i < 2; i++)
    {
        uint8_t popped = cbffr_pop(&cBuffer, outBffr, sizeof(outBffr));
        PRINTF(0, "pop #%d got %d chars\r", i, popped);
        PRINTF(0, "post pop out:%s\r", outBffr);
    }
    PRINTF(0,"\r");

    cbffr_reset(&cBuffer);
    memset(outBffr, 0, sizeof(outBffr));                        // clear for visibility
    memset(bmap, 0, sizeof(bmap));

    PRINTF(dbgColor__cyan, "available=%d\r", cbffr_getAvailable(&cBuffer));

}
//"Q inQ in TC.LooUQ in TC.LooUQ in TC.LooUQ in TC.Q in TC.LooUQ in TC.LooU"


uint16_t loopcnt = 1;
uint8_t popSz = 9;

void loop() 
{
    uint16_t c = 0;
    PRINTF(dbgColor__cyan, "\r\rPass:%d\r", loopcnt);

    if (cbffr_push(&cBuffer, test, strlen(test)))
        PRINTF(dbgColor__cyan, "^ ");
        // PRINTF(dbgColor__white, "Pushed: \"%s\" (%d), available=%d\r", test_1, strlen(test_1), cbffr_getAvailable(&cBuffer));
    else
        PRINTF(dbgColor__cyan, "- ");
        // PRINTF(dbgColor__warn, "Can't push now\r");

    PRINTF(dbgColor__white, "push \"%s\"                available (-%d) = %d\r", test, strlen(test), cbffr_getAvailable(&cBuffer));

    memset(bmap, ' ', sizeof(bmap)-1);
    bmap[cBuffer.tail - cBuffer.buffer] = 't';
    bmap[cBuffer.head - cBuffer.buffer] = 'h';
    c = (cBuffer.head >= cBuffer.buffer && cBuffer.head < cBuffer.bufferEnd) ? dbgColor__green : dbgColor__warn;
    PRINTF(dbgColor__dGreen, "        %s   B=%p\\%p \r", legend, cBuffer.buffer, cBuffer.bufferEnd);
    PRINTF(c, "       \"%s\"  H=%p\r", bmap, cBuffer.head);
    memcpy(outBffr, cBuffer.buffer, cBuffer.bufferSz);
    PRINTF(0, "       \"%s\"", outBffr);

    PRINTF(0,"\r\r");

    memset(outBffr, 0, sizeof(outBffr));
    uint8_t popcnt = cbffr_pop(&cBuffer, outBffr, popSz);
    PRINTF(dbgColor__white, "  pop  \"%s\"                available (+%d) = %d\r", outBffr, strlen(outBffr), cbffr_getAvailable(&cBuffer));

    memset(bmap, ' ', sizeof(bmap)-1);
    bmap[cBuffer.tail - cBuffer.buffer] = 't';
    bmap[cBuffer.head - cBuffer.buffer] = 'h';
    c = (cBuffer.tail >= cBuffer.buffer && cBuffer.tail < cBuffer.bufferEnd) ? dbgColor__green : dbgColor__warn;
    PRINTF(dbgColor__dGreen, "        %s   B=%p\\%p \r", legend, cBuffer.buffer, cBuffer.bufferEnd);
    PRINTF(c, "       \"%s\"  T=%p\r", bmap, cBuffer.tail);
    memcpy(outBffr, cBuffer.buffer, cBuffer.bufferSz);
    PRINTF(0, "       \"%s\"", outBffr);

    delay(2000);
    loopcnt++;
    // PRINTF(dbgColor__magenta, "\rFreeMem=%u  <<Loop=%d>>\r", getFreeMemory(), loopCnt);
}
