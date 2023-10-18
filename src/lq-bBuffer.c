/******************************************************************************
 *  \file lq-bBuffer.c
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

#define SRCFILE "BBF"                       // create SRCFILE (3 char) MACRO for lq-diagnostics ASSERT
#define ENABLE_DIAGPRINT                    // expand DPRINT into debug output
//#define ENABLE_DIAGPRINT_VERBOSE            // expand DPRINT and DPRINT_V into debug output
#define ENABLE_ASSERT
#include <lqdiag.h>

#include "lq-bBuffer.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define BBFFR_WRAPPED (bbffr->head < bbffr->tail)
#define BBFFR_RIGHTEDGE ((BBFFR_WRAPPED) ? bbffr->bufferEnd : bbffr->head)
#define BBFFR_SIZE (bbffr->bufferEnd - bbffr->buffer)
#define BBFFR_OCCUPIED ((bbffr->head >= bbffr->tail) ? bbffr->head - bbffr->tail : BBFFR_SIZE - (bbffr->tail - bbffr->head)) 
#define BBFFR_VACANT  (BBFFR_SIZE - BBFFR_OCCUPIED)




#define SRCFILE "CBF"                   // create SRCFILE (3 char) MACRO for lq-diagnostics ASSERT


void bbffr_init(bBuffer_t *bbffr, char *rawBuffer, uint16_t bufferSz)
{
    bbffr->buffer = rawBuffer;
    bbffr->bufferEnd = rawBuffer + bufferSz;
    bbffr->head = rawBuffer;
    bbffr->tail = rawBuffer;
    bbffr->rbHead = NULL;
    bbffr->pTail = NULL;
}


void bbffr_reset(bBuffer_t *bbffr)
{
    bbffr->head = bbffr->buffer;
    bbffr->tail = bbffr->buffer;
    bbffr->rbHead = NULL;
    bbffr->pTail = NULL;
    // temporary, not technically required just makes diag a little easier
    memset(bbffr->buffer, 0, BBFFR_SIZE);
}


void bbffr_GETMACROS(bBuffer_t *bbffr, char *macros)
{
    /*
        #define BBFFR_WRAPPED (bbffr->head < bbffr->tail)
        #define BBFFR_RIGHTEDGE ((BBFFR_WRAPPED) ? bbffr->bufferEnd : bbffr->head)
        #define BBFFR_SIZE (bbffr->bufferEnd - bbffr->buffer)

        // effective "tail" not to be overritten by head increments. It is the tail or (rollback) pTail 
        #define BBFFR_VTAIL ((bbffr->bTail == NULL) ? bbffr->tail : bbffr->bTail)

        #define BBFFR_OCCUPIED ((bbffr->head >= BBFFR_VTAIL) ? bbffr->head - BBFFR_VTAIL : BBFFR_SIZE - (BBFFR_VTAIL - bbffr->head)) 
        #define BBFFR_VACANT  ((bbffr->head >= BBFFR_VTAIL) ? BBFFR_SIZE - (bbffr->head - BBFFR_VTAIL) - 1 : BBFFR_VTAIL - bbffr->head - 1) 
    */
    snprintf(macros, "CBFFR MACROS: size=%d, isWrap=%d, right=%p, occpd=%d, vacnt=%d\r\r", BBFFR_SIZE, BBFFR_WRAPPED, BBFFR_RIGHTEDGE, BBFFR_OCCUPIED, BBFFR_VACANT);
}

/**
 * @brief Get total capacity of buffer.
 * @param [in] bbffr pointer to buffer to report on.
 * @return Buffer capacity in bytes/chars.
 */
uint16_t bbffr_getCapacity(bBuffer_t *bbffr)
{
    return BBFFR_SIZE - 1;
}


/**
 * @brief Get count of characters occupying the buffer.
 */
uint16_t bbffr_getOccupied(bBuffer_t *bbffr)
{
    return BBFFR_OCCUPIED;
}


/**
 * @brief Get count of available space in the buffer.
 */
uint16_t bbffr_getVacant(bBuffer_t *bbffr)
{
    return BBFFR_VACANT;
}


/* Basic push/pop/find/grab
 ----------------------------------------------------------------------------------------------- */

