/******************************************************************************
 *  \file lq-bbffr.h
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

#ifndef __LQ_BBFFR_H__
#define __LQ_BBFFR_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Internal control structure for a block buffer.
 */
typedef struct bbuffer_tag
{
    char *buffer;                                           ///< Raw character buffer
    char *bufferEnd;                                        ///< End of character buffer, points to the last position + 1

    char * volatile head;                                   ///< Pointer to the head position, where new char are logically ADDED to the buffer (pushed to)
    char * volatile tail;                                   ///< Pointer to the tail position, where char are logically REMOVED from the buffer (popped from)
    char * volatile rbHead;                                 ///< Pointer stash for possible rollback of head from a pushBlock operation
    char * volatile pTail;                                  ///< Pointer stash for possible rollback of tail from a popBlock operation
} bbuffer_t;


#define BBFFR_NOTFOUND 0xFFFF                               ///< Value returned from a buffer find operation signalling NOT FOUND
#define BBFFR_ISFOUND(x) (x != BBFFR_NOTFOUND)              ///< Convenience macro, returns True if search target is FOUND
#define BBFFR_ISNOTFOUND(x) (x == BBFFR_NOTFOUND)           ///< Convenience macro, returns True if search target is NOT FOUND


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
void bbffr_init(bbuffer_t *cbffr, char * rawBuffer, uint16_t bufferSz);


/**
 * @brief Reset the buffer to an empty and initial state. Does not "resize" buffer.
 * 
 * @param cbffr The buffer to operate on.
 */
void bbffr_reset(bbuffer_t *cbffr);


/**
 * @brief Get number of characters occupying the buffer.
 * 
 * @param cbffr The buffer to report on.
 * @return The number of characters in the buffer.
 */
uint16_t bbffr_getCapacity(bbuffer_t *cbffr);


/**
 * @brief Get number of characters occupying the buffer.
 * 
 * @param cbffr The buffer to report on.
 * @return The number of characters in the buffer.
 */
uint16_t bbffr_getOccupied(bbuffer_t *cbffr);


/**
 * @brief Get number of bytes of free space available in the buffer.
 * 
 * @param cbffr The buffer to report on.
 * @return The number of free bytes in the buffer.
 */
uint16_t bbffr_getVacant(bbuffer_t *cbffr);


/**
 * @brief Diagnostic method to get information on all cBuffer internal macro values.
 * @note macros char buffer should be at least 128 chars in length
 * 
 * @param cbffr The buffer to report on.
 * @param macrosRpt Pointer to character buffer to fill with internal MACRO values info.
 * @param macrosRptSz Size of the result reporting buffer.
 */
void bbffr_GETMACROS(bbuffer_t *bbffr, char *macrosRpt, uint8_t macrosRptSz);


/* Operate on Buffer 
 =============================================================================================== */

/**
 * @brief Push a series of characters into buffer at buffer-head and update buffer internals accordingly.
 * 
 * @param cbffr The buffer receiving the characters.
 * @param src 
 * @param requestSz 
 * @return true Buffer was updated with src characters. Only returns true if the full size of src can be accepted.
 * @return Number of characters "pushed"; the lesser of available and requestSz.
 */
uint16_t bbffr_push(bbuffer_t *cbffr, const char *src, uint16_t requestSz);


/**
 * @brief Get length of contiguous block of chars from head (incoming). Distance to tail or buffer-wrap.
 * 
 * @param cbffr [in] The buffer to be operated on.
 * @param copyTo [out] Pointer to delegated copy destination, where your function needs to copy to.
 * @param requestSz [in] Requested (contiguous) space in buffer.
 * @return Number of characters available to be pushed; the lesser of contiguous vacant and requestSz.
 */
uint16_t bbffr_pushBlock(bbuffer_t *cbffr, char **copyTo, uint16_t requestSz);


/**
 * @brief Commit or rollback a pending push block. 
 * @details Required for complete a pushBlock() operation, pushBlock is blocking to additional buffer pushes until finalized.
 * 
 * @param cbffr [in] The buffer to be operated on.
 * @param commit [in] Commit pending push block (true) or roll-back pending push (false).
 */
