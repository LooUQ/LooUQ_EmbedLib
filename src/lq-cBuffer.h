/******************************************************************************
 *  \file lq-cBffr.h
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

#ifndef __LQ_CBFFR_H__
#define __LQ_CBFFR_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct cBuffer_tag
{
    char *buffer;
    char *bufferEnd;
    // uint16_t bufferSz;
    char *head;
    char *tail;

    char *rbTail;
    // uint8_t *offerAddr;
    // uint16_t offerLen;
} cBuffer_t;


// typedef struct cBffrOffer_tag
// {
//     char *address;
//     uint16_t length;
// } cBffrOffer_t;


#define CBFFR_NOFIND 0xFFFF
// #define CBFFR_OFFERADDR(cbffr) (cbffr->pOfferAddr)
// #define CBFFR_OFFERAVAIL(cbffr) (cbffr->offerAvailable)
//#define CBFFR_MARKADDR(cbffr) (cbffr->mark)


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/**
 * @brief 
 * 
 * @param cbffr 
 * @param rawBuffer 
 * @param bufferSz 
 */
void cbffr_init(cBuffer_t *cbffr, char * rawBuffer, uint16_t bufferSz);


/**
 * @brief Reset the buffer to an empty and initial state. Does not "resize" buffer.
 * 
 * @param cbffr The buffer to operate on.
 */
void cbffr_reset(cBuffer_t *cbffr);


/**
 * @brief Get number of characters occupying the buffer.
 * 
 * @param cbffr The buffer to report on.
 * @return The number of characters in the buffer.
 */
uint16_t cbffr_getCapacity(cBuffer_t *cbffr);


/**
 * @brief Get number of characters occupying the buffer.
 * 
 * @param cbffr The buffer to report on.
 * @return The number of characters in the buffer.
 */
uint16_t cbffr_getFillCnt(cBuffer_t *cbffr);


/**
 * @brief Get number of bytes of free space available in the buffer.
 * 
 * @param cbffr The buffer to report on.
 * @return The number of free bytes in the buffer.
 */
uint16_t cbffr_getOpenCnt(cBuffer_t *cbffr);


/**
 * @brief Diagnostic method to get information on all cBuffer internal macro values.
 * @note macros char buffer should be at least 128 chars in length
 * 
 * @param cbffr The buffer to report on.
 * @param macros Pointer to character buffer to fill with internal MACRO values info.
 */
void cbffr_getMacros(cBuffer_t *cbffr, char *macros);


/* Operate on Buffer 
 =============================================================================================== */

/**
 * @brief Push a series of characters into buffer at buffer-head and update buffer internals accordingly.
 * 
 * @param cbffr The buffer receiving the characters.
 * @param src 
 * @param srcSz 
 * @return true Buffer was updated with src characters. Only returns true if the full size of src can be accepted.
 * @return Number of characters "pushed"; the lesser of available and requestSz.
 */
uint16_t cbffr_push(cBuffer_t *cbffr, const char *src, uint16_t srcSz);


/**
 * @brief Pop a number of chars from buffer at buffer-tail. Number of characters "popped" will be the lesser of available and requestSz.
 * 
 * @param cbffr The buffer sourcing the characters.
 * @param dest Pointer to memory location where popped chars are copied.
 * @param requestSz Number of chars requested from the buffer.
 * @return Number of characters "popped"; the lesser of available and requestSz.
 */
uint16_t cbffr_pop(cBuffer_t *cbffr, char *dest, uint16_t requestSz);


/**
 * @brief Pop a number of chars from buffer at buffer-tail. Number of characters "popped" will be the lesser of available and requestSz.
 * 
 * @param cbffr The buffer sourcing the characters.
 * @param dest Pointer to memory location where popped chars are copied.
 * @param requestSz Number of chars requested from the buffer.
 * @param leaveSz Number of chars to leave in buffer after pop, if buffer has less than leaveSz no chars are popped.
 * @return Number of characters "popped"; the lesser of (available - leaveSz) and requestSz.
 */
uint16_t cbffr_popLeave(cBuffer_t *cbffr, char *dest, uint16_t requestSz, uint16_t leaveSz);


/**
 * @brief Peeks a number of chars ahead in the buffer starting at buffer-tail. Number of characters "peeked" will be the lesser of available and requestSz.
 * 
 * @param cbffr The buffer sourcing the characters.
 * @param dest Pointer to memory location where peeked chars are copied.
 * @param requestSz Number of chars requested from the buffer.
 * @return uint16_t Number of characters "peeked" (the lesser of available and requestSz).
 */
uint16_t cbffr_peek(cBuffer_t *cbffr, char *dest, uint16_t requestSz);


/**
 * @brief Find needle in the buffer. Search area is controlled by searchOffset and searchWindow.
 * 
 * @param cbffr The buffer to be searched.
 * @param pNeedle The character sequence you are looking for.
 * @param searchOffset The number of characters to skip forward from tail -OR- skip back from head to start search. If 0: this is ignored and search starts at buffer-tail.
 * @param searchWindowSz The number of chars from search start to examine for find. If 0, count is ignored and searching continues until buffer-head.
 * @param setTail If true, the buffer tail is advanced to the found needle.
 * @return Offset from tail to the position of the needle within the buffer. If no find returns CBFFR_NOFIND (UINT16_MAX: 0xFFFF)
 */
