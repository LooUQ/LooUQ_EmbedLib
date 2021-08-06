#include "lq-str.h"


char *lq_strnstr(char *haystack, char *needle, size_t length)
{
    size_t needle_length = strlen(needle);
    size_t i;
    for (i = 0; i < length; i++) {
        if (i + needle_length > length) {
            return NULL;
        }
        if (strncmp(&haystack[i], needle, needle_length) == 0) {
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


