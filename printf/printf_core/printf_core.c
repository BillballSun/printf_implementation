/*
 * Created by Bill Sun. All rights reserved.
 *
 * Email: captainallredbillball@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Bill Sun
 * 4. The name of the Author must not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *    printf_core.c 2019/10/15
 */

#include <stdio.h>
#include <float.h>
#include <wchar.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "CFMacro.h"
#include "printf_core.h"
#include "string_type.h"
#include "extended_integer.h"

#pragma mark - Macro

#pragma mark Behaviour Control

#pragma mark interprate parsing

/*!
    @name PRINTF_DISABLE_FLAG_ZERO_WHEN_FLAG_NIMUS_PRESENT
    @discussion [GNUC implementation] enabled
                [C standard] vague
    @author Bill says he doesn't much know, you may choose to #define or #undef it
 */
#define PRINTF_DISABLE_FLAG_ZERO_WHEN_FLAG_NIMUS_PRESENT

#pragma mark integer output

/*!
   @name printf disable flag zero for integer sepcifier with precision specified
   @discussion [GNUC implementation] enabled
               [C standard] vague
   @author Bill says he doesn't much know, you may choose to #define or #undef it
*/
#define PRINTF_DISBALE_FLAG_ZERO_FOR_INTEGER_SPECIFIER_WITH_PRECISION_SPECIFIED

/*!
   @name printf disable flag complex for integer value zero with precision zero
   @discussion [GNUC implementation] enabled for specifier x and X, disabled for specifier o
               [C standard] vague
   @author Bill says GNUC seems once had a argument with members
*/
#define PRINTF_DISBALE_FLAG_COMPLEX_FOR_INTEGER_VALUE_ZERO_WITH_PRECISION_ZERO

#pragma mark floating point specifier a type output

/*!
   @name printf floating point specifier %a unspecified precision output needed
   @discussion [GNUC implementation] enabled
               [C standard] not enabled
   @author Bill says GNUC not quite C standard
*/
#define PRINTF_FLOATING_POINT_SPECIFIER_A_UNSPECIFIED_PRECISION_OUTPUT_NEEDED

#pragma mark string type

/*!
   @name printf enable null string output
   @discussion [GNUC implementation] enabled
               [C standard] vague
   @author Bill says this should be done
*/
#define PRINTF_ENABLE_NULL_STRING_OUTPUT

#pragma mark printf limit

/*!
    @name PRINTF_INTEGER_MAX_DIGITS_AMOUNT
    @discussion max ouput digits amount for d i o u x X specifier
 */
#define PRINTF_INTEGER_MAX_DIGITS_AMOUNT    1024

/*!
    @name PRINTF_PRECISON_LIMIT
    @discussion this is a vague limit and may not be valid
 */
#define PRINTF_PRECISON_LIMIT               1024

/*!
   @name PRINTF_CHAR_VALUE_MAX
   @discussion max value for reading from int type specifier [ %c ]
*/
#define PRINTF_CHAR_VALUE_MAX               0xFF

#pragma mark character

#define CHARACTER_null          0x00
#define CHARACTER_space         0x20
#define CHARACTER_hash          0x23
#define CHARACTER_percentage    0x25
#define CHARACTER_left_brace    0x28
#define CHARACTER_right_brace   0x29
#define CHARACTER_asterisk      0x2A
#define CHARACTER_plus          0x2B
#define CHARACTER_minus         0x2D
#define CHARACTER_dot           0x2E
#define CHARACTER_0             0x30
#define CHARACTER_1             0x31
#define CHARACTER_2             0x32
#define CHARACTER_3             0x33
#define CHARACTER_4             0x34
#define CHARACTER_5             0x35
#define CHARACTER_6             0x36
#define CHARACTER_7             0x37
#define CHARACTER_8             0x38
#define CHARACTER_9             0x39
#define CHARACTER_A             0x41
#define CHARACTER_B             0x42
#define CHARACTER_C             0x43
#define CHARACTER_D             0x44
#define CHARACTER_E             0x45
#define CHARACTER_F             0x46
#define CHARACTER_G             0x47
#define CHARACTER_I             0x49
#define CHARACTER_N             0x4E
#define CHARACTER_P             0x50
#define CHARACTER_L             0x4C
#define CHARACTER_X             0x58
#define CHARACTER_a             0x61
#define CHARACTER_b             0x62
#define CHARACTER_c             0x63
#define CHARACTER_d             0x64
#define CHARACTER_e             0x65
#define CHARACTER_f             0x66
#define CHARACTER_g             0x67
#define CHARACTER_h             0x68
#define CHARACTER_i             0x69
#define CHARACTER_j             0x6A
#define CHARACTER_l             0x6C
#define CHARACTER_n             0x6E
#define CHARACTER_o             0x6F
#define CHARACTER_p             0x70
#define CHARACTER_s             0x73
#define CHARACTER_t             0x74
#define CHARACTER_u             0x75
#define CHARACTER_x             0x78
#define CHARACTER_z             0x7A

#pragma mark UTF8 mask

/*!
    @name UTF8Mask[....]0
    @abstract if zero exit at that point
 */
#define UTF8Mask0(mask)        (!(((UTF8Char)(1 << 7)) & (mask)))
#define UTF8Mask10(mask)       (!(((UTF8Char)(1 << 6)) & (mask)))
#define UTF8Mask110(mask)      (!(((UTF8Char)(1 << 5)) & (mask)))
#define UTF8Mask1110(mask)     (!(((UTF8Char)(1 << 4)) & (mask)))
#define UTF8Mask11110(mask)    (!(((UTF8Char)(1 << 3)) & (mask)))
#define UTF8Mask111110(mask)   (!(((UTF8Char)(1 << 2)) & (mask)))
#define UTF8Mask1111110(mask)  (!(((UTF8Char)(1 << 1)) & (mask)))
#define UTF8Mask11111110(mask) (!(((UTF8Char)(1 << 0)) & (mask)))
#define UTF8AllBitTest(value)  (!((~((UTF8Char)0)) ^ ((UTF8Char)(value))))

#pragma mark stack allocation

#define PRINTF_STACK_ALLOCATION_LIMIT   1024

#define printf_stack_alloc(size) __builtin_alloca(size)

#pragma mark error handle

/*!
    @name PRINTF_ERROR_RETURN
    @discussion a negative value used for general case error return value internally and externally
 */
#define PRINTF_ERROR_RETURN -1

#pragma mark check environment

#if defined __GNUC__
  #if   defined __LITTLE_ENDIAN__
    #define PRINTF_LITTLE_ENDIAN
  #elif defined __BIG_ENDIAN__
    #define PRINTF_BIG_ENDIAN
  #else
    #error GUNC endianess not found [ predefined macro ]
  #endif
#else
  #error endianness could not be determinated \
         comment this line and adjust below if need
  #define PRINTF_LITTLE_ENDIAN
#endif

#pragma mark - static storage

static const UTF8Char base8_character[] = {
    CHARACTER_0,    CHARACTER_1,    CHARACTER_2,    CHARACTER_3,
    CHARACTER_4,    CHARACTER_5,    CHARACTER_6,    CHARACTER_7,
};

static const UTF8Char base10_character[] = {
    CHARACTER_0,    CHARACTER_1,    CHARACTER_2,    CHARACTER_3,
    CHARACTER_4,    CHARACTER_5,    CHARACTER_6,    CHARACTER_7,
    CHARACTER_8,    CHARACTER_9
};

static const UTF8Char base16_character_lowercase[] = {
    CHARACTER_0,    CHARACTER_1,    CHARACTER_2,    CHARACTER_3,
    CHARACTER_4,    CHARACTER_5,    CHARACTER_6,    CHARACTER_7,
    CHARACTER_8,    CHARACTER_9,    CHARACTER_a,    CHARACTER_b,
    CHARACTER_c,    CHARACTER_d,    CHARACTER_e,    CHARACTER_f
};

static const UTF8Char base16_character_uppercase[] = {
    CHARACTER_0,    CHARACTER_1,    CHARACTER_2,    CHARACTER_3,
    CHARACTER_4,    CHARACTER_5,    CHARACTER_6,    CHARACTER_7,
    CHARACTER_8,    CHARACTER_9,    CHARACTER_A,    CHARACTER_B,
    CHARACTER_C,    CHARACTER_D,    CHARACTER_E,    CHARACTER_F
};

#pragma mark - UTF8

/*!
   @name PRINTF_UINT32_TO_UTF8CHAR_ARRAY_LENGTH_MAX
   @abstract used by printf_UTF32Char_to_UTF8Char function
*/
#define PRINTF_UINT32_TO_UTF8CHAR_ARRAY_LENGTH_MAX  7

/*!
    @name PRINTF_WCHAR_TO_UTF8CHAR_ARRAY_LENGTH_MAX
    @abstract used by printf_wchar_to_UTF8Char function
 */
#define PRINTF_WCHAR_TO_UTF8CHAR_ARRAY_LENGTH_MAX   \
        PRINTF_UINT32_TO_UTF8CHAR_ARRAY_LENGTH_MAX

#ifdef DEBUG
CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_UNUSED_FUNCTION
static void __HIDDEN__check_wchar_type(void) {  // DO NOT DELETE
    COMILE_ASSERT(sizeof(wchar_t) <= sizeof(uint32_t));
}
CLANG_DIAGNOSTIC_POP
#endif

#pragma mark UTF32Char to UTF8Char

/*!
    @function printf_wchar_to_UTF8Char
    @param ch the character to convert
    @param array if all-right used to store the result
    @param array_size before call this function, this stored the max size for the array, on return the exact need for wch returned
    @return true if convert success, false if error occurs, or more size need [ check if array_size is larger than input ]
 */
