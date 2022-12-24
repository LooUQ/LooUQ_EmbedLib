/******************************************************************************
 *  \file lq-cBffr.c
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2022 LooUQ Incorporated.
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
 * LooUQ circular buffer implementation for device streaming
 *****************************************************************************/

#include "lq-cBffr.h"
#include "lq-diagnostics.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define BFFR_INUSE() ((cbffr->head >= cbffr->tail) ? cbffr->head - cbffr->tail : cbffr->bufferSz - (cbffr->tail - cbffr->head)) 
#define BFFR_AVAIL()  ((cbffr->head >= cbffr->tail) ? cbffr->bufferSz - (cbffr->head - cbffr->tail) - 1 : cbffr->tail - cbffr->head - 1) 

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

void cbffr_init(cBffr_t *cbffr, char *rawBuffer, size_t bufferSz)
{
    cbffr->buffer = rawBuffer;
    cbffr->bufferEnd = rawBuffer + bufferSz;
    cbffr->bufferSz = bufferSz;
    cbffr->head = rawBuffer;
    cbffr->tail = rawBuffer;

    cbffr->offerAddr = NULL;
    cbffr->offerLen = false;
    cbffr->tMark = NULL;
}


void cbffr_reset(cBffr_t *cbffr)
{
    cbffr->head = cbffr->buffer;
    cbffr->tail = cbffr->buffer;
    cbffr->tMark = NULL;
    cbffr->offerLen = 0;
    cbffr->offerAddr = NULL;

    // temporary
    memset(cbffr->buffer, 0, cbffr->bufferSz);
}


uint16_t cbffr_getAvailable(cBffr_t *cbffr)
{
    return BFFR_AVAIL();
}


/* Basic push/pop/find/grab
 ----------------------------------------------------------------------------------------------- */

bool cbffr_push(cBffr_t *cbffr, const char *src, size_t srcSz)
{
    //uint16_t avail = (cbffr->head >= cbffr->tail) ? cbffr->bufferSz - (cbffr->head - cbffr->tail) - 1 : cbffr->tail - cbffr->head;

    if (srcSz > BFFR_AVAIL() || cbffr->offerAddr)                       // block push on outstand (copy in) offer
        return false;

    if (cbffr->head >= cbffr->tail)                                     // ** linear copy
    {
        uint16_t copied = MIN(srcSz, cbffr->bufferEnd - cbffr->head);       // 1st copy: right-side of buffer
        memcpy(cbffr->head, src, copied);
        cbffr->head += copied;
        srcSz -= copied;

        if (srcSz > 0)                                                      // 2nd copy: left-side of buffer
        {
            memcpy(cbffr->buffer, src + copied, srcSz);
            cbffr->head = cbffr->buffer + srcSz;                            // remaining:srcSz offset from start of raw buffer
        }
        else if (cbffr->head == cbffr->bufferEnd)                           // edge case: push in exactly fit, move head to buffer start
            cbffr->head = cbffr->buffer;
    }
    else                                                                // ** wrapped
    {
        //uint16_t copied =  cbffr->tail - cbffr->head - 1; 
        memcpy(cbffr->head, src, srcSz);
        cbffr->head += srcSz;
    }
    return true;
}


uint16_t cbffr_pop(cBffr_t *cbffr, char *dest, size_t requestSz)
{
    uint16_t popped;

    if (cbffr->head >= cbffr->tail)                                     // ** linear
    {
        popped = MIN(requestSz, BFFR_INUSE());
        memcpy(dest, cbffr->tail, popped);
        cbffr->tail += popped;
    }
    else                                                                // ** wrapped
    {
        popped = MIN(requestSz, cbffr->bufferEnd - cbffr->tail);            // get right-side of buffer
        memcpy(dest, cbffr->tail, popped);
        cbffr->tail += popped;
        requestSz -= popped;

        if (requestSz > 0)                                                  // get left-side of buffer
        {
            memcpy(dest + popped, cbffr->buffer, requestSz);
            cbffr->tail = cbffr->buffer + requestSz;
            popped += requestSz;                                            // add back to report to caller
        }
        else if (cbffr->tail == cbffr->bufferEnd)                           // edge case: pop exhausted right-side, move tail to buffer start
            cbffr->tail = cbffr->buffer;
    }
    return popped;
}


uint16_t cbffr_discard(cBffr_t *cbffr, size_t requestSz)
{
    uint16_t discarded;

    if (cbffr->head >= cbffr->tail)                                     // ** linear
    {
        discarded = MIN(requestSz, BFFR_INUSE());
        cbffr->tail += discarded;
    }
    else                                                                // ** wrapped
    {
        discarded = MIN(requestSz, cbffr->bufferEnd - cbffr->tail);         // get right-side of buffer
        cbffr->tail += discarded;
        requestSz -= discarded;

        if (requestSz > 0)                                                  // get left-side of buffer
        {
            cbffr->tail = cbffr->buffer + requestSz;
            discarded += requestSz;                                         // add back to report to caller
        }
        else if (cbffr->tail == cbffr->bufferEnd)                           // edge case: pop exhausted right-side, move tail to buffer start
            cbffr->tail = cbffr->buffer;
    }
    return discarded;
}