uint16_t cbffr_find(cBuffer_t *cbffr, const char *needle, int16_t searchOffset, uint16_t searchWindowSz, bool setTail);


/**
 * @brief Advances the buffer's tail (outgoing) by the requested number of chars.
 * @details Typically used after a call to cbffr_getTailBlock() to set cbffr tail to match an outside buffer in operation
 * or to skip a number of chars after a cbffr_find()
 * 
 * @param cbffr The buffer to be operated on.
 * @param skipCnt The number of chars to advance the tail.
 */
void cbffr_skipTail(cBuffer_t *cbffr, uint16_t skipCnt);


/**
 * @brief Advances the buffer's head (incoming) by the requested number of chars.
 * @details Typically used after a call to cbffr_getHeadBlock() to set cbffr head to match an outside buffer in operation.
 * 
 * @param cbffr The buffer to be operated on.
 * @param skipCnt The number of chars to advance the head.
 */
void cbffr_skipHead(cBuffer_t *cbffr, uint16_t skipCnt);

// /**
//  * @brief Advances the buffer's tail to the provided skipToIndex point.
//  * 
//  * @param cbffr The buffer to be searched.
//  * @param skipToIndex The position within the buffer to set the tail to.
//  */
// void cbffr_skipTo(cBuffer_t *cbffr, uint16_t skipToIndex);


// /**
//  * @brief Grab a block of characters from buffer without affecting buffer controls (tail is uneffected).
//  * 
//  * @param cbffr The buffer to be operated on.
//  * @param grabIndex The buffer index to start the copy from. 
//  * @param destPtr Pointer to the location where the grabbed characters are returned.
//  * @param requestCnt Space available to accept grabbed chars. Limits the number of chars grabbed.
//  * @return uint8_t* The number of characters grabbed and placed into location at grabPtr. The smaller of buffer available or destSz.
//  */
// uint16_t cbffr_grab(cBuffer_t *cbffr, uint16_t grabIndex, char *destPtr, uint16_t requestCnt);


/**
 * @brief Get length of contiguous block of chars from tail (outgoing). Distance to head or buffer-wrap.
 * 
 * @param cbffr [in] The buffer to be operated on.
 * @param tail [in/out] Pointer to buffer-tail.
 * @param length [in/out] Pointer to integer length variable with the contiguous length of buffer between tail and head/buffer-end (wrap).
 */
void cbffr_getTailBlock(cBuffer_t *cbffr, char **tail, uint16_t *length);

/**
 * @brief Get length of contiguous block of chars from head (incoming). Distance to tail or buffer-wrap.
 * 
 * @param cbffr [in] The buffer to be operated on.
 * @param tail [in/out] Pointer to buffer-head.
 * @param length [in/out] Pointer to integer length variable with the contiguous length of buffer between tail and head/buffer-end (wrap).
 */
void cbffr_getHeadBlock(cBuffer_t *cbffr, char **head, uint16_t *length);


// /**
//  * @brief Request a offer of linear (no wrap) space available from the buffer-head to copy into (SPI, I2C peripheral copy in).
//  * 
//  * @param cbffr The buffer to operate on, where the offer will be serviced from.
//  * @param requestSz 
//  * @return cBffrOffer_t Struct with the starting address and number of chars available to receive copy in ().
//  */
// cBffrOffer_t cbffr_getOffer(cBuffer_t *cbffr, uint16_t requestSz);


// /**
//  * @brief Tell buffer that the previously requested offer was taken and the copy in was completed.
//  * 
//  * @param cbffr The buffer to operate on, where characters were transfered in to fulfill the offer.
//  */
// void cbffr_takeOffer(cBuffer_t *cbffr);


// /**
//  * @brief Tell buffer that the previously requested offer was NOT used and the space requested is available for other use.
//  * 
//  * @param cbffr 
//  */
// void cbffr_releaseOffer(cBuffer_t *cbffr);




bool cbffr_startTransaction(cBuffer_t *cbffr);

void cbffr_commitTransaction(cBuffer_t *cbffr);

void cbffr_rollbackTransaction(cBuffer_t *cbffr);


// /**
//  * @brief Sets a marker at the current buffer tail to allow for future copy of buffer section.
//  * 
//  * @param cbffr The cBffr containing the current state and space to be marked
//  */
// void cbffr_setTMark(cBuffer_t *cbffr);


// /**
//  * @brief Copies content from cBffr at the previously set "mark" up to curret head. Ensures the passed buffer has sufficient space for copy + null char.
//  * @details Does not update cBffr internal tail pointer, the tMark pointer is released.
//  * 
//  * @param cbffr The buffer to source the copy from.
//  * @param dest 
//  * @param destSz 
//  * @return bool True if the destination was large enough to accept the copy. No copy is performed if it is too smal.
//  */
// uint16_t cbffr_grabTMark(cBuffer_t *cbffr, char *dest, uint16_t destSz);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_CBFFR_H__ */