static bool printf_UTF32Char_to_UTF8Char(UTF32Char ch, UTF8Char * _Nonnull array, size_t * _Nonnull array_size) {
    if(array != NULL && array_size != NULL && array_size[0] > 0) {
        // 0xxxxxxx                                                             7       0x7F
        // 110xxxxx 10xxxxxx                                                    11      0x7FF
        // 1110xxxx 10xxxxxx 10xxxxxx                                           16      0xFFFF
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx                                  21      0X1FFFFF
        // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx                         26      0X3FFFFFF
        // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx                31      0X7FFFFFFF
        // 11111110 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx       36      0XFFFFFFFFF
        UTF8Char trail_head = 0x80;                     // 10000000
        UTF8Char trail_mask = 0x3F;                     // 00111111
        unsigned int trail_offset = 6;                  // valid bit count for trail_mask
        if(ch <= 0x7F) {
            if(array_size[0] >= 1) { array_size[0] = 1;
                array[0] = ch;
                return true;
            } else { array_size[0] = 1; return false; }
        }
        else if(ch <= 0x7FF) {
            if(array_size[0] >= 2) { array_size[0] = 2;
                UTF8Char lead_head = 0xC0;              // 11000000
                UTF8Char m1 =  ch >> (trail_offset * 1);
                UTF8Char m2 = (ch >> (trail_offset * 0)) & trail_mask;
                array[0] =  lead_head | m1;
                array[1] = trail_head | m2;
                return true;
            } else { array_size[0] = 2; return false; }
        }
        else if(ch <= 0xFFFF) {
            if(array_size[0] >= 3) { array_size[0] = 3;
                UTF8Char lead_head = 0xE0;              // 11100000
                UTF8Char m1 =  ch >> (trail_offset * 2);
                UTF8Char m2 = (ch >> (trail_offset * 1)) & trail_mask;
                UTF8Char m3 = (ch >> (trail_offset * 0)) & trail_mask;
                array[0] =  lead_head | m1;
                array[1] = trail_head | m2;
                array[2] = trail_head | m3;
                return true;
            } else { array_size[0] = 3; return false; }
        }
        else if(ch <= 0X1FFFFF) {
            if(array_size[0] >= 4) { array_size[0] = 4;
                UTF8Char lead_head = 0xF0;              // 11110000
                UTF8Char m1 =  ch >> (trail_offset * 3);
                UTF8Char m2 = (ch >> (trail_offset * 2)) & trail_mask;
                UTF8Char m3 = (ch >> (trail_offset * 1)) & trail_mask;
                UTF8Char m4 = (ch >> (trail_offset * 0)) & trail_mask;
                array[0] =  lead_head | m1;
                array[1] = trail_head | m2;
                array[2] = trail_head | m3;
                array[3] = trail_head | m4;
                return true;
            } else { array_size[0] = 4; return false; }
        }
        else if(ch <= 0X3FFFFFF) {
            if(array_size[0] >= 5) { array_size[0] = 5;
                UTF8Char lead_head = 0xF8;              // 11111000
                UTF8Char m1 =  ch >> (trail_offset * 4);
                UTF8Char m2 = (ch >> (trail_offset * 3)) & trail_mask;
                UTF8Char m3 = (ch >> (trail_offset * 2)) & trail_mask;
                UTF8Char m4 = (ch >> (trail_offset * 1)) & trail_mask;
                UTF8Char m5 = (ch >> (trail_offset * 0)) & trail_mask;
                array[0] =  lead_head | m1;
                array[1] = trail_head | m2;
                array[2] = trail_head | m3;
                array[3] = trail_head | m4;
                array[4] = trail_head | m5;
                return true;
            } else { array_size[0] = 5; return false; }
        }
        else if(ch <= 0X7FFFFFFF) {
            if(array_size[0] >= 6) { array_size[0] = 6;
                UTF8Char lead_head = 0xFC;              // 11111100
                UTF8Char m1 =  ch >> (trail_offset * 5);
                UTF8Char m2 = (ch >> (trail_offset * 4)) & trail_mask;
                UTF8Char m3 = (ch >> (trail_offset * 3)) & trail_mask;
                UTF8Char m4 = (ch >> (trail_offset * 2)) & trail_mask;
                UTF8Char m5 = (ch >> (trail_offset * 1)) & trail_mask;
                UTF8Char m6 = (ch >> (trail_offset * 0)) & trail_mask;
                array[0] =  lead_head | m1;
                array[1] = trail_head | m2;
                array[2] = trail_head | m3;
                array[3] = trail_head | m4;
                array[4] = trail_head | m5;
                array[5] = trail_head | m6;
                return true;
            } else { // array_size[0] = 6; un-commnent if need to implement [unlikely ?]
                return false;
            }
        }
        else {  // may just return false to indicate failure ?
            DEBUG_ASSERT(UINT32_MAX <= 0xFFFFFFFFF);
            DEBUG_RETURN(false);
        }
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_wchar_to_UTF8Char
    @param wch the character to convert
    @param array if all-right used to store the result
    @param array_size before call this function, this stored the max size for the array, on return the exact need for wch returned
    @return true if convert success, false if error occurs, or more size need [ check if array_size is larger than input ]
 */
static bool printf_wchar_to_UTF8Char(wchar_t wch, UTF8Char * _Nonnull array, size_t * _Nonnull array_size) {
    if(array != NULL && array_size != NULL && array_size[0] > 0) {
        if(wch == WEOF) return false;
        return printf_UTF32Char_to_UTF8Char(wch, array, array_size);
    } DEBUG_ELSE
    return false;
}

#pragma mark convert to UT8Str

/*!
    @typedef printf_UTF8_convert_callback_func_t
    @abstract used when convert UTF8String for receiving
    @param array_size should be more than zero
    @param input passing down as additional variable
    @return return false if anything wrong
 */
typedef bool (* printf_UTF8_convert_callback_func_t)
             (UTF8Char * _Nonnull array, size_t array_size, void * _Nullable input);

#pragma mark UTF16Str to UTF8Str

#define HIGH_SURROGATE_BEGIN    UTF16_C(0xD800)
#define HIGH_SURROGATE_END      UTF16_C(0xDBFF)

#define LOW_SURROGATE_BEGIN     UTF16_C(0xDC00)
#define LOW_SURROGATE_END       UTF16_C(0xDFFF)

/*!
    @function printf_UTF16Str_to_UTF8Char
    @abstract used to assistant convert UTF16Str to UTF8Str
    @param func called each character is converted, return false to terminate convertion and this function return false too
    @param max_character_parsing max character could be read from str
    @param character_count when not NULL and return true the character count are stored in it, no more than max_character_parsing
    @param UTF8Str_store_length when not NULL and return true the array length used to store all UTF8 character returned not counting final null-character
    @return true if convertion success, false otherwise
 */
static bool printf_UTF16Str_to_UTF8Char(UTF16Str _Nonnull str,
                                        printf_UTF8_convert_callback_func_t _Nullable func,
                                        void * _Nullable input,
                                        size_t * _Nullable max_character_parsing,
                                        size_t * _Nullable character_count,
                                        size_t * _Nullable UTF8Str_store_length) {
    if(str != NULL) {
        UTF16Str current = str;
        size_t internal_character_count = 0;
        size_t internal_UTF8Str_store_length = 0;
        
        UTF32Char ch = 0;
        UTF8Char array[PRINTF_UINT32_TO_UTF8CHAR_ARRAY_LENGTH_MAX];
        size_t array_size;
        
        while(current[0] != UTF16_C(0)) {
            if(max_character_parsing != NULL && internal_character_count + 1 > max_character_parsing[0])
                break;
            if(current[0] >= HIGH_SURROGATE_BEGIN && current[0] <= HIGH_SURROGATE_END) {
                // supplementary plane
                if(current[1] >= LOW_SURROGATE_BEGIN && current[1] <= LOW_SURROGATE_END) {
                    // this even valid if current[1] == UINT16_C(0)
                    COMILE_ASSERT(UTF32CHAR_MAX >= 0x10FFFF);    // supplementary plane
                    COMILE_ASSERT(UTF32CHAR_MAX >= UTF16CHAR_MAX);
                    UTF32Char high = current[0];
                    UTF32Char low = current[1];
                    high -= HIGH_SURROGATE_BEGIN;
                    low -= LOW_SURROGATE_BEGIN;
                    UTF32Char ch = low;
                    high = high << 10;      // USC-2 defined 10 bit offset
                    ch |= high;
                    current += 2;
                } ELSE_DEVELOP_BREAKPOINT_RETURN(false);
                // [BREAKPOINT] the format is not a valide USC-2 string
                // this should not happen under Cocoa framework as
                // everything is USC-4 format
            }
            else {ch = current[0]; current += 1;}
            // currently ch is a valid UTF32 character with 10 plane
            DEVELOP_ASSERT(ch <= 0x10FFFF);
            // [BREAKPOINT] this character is not currently encoded into UNICODE
            // and what a fxxk is it ?
            array_size = ARRAY_SIZE(array);
            if(printf_UTF32Char_to_UTF8Char(ch, array, &array_size)) {
                DEBUG_ASSERT(array_size > 0);
                if(func != NULL && !func(array, array_size, input))
                    DEVELOP_BREAKPOINT_RETURN(false);
                internal_UTF8Str_store_length += array_size;
            } ELSE_DEVELOP_BREAKPOINT_RETURN(false);
            // [BREAKPOINT] this is not a valid UTF32 character to be encoded into UTF8 form
            internal_character_count++;     // always adavace 1
        }
        if(character_count != NULL)
            character_count[0] = internal_character_count;
        if(UTF8Str_store_length != NULL)
            UTF8Str_store_length[0] = internal_UTF8Str_store_length;
        return true;
    } DEBUG_ELSE
    return false;
}

#pragma mark UTF32Str to UTF8Str

/*!
    @function printf_UTF32Str_to_UTF8Char
    @abstract used to assistant convert UTF32Str to UTF8Str
    @param func called each character is converted, return false to terminate convertion and this function return false too
    @param max_character_parsing max character could be read from str
    @param character_count when not NULL and return true the character count are stored in it, no more than max_character_parsing
    @param UTF8Str_store_length when not NULL and return true the array length used to store all UTF8 character returned not counting final null-character
    @return true if convertion success, false otherwise
 */
static bool printf_UTF32Str_to_UTF8Char(UTF32Str _Nonnull str,
                                        printf_UTF8_convert_callback_func_t _Nullable func,
                                        void * _Nullable input,
                                        size_t * _Nullable max_character_parsing,
                                        size_t * _Nullable character_count,
                                        size_t * _Nullable UTF8Str_store_length) {
    if(str != NULL) {
        UTF32Str current = str;
        size_t internal_character_count = 0;
        size_t internal_UTF8Str_store_length = 0;

        UTF8Char array[PRINTF_UINT32_TO_UTF8CHAR_ARRAY_LENGTH_MAX];
        size_t array_size;
        
        while(current[0] != UTF32_C(0)) {
            if(max_character_parsing != NULL && internal_character_count + 1 > max_character_parsing[0])
                break;
            array_size = ARRAY_SIZE(array);
            if(printf_UTF32Char_to_UTF8Char(current[0], array, &array_size)) {
                DEBUG_ASSERT(array_size > 0);
                if(func != NULL && !func(array, array_size, input)) DEVELOP_BREAKPOINT_RETURN(false);
                internal_UTF8Str_store_length += array_size;
            } ELSE_DEVELOP_BREAKPOINT_RETURN(false);
            current += 1;
            internal_character_count++;     // always adavace 1
        }
        if(character_count != NULL)
            character_count[0] = internal_character_count;
        if(UTF8Str_store_length != NULL)
            UTF8Str_store_length[0] = internal_UTF8Str_store_length;
        return true;
    } DEBUG_ELSE
    return false;
}

#pragma mark UTF8 Validation

/*!
    @function printf_utf8_validate
    @param string a pointer to UTF8 string for validation
    @discussion it is safe to pass non-UTF8 encoding string [ null-terminated ]
    @param count if not NULL when return true we store the actual UTF8 character count into it
    @return true if a valid UTF8 string
 */
static bool printf_UTF8Str_validate(const UTF8Char * _Nonnull string, size_t * _Nullable count) {
    if(string != NULL) {
        size_t internal_count = 0;
        const UTF8Char * _Nonnull current = string;
        while (current[0] != CHARACTER_null)
            if(UTF8Mask0(current[0])) { current++; internal_count++; }
            else if(UTF8Mask10(current[0])) return false;
            else if(UTF8AllBitTest(current[0])) return false;
            else if(UTF8Mask110(current[0])) {  // 110xxxxx 10xxxxxx
                // this test make sure match 10xxxxxx, no need to test null-existance
                if(UTF8Mask0(current[1]) || !UTF8Mask10(current[1])) return false;
                current += 2; internal_count++;
            }
            else if(UTF8Mask1110(current[0])) {  // 1110xxxx 10xxxxxx 10xxxxxx
                if(UTF8Mask0(current[1]) || !UTF8Mask10(current[1])) return false;
                if(UTF8Mask0(current[2]) || !UTF8Mask10(current[2])) return false;
                current += 3; internal_count++;
            }
            else if(UTF8Mask11110(current[0])) {  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                if(UTF8Mask0(current[1]) || !UTF8Mask10(current[1])) return false;
                if(UTF8Mask0(current[2]) || !UTF8Mask10(current[2])) return false;
                if(UTF8Mask0(current[3]) || !UTF8Mask10(current[3])) return false;
                current += 4; internal_count++;
            }
            else if(UTF8Mask111110(current[0])) {  // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
                if(UTF8Mask0(current[1]) || !UTF8Mask10(current[1])) return false;
                if(UTF8Mask0(current[2]) || !UTF8Mask10(current[2])) return false;
                if(UTF8Mask0(current[3]) || !UTF8Mask10(current[3])) return false;
                if(UTF8Mask0(current[4]) || !UTF8Mask10(current[4])) return false;
                current += 5; internal_count++;
            }
            else if(UTF8Mask1111110(current[0])) {  // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
                if(UTF8Mask0(current[1]) || !UTF8Mask10(current[1])) return false;
                if(UTF8Mask0(current[2]) || !UTF8Mask10(current[2])) return false;
                if(UTF8Mask0(current[3]) || !UTF8Mask10(current[3])) return false;
                if(UTF8Mask0(current[4]) || !UTF8Mask10(current[4])) return false;
                if(UTF8Mask0(current[5]) || !UTF8Mask10(current[5])) return false;
                current += 6; internal_count++;
            }
            else if(UTF8Mask11111110(current[0])) {  // 11111110 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
                if(UTF8Mask0(current[1]) || !UTF8Mask10(current[1])) return false;
                if(UTF8Mask0(current[2]) || !UTF8Mask10(current[2])) return false;
                if(UTF8Mask0(current[3]) || !UTF8Mask10(current[3])) return false;
                if(UTF8Mask0(current[4]) || !UTF8Mask10(current[4])) return false;
                if(UTF8Mask0(current[5]) || !UTF8Mask10(current[5])) return false;
                if(UTF8Mask0(current[6]) || !UTF8Mask10(current[6])) return false;
                current += 7; internal_count++;
            }
            ELSE_DEVELOP_BREAKPOINT_RETURN(false);
        if(count != NULL) count[0] = internal_count;
        return true;
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_UTF8_character_validate
    @param character point to a valid multi-byte string terminated with null character
    @param length if return true and not null, the length of character if stored
    @abstract validate null if return true, and length is store 1
    @return if it is a valid UTF8 character
 */
static bool printf_UTF8_character_validate(const UTF8Char * _Nonnull character, size_t * _Nullable length) {
    if(character != NULL) {
        size_t internal_length;
        if(UTF8Mask0(character[0])) {       // this include null character
            internal_length = 1;
        }
        else if(UTF8Mask10(character[0])) return false;
        else if(UTF8AllBitTest(character[0])) return false;
        else if(UTF8Mask110(character[0])) {  // 110xxxxx 10xxxxxx
            // this test make sure match 10xxxxxx, no need to test null-existance
            if(UTF8Mask0(character[1]) || !UTF8Mask10(character[1])) return false;
            internal_length = 2;
        }
        else if(UTF8Mask1110(character[0])) {  // 1110xxxx 10xxxxxx 10xxxxxx
            if(UTF8Mask0(character[1]) || !UTF8Mask10(character[1])) return false;
            if(UTF8Mask0(character[2]) || !UTF8Mask10(character[2])) return false;
            internal_length = 3;
        }
        else if(UTF8Mask11110(character[0])) {  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            if(UTF8Mask0(character[1]) || !UTF8Mask10(character[1])) return false;
            if(UTF8Mask0(character[2]) || !UTF8Mask10(character[2])) return false;
            if(UTF8Mask0(character[3]) || !UTF8Mask10(character[3])) return false;
            internal_length = 4;
        }
        else if(UTF8Mask111110(character[0])) {  // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            if(UTF8Mask0(character[1]) || !UTF8Mask10(character[1])) return false;
            if(UTF8Mask0(character[2]) || !UTF8Mask10(character[2])) return false;
            if(UTF8Mask0(character[3]) || !UTF8Mask10(character[3])) return false;
            if(UTF8Mask0(character[4]) || !UTF8Mask10(character[4])) return false;
            internal_length = 5;
        }
        else if(UTF8Mask1111110(character[0])) {  // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            if(UTF8Mask0(character[1]) || !UTF8Mask10(character[1])) return false;
            if(UTF8Mask0(character[2]) || !UTF8Mask10(character[2])) return false;
            if(UTF8Mask0(character[3]) || !UTF8Mask10(character[3])) return false;
            if(UTF8Mask0(character[4]) || !UTF8Mask10(character[4])) return false;
            if(UTF8Mask0(character[5]) || !UTF8Mask10(character[5])) return false;
            internal_length = 6;
        }
        else if(UTF8Mask11111110(character[0])) {  // 11111110 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            if(UTF8Mask0(character[1]) || !UTF8Mask10(character[1])) return false;
            if(UTF8Mask0(character[2]) || !UTF8Mask10(character[2])) return false;
            if(UTF8Mask0(character[3]) || !UTF8Mask10(character[3])) return false;
            if(UTF8Mask0(character[4]) || !UTF8Mask10(character[4])) return false;
            if(UTF8Mask0(character[5]) || !UTF8Mask10(character[5])) return false;
            if(UTF8Mask0(character[6]) || !UTF8Mask10(character[6])) return false;
            internal_length = 7;
        }
        ELSE_DEVELOP_BREAKPOINT_RETURN(false);
        if(length != NULL) length[0] = internal_length;
        return true;
    } DEBUG_ELSE
    return false;
}

#pragma mark - printf_core

/*!
    @enum printf_type
    @discussion used to distinguish snprintf & fprintf
*/
typedef enum printf_type {
    printf_type_string,
    printf_type_FILE,
} printf_type;

/*!
    @struct printf_core
    @arg format printf_style format string encoded in UTF8
    @arg args va_list query for parsing
    @arg output.string.index current store index
*/
struct printf_core {
    const UTF8Char * _Nonnull format;
    va_list args;
    printf_type type;           // distiguish output type
    union {
        struct {
            size_t count;       // provided store count, if store is NULL,
                                // this should be zero
            size_t index;       // current store index,
                                // only when index + 1 < count
                                // store could be performed
                                // everytime it is point to the null-character
                                // null-terminate pending should always
                                // index should always be (index < count || index == count == 0)
            UTF8Char * _Nullable store;
        } string;
        FILE * _Nonnull file;   // assigned file output buffer
    } output;
    size_t actual_need; // return value for snprintf, not counting the terminate null character
    
#ifdef DEBUG        // used for debug perpose
    const UTF8Char * _Nonnull original_format;
    va_list original_args;
#endif
};

typedef struct printf_core printf_core_t;

typedef printf_core_t *printf_core_ref;

/*!
    @function printf_core_validate
    @abstract used to validate printf core
    @return ture if it is a valid printf_core, false otherwise
*/
static bool printf_core_validate(printf_core_ref _Nonnull core) {
    if(core != NULL && core->format != NULL) {
        DEBUG_ASSERT(core->original_format != NULL);
        switch (core->type) {
            case printf_type_string:
                if(core->output.string.store == NULL) {
                    if(core->output.string.count != 0)
                        DEBUG_RETURN(false);
                }
                else if(core->output.string.count == 0) {
                    if(core->output.string.index != 0)
                        DEBUG_RETURN(false);
                }
                else if(core->output.string.index >= core->output.string.count) {
                    DEBUG_RETURN(false);
                }
                break;
            case printf_type_FILE:
                if(core->output.file == NULL) {
                    DEBUG_RETURN(false);
                }
                break;
            default:
                DEBUG_RETURN(false);
        }
        return true;
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_core_output_initialize_inline
    @abstract used to intialize some ouput field data
*/
static inline void printf_core_output_initialize_inline(printf_core_ref _Nonnull core) {
    if(core != NULL) {
        if(core->type == printf_type_string) {
            if(core->output.string.store != NULL) {
                if(core->output.string.count >= 1)
                    core->output.string.store[0] = CHARACTER_null;
                else
                    core->output.string.store = NULL;  // optimized
            }
            core->output.string.index = 0;
            core->actual_need = 0;
        }
    } DEBUG_ELSE
}

/*!
    @function printf_core_error_handle
    @param core should be a valid printf_core
    @abstract used to handle error condition if write error occurs
*/
static void printf_core_error_handle(printf_core_ref _Nonnull core) {
    if(core != NULL && printf_core_validate(core)) {
        switch (core->type) {
            case printf_type_string:
                if(core->output.string.store != NULL)
                    for(size_t index = 0; index < core->output.string.count; index++)
                        core->output.string.store[index] = CHARACTER_null;
                break;
            case printf_type_FILE:
                break;
        }
    } DEBUG_ELSE
    DEBUG_POINT
}

#pragma mark - printf_interprate

/*

 [ printf format ]

 [ length ]
          di                  uoxX                        fFeEgGaA        c           s           p           n
 (none)   int                 unsigned int                double          int         char *      void *      int *
 hh       signed char         unsigned char                                                                   signed char *
 h        short int           unsigned short int                                                              short int *
 l        long int            unsigned long int           double          wint_t      wchar_t *               long int *
 ll       long long int       unsigned long long int                                                          long long int *
 j        intmax_t            uintmax_t                                                                       intmax_t *
 z        size_t              size_t                                                                          size_t *
 t        ptrdiff_t           ptrdiff_t                                                                       ptrdiff_t *
 L                                                        long double
 
 [ .precision ]
 A precision of 0 means that no character is written for the value 0
 If the period is specified without an explicit value for precision, 0 is assumed

 [ width ]
 Minimum number of characters to be printed
 Distiguish width and flag, no width for zero, default is zero

 [ flag ]
 - + (space) # 0
 
*/

typedef enum printf_specifier_type {
    printf_specifier_invalid,
    printf_specifier_d_or_i,
    printf_specifier_u,
    printf_specifier_o,
    printf_specifier_x,
    printf_specifier_X,
    printf_specifier_f,
    printf_specifier_F,
    printf_specifier_e,
    printf_specifier_E,
    printf_specifier_g,
    printf_specifier_G,
    printf_specifier_a,
    printf_specifier_A,
    printf_specifier_c,
    printf_specifier_s,
    printf_specifier_p,
    printf_specifier_n,
    printf_specifier_percentage
} printf_specifier_type;

typedef enum printf_interprate_flag {
    printf_interprate_flag_none              = 0,
    printf_interprate_flag_left_justify      = 1 << 0,
    printf_interprate_flag_force_sign        = 1 << 1,
    printf_interprate_flag_sign_space        = 1 << 2,
    printf_interprate_flag_complex           = 1 << 3,
    printf_interprate_flag_zero_padding = 1 << 4
} printf_interprate_flag;

typedef enum printf_interprate_adjustment_type {
    printf_interprate_adjustment_unspecified,
    printf_interprate_adjustment_fix_number,
    printf_interprate_adjustment_waiting_input,
    printf_interprate_adjustment_after_input
} printf_interprate_adjustment_type;

typedef enum printf_interprate_length_type {
    printf_interprate_length_hh,
    printf_interprate_length_h,
    printf_interprate_length_none,
    printf_interprate_length_l,
    printf_interprate_length_ll,
    printf_interprate_length_j,
    printf_interprate_length_z,
    printf_interprate_length_t,
    printf_interprate_length_L
} printf_interprate_length_type;

struct printf_interprate {
    // [length]
    printf_interprate_length_type length_type;
    
    // [.precision]
    printf_interprate_adjustment_type precision_type;
    size_t precision;
    
    // [width]
    printf_interprate_adjustment_type width_type;
    size_t width;
    
    // [flag]
    printf_interprate_flag flag;
    
    // [type]
    printf_specifier_type type;
    
    // [additional]
    const UTF8Char *start;
    const UTF8Char *end;
};

typedef struct printf_interprate printf_interprate_t;

typedef printf_interprate_t *printf_interprate_ref;

/*!
    @function printf_specifier_type_is_integer_inline
    @param specifier_type should be a valid specifier type
    @return true it is integer type
*/
static inline bool printf_specifier_type_is_integer_inline(printf_specifier_type specifier_type) {
    if(specifier_type == printf_specifier_d_or_i    ||
       specifier_type == printf_specifier_u         ||
       specifier_type == printf_specifier_o         ||
       specifier_type == printf_specifier_x         ||
       specifier_type == printf_specifier_X)
        return true;
    return false;
}

/*!
    @function printf_specifier_type_is_floating_point_inline
    @param specifier_type should be a valid specifier type
    @return true it is floating type
*/
static inline bool printf_specifier_type_is_floating_point_inline(printf_specifier_type specifier_type) {
    if(specifier_type == printf_specifier_f ||
       specifier_type == printf_specifier_F ||
       specifier_type == printf_specifier_e ||
       specifier_type == printf_specifier_E ||
       specifier_type == printf_specifier_g ||
       specifier_type == printf_specifier_G ||
       specifier_type == printf_specifier_a ||
       specifier_type == printf_specifier_A)
        return true;
    return false;
}

/*!
    @function printf_interprate_initialize
    @param specifier_type should be a valid specifier type
    @abstract intiailize an printf_interprate object according to the type
    @discussion [length] & [.precision] & [width] & [flag] field intialized according to the specifier_type
    @return true if intialize success
*/
static bool printf_interprate_initialize(printf_interprate_ref _Nonnull interprate,
                                         printf_specifier_type specifier_type) {
    DEBUG_ASSERT(interprate != NULL && specifier_type != printf_specifier_invalid);
    if(specifier_type == printf_specifier_d_or_i ||
       specifier_type == printf_specifier_o ||
       specifier_type == printf_specifier_u ||
       specifier_type == printf_specifier_x ||
       specifier_type == printf_specifier_X ||
       specifier_type == printf_specifier_p)
        interprate->precision = 1;
    else if(specifier_type == printf_specifier_a ||
            specifier_type == printf_specifier_A ||
            specifier_type == printf_specifier_e ||
            specifier_type == printf_specifier_E ||
            specifier_type == printf_specifier_f ||
            specifier_type == printf_specifier_F ||
            specifier_type == printf_specifier_g ||
            specifier_type == printf_specifier_G)
        interprate->precision = 6;
    else if(specifier_type == printf_specifier_s)
        interprate->precision = SIZE_MAX;
    else if(specifier_type == printf_specifier_c ||
            specifier_type == printf_specifier_n ||
            specifier_type == printf_specifier_percentage)
        interprate->precision = 0;
    ELSE_DEBUG_RETURN(false);
    interprate->type = specifier_type;
    interprate->flag = printf_interprate_flag_none;
    interprate->width_type = printf_interprate_adjustment_unspecified;
    interprate->width = 0;
    interprate->precision_type = printf_interprate_adjustment_unspecified;
    interprate->length_type = printf_interprate_length_none;
    return true;
}

/*!
    @function printf_interprate_fixup_inline
    @abstract make final fixup for interprate, mostly used for customized behavior control
    @return true if intialize success
*/
static inline bool printf_interprate_fixup_inline(printf_interprate_ref _Nonnull interprate) {
    if(interprate != NULL) {
#ifdef PRINTF_DISABLE_FLAG_ZERO_WHEN_FLAG_NIMUS_PRESENT
        if((interprate->flag & printf_interprate_flag_left_justify) &&
           (interprate->flag & printf_interprate_flag_zero_padding)) {
            interprate->flag &= ~ printf_interprate_flag_zero_padding;
        }
#endif
        return true;
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_interprate_check_specifier_byte_inline
    @param byte this maybe part of a long UTF8 character
    @return printf_specifier_type for a possible byte
*/
static inline printf_specifier_type printf_interprate_check_specifier_byte_inline(const UTF8Char byte) {
    switch (byte) {
        case CHARACTER_d:
        case CHARACTER_i: return printf_specifier_d_or_i;
        case CHARACTER_u: return printf_specifier_u;
        case CHARACTER_o: return printf_specifier_o;
        case CHARACTER_x: return printf_specifier_x;
        case CHARACTER_X: return printf_specifier_X;
        case CHARACTER_f: return printf_specifier_f;
        case CHARACTER_F: return printf_specifier_F;
        case CHARACTER_e: return printf_specifier_e;
        case CHARACTER_E: return printf_specifier_E;
        case CHARACTER_g: return printf_specifier_g;
        case CHARACTER_G: return printf_specifier_G;
        case CHARACTER_a: return printf_specifier_a;
        case CHARACTER_A: return printf_specifier_A;
        case CHARACTER_c: return printf_specifier_c;
        case CHARACTER_s: return printf_specifier_s;
        case CHARACTER_p: return printf_specifier_p;
        case CHARACTER_n: return printf_specifier_n;
        case CHARACTER_percentage: return printf_specifier_percentage;
        default: return printf_specifier_invalid;
    }
}

/*!
    @function printf_interprate_search_specifier_inline
    @param escape string point to escape character start which means escape[0] == CHARACTER_percentage
    @param specifier_byte used to store the specifer type if not return NULL
    @abstract search for an interprate specifier character
*/
static inline const UTF8Char * _Nullable printf_interprate_search_specifier_inline(const UTF8Char * _Nonnull escape,
                                                                                   printf_specifier_type * _Nullable specifier_byte) {
    if(escape != NULL && escape[0] == CHARACTER_percentage) {
        escape += 1;
        printf_specifier_type current_type;
        while (escape[0] != CHARACTER_null)
            if((current_type = printf_interprate_check_specifier_byte_inline(escape[0])) != printf_specifier_invalid) {
                if(specifier_byte != NULL) specifier_byte[0] = current_type;
                return escape;
            }
            else escape++;
    } DEBUG_ELSE
    return NULL;
}

/*!
    @function printf_interprate_length_parsing
    @abstract parsing interprate length field
    @discussion according to the parsingPoint [specifier] and format [% character] only affect [length] field
                if no length field found return true and not affect anything
    @param interprate if parsing success, it is used to store result
    @param format point to the escape character (%)
    @param parsingPoint point to the actual specifier, if return true, this is updated to the next parsing point
    @return true if parsing success, even no length field; false if encounter some illegal character
*/
static bool printf_interprate_length_parsing(printf_interprate_ref _Nonnull interprate,
                                             const UTF8Char * _Nonnull format,
                                             const UTF8Char * _Nonnull * _Nonnull parsingPoint) {
    if(interprate != NULL && format != NULL && parsingPoint != NULL && parsingPoint[0] != NULL && format < parsingPoint[0] && format[0] == CHARACTER_percentage) {
        printf_specifier_type specifier_type;
        const UTF8Char * _Nonnull current = parsingPoint[0];
        if((specifier_type = printf_interprate_check_specifier_byte_inline(current[0])) != printf_specifier_invalid) {
            switch (specifier_type) {
                case printf_specifier_d_or_i:
                case printf_specifier_u:
                case printf_specifier_o:
                case printf_specifier_x:
                case printf_specifier_X:
                case printf_specifier_n: {
                    bool found_length_type = false;
                    // when set found_length_type, it is responsible to set current as well
                    if(current >= format + 3) {
                        found_length_type = true;
                        if(current[-1] == CHARACTER_h && current[-2] == CHARACTER_h)
                            interprate->length_type = printf_interprate_length_hh;
                        else if(current[-1] == CHARACTER_l && current[-2] == CHARACTER_l)
                            interprate->length_type = printf_interprate_length_ll;
                        else found_length_type = false;
                        if(found_length_type) current = current - 3;     // lld
                    }
                    if(!found_length_type && current >= format + 2) {
                        found_length_type = true;
                        if(current[-1] == CHARACTER_h)
                            interprate->length_type = printf_interprate_length_h;
                        else if(current[-1] == CHARACTER_l)
                            interprate->length_type = printf_interprate_length_l;
                        else if(current[-1] == CHARACTER_j)
                            interprate->length_type = printf_interprate_length_j;
                        else if(current[-1] == CHARACTER_z)
                            interprate->length_type = printf_interprate_length_z;
                        else if(current[-1] == CHARACTER_t)
                            interprate->length_type = printf_interprate_length_t;
                        else found_length_type = false;
                        if(found_length_type) current = current - 2;     // ld
                    }
                    if(!found_length_type) current = current - 1;        // d
                }
                    break;
                    
                case printf_specifier_f:
                case printf_specifier_F:
                case printf_specifier_e:
                case printf_specifier_E:
                case printf_specifier_g:
                case printf_specifier_G:
                case printf_specifier_a:
                case printf_specifier_A: {
                    bool found_length_type = false;
                    // when set found_length_type, it is responsible to set current as well
                    if(current >= format + 2) {
                        found_length_type = true;
                        if(current[-1] == CHARACTER_L)
                            interprate->length_type = printf_interprate_length_L;
                        else if(current[-1] == CHARACTER_l)
                            interprate->length_type = printf_interprate_length_l;
                        else found_length_type = false;
                        if(found_length_type) current = current - 2;     // Lf
                    }
                    if(!found_length_type) current = current - 1;        // f
                }
                    break;
                    
                case printf_specifier_c:
                case printf_specifier_s:
                    if(current >= format + 2 && current[-1] == CHARACTER_l) {
                        interprate->length_type = printf_interprate_length_l;
                        current = current - 2;
                    }
                    else current = current - 1;
                    break;
                    
                // [ no parsing type ]
                case printf_specifier_p:
                case printf_specifier_percentage:
                    current = current - 1;
                    break;
                
                // [ unacceptable result ]
                case printf_specifier_invalid:
                default:
                    DEBUG_RETURN(false);
            }
            parsingPoint[0] = current;  // updated parsingPoint
            return true;
        } DEVELOP_ELSE_BREAKPOINT
        // [BREAKPOINT] fund a not valid specifier [not match C99 standard]
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_interprate_precision_parsing
    @abstract parsing interprate precision field
    @discussion only affect [precision] field
                if no precision field found return true and not affect anything
    @param interprate if parsing success, it is used to store result
    @param format point to the escape character (%)
    @param parsingPoint point to the precision parsing point, if return true, this is updated to the next parsing point
    @return true if parsing success, even no length field; false if encounter some illegal character
*/
static bool printf_interprate_precision_parsing(printf_interprate_ref _Nonnull interprate,
                                                const UTF8Char * _Nonnull format,
                                                const UTF8Char * _Nonnull * _Nonnull parsingPoint) {
    if(interprate != NULL && format != NULL && parsingPoint != NULL && parsingPoint[0] != NULL && format <= parsingPoint[0]) {
        const UTF8Char * _Nonnull current = parsingPoint[0];
        for(const UTF8Char *temp = current; temp > format; temp--)
            if(temp[0] == CHARACTER_dot) {
                if(temp == current) {       // only dot exist at current parsing point
                    interprate->precision_type = printf_interprate_adjustment_fix_number;
                    interprate->precision = 0;
                    current--;              // valid for this is dot(.) and before format
                    break;                  // exit precison parsing
                }
                if(temp + 1 == current && current[0] == CHARACTER_asterisk) {
                    interprate->precision_type = printf_interprate_adjustment_waiting_input;
                    parsingPoint[0] = temp - 1;     // may at most-left to [format]
                    return true;
                }
                
                // and this dot position is after % mark
                const UTF8Char * _Nonnull dot_position = temp;
                
                // temp < current
                // temp point to the dot(.) as precision
                temp++;
                
                // temp <= current
                // temp point to first possible number position for precision parsing
                // current is the last possible number position for precision parsing
                size_t precision = 0;
                while (temp <= current)
                    if(temp[0] >= CHARACTER_0 && temp[0] <= CHARACTER_9) {
#ifdef DEBUG
                        // overflow test for precision field
                        size_t overflow_test;
                        if(__builtin_mul_overflow(precision, 10, &overflow_test)) DEBUG_POINT;
                        if(__builtin_add_overflow(overflow_test, temp[0] - CHARACTER_0, &overflow_test)) DEBUG_POINT;
#endif
// [NOTE] [optimization] limited precison support
                        precision *= 10;
                        precision += temp[0] - CHARACTER_0;
                        
                        temp++; // don't miss it
                    }
                    ELSE_DEVELOP_BREAKPOINT_RETURN(false);
                    // [BREAKPOINT] unexpected character
                interprate->precision_type = printf_interprate_adjustment_fix_number;
                interprate->precision = precision;
                current = dot_position - 1;
                break;  // exit precison parsing
            }
        // break exit point [for]
        parsingPoint[0] = current;  // updated parsingPoint
        return true;
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_interprate_width_parsing
    @abstract parsing interprate width field
    @discussion only affect [width] field
                if no width field found return true and not affect anything
    @param interprate if parsing success, it is used to store result
    @param format point to the escape character (%)
    @param parsingPoint point to the precision parsing point, if return true, this is updated to the next parsing point
    @return true if parsing success, even no width field; false if encounter some illegal character
*/
static bool printf_interprate_width_parsing(printf_interprate_ref _Nonnull interprate,
                                            const UTF8Char * _Nonnull format,
                                            const UTF8Char * _Nonnull * _Nonnull parsingPoint) {
    if(interprate != NULL && format != NULL && parsingPoint != NULL && parsingPoint[0] != NULL && format <= parsingPoint[0]) {
        const UTF8Char * _Nonnull current = parsingPoint[0];
        if(current > format && current[0] == CHARACTER_asterisk) {
            interprate->width_type = printf_interprate_adjustment_waiting_input;
            parsingPoint[0] = current - 1;     // may at most-left to [format]
            return true;
        }
        if(current > format && current[0] >= CHARACTER_0 && current[0] <= CHARACTER_9) {    // enter width parsing exist
            // unless all zero, or there will be a width
            const UTF8Char *leftMostNumber = current;
            while(leftMostNumber - 1 > format && leftMostNumber[-1] >= CHARACTER_0 && leftMostNumber[-1] <= CHARACTER_9)
                leftMostNumber--;
            
            bool should_parsing_width = true;       // flag for all zero test
            
            if(leftMostNumber[0] == CHARACTER_0) {      // enter flag zero parsing
                const UTF8Char *leftMostZero = leftMostNumber;      DEBUG_ASSERT(leftMostZero > format);
                while(leftMostZero + 1 <= current && leftMostZero[1] == CHARACTER_0)
                    leftMostZero++;
                if(leftMostZero == current) should_parsing_width = false;
                else leftMostNumber = leftMostZero + 1;
            }
            
            // if no need for parsing width, it must be all zero sequence which left for flag parsing
            
            if(should_parsing_width) {
                // when enter width parsing
                // there is need to set next current point for flag parsing
                DEBUG_ASSERT(leftMostNumber <= current);
                size_t width = 0;
                
                const UTF8Char *moveFromLeft = leftMostNumber;
                
                while(moveFromLeft <= current) {
                    DEBUG_ASSERT(moveFromLeft[0] >= CHARACTER_0 && moveFromLeft[0] <= CHARACTER_9);
#ifdef DEBUG
                    // overflow test for width field
                    size_t overflow_test;
                    if(__builtin_mul_overflow(width, 10, &overflow_test)) DEBUG_POINT;
                    if(__builtin_add_overflow(width, moveFromLeft[0] - CHARACTER_0, &overflow_test)) DEBUG_POINT;
#endif
// [NOTE] [optimization] limited width support
                    width *= 10;
                    width += moveFromLeft[0] - CHARACTER_0;
                    moveFromLeft++;
                }
                
                interprate->width_type = printf_interprate_adjustment_fix_number;
                interprate->width = width;
                
                current = leftMostNumber - 1; // current set to the next parsing point
            }
            parsingPoint[0] = current;  // updated parsingPoint
            return true;
        }
        return true;        // no width for parsing
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_interprate_flag_parsing
    @abstract parsing interprate flag field
    @discussion only affect [flag] field
                if no flag field found return true and not affect anything
    @param interprate if parsing success, it is used to store result
    @param format point to the escape character (%)
    @param parsingPoint point to the precision parsing point, if return true, this is updated to the next parsing point
    @return true if parsing success, even no flag field; false if encounter some illegal character
*/
static bool printf_interprate_flag_parsing(printf_interprate_ref _Nonnull interprate,
                                           const UTF8Char * _Nonnull format,
                                           const UTF8Char * _Nonnull * _Nonnull parsingPoint) {
    if(interprate != NULL && format != NULL && parsingPoint != NULL && parsingPoint[0] != NULL && format <= parsingPoint[0]) {
        const UTF8Char * _Nonnull current = parsingPoint[0];
        printf_interprate_flag flag = printf_interprate_flag_none;
        while(current > format) {
            // legal flags are - + (space) # 0
            if(current[0] == CHARACTER_minus) flag ^= printf_interprate_flag_left_justify;
            else if(current[0] == CHARACTER_plus) flag ^= printf_interprate_flag_force_sign;
            else if(current[0] == CHARACTER_space) flag ^= printf_interprate_flag_sign_space;
            else if(current[0] == CHARACTER_hash) flag ^= printf_interprate_flag_complex;
            else if(current[0] == CHARACTER_0) flag ^= printf_interprate_flag_zero_padding;
            ELSE_DEVELOP_BREAKPOINT_RETURN(false);
            // [BREAKPOINT] unexpected character
            current--;
        }
        interprate->flag = flag;
        parsingPoint[0] = current;  // updated parsingPoint
        return true;
    } DEBUG_ELSE
    return false;
}

/*!
    @function printf_interprate_parser
    @param core query for format infomation, updated to accept next output match if return true
           means its format is point to the next parsing point
           only format consumed, while va_list untouched.
    @param interprate used to store the interprate value
    @abstract parsing an interprate structure used later
*/
static bool printf_interprate_parser(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    // [0 - 1] check input valid
    if(core != NULL && interprate != NULL && printf_core_validate(core)) {
        const UTF8Char *format = core->format;
        
        // [0 - 2] check input valid
        if(format != NULL && format[0] == CHARACTER_percentage) {
            const UTF8Char *specifier; printf_specifier_type specifier_type;
            // [1] find the specifier and its type
            if((specifier = printf_interprate_search_specifier_inline(format, &specifier_type)) != NULL) {
                DEBUG_ASSERT(specifier > format && specifier_type != printf_specifier_invalid);
                const UTF8Char *current = specifier;
                
                // [2] intialize interprate object according to specifier_type
                if(!printf_interprate_initialize(interprate, specifier_type)) return false;
                DEBUG_ASSERT(current >= format); if(format == current) goto JUMP_QUICK_COMPLETE;
                
                // [3] length parsing
                if(!printf_interprate_length_parsing(interprate, format, &current)) return false;
                DEBUG_ASSERT(current >= format); if(format == current) goto JUMP_QUICK_COMPLETE;
                
                // [4] precision parsing
                if(!printf_interprate_precision_parsing(interprate, format, &current)) return false;
                DEBUG_ASSERT(current >= format); if(format == current) goto JUMP_QUICK_COMPLETE;
                
                // [5] width parsing
                if(!printf_interprate_width_parsing(interprate, format, &current)) return false;
                DEBUG_ASSERT(current >= format); if(format == current) goto JUMP_QUICK_COMPLETE;
                
                // [6] flag parsing
                if(!printf_interprate_flag_parsing(interprate, format, &current)) return false;
                
// [NOTE] [optimization] validate, breakpoint at debug, fixed at runtime
                
                JUMP_QUICK_COMPLETE:
                if(!printf_interprate_fixup_inline(interprate)) DEBUG_RETURN(false);
                interprate->start = format;
                interprate->end = specifier;
                core->format = specifier + 1;
                return true;
            } DEBUG_ELSE
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

#pragma mark - supporting

/*!
    @function printf_strchr_inline
    @abstract just a strchr with default implementation
*/
static inline const UTF8Char * _Nullable printf_strchr_inline(const UTF8Char * _Nonnull str, const UTF8Char ch) {
    if(str != NULL) {
        LOOP {
            if(str[0] == ch) return str;
            if(str[0] == CHARACTER_null) return NULL;
            str++;
        }
    } DEBUG_ELSE
    return NULL;
}

/*!
    @function printf_strlen_inline
    @abstract just a strlen with default implementation
*/
static inline size_t printf_strlen_inline(const UTF8Char * _Nonnull str) {
    if(str == NULL) DEBUG_RETURN(0);
    size_t index;
    for(index = 0; str[index] != CHARACTER_null; index++) continue;
    return index;
}

/*!
    @function printf_strlen_minimum_inline
    @return the min[ strlen(str), minimum_length ]
*/
static inline size_t printf_strlen_minimum_inline(const UTF8Char * _Nonnull str, size_t minimum_length) {
    if(str == NULL) { DEBUG_POINT; return 0; }
    size_t index;
    for(index = 0; index < minimum_length && str[index] != CHARACTER_null; index++) continue;
    return index;
}

/*!
    @function printf_core_output_raw_string
    @abstract when str == NULL or length == zero, nothing is ouputed
    @discussion this will not update core->format location info
    @param length  length and strlen the minimun decide which to ouput, also need enough storage
*/
static void printf_core_output_raw_string(printf_core_ref _Nonnull core, const UTF8Char * _Nullable str, size_t length) {
    if(core != NULL) {
        if(str != NULL && length > 0) {
            size_t actual_string_length = printf_strlen_minimum_inline(str, length);
            if(core->type == printf_type_FILE) {
                if(core->output.file != NULL) {
                    fwrite(str, actual_string_length * sizeof(UTF8Char), 1, core->output.file);
                } DEBUG_ELSE
            }
            else if(core->type == printf_type_string) {
                if(core->output.string.store != NULL && core->output.string.index + 1 < core->output.string.count) {
                    DEBUG_ASSERT(core->output.string.store[core->output.string.index] == CHARACTER_null);
                    size_t max_store_amount = core->output.string.count - core->output.string.index - 1;
                    size_t store_amount = max_store_amount;
                    if(actual_string_length < store_amount) store_amount = actual_string_length;
                    
                    for(size_t increase = 0; increase < store_amount; increase++)
                        core->output.string.store[core->output.string.index + increase] = str[increase];
                    
                    core->output.string.store[core->output.string.index + store_amount] = CHARACTER_null;
                    core->output.string.index += store_amount;
                }
            } DEBUG_ELSE
            core->actual_need += actual_string_length;
        }
    } DEBUG_ELSE
}

static void printf_core_output_character(printf_core_ref _Nonnull core, UTF8Char character) {
    if(core != NULL) {
        if(core->type == printf_type_FILE) {
            if(core->output.file != NULL) {
                fputc(character, core->output.file);
            } DEBUG_ELSE
        }
        else if(core->type == printf_type_string) {
            if(core->output.string.store != NULL && core->output.string.index + 1 < core->output.string.count) {
                DEBUG_ASSERT(core->output.string.store[core->output.string.index] == CHARACTER_null);
                core->output.string.store[core->output.string.index++] = character;
                core->output.string.store[core->output.string.index] = CHARACTER_null;
            }
        } DEBUG_ELSE
        core->actual_need += 1;
    } DEBUG_ELSE
}

#pragma mark - flag / width helper

typedef enum printf_sign {
    printf_sign_none,
    printf_sign_space,
    printf_sign_plus,
    printf_sign_minus
} printf_sign;

typedef enum printf_complex_prefix {
    printf_complex_prefix_none,
    printf_complex_prefix_0,
    printf_complex_prefix_0x,
    printf_complex_prefix_0X
} printf_complex_prefix;

/*!
    @typedef printf_pure_func_t
    @abstract used by printf_core_output_flag_width_help for common flag and width output
 */
typedef bool (* printf_pure_func_t)(printf_core_ref _Nonnull core,
                                    printf_interprate_ref _Nonnull interprate,
                                    size_t pure_width,
                                    void * _Nonnull input);

/*!
    @function printf_core_output_flag_width_help
    @param interprate should be already parsed for width and precsion input
    @param pure_width output length if only consider precision and may consider flag #
           [ consider flag # ] only when it represent whether has decimal point
           this is not storage count, but actually character count
    @param complex_prefix would be prefixed before flag zero insert if complex flag meet
    @param input is passed to function when called
    @warning all flag in interprate is considered, YOU SHOULD DISABLE IF NOT NEED FOR THAT SPECIFIER
 */
static bool printf_core_output_flag_width_help(printf_core_ref _Nonnull core,
                                               printf_interprate_ref _Nonnull interprate,
                                               size_t pure_width,
                                               printf_complex_prefix complex_prefix,
                                               bool is_negative,
                                               printf_pure_func_t _Nonnull pure_function,
                                               void * _Nonnull input) {
    if(core != NULL && interprate != NULL && pure_function != NULL && input != NULL) {

        size_t least_width = pure_width;
        
        printf_sign sign = printf_sign_none;
        if(is_negative) sign = printf_sign_minus;
        else if(interprate->flag & printf_interprate_flag_force_sign) sign = printf_sign_plus;
        else if(interprate->flag & printf_interprate_flag_sign_space) sign = printf_sign_space;
        
        if(sign != printf_sign_none) least_width += 1;
        
        bool using_complex_prefix = false;
        if(interprate->flag & printf_interprate_flag_complex && complex_prefix != printf_complex_prefix_none) {
            if(complex_prefix == printf_complex_prefix_0) least_width += 1;
            else if(complex_prefix == printf_complex_prefix_0x) least_width += 2;
            else if(complex_prefix == printf_complex_prefix_0X) least_width += 2;
            ELSE_DEBUG_RETURN(false);
            using_complex_prefix = true;
        }
        
        size_t insert_amount = 0;
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        if(least_width < interprate->width) insert_amount = interprate->width - least_width;
        
        if(insert_amount == 0) {
            if(sign == printf_sign_minus) printf_core_output_character(core, CHARACTER_minus);
            else if(sign == printf_sign_plus) printf_core_output_character(core, CHARACTER_plus);
            else if(sign == printf_sign_space) printf_core_output_character(core, CHARACTER_space);
            if(using_complex_prefix)
                switch (complex_prefix) {
                    case printf_complex_prefix_0:
                        printf_core_output_character(core, CHARACTER_0);
                        break;
                    case printf_complex_prefix_0x:
                        printf_core_output_character(core, CHARACTER_0);
                        printf_core_output_character(core, CHARACTER_x);
                        break;
                    case printf_complex_prefix_0X:
                        printf_core_output_character(core, CHARACTER_0);
                        printf_core_output_character(core, CHARACTER_X);
                        break;
                    case printf_complex_prefix_none: default:
                        DEBUG_RETURN(false);
                }
            if(!pure_function(core, interprate, pure_width, input))
                DEBUG_RETURN(false);
        } else {
            bool is_insert_zero = false;
            if(interprate->flag & printf_interprate_flag_zero_padding) is_insert_zero = true;
            if(is_insert_zero) {
                if(sign == printf_sign_minus) printf_core_output_character(core, CHARACTER_minus);
                else if(sign == printf_sign_plus) printf_core_output_character(core, CHARACTER_plus);
                else if(sign == printf_sign_space) printf_core_output_character(core, CHARACTER_space);
                if(using_complex_prefix)
                    switch (complex_prefix) {
                        case printf_complex_prefix_0:
                            printf_core_output_character(core, CHARACTER_0);
                            break;
                        case printf_complex_prefix_0x:
                            printf_core_output_character(core, CHARACTER_0);
                            printf_core_output_character(core, CHARACTER_x);
                            break;
                        case printf_complex_prefix_0X:
                            printf_core_output_character(core, CHARACTER_0);
                            printf_core_output_character(core, CHARACTER_X);
                            break;
                        case printf_complex_prefix_none: default:
                            DEBUG_RETURN(false);
                    }
                for(size_t index = 0; index < insert_amount; index++)
                    printf_core_output_character(core, CHARACTER_0);
                if(!pure_function(core, interprate, pure_width, input)) DEBUG_RETURN(false);
            }
            else if(interprate->flag & printf_interprate_flag_left_justify) {
                if(sign == printf_sign_minus) printf_core_output_character(core, CHARACTER_minus);
                else if(sign == printf_sign_plus) printf_core_output_character(core, CHARACTER_plus);
                else if(sign == printf_sign_space) printf_core_output_character(core, CHARACTER_space);
                if(using_complex_prefix)
                    switch (complex_prefix) {
                        case printf_complex_prefix_0:
                            printf_core_output_character(core, CHARACTER_0);
                            break;
                        case printf_complex_prefix_0x:
                            printf_core_output_character(core, CHARACTER_0);
                            printf_core_output_character(core, CHARACTER_x);
                            break;
                        case printf_complex_prefix_0X:
                            printf_core_output_character(core, CHARACTER_0);
                            printf_core_output_character(core, CHARACTER_X);
                            break;
                        case printf_complex_prefix_none: default:
                            DEBUG_RETURN(false);
                    }
                if(!pure_function(core, interprate, pure_width, input)) DEBUG_RETURN(false);
                for(size_t index = 0; index < insert_amount; index++)
                    printf_core_output_character(core, CHARACTER_space);
            }
            else {
                for(size_t index = 0; index < insert_amount; index++)
                    printf_core_output_character(core, CHARACTER_space);
                if(sign == printf_sign_minus) printf_core_output_character(core, CHARACTER_minus);
                else if(sign == printf_sign_plus) printf_core_output_character(core, CHARACTER_plus);
                else if(sign == printf_sign_space) printf_core_output_character(core, CHARACTER_space);
                if(using_complex_prefix)
                    switch (complex_prefix) {
                        case printf_complex_prefix_0:
                            printf_core_output_character(core, CHARACTER_0);
                            break;
                        case printf_complex_prefix_0x:
                            printf_core_output_character(core, CHARACTER_0);
                            printf_core_output_character(core, CHARACTER_x);
                            break;
                        case printf_complex_prefix_0X:
                            printf_core_output_character(core, CHARACTER_0);
                            printf_core_output_character(core, CHARACTER_X);
                            break;
                        case printf_complex_prefix_none: default:
                            DEBUG_RETURN(false);
                    }
                if(!pure_function(core, interprate, pure_width, input)) DEBUG_RETURN(false);
            }
        }
        return true;
    } DEBUG_ELSE
    return false;
}

#pragma mark - IEE 754 floating point info

// this assume IEEE 754 standard Binary interchange format (binary 32/binary 64)
// [Documentation] IEEE Std 754 - 2008 (page 13)

// float                (binary 32)
// double & long double (binary 64)

#pragma mark supported binary exchange format

typedef enum IEEE754_type {
    IEEE754_type_32,
    IEEE754_type_64
} IEEE754_type;

typedef enum fp_classification {
    fp_classification_zero,
    fp_classification_nan,
    fp_classification_infinite,
    fp_classification_normal,
    fp_classification_subnormal
} fp_classification;

#ifdef DEBUG
CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_UNUSED_FUNCTION
static void __HIDDEN__check_floating_point_alignment(void) {  // DO NOT DELETE
    // sizeof check mark
//    COMILE_ASSERT(sizeof(long double) == 8);
    COMILE_ASSERT(sizeof(double) == 8);
    COMILE_ASSERT(sizeof(float) == 4);
    
    // mantissa digit [including implicit one]
    COMILE_ASSERT(FLT_MANT_DIG == 24);
    COMILE_ASSERT(DBL_MANT_DIG == 53);
//    COMILE_ASSERT(LDBL_MANT_DIG == 53);
    
    // binary exchange format
    COMILE_ASSERT(FLT_RADIX == 2);
}
CLANG_DIAGNOSTIC_POP
#endif

#pragma mark component macro

#define FP32_SIGN_BIT           1
#define FP32_EXPONENT_BIT       8
#define FP32_SIGNIFICAND_BIT    23

#define FP64_SIGN_BIT           1
#define FP64_EXPONENT_BIT       11
#define FP64_SIGNIFICAND_BIT    52

// this split significand into two unsigned int bit field
#define FP64_SIGNIFICAND_0_BIT  32
#define FP64_SIGNIFICAND_1_BIT  20

#pragma mark range macro

#define FP32_EXPONENT_RAW_MAX   0xFF
#define FP64_EXPONENT_RAW_MAX   0x7FF

#define FP32_EXPONENT_BASE      0x7F
#define FP64_EXPONENT_BASE      0x3FF

// max exponent bit for all supported floating point type
#define FP_EXPONENT_BIT_MAX     FP64_EXPONENT_BIT

#pragma mark nan string array

static UTF8Char __IEE_754_nan_string_uppercase__[] = {CHARACTER_N, CHARACTER_A, CHARACTER_N, CHARACTER_null};
static UTF8Char __IEE_754_nan_string_lowercase__[] = {CHARACTER_n, CHARACTER_a, CHARACTER_n, CHARACTER_null};

#define NAN_STRING_UPPERCASE __IEE_754_nan_string_uppercase__
#define NAN_STRING_LOWERCASE __IEE_754_nan_string_lowercase__
#define NAN_STRING_LENGTH 3

#pragma mark inf string array

static UTF8Char __IEE_754_inf_string_uppercase__[] = {CHARACTER_I, CHARACTER_N, CHARACTER_F, CHARACTER_null};
static UTF8Char __IEE_754_inf_string_lowercase__[] = {CHARACTER_i, CHARACTER_n, CHARACTER_f, CHARACTER_null};

#define INF_STRING_UPPERCASE __IEE_754_inf_string_uppercase__
#define INF_STRING_LOWERCASE __IEE_754_inf_string_lowercase__
#define INF_STRING_LENGTH 3

#pragma mark general floating point info

typedef float  fp32_t;      // 32 bit
typedef double fp64_t;      // 64 bit

typedef struct fp_info {
    IEEE754_type type;
    union {
        fp32_t fp32;
        fp64_t fp64;
    } value;
} fp_info;

typedef fp_info *fp_info_ref;

#pragma mark floating point binary 64 component

typedef union fp64_info {
    fp64_t value;
    struct {
#if   defined PRINTF_LITTLE_ENDIAN
        unsigned int significand_0 : FP64_SIGNIFICAND_0_BIT;
        unsigned int significand_1 : FP64_SIGNIFICAND_1_BIT;
        unsigned int exponent:       FP64_EXPONENT_BIT;
        unsigned int sign:           FP64_SIGN_BIT;
#elif defined PRINTF_BIG_ENDIAN
        unsigned int sign:           FP64_SIGN_BIT;
        unsigned int exponent:       FP64_EXPONENT_BIT;
        unsigned int significand_1 : FP64_SIGNIFICAND_1_BIT;
        unsigned int significand_0 : FP64_SIGNIFICAND_0_BIT;
#endif
    } component GNU_ATTRIBUTE_PACKED;
} fp64_info;

#pragma mark floating point binary 32 component

typedef union fp32_info {
    fp32_t value;
    struct {
#if   defined PRINTF_LITTLE_ENDIAN
        unsigned int significand : FP32_SIGNIFICAND_BIT;
        unsigned int exponent:     FP32_EXPONENT_BIT;
        unsigned int sign:         FP32_SIGN_BIT;
#elif defined PRINTF_BIG_ENDIAN
        unsigned int sign:         FP32_SIGN_BIT;
        unsigned int exponent:     FP32_EXPONENT_BIT;
        unsigned int significand : FP32_SIGNIFICAND_BIT;
#endif
    } component GNU_ATTRIBUTE_PACKED;
} fp32_info;

#pragma mark general floating info query function

/*!
    @function floating_point_info_validate_inline
    @return true if this is a valid fp_info object, false otherwise
 */
static inline bool floating_point_info_validate_inline(fp_info_ref _Nonnull info) {
    if(info != NULL) {
        if(info->type == IEEE754_type_32 || info->type == IEEE754_type_64) {
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_UNUSED_FUNCTION

/*!
    @function floating_point_intialize_32bit_inline
    @discussion used to intialized an fp_info object for 32 bit
    @return true if initialize complete, false if error occured
*/
static inline bool floating_point_intialize_32bit_inline(fp_info_ref _Nonnull info, fp32_t fp32) {
    if(info != NULL) {
        info->type = IEEE754_type_32;
        info->value.fp32 = fp32;
        return true;
    } DEBUG_ELSE
    return false;
}

/*!
    @function floating_point_intialize_64bit_inline
    @discussion used to intialized an fp_info object for 64 bit
    @return true if initialize complete, false if error occured
*/
static inline bool floating_point_intialize_64bit_inline(fp_info_ref _Nonnull info, fp64_t fp64) {
    if(info != NULL) {
        info->type = IEEE754_type_64;
        info->value.fp64 = fp64;
        return true;
    } DEBUG_ELSE
    return false;
}

CLANG_DIAGNOSTIC_POP

/*!
    @function floating_point_info_query_sign_bit_inline
    @discussion used to query sign bit of floating point value
    @return false if possitive, true if negative
*/
static inline bool floating_point_info_query_sign_bit_inline(fp_info_ref _Nonnull info) {
    if(info != NULL && floating_point_info_validate_inline(info)) {
        switch (info->type) {
            case IEEE754_type_32:
                return ((uint8_t *)(&info->value.fp32))[3] & 0x80;
            case IEEE754_type_64:
                return ((uint8_t *)(&info->value.fp32))[7] & 0x80;
            default: DEBUG_POINT break;
        }
    } DEBUG_ELSE
    return false;
}

static inline unsigned int floating_point_info_query_raw_exponent_inline(fp_info_ref _Nonnull info) {
    COMILE_ASSERT(sizeof(unsigned int) * CHAR_BIT > FP_EXPONENT_BIT_MAX);
    if(info != NULL && floating_point_info_validate_inline(info)) {
        switch (info->type) {
            case IEEE754_type_32: {
                fp32_info query;
                query.value = info->value.fp32;
                return query.component.exponent;
            }
            case IEEE754_type_64: {
                fp64_info query;
                query.value = info->value.fp64;
                return query.component.exponent;
            }
            default: DEBUG_POINT break;
        }
    } DEBUG_ELSE
    return 0;       // not much useful
}

static inline unsigned int floating_point_query_exponent_base(fp_info_ref _Nonnull info) {
    COMILE_ASSERT(sizeof(unsigned int) * CHAR_BIT > FP_EXPONENT_BIT_MAX);
    if(info != NULL && floating_point_info_validate_inline(info)) {
        switch (info->type) {
            case IEEE754_type_32:
                return FP32_EXPONENT_BASE;
            case IEEE754_type_64: {
                return FP64_EXPONENT_BASE;
            }
            default: DEBUG_POINT break;
        }
    }
    return 0;
}

static fp_classification floating_point_info_classification(fp_info_ref _Nonnull info) {
    if(info != NULL && floating_point_info_validate_inline(info)) {
        switch (info->type) {
            case IEEE754_type_32: {
                unsigned int raw_exponent = floating_point_info_query_raw_exponent_inline(info);
                DEBUG_ASSERT(raw_exponent <= FP32_EXPONENT_RAW_MAX);
                fp32_info query;
                query.value = info->value.fp32;
                if(raw_exponent == FP32_EXPONENT_RAW_MAX) {
                    if(query.component.significand == 0) return fp_classification_infinite;
                    else return fp_classification_nan;
                }
                else if(raw_exponent == 0) {
                    if(query.component.significand == 0) return fp_classification_zero;
                    else return fp_classification_subnormal;
                }
                else return fp_classification_normal;
            }
            case IEEE754_type_64: {
                unsigned int raw_exponent = floating_point_info_query_raw_exponent_inline(info);
                DEBUG_ASSERT(raw_exponent <= FP64_EXPONENT_RAW_MAX);
                fp64_info query;
                query.value = info->value.fp64;
                if(raw_exponent == FP64_EXPONENT_RAW_MAX) {
                    if(query.component.significand_0 == 0 && query.component.significand_1 == 0) return fp_classification_infinite;
                    else return fp_classification_nan;
                }
                else if(raw_exponent == 0) {
                    if(query.component.significand_0 == 0 &&
                       query.component.significand_1 == 0)
                        return fp_classification_zero;
                    else return fp_classification_subnormal;
                }
                else return fp_classification_normal;
            }
            default: DEBUG_POINT break;
        }
    } DEBUG_ELSE
    return fp_classification_normal;
}

/*!
    @function floating_point_info_query_significand_bit
    @param significand used to store significand bit not count for
           implicit 1 for normal form and nor the 0 for abnormal form
           if not null, at most "size" amount of data is stored
    @param size maximun size for significand
    @return actual needed "size" amount, index 0 point to the highest bit [ 2^(-1) mostly ]
    @discussion if significand is not null, min [ returned, size ] data is stored
 */
static size_t floating_point_info_query_significand_bit(fp_info_ref _Nonnull info, bool * _Nullable significand, size_t size) {
    if(info != NULL && floating_point_info_validate_inline(info)) {
        switch (info->type) {
            case IEEE754_type_32: {
                if(significand != NULL) {
                    fp32_info query;
                    query.value = info->value.fp32;
                    size_t count = FP32_SIGNIFICAND_BIT;
                    if(size < count) count = size;
                    for(size_t index = 0; index < count; index++)
                        significand[index] = query.component.significand & (1 << (FP32_SIGNIFICAND_BIT - 1 - index));
                }
                return FP32_SIGNIFICAND_BIT;
            }
            case IEEE754_type_64: {
                if(significand != NULL) {
                    fp64_info query;
                    query.value = info->value.fp64;
                    size_t count = FP64_SIGNIFICAND_BIT;
                    if(size < count) count = size;
                    for(size_t index = 0; index < count; index++)
                        if(index >= FP64_SIGNIFICAND_1_BIT)
                            significand[index] = query.component.significand_0 & (1 << (FP64_SIGNIFICAND_0_BIT - 1 - (index - FP64_SIGNIFICAND_1_BIT)));
                        else
                            significand[index] = query.component.significand_1 & (1 << (FP64_SIGNIFICAND_1_BIT - 1 - index));
                }
                return FP64_SIGNIFICAND_BIT;
            }
            default: DEBUG_POINT break;
        }
    } DEBUG_ELSE
    return 0;
}

#pragma mark debug

#ifdef DEBUG

void floating_point_info_debug(double value) {
    fp64_info query = {.value = value};
    fprintf(stdout, "[double info] %.20f\n", value);
    fprintf(stdout, " sign bit: %s\n", query.component.sign ? "true" : "false");
    fprintf(stdout, " exponent: %u\n", query.component.exponent);
    fprintf(stdout, "signifand: ");
    unsigned int significand_0 = query.component.significand_0;
    unsigned int significand_1 = query.component.significand_1;
    for(size_t index = 0; index < FP64_SIGNIFICAND_1_BIT; index++) {
        unsigned int offset = 1u << (FP64_SIGNIFICAND_1_BIT - index - 1);
        if(offset & significand_1) putc('1', stdout);
        else putc('0', stdout);
    }
    for(size_t index = 0; index < FP64_SIGNIFICAND_0_BIT; index++) {
        unsigned int offset = 1u << (FP64_SIGNIFICAND_0_BIT - index - 1);
        if(offset & significand_0) putc('1', stdout);
        else putc('0', stdout);
    }
    putc('\n', stdout);
}

#endif

#pragma mark - printf output specifier

#pragma mark integer type [iduoxX]

static bool printf_output_specifier_integer_iduoxX(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    if(core != NULL && interprate != NULL && printf_specifier_type_is_integer_inline(interprate->type)) {
        
        COMILE_ASSERT(sizeof(intmax_t) >= sizeof(long long));
        COMILE_ASSERT(sizeof(intmax_t) >= sizeof(size_t));
        COMILE_ASSERT(sizeof(intmax_t) >= sizeof(ptrdiff_t));
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_after_input);
        if(interprate->width_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
            interprate->width_type = printf_interprate_adjustment_after_input;
        }
        
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_after_input);
        if(interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
            interprate->precision_type = printf_interprate_adjustment_after_input;
        }
        
        bool is_signed_integer_type = interprate->type == printf_specifier_d_or_i;
        
        bool negative = false;
        uintmax_t value;
        
        switch (interprate->length_type) {
            // value will be promoted to int
            case printf_interprate_length_hh:
            case printf_interprate_length_h:
            case printf_interprate_length_none:
                if(is_signed_integer_type) {
                    int query = va_arg(core->args, int);
                    if(query < 0) {
                        negative = true;
                        query = - query;
                    }
                    value = query;
                } else value = va_arg(core->args, unsigned int);
                break;
            
            case printf_interprate_length_l:
                if(is_signed_integer_type) {
                    long query = va_arg(core->args, long);
                    if(query < 0) {
                        negative = true;
                        query = - query;
                    }
                    value = query;
                } else value = va_arg(core->args, unsigned long);
                break;
                
            case printf_interprate_length_ll:
                if(is_signed_integer_type) {
                    long long query = va_arg(core->args, long long);
                    if(query < 0) {
                        negative = true;
                        query = - query;
                    }
                    value = query;
                } else value = va_arg(core->args, unsigned long long);
                break;
                
            case printf_interprate_length_j:
                if(is_signed_integer_type) {
                    intmax_t query = va_arg(core->args, intmax_t);
                    if(query < 0) {
                        negative = true;
                        query = - query;
                    }
                    value = query;
                } else value = va_arg(core->args, uintmax_t);
                break;
                
            case printf_interprate_length_z:
                value = va_arg(core->args, size_t);
                break;
                
            case printf_interprate_length_t:
                value = va_arg(core->args, ptrdiff_t);
                break;
            
            /* case error handle */
            case printf_interprate_length_L: default:
                DEBUG_RETURN(false);
        }
        
        // consider zero as one digit ouput
        // not fixed up for precison zero
        unsigned int digits_amount = 1;
        
        uintmax_t base = 10u;
        if(interprate->type == printf_specifier_d_or_i ||
           interprate->type == printf_specifier_u)
            base = 10u;
        else if(interprate->type == printf_specifier_o)
            base = 8u;
        else if(interprate->type == printf_specifier_x ||
                interprate->type == printf_specifier_X)
            base = 16u;
        ELSE_DEBUG_RETURN(false);
        
        uintmax_t temp = value;
        while((temp = temp / base) > 0) digits_amount++;
        
        // not much meaningful
        COMILE_ASSERT(sizeof(intmax_t) <= PRINTF_INTEGER_MAX_DIGITS_AMOUNT);
        
        // most impossible condition
        if(digits_amount > PRINTF_INTEGER_MAX_DIGITS_AMOUNT) DEBUG_RETURN(false);
        
        // most impossible condition
        if(digits_amount > PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
        
        uint8_t * eachDigits = printf_stack_alloc(digits_amount);
        
        size_t index = 0; uintmax_t temp2 = value;
        do eachDigits[index++] = temp2 % base;
        while((temp2 = temp2 / base) > 0);
        
        // unable to parsing complex flag for integer type d or i
        DEBUG_ASSERT((interprate->type != printf_specifier_d_or_i) || !(interprate->flag & printf_interprate_flag_complex));
        
        // sign mark is only valid for integer type d or i
        printf_sign sign = printf_sign_none;
        if(interprate->type == printf_specifier_d_or_i) {
            if(negative) sign = printf_sign_minus;
            else if(interprate->flag & printf_interprate_flag_force_sign) sign = printf_sign_plus;
            else if(interprate->flag & printf_interprate_flag_sign_space) sign = printf_sign_space;
        }
        
        DEBUG_ASSERT(digits_amount >= 1);
        
        size_t precision_padding_zero = 0;
        bool zero_no_digit = false;
        
        // fetch precision if needs
        if(interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            interprate->precision = va_arg(core->args, int);
// [NOTE] [optimization] limited precision in expected value
            interprate->precision_type = printf_interprate_adjustment_after_input;
        } else if(interprate->precision_type == printf_interprate_adjustment_unspecified) {
            DEBUG_ASSERT(interprate->precision == 1);   // should be initialized
            interprate->precision = 1;
        } else if(interprate->precision_type == printf_interprate_adjustment_after_input) {
            DEBUG_RETURN(false);
        } else if(interprate->precision_type == printf_interprate_adjustment_fix_number) {
// [NOTE] [optimization] limited width in expected value
        } ELSE_DEBUG_RETURN(false);
        
        if(interprate->precision == 0 && value == 0)
            zero_no_digit = true;
        else if(interprate->precision > digits_amount)
            precision_padding_zero = interprate->precision - digits_amount;
        
        // [least digits amount] including precision + value, not count for flag(0) and sign_mark
        size_t least_digits_amount;
        if(zero_no_digit) least_digits_amount = 0;
        else least_digits_amount = digits_amount + precision_padding_zero;
        
        // [how to output]
        // if(zero_no_digit) output nothing
        // else output precsion_padding_zero and digits_amount
        
        printf_complex_prefix prefix = printf_complex_prefix_none;
        size_t prefix_length = 0;
        if(interprate->type == printf_specifier_o) {
            if(interprate->flag & printf_interprate_flag_complex) {
                prefix = printf_complex_prefix_0;
                prefix_length = 1;
            }
        }
        else if(interprate->type == printf_specifier_x) {
            if(interprate->flag & printf_interprate_flag_complex) {
                prefix = printf_complex_prefix_0x;
                prefix_length = 2;
            }
        }
        else if(interprate->type == printf_specifier_X) {
            if(interprate->flag & printf_interprate_flag_complex) {
                prefix = printf_complex_prefix_0X;
                prefix_length = 2;
            }
        }
        
#ifdef PRINTF_DISBALE_FLAG_COMPLEX_FOR_INTEGER_VALUE_ZERO_WITH_PRECISION_ZERO
        if(zero_no_digit) {
            prefix = printf_complex_prefix_none;
            prefix_length = 0;      // not quite aligned to GNUC odd implementation
        }
#endif
        
        // unable to parsing complex flag for specifier type u or d or i
        DEBUG_ASSERT((interprate->type != printf_specifier_u && interprate->type != printf_specifier_d_or_i) || !(interprate->flag & printf_interprate_flag_complex));
        
        // not much useful
        DEBUG_ASSERT(prefix == printf_complex_prefix_none || sign == printf_sign_none);
        
        // [least character amount] include precison digits and sign mark
        size_t least_character_amount = least_digits_amount;
        if(sign != printf_sign_none) least_character_amount += 1;
        if(prefix != printf_complex_prefix_none) least_character_amount += prefix_length;
        
        // fetch width if needs
        if(interprate->width_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
// [NOTE] [optimization] limited width in expected value
            interprate->width_type = printf_interprate_adjustment_after_input;
        } else if(interprate->width_type == printf_interprate_adjustment_unspecified) {
            DEBUG_ASSERT(interprate->width == 0);   // should be initialized
            interprate->width = 0;
        } else if(interprate->width_type == printf_interprate_adjustment_after_input) {
            DEBUG_RETURN(false);
        } else if(interprate->width_type == printf_interprate_adjustment_fix_number) {
// [NOTE] [optimization] limited width in expected value
        } ELSE_DEBUG_RETURN(false);
        
        bool need_consider_width;
        if(interprate->width > least_character_amount) need_consider_width = true;
        else need_consider_width = false;
        
        size_t full_output_character_amount;
        if(need_consider_width) full_output_character_amount = interprate->width;
        else full_output_character_amount = least_character_amount;
        
        size_t width_addition_amount = 0;
        if(need_consider_width) width_addition_amount = interprate->width - least_character_amount;
        
        // used for flag 0
        bool width_using_zero_padding = interprate->flag & printf_interprate_flag_zero_padding;
        
#ifdef PRINTF_DISBALE_FLAG_ZERO_FOR_INTEGER_SPECIFIER_WITH_PRECISION_SPECIFIED
        if(width_using_zero_padding) {
            if(interprate->precision_type != printf_interprate_adjustment_unspecified)
                width_using_zero_padding = false;   // this aligned to GNUC implementation
        }
#endif
        
        if(core->type == printf_type_FILE) {
            if(core->output.file != NULL) {
                if(need_consider_width) {
                    DEBUG_ASSERT(width_addition_amount > 0);
                    if(width_using_zero_padding) {
                        // sign output
                        if(sign == printf_sign_plus) putc(CHARACTER_plus, core->output.file);
                        else if(sign == printf_sign_minus) putc(CHARACTER_minus, core->output.file);
                        else if(sign == printf_sign_space) putc(CHARACTER_space, core->output.file);
                        
                        // prefix output
                        if(prefix == printf_complex_prefix_0) putc(CHARACTER_0, core->output.file);
                        else if(prefix == printf_complex_prefix_0x) {
                            putc(CHARACTER_0, core->output.file);
                            putc(CHARACTER_x, core->output.file);
                        }
                        else if(prefix == printf_complex_prefix_0X) {
                            putc(CHARACTER_0, core->output.file);
                            putc(CHARACTER_X, core->output.file);
                        }
                        
                        // width zero padding output
                        for(size_t index = 0; index < width_addition_amount; index++)
                            putc(CHARACTER_0, core->output.file);
                        
                        if(!zero_no_digit) {
                            for(size_t index = 0; index < precision_padding_zero; index++)
                                putc(CHARACTER_0, core->output.file);
                            if(interprate->type == printf_specifier_d_or_i || interprate->type == printf_specifier_u) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base10_character));
                                    putc(base10_character[eachDigits[count - 1]], core->output.file);
                                }
                            }
                            else if(interprate->type == printf_specifier_o) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base8_character));
                                    putc(base8_character[eachDigits[count - 1]], core->output.file);
                                }
                            }
                            else if(interprate->type == printf_specifier_x) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_lowercase));
                                    putc(base16_character_lowercase[eachDigits[count - 1]], core->output.file);
                                }
                            }
                            else if(interprate->type == printf_specifier_X) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_uppercase));
                                    putc(base16_character_uppercase[eachDigits[count - 1]], core->output.file);
                                }
                            } ELSE_DEBUG_RETURN(false);
                        }
                    }
                    else {
                        bool space_added_to_left = !(interprate->flag & printf_interprate_flag_left_justify);
                        
                        // space output
                        if(space_added_to_left) for(size_t index = 0; index < width_addition_amount; index++)
                            fputc(CHARACTER_space, core->output.file);
                        
                        // sign output
                        if(sign == printf_sign_plus) putc(CHARACTER_plus, core->output.file);
                        else if(sign == printf_sign_minus) putc(CHARACTER_minus, core->output.file);
                        else if(sign == printf_sign_space) putc(CHARACTER_space, core->output.file);
                        
                        // prefix output
                        if(prefix == printf_complex_prefix_0) putc(CHARACTER_0, core->output.file);
                        else if(prefix == printf_complex_prefix_0x) {
                            putc(CHARACTER_0, core->output.file);
                            putc(CHARACTER_x, core->output.file);
                        }
                        else if(prefix == printf_complex_prefix_0X) {
                            putc(CHARACTER_0, core->output.file);
                            putc(CHARACTER_X, core->output.file);
                        }
                        
                        if(!zero_no_digit) {
                            for(size_t index = 0; index < precision_padding_zero; index++)
                                putc(CHARACTER_0, core->output.file);
                            if(interprate->type == printf_specifier_d_or_i || interprate->type == printf_specifier_u) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base10_character));
                                    putc(base10_character[eachDigits[count - 1]], core->output.file);
                                }
                            }
                            else if(interprate->type == printf_specifier_o) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base8_character));
                                    putc(base8_character[eachDigits[count - 1]], core->output.file);
                                }
                            }
                            else if(interprate->type == printf_specifier_x) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_lowercase));
                                    putc(base16_character_lowercase[eachDigits[count - 1]], core->output.file);
                                }
                            }
                            else if(interprate->type == printf_specifier_X) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_uppercase));
                                    putc(base16_character_uppercase[eachDigits[count - 1]], core->output.file);
                                }
                            } ELSE_DEBUG_RETURN(false);
                        }
                        
                        // space output
                        if(!space_added_to_left) for(size_t index = 0; index < width_addition_amount; index++)
                            fputc(CHARACTER_space, core->output.file);
                    }
                }
                else {
                    // sign output
                    if(sign == printf_sign_plus) putc(CHARACTER_plus, core->output.file);
                    else if(sign == printf_sign_minus) putc(CHARACTER_minus, core->output.file);
                    else if(sign == printf_sign_space) putc(CHARACTER_space, core->output.file);
                    
                    // prefix output
                    if(prefix == printf_complex_prefix_0) putc(CHARACTER_0, core->output.file);
                    else if(prefix == printf_complex_prefix_0x) {
                        putc(CHARACTER_0, core->output.file);
                        putc(CHARACTER_x, core->output.file);
                    }
                    else if(prefix == printf_complex_prefix_0X) {
                        putc(CHARACTER_0, core->output.file);
                        putc(CHARACTER_X, core->output.file);
                    }
                    
                    if(!zero_no_digit) {
                        for(size_t index = 0; index < precision_padding_zero; index++)
                            putc(CHARACTER_0, core->output.file);
                        if(interprate->type == printf_specifier_d_or_i || interprate->type == printf_specifier_u) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base10_character));
                                putc(base10_character[eachDigits[count - 1]], core->output.file);
                            }
                        }
                        else if(interprate->type == printf_specifier_o) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base8_character));
                                putc(base8_character[eachDigits[count - 1]], core->output.file);
                            }
                        }
                        else if(interprate->type == printf_specifier_x) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_lowercase));
                                putc(base16_character_lowercase[eachDigits[count - 1]], core->output.file);
                            }
                        }
                        else if(interprate->type == printf_specifier_X) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_uppercase));
                                putc(base16_character_uppercase[eachDigits[count - 1]], core->output.file);
                            }
                        } ELSE_DEBUG_RETURN(false);
                    }
                }
            } ELSE_DEBUG_RETURN(false);
        }
        else if(core->type == printf_type_string) {
            if(core->output.string.store != NULL && core->output.string.index + 1 < core->output.string.count) {
                
                size_t max_store_amount = core->output.string.count - core->output.string.index - 1;
                size_t current_stored = 0;
                
#ifdef PF_TRY_STORE
#Warning PF_TRY_STORE predefined unexpected condition
#undef PF_TRY_STORE
#endif

    // [WARNING] this macro did not append trailing null character [mind]
#define PF_TRY_STORE(uint8_byte) do {                                           \
                    if(current_stored < max_store_amount) {                     \
                         core->output.string.store                              \
                        [core->output.string.index++] = (uint8_byte);           \
                         current_stored++; }} while(0)
                
                if(need_consider_width) {
                    DEBUG_ASSERT(width_addition_amount > 0);
                    if(width_using_zero_padding) {
                        // sign output
                        if(sign == printf_sign_plus) PF_TRY_STORE(CHARACTER_plus);
                        else if(sign == printf_sign_minus) PF_TRY_STORE(CHARACTER_minus);
                        else if(sign == printf_sign_space) PF_TRY_STORE(CHARACTER_space);
                        
                        // prefix output
                        if(prefix == printf_complex_prefix_0) PF_TRY_STORE(CHARACTER_0);
                        else if(prefix == printf_complex_prefix_0x) {
                            PF_TRY_STORE(CHARACTER_0);
                            PF_TRY_STORE(CHARACTER_x);
                        }
                        else if(prefix == printf_complex_prefix_0X) {
                            PF_TRY_STORE(CHARACTER_0);
                            PF_TRY_STORE(CHARACTER_X);
                        }
                        
                        // width zero padding output
                        for(size_t index = 0; index < width_addition_amount; index++)
                            PF_TRY_STORE(CHARACTER_0);
                        
                        if(!zero_no_digit) {
                            for(size_t index = 0; index < precision_padding_zero; index++)
                                PF_TRY_STORE(CHARACTER_0);
                            
                            if(interprate->type == printf_specifier_d_or_i || interprate->type == printf_specifier_u) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base10_character));
                                    PF_TRY_STORE(base10_character[eachDigits[count - 1]]);
                                }
                            }
                            else if(interprate->type == printf_specifier_o) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base8_character));
                                    PF_TRY_STORE(base8_character[eachDigits[count - 1]]);
                                }
                            }
                            else if(interprate->type == printf_specifier_x) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_lowercase));
                                    PF_TRY_STORE(base16_character_lowercase[eachDigits[count - 1]]);
                                }
                            }
                            else if(interprate->type == printf_specifier_X) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_uppercase));
                                    PF_TRY_STORE(base16_character_uppercase[eachDigits[count - 1]]);
                                }
                            } ELSE_DEBUG_RETURN(false);
                        }
                    }
                    else {
                        bool space_added_to_left = !(interprate->flag & printf_interprate_flag_left_justify);
                        
                        // space output
                        if(space_added_to_left) for(size_t index = 0; index < width_addition_amount; index++)
                            PF_TRY_STORE(CHARACTER_space);
                        
                        // sign output
                        if(sign == printf_sign_plus) PF_TRY_STORE(CHARACTER_plus);
                        else if(sign == printf_sign_minus) PF_TRY_STORE(CHARACTER_minus);
                        else if(sign == printf_sign_space) PF_TRY_STORE(CHARACTER_space);
                        
                        // prefix output
                        if(prefix == printf_complex_prefix_0) PF_TRY_STORE(CHARACTER_0);
                        else if(prefix == printf_complex_prefix_0x) {
                            PF_TRY_STORE(CHARACTER_0);
                            PF_TRY_STORE(CHARACTER_x);
                        }
                        else if(prefix == printf_complex_prefix_0X) {
                            PF_TRY_STORE(CHARACTER_0);
                            PF_TRY_STORE(CHARACTER_X);
                        }
                        
                        if(!zero_no_digit) {
                            for(size_t index = 0; index < precision_padding_zero; index++)
                                PF_TRY_STORE(CHARACTER_0);
                            if(interprate->type == printf_specifier_d_or_i || interprate->type == printf_specifier_u) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base10_character));
                                    PF_TRY_STORE(base10_character[eachDigits[count - 1]]);
                                }
                            }
                            else if(interprate->type == printf_specifier_o) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base8_character));
                                    PF_TRY_STORE(base8_character[eachDigits[count - 1]]);
                                }
                            }
                            else if(interprate->type == printf_specifier_x) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_lowercase));
                                    PF_TRY_STORE(base16_character_lowercase[eachDigits[count - 1]]);
                                }
                            }
                            else if(interprate->type == printf_specifier_X) {
                                for(size_t count = digits_amount; count > 0; count--) {
                                    DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_uppercase));
                                    PF_TRY_STORE(base16_character_uppercase[eachDigits[count - 1]]);
                                }
                            } ELSE_DEBUG_RETURN(false);
                        }
                        
                        // space output
                        if(!space_added_to_left) for(size_t index = 0; index < width_addition_amount; index++)
                            PF_TRY_STORE(CHARACTER_space);
                    }
                }
                else {
                    // sign output
                    if(sign == printf_sign_plus) PF_TRY_STORE(CHARACTER_plus);
                    else if(sign == printf_sign_minus) PF_TRY_STORE(CHARACTER_minus);
                    else if(sign == printf_sign_space) PF_TRY_STORE(CHARACTER_space);
                    
                    // prefix output
                    if(prefix == printf_complex_prefix_0) PF_TRY_STORE(CHARACTER_0);
                    else if(prefix == printf_complex_prefix_0x) {
                        PF_TRY_STORE(CHARACTER_0);
                        PF_TRY_STORE(CHARACTER_x);
                    }
                    else if(prefix == printf_complex_prefix_0X) {
                        PF_TRY_STORE(CHARACTER_0);
                        PF_TRY_STORE(CHARACTER_X);
                    }
                    
                    if(!zero_no_digit) {
                        for(size_t index = 0; index < precision_padding_zero; index++)
                            PF_TRY_STORE(CHARACTER_0);
                        if(interprate->type == printf_specifier_d_or_i || interprate->type == printf_specifier_u) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base10_character));
                                PF_TRY_STORE(base10_character[eachDigits[count - 1]]);
                            }
                        }
                        else if(interprate->type == printf_specifier_o) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base8_character));
                                PF_TRY_STORE(base8_character[eachDigits[count - 1]]);
                            }
                        }
                        else if(interprate->type == printf_specifier_x) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_lowercase));
                                PF_TRY_STORE(base16_character_lowercase[eachDigits[count - 1]]);
                            }
                        }
                        else if(interprate->type == printf_specifier_X) {
                            for(size_t count = digits_amount; count > 0; count--) {
                                DEBUG_ASSERT(eachDigits[count - 1] < ARRAY_SIZE(base16_character_uppercase));
                                PF_TRY_STORE(base16_character_uppercase[eachDigits[count - 1]]);
                            }
                        } ELSE_DEBUG_RETURN(false);
                    }
                }