uint16_t bbffr_push(bBuffer_t *bbffr, const char *src, uint16_t srcSz)
{
    ASSERT(bbffr->buffer <= bbffr->head && bbffr->head < bbffr->bufferEnd);
    ASSERT(bbffr->buffer <= bbffr->tail && bbffr->tail < bbffr->bufferEnd);

    uint16_t pushCnt;
    if (BBFFR_WRAPPED)                                                      // WRAPPED: space avail = tail-head-1
    {
        pushCnt = MIN(srcSz, bbffr->tail - bbffr->head - 1);
        memcpy(bbffr->head, src, pushCnt);
        bbffr->head += pushCnt;
    }
    else                                                                    // LINEAR: space avail = (bffrEnd-head) + (tail-bffr-1)
    {
        pushCnt = MIN(srcSz, bbffr->bufferEnd - bbffr->head - 1);
        memcpy(bbffr->head, src, pushCnt);                                      // 1st copy: right-side of buffer
        bbffr->head += pushCnt;

        if (pushCnt < srcSz)
        {                                                                       // have not pushed all of srcSz
            uint16_t leftCnt = MIN(srcSz - pushCnt, bbffr->tail - bbffr->head - 1);
            memcpy(bbffr->buffer, src, leftCnt);
            bbffr->head = bbffr->buffer + leftCnt;
            pushCnt += leftCnt;
        }
    }
    ASSERT(bbffr->buffer <= bbffr->head && bbffr->head < bbffr->bufferEnd);
    return pushCnt;
}


/**
 * @brief Get address/length of contiguous block of chars from head (incoming). Distance to tail or buffer-wrap.
 */
uint16_t bbffr_pushBlock(bBuffer_t *bbffr, char **copyTo, uint16_t requestSz)
{
    ASSERT(bbffr->rbHead == 0);
    ASSERT(bbffr->buffer <= bbffr->head && bbffr->head < bbffr->bufferEnd);
    ASSERT(bbffr->buffer <= bbffr->tail && bbffr->tail < bbffr->bufferEnd);

    *copyTo = bbffr->head;
    bbffr->rbHead = bbffr->head;                                             // save current for possible rollback

    uint16_t pushCnt;
    if (BBFFR_WRAPPED)                                                      // WRAPPED
    {
        pushCnt =  MIN(requestSz, (bbffr->tail - bbffr->head) - 1);
        bbffr->head += pushCnt;
    }
    else                                                                    // LINEAR
    {
        pushCnt = MIN(requestSz, bbffr->bufferEnd - bbffr->head);
        bbffr->head +=  pushCnt;
        if (bbffr->head == bbffr->bufferEnd)
        {
            bbffr->head = bbffr->buffer;
        }
    }
    ASSERT(bbffr->buffer <= bbffr->head && bbffr->head < bbffr->bufferEnd);
    return pushCnt;
}


/**
 * @brief Get commit or rollback pending allocation from pushBlock().
 */
void bbffr_pushBlockFinalize(bBuffer_t *bbffr, bool commit)
{
    if (!commit && bbffr->rbHead != NULL)
        bbffr->head = bbffr->rbHead;
    bbffr->rbHead = NULL;
}


/**
 * @brief Pop a number of chars from buffer at buffer-tail. Number of characters "popped" will be the lesser of available and requestSz.
 */
uint16_t bbffr_pop(bBuffer_t *bbffr, char *dest, uint16_t requestSz)
{
    ASSERT(bbffr->buffer <= bbffr->head && bbffr->head < bbffr->bufferEnd);
    ASSERT(bbffr->buffer <= bbffr->tail && bbffr->tail < bbffr->bufferEnd);

    uint16_t popCnt;
    if (BBFFR_WRAPPED)                                                      // WRAPPED
    {
        uint16_t popCnt = MIN(requestSz, bbffr->bufferEnd - bbffr->tail);       // get right-side of buffer
        memcpy(dest, bbffr->tail, popCnt);

        bbffr->tail += popCnt;
        if (bbffr->tail == bbffr->bufferEnd)                                    // edge case: pop exhausted right-side, move tail to buffer start
        {
            bbffr->tail = bbffr->buffer;
        }

        if (popCnt < requestSz)                                                 // not complete, now get left-side of buffer
        {
            uint16_t leftCnt = MIN((requestSz - popCnt), (bbffr->head - bbffr->buffer));
            memcpy(dest + popCnt, bbffr->buffer, leftCnt);
            popCnt += leftCnt;
            bbffr->tail = bbffr->buffer + leftCnt;
        }
    }
    else                                                                    // LINEAR
    {
        popCnt = MIN(requestSz, BBFFR_OCCUPIED);
        memcpy(dest, bbffr->tail, popCnt);
        bbffr->tail += popCnt;
    }
    ASSERT(bbffr->buffer <= bbffr->tail && bbffr->tail < bbffr->bufferEnd);
    return popCnt;
}


/**
 * @brief Allocate a POP of chars from buffer at buffer-tail. Number of chars accounted for will be the lesser of available and requestSz.
 */
