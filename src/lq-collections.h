/******************************************************************************
 *  \file lq-collections.h
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
 ******************************************************************************
 * Data collection functions used by LQ Cloud
 *****************************************************************************/

#ifndef __LQ_COLLECTIONS_H__
#define __LQ_COLLECTIONS_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum {
    lqCollections__maxKeyValueDictCnt = 15
};

/** 
 *  @brief Struct exposing action's parameters collection (names and values as c-strings).
 * 
 *  NOTE: This struct maps key/value pairs in an existing HTTP query string formated char array. The array is parsed
 *  using the lqc_parseQueryStringDict() function. Parsing MUTATES the original char array. The original char array
 *  must stay in scope as it contains the keys and the values. The keyValueDict struct only provides a higher level
 *  map of the data stored in the char array and a utility function lqc_getDictValue(key) to access the value.
 * 
 *  If copy (ex: free source) of underlying char array needed: memcpy(destination_ptr, keys[0], length)
*/
typedef struct keyValueDict_tag
{
    uint8_t count;                                      ///< During parsing, how many properties (name/value pairs) were mapped.
    uint16_t length;                                    ///< Underlying char array original length, use if copy needed to free source
    char *keys[lqCollections__maxKeyValueDictCnt];      ///< Array of property keys.
    char *values[lqCollections__maxKeyValueDictCnt];    ///< Array of property values (as c-strings). Application is responsible for any type conversion.
} keyValueDict_t;


typedef enum lqJsonPropType_tag
{
    lqcJsonPropType_notFound = 0,
    lqcJsonPropType_object = 1,
    lqcJsonPropType_array = 2,
    lqcJsonPropType_text = 3,
    lqcJsonPropType_bool = 4,
    lqcJsonPropType_int = 5,
    lqcJsonPropType_float = 6,
    lqcJsonPropType_null = 9
} lqJsonPropType_t;


/**
 * @brief Structure representing a property/value pair in a collection.
 */
typedef struct lqJsonPropValue_tag
{
    char *value;                                            ///< Pointer to char representation of the value
    uint16_t len;                                           ///< Value's length
    lqJsonPropType_t type;                                  ///< Value's type
} lqJsonPropValue_t;


/**
 * @brief Alias for action/parameters type.
 */
typedef keyValueDict_t actnParams_t;


#ifdef __cplusplus
extern "C"
{
#endif


// Query String Dictionary

/**
 * @brief Converts a character array holding a URL-style key/value collection into a LooUQ keyValueDict_t
 * 
 * @param [in] dictSrc Sournce character array
 * @param [in] qsSize 
 * @return keyValueDict_t 
 */
keyValueDict_t lq_createQryStrDictionary(char *dictSrc, size_t qsSize);

/**
 * @brief Get a value from a dictionary
 * 
 * @param key 
 * @param dict 
 * @param value 
 * @param valSz 
 */
void lq_getQryStrDictionaryValue(const char *key, keyValueDict_t dict, char *value, uint8_t valSz);


/**
 * @brief JSON (body) Documents
 * 
 * @param jsonSrc 
 * @param propName 
 * @return lqJsonPropValue_t 
 */
lqJsonPropValue_t lq_getJsonPropValue(const char *jsonSrc, const char *propName);


// MOVED to LQ-DeviceCommon
// char *lqc_getActionParamValue(const char *paramName, keyValueDict_t actnParams);
// lqcJsonProp_t lqc_getJsonProp(const char *jsonSrc, const char *propName);
// keyValueDict_t lqc_parseQueryStringDict(char *dictSrc);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif  /* !__LQ_COLLECTIONS_H__ */
