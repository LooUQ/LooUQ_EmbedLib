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

typedef struct cBffr_tag
{
    char *buffer;
    char *bufferEnd;
    uint16_t bufferSz;
    char *head;
    char *tail;

    uint8_t *offerAddr;
    uint16_t offerLen;
    uint8_t *tMark;
} cBffr_t;


typedef struct cBffrOffer_tag
{
    char *address;
    uint16_t length;
} cBffrOffer_t;



#define CBFFR_OFFERADDR(cbffr) (cbffr->pOfferAddr)
#define CBFFR_OFFERAVAIL(cbffr) (cbffr->offerAvailable)
#define CBFFR_MARKADDR(cbffr) (cbffr->mark)


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
void cbffr_init(cBffr_t *cbffr, char * rawBuffer, size_t bufferSz);


void cbffr_reset(cBffr_t *cbffr);

uint16_t cbffr_getAvailable(cBffr_t *cbffr);

/**
 * @brief Push a series of characters into buffer at buffer-head and update buffer internals accordingly.
 * 
 * @param cbffr The buffer receiving the characters.
 * @param src 
 * @param srcSz 
 * @return true Buffer was updated with src characters. Only returns true if the full size of src can be accepted.
 * @return false Copy in could not proceed. Either src was too large or there is an outstanding "offer".
 */
bool cbffr_push(cBffr_t *cbffr, const char *src, size_t srcSz);


/**
 * @brief Pop a series of characters from buffer at buffer-tail. Number of characters "popped" will be the lesser of available and destSz.
 * 
 * @param cbffr The buffer sourcing the characters.
 * @param dest 
 * @param destSz 
 * @return uint16_t Number of characters "popped" (the lesser of available and destSz).
 */
uint16_t cbffr_pop(cBffr_t *cbffr, char *dest, size_t requestSz);


uint16_t cbffr_discard(cBffr_t *cbffr, size_t requestSz);


/**
 * @brief Find needle in the buffer, searching forward from the buffer-tail to buffer-head.
 * 
 * @param cbffr The buffer to be searched.
 * @param pNeedle The character sequence you are looking for.
 * @param lookbackCnt The number of characters skip back from head prior to starting find. If 0, count is ignored and search starts at buffer-tail.
 * @param searchCnt The number of chars from search start to examine for find. If 0, count is ignored and searching stops at buffer-head.
 * @return uint8_t* The location where the find matched contents in the buffer.
 */
char *cbffr_find(cBffr_t *cbffr, const char *needle, uint16_t lookbackCnt, uint16_t searchCnt);


// /**
//  * @brief Find needle in the buffer, searching forward from the search start (buffer-head - lookbackCnt) to buffer-head.
//  * 
//  * @param cbffr The buffer to be searched.
//  * @param pNeedle The character sequence you are looking for.
//  * @return uint8_t* The location where the find matched contents in the buffer.
//  */
// uint8_t *cbffr_lbFind(cBffr_t *cbffr, const char *pNeedle);
uint16_t cbffr_grab(cBffr_t *cbffr, const char *grabPtr, char *dest, size_t destSz);



/**
 * @brief Request a offer of linear (no wrap) space available from the buffer-head to copy into (SPI, I2C peripheral copy in).
 * 
 * @param cbffr The buffer to operate on, where the offer will be serviced from.
 * @param requestSz 
 * @return cBffrOffer_t Struct with the starting address and number of chars available to receive copy in ().
 */
cBffrOffer_t cbffr_getOffer(cBffr_t *cbffr, uint16_t requestSz);


/**
 * @brief Tell buffer that the previously requested offer was taken and the copy in was completed.
 * 
 * @param cbffr The buffer to operate on, where characters were transfered in to fulfill the offer.
 */
void cbffr_takeOffer(cBffr_t *cbffr);


/**
 * @brief Tell buffer that the previously requested offer was NOT used and the space requested is available for other use.
 * 
 * @param cbffr 
 */
void cbffr_releaseOffer(cBffr_t *cbffr);



// /**
//  * @brief Sets a marker at the current buffer head to allow rollback of buffer state.
//  * 
//  * @param cbffr The cBffr containing the current state and space to be marked
//  */
// void cbffr_setHMark(cBffr_t *cbffr);


// /**
//  * @brief Rolls head back to previously set hMark (previous head).
//  * 
//  * @param cbffr The buffer to be updated.
//  */
// void cbffr_rollbackHMark(cBffr_t *cbffr);



/**
 * @brief Sets a marker at the current buffer tail to allow for future copy of buffer section.
 * 
 * @param cbffr The cBffr containing the current state and space to be marked
 */
void cbffr_setTMark(cBffr_t *cbffr);


/**
 * @brief Copies content from cBffr at the previously set "mark" up to curret head. Ensures the passed buffer has sufficient space for copy + null char.
 * @details Does not update cBffr internal tail pointer, the tMark pointer is released.
 * 
 * @param cbffr The buffer to source the copy from.
 * @param dest 
 * @param destSz 
 * @return bool True if the destination was large enough to accept the copy. No copy is performed if it is too smal.
 */
uint16_t cbffr_grabTMark(cBffr_t *cbffr, char *dest, size_t destSz);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_CBFFR_H__ */