#undef PF_TRY_STORE
                // even output nothing this is valid
                core->output.string.store[core->output.string.index] = CHARACTER_null;
            }
        } ELSE_DEBUG_RETURN(false);
        
        // increase core->actual_need
        core->actual_need += full_output_character_amount;
        
        return true;        // process success point
        
    } DEBUG_ELSE
    return false;
}

#pragma mark floating point type [fFeEgGaA]

#pragma mark floating point base convert

typedef struct floating_point_base_convert_type {
    bool sign;
    EXT_ARRAY remain;
    EXT_ARRAY scale;
    EXT_ARRAY nearby_down;
    EXT_ARRAY nearby_up;
    EXT_ARRAY base;
    uint32_t base_value;
    long exponent;
    size_t length;
} fpbc_t;

typedef fpbc_t * fpbc_ref;

#pragma mark floating point simple fix up

static bool printf_output_specifier_floating_point_fpbc_simple_fixup(fp_info_ref _Nonnull info,
                                                                     fpbc_ref _Nonnull fpbc,
                                                                     bool nearby_down_decrease_exponent) {
    if(info != NULL && fpbc != NULL) {
        // e - p
        // [positive] could just store significand(with implicit one) offseted (e - p)
        // [negative] max[ (need_for_query_significand_bit + 1), e_minus_p + 1 ]
        
        size_t length = fpbc->length;
        
        EXT_ARRAY temp1 = printf_stack_alloc(sizeof(EXT_TYPE) * length);
        EXT_ARRAY temp2 = printf_stack_alloc(sizeof(EXT_TYPE) * length);
        
        if(nearby_down_decrease_exponent) {
            // nearby_down_decrease_exponent need even more one
            
            // nearby_up
            extended_integer_bit_offset(fpbc->nearby_up, temp1, length, 1);
            extended_integer_copy(temp1, fpbc->nearby_up, length);
            
            // remain
            extended_integer_bit_offset(fpbc->remain, temp1, length, 1);
            extended_integer_copy(temp1, fpbc->remain, length);
            
            // scale
            extended_integer_bit_offset(fpbc->scale, temp1, length, 1);
            extended_integer_copy(temp1, fpbc->scale, length);
        }
        
        bool error_flag;
        
        long exponent = 0;
// [NOTE] [optimization] exponent range overflow detect [not likely]
        
        LOOP {
            // v            (remain       / scale) x Base^(k)
            // v - v(-)     (nearby_minus / scale) x Base^(k)
            // v(+) - v     (nearby_plus  / scale) x Base^(k)
            
            extended_integer_divide(fpbc->scale, fpbc->base, temp1, length, &error_flag);
            if(error_flag) DEBUG_RETURN(false);
            
            // temp = scale / base
            if(extended_integer_compare(fpbc->remain, temp1, length) < 0) {
                DEBUG_ASSERT(exponent != INT_MIN);
                exponent--;
                extended_integer_quick_multiply(fpbc->remain, fpbc->base_value, length, &error_flag);
                if(error_flag) DEBUG_RETURN(false);
                extended_integer_quick_multiply(fpbc->nearby_down, fpbc->base_value, length, &error_flag);
                if(error_flag) DEBUG_RETURN(false);
                extended_integer_quick_multiply(fpbc->nearby_up, fpbc->base_value, length, &error_flag);
                if(error_flag) DEBUG_RETURN(false);
                // remain & nearby_down & nearby_up scale B size
            }
            else break;     // exit LOOP
        }
        // exponent = min{0, 1 + [log(B, v)]}
        LOOP {
            // v            (remain       / scale) x Base^(k)
            // v - v(-)     (nearby_minus / scale) x Base^(k)
            // v(+) - v     (nearby_plus  / scale) x Base^(k)
            
            // remain sacled two plus nearby_up
            
            // temp1 = (2 x remain) + nearby_up
            extended_integer_add(fpbc->remain, fpbc->remain, temp2, length, &error_flag);
            if(error_flag) DEBUG_RETURN(false);
            extended_integer_add(temp2, fpbc->nearby_up, temp1, length, &error_flag);
            if(error_flag) DEBUG_RETURN(false);
            
            // temp2 = 2 x scale
            extended_integer_add(fpbc->scale, fpbc->scale, temp2, length, &error_flag);
            if(error_flag) DEBUG_RETURN(false);
            
            if(extended_integer_compare(temp1, temp2, length) >= 0) {
                // scale scaled to B
                extended_integer_multiply(fpbc->scale, fpbc->base, temp1, length, &error_flag);
                if(error_flag) DEBUG_RETURN(false);
                extended_integer_copy(temp1, fpbc->scale, length);
                exponent++;
            }
            else break;
        }
        
        // exponent = 1 + [log(B, (v+v(+))/2)]
        fpbc->exponent = exponent;
        
        return true;
    }
    return false;
}