char *cbffr_find(cBffr_t *cbffr, const char *pNeedle, uint16_t lookbackCnt, uint16_t searchCnt)
{
    uint8_t needleLen = strlen(pNeedle);
    uint8_t toMatchCnt = needleLen;
    char *searchPtr;
    uint8_t needleRemainingCnt = 0;

    if (lookbackCnt == 0)                                       // where to start searching
        searchPtr = cbffr->tail;
    else
    {
        if (cbffr->head > cbffr->tail)
            searchPtr = MAX(cbffr->head - lookbackCnt, cbffr->tail);
        else
        {
            uint16_t leftSide = MAX(cbffr->head - lookbackCnt, cbffr->buffer);
            searchPtr = cbffr->bufferEnd - (lookbackCnt - leftSide);
        }
    }

    /* Search right-side (start to buffer wrap)
     */
    while (searchPtr < cbffr->bufferEnd && toMatchCnt != 0)
    {
        if (memcmp(searchPtr, pNeedle, toMatchCnt) == 0)
        {
            needleRemainingCnt = strlen(pNeedle) - toMatchCnt;
            if (needleRemainingCnt == 0)
                return searchPtr;                               // found full needle before wrap
            else
                break;                                          // found start of needle
        }
        searchPtr++;
        toMatchCnt -= (cbffr->bufferEnd - searchPtr < toMatchCnt) ? 1 : 0;
    }

    /* Handle transition at wrap
     */
    if (needleRemainingCnt != 0)
        searchPtr = cbffr->buffer;
    else
    {
        if (memcmp(cbffr->buffer, pNeedle + needleRemainingCnt, needleLen - needleRemainingCnt) == 0)
            return searchPtr;
    }

    /* Search left-side (wrap point to head)
     */
    while (searchPtr < cbffr->head)
    {
        if (memcmp(searchPtr, pNeedle, needleLen) == 0)
        {
            return searchPtr;                                       // found needle
        }
        searchPtr++;
    }

    return NULL;
}


uint16_t cbffr_grab(cBffr_t *cbffr, const char *grabPt, char *dest, size_t destSz)
{
    uint16_t copied;

    if (cbffr->head >= cbffr->tail)                                     // ** linear
    {
        copied = MIN(destSz, BFFR_INUSE());
        memcpy(dest, grabPt, copied);
    }
    else                                                                // ** wrapped
    {
        copied = MIN(destSz, cbffr->bufferEnd - cbffr->tail);               // get right-side of buffer
        memcpy(dest, grabPt, copied);
        destSz -= copied;

        if (destSz > 0)                                                     // get left-side of buffer
        {
            memcpy(dest + copied, cbffr->buffer, destSz);
            copied += destSz;                                               // add back to report to caller
        }
    }
    return copied;
}


/* offers
 ----------------------------------------------------------------------------------------------- */

cBffrOffer_t cbffr_getOffer(cBffr_t *cbffr, uint16_t requestSz)
{
    cBffrOffer_t offer;

    if (!cbffr->offerAddr)
    {
        if (cbffr->head >= cbffr->tail)                                     // ** linear
        {
            offer.length = MIN(requestSz, cbffr->bufferEnd - cbffr->head);
        }
        else                                                                // ** wrapped
        {
            offer.length = MIN(requestSz, cbffr->tail - cbffr->head - 1); 
        }
        offer.address = cbffr->head;                                        // current head (requester will copy in here)
    }
    return offer;
}


void cbffr_takeOffer(cBffr_t *cbffr)
{
    cbffr->head += cbffr->offerLen;                                     // update internal head pointer, requester copied in
    if (cbffr->head == cbffr->bufferEnd)
        cbffr->head = cbffr->buffer;                                    // if right-side offer, swing head around to buffer start 
    cbffr->offerAddr = NULL;
    cbffr->offerLen = 0;
}


void cbffr_releaseOffer(cBffr_t *cbffr)                                 // Important: push is blocked by an outstanding offer
{
    cbffr->offerAddr = NULL;                                            // discard offer
    cbffr->offerLen = 0;
}


/* Marks
 ----------------------------------------------------------------------------------------------- */

void cbffr_setTMark(cBffr_t *cbffr)
{
    cbffr->tMark = cbffr->tail;
}


uint16_t cbffr_grabTMark(cBffr_t *cbffr, char *dest, size_t destSz)
{
    uint16_t grabbed = cbffr_grab(cbffr, cbffr->tMark, dest, destSz);
    cbffr->tMark = NULL;
    return grabbed;
}