uint16_t bbffr_popBlock(bBuffer_t *bbffr, char **copyFrom, uint16_t requestSz)
{
    ASSERT(bbffr->pTail == 0);                                                  // pending popBlock already, no nesting popBlock()
    ASSERT(bbffr->buffer <= bbffr->head && bbffr->head < bbffr->bufferEnd);
    ASSERT(bbffr->buffer <= bbffr->tail && bbffr->tail < bbffr->bufferEnd);

    uint16_t popCnt;
    *copyFrom = bbffr->tail;                                                    // current tail 
    bbffr->pTail = bbffr->tail;                                                 // now pop operating on pTail

    // now advance tail in expectation of copy out
    if (BBFFR_WRAPPED)                                                          // ** wrapped
    {
        popCnt = MIN(requestSz, bbffr->bufferEnd - bbffr->pTail);               // get right-side of buffer
        bbffr->pTail += popCnt;
        if (bbffr->pTail == bbffr->bufferEnd)                                   // edge case: pop exhausted right-side, move tail to buffer start
        {
            bbffr->pTail = bbffr->buffer;
        }
    }
    else                                                                        // ** linear
    {
        popCnt = MIN(requestSz, bbffr->head - bbffr->pTail);
        bbffr->pTail += popCnt;
    }
    if (popCnt == 0)
    {
        bbffr->pTail = NULL;
    }
    ASSERT(bbffr->buffer <= bbffr->tail && bbffr->tail < bbffr->bufferEnd);
    return popCnt;
}


/**
 * @brief Commit or rollback pending allocation from popBlock().
 * @details popBlock() rollback is opposite of pushBlock(), the block operators protect the existing and uncommitted space in the buffer.
 */
void bbffr_popBlockFinalize(bBuffer_t *bbffr, bool commit)
{
    if (commit && bbffr->pTail != NULL)
        bbffr->tail = bbffr->pTail;         // move tail ahead (unprotecting popBlock() copy out space)
    bbffr->pTail = NULL;
}


/**
 * @brief Pop a number of chars from buffer at buffer-tail, leaving the leaveSz number of chars in buffer.
 * @details This allows for simplified trailer parsing. Number of characters "popped" will be the lesser of available and requestSz - leaveSz.
 */
uint16_t bbffr_popLeave(bBuffer_t *bbffr, char *dest, uint16_t requestSz, uint16_t leaveSz)
{
    if (BBFFR_OCCUPIED <= leaveSz)
        return 0;
    else
    {
        uint16_t popSz = 0;
        popSz = MIN((BBFFR_OCCUPIED - leaveSz), requestSz);
        return bbffr_pop(bbffr, dest, popSz);
    }
}


/**
 * @brief Peeks a number of chars ahead in the buffer starting at buffer-tail. 
 * @details Number of characters "peeked" will be the lesser of available and requestSz.
 */
uint16_t bbffr_peek(bBuffer_t *bbffr, char *dest, uint16_t requestSz)
{
    uint16_t available = BBFFR_OCCUPIED;
    uint16_t peeked;

    if (!BBFFR_WRAPPED)                                              // ** linear
    {
        peeked = MIN(available, requestSz);
        memcpy(dest, bbffr->tail, peeked);
    }
    else                                                                // ** wrapped
    {
        peeked = MIN(requestSz, bbffr->bufferEnd - bbffr->tail);            // get right-side of buffer
        memcpy(dest, bbffr->tail, peeked);
        available -= peeked;                                                // peeked number of chars serviced from request 

        if (available > 0)                                                  // get left-side of buffer
        {
            memcpy(dest + peeked, bbffr->buffer, available);
            peeked += available;                                            // add 2nd copy back to report request serviced to caller
        }
    }
    return peeked;
}


/**
 * @brief Find needle in the buffer, searching forward from the buffer-tail to buffer-head.
 */
