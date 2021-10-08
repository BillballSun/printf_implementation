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
*    extended_integer.h 2019/10/15
*/

/*!
    @header extended_integer.h
 
    This header file describes way of extended integer calculation
 */

#ifndef extended_integer_h
#define extended_integer_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

#pragma mark - Basic Info

#pragma mark DO NOT CHANGE

/*!
    @typedef extended_integer_basic_type
    @abstract basic unsigned integer type used in extended integer calculation, this should be unsigned type
 */
typedef uint32_t    extended_integer_basic_type;

/*!
   @typedef extended_integer_basic_type_array
   @abstract used to pass parameters
*/
typedef extended_integer_basic_type *extended_integer_basic_type_array;

#pragma mark Basic Macro

/*!
    @name EXT_TYPE
    @abstract used to abbr extended_integer_basic_type
 */
#define EXT_TYPE    extended_integer_basic_type

/*!
    @name EXT_ARRAY
    @abstract used to abbr extended_integer_basic_type_array
 */
#define EXT_ARRAY extended_integer_basic_type_array

#pragma mark PRINTF & SCANF Macro

/*!
    @name EXT_PRI ?
    @abstract printf specifier for EXT_TYPE [ note only unsigned type is correct form ]
 */
#define EXT_PRId    PRId32
#define EXT_PRIi    PRIi32
#define EXT_PRIo    PRIo32
#define EXT_PRIu    PRIu32
#define EXT_PRIx    PRIx32
#define EXT_PRIX    PRIX32

/*!
    @name EXT_SCN ?
    @abstract scanf specifier for EXT_TYPE [ note only unsigned type is correct form ]
 */
#define EXT_SCNd    SCNd32
#define EXT_SCNi    SCNi32
#define EXT_SCNo    SCNo32
#define EXT_SCNu    SCNu32
#define EXT_SCNx    SCNx32

#pragma mark Type Info Macro

/*!
    @name EXT_BIT
    @abstract bits of each basic type
 */
#define EXT_BIT     32

/*!
   @name EXT_MAX
   @abstract max value for EXT_TYPE
*/
#define EXT_MAX     ((EXT_TYPE)(UINT32_MAX))

#pragma mark Help Macro

#define EXT_TYPE_C(value) UINT32_C(value)

/*!
    @name EXT_ARRAY_SIZE_FOR_BIT
    @abstract bit should be an unsigned value
 */
#define EXT_ARRAY_SIZE_FOR_BIT(bit)  \
        ((size_t)(((bit) / EXT_BIT) + ((bit) % EXT_BIT == 0 ? 0 : 1)))

/*!
   @name BIT_FOR_EXT_ARRAY_LENGTH
   @abstract actual bit for an array length
*/
#define BIT_FOR_EXT_ARRAY_LENGTH(length) (EXT_BIT * (length))

#pragma mark - Basic Operation

/*!
    @function extended_integer_initialize
    @abstract used to intialize an extended_integer array
 */
void extended_integer_initialize(EXT_ARRAY _Nonnull array,
                                 size_t length);

/*!
    @function extended_integer_compare
    @abstract compare two extended integer, return positive if parm1 > parm2, negative if parm1 < parm2
              and zero if equal, if anything wrong return zero
    @param length should not be zero
 */
int extended_integer_compare(EXT_ARRAY _Nonnull parm1,
                             EXT_ARRAY _Nonnull parm2,
                             size_t length);

/*!
   @function extended_integer_copy
   @abstract copy array into result
   @param length should not be zero
*/
void extended_integer_copy(EXT_ARRAY _Nonnull restrict array,
                           EXT_ARRAY _Nonnull restrict result,
                           size_t length);

#pragma mark - Standard Operation

/*!
    @function extended_integer_add
    @abstract result = parm1 + parm2
    @discussion [ overflow ] will round off like C standard
    @param length should not be zero
*/
void extended_integer_add(EXT_ARRAY _Nonnull parm1,
                          EXT_ARRAY _Nonnull parm2,
                          EXT_ARRAY _Nonnull restrict result,
                          size_t length,
                          bool * _Nullable overflow);

/*!
    @function extended_integer_abtract
    @abstract result = parm1 - parm2
    @discussion [ downflow ] if parm2 is larger than parm1, zero is store into result
    @param length should not be zero
*/
void extended_integer_sbtract(EXT_ARRAY _Nonnull parm1,
                              EXT_ARRAY _Nonnull parm2,
                              EXT_ARRAY _Nonnull restrict result,
                              size_t length,
                              bool * _Nullable downflow);

/*!
    @function extended_integer_multiply
    @abstract result = parm1 x parm2
    @discussion [ overflow ] will round off like C standard
    @param length should not be zero
*/
void extended_integer_multiply(EXT_ARRAY _Nonnull parm1,
                               EXT_ARRAY _Nonnull parm2,
                               EXT_ARRAY _Nonnull restrict result,
                               size_t length,
                               bool * _Nullable overflow);

/*!
    @function extended_integer_divide
    @abstract result = parm1 / parm2
    @discussion divide zero result in zero and runtime breakpoint
    @param length should not be zero
*/
void extended_integer_divide(EXT_ARRAY _Nonnull parm1,
                             EXT_ARRAY _Nonnull parm2,
                             EXT_ARRAY _Nonnull restrict result,
                             size_t length,
                             bool * _Nullable error_flag);

#pragma mark - Bit Operation

/*!
   @function extended_integer_get_bit
   @param length should not be zero
   @param bit_index begin at index zero
   @return the value at this index, if anything wrong return false
*/
bool extended_integer_get_bit(EXT_ARRAY _Nonnull array,
                              size_t length,
                              size_t bit_index);

/*!
   @function extended_integer_set_bit
   @param length should not be zero
   @param bit_index begin at index zero
*/
void extended_integer_set_bit(EXT_ARRAY _Nonnull array,
                              size_t length,
                              size_t bit_index,
                              bool value);

void extended_integer_bit_offset(EXT_ARRAY _Nonnull restrict array,
                                 EXT_ARRAY _Nonnull restrict result,
                                 size_t length,
                                 int offset);

#pragma mark - Quick Function

/*!
    @function extended_integer_quick_multiply
    @abstract array = array x value
    @discussion [ overflow ] will round off like C standard
    @param length should not be zero
*/
void extended_integer_quick_multiply(EXT_ARRAY _Nonnull array,
                                     uint32_t value,
                                     size_t length,
                                     bool * _Nullable overflow);

/*!
    @function extended_integer_quick_divide_modulo
    @param dividend after calculation the reminder is stored in it
    @param divisor should not be zero
    @param length should not be zero
    @param error_flag if not NULL, error indicator is stored. If nothing ever wrong, false should be stored.
    @result quotient of the operation, it is valid if error_flag stored false
    @abstract used for quick modulo operation taken digits out of current base exponent level
    @discussion [ 1 ] when divisor is zero, all operation is cancelled, zero returned, error_flag set to true
                [ 2 ] when result is larger than uint32_t could store, result is overflow like C style, error_flag set to true
 */
uint32_t extended_integer_quick_divide_modulo(EXT_ARRAY _Nonnull restrict dividend,
                                              EXT_ARRAY _Nonnull restrict divisor,
                                              size_t length,
                                              bool * _Nullable error_flag);

#pragma mark - DEBUG

#ifdef DEBUG

void extended_integer_debug(EXT_ARRAY _Nullable array, size_t length);

#endif

#endif /* extended_integer_h */
