/******************************************************************************
 *  \file lq-cBuffer.c
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

#include "lq-cBuffer.h"
#include "lq-diagnostics.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define CBFFR_WRAPPED (cbffr->head < cbffr->tail)
#define CBFFR_RIGHTEDGE ((CBFFR_WRAPPED) ? cbffr->bufferEnd : cbffr->head)
#define CBFFR_SIZE (cbffr->bufferEnd - cbffr->buffer)

// effective "tail" not to be overritten by head increments. It is the tail or (rollback) rbTail 
#define CBFFR_VTAIL ((cbffr->rbTail == NULL) ? cbffr->tail : cbffr->rbTail)

#define CBFFR_FILLCNT ((cbffr->head >= CBFFR_VTAIL) ? cbffr->head - CBFFR_VTAIL : CBFFR_SIZE - (CBFFR_VTAIL - cbffr->head)) 
#define CBFFR_OPENCNT  ((cbffr->head >= CBFFR_VTAIL) ? CBFFR_SIZE - (cbffr->head - CBFFR_VTAIL) - 1 : CBFFR_VTAIL - cbffr->head - 1) 


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


void cbffr_init(cBuffer_t *cbffr, char *rawBuffer, uint16_t bufferSz)
{
    cbffr->buffer = rawBuffer;
    cbffr->bufferEnd = rawBuffer + bufferSz;
    cbffr->head = rawBuffer;
    cbffr->tail = rawBuffer;

    cbffr->rbTail = NULL;
    // cbffr->offerAddr = NULL;
    // cbffr->offerLen = false;
}


void cbffr_reset(cBuffer_t *cbffr)
{
    cbffr->head = cbffr->buffer;
    cbffr->tail = cbffr->buffer;
    cbffr->rbTail = NULL;
    // cbffr->offerLen = 0;
    // cbffr->offerAddr = NULL;

    // temporary, not technically required just makes diag a little easier
    memset(cbffr->buffer, 0, CBFFR_SIZE);
}


void cbffr_getMacros(cBuffer_t *cbffr, char *macros)
{
    /*
        #define CBFFR_WRAPPED (cbffr->head < cbffr->tail)
        #define CBFFR_RIGHTEDGE ((CBFFR_WRAPPED) ? cbffr->bufferEnd : cbffr->head)
        #define CBFFR_SIZE (cbffr->bufferEnd - cbffr->buffer)

        // effective "tail" not to be overritten by head increments. It is the tail or (rollback) rbTail 
        #define CBFFR_VTAIL ((cbffr->rbTail == NULL) ? cbffr->tail : cbffr->rbTail)

        #define CBFFR_FILLCNT ((cbffr->head >= CBFFR_VTAIL) ? cbffr->head - CBFFR_VTAIL : CBFFR_SIZE - (CBFFR_VTAIL - cbffr->head)) 
        #define CBFFR_OPENCNT  ((cbffr->head >= CBFFR_VTAIL) ? CBFFR_SIZE - (cbffr->head - CBFFR_VTAIL) - 1 : CBFFR_VTAIL - cbffr->head - 1) 
    */
    snprintf(macros, "CBFFR MACROS: size=%d, isWrap=%d, right=%p, vtail=%p, fill=%d, open=%d\r\r", CBFFR_SIZE, CBFFR_WRAPPED, CBFFR_RIGHTEDGE, CBFFR_VTAIL, CBFFR_FILLCNT, CBFFR_OPENCNT);
}

/**
 * @brief Get number of characters occupying the buffer.
 */
uint16_t cbffr_getCapacity(cBuffer_t *cbffr)
{
    return CBFFR_SIZE - 1;
}


/**
 * @brief Get number of characters occupying the buffer.
 */
uint16_t cbffr_getFillCnt(cBuffer_t *cbffr)
{
    return CBFFR_FILLCNT;
}


/**
 * @brief Get number of available space in the buffer.
 */
uint16_t cbffr_getOpenCnt(cBuffer_t *cbffr)
{
    return CBFFR_OPENCNT;
}


/* Basic push/pop/find/grab
 ----------------------------------------------------------------------------------------------- */

