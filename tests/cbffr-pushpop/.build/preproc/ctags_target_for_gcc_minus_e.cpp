# 1 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-EmbedDvc\\tests\\cbffr-pushpop\\cbffr-pushpop.ino"
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
# 30 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-EmbedDvc\\tests\\cbffr-pushpop\\cbffr-pushpop.ino"
// debugging output options             // LTEm1c will satisfy PRINTF references with empty definition if not already resolved

    asm(".global _printf_float"); // forces build to link in float support for printf

# 35 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-EmbedDvc\\tests\\cbffr-pushpop\\cbffr-pushpop.ino" 2
# 43 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-EmbedDvc\\tests\\cbffr-pushpop\\cbffr-pushpop.ino"
# 44 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-EmbedDvc\\tests\\cbffr-pushpop\\cbffr-pushpop.ino" 2

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
# 66 "c:\\Users\\GregTerrell\\Documents\\CodeDev\\Arduino\\libraries\\LooUQ-EmbedDvc\\tests\\cbffr-pushpop\\cbffr-pushpop.ino"
    cbffr_init(&cBuffer, rawBuffer, sizeof(rawBuffer));
    cbffr_push(&cBuffer, test_1, strlen(test_1));

    grabAt = cbffr_find(&cBuffer, "in", 0, 0);
    cbffr_grab(&cBuffer, grabAt, outBffr, sizeof(outBffr));

    // overflow disallowed
    do { rtt_printf(0, ("Verify overflow disallowed (push #3 rslt=0)\r")); } while(0);
    for (size_t i = 2; i < 4; i++)
    {
        bool rslt = cbffr_push(&cBuffer, test_1, strlen(test_1));
        do { rtt_printf(0, ("push #%d rslt=%d \r"), i, rslt); } while(0);
        if (!rslt && i < 3)
        {
            do { rtt_printf(dbgColor__red, ("Bad, push failed!")); } while(0);
            while (1){}
        }
    }
    do { rtt_printf(0, ("\r")); } while(0);


    // can't pop an empty buffer
    do { rtt_printf(0, ("Verify can't pop an empty buffer (pop#1 empty)\r")); } while(0);
    memset(outBffr, 0, sizeof(outBffr)); // clear for visibility
    for (size_t i = 0; i < 2; i++)
    {
        uint8_t popped = cbffr_pop(&cBuffer, outBffr, sizeof(outBffr));
        do { rtt_printf(0, ("pop #%d got %d chars\r"), i, popped); } while(0);
        do { rtt_printf(0, ("post pop out:%s\r"), outBffr); } while(0);
    }
    do { rtt_printf(0, ("\r")); } while(0);

    cbffr_reset(&cBuffer);
    memset(outBffr, 0, sizeof(outBffr)); // clear for visibility
    memset(bmap, 0, sizeof(bmap));

    do { rtt_printf(dbgColor__cyan, ("available=%d\r"), cbffr_getAvailable(&cBuffer)); } while(0);

}
//"Q inQ in TC.LooUQ in TC.LooUQ in TC.LooUQ in TC.Q in TC.LooUQ in TC.LooU"


uint16_t loopcnt = 1;
uint8_t popSz = 9;

void loop()
{
    uint16_t c = 0;
    do { rtt_printf(dbgColor__cyan, ("\r\rPass:%d\r"), loopcnt); } while(0);

    if (cbffr_push(&cBuffer, test, strlen(test)))
        do { rtt_printf(dbgColor__cyan, ("^ ")); } while(0);
        // PRINTF(dbgColor__white, "Pushed: \"%s\" (%d), available=%d\r", test_1, strlen(test_1), cbffr_getAvailable(&cBuffer));
    else
        do { rtt_printf(dbgColor__cyan, ("- ")); } while(0);
        // PRINTF(dbgColor__warn, "Can't push now\r");

    do { rtt_printf(dbgColor__white, ("push \"%s\"                available (-%d) = %d\r"), test, strlen(test), cbffr_getAvailable(&cBuffer)); } while(0);

    memset(bmap, ' ', sizeof(bmap)-1);
    bmap[cBuffer.tail - cBuffer.buffer] = 't';
    bmap[cBuffer.head - cBuffer.buffer] = 'h';
    c = (cBuffer.head >= cBuffer.buffer && cBuffer.head < cBuffer.bufferEnd) ? dbgColor__green : dbgColor__warn;
    do { rtt_printf(dbgColor__dGreen, ("        %s   B=%p\\%p \r"), legend, cBuffer.buffer, cBuffer.bufferEnd); } while(0);
    do { rtt_printf(c, ("       \"%s\"  H=%p\r"), bmap, cBuffer.head); } while(0);
    memcpy(outBffr, cBuffer.buffer, cBuffer.bufferSz);
    do { rtt_printf(0, ("       \"%s\""), outBffr); } while(0);

    do { rtt_printf(0, ("\r\r")); } while(0);

    memset(outBffr, 0, sizeof(outBffr));
    uint8_t popcnt = cbffr_pop(&cBuffer, outBffr, popSz);
    do { rtt_printf(dbgColor__white, ("  pop  \"%s\"                available (+%d) = %d\r"), outBffr, strlen(outBffr), cbffr_getAvailable(&cBuffer)); } while(0);

    memset(bmap, ' ', sizeof(bmap)-1);
    bmap[cBuffer.tail - cBuffer.buffer] = 't';
    bmap[cBuffer.head - cBuffer.buffer] = 'h';
    c = (cBuffer.tail >= cBuffer.buffer && cBuffer.tail < cBuffer.bufferEnd) ? dbgColor__green : dbgColor__warn;
    do { rtt_printf(dbgColor__dGreen, ("        %s   B=%p\\%p \r"), legend, cBuffer.buffer, cBuffer.bufferEnd); } while(0);
    do { rtt_printf(c, ("       \"%s\"  T=%p\r"), bmap, cBuffer.tail); } while(0);
    memcpy(outBffr, cBuffer.buffer, cBuffer.bufferSz);
    do { rtt_printf(0, ("       \"%s\""), outBffr); } while(0);

    delay(2000);
    loopcnt++;
    // PRINTF(dbgColor__magenta, "\rFreeMem=%u  <<Loop=%d>>\r", getFreeMemory(), loopCnt);
}
