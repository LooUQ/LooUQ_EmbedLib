/******************************************************************************
 *  \file lq-str.c
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
 * C-string functions from LooUQ-DeviceCommon.
 *****************************************************************************/

#include "lq-str.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

char *lq_strnstr(char *haystack, char *needle, size_t length)
{
    size_t needle_length = MIN(length, strlen(needle));
    size_t i;
    for (i = 0; i < length; i++) {
        if (i + needle_length > length) {                           // once needle is longer that compare, can't match
            return NULL;
        }
        if (strncmp(&haystack[i], needle, needle_length) == 0) {    // now simple compare at index
            return &haystack[i];
        }
    }
    return NULL;
}


/**
 *  \brief Scans the source string for fromChr and replaces with toChr. Usefull for substitution of special char in query strings.
 * 
 *  \param [in\out] srcStr - Char pointer to the c-string containing required substitutions 
 *  \param [in] fromChr - Char value to replace in src 
 *  \param [in] toChr - Char value to put in place of fromChr 
 * 
 *  \return Number of substitutions made
*/
uint16_t lq_strReplace(char *srcStr, char fromChr, char toChr)
{
    char *opPtr = srcStr;
    uint16_t replacements = 0;

    if (fromChr == 0 || toChr == 0 || fromChr == toChr)
        return 0;

    while(*opPtr != 0)
    {
        if (*opPtr == fromChr)
        {
            *opPtr = toChr;
            replacements++;
        }
        opPtr++;
    }
    return replacements;
}


/**
 *  \brief Performs URL escape removal for special char (%20-%2F) without malloc.
 * 
 *  \param src [in] - Input text string to URL decode.
 *  \param len [in] - Length of input text string.
*/
uint16_t lq_strUriDecode(char *src, int len)
{
    char subTable[] = " !\"#$%&'()*+,-./";
    uint8_t srcChar;
    uint8_t subKey;
    uint16_t dest = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (src[i] == '\0')
            break;
        if (src[i] == '%')
        {
            srcChar = src[i + 2];
            subKey = (srcChar >= 0x30 && srcChar <= 0x39) ? srcChar - 0x30 : \
                     ((srcChar >= 0x41 && srcChar <= 0x46) ? srcChar - 0x37 : \
                     ((srcChar >= 0x61 && srcChar <= 0x66) ? srcChar - 0x57: 255));
            if (subKey != 255)
            {
                src[dest] = subTable[subKey];
                i += 2;
            }
        }
        else
            src[dest] = src[i];
        
        dest++;
    }
    src[dest] = '\0';
    return dest;
}


/**
 *  \brief Scans a C-String (char array) for the next delimeted token and null terminates it.
 * 
 *  \param source [in] - Original char array to scan.
 *  \param delimeter [in] - Character separating tokens (passed as integer value).
 *  \param tokenBuf [out] - Pointer to where token should be copied to.
 *  \param tokenBufSz [in] - Size of buffer to receive token.
 * 
 *  \return Pointer to the location in the source string immediately following the token.
*/
static char *lq_strGrabToken(char *source, int delimiter, char *tokenBuf, uint8_t tokenBufSz) 
{
    char *delimAt;
    if (source == NULL)
        return 0;

    delimAt = strchr(source, delimiter);
    uint8_t tokenSz = delimAt - source;
    if (tokenSz == 0)
        return NULL;

    memset(tokenBuf, 0, tokenBufSz);
    memcpy(tokenBuf, source, MIN(tokenSz, tokenBufSz-1));
    return delimAt + 1;
}