uint16_t cbffr_push(cBuffer_t *cbffr, const char *src, uint16_t srcSz)
{
    uint16_t pushCnt = MIN(CBFFR_OPENCNT, srcSz);                           // effective count
    uint16_t copiedCnt = pushCnt;

    if (CBFFR_WRAPPED)                                                      // ** wrapped
    {
        uint16_t rCopied = MIN(pushCnt, cbffr->bufferEnd - cbffr->head);                // 1st copy: right-side of buffer
        memcpy(cbffr->head, src, rCopied);
        cbffr->head += rCopied;
        pushCnt -= rCopied;

        if (pushCnt > 0) {                                                              // 2nd copy: left-side of buffer
            memcpy(cbffr->buffer, src + rCopied, pushCnt);
            cbffr->head = cbffr->buffer + pushCnt;                                      // remaining:pushCnt offset from start of raw buffer
        }
    }
    else                                                                    // ** linear copy
    {
        memcpy(cbffr->head, src, pushCnt);
        cbffr->head += pushCnt;
    }
    if (cbffr->head == cbffr->bufferEnd) {
        cbffr->head = cbffr->buffer;                                        // edge case: push in exactly fit, move head to buffer start
    }
    return copiedCnt;
}


uint16_t cbffr_pop(cBuffer_t *cbffr, char *dest, uint16_t requestSz)
{
    uint16_t popCnt = CBFFR_FILLCNT;
    uint16_t copiedCnt = popCnt;

    if (CBFFR_WRAPPED)                                                  // ** wrapped
    {
        uint16_t rCopied = MIN(requestSz, cbffr->bufferEnd - cbffr->tail);            // get right-side of buffer
        memcpy(dest, cbffr->tail, rCopied);
        popCnt -= rCopied;
        cbffr->tail += rCopied;

        if (popCnt > 0)                                                  // get left-side of buffer
        {
            memcpy(dest + rCopied, cbffr->buffer, popCnt);
            cbffr->tail = cbffr->buffer + popCnt;
        }
    }
    else                                                                // ** linear
    {
        popCnt = MIN(requestSz, CBFFR_FILLCNT);
        memcpy(dest, cbffr->tail, popCnt);
        cbffr->tail += popCnt;
    }
    if (cbffr->tail == cbffr->bufferEnd)                                // edge case: pop exhausted right-side, move tail to buffer start
        cbffr->tail = cbffr->buffer;

    return copiedCnt;
}


/**
 * @brief Pop a number of chars from buffer at buffer-tail, leaving the leaveSz number of chars in buffer.
 * @details This allows for simplified trailer parsing. Number of characters "popped" will be the lesser of available and requestSz.
 */
uint16_t cbffr_popLeave(cBuffer_t *cbffr, char *dest, uint16_t requestSz, uint16_t leaveSz)
{
    if (CBFFR_FILLCNT <= leaveSz)
        return 0;
    else
    {
        uint16_t popSz = 0;
        popSz = MIN((CBFFR_FILLCNT - leaveSz), requestSz);
        return cbffr_pop(cbffr, dest, popSz);
    }
}


/**
 * @brief Peeks a number of chars ahead in the buffer starting at buffer-tail. 
 * @details Number of characters "peeked" will be the lesser of available and requestSz.
 */
uint16_t cbffr_peek(cBuffer_t *cbffr, char *dest, uint16_t requestSz)
{
    uint16_t available = CBFFR_FILLCNT;
    uint16_t peeked;

    if (!CBFFR_WRAPPED)                                              // ** linear
    {
        peeked = MIN(available, requestSz);
        memcpy(dest, cbffr->tail, peeked);
    }
    else                                                                // ** wrapped
    {
        peeked = MIN(requestSz, cbffr->bufferEnd - cbffr->tail);            // get right-side of buffer
        memcpy(dest, cbffr->tail, peeked);
        available -= peeked;                                                // peeked number of chars serviced from request 

        if (available > 0)                                                  // get left-side of buffer
        {
            memcpy(dest + peeked, cbffr->buffer, available);
            peeked += available;                                            // add 2nd copy back to report request serviced to caller
        }
    }
    return peeked;
}


/**
 * @brief Find needle in the buffer, searching forward from the buffer-tail to buffer-head.
 */
