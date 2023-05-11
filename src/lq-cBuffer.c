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

#define _DEBUG 2                        // set to non-zero value for PRINTF debugging output, 
// debugging output options             // LTEm1c will satisfy PRINTF references with empty definition if not already resolved
#if _DEBUG > 0
    asm(".global _printf_float");       // forces build to link in float support for printf
    #if _DEBUG == 1
    #define SERIAL_DBG 1                // enable serial port output using devl host platform serial, 1=wait for port
    #elif _DEBUG >= 2
    #include <jlinkRtt.h>               // output debug PRINTF macros to J-Link RTT channel
    #define PRINTF(c_,f_,__VA_ARGS__...) do { rtt_printf(c_, (f_), ## __VA_ARGS__); } while(0)
    #endif
#else
#define PRINTF(c_, f_, ...) ;
#endif

#include "lq-cBuffer.h"
#include "lq-diagnostics.h"


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define CBFFR_WRAPPED (cbffr->head < cbffr->tail)
#define CBFFR_RIGHTEDGE ((CBFFR_WRAPPED) ? cbffr->bufferEnd : cbffr->head)
#define CBFFR_SIZE (cbffr->bufferEnd - cbffr->buffer)
#define CBFFR_OCCUPIED ((cbffr->head >= cbffr->tail) ? cbffr->head - cbffr->tail : CBFFR_SIZE - (cbffr->tail - cbffr->head)) 
#define CBFFR_VACANT  (CBFFR_SIZE - CBFFR_OCCUPIED)




#define SRCFILE "CBF"                   // create SRCFILE (3 char) MACRO for lq-diagnostics ASSERT


void cbffr_init(cBuffer_t *cbffr, char *rawBuffer, uint16_t bufferSz)
{
    cbffr->buffer = rawBuffer;
    cbffr->bufferEnd = rawBuffer + bufferSz;
    cbffr->head = rawBuffer;
    cbffr->tail = rawBuffer;
    cbffr->rbHead = NULL;
    cbffr->pTail = NULL;
}


void cbffr_reset(cBuffer_t *cbffr)
{
    cbffr->head = cbffr->buffer;
    cbffr->tail = cbffr->buffer;
    cbffr->rbHead = NULL;
    cbffr->pTail = NULL;
    // temporary, not technically required just makes diag a little easier
    memset(cbffr->buffer, 0, CBFFR_SIZE);
}


void cbffr_GETMACROS(cBuffer_t *cbffr, char *macros)
{
    /*
        #define CBFFR_WRAPPED (cbffr->head < cbffr->tail)
        #define CBFFR_RIGHTEDGE ((CBFFR_WRAPPED) ? cbffr->bufferEnd : cbffr->head)
        #define CBFFR_SIZE (cbffr->bufferEnd - cbffr->buffer)

        // effective "tail" not to be overritten by head increments. It is the tail or (rollback) pTail 
        #define CBFFR_VTAIL ((cbffr->bTail == NULL) ? cbffr->tail : cbffr->bTail)

        #define CBFFR_OCCUPIED ((cbffr->head >= CBFFR_VTAIL) ? cbffr->head - CBFFR_VTAIL : CBFFR_SIZE - (CBFFR_VTAIL - cbffr->head)) 
        #define CBFFR_VACANT  ((cbffr->head >= CBFFR_VTAIL) ? CBFFR_SIZE - (cbffr->head - CBFFR_VTAIL) - 1 : CBFFR_VTAIL - cbffr->head - 1) 
    */
    snprintf(macros, "CBFFR MACROS: size=%d, isWrap=%d, right=%p, occpd=%d, vacnt=%d\r\r", CBFFR_SIZE, CBFFR_WRAPPED, CBFFR_RIGHTEDGE, CBFFR_OCCUPIED, CBFFR_VACANT);
}

/**
 * @brief Get number of characters occupying the buffer.
 */
uint16_t cbffr_getCapacity(cBuffer_t *cbffr)
{
    return CBFFR_SIZE - 1;
}


/**
 * @brief Get count of characters occupying the buffer.
 */
uint16_t cbffr_getOccupied(cBuffer_t *cbffr)
{
    return CBFFR_OCCUPIED;
}


/**
 * @brief Get count of available space in the buffer.
 */
uint16_t cbffr_getVacant(cBuffer_t *cbffr)
{
    return CBFFR_VACANT;
}


/* Basic push/pop/find/grab
 ----------------------------------------------------------------------------------------------- */