uint16_t bbffr_find(bBuffer_t *bbffr, const char *pNeedle, int16_t searchOffset, uint16_t searchWindowSz, bool setTail)
{
    uint8_t needleLen = strlen(pNeedle);
    uint8_t matchedCnt = 0;                                     // chars matched, 
    uint8_t rightSideLen = 0;
    char *searchPtr;
    char *pMatchPtr;                                            // possible match pointer (right-side partial length)

    /* Where to start searching
     */
    if (searchOffset == 0)
        searchPtr = bbffr->tail;
    else
    {
        if (searchOffset > 0)                                               // (+) skip ahead from tail
        {
            if (BBFFR_WRAPPED) {
                uint16_t rightside = MIN(bbffr->bufferEnd - bbffr->tail, searchOffset);
                searchPtr = MIN(bbffr->buffer + rightside, bbffr->head);
            }
            else {
                searchPtr = MIN(bbffr->tail + searchOffset, bbffr->head);
            }
        }
        else                                                                // (-) skip back from head
        {
            searchOffset *= -1;                                                 // make positive
            if (BBFFR_WRAPPED) {
                searchPtr = MAX(bbffr->head - searchOffset, bbffr->tail);
            }
            else {
                uint16_t leftSide = MAX(bbffr->head - bbffr->buffer, searchOffset);
                searchPtr = bbffr->bufferEnd - (searchOffset - leftSide);
            }
        }
    }

    /* Search right-side (start to buffer wrap)
     */
    rightSideLen = needleLen;
    while (searchPtr < BBFFR_RIGHTEDGE)
    {
        if (memcmp(searchPtr, pNeedle, rightSideLen) == 0)
        {
            if (rightSideLen == needleLen)                     // found full needle before right-edge
            {
                if (setTail)
                    bbffr->tail = searchPtr;
                return searchPtr - bbffr->tail;                 // return offset from tail
            }
            else
            {
                pMatchPtr = searchPtr;                          // POSSIBLE start here
                matchedCnt = BBFFR_RIGHTEDGE - searchPtr;
                break;                                          // found start of needle
            }
        }
        searchPtr++;
        rightSideLen = MIN(BBFFR_RIGHTEDGE - searchPtr, needleLen);
    }

    if (!BBFFR_WRAPPED)                                                 // if not wrapped, no further opportunity to match
        return BBFFR_NOFIND_VAL;

    /* Handle partial match at right edge, remaining must be at left edge before head
     */
    if (matchedCnt > 0)                                                 // partial match at right edge
    {
        if (memcmp(bbffr->buffer, pNeedle + matchedCnt, needleLen - matchedCnt) == 0)
        {
            if (setTail)
                bbffr->tail = searchPtr;
            return searchPtr - bbffr->tail;
        }
        else
        {
            matchedCnt = 0;                                             // any match from right side is orphaned, start over
        }
    }
    searchPtr = bbffr->buffer;                                          // moving to left side

    /* Continue searching left-side (wrap point to head)
     */
    while (searchPtr < bbffr->head)
    {
        if (memcmp(searchPtr, pNeedle, needleLen) == 0)
        {   
            if (setTail)
                bbffr->tail = searchPtr;
            return (searchPtr - bbffr->buffer) + (bbffr->bufferEnd - bbffr->tail);
        }
        searchPtr++;
    }
    return BBFFR_NOFIND_VAL;
}


/**
 * @brief Advances the buffer's head (incoming) by the requested number of chars.
 */
void bbffr_skipHead(bBuffer_t *bbffr, uint16_t skipCnt)
{
    if (BBFFR_WRAPPED) {                                                        // ** wrapped
        bbffr->head = MIN(bbffr->head + skipCnt, bbffr->tail-1);
    }
    else 
    {                                                                           // ** linear
        if (bbffr->head + skipCnt < bbffr->bufferEnd)
            bbffr->head += skipCnt;
        else if (bbffr->head + skipCnt == bbffr->bufferEnd)
            bbffr->head = bbffr->buffer;
        else {
            uint16_t leftSkip = skipCnt - (bbffr->bufferEnd - bbffr->head);
            bbffr->head = MIN(bbffr->head + leftSkip, bbffr->tail-1);
        }
    }
}


/**
 * @brief Advances the buffer's tail (outgoing) by the requested number of chars.
 */
void bbffr_skipTail(bBuffer_t *bbffr, uint16_t skipCnt)
{
    uint16_t skipped;

    if (!BBFFR_WRAPPED)                                                         // ** linear
    {
            bbffr->tail = MIN(bbffr->tail + skipCnt, bbffr->head);
    }
    else                                                                        // ** wrapped
    {
            skipped = MIN(skipCnt, bbffr->bufferEnd - bbffr->tail);                 // right-side
            bbffr->tail = MIN(bbffr->buffer + (skipCnt - skipped), bbffr->head);    // consume remaining to skip, up to buffer-head
    }
}


// /* Transaction
//  ----------------------------------------------------------------------------------------------- */

// bool bbffr_startTransaction(bBuffer_t *bbffr)
// {
//     if (bbffr->bTail == NULL)
//     {
//         bbffr->bTail = bbffr->tail;
//         return true;
//     }
//     return false;
// }


// void bbffr_commitTransaction(bBuffer_t *bbffr)
// {
//     bbffr->tail = bbffr->bTail;
//     bbffr->bTail = NULL;
// }


// void bbffr_rollbackTransaction(bBuffer_t *bbffr)
// {
//     bbffr->bTail = NULL;
// }