static bool floating_point_base_convert_query_next_digit(fpbc_ref _Nonnull fpbc,
                                                         long * _Nonnull exponent,
                                                         uint32_t * _Nonnull value,
                                                         bool * _Nonnull complete) {
    if(fpbc != NULL && exponent != NULL && value != NULL && complete != NULL) {
        
        /* fpbc var */
        EXT_ARRAY remain = fpbc->remain;                DEBUG_ASSERT(remain != 0);
        EXT_ARRAY scale = fpbc->scale;                  DEBUG_ASSERT(scale != 0);
        EXT_ARRAY nearby_up = fpbc->nearby_up;          DEBUG_ASSERT(nearby_up != 0);
        EXT_ARRAY nearby_down = fpbc->nearby_down;      DEBUG_ASSERT(nearby_down != 0);
        uint32_t base_value = fpbc->base_value;         DEBUG_ASSERT(base_value > 0);
        size_t length = fpbc->length;                   DEBUG_ASSERT(length > 0);
        
        /* temp var */
        bool error_flag;
        uint32_t current;
        bool low, high;
        
        EXT_ARRAY remain_times_two = printf_stack_alloc(sizeof(EXT_TYPE) * length);
        EXT_ARRAY sacle_times_two = printf_stack_alloc(sizeof(EXT_TYPE) * length);
        EXT_ARRAY scale_times_two_minus_nearby_up = printf_stack_alloc(sizeof(EXT_TYPE) * length);
        
        // remain / scale * base ^ (k) + SUM(H, i = k, Di * B^(i)) == value
        fpbc->exponent = fpbc->exponent - 1;    // current output exponent
        
        extended_integer_quick_multiply(remain, base_value, length, &error_flag);
        DEBUG_ASSERT(!error_flag); if(error_flag) return false;
        
        current = extended_integer_quick_divide_modulo(remain, scale, length, &error_flag);
        if(error_flag) DEBUG_RETURN(false);
        if(current >= ARRAY_SIZE(base10_character)) DEBUG_RETURN(false);
        
        extended_integer_quick_multiply(nearby_down, base_value, length, &error_flag);
        if(error_flag) DEBUG_RETURN(false);
        
        extended_integer_quick_multiply(nearby_up, base_value, length, &error_flag);
        if(error_flag) DEBUG_RETURN(false);
        
        extended_integer_bit_offset(remain, remain_times_two, length, 1);   // scale by two
        
        low  = extended_integer_compare(remain_times_two, nearby_down, length) < 0;
        
        extended_integer_bit_offset(scale, sacle_times_two, length, 1);   // scale by two
        
        // optimized high decision
        if(extended_integer_compare(sacle_times_two, nearby_up, length) < 0)
            high = true;
        else {
            extended_integer_sbtract(sacle_times_two, nearby_up, scale_times_two_minus_nearby_up, length, &error_flag);
            if(error_flag) DEBUG_RETURN(false);
            high = extended_integer_compare(remain_times_two, scale_times_two_minus_nearby_up, length) > 0;
        }
        
        if(!low && !high) {     // correct output with more digits need for read back form
            exponent[0] = fpbc->exponent;       // output assignment
            value[0] = current;                 // value assignment
            complete[0] = false;                // complete assignment
            return true;
        }
        
        if(low && !high) value[0] = current;
        else if(high && !low) value[0] = current + 1;
        else { // low && high
            // remain_times_two should be valid
            int round = extended_integer_compare(remain_times_two, scale, length);
            if(round < 0) value[0] = current;
            else if(round > 0) {
                if(current + 1 >= ARRAY_SIZE(base10_character)) DEBUG_RETURN(false);
                value[0] = current + 1;
            }
            // default round direction
            else value[0] = current;
        }
        
        exponent[0] = fpbc->exponent;       // output assignment
        complete[0] = true;                 // complete assignment
        return true;
        
    } DEBUG_ELSE
    return 0u;
}

