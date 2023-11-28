/******************************************************************************
 *  \file lq-str.h
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
 * C string helper functions
 *****************************************************************************/

#ifndef __LQ_STR_H__
#define __LQ_STR_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


/**
 *  @brief Constrained string search, only check haystack for length characters. Allows for deterministic searching non-NULL terminated char sequences.
 * 
 *  @param [in] haystack Pointer to a char array that may/may not be NULL terminated 
 *  @param [in] needle The char sequence to search for, needs to be an NULL terminated C-string
 *  @param [in] maxSearch Maximum number of chars to search for needle 
 *  @return Pointer to the location of needle, NULL=no find.
*/
const char *lq_strnstr(const char *haystack, const char *needle, size_t length);


/**
 *  @brief Scans the source string for fromChr and replaces with toChr. Usefull for substitution of special char in query strings.
 * 
 *  @param [in\out] srcStr - Char pointer to the c-string containing required substitutions 
 *  @param [in] fromChr - Char value to replace in src 
 *  @param [in] toChr - Char value to put in place of fromChr 
 *  @return Number of substitutions made
*/
uint16_t lq_strReplace(char *srcStr, char fromChr, char toChr);

/**
 *  @brief Performs URL escape removal for special char (%20-%2F) without malloc.
 * 
 *  @param src [in] - Input text string to URL decode.
 *  @param len [in] - Length of input text string.
 *  @return 
*/
uint16_t lq_strUriDecode(char *src, int len);


/**
 *  @brief Performs URL escape removal for special char (%20-%2F) without malloc.
 *  @details Field Type Specifiers
 *      '-' - alpha-numeric
 *      'n' - numeric
 *      'h' - hex-digits
 *      'a' - alpha
 *      'u' - upper-case alpha
 *      'l' - lower-case alpha
 *      '*' - printable
 * 
 *  @param src [in] - The type of field to search for, see list above.
 *  @return pointer to the start of the specified field, NULL if not found
*/
const char* lq_strFindField(char fldType, char* source);


/**
 *  @brief Scans a C-String (char array) for the next delimeted token and null terminates it.
 * 
 *  @param [in] source Original char array to scan.
 *  @param [in] delimeter Character separating tokens (passed as integer value).
 *  @param [out] tokenBffr Pointer to where token should be copied to.
 *  @param [in] tokenBffrSz Size of buffer
 *  @return Pointer to the location in the source string immediately following the token.
*/
char *lq_strToken(char *source, int delimiter, char *tokenBffr, uint8_t tokenBffrSz);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_STR_H__ */
