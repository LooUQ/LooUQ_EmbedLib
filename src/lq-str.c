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

#include <string.h>

#include "lqdiag.h"
#include "lq-str.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 *  \brief Constrained string search, only check haystack for length characters. Allows for deterministic searching non-NULL terminated char sequences.
*/
const char * lq_strnstr(const char *haystack, const char *needle, size_t maxSearch)
{
    size_t needleSz = strlen(needle);
    if (needleSz > maxSearch)
        return NULL;

    for (size_t i = 0; i < maxSearch; i++) {
        if (i + needleSz > maxSearch)                          // once needle is longer that compare, can't match
        {
            return NULL;
        }
        if (strncmp(&haystack[i], needle, needleSz) == 0)      // now simple compare at index
        {
            return &haystack[i];
        }
    }
    return NULL;
}


/**
 *  \brief Scans the source string for fromChr and replaces with toChr. Usefull for substitution of special char in query strings.
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
*/
char *lq_strGrabToken(char *source, int delimiter, char *tokenBuf, uint8_t tokenBufSz) 
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


const char* lq_strFindField(char fldType, char* source)
{
    char *responsePtr = source;

    if (fldType < 61)                                               // handle mixed case field type specifier
        fldType += 0x20;

    if (fldType == '-')                                             // alpha-numeric
    {
        while (*responsePtr != '\0')
        {
            if ('0' < *responsePtr && *responsePtr < '9' ||
                'a' < *responsePtr && *responsePtr < 'z' ||
                'A' < *responsePtr && *responsePtr < 'Z')
                break;
            responsePtr++;
        }
    }
    else if (fldType == 'n')                                        // numeric
    {
        while (*responsePtr != '\0')
        {
            if ('0' < *responsePtr && *responsePtr < '9')
                break;
            responsePtr++;
        }
    }
    else if (fldType == 'h')                                        // hex-digits
    {
        while (*responsePtr != '\0')
        {
            if ('0' < *responsePtr && *responsePtr < '9' ||
                'a' < *responsePtr && *responsePtr < 'f' ||
                'A' < *responsePtr && *responsePtr < 'F')
                break;
            responsePtr++;
        }
    }
    else if (fldType == 'a')                                        // alpha
    {
        while (*responsePtr != '\0')
        {
            if ('a' < *responsePtr && *responsePtr < 'z' ||
                'A' < *responsePtr && *responsePtr < 'Z')
                break;
            responsePtr++;
        }
    }
    else if (fldType == 'u')                                        // upper-case alpha
    {
        while (*responsePtr != '\0')
        {
            if ('A' < *responsePtr && *responsePtr < 'Z')
                break;
            responsePtr++;
        }
    }
    else if (fldType == 'l')                                       // lower-case alpha
    {
        while (*responsePtr != '\0')
        {
            if ('a' < *responsePtr && *responsePtr < 'z')
                break;
            responsePtr++;
        }
    }
    else if (fldType == '*')                                        // printable
    {
        while (*responsePtr != '\0')
        {
            if ((char)0x20 < *responsePtr && *responsePtr < (char)0x7f)
                break;
            responsePtr++;
        }
    }
    
    else 
        return "";

    return responsePtr;
}