/*!
    @function floating_point_base_convert_try_next_exponent_inline
    @abstract this is only valid after simple fix up
 */
static inline bool floating_point_base_convert_try_next_exponent_inline(fpbc_ref _Nonnull fpbc,
                                                                        long * _Nonnull exponent) {
    if(fpbc != NULL && exponent != NULL) {
        exponent[0] = fpbc->exponent - 1;
        return true;
    } DEBUG_ELSE
    return 0u;
}

#pragma mark floating point subnormal types

typedef struct printf_floating_point_zero_pure_data {
    size_t pure_width;
} printf_floating_point_zero_pure_data_t;

typedef printf_floating_point_zero_pure_data_t * printf_floating_point_zero_pure_data_ref;

static bool printf_floating_point_zero_pure_function(printf_core_ref _Nonnull core,
                                                     printf_interprate_ref _Nonnull interprate,
                                                     size_t pure_width,
                                                     void * _Nonnull input);

static bool printf_output_specifier_floating_point_zero(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate, fp_info_ref _Nonnull info) {
    if(core != NULL && interprate != NULL && info != NULL && printf_specifier_type_is_floating_point_inline(interprate->type)) {
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_waiting_input);
        
        size_t pure_width = 0;
        
        switch (interprate->type) {
            case printf_specifier_f:
            case printf_specifier_F:
                pure_width += 1;        // 0
                if(interprate->precision > 0)
                    pure_width += interprate->precision + 1;        // .xxx
                else if(interprate->flag & printf_interprate_flag_complex)
                    pure_width += 1;                                // .
                break;
            case printf_specifier_e:
            case printf_specifier_E:
                pure_width += 1;        // 0
                if(interprate->precision > 0)
                    pure_width += interprate->precision + 1;        // .xxx
                else if(interprate->flag & printf_interprate_flag_complex)
                    pure_width += 1;                                // .
                pure_width += 4;                                    // (e/E)+00
                break;
            case printf_specifier_g:
            case printf_specifier_G:
                // must be in %f style format
                pure_width += 1;        // 0
                if(interprate->flag & printf_interprate_flag_complex)
                    pure_width += 1;                                // .
                break;
            case printf_specifier_a:
            case printf_specifier_A:
                pure_width += 3;        // 0x0
                if(interprate->precision_type == printf_interprate_adjustment_unspecified)
                    interprate->precision = 0;
                if(interprate->precision > 0)
                    pure_width += interprate->precision + 1;        // .xxx
                else if(interprate->flag & printf_interprate_flag_complex)
                    pure_width += 1;                                // .
                pure_width += 3;        // p+0
                break;
            case printf_specifier_invalid:
            default: DEBUG_RETURN(false);
        }
        
        printf_floating_point_zero_pure_data_t pure_data_store;
        printf_floating_point_zero_pure_data_ref pure_data = &pure_data_store;
        pure_data->pure_width = pure_width;
        
        bool is_floating_negative = floating_point_info_query_sign_bit_inline(info);
        
        return printf_core_output_flag_width_help(core,
                                                  interprate,
                                                  pure_width,
                                                  printf_complex_prefix_none,
                                                  is_floating_negative,
                                                  printf_floating_point_zero_pure_function,
                                                  pure_data);
    } DEBUG_ELSE
    return false;
}