void bbffr_pushBlockFinalize(bbuffer_t *cbffr, bool commit);


/**
 * @brief Pop a number of chars from buffer at buffer-tail. Number of characters "popped" will be the lesser of available and requestSz.
 * 
 * @param cbffr The buffer sourcing the characters.
 * @param dest Pointer to memory location where popped chars are copied.
 * @param requestSz Number of chars requested from the buffer.
 * @return Number of characters "popped"; the lesser of available and requestSz.
 */
uint16_t bbffr_pop(bbuffer_t *cbffr, char *dest, uint16_t requestSz);


/**
 * @brief Allocate a POP of chars from buffer at buffer-tail. Number of chars accounted for will be the lesser of available and requestSz.
 * @note The buffer space returned is gaurded against overwrite until  bbffr_popFinalize() function is invoked; which can be commit or rollback.
 * 
 * @param cbffr [in] The buffer sourcing the characters.
 * @param copyFrom [out] Dbl-pointer to memory location where popped chars are TO BE copied from.
 * @param requestSz [in] Number of chars requested from the buffer.
 * @return Number of characters available to be popped; the lesser of occupied and requestSz.
 */
uint16_t bbffr_popBlock(bbuffer_t *cbffr, char **copyFrom, uint16_t requestSz);


/**
 * @brief Commit or rollback a pending pop block. 
 * @note Required for complete a popBlock() operation, popBlock is blocking to additional buffer pops until finalized.
 * 
 * @param cbffr [in] The buffer to be operated on.
 * @param commit [in] Commit pending pop block (true) or roll-back pending pop (false).
 */
void bbffr_popBlockFinalize(bbuffer_t *cbffr, bool commit);


/**
 * @brief Peeks a number of chars ahead in the buffer starting at buffer-tail. Number of characters "peeked" will be the lesser of available and requestSz.
 * 
 * @param cbffr The buffer sourcing the characters.
 * @param dest Pointer to memory location where peeked chars are copied.
 * @param requestSz Number of chars requested from the buffer.
 * @return uint16_t Number of characters "peeked" (the lesser of available and requestSz).
 */
uint16_t bbffr_peek(bbuffer_t *cbffr, char *dest, uint16_t requestSz);


/**
 * @brief Find needle in the buffer. Search begins at the buffer TAIL unless overridden.
 * @details Search area can be overridden with searchOffset and searchWindow. The offset sets the starting point (referenced from TAIL if positive, from HEAD
 * if negative), window sets the stopping point (referenced from start: TAIL/offset).
 * 
 * @param cbffr The buffer to be searched.
 * @param pNeedle The character sequence you are looking for.
 * @param searchOffset The number of characters to skip forward from tail -OR- skip back from head to start search. If 0: this is ignored and search starts at buffer-tail.
 * @param searchWindowSz The number of chars from search start to examine for find. If 0, count is ignored and searching continues until buffer-head.
 * @param setTail If true, the buffer tail is advanced to the first character of found needle.
 * @return Offset from buffer TAIL to the position of the needle within the buffer. If no find returns CBFFR_NOFIND (UINT16_MAX: 0xFFFF)
 */
uint16_t bbffr_find(bbuffer_t *cbffr, const char *needle, int16_t searchOffset, uint16_t searchWindowSz, bool setTail);


/**
 * @brief Advances the buffer's tail (outgoing) by the requested number of chars.
 * @details Typically used after a call to bbffr_getTailBlock() to set cbffr tail to match an outside buffer in operation
 * or to skip a number of chars after a bbffr_find()
 * 
 * @param cbffr The buffer to be operated on.
 * @param skipCnt The number of chars to advance the tail.
 */
void bbffr_skipTail(bbuffer_t *cbffr, uint16_t skipCnt);


/**
 * @brief Advances the buffer's head (incoming) by the requested number of chars.
 * @details Typically used after a call to bbffr_getHeadBlock() to set cbffr head to match an outside buffer in operation.
 * 
 * @param cbffr The buffer to be operated on.
 * @param skipCnt The number of chars to advance the head.
 */
void bbffr_skipHead(bbuffer_t *cbffr, uint16_t skipCnt);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_BBFFR_H__ */