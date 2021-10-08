//
//  string_type.h
//  Pods
//
//  Created by Bill Sun on 2019/10/16.
//

#ifndef string_type_h
#define string_type_h

#include <stdint.h>

#define UTF8_C(x)  UINT8_C(x)
#define UTF16_C(x) UINT16_C(x)
#define UTF32_C(x) UINT32_C(x)

#define UTF8CHAR_MAX  UINT8_MAX
#define UTF16CHAR_MAX UINT16_MAX
#define UTF32CHAR_MAX UINT32_MAX

typedef uint8_t  UTF8Char;
typedef uint16_t UTF16Char;
typedef uint32_t UTF32Char;

typedef UTF8Char  * UTF8Str;
typedef UTF16Char * UTF16Str;
typedef UTF32Char * UTF32Str;

#endif /* string_type_h */