static bool printf_floating_point_zero_pure_function(printf_core_ref _Nonnull core,
                                                     printf_interprate_ref _Nonnull interprate,
                                                     size_t pure_width,
                                                     void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_floating_point_zero_pure_data_ref pure_data = (printf_floating_point_zero_pure_data_ref)input;
        if(pure_data->pure_width == pure_width) {
            switch (interprate->type) {
                case printf_specifier_f:
                case printf_specifier_F:
                    printf_core_output_character(core, CHARACTER_0);
                    if(interprate->precision > 0) {
                        printf_core_output_character(core, CHARACTER_dot);
                        for(size_t index = 0; index < interprate->precision; index++)
                            printf_core_output_character(core, CHARACTER_0);
                    }
                    else if(interprate->flag & printf_interprate_flag_complex)
                        printf_core_output_character(core, CHARACTER_dot);
                    break;
                case printf_specifier_e:
                case printf_specifier_E:
                    printf_core_output_character(core, CHARACTER_0);
                    if(interprate->precision > 0) {
                        printf_core_output_character(core, CHARACTER_dot);
                        for(size_t index = 0; index < interprate->precision; index++)
                            printf_core_output_character(core, CHARACTER_0);
                    }
                    else if(interprate->flag & printf_interprate_flag_complex)
                        printf_core_output_character(core, CHARACTER_dot);
                    if(interprate->type == printf_specifier_e)
                        printf_core_output_character(core, CHARACTER_e);
                    else printf_core_output_character(core, CHARACTER_E);
                    printf_core_output_character(core, CHARACTER_plus);
                    printf_core_output_character(core, CHARACTER_0);
                    printf_core_output_character(core, CHARACTER_0);
                    break;
                case printf_specifier_g:
                case printf_specifier_G:
                    // must be in %f style format
                    printf_core_output_character(core, CHARACTER_0);
                    if(interprate->flag & printf_interprate_flag_complex)
                        printf_core_output_character(core, CHARACTER_dot);
                    break;
                case printf_specifier_a:
                case printf_specifier_A:
                    pure_width += 3;        // 0x0
                    printf_core_output_character(core, CHARACTER_0);
                    if(interprate->type == printf_specifier_a)
                        printf_core_output_character(core, CHARACTER_x);
                    else printf_core_output_character(core, CHARACTER_X);
                    printf_core_output_character(core, CHARACTER_0);
                    DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_unspecified || interprate->precision == 0);
                    if(interprate->precision > 0) {
                        printf_core_output_character(core, CHARACTER_dot);
                        for(size_t index = 0; index < interprate->precision; index++)
                            printf_core_output_character(core, CHARACTER_0);
                    }
                    else if(interprate->flag & printf_interprate_flag_complex)
                        printf_core_output_character(core, CHARACTER_dot);
                    if(interprate->type == printf_specifier_a)
                        printf_core_output_character(core, CHARACTER_p);
                    else printf_core_output_character(core, CHARACTER_P);
                    printf_core_output_character(core, CHARACTER_plus);
                    printf_core_output_character(core, CHARACTER_0);
                    break;
                case printf_specifier_invalid:
                default: DEBUG_RETURN(false);
            }
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point_nan(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate, fp_info_ref _Nonnull info) {
    if(core != NULL && interprate != NULL && info != NULL && printf_specifier_type_is_floating_point_inline(interprate->type)) {
        DEBUG_ASSERT(floating_point_info_classification(info) == fp_classification_nan);
        
        bool uppercase = false;
        if(interprate->type == printf_specifier_F ||
           interprate->type == printf_specifier_E ||
           interprate->type == printf_specifier_G ||
           interprate->type == printf_specifier_A)
            uppercase = true;
        
        if(uppercase) printf_core_output_raw_string(core, NAN_STRING_UPPERCASE, NAN_STRING_LENGTH);
        else printf_core_output_raw_string(core, NAN_STRING_LOWERCASE, NAN_STRING_LENGTH);
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point_inf(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate, fp_info_ref _Nonnull info) {
    if(core != NULL && interprate != NULL && info != NULL && printf_specifier_type_is_floating_point_inline(interprate->type)) {
        DEBUG_ASSERT(floating_point_info_classification(info) == fp_classification_infinite);
        
        if(floating_point_info_query_sign_bit_inline(info)) {
            if(core->type == printf_type_FILE) {
                if(core->output.file != NULL) {
                    putc(CHARACTER_minus, core->output.file);
                } DEBUG_ELSE
            }
            else if(core->type == printf_type_string) {
                if(core->output.string.store != NULL && core->output.string.index + 1 < core->output.string.count) {
                    DEBUG_ASSERT(core->output.string.store[core->output.string.index] == CHARACTER_null);
                    
                    core->output.string.store[core->output.string.index] = CHARACTER_minus;
                    core->output.string.store[core->output.string.index + 1] = CHARACTER_null;
                    core->output.string.index += 1;
                }
            } ELSE_DEBUG_RETURN(false);
            core->actual_need += 1;
        }
        
        bool uppercase = false;
        if(interprate->type == printf_specifier_F ||
           interprate->type == printf_specifier_E ||
           interprate->type == printf_specifier_G ||
           interprate->type == printf_specifier_A)
            uppercase = true;
        
        if(uppercase) printf_core_output_raw_string(core, INF_STRING_UPPERCASE, INF_STRING_LENGTH);
        else printf_core_output_raw_string(core, INF_STRING_LOWERCASE, INF_STRING_LENGTH);
    } DEBUG_ELSE
    return false;
}

#pragma mark floating point normal types

typedef struct printf_specifier_f_pure_data {
    unsigned char *significand;
    size_t left_index;
    size_t zero_index;
    size_t right_index;
    size_t pure_width;
} printf_specifier_f_pure_data_t;

typedef printf_specifier_f_pure_data_t * printf_specifier_f_pure_data_ref;

static bool printf_specifier_f_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input);

static bool printf_output_specifier_floating_point_specifier_fF(printf_core_ref _Nonnull core,
                                                                printf_interprate_ref _Nonnull interprate,
                                                                fp_info_ref _Nonnull info,
                                                                fpbc_ref _Nonnull fpbc) {
    if(core != NULL && interprate != NULL && info != NULL && fpbc != NULL) {
        
        DEBUG_ASSERT(interprate->type == printf_specifier_f || interprate->type == printf_specifier_F);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_waiting_input);
        
        bool check_flag; long exponent; uint32_t value; bool complete;
        check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
        if(!check_flag) DEBUG_RETURN(false);
        
        // left  = MAX{exponent, 0}
        // right = - precision
        // right_abs = |right| = precision;
        
        // [0, 1(left_index), zero_index, right_index]
        // may happen left_index == zero_index == right_index
        
        // allocated (left + right_abs + 1) + 1
        
        COMILE_ASSERT(SIZE_MAX >= LONG_MAX);
        
        size_t left, right_abs;
        if(exponent <= 0) left = 0; else left = exponent;
        right_abs = interprate->precision;
        
        size_t allocated_size;
        if(left > PRINTF_STACK_ALLOCATION_LIMIT || right_abs > PRINTF_STACK_ALLOCATION_LIMIT)
            DEVELOP_BREAKPOINT_RETURN(false);
            // [BREAKPOINT] this is too large to parsing
        if((allocated_size = left + right_abs + 2) > PRINTF_STACK_ALLOCATION_LIMIT)
            DEVELOP_BREAKPOINT_RETURN(false);
            // [BREAKPOINT] this is too large to parsing
        
        DEBUG_ASSERT(UCHAR_MAX >= fpbc->base_value && UCHAR_MAX >= 9u);
        unsigned char *significand = printf_stack_alloc(sizeof(unsigned char) * allocated_size);
        
        size_t left_index = 1;
        size_t zero_index = left_index + left;
        size_t right_index = allocated_size - 1;
        DEBUG_ASSERT(zero_index >= left_index && right_index >= zero_index);
        DEBUG_ASSERT(right_index - left_index + 2 == allocated_size);
        
        long highest_exponent;
        if(exponent <= 0) highest_exponent = 0; else highest_exponent = exponent;
        
        for(size_t index = left_index; index < right_index; index++) {
            long current_exponent = highest_exponent - (index - left_index);
            if(exponent > current_exponent && !complete) {
                DEBUG_ASSERT(exponent == current_exponent + 1);
                check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                if(!check_flag) DEBUG_RETURN(false);
                DEBUG_ASSERT(exponent == current_exponent);
            }
            if(current_exponent == exponent) significand[index] = value;
            else
                significand[index] = 0;
        }
        long current_exponent = highest_exponent - (right_index - left_index);
        DEBUG_ASSERT(exponent <= current_exponent + 1 || complete);
        if(exponent > current_exponent && !complete) {
            DEBUG_ASSERT(exponent == current_exponent + 1);
            check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
            if(!check_flag) DEBUG_RETURN(false);
            DEBUG_ASSERT(exponent == current_exponent);
        }
        
        bool perform_add_one_operation = false;
        
        if(exponent == current_exponent) {
            significand[right_index] = value;
            if(!complete) {
                DEBUG_ASSERT(exponent == current_exponent);
                check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                if(!check_flag) DEBUG_RETURN(false);
                DEBUG_ASSERT(exponent + 1 == current_exponent);
                if(value >= 5) perform_add_one_operation = true;
            }
        }
        else significand[right_index] = 0;
        
        if(perform_add_one_operation) {
            bool carry_over = true;
            size_t index;
            for(index = right_index; carry_over && index > left_index; index--) {
                DEBUG_ASSERT(significand[index] >= 0 && significand[index] <= 9);
                if(significand[index] >= 9) {
                    significand[index] = 0;
                    carry_over = true;
                }
                else {
                    significand[index] += 1;
                    carry_over = false;
                }
            }
            if(carry_over) {
                DEBUG_ASSERT(index == left_index);
                DEBUG_ASSERT(significand[left_index] >= 0 && significand[left_index] <= 9);
                if(significand[left_index] >= 9) {
                    significand[left_index] = 0;
                    significand[0] = 1;
                    left_index = 0;
                }
                else significand[left_index] += 1;
            }
        }
        
        size_t pure_width = right_index - left_index + 1;
        if(right_index > zero_index || interprate->flag & printf_interprate_flag_complex) pure_width += 1;
    
        printf_specifier_f_pure_data_t pure_data_store;
        printf_specifier_f_pure_data_ref pure_data = &pure_data_store;
        pure_data->left_index = left_index;
        pure_data->zero_index = zero_index;
        pure_data->right_index = right_index;
        pure_data->significand = significand;
        pure_data->pure_width = pure_width;
        
        return printf_core_output_flag_width_help(core,
                                                  interprate,
                                                  pure_width,
                                                  printf_complex_prefix_none,
                                                  fpbc->sign,
                                                  printf_specifier_f_pure_function,
                                                  pure_data);
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_f_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_specifier_f_pure_data_ref pure_data = (printf_specifier_f_pure_data_ref)input;
        DEBUG_ASSERT(pure_data->pure_width == pure_width);
        if(pure_data->significand != NULL && pure_data->pure_width == pure_width) {
            DEBUG_ASSERT(pure_data->left_index <= pure_data->zero_index && pure_data->zero_index <= pure_data->right_index);
            for(size_t index = pure_data->left_index; index <= pure_data->zero_index; index++) {
                DEBUG_ASSERT(pure_data->significand[index] < ARRAY_SIZE(base10_character));
                UTF8Char ch = base10_character[pure_data->significand[index]];
                printf_core_output_character(core, ch);
            }
            if(pure_data->zero_index == pure_data->right_index) {
                if(interprate->flag & printf_interprate_flag_complex)
                    printf_core_output_character(core, CHARACTER_dot);
            }
            else {
                printf_core_output_character(core, CHARACTER_dot);
                for(size_t index = pure_data->zero_index + 1; index <= pure_data->right_index; index++) {
                    DEBUG_ASSERT(pure_data->significand[index] < ARRAY_SIZE(base10_character));
                    UTF8Char ch = base10_character[pure_data->significand[index]];
                    printf_core_output_character(core, ch);
                }
            }
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

typedef struct printf_specifier_e_pure_data {
    size_t first_index;
    size_t last_index;
    bool need_decimal_point;
    bool first_exponent_negative;
    long first_exponent_abs;
    size_t first_exponent_pure_width;
    size_t first_exponent_need_width;
    unsigned char *significand;
    size_t pure_width;
} printf_specifier_e_pure_data_t;

typedef printf_specifier_e_pure_data_t * printf_specifier_e_pure_data_ref;

static bool printf_specifier_e_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input);

static bool printf_output_specifier_floating_point_specifier_eE(printf_core_ref _Nonnull core,
                                                                printf_interprate_ref _Nonnull interprate,
                                                                fp_info_ref _Nonnull info,
                                                                fpbc_ref _Nonnull fpbc) {
    if(core != NULL && interprate != NULL && info != NULL && fpbc != NULL) {
        
        DEBUG_ASSERT(interprate->type == printf_specifier_e || interprate->type == printf_specifier_E);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_waiting_input);
        
        bool check_flag; long exponent; uint32_t value; bool complete;
        check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
        if(!check_flag) DEBUG_RETURN(false);
        
        bool need_decimal_point = true;
        if(interprate->precision == 0 && !(interprate->flag & printf_interprate_flag_complex))
            need_decimal_point = false;
        
        if(interprate->precision > PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
        size_t allocated_size = interprate->precision + 2;  // [oveflow]9.1234
        if(allocated_size > PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
        
        long first_exponent = exponent;
        long first_exponent_abs = first_exponent;
        bool first_exponent_negative = false;
        if(first_exponent_abs < 0) {
            first_exponent_negative = true;
            first_exponent_abs = - first_exponent_abs;
        }
        
        size_t first_exponent_pure_width = 1;
        for(long temp = first_exponent_abs; (temp /= 10) > 0;)
            first_exponent_pure_width++;
        
        // not possible to happen
        if(first_exponent_pure_width > PRINTF_INTEGER_MAX_DIGITS_AMOUNT) DEBUG_RETURN(false);
        
        size_t first_exponent_need_width = 2;
        if(first_exponent_pure_width > first_exponent_need_width)
            first_exponent_need_width = first_exponent_pure_width;
        
        size_t first_index = 1;
        size_t last_index = interprate->precision + 1;
        
        DEBUG_ASSERT(UCHAR_MAX >= fpbc->base_value && UCHAR_MAX >= 9u);
        unsigned char *significand = printf_stack_alloc(sizeof(unsigned char) * allocated_size);
        
        for(size_t index = first_index; index < last_index; index++) {
            long current_exponent = first_exponent - (index - first_index);
            if(current_exponent < exponent && !complete) {
                DEBUG_ASSERT(current_exponent + 1 == exponent);
                check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                if(!check_flag) DEBUG_RETURN(false);
                DEBUG_ASSERT(current_exponent == exponent);
            }
            if(current_exponent == exponent) significand[index] = value;
            else significand[index] = 0u;
        }
        
        long current_exponent = first_exponent - (last_index - first_index);
        DEBUG_ASSERT(exponent <= current_exponent + 1 || complete);
        if(exponent > current_exponent && !complete) {
            DEBUG_ASSERT(exponent == current_exponent + 1);
            check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
            if(!check_flag) DEBUG_RETURN(false);
            DEBUG_ASSERT(exponent == current_exponent);
        }
        
        bool perform_add_one_operation = false;
        
        if(exponent == current_exponent) {
            significand[last_index] = value;
            if(!complete) {
                DEBUG_ASSERT(exponent == current_exponent);
                check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                if(!check_flag) DEBUG_RETURN(false);
                DEBUG_ASSERT(exponent + 1 == current_exponent);
                if(value >= 5) perform_add_one_operation = true;
            }
        }
        else significand[last_index] = 0;
        
        if(perform_add_one_operation) {
            bool carry_over = true;
            size_t index;
            for(index = last_index; carry_over && index > first_index; index--) {
                DEBUG_ASSERT(significand[index] >= 0 && significand[index] <= 9);
                if(significand[index] >= 9) {
                    significand[index] = 0;
                    carry_over = true;
                }
                else {
                    significand[index] += 1;
                    carry_over = false;
                }
            }
            if(carry_over) {
                DEBUG_ASSERT(index == first_index);
                DEBUG_ASSERT(significand[first_index] >= 0 && significand[first_index] <= 9);
                if(significand[first_index] >= 9) {
                    significand[first_index] = 0;
                    significand[0] = 1;
                    first_index = 0;
                    last_index -= 1;    // last could not round again
                }
                else significand[first_index] += 1;
            }
        }
        
        size_t pure_width = last_index - first_index + 1;
        if(need_decimal_point) pure_width += 1;
        pure_width += 2;        // e+
        pure_width += first_exponent_need_width;

        printf_specifier_e_pure_data_t pure_data_store;
        printf_specifier_e_pure_data_ref pure_data = &pure_data_store;
        pure_data->first_index = first_index;
        pure_data->last_index = last_index;
        pure_data->need_decimal_point = need_decimal_point;
        pure_data->first_exponent_negative = first_exponent_negative;
        pure_data->first_exponent_abs = first_exponent_abs;
        pure_data->first_exponent_pure_width = first_exponent_pure_width;
        pure_data->first_exponent_need_width = first_exponent_need_width;
        pure_data->significand = significand;
        pure_data->pure_width = pure_width;
        
        return printf_core_output_flag_width_help(core,
                                                  interprate,
                                                  pure_width,
                                                  printf_complex_prefix_none,
                                                  fpbc->sign,
                                                  printf_specifier_e_pure_function,
                                                  pure_data);
        
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_e_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_specifier_e_pure_data_ref pure_data = (printf_specifier_e_pure_data_ref)input;
        DEBUG_ASSERT(pure_data->pure_width == pure_width);
        if(pure_data->significand != NULL && pure_data->pure_width == pure_width) {
            DEBUG_ASSERT(pure_data->first_index <= pure_data->last_index);
            DEBUG_ASSERT(pure_data->first_exponent_abs >= 0);
            
            DEBUG_ASSERT(pure_data->significand[pure_data->first_index] < ARRAY_SIZE(base10_character));
            UTF8Char ch = base10_character[pure_data->significand[pure_data->first_index]];
            printf_core_output_character(core, ch);
            if(pure_data->need_decimal_point) printf_core_output_character(core, CHARACTER_dot);
            for(size_t index = pure_data->first_index + 1; index <= pure_data->last_index; index++) {
                DEBUG_ASSERT(pure_data->significand[index] < ARRAY_SIZE(base10_character));
                UTF8Char ch = base10_character[pure_data->significand[index]];
                printf_core_output_character(core, ch);
            }
            if(interprate->type == printf_specifier_e)
                printf_core_output_character(core, CHARACTER_e);
            else if(interprate->type == printf_specifier_E)
                printf_core_output_character(core, CHARACTER_E);
            ELSE_DEBUG_RETURN(false);
            if(pure_data->first_exponent_negative)
                printf_core_output_character(core, CHARACTER_minus);
            else printf_core_output_character(core, CHARACTER_plus);
            if(pure_data->first_exponent_need_width > pure_data->first_exponent_pure_width)
                for(size_t index = 0; index < (pure_data->first_exponent_need_width - pure_data->first_exponent_pure_width); index++)
                    printf_core_output_character(core, CHARACTER_0);
            DEBUG_ASSERT(UCHAR_MAX >= 9u);
            DEBUG_ASSERT(pure_data->first_exponent_pure_width >= 1);
            unsigned char *exponent_value = printf_stack_alloc(sizeof(unsigned char) * pure_data->first_exponent_pure_width);
            long temp = pure_data->first_exponent_abs;
            size_t index = 0;
            do {
                DEBUG_ASSERT(index < pure_data->first_exponent_pure_width);
                if(index >= pure_data->first_exponent_pure_width) DEBUG_RETURN(false);
                exponent_value[index++] = temp % 10;
                temp /= 10;
            } while(temp > 0);
            DEBUG_ASSERT(index == pure_data->first_exponent_pure_width);
            if(index != pure_data->first_exponent_pure_width) DEBUG_RETURN(false);
            for(size_t index = 0; index < pure_data->first_exponent_pure_width; index++) {
                size_t query_index = pure_data->first_exponent_pure_width - index - 1;
                DEBUG_ASSERT(exponent_value[query_index] < ARRAY_SIZE(base10_character));
                UTF8Char ch = base10_character[exponent_value[query_index]];
                printf_core_output_character(core, ch);
            }
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point_specifier_gG_f_form(printf_core_ref _Nonnull core,
                                                                       printf_interprate_ref _Nonnull interprate,
                                                                       fp_info_ref _Nonnull info,
                                                                       fpbc_ref _Nonnull fpbc) {
    if(core != NULL && interprate != NULL && info != NULL && fpbc != NULL) {
        
        DEBUG_ASSERT(interprate->type == printf_specifier_g || interprate->type == printf_specifier_G);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision >= 1);
        
        if(interprate->precision == 0) interprate->precision = 1;
        
        bool check_flag; long exponent; uint32_t value; bool complete;
        check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
        if(!check_flag) DEBUG_RETURN(false);
        
        // left  = MAX{exponent, 0}
        // right = - precision
        
        // exponent >= 0 => exponent < precision => right = precision - exponent
        // exponent < 0  => exponent >= -4       => right = exponent - (precision - 1)
        
        
        // exponent >= 0 => right_abs = 0
        // exponent < 0  => right_abs = precision - 1 - exponent
        
        // [0, 1(left_index), zero_index, right_index]
        // may happen left_index == zero_index == right_index
        
        // allocated (left + right_abs + 1) + 1
        
        COMILE_ASSERT(SIZE_MAX >= LONG_MAX);
        
        // left is left exponent value
        // right_abs is the right exponent value absolutely
        
        size_t precision = interprate->precision;
        size_t left, right_abs;
        if(exponent >= 0) {
            DEBUG_ASSERT(precision > exponent);
            left = exponent;
            right_abs = precision - exponent - 1;
        }
        else {
            left = 0;
            DEBUG_ASSERT(interprate->precision >= 1);
            long exponent_abs = - exponent;
            right_abs = interprate->precision - 1 + exponent_abs;
        }
        
        size_t allocated_size;
        if(left > PRINTF_STACK_ALLOCATION_LIMIT || right_abs > PRINTF_STACK_ALLOCATION_LIMIT)
            DEVELOP_BREAKPOINT_RETURN(false);
            // [BREAKPOINT] this is too large to parsing
        if((allocated_size = left + right_abs + 2) > PRINTF_STACK_ALLOCATION_LIMIT)
            DEVELOP_BREAKPOINT_RETURN(false);
            // [BREAKPOINT] this is too large to parsing
        
        DEBUG_ASSERT(UCHAR_MAX >= fpbc->base_value && UCHAR_MAX >= 9u);
        unsigned char *significand = printf_stack_alloc(sizeof(unsigned char) * allocated_size);
        
        size_t left_index = 1;
        size_t zero_index = left_index + left;
        size_t right_index = allocated_size - 1;
        DEBUG_ASSERT(zero_index >= left_index && right_index >= zero_index);
        DEBUG_ASSERT(right_index - left_index + 2 == allocated_size);
        
        long highest_exponent;
        if(exponent <= 0) highest_exponent = 0; else highest_exponent = exponent;
        
        // which means will disable the trailing zero after the decimal point
        // which is useless in %g mode
        bool form_well = false;
        
        for(size_t index = left_index; index < right_index; index++) {
            long current_exponent = highest_exponent - (index - left_index);
            if(exponent > current_exponent) {
                if(!complete) {
                    DEBUG_ASSERT(exponent == current_exponent + 1);
                    check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                    if(!check_flag) DEBUG_RETURN(false);
                    DEBUG_ASSERT(exponent == current_exponent);
                }
                else {
                    // quick finish
                    form_well = true;
                    DEBUG_ASSERT(index > left_index);
                    // this should not happen
                    if(index <= left_index) DEBUG_RETURN(false);
                    if(index > zero_index)
                        right_index = index - 1;
                    else {
                        right_index = zero_index;
                        for(size_t loop = index; loop <= zero_index; loop++)
                            significand[loop] = 0;
                    }
                    // when exit
                    // index >  zero_index, just put right_index to (index - 1)
                    // index <= zero_index, just put right_index to zero_index, and fill index - zero_index with zero
                    break; // break exit for
                }
            }
            if(current_exponent == exponent) significand[index] = value;
            else significand[index] = 0;
        }
        // break exit here
        
        if(!form_well) {
            long current_exponent = highest_exponent - (right_index - left_index);
            DEBUG_ASSERT(exponent <= current_exponent + 1 || complete);
            if(exponent > current_exponent && !complete) {
                DEBUG_ASSERT(exponent == current_exponent + 1);
                check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                if(!check_flag) DEBUG_RETURN(false);
                DEBUG_ASSERT(exponent == current_exponent);
            }
            
            bool perform_add_one_operation = false;
            
            if(exponent == current_exponent) {
                significand[right_index] = value;
                if(!complete) {
                    DEBUG_ASSERT(exponent == current_exponent);
                    check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                    if(!check_flag) DEBUG_RETURN(false);
                    DEBUG_ASSERT(exponent + 1 == current_exponent);
                    if(value >= 5) perform_add_one_operation = true;
                }
            }
            else significand[right_index] = 0;
            
            if(perform_add_one_operation) {
                bool carry_over = true;
                size_t index;
                for(index = right_index; carry_over && index > left_index; index--) {
                    DEBUG_ASSERT(significand[index] >= 0 && significand[index] <= 9);
                    if(significand[index] >= 9) {
                        significand[index] = 0;
                        carry_over = true;
                    }
                    else {
                        significand[index] += 1;
                        carry_over = false;
                    }
                }
                if(carry_over) {
                    DEBUG_ASSERT(index == left_index);
                    DEBUG_ASSERT(significand[left_index] >= 0 && significand[left_index] <= 9);
                    if(significand[left_index] >= 9) {
                        significand[left_index] = 0;
                        significand[0] = 1;
                        left_index = 0;
                    }
                    else significand[left_index] += 1;
                }
            }
        }
        
        // get ride of trilling zero
        while(right_index > zero_index)
            if(significand[right_index] == 0)
                right_index = right_index - 1;
            else break;
        
        size_t pure_width = right_index - left_index + 1;
        if(right_index > zero_index || interprate->flag & printf_interprate_flag_complex) pure_width += 1;
    
        printf_specifier_f_pure_data_t pure_data_store;
        printf_specifier_f_pure_data_ref pure_data = &pure_data_store;
        pure_data->left_index = left_index;
        pure_data->zero_index = zero_index;
        pure_data->right_index = right_index;
        pure_data->significand = significand;
        pure_data->pure_width = pure_width;
        
        // cheating code
        if(interprate->type == printf_specifier_g)
            interprate->type = printf_specifier_f;
        else if(interprate->type == printf_specifier_G)
            interprate->type = printf_specifier_F;
        ELSE_DEBUG_RETURN(false);
        
        return printf_core_output_flag_width_help(core,
                                                  interprate,
                                                  pure_width,
                                                  printf_complex_prefix_none,
                                                  fpbc->sign,
                                                  printf_specifier_f_pure_function,
                                                  pure_data);
        
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point_specifier_gG_e_form(printf_core_ref _Nonnull core,
                                                                       printf_interprate_ref _Nonnull interprate,
                                                                       fp_info_ref _Nonnull info,
                                                                       fpbc_ref _Nonnull fpbc) {
    if(core != NULL && interprate != NULL && info != NULL && fpbc != NULL) {
        
        DEBUG_ASSERT(interprate->type == printf_specifier_g || interprate->type == printf_specifier_G);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision >= 1);
        
        if(interprate->precision == 0) interprate->precision = 1;
        
        bool check_flag; long exponent; uint32_t value; bool complete;
        check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
        if(!check_flag) DEBUG_RETURN(false);
        
        if(interprate->precision > PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
        size_t allocated_size = interprate->precision + 1;  // [oveflow]9.1234
        if(allocated_size > PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
        
        long first_exponent = exponent;
        long first_exponent_abs = first_exponent;
        bool first_exponent_negative = false;
        if(first_exponent_abs < 0) {
            first_exponent_negative = true;
            first_exponent_abs = - first_exponent_abs;
        }
        
        size_t first_exponent_pure_width = 1;
        for(long temp = first_exponent_abs; (temp /= 10) > 0;)
            first_exponent_pure_width++;
        
        // not possible to happen
        if(first_exponent_pure_width > PRINTF_INTEGER_MAX_DIGITS_AMOUNT) DEBUG_RETURN(false);
        
        size_t first_exponent_need_width = 2;
        if(first_exponent_pure_width > first_exponent_need_width)
            first_exponent_need_width = first_exponent_pure_width;
        
        size_t first_index = 1;
        size_t last_index = interprate->precision;
        
        DEBUG_ASSERT(UCHAR_MAX >= fpbc->base_value && UCHAR_MAX >= 9u);
        unsigned char *significand = printf_stack_alloc(sizeof(unsigned char) * allocated_size);
        
        bool form_well = false;
        
        for(size_t index = first_index; index < last_index; index++) {
            long current_exponent = first_exponent - (index - first_index);
            if(current_exponent < exponent) {
                if(!complete) {
                    DEBUG_ASSERT(current_exponent + 1 == exponent);
                    check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                    if(!check_flag) DEBUG_RETURN(false);
                    DEBUG_ASSERT(current_exponent == exponent);
                }
                else {
                    // quick finish
                    form_well = true;
                    DEBUG_ASSERT(index > first_index);
                    // this should not happen
                    if(index <= first_index) DEBUG_RETURN(false);
                    last_index = index - 1;
                    break; // break exit for
                }
            }
            if(current_exponent == exponent) significand[index] = value;
            else significand[index] = 0u;
        }
        
        if(!form_well) {
            long current_exponent = first_exponent - (last_index - first_index);
            DEBUG_ASSERT(exponent <= current_exponent + 1 || complete);
            if(exponent > current_exponent && !complete) {
                DEBUG_ASSERT(exponent == current_exponent + 1);
                check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                if(!check_flag) DEBUG_RETURN(false);
                DEBUG_ASSERT(exponent == current_exponent);
            }
            
            bool perform_add_one_operation = false;
            
            if(exponent == current_exponent) {
                significand[last_index] = value;
                if(!complete) {
                    DEBUG_ASSERT(exponent == current_exponent);
                    check_flag = floating_point_base_convert_query_next_digit(fpbc, &exponent, &value, &complete);
                    if(!check_flag) DEBUG_RETURN(false);
                    DEBUG_ASSERT(exponent + 1 == current_exponent);
                    if(value >= 5) perform_add_one_operation = true;
                }
            }
            else significand[last_index] = 0;
            
            if(perform_add_one_operation) {
                bool carry_over = true;
                size_t index;
                for(index = last_index; carry_over && index > first_index; index--) {
                    DEBUG_ASSERT(significand[index] >= 0 && significand[index] <= 9);
                    if(significand[index] >= 9) {
                        significand[index] = 0;
                        carry_over = true;
                    }
                    else {
                        significand[index] += 1;
                        carry_over = false;
                    }
                }
                if(carry_over) {
                    DEBUG_ASSERT(index == first_index);
                    DEBUG_ASSERT(significand[first_index] >= 0 && significand[first_index] <= 9);
                    if(significand[first_index] >= 9) {
                        significand[first_index] = 0;
                        significand[0] = 1;
                        first_index = 0;
                        last_index -= 1;    // last could not round again
                    }
                    else significand[first_index] += 1;
                }
            }
        }
        
        // get ride of trilling zero
        while(last_index > first_index)
            if(significand[last_index] == 0)
                last_index = last_index - 1;
            else break;
        
        bool need_decimal_point = true;
        if(first_index == last_index && !(interprate->flag & printf_interprate_flag_complex))
            need_decimal_point = false;
        
        size_t pure_width = last_index - first_index + 1;
        if(need_decimal_point) pure_width += 1;
        pure_width += 2;        // e+
        pure_width += first_exponent_need_width;

        printf_specifier_e_pure_data_t pure_data_store;
        printf_specifier_e_pure_data_ref pure_data = &pure_data_store;
        pure_data->first_index = first_index;
        pure_data->last_index = last_index;
        pure_data->need_decimal_point = need_decimal_point;
        pure_data->first_exponent_negative = first_exponent_negative;
        pure_data->first_exponent_abs = first_exponent_abs;
        pure_data->first_exponent_pure_width = first_exponent_pure_width;
        pure_data->first_exponent_need_width = first_exponent_need_width;
        pure_data->significand = significand;
        pure_data->pure_width = pure_width;
        
        // cheating code
        if(interprate->type == printf_specifier_g)
            interprate->type = printf_specifier_e;
        else if(interprate->type == printf_specifier_G)
            interprate->type = printf_specifier_E;
        ELSE_DEBUG_RETURN(false);
        
        return printf_core_output_flag_width_help(core,
                                                  interprate,
                                                  pure_width,
                                                  printf_complex_prefix_none,
                                                  fpbc->sign,
                                                  printf_specifier_e_pure_function,
                                                  pure_data);
        
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point_specifier_gG(printf_core_ref _Nonnull core,
                                                                printf_interprate_ref _Nonnull interprate,
                                                                fp_info_ref _Nonnull info,
                                                                fpbc_ref _Nonnull fpbc) {
    if(core != NULL && interprate != NULL && info != NULL && fpbc != NULL) {
        
        DEBUG_ASSERT(interprate->type == printf_specifier_g || interprate->type == printf_specifier_G);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_waiting_input);
        
        long exponent; bool check_flag;
        check_flag = floating_point_base_convert_try_next_exponent_inline(fpbc, &exponent);
        if(!check_flag) DEBUG_RETURN(false);
        
        /* fix in range */
        size_t precision = interprate->precision;
        if(precision > PRINTF_PRECISON_LIMIT) precision = PRINTF_PRECISON_LIMIT;
        DEBUG_ASSERT(LONG_MAX >= PRINTF_PRECISON_LIMIT);
        
        /* fix for %g */
        if(interprate->precision == 0) interprate->precision = 1;
        
        long compare_exponent = interprate->precision;
        if(exponent < -4 || exponent >= compare_exponent) {
            // form %e / %E
            return printf_output_specifier_floating_point_specifier_gG_e_form(core, interprate, info, fpbc);
        }
        else {
            // form %f / %F
            DEBUG_ASSERT(exponent < precision || exponent < 0);
            return printf_output_specifier_floating_point_specifier_gG_f_form(core, interprate, info, fpbc);
        }
        
    } DEBUG_ELSE
    return false;
}

typedef struct printf_specifier_a_pure_data {
    size_t pure_width;
    bool * _Nonnull significand_bit_data;
    bool is_uppercase;
    bool has_decimal_point;
    size_t index_after_first_none_zero;
    size_t last_none_zero_index;
    bool is_exponent_negative;
    size_t exponent_abs;
    size_t exponent_digits;
} printf_specifier_a_pure_data_t;

typedef printf_specifier_a_pure_data_t * printf_specifier_a_pure_data_ref;

static bool printf_specifier_a_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input);

static bool printf_output_specifier_floating_point_specifier_aA(printf_core_ref _Nonnull core,
                                                                printf_interprate_ref _Nonnull interprate,
                                                                fp_info_ref _Nonnull info,
                                                                bool is_normal) {
    if(core != NULL && interprate != NULL && info != NULL) {
        
        DEBUG_ASSERT(interprate->type == printf_specifier_a || interprate->type == printf_specifier_A);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_waiting_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_waiting_input);
        
        bool is_uppercase;
        if(interprate->type == printf_specifier_a) is_uppercase = false;
        else if(interprate->type == printf_specifier_A) is_uppercase = true;
        ELSE_DEBUG_RETURN(false);
        
        bool is_floating_negative = floating_point_info_query_sign_bit_inline(info);
        unsigned int raw_exponent = floating_point_info_query_raw_exponent_inline(info);
        unsigned int exponent_base = floating_point_query_exponent_base(info);
        
        size_t need_for_query_significand_bit =
        floating_point_info_query_significand_bit(info, NULL, 0);
        
        // most impossible condition
        if(need_for_query_significand_bit * sizeof(bool) >= PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
        
        bool *significand_bit_data = printf_stack_alloc(need_for_query_significand_bit * sizeof(bool) + 1);
        floating_point_info_query_significand_bit(info, significand_bit_data + 1, need_for_query_significand_bit);
        
        if(is_normal) significand_bit_data[0] = true;
        else significand_bit_data[0] = false;
        
        size_t significand_count = need_for_query_significand_bit + 1;
        
        size_t first_none_zero_index = 0;
        while(first_none_zero_index < significand_count)
            if(significand_bit_data[first_none_zero_index]) break;
            else first_none_zero_index++;
        
        if(first_none_zero_index >= significand_count) DEBUG_RETURN(false);
        
        size_t last_none_zero_index = first_none_zero_index;
        DEBUG_ASSERT(significand_count >= 1);
        for(size_t index = significand_count - 1; index > first_none_zero_index; index--)
            if(significand_bit_data[index]) { last_none_zero_index = index; break; }
        
        DEBUG_ASSERT(last_none_zero_index >= first_none_zero_index &&
                     last_none_zero_index < significand_count);
        
        bool is_exponent_negative;
        size_t exponent_abs;
        size_t temp = exponent_base + first_none_zero_index;
        
        if(raw_exponent >= temp) {
            is_exponent_negative = false;
            exponent_abs = raw_exponent - temp;
        }
        else {
            is_exponent_negative = true;
            exponent_abs = temp - raw_exponent;
        }
        
        size_t exponent_digits = 1;
        temp = exponent_abs;
        while((temp /= 10) > 0) exponent_digits++;
        
        size_t pure_width = 0;
        
        pure_width += 1;    // 1 first significand
        
        bool has_decimal_point = false;
        size_t index_after_first_none_zero = first_none_zero_index + 1;
        
        if(index_after_first_none_zero <= last_none_zero_index)
            has_decimal_point = true;
        else if(interprate->flag & printf_interprate_flag_complex)
            has_decimal_point = true;
        
        if(has_decimal_point) pure_width += 1;  // decimal point
        
#ifdef PRINTF_FLOATING_POINT_SPECIFIER_A_UNSPECIFIED_PRECISION_OUTPUT_NEEDED
        if(interprate->precision_type == printf_interprate_adjustment_unspecified) {
            size_t re_precision = 0;
            for(size_t index = index_after_first_none_zero; index <= last_none_zero_index; index += 4) {
#ifdef DEBUG
                size_t value = 0;
                if(significand_bit_data[index]) value += 2 * 2 * 2;         // 2^(3)
                if(index + 1 <= last_none_zero_index) {
                    if(significand_bit_data[index + 1]) value += 2 * 2;     // 2^(2)
                    if(index + 2 <= last_none_zero_index) {
                        if(significand_bit_data[index + 2]) value += 2;     // 2^(1)
                        if(index + 3 <= last_none_zero_index) {
                            if(significand_bit_data[index + 3]) value += 1; // 2^(0)
                        }
                    }
                }
                DEBUG_ASSERT(value < ARRAY_SIZE(base16_character_lowercase));
#endif
                re_precision++;
            }
            interprate->precision = re_precision;
        }
#endif
        pure_width += interprate->precision; // 1.xxxxxxx [x num]
        pure_width += 2;                     // p(+/-)
        pure_width += exponent_digits;       // p+xxx [x num]
        
        printf_specifier_a_pure_data_t pure_data_store;
        printf_specifier_a_pure_data_ref pure_data = &pure_data_store;
        pure_data->pure_width = pure_width;
        pure_data->significand_bit_data = significand_bit_data;
        pure_data->is_uppercase = is_uppercase;
        pure_data->has_decimal_point = has_decimal_point;
        pure_data->index_after_first_none_zero = index_after_first_none_zero;
        pure_data->last_none_zero_index = last_none_zero_index;
        pure_data->is_exponent_negative = is_exponent_negative;
        pure_data->exponent_abs = exponent_abs;
        pure_data->exponent_digits = exponent_digits;
        
        interprate->flag |= printf_interprate_flag_complex; // mark complex flag as default
        
        return printf_core_output_flag_width_help(core,
                                                  interprate,
                                                  pure_width,
                                                  is_uppercase ?
                                                  printf_complex_prefix_0X :
                                                  printf_complex_prefix_0x,
                                                  is_floating_negative,
                                                  printf_specifier_a_pure_function,
                                                  pure_data);
        
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_a_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_specifier_a_pure_data_ref pure_data = (printf_specifier_a_pure_data_ref)input;
        if(pure_data->significand_bit_data != NULL && pure_data->pure_width == pure_width) {
            printf_core_output_character(core, CHARACTER_1);
            if(pure_data->has_decimal_point) printf_core_output_character(core, CHARACTER_dot);
            bool * _Nonnull significand_bit_data = pure_data->significand_bit_data;
            bool is_uppercase = pure_data->is_uppercase;
            bool is_exponent_negative = pure_data->is_exponent_negative;
            size_t exponent_abs = pure_data->exponent_abs;
            size_t precision = interprate->precision;
            size_t index_after_first_none_zero = pure_data->index_after_first_none_zero;
            size_t last_none_zero_index = pure_data->last_none_zero_index;
            size_t exponent_digits = pure_data->exponent_digits;
            DEBUG_ASSERT(exponent_digits >= 1);
            for(size_t index = 0; index < precision; index++) {
                // index_after_first_none_zero + index * 4
                // index_after_first_none_zero + index * 4 + 1
                // index_after_first_none_zero + index * 4 + 2
                // index_after_first_none_zero + index * 4 + 3
                if(index_after_first_none_zero + index * 4 > last_none_zero_index)
                    printf_core_output_character(core, CHARACTER_0);
                else {
                    size_t value = 0;
                    if(significand_bit_data[index_after_first_none_zero + index * 4]) value += 2 * 2 * 2;         // 2^(3)
                    if(index_after_first_none_zero + index * 4 + 1 <= last_none_zero_index) {
                        if(significand_bit_data[index_after_first_none_zero + index * 4 + 1]) value += 2 * 2;     // 2^(2)
                        if(index_after_first_none_zero + index * 4 + 2 <= last_none_zero_index) {
                            if(significand_bit_data[index_after_first_none_zero + index * 4 + 2]) value += 2;     // 2^(1)
                            if(index_after_first_none_zero + index * 4 + 3 <= last_none_zero_index) {
                                if(significand_bit_data[index_after_first_none_zero + index * 4 + 3]) value += 1; // 2^(0)
                            }
                        }
                    }
                    DEBUG_ASSERT(value < ARRAY_SIZE(base16_character_lowercase));
                    printf_core_output_character(core,
                                                 is_uppercase ?
                                                 base16_character_uppercase[value]:
                                                 base16_character_lowercase[value]);
                }
            }
            printf_core_output_character(core,
                                         is_uppercase ?
                                         CHARACTER_P:
                                         CHARACTER_p);
            printf_core_output_character(core,
                                         is_exponent_negative ?
                                         CHARACTER_minus:
                                         CHARACTER_plus);
            DEBUG_ASSERT(UCHAR_MAX >= 9);
            unsigned char * exponent_store = printf_stack_alloc(sizeof(unsigned char) * exponent_digits);
            size_t index = 0;
            do exponent_store[index++] = exponent_abs % 10; while((exponent_abs /= 10) > 0);
            DEBUG_ASSERT(index == exponent_digits);
            for(size_t index = 0; index < exponent_digits; index++) {
                DEBUG_ASSERT(exponent_store[exponent_digits - index - 1] < ARRAY_SIZE(base10_character));
                DEBUG_ASSERT(exponent_digits - index - 1 < exponent_digits);
                printf_core_output_character(core, base10_character[exponent_store[exponent_digits - index - 1]]);
            }
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point_classification_normal(printf_core_ref _Nonnull core,
                                                                         printf_interprate_ref _Nonnull interprate,
                                                                         fp_info_ref _Nonnull info) {
    if(core != NULL && interprate != NULL && info != NULL && printf_specifier_type_is_floating_point_inline(interprate->type)) {
        
        if(interprate->type == printf_specifier_a || interprate->type == printf_specifier_A)
            return printf_output_specifier_floating_point_specifier_aA(core, interprate, info, true);
        else {
            fpbc_t storage;
            fpbc_ref fpbc = &storage;
            
            fpbc->sign = floating_point_info_query_sign_bit_inline(info);
            
            unsigned int exponent = floating_point_info_query_raw_exponent_inline(info);
            unsigned int exponent_base = floating_point_query_exponent_base(info);
            
            size_t need_for_query_significand_bit =
            floating_point_info_query_significand_bit(info, NULL, 0);
            
            // most impossible condition
            if(need_for_query_significand_bit * sizeof(bool) > PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
            
            bool *significand_bit_data = printf_stack_alloc(need_for_query_significand_bit * sizeof(bool));
            floating_point_info_query_significand_bit(info, significand_bit_data, need_for_query_significand_bit);
            
            bool nearby_down_decrease_exponent = true;
            for(size_t index = 0; index < need_for_query_significand_bit; index++)
                if(significand_bit_data[index]) {
                    nearby_down_decrease_exponent = false;
                    break;
                }
            
            // bit for significand (implicit + explicit) == significand_bit + 1
            // e - p == exponent - exponent_base - need_for_query_significand_bit
            
            int e_minus_p;
            if(__builtin_sub_overflow(exponent, exponent_base, &e_minus_p)) DEBUG_RETURN(false);
            if(__builtin_sub_overflow(e_minus_p, need_for_query_significand_bit, &e_minus_p)) DEBUG_RETURN(false);
            
            // [Document] GNU other builtins
            // The lifetime of the allocated object ends just before the calling function returns to its caller.
            // This is so even when __builtin_alloca is called within a nested block.
            if(e_minus_p >= 0) {
                
                // [1] could just store significand(with implicit one) offseted (e - p)
                size_t bit_length = need_for_query_significand_bit + 1 + e_minus_p;
                
                // [2] nearby_down_decrease_exponent
                if(nearby_down_decrease_exponent) bit_length++;
                
                // [3] remain less than scale [impossible for e-p positive, ignored]
                
                // [4] remain * 2 + nearby_up larger or equal to scale * 2
                //      (1) scale by two always need one more
                bit_length += 1;
                //      (2) plus nearby_up never need one more
                //      (3) scale is scaled by base to more than this need 4 more
                bit_length += 4;
                
                // [5] calculation remain multi base [impossible for scale is scaled by B need 4 more]
                
                size_t length = EXT_ARRAY_SIZE_FOR_BIT(bit_length);
                fpbc->remain = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->scale = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_down = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_up = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base_value = UINT32_C(10);
                fpbc->length = length;
                fpbc->exponent = 0;         // not much useful as decided in simple fixup
                
                extended_integer_initialize(fpbc->remain, length);
                extended_integer_initialize(fpbc->scale, length);
                extended_integer_initialize(fpbc->nearby_down, length);
                extended_integer_initialize(fpbc->nearby_up, length);
                extended_integer_initialize(fpbc->base, length);
                
                // [base] 1010
                DEBUG_ASSERT(bit_length >= 4);
                extended_integer_set_bit(fpbc->base, length, 1, true);
                extended_integer_set_bit(fpbc->base, length, 3, true);
                
                // [remain]
                for(size_t index = 0; index < need_for_query_significand_bit; index++) {
                    // bit_index = e_minus_p + index
                    // bit_value = significand_bit_data[need_for_query_significand_bit - index - 1]
                    size_t bit_index = e_minus_p + index;
                    DEBUG_ASSERT(!__builtin_add_overflow(e_minus_p, index, &bit_index));
                    bool bit_value = significand_bit_data[need_for_query_significand_bit - index - 1];
                    extended_integer_set_bit(fpbc->remain, length, bit_index, bit_value);
                }
                
                // [remain] implicit to one
                extended_integer_set_bit(fpbc->remain, length, e_minus_p + need_for_query_significand_bit, true);
                
                // [scale] setted to one
                extended_integer_set_bit(fpbc->scale, length, 0, true);
                
                // floating_value = Remain / Scale = Significand x Base (Exponent - Position)
                
                // [nearby_down]
                extended_integer_set_bit(fpbc->nearby_down, length, e_minus_p, true);
                
                // [nearby up]
                extended_integer_set_bit(fpbc->nearby_up, length, e_minus_p, true);
            }
            else {
                int p_minus_e = - e_minus_p;
                bool is_remain_less_than_scale;
                size_t bit_length;
                
                // [1] max[ (need_for_query_significand_bit + 1), p_minus_e + 1 ]
                if(need_for_query_significand_bit + 1 > p_minus_e + 1)
                     is_remain_less_than_scale = true;
                else is_remain_less_than_scale = false;
                
                if(is_remain_less_than_scale)
                     bit_length = need_for_query_significand_bit + 1;
                else bit_length = p_minus_e + 1;
                
                // [2] nearby_down_decrease_exponent
                if(nearby_down_decrease_exponent) bit_length++;
                
                // [3] remain less than scale
                // [4] remain * 2 + nearby_up larger or equal to scale * 2
                //      (1) scale by two always need one more
                bit_length += 1;
                //      (2) plus nearby_up never need one more
                //      (3) scale is scaled by base to more than this need 4 more
                
                // [3] & [4]-(3) we got this value
                bit_length += 4;
                
                // [5] calculation remain multi base [impossible for scale is scaled by B need 4 more]
                bit_length += 4;
                
                size_t length = EXT_ARRAY_SIZE_FOR_BIT(bit_length);
                fpbc->remain = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->scale = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_down = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_up = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base_value = UINT32_C(10);
                fpbc->length = length;
                fpbc->exponent = 0;         // not much useful as decided in simple fixup
                
                extended_integer_initialize(fpbc->remain, length);
                extended_integer_initialize(fpbc->scale, length);
                extended_integer_initialize(fpbc->nearby_down, length);
                extended_integer_initialize(fpbc->nearby_up, length);
                extended_integer_initialize(fpbc->base, length);
                
                // [base] 1010 [value(decimal): 10]
                DEBUG_ASSERT(bit_length >= 4);
                extended_integer_set_bit(fpbc->base, length, 1, true);
                extended_integer_set_bit(fpbc->base, length, 3, true);
                
                // [remain]
                for(size_t index = 0; index < need_for_query_significand_bit; index++) {
                    // bit_index = index
                    // bit_value = significand_bit_data[need_for_query_significand_bit - index - 1]
                    size_t bit_index = index;
                    bool bit_value = significand_bit_data[need_for_query_significand_bit - index - 1];
                    extended_integer_set_bit(fpbc->remain, length, bit_index, bit_value);
                }
                
                // [remain] implicit to one
                extended_integer_set_bit(fpbc->remain, length, need_for_query_significand_bit, true);
                
                // [scale] setted to one
                extended_integer_set_bit(fpbc->scale, length, p_minus_e, true);
                
                // floating_value = Remain / Scale = Significand x Base (Exponent - Position)
                
                // [nearby_down]
                extended_integer_set_bit(fpbc->nearby_down, length, 0, true);
                
                // [nearby up]
                extended_integer_set_bit(fpbc->nearby_up, length, 0, true);
            }
            
            if(!printf_output_specifier_floating_point_fpbc_simple_fixup(info, fpbc, nearby_down_decrease_exponent))
                DEBUG_RETURN(false);
            
            switch (interprate->type) {
                case printf_specifier_f:
                case printf_specifier_F:
                    return printf_output_specifier_floating_point_specifier_fF(core, interprate, info, fpbc);
                case printf_specifier_e:
                case printf_specifier_E:
                    return printf_output_specifier_floating_point_specifier_eE(core, interprate, info, fpbc);
                case printf_specifier_g:
                case printf_specifier_G:
                    return printf_output_specifier_floating_point_specifier_gG(core, interprate, info, fpbc);
                default: DEBUG_RETURN(false);
            }
        }
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point_classification_subnormal(printf_core_ref _Nonnull core,
                                                                            printf_interprate_ref _Nonnull interprate,
                                                                            fp_info_ref _Nonnull info) {
    if(core != NULL && interprate != NULL && info != NULL && printf_specifier_type_is_floating_point_inline(interprate->type)) {
        
        if(interprate->type == printf_specifier_a || interprate->type == printf_specifier_A)
            return printf_output_specifier_floating_point_specifier_aA(core, interprate, info, false);
        else {
            fpbc_t storage;
            fpbc_ref fpbc = &storage;
            
            fpbc->sign = floating_point_info_query_sign_bit_inline(info);
            
            unsigned int exponent = floating_point_info_query_raw_exponent_inline(info);
            unsigned int exponent_base = floating_point_query_exponent_base(info);
            
            size_t need_for_query_significand_bit =
            floating_point_info_query_significand_bit(info, NULL, 0);
            
            // most impossible condition
            if(need_for_query_significand_bit * sizeof(bool) > PRINTF_STACK_ALLOCATION_LIMIT) DEBUG_RETURN(false);
            
            bool *significand_bit_data = printf_stack_alloc(need_for_query_significand_bit * sizeof(bool));
            floating_point_info_query_significand_bit(info, significand_bit_data, need_for_query_significand_bit);
            
            // bit for significand (implicit + explicit) == significand_bit + 1
            // e - p == exponent - exponent_base - need_for_query_significand_bit
            
            bool all_zero_significand_bit = true;
            for(size_t index = 0; index < need_for_query_significand_bit; index++)
                if(significand_bit_data[index]) {
                    all_zero_significand_bit = false;
                    break;
                }
            if(all_zero_significand_bit) DEBUG_RETURN(false);
            
            int e_minus_p;
            if(__builtin_sub_overflow(exponent, exponent_base, &e_minus_p)) DEBUG_RETURN(false);
            if(__builtin_sub_overflow(e_minus_p, need_for_query_significand_bit, &e_minus_p)) DEBUG_RETURN(false);
            
            DEBUG_ASSERT(e_minus_p < 0); // once not possible ??? [with IEE 754 implementation]
            
            // [Document] GNU other builtins
            // The lifetime of the allocated object ends just before the calling function returns to its caller.
            // This is so even when __builtin_alloca is called within a nested block.
            if(e_minus_p >= 0) {
                // unreachable code [currently]
                
                // [1] could just store significand(without implicit one) offseted (e - p)
                size_t bit_length = need_for_query_significand_bit + e_minus_p;
                
                // [2] nearby_down_decrease_exponent [never possible]
                
                // [3] remain less than scale [impossible for e-p positive, ignored]
                
                // [4] remain * 2 + nearby_up larger or equal to scale * 2
                //      (1) scale by two always need one more
                bit_length += 1;
                //      (2) plus nearby_up never need one more
                //      (3) scale is scaled by base to more than this need 4 more
                bit_length += 4;
                
                // [5] calculation remain multi base [impossible for scale is scaled by B need 4 more]
                bit_length += 4;
                
                size_t length = EXT_ARRAY_SIZE_FOR_BIT(bit_length);
                fpbc->remain = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->scale = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_down = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_up = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base_value = UINT32_C(10);
                fpbc->length = length;
                fpbc->exponent = 0;         // not much useful as decided in simple fixup
                
                extended_integer_initialize(fpbc->remain, length);
                extended_integer_initialize(fpbc->scale, length);
                extended_integer_initialize(fpbc->nearby_down, length);
                extended_integer_initialize(fpbc->nearby_up, length);
                extended_integer_initialize(fpbc->base, length);
                
                // [base] 1010
                DEBUG_ASSERT(bit_length >= 4);
                extended_integer_set_bit(fpbc->base, length, 1, true);
                extended_integer_set_bit(fpbc->base, length, 3, true);
                
// [NOTE] [optimization] subnormal floating point without implicit one may need less space
                
                // [remain]
                for(size_t index = 0; index < need_for_query_significand_bit; index++) {
                    // bit_index = e_minus_p + index
                    // bit_value = significand_bit_data[need_for_query_significand_bit - index - 1]
                    size_t bit_index = e_minus_p + index;
                    DEBUG_ASSERT(!__builtin_add_overflow(e_minus_p, index, &bit_index));
                    bool bit_value = significand_bit_data[need_for_query_significand_bit - index - 1];
                    extended_integer_set_bit(fpbc->remain, length, bit_index, bit_value);
                }
                
                // [scale] setted to one
                extended_integer_set_bit(fpbc->scale, length, 0, true);
                
                // floating_value = Remain / Scale = Significand x Base (Exponent - Position)
                
                // [nearby_down]
                extended_integer_set_bit(fpbc->nearby_down, length, e_minus_p, true);
                
                // [nearby up]
                extended_integer_set_bit(fpbc->nearby_up, length, e_minus_p, true);
            }
            else {
                int p_minus_e = - e_minus_p;
                bool is_remain_less_than_scale;
                size_t bit_length;

                // [1] max[ (need_for_query_significand_bit), p_minus_e + 1 ]
                if(need_for_query_significand_bit > p_minus_e + 1)
                     is_remain_less_than_scale = true;
                else is_remain_less_than_scale = false;

                if(is_remain_less_than_scale)
                     bit_length = need_for_query_significand_bit;
                else bit_length = p_minus_e + 1;
                
                // [2] nearby_down_decrease_exponent [impossible]
                
                // [3] remain less than scale [may happen]
                // [4] remain * 2 + nearby_up larger or equal to scale * 2
                //      (1) scale by two always need one more
                bit_length += 1;
                //      (2) plus nearby_up never need one more
                //      (3) scale is scaled by base to more than this need 4 more
                
                // [3] & [4]-(3)
                bit_length += 4;
                
                // [5] calculation remain multi base [impossible for scale is scaled by B need 4 more]
                bit_length += 4;
                
// [NOTE] [optimization] need_for_query_significand_bit first non-zero position need less space
                
                size_t length = EXT_ARRAY_SIZE_FOR_BIT(bit_length);
                fpbc->remain = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->scale = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_down = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->nearby_up = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base = printf_stack_alloc(sizeof(EXT_TYPE) * length);
                fpbc->base_value = UINT32_C(10);
                fpbc->length = length;
                fpbc->exponent = 0;         // not much useful as decided in simple fixup
                
                extended_integer_initialize(fpbc->remain, length);
                extended_integer_initialize(fpbc->scale, length);
                extended_integer_initialize(fpbc->nearby_down, length);
                extended_integer_initialize(fpbc->nearby_up, length);
                extended_integer_initialize(fpbc->base, length);
                
                // [base] 1010
                DEBUG_ASSERT(bit_length >= 4);
                extended_integer_set_bit(fpbc->base, length, 1, true);
                extended_integer_set_bit(fpbc->base, length, 3, true);
                
                // [remain]
                for(size_t index = 0; index < need_for_query_significand_bit; index++) {
                    // bit_index = index
                    // bit_value = significand_bit_data[need_for_query_significand_bit - index - 1]
                    size_t bit_index = index;
                    bool bit_value = significand_bit_data[need_for_query_significand_bit - index - 1];
                    extended_integer_set_bit(fpbc->remain, length, bit_index, bit_value);
                }
                
                // [scale] setted to one
                extended_integer_set_bit(fpbc->scale, length, p_minus_e, true);
                
                // floating_value = Remain / Scale = Significand x Base (Exponent - Position)
                
                // [nearby_down]
                extended_integer_set_bit(fpbc->nearby_down, length, 0, true);
                
                // [nearby up]
                extended_integer_set_bit(fpbc->nearby_up, length, 0, true);
            }
            
            if(!printf_output_specifier_floating_point_fpbc_simple_fixup(info, fpbc, false))
                DEBUG_RETURN(false);
            
            switch (interprate->type) {
                case printf_specifier_f:
                case printf_specifier_F:
                    return printf_output_specifier_floating_point_specifier_fF(core, interprate, info, fpbc);
                case printf_specifier_e:
                case printf_specifier_E:
                    return printf_output_specifier_floating_point_specifier_eE(core, interprate, info, fpbc);
                case printf_specifier_g:
                case printf_specifier_G:
                    return printf_output_specifier_floating_point_specifier_gG(core, interprate, info, fpbc);
                default: DEBUG_RETURN(false);
            }
        }
    } DEBUG_ELSE
    return false;
}

static bool printf_output_specifier_floating_point(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    if(core != NULL && interprate != NULL && printf_specifier_type_is_floating_point_inline(interprate->type)) {
        
        fp_info printf_info_storage;
        fp_info_ref info = &printf_info_storage;
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_after_input);
        if(interprate->width_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
            interprate->width_type = printf_interprate_adjustment_after_input;
        }
        
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_after_input);
        if(interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
            interprate->precision_type = printf_interprate_adjustment_after_input;
        }
        
        switch (interprate->length_type) {
            case printf_interprate_length_none:     // double       (64)
            case printf_interprate_length_l: {      // double       (64)
                fp64_t value = va_arg(core->args, double);
                if(!floating_point_intialize_64bit_inline(info, value)) DEBUG_RETURN(false);
            }   break;
            case printf_interprate_length_L: {
                fp64_t value = va_arg(core->args, long double);
                if(!floating_point_intialize_64bit_inline(info, value)) DEBUG_RETURN(false);
            }   break;
            default: DEBUG_RETURN(false);
        }
        
        fp_classification classification = floating_point_info_classification(info);
        switch (classification) {
            case fp_classification_zero:
                return printf_output_specifier_floating_point_zero(core, interprate, info);
            case fp_classification_nan:
                return printf_output_specifier_floating_point_nan(core, interprate, info);
            case fp_classification_infinite:
                return printf_output_specifier_floating_point_inf(core, interprate, info);
            case fp_classification_normal:
                return printf_output_specifier_floating_point_classification_normal(core, interprate, info);
            case fp_classification_subnormal:
                return printf_output_specifier_floating_point_classification_subnormal(core, interprate, info);
            default: DEBUG_RETURN(false);
        }
        
    } DEBUG_ELSE
    return false;
}

#pragma mark character type [c]

typedef struct printf_specifier_c_pure_data {
    UTF8Char * _Nonnull array;
    size_t array_size;
} printf_specifier_c_pure_data_t;

typedef printf_specifier_c_pure_data_t * printf_specifier_c_pure_data_ref;

static bool printf_specifier_c_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input);

static bool printf_output_specifier_character(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    if(core != NULL && interprate != NULL) {
        DEBUG_ASSERT(interprate->type == printf_specifier_c);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_after_input);
        if(interprate->width_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
            interprate->width_type = printf_interprate_adjustment_after_input;
        }
        
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_after_input);
        if(interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            DEVELOP_BREAKPOINT_RETURN(false);
            // [BREAKPOINT] precision is not valid for %c type
        }
        
        UTF8Char array[PRINTF_WCHAR_TO_UTF8CHAR_ARRAY_LENGTH_MAX];
        size_t array_size = ARRAY_SIZE(array);
        
        if(interprate->length_type == printf_interprate_length_l) {
            wint_t wch = va_arg(core->args, wint_t);
            if(wch == WEOF) return false;
            if(wch >= 0) {
                if(printf_wchar_to_UTF8Char(wch, array, &array_size))
                    DEBUG_RETURN(false);
            } else return false;
        }
        else if(interprate->length_type == printf_interprate_length_none) {
            int ch = va_arg(core->args, int);
            if(ch == EOF) return false;
            if(ch >= 0 && ch <= PRINTF_CHAR_VALUE_MAX) {
                wchar_t wch = ch;
                if(printf_wchar_to_UTF8Char(wch, array, &array_size))
                    DEBUG_RETURN(false);
            } else return false;
        } ELSE_DEBUG_RETURN(false);
        
        DEBUG_ASSERT(array_size > 0 &&
                     array_size <= PRINTF_WCHAR_TO_UTF8CHAR_ARRAY_LENGTH_MAX);
        
        printf_interprate_flag valid_flag = printf_interprate_flag_left_justify;
        interprate->flag &= valid_flag;
        
        printf_specifier_c_pure_data_t pure_data_store;
        printf_specifier_c_pure_data_ref pure_data = &pure_data_store;
        pure_data->array = array;
        pure_data->array_size = array_size;
        
        return printf_core_output_flag_width_help(core,
                                                  interprate,
                                                  array_size,
                                                  printf_complex_prefix_none,
                                                  false,
                                                  printf_specifier_c_pure_function,
                                                  pure_data);
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_c_pure_function(printf_core_ref _Nonnull core,
                                             printf_interprate_ref _Nonnull interprate,
                                             size_t pure_width,
                                             void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_specifier_c_pure_data_ref pure_data = (printf_specifier_c_pure_data_ref)input;
        DEBUG_ASSERT(pure_data->array_size == pure_width);
        if(pure_data->array != NULL && pure_data->array_size > 0) {
            printf_core_output_raw_string(core, pure_data->array, pure_data->array_size);
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

#pragma mark string type [s]

static UTF8Char __printf_null_string_UTF8__[] =
{CHARACTER_left_brace, CHARACTER_n, CHARACTER_u, CHARACTER_l, CHARACTER_l, CHARACTER_right_brace, CHARACTER_null};
static UTF16Char __printf_null_string_UTF16__[] =
{CHARACTER_left_brace, CHARACTER_n, CHARACTER_u, CHARACTER_l, CHARACTER_l, CHARACTER_right_brace, CHARACTER_null};
static UTF32Char __printf_null_string_UTF32__[] =
{CHARACTER_left_brace, CHARACTER_n, CHARACTER_u, CHARACTER_l, CHARACTER_l, CHARACTER_right_brace, CHARACTER_null};

typedef struct printf_specifier_s_UTF8_pure_data {
    UTF8Str _Nonnull str;
    size_t * _Nullable parsing_width;
    size_t pure_width;
} printf_specifier_s_UTF8_pure_data_t;

typedef printf_specifier_s_UTF8_pure_data_t * printf_specifier_s_UTF8_pure_data_ref;

typedef struct printf_specifier_s_UTF16_pure_data {
    UTF16Str _Nonnull str;
    size_t * _Nullable parsing_width;
    size_t pure_width;
} printf_specifier_s_UTF16_pure_data_t;

typedef printf_specifier_s_UTF16_pure_data_t * printf_specifier_s_UTF16_pure_data_ref;

typedef struct printf_specifier_s_UTF32_pure_data {
    UTF32Str _Nonnull str;
    size_t * _Nullable parsing_width;
    size_t pure_width;
} printf_specifier_s_UTF32_pure_data_t;

typedef printf_specifier_s_UTF32_pure_data_t * printf_specifier_s_UTF32_pure_data_ref;

typedef struct printf_specifier_s_UTF8_character_output_input {
    printf_core_ref _Nonnull core;
} printf_specifier_s_UTF8_character_output_t;

typedef printf_specifier_s_UTF8_character_output_t * printf_specifier_s_UTF8_character_output_ref;

static bool printf_specifier_s_UTF16_pure_function(printf_core_ref _Nonnull core,
                                                   printf_interprate_ref _Nonnull interprate,
                                                   size_t pure_width,
                                                   void * _Nonnull input);

static bool printf_specifier_s_UTF32_pure_function(printf_core_ref _Nonnull core,
                                                   printf_interprate_ref _Nonnull interprate,
                                                   size_t pure_width,
                                                   void * _Nonnull input);

static bool printf_specifier_s_UTF8_pure_function(printf_core_ref _Nonnull core,
                                                  printf_interprate_ref _Nonnull interprate,
                                                  size_t pure_width,
                                                  void * _Nonnull input);

static bool printf_specifier_s_UTF8_character_output(UTF8Char * _Nonnull array,
                                                     size_t array_size,
                                                     void * _Nonnull input);

static bool printf_output_specifier_string(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    if(core != NULL && interprate != NULL) {
        DEBUG_ASSERT(interprate->type == printf_specifier_s);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_after_input);
        if(interprate->width_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
            interprate->width_type = printf_interprate_adjustment_after_input;
        }
        
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_after_input);
        if(interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            interprate->width = va_arg(core->args, int);
            interprate->precision_type = printf_interprate_adjustment_after_input;
        }
        
        printf_interprate_flag flag_mask = printf_interprate_flag_left_justify;
        interprate->flag &= flag_mask;
        
        size_t width; size_t * parsing_width = NULL;
        if(interprate->precision_type == printf_interprate_adjustment_fix_number ||
           interprate->precision_type == printf_interprate_adjustment_after_input) {
            width = interprate->precision;
            parsing_width = &width;
        }
        
        if(interprate->length_type == printf_interprate_length_l) {
            wchar_t * _Nullable ws = va_arg(core->args, wchar_t *);
#ifdef PRINTF_ENABLE_NULL_STRING_OUTPUT
            if(ws == NULL) {
                if(sizeof(wchar_t) == sizeof(UTF16Char)) ws = (wchar_t *)__printf_null_string_UTF16__;
                else if(sizeof(wchar_t) == sizeof(UTF32Char)) ws = (wchar_t *)__printf_null_string_UTF32__;
                ELSE_DEBUG_RETURN(false);
            }
#endif
            if(ws != NULL) {
                COMILE_ASSERT(sizeof(wchar_t) == sizeof(UTF16Char) || sizeof(wchar_t) == sizeof(UTF32Char));
                if(sizeof(wchar_t) == sizeof(UTF16Char)) {
                    // consider wchar_t as USC-2 || UTF-16 encoding
                    UTF16Str str = (UTF16Str)ws;
                    size_t pure_width;
                    if(printf_UTF16Str_to_UTF8Char(str, NULL, NULL, parsing_width, &pure_width, NULL)) {
                        
                        printf_specifier_s_UTF16_pure_data_t pure_data_store;
                        printf_specifier_s_UTF16_pure_data_ref pure_data = &pure_data_store;
                        
                        pure_data->str = str;
                        pure_data->parsing_width = parsing_width;
                        pure_data->pure_width = pure_width;
                        
                        return printf_core_output_flag_width_help(core,
                                                                  interprate,
                                                                  pure_width,
                                                                  printf_complex_prefix_none,
                                                                  false,
                                                                  printf_specifier_s_UTF16_pure_function,
                                                                  pure_data);
                    } DEVELOP_ELSE_BREAKPOINT
                }
                else if(sizeof(wchar_t) == sizeof(UTF32Char)) {
                    // consider wchar_t as USC-4 || UTF-32 encoding
                    UTF32Str str = (UTF32Str)ws;
                    size_t pure_width;
                    if(printf_UTF32Str_to_UTF8Char(str, NULL, NULL, parsing_width, &pure_width, NULL)) {
                        
                        printf_specifier_s_UTF32_pure_data_t pure_data_store;
                        printf_specifier_s_UTF32_pure_data_ref pure_data = &pure_data_store;
                        
                        pure_data->str = str;
                        pure_data->parsing_width = parsing_width;
                        pure_data->pure_width = pure_width;
                        
                        return printf_core_output_flag_width_help(core,
                                                                  interprate,
                                                                  pure_width,
                                                                  printf_complex_prefix_none,
                                                                  false,
                                                                  printf_specifier_s_UTF32_pure_function,
                                                                  pure_data);
                    } DEVELOP_ELSE_BREAKPOINT
                    
                } ELSE_DEBUG_RETURN(false);     // this should never happen
            }
        }
        else if(interprate->length_type == printf_interprate_length_none) {
            char *s = va_arg(core->args, char *);
#ifdef PRINTF_ENABLE_NULL_STRING_OUTPUT
            if(s == NULL) s = (char *)__printf_null_string_UTF8__;
#endif
            if(s != NULL) {
                UTF8Char *str = (UTF8Char *)s;
                UTF8Char *current = str;
                size_t pure_width = 0;
                while(current[0] != UTF8_C(0)) {
                    if(parsing_width != NULL && pure_width + 1 > parsing_width[0])
                        break;
                    size_t skip_length;
                    if(!printf_UTF8_character_validate(current, &skip_length)) DEVELOP_BREAKPOINT_RETURN(false);
                    DEBUG_ASSERT(skip_length <= 100);        // just a unreachable check mark
                    pure_width += 1;
                    current += skip_length;
                }
                
                printf_specifier_s_UTF8_pure_data_t pure_data_store;
                printf_specifier_s_UTF8_pure_data_ref pure_data = &pure_data_store;
                
                pure_data->str = str;
                pure_data->parsing_width = parsing_width;
                pure_data->pure_width = pure_width;
                
                return printf_core_output_flag_width_help(core,
                                                          interprate,
                                                          pure_width,
                                                          printf_complex_prefix_none,
                                                          false,
                                                          printf_specifier_s_UTF8_pure_function,
                                                          pure_data);
            }
        } ELSE_DEVELOP_BREAKPOINT_RETURN(false);
    }
    return false;
}

static bool printf_specifier_s_UTF16_pure_function(printf_core_ref _Nonnull core,
                                                   printf_interprate_ref _Nonnull interprate,
                                                   size_t pure_width,
                                                   void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_specifier_s_UTF16_pure_data_ref pure_data = (printf_specifier_s_UTF16_pure_data_ref)input;
        DEBUG_ASSERT(pure_data->pure_width == pure_width);
        if(pure_data->str != NULL) {
            printf_specifier_s_UTF8_character_output_t input_store;
            printf_specifier_s_UTF8_character_output_ref input = &input_store;
            input->core = core;
            
            return printf_UTF16Str_to_UTF8Char(pure_data->str,
                                               printf_specifier_s_UTF8_character_output,
                                               input,
                                               pure_data->parsing_width,
                                               NULL,
                                               NULL);
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_s_UTF32_pure_function(printf_core_ref _Nonnull core,
                                                   printf_interprate_ref _Nonnull interprate,
                                                   size_t pure_width,
                                                   void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_specifier_s_UTF32_pure_data_ref pure_data = (printf_specifier_s_UTF32_pure_data_ref)input;
        DEBUG_ASSERT(pure_data->pure_width == pure_width);
        if(pure_data->str != NULL) {
            printf_specifier_s_UTF8_character_output_t input_store;
            printf_specifier_s_UTF8_character_output_ref input = &input_store;
            input->core = core;
            
            return printf_UTF32Str_to_UTF8Char(pure_data->str,
                                               printf_specifier_s_UTF8_character_output,
                                               input,
                                               pure_data->parsing_width,
                                               NULL,
                                               NULL);
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_s_UTF8_pure_function(printf_core_ref _Nonnull core,
                                                  printf_interprate_ref _Nonnull interprate,
                                                  size_t pure_width,
                                                  void * _Nonnull input) {
    if(core != NULL && interprate != NULL && input != NULL) {
        printf_specifier_s_UTF8_pure_data_ref pure_data = (printf_specifier_s_UTF8_pure_data_ref)input;
        DEBUG_ASSERT(pure_data->pure_width == pure_width);
        if(pure_data->str != NULL) {
            
            UTF8Char *current = pure_data->str;
            size_t internal_pure_width = 0;
            while(current[0] != UTF8_C(0)) {
                if(pure_data->parsing_width != NULL && internal_pure_width + 1 > pure_data->parsing_width[0])
                    break;
                size_t skip_length;
                if(printf_UTF8_character_validate(current, &skip_length)) {
                    printf_core_output_raw_string(core, current, skip_length);
                } ELSE_DEBUG_RETURN(false);
                internal_pure_width += 1;
                current += skip_length;
            }
            DEBUG_ASSERT(internal_pure_width == pure_width);
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_s_UTF8_character_output(UTF8Char * _Nonnull array, size_t array_size, void * _Nonnull input) {
    if(array != NULL && array_size > 0 && input != NULL) {
        printf_specifier_s_UTF8_character_output_ref data = (printf_specifier_s_UTF8_character_output_ref)input;
        if(data->core != NULL) {
            printf_core_output_raw_string(data->core, array, array_size);
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

#pragma mark pointer type [p]

static bool printf_output_specifier_pointer(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    COMILE_ASSERT(sizeof(void *) == sizeof(unsigned short) ||
                  sizeof(void *) == sizeof(unsigned int) ||
                  sizeof(void *) == sizeof(unsigned long) ||
                  sizeof(void *) == sizeof(unsigned long long));
    if(core != NULL && interprate != NULL) {
        DEBUG_ASSERT(interprate->type == printf_specifier_p);
        
        // [NOTE] not reading the width info
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_after_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_after_input);
        if(interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            // undefined behavior
            DEVELOP_BREAKPOINT_RETURN(false);
        }
        if(interprate->length_type != printf_interprate_length_none) {
            // undefined behavior
            DEVELOP_BREAKPOINT_RETURN(false);
        }
        
        printf_interprate_flag flag_mask = printf_interprate_flag_left_justify;
        interprate->flag &= flag_mask;
        
        if(sizeof(void *) == sizeof(unsigned short))
            interprate->length_type = printf_interprate_length_h;
        else if(sizeof(void *) == sizeof(unsigned int))
            interprate->length_type = printf_interprate_length_none;
        else if(sizeof(void *) == sizeof(unsigned long))
            interprate->length_type = printf_interprate_length_l;
        else if(sizeof(void *) == sizeof(unsigned long long))
            interprate->length_type = printf_interprate_length_ll;
        ELSE_DEBUG_RETURN(false);       // should never happen
        
        // output as format x
        interprate->type = printf_specifier_x;
        
        // force to mark complex flag
        interprate->flag |= printf_interprate_flag_complex;
        
        return printf_output_specifier_integer_iduoxX(core, interprate);
    } DEBUG_ELSE
    return false;
}

#pragma mark count type [n]

static bool printf_output_specifier_count(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    if(core != NULL && interprate != NULL) {
        DEBUG_ASSERT(interprate->type == printf_specifier_n);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_after_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_after_input);
        if(interprate->width_type == printf_interprate_adjustment_waiting_input ||
           interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            // undefined behavior
            DEVELOP_BREAKPOINT_RETURN(false);
        }
        
        size_t already_stored = 0;
        switch (core->type) {
            case printf_type_string: {
                if(core->output.string.store == NULL) already_stored = 0;
                else already_stored = core->output.string.index;
            }   break;
            case printf_type_FILE:
                already_stored = core->actual_need;
                break;
            default: DEBUG_RETURN(false);
        }
        
        if(interprate->length_type == printf_interprate_length_none) {
            int *pointer = va_arg(core->args, int *);
            if(pointer != NULL) pointer[0] = (int)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        else if(interprate->length_type == printf_interprate_length_hh) {
            signed char *pointer = va_arg(core->args, signed char *);
            if(pointer != NULL) pointer[0] = (signed char)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        else if(interprate->length_type == printf_interprate_length_h) {
            short int *pointer = va_arg(core->args, short int *);
            if(pointer != NULL) pointer[0] = (short int)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        else if(interprate->length_type == printf_interprate_length_l) {
            long int *pointer = va_arg(core->args, long int *);
            if(pointer != NULL) pointer[0] = (long int)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        else if(interprate->length_type == printf_interprate_length_ll) {
            long long int *pointer = va_arg(core->args, long long int *);
            if(pointer != NULL) pointer[0] = (long long int)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        else if(interprate->length_type == printf_interprate_length_j) {
            intmax_t *pointer = va_arg(core->args, intmax_t *);
            if(pointer != NULL) pointer[0] = (intmax_t)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        else if(interprate->length_type == printf_interprate_length_z) {
            size_t *pointer = va_arg(core->args, size_t *);
            if(pointer != NULL) pointer[0] = (size_t)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        else if(interprate->length_type == printf_interprate_length_t) {
            ptrdiff_t *pointer = va_arg(core->args, ptrdiff_t *);
            if(pointer != NULL) pointer[0] = (ptrdiff_t)already_stored;
            DEVELOP_ELSE_BREAKPOINT
        }
        ELSE_DEVELOP_BREAKPOINT_RETURN(false);
        
        return true;
        
    } DEBUG_ELSE
    return false;
}

#pragma mark percentage type [%]

static bool printf_specifier_percentage_pure_function(printf_core_ref _Nonnull core,
                                                      printf_interprate_ref _Nonnull interprate,
                                                      size_t pure_width,
                                                      void * _Nonnull input);

static bool printf_output_specifier_percentage(printf_core_ref _Nonnull core, printf_interprate_ref _Nonnull interprate) {
    if(core != NULL && interprate != NULL) {
        DEBUG_ASSERT(interprate->type == printf_specifier_percentage);
        
        DEBUG_ASSERT(interprate->width_type != printf_interprate_adjustment_after_input);
        DEBUG_ASSERT(interprate->precision_type != printf_interprate_adjustment_after_input);
        if(interprate->width_type == printf_interprate_adjustment_waiting_input ||
           interprate->precision_type == printf_interprate_adjustment_waiting_input) {
            // undefined behavior
            DEVELOP_BREAKPOINT_RETURN(false);
        }
        
        printf_interprate_flag flag_mask = printf_interprate_flag_left_justify;
        interprate->flag &= flag_mask;
        
        printf_core_output_flag_width_help(core,
                                           interprate,
                                           1,
                                           printf_complex_prefix_none,
                                           false,
                                           printf_specifier_percentage_pure_function,
                                           core);       // not much useful
        
        return true;
        
    } DEBUG_ELSE
    return false;
}

static bool printf_specifier_percentage_pure_function(printf_core_ref _Nonnull core,
                                                      printf_interprate_ref _Nonnull interprate,
                                                      size_t pure_width,
                                                      void * _Nonnull input) {
    if(core != NULL && interprate != NULL && pure_width == 1) {
        printf_core_output_character(core, CHARACTER_percentage);
        return true;
    } DEBUG_ELSE
    return false;
}

#pragma mark - printf_internal

int printf_internal(printf_core_ref _Nonnull core) {
    if(!printf_core_validate(core))
        return PRINTF_ERROR_RETURN;
    if(core->format == NULL || !printf_UTF8Str_validate(core->format, NULL))
        return PRINTF_ERROR_RETURN;
    
    printf_core_output_initialize_inline(core);
    
    bool validate_check = true;
    
    while(core->format[0] != CHARACTER_null) {      // output loop
        const UTF8Char *next_escape = printf_strchr_inline(core->format, CHARACTER_percentage);
        if(next_escape == NULL) {
            size_t format_length = printf_strlen_inline(core->format);
            printf_core_output_raw_string(core, core->format, format_length);
            // core->format += format_length;
            // DEBUG_ASSERT(core->format[0] == CHARACTER_null);
            goto LABEL_validate_failed_exit_point;
        }
        else {
            DEBUG_ASSERT(next_escape >= core->format);
            if(next_escape > core->format) {
                size_t before_escape = next_escape - core->format;
                printf_core_output_raw_string(core, core->format, before_escape);
                core->format += before_escape;
                DEBUG_ASSERT(core->format[0] == CHARACTER_percentage);
            }
            DEBUG_ASSERT(core->format == next_escape);
            // currently core->format point to the escape character
            printf_interprate_t interprate;
            if(printf_interprate_parser(core, &interprate)) {
                switch (interprate.type) {
                    case printf_specifier_d_or_i:
                    case printf_specifier_u:
                    case printf_specifier_o:
                    case printf_specifier_x:
                    case printf_specifier_X:
                        if(!printf_output_specifier_integer_iduoxX(core, &interprate))
                            goto LABEL_validate_failed_exit_point;
                        break;
                        
                    case printf_specifier_f:
                    case printf_specifier_F:
                    case printf_specifier_e:
                    case printf_specifier_E:
                    case printf_specifier_g:
                    case printf_specifier_G:
                    case printf_specifier_a:
                    case printf_specifier_A:
                        if(!printf_output_specifier_floating_point(core, &interprate))
                            goto LABEL_validate_failed_exit_point;
                        break;
                    
                    case printf_specifier_c:
                        if(!printf_output_specifier_character(core, &interprate))
                            goto LABEL_validate_failed_exit_point;
                        break;
                    
                    case printf_specifier_s:
                        if(!printf_output_specifier_string(core, &interprate))
                            goto LABEL_validate_failed_exit_point;
                        break;
                    
                    case printf_specifier_p:
                        if(!printf_output_specifier_pointer(core, &interprate))
                            goto LABEL_validate_failed_exit_point;
                        break;
                    
                    case printf_specifier_n:
                        if(!printf_output_specifier_count(core, &interprate))
                            goto LABEL_validate_failed_exit_point;
                        break;
                    
                    case printf_specifier_percentage:
                        if(!printf_output_specifier_percentage(core, &interprate))
                            goto LABEL_validate_failed_exit_point;
                        break;
                    
                    /* case error handle */
                    case printf_specifier_invalid: default:
                        DEBUG_POINT
                        validate_check = false;
                        goto LABEL_validate_failed_exit_point;
                }
            } else {
                DEVELOP_BREAKPOINT
                validate_check = false;
                goto LABEL_validate_failed_exit_point;
            }
        }
    }
    
    LABEL_validate_failed_exit_point:;
    
    if(validate_check)
        switch (core->type) {
            case printf_type_string:
                return (int)core->actual_need;
            case printf_type_FILE:
                return 0;
            default: DEBUG_RETURN(PRINTF_ERROR_RETURN);
        }
    
    printf_core_error_handle(core);     // last error handling
    
    DEBUG_RETURN(PRINTF_ERROR_RETURN);
}

#pragma mark - stack_snprintf

int CA_snprintf(char * _Nullable store, size_t n, const char * _Nonnull format, ... ) {
    if(format != NULL) {
        printf_core_t data; printf_core_ref core = &data;
        core->type = printf_type_string;
        core->format = (const UTF8Char *)format;
        va_list ap;
        va_start(ap, format);
        va_copy(core->args, ap);
#ifdef DEBUG
        va_copy(core->original_args, ap);
        core->original_format = (const UTF8Char *)format;
#endif
        core->output.string.count = n;
        core->output.string.index = 0;
        core->actual_need = 0;
        core->output.string.store = (UTF8Char *)store;
        int rt = printf_internal(core);
        va_end(ap);
        va_end(core->args);
#ifdef DEBUG
        va_end(core->original_args);
#endif
        return rt;
    }
    DEVELOP_BREAKPOINT_RETURN(PRINTF_ERROR_RETURN);
}

int CA_fprintf(FILE * _Nonnull fp, const char * _Nonnull format, ...) {
    if(fp != NULL && format != NULL) {
        printf_core_t data; printf_core_ref core = &data;
        core->type = printf_type_FILE;
        core->format = (const UTF8Char *)format;
        va_list ap;
        va_start(ap, format);
        va_copy(core->args, ap);
#ifdef DEBUG
        va_copy(core->original_args, ap);
        core->original_format = (const UTF8Char *)format;
#endif
        core->output.file = fp;
        int rt = printf_internal(core);
        va_end(ap);
        va_end(core->args);
#ifdef DEBUG
        va_end(core->original_args);
#endif
        return rt;
    }
    DEVELOP_BREAKPOINT_RETURN(PRINTF_ERROR_RETURN);
}

int CA_printf(const char * _Nonnull format, ...) {
    if(format != NULL) {
        printf_core_t data; printf_core_ref core = &data;
        core->type = printf_type_FILE;
        core->format = (const UTF8Char *)format;
        va_list ap;
        va_start(ap, format);
        va_copy(core->args, ap);
#ifdef DEBUG
        va_copy(core->original_args, ap);
        core->original_format = (const UTF8Char *)format;
#endif
        core->output.file = stdout;
        int rt = printf_internal(core);
        va_end(ap);
        va_end(core->args);
#ifdef DEBUG
        va_end(core->original_args);
#endif
        return rt;
    }
    DEVELOP_BREAKPOINT_RETURN(PRINTF_ERROR_RETURN);
    // [BREAKPOINT] something is wrong but may not
    // be software problem please check the format input
}