uint16_t cbffr_find(cBuffer_t *cbffr, const char *pNeedle, int16_t searchOffset, uint16_t searchWindowSz, bool setTail)
{
    uint8_t needleLen = strlen(pNeedle);
    uint8_t matchedCnt = 0;                                     // chars matched, 
    uint8_t rightSideLen = 0;
    char *searchPtr;

    /* Where to start searching
     */
    if (searchOffset == 0)
        searchPtr = cbffr->tail;
    else
    {
        if (searchOffset > 0)                                               // (+) skip ahead from tail
        {
            if (CBFFR_WRAPPED) {
                uint16_t rightside = MIN(cbffr->bufferEnd - cbffr->tail, searchOffset);
                searchPtr = MIN(cbffr->buffer + rightside, cbffr->head);
            }
            else {
                searchPtr = MIN(cbffr->tail + searchOffset, cbffr->head);
            }
        }
        else                                                                // (-) skip back from head
        {
            searchOffset *= -1;                                                 // make positive
            if (CBFFR_WRAPPED) {
                searchPtr = MAX(cbffr->head - searchOffset, cbffr->tail);
            }
            else {
                uint16_t leftSide = MAX(cbffr->head - cbffr->buffer, searchOffset);
                searchPtr = cbffr->bufferEnd - (searchOffset - leftSide);
            }
        }
    }

    /* Search right-side (start to buffer wrap)
     */
    rightSideLen = needleLen;
    while (searchPtr < CBFFR_RIGHTEDGE && matchedCnt < needleLen)
    {
        if (memcmp(searchPtr, pNeedle, rightSideLen - matchedCnt) == 0)
        {
            if (matchedCnt == needleLen)                        // found full needle before right-edge
            {
                if (setTail)
                    cbffr->tail = searchPtr;
                return searchPtr - cbffr->tail;                 // return offset from tail
            }
            else
                break;                                          // found start of needle
        }
        searchPtr++;
        rightSideLen = MIN(CBFFR_RIGHTEDGE - searchPtr, needleLen);
    }

    if (!CBFFR_WRAPPED)                                                 // if not wrapped, no further opportunity to match
        return CBFFR_NOFIND;

    /* Handle partial match at right edge, remaining must be at left edge before head
     */
    if (matchedCnt > 0)                                                 // partial match at right edge
    {
        if (memcmp(cbffr->buffer, pNeedle + matchedCnt, needleLen - matchedCnt) == 0)
        {
            if (setTail)
                cbffr->tail = searchPtr;
            return searchPtr - cbffr->tail;
        }
        else
        {
            searchPtr = cbffr->buffer;                                  // moving to left side
            matchedCnt = 0;                                             // any match from right side is orphaned, start over
        }
    }

    /* Continue searching left-side (wrap point to head)
     */
    while (searchPtr < cbffr->head)
    {
        if (memcmp(searchPtr, pNeedle, needleLen) == 0)
        {   
            if (setTail)
                cbffr->tail = searchPtr;
            return (searchPtr - cbffr->buffer) + (cbffr->bufferEnd - cbffr->tail);
        }
        searchPtr++;
    }
    return CBFFR_NOFIND;
}


/**
 * @brief Get length of contiguous block of chars from tail. Distance to head or buffer-wrap.
 */
void cbffr_getHeadBlock(cBuffer_t *cbffr, char **head, uint16_t *length)
{
    *head = cbffr->head;
    if (CBFFR_WRAPPED)
        *length = cbffr->tail - cbffr->head - 1;
    else
    {
        *length = cbffr->bufferEnd - cbffr->head - 1;
    }
}


/**
 * @brief Get length of contiguous block of chars from tail. Distance to head or buffer-wrap.
 */
void cbffr_getTailBlock(cBuffer_t *cbffr, char **tail, uint16_t *length)
{
    *tail = cbffr->tail;
    if (CBFFR_WRAPPED)
        *length = cbffr->bufferEnd - cbffr->tail;
    else
        *length = cbffr->head - cbffr->tail;
}


/**
 * @brief Advances the buffer's head (incoming) by the requested number of chars.
 */
void cbffr_skipHead(cBuffer_t *cbffr, uint16_t skipCnt)
{
    if (CBFFR_WRAPPED) {                                                        // ** wrapped
        cbffr->head = MIN(cbffr->head + skipCnt, cbffr->tail-1);
    }
    else 
    {                                                                       // ** linear
        if (cbffr->head + skipCnt < cbffr->bufferEnd)
            cbffr->head += skipCnt;
        else if (cbffr->head + skipCnt == cbffr->bufferEnd)
            cbffr->head = cbffr->buffer;
        else {
            uint16_t leftSkip = skipCnt - (cbffr->bufferEnd - cbffr->head);
            cbffr->head = MIN(cbffr->head + leftSkip, cbffr->tail-1);
        }
    }
}


/**
 * @brief Advances the buffer's tail (outgoing) by the requested number of chars.
 */
void cbffr_skipTail(cBuffer_t *cbffr, uint16_t skipCnt)
{
    uint16_t skipped;

    if (!CBFFR_WRAPPED)                                                      // ** linear
    {
            cbffr->tail = MIN(cbffr->tail + skipCnt, cbffr->head);
    }
    else                                                                        // ** wrapped
    {
            skipped = MIN(skipCnt, cbffr->bufferEnd - cbffr->tail);                 // right-side
            cbffr->tail = MIN(cbffr->buffer + (skipCnt - skipped), cbffr->head);    // consume remaining to skip, up to buffer-head
    }
}


// /**
//  * @brief Grab a block of characters from buffer without affecting buffer controls (tail is uneffected).
//  */
// uint16_t cbffr_grab(cBuffer_t *cbffr, uint16_t grabIndex, char *dest, uint16_t requestSz)
// {
//     uint16_t copied;

//     if (!CBFFR_WRAPPED)                                              // ** linear
//     {
//         copied = MIN(requestSz, CBFFR_FILLCNT);
//         memcpy(dest, cbffr->buffer + grabIndex, copied);
//     }
//     else                                                                // ** wrapped
//     {
//         copied = MIN(requestSz, cbffr->bufferEnd - cbffr->tail);               // get right-side of buffer
//         memcpy(dest, cbffr->buffer + grabIndex, copied);
//         requestSz -= copied;

//         if (requestSz > 0)                                                     // get left-side of buffer
//         {
//             memcpy(dest + copied, cbffr->buffer, requestSz);
//             copied += requestSz;                                               // add back to report to caller
//         }
//     }
//     return copied;
// }


/* offers
 ----------------------------------------------------------------------------------------------- */

// cBffrOffer_t cbffr_getOffer(cBuffer_t *cbffr, uint16_t requestSz)
// {
//     cBffrOffer_t offer;

//     if (!cbffr->offerAddr)
//     {
//         if (!CBFFR_WRAPPED)                                              // ** linear
//         {
//             offer.length = MIN(requestSz, cbffr->bufferEnd - cbffr->head);
//         }
//         else                                                                // ** wrapped
//         {
//             offer.length = MIN(requestSz, cbffr->tail - cbffr->head - 1); 
//         }
//         offer.address = cbffr->head;                                        // current head (requester will copy in here)
//     }
//     return offer;
// }


// void cbffr_takeOffer(cBuffer_t *cbffr)
// {
//     cbffr->head += cbffr->offerLen;                                     // update internal head pointer, requester copied in
//     if (cbffr->head == cbffr->bufferEnd)
//         cbffr->head = cbffr->buffer;                                    // if right-side offer, swing head around to buffer start 
//     cbffr->offerAddr = NULL;
//     cbffr->offerLen = 0;
// }


// void cbffr_releaseOffer(cBuffer_t *cbffr)                                 // Important: push is blocked by an outstanding offer
// {
//     cbffr->offerAddr = NULL;                                            // discard offer
//     cbffr->offerLen = 0;
// }


/* Transaction
 ----------------------------------------------------------------------------------------------- */

bool cbffr_startTransaction(cBuffer_t *cbffr)
{
    if (cbffr->rbTail == NULL)
    {
        cbffr->rbTail = cbffr->tail;
        return true;
    }
    return false;
}


void cbffr_commitTransaction(cBuffer_t *cbffr)
{
    cbffr->tail = cbffr->rbTail;
    cbffr->rbTail = NULL;
}


void cbffr_rollbackTransaction(cBuffer_t *cbffr)
{
    cbffr->rbTail = NULL;
}