uint16_t cbffr_push(cBuffer_t *cbffr, const char *src, uint16_t srcSz)
{
    ASSERT(cbffr->buffer <= cbffr->head && cbffr->head < cbffr->bufferEnd);
    ASSERT(cbffr->buffer <= cbffr->tail && cbffr->tail < cbffr->bufferEnd);

    uint16_t pushCnt;
    if (CBFFR_WRAPPED)                                                      // WRAPPED: space avail = tail-head-1
    {
        pushCnt = MIN(srcSz, cbffr->tail - cbffr->head - 1);
        memcpy(cbffr->head, src, pushCnt);
        cbffr->head += pushCnt;
    }
    else                                                                    // LINEAR: space avail = (bffrEnd-head) + (tail-bffr-1)
    {
        pushCnt = MIN(srcSz, cbffr->bufferEnd - cbffr->head - 1);
        memcpy(cbffr->head, src, pushCnt);                                      // 1st copy: right-side of buffer
        cbffr->head += pushCnt;

        if (pushCnt < srcSz)
        {                                                                       // have not pushed all of srcSz
            uint16_t leftCnt = MIN(srcSz - pushCnt, cbffr->tail - cbffr->head - 1);
            memcpy(cbffr->buffer, src, leftCnt);
            cbffr->head = cbffr->buffer + leftCnt;
            pushCnt += leftCnt;
        }
    }
    ASSERT(cbffr->buffer <= cbffr->head && cbffr->head < cbffr->bufferEnd);
    return pushCnt;
}


/**
 * @brief Get address/length of contiguous block of chars from head (incoming). Distance to tail or buffer-wrap.
 */
uint16_t cbffr_pushBlock(cBuffer_t *cbffr, char **copyTo, uint16_t requestSz)
{
    ASSERT(cbffr->rbHead == 0);
    ASSERT(cbffr->buffer <= cbffr->head && cbffr->head < cbffr->bufferEnd);
    ASSERT(cbffr->buffer <= cbffr->tail && cbffr->tail < cbffr->bufferEnd);

    *copyTo = cbffr->head;
    cbffr->rbHead = cbffr->head;                                             // save current for possible rollback

    uint16_t pushCnt;
    if (CBFFR_WRAPPED)                                                      // WRAPPED
    {
        pushCnt =  MIN(requestSz, (cbffr->tail - cbffr->head) - 1);
        cbffr->head += pushCnt;
    }
    else                                                                    // LINEAR
    {
        pushCnt = MIN(requestSz, cbffr->bufferEnd - cbffr->head);
        cbffr->head +=  pushCnt;
        if (cbffr->head == cbffr->bufferEnd)
        {
            cbffr->head = cbffr->buffer;
        }
    }
    ASSERT(cbffr->buffer <= cbffr->head && cbffr->head < cbffr->bufferEnd);
    return pushCnt;
}


/**
 * @brief Get commit or rollback pending allocation from pushBlock().
 */
void cbffr_pushBlockFinalize(cBuffer_t *cbffr, bool commit)
{
    if (!commit && cbffr->rbHead != NULL)
        cbffr->head = cbffr->rbHead;
    cbffr->rbHead = NULL;
}


/**
 * @brief Pop a number of chars from buffer at buffer-tail. Number of characters "popped" will be the lesser of available and requestSz.
 */
uint16_t cbffr_pop(cBuffer_t *cbffr, char *dest, uint16_t requestSz)
{
    ASSERT(cbffr->buffer <= cbffr->head && cbffr->head < cbffr->bufferEnd);
    ASSERT(cbffr->buffer <= cbffr->tail && cbffr->tail < cbffr->bufferEnd);

    uint16_t popCnt;
    if (CBFFR_WRAPPED)                                                      // WRAPPED
    {
        uint16_t popCnt = MIN(requestSz, cbffr->bufferEnd - cbffr->tail);       // get right-side of buffer
        memcpy(dest, cbffr->tail, popCnt);

        cbffr->tail += popCnt;
        if (cbffr->tail == cbffr->bufferEnd)                                    // edge case: pop exhausted right-side, move tail to buffer start
        {
            cbffr->tail = cbffr->buffer;
        }

        if (popCnt < requestSz)                                                 // not complete, now get left-side of buffer
        {
            uint16_t leftCnt = MIN((requestSz - popCnt), (cbffr->head - cbffr->buffer));
            memcpy(dest + popCnt, cbffr->buffer, leftCnt);
            popCnt += leftCnt;
            cbffr->tail = cbffr->buffer + leftCnt;
        }
    }
    else                                                                    // LINEAR
    {
        popCnt = MIN(requestSz, CBFFR_OCCUPIED);
        memcpy(dest, cbffr->tail, popCnt);
        cbffr->tail += popCnt;
    }
    ASSERT(cbffr->buffer <= cbffr->tail && cbffr->tail < cbffr->bufferEnd);
    return popCnt;
}


/**
 * @brief Allocate a POP of chars from buffer at buffer-tail. Number of chars accounted for will be the lesser of available and requestSz.
 */
uint16_t cbffr_popBlock(cBuffer_t *cbffr, char **copyFrom, uint16_t requestSz)
{
    ASSERT(cbffr->pTail == 0);                                                  // pending popBlock already, no nesting popBlock()
    ASSERT(cbffr->buffer <= cbffr->head && cbffr->head < cbffr->bufferEnd);
    ASSERT(cbffr->buffer <= cbffr->tail && cbffr->tail < cbffr->bufferEnd);

    uint16_t popCnt;
    *copyFrom = cbffr->tail;                                                    // current tail 
    cbffr->pTail = cbffr->tail;                                                 // now pop operating on pTail

    // now advance tail in expectation of copy out
    if (CBFFR_WRAPPED)                                                          // ** wrapped
    {
        popCnt = MIN(requestSz, cbffr->bufferEnd - cbffr->pTail);               // get right-side of buffer
        cbffr->pTail += popCnt;
        if (cbffr->pTail == cbffr->bufferEnd)                                   // edge case: pop exhausted right-side, move tail to buffer start
        {
            cbffr->pTail = cbffr->buffer;
        }
    }
    else                                                                        // ** linear
    {
        popCnt = MIN(requestSz, cbffr->head - cbffr->pTail);
        cbffr->pTail += popCnt;
    }
    if (popCnt == 0)
    {
        cbffr->pTail = NULL;
    }
    ASSERT(cbffr->buffer <= cbffr->tail && cbffr->tail < cbffr->bufferEnd);
    return popCnt;
}


/**
 * @brief Commit or rollback pending allocation from popBlock().
 * @details popBlock() rollback is opposite of pushBlock(), the block operators protect the existing and uncommitted space in the buffer.
 */
void cbffr_popBlockFinalize(cBuffer_t *cbffr, bool commit)
{
    if (commit && cbffr->pTail != NULL)
        cbffr->tail = cbffr->pTail;         // move tail ahead (unprotecting popBlock() copy out space)
    cbffr->pTail = NULL;
}


/**
 * @brief Pop a number of chars from buffer at buffer-tail, leaving the leaveSz number of chars in buffer.
 * @details This allows for simplified trailer parsing. Number of characters "popped" will be the lesser of available and requestSz - leaveSz.
 */
uint16_t cbffr_popLeave(cBuffer_t *cbffr, char *dest, uint16_t requestSz, uint16_t leaveSz)
{
    if (CBFFR_OCCUPIED <= leaveSz)
        return 0;
    else
    {
        uint16_t popSz = 0;
        popSz = MIN((CBFFR_OCCUPIED - leaveSz), requestSz);
        return cbffr_pop(cbffr, dest, popSz);
    }
}


/**
 * @brief Peeks a number of chars ahead in the buffer starting at buffer-tail. 
 * @details Number of characters "peeked" will be the lesser of available and requestSz.
 */
uint16_t cbffr_peek(cBuffer_t *cbffr, char *dest, uint16_t requestSz)
{
    uint16_t available = CBFFR_OCCUPIED;
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
    char *pMatchPtr;                                            // possible match pointer (right-side partial length)

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
    while (searchPtr < CBFFR_RIGHTEDGE)
    {
        if (memcmp(searchPtr, pNeedle, rightSideLen) == 0)
        {
            if (rightSideLen == needleLen)                     // found full needle before right-edge
            {
                if (setTail)
                    cbffr->tail = searchPtr;
                return searchPtr - cbffr->tail;                 // return offset from tail
            }
            else
            {
                pMatchPtr = searchPtr;                          // POSSIBLE start here
                matchedCnt = CBFFR_RIGHTEDGE - searchPtr;
                break;                                          // found start of needle
            }
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
 * @brief Advances the buffer's head (incoming) by the requested number of chars.
 */
void cbffr_skipHead(cBuffer_t *cbffr, uint16_t skipCnt)
{
    if (CBFFR_WRAPPED) {                                                        // ** wrapped
        cbffr->head = MIN(cbffr->head + skipCnt, cbffr->tail-1);
    }
    else 
    {                                                                           // ** linear
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

    if (!CBFFR_WRAPPED)                                                         // ** linear
    {
            cbffr->tail = MIN(cbffr->tail + skipCnt, cbffr->head);
    }
    else                                                                        // ** wrapped
    {
            skipped = MIN(skipCnt, cbffr->bufferEnd - cbffr->tail);                 // right-side
            cbffr->tail = MIN(cbffr->buffer + (skipCnt - skipped), cbffr->head);    // consume remaining to skip, up to buffer-head
    }
}


// /* Transaction
//  ----------------------------------------------------------------------------------------------- */

// bool cbffr_startTransaction(cBuffer_t *cbffr)
// {
//     if (cbffr->bTail == NULL)
//     {
//         cbffr->bTail = cbffr->tail;
//         return true;
//     }
//     return false;
// }


// void cbffr_commitTransaction(cBuffer_t *cbffr)
// {
//     cbffr->tail = cbffr->bTail;
//     cbffr->bTail = NULL;
// }


// void cbffr_rollbackTransaction(cBuffer_t *cbffr)
// {
//     cbffr->bTail = NULL;
// }

