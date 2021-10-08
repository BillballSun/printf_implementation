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
*    extended_integer.c 2019/10/15
*/

/*!
    @header extended_integer.c
    @abstract extended_integer is implemented in by uint32_t as basic type, align as little endian
 */

#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include "CFMacro.h"
#include "extended_integer.h"

#define HIGH_BIT(basic_element) ((EXT_TYPE_C(1) << (EXT_BIT - 1)) & ((EXT_TYPE)basic_element))

#ifdef __GNUC__
#define EXTENDED_INTEGER_TEMP_ALLOC(size) __builtin_alloca(size)
#else
#error GNUC compiler missing extended integer may use heap storage  \
       [ COMMENT THIS ERROR ] if you understand the problem
#include <stdlib.h>
#define EXTENDED_INTEGER_TEMP_ALLOC(size) malloc(size)
#endif

#pragma mark - Common Static Decleration

/*!
   @function extended_integer_query_info
   @abstract query related info of an extended integer
   @param length should not be zero
   @param all_zero if not null, store whether the whole array is zero
   @param highest_bit_index if not null, store the first bit index that is not zero
                       this could be non-null only if all_zero is not null
                       if all_zero is null and this is not null, debug exception raised
                       if all bit is zero, stored zero for general purpose
   @param lowest_bit_index if not null, store the first bit index that is not zero
                       this could be non-null only if all_zero is not null
                       if all_zero is null and this is not null, debug exception raised
                       if all bit is zero, stored zero for general purpose
*/
static void extended_integer_query_info(EXT_ARRAY _Nonnull restrict array,
                                        size_t length,
                                        bool * _Nonnull restrict all_zero,
                                        size_t * _Nullable restrict highest_bit_index,
                                        size_t * _Nullable restrict lowest_bit_index);

#pragma mark - Inline Static Decleration

/*!
    @function extended_integer_inline_get_bit
    @param length should not be zero
    @param bit_index begin at index zero
 */
static inline bool extended_integer_inline_get_bit(EXT_ARRAY _Nonnull array,
                                                   size_t length,
                                                   size_t bit_index);

/*!
    @function extended_integer_inline_set_bit
    @param length should not be zero
    @param bit_index begin at index zero
 */
static inline void extended_integer_inline_set_bit(EXT_ARRAY _Nonnull array,
                                                   size_t length,
                                                   size_t bit_index,
                                                   bool value);

static inline void extended_integer_inline_bit_offset(EXT_ARRAY _Nonnull restrict array,
                                                      EXT_ARRAY _Nonnull restrict result,
                                                      size_t length,
                                                      int offset);

static void inline extended_integer_inline_all_zero(EXT_ARRAY _Nonnull array, size_t length);

#pragma mark - Basic Operation

void extended_integer_initialize(EXT_ARRAY _Nonnull array, size_t length) {
    extended_integer_inline_all_zero(array, length);
}

int extended_integer_compare(EXT_ARRAY parm1,
                             EXT_ARRAY parm2,
                             size_t length) {
    if(parm1 != NULL && parm2 != NULL && length > 0) {
        for(size_t count = length; count > 0; count--) {
            if(parm1[count - 1] != parm2[count - 1])
                return (parm1[count - 1] > parm2[count - 1]) ? 1 : - 1;
        }
        return 0;
    }
    DEBUG_RETURN(0);
}

void extended_integer_copy(EXT_ARRAY _Nonnull restrict array,
                           EXT_ARRAY _Nonnull restrict result,
                           size_t length) {
    if(array != NULL && result != NULL && length > 0) {
        for(size_t index = 0; index < length; index++)
            result[index] = array[index];
    } DEBUG_ELSE
}

#pragma mark - Standard Operation

/*!
    @function basic_type_promotion_add_inline
    @param promotion input if promotion from lower array element, output if promoted to next significand array element
    @return parm1 + parm2
 */
static inline EXT_TYPE basic_type_promotion_add_inline(EXT_TYPE parm1,
                                                       EXT_TYPE parm2,
                                                       bool * _Nonnull promotion) {
    if(promotion != NULL) {
        bool pre_promotion = promotion[0];
        EXT_TYPE remain = EXT_MAX - parm1;
        if(pre_promotion) {
            if(parm2 >= remain) promotion[0] = true;
            else promotion[0] = false;
        }
        else {
            if(parm2 > remain) promotion[0] = true;
            else promotion[0] = false;
        }
        return parm1 + parm2 + (pre_promotion ? EXT_TYPE_C(1) : EXT_TYPE_C(0));
    } DEBUG_ELSE
    return 0;
}

void extended_integer_add(EXT_ARRAY _Nonnull parm1,
                          EXT_ARRAY _Nonnull parm2,
                          EXT_ARRAY _Nonnull restrict result,
                          size_t length,
                          bool * _Nullable overflow) {
    if(parm1 != NULL && parm2 != NULL && result != NULL && length > 0) {
        bool promotion = false;
        for(size_t index = 0; index < length; index++) {
            if(HIGH_BIT(parm1[index]) || HIGH_BIT(parm2[index]))
                result[index] = basic_type_promotion_add_inline(parm1[index], parm2[index], &promotion);
            else {
                result[index] = parm1[index] + parm2[index] + (promotion ? EXT_TYPE_C(1) : EXT_TYPE_C(0));
                promotion = false;
            }
        }
        if(overflow != NULL) overflow[0] = promotion;
    } DEBUG_ELSE
}

void extended_integer_sbtract(EXT_ARRAY _Nonnull parm1,
                              EXT_ARRAY _Nonnull parm2,
                              EXT_ARRAY _Nonnull restrict result,
                              size_t length,
                              bool * _Nullable downflow) {
    if(parm1 != NULL && parm2 != NULL && result != NULL && length > 0) {
        int compare_result = extended_integer_compare(parm1, parm2, length);
        if(compare_result > 0) {
            bool borrow = false;
            for(size_t index = 0; index < length; index++) {
                if(borrow ? parm1[index] <= parm2[index] : parm1[index] < parm2[index]) {
                    // parm1 + ((MAX + 1) - parm2) - borrow
                    result[index] = parm1[index] + (EXT_MAX - parm2[index]) + (borrow ? EXT_TYPE_C(0) : EXT_TYPE_C(1));
                    borrow = true;
                }
                else {
                    result[index] = parm1[index] - parm2[index] - (borrow ? EXT_TYPE_C(1) : EXT_TYPE_C(0));
                    borrow = false;
                }
            }
            DEBUG_ASSERT(!borrow);
            if(downflow != NULL) downflow[0] = false;
        } else {
            extended_integer_inline_all_zero(result, length);
            if(downflow != NULL) {
                if(compare_result == 0) downflow[0] = false;
                else downflow[0] = true;
            }
        }
    } DEBUG_ELSE
}

void extended_integer_multiply(EXT_ARRAY _Nonnull parm1,
                               EXT_ARRAY _Nonnull parm2,
                               EXT_ARRAY _Nonnull restrict result,
                               size_t length,
                               bool * _Nullable overflow) {
    if(parm1 != NULL && parm2 != NULL && result != NULL && length > 0) {
        
        bool is_overflow = false;
        
        // anyway to all zero result
        extended_integer_inline_all_zero(result, length);
        
        bool parm1_all_zero;
        size_t parm1_highest_bit_index;
        extended_integer_query_info(parm1, length, &parm1_all_zero, &parm1_highest_bit_index, NULL);
        if(parm1_all_zero) goto LABLE_QUICK_FINISH;
        
        bool parm2_all_zero;
        size_t parm2_highest_bit_index;
        size_t parm2_lowest_bit_index;
        extended_integer_query_info(parm2, length, &parm2_all_zero, &parm2_highest_bit_index, &parm2_lowest_bit_index);
        if(parm2_all_zero) goto LABLE_QUICK_FINISH;
        DEBUG_ASSERT(parm2_lowest_bit_index <= parm2_highest_bit_index);
        
        size_t parm1_overflow_least_offset = (EXT_BIT * length) - parm1_highest_bit_index;
        
        EXT_ARRAY offseted_parm1 = EXTENDED_INTEGER_TEMP_ALLOC(sizeof(EXT_TYPE) * length);
        EXT_ARRAY pre_saved_result = EXTENDED_INTEGER_TEMP_ALLOC(sizeof(EXT_TYPE) * length);
        
        if(parm2_highest_bit_index >= parm1_overflow_least_offset) is_overflow = true;
        
        for(size_t index = parm2_lowest_bit_index; index <= parm2_highest_bit_index; index++)
            if(extended_integer_inline_get_bit(parm2, length, index)) {
                extended_integer_inline_bit_offset(parm1, offseted_parm1, length,(int)index);
                extended_integer_copy(result, pre_saved_result, length);
                extended_integer_add(offseted_parm1, pre_saved_result, result, length, is_overflow ? NULL : &is_overflow);
            }
        LABLE_QUICK_FINISH:
        if(overflow != NULL) overflow[0] = is_overflow;
        
    } DEBUG_ELSE
}

void extended_integer_divide(EXT_ARRAY _Nonnull parm1,
                             EXT_ARRAY _Nonnull parm2,
                             EXT_ARRAY _Nonnull restrict result,
                             size_t length,
                             bool * _Nullable error_flag) {
    if(parm1 != NULL && parm2 != NULL && result != NULL && length > 0) {
        bool is_error = false;
        
        extended_integer_inline_all_zero(result, length);
        
        bool parm2_all_zero; size_t parm2_highest_bit_index;
        extended_integer_query_info(parm2, length, &parm2_all_zero, &parm2_highest_bit_index, NULL);
        if(parm2_all_zero) {
            is_error = true;
            goto LABLE_QUICK_FINISH;
        }
        
        bool parm1_all_zero; size_t parm1_highest_bit_index;
        extended_integer_query_info(parm1, length, &parm1_all_zero, &parm1_highest_bit_index, NULL);
        if(parm1_all_zero) goto LABLE_QUICK_FINISH;
        
        if(parm2_highest_bit_index > parm1_highest_bit_index) goto LABLE_QUICK_FINISH;
        
        size_t offset = parm1_highest_bit_index - parm2_highest_bit_index;
        
        EXT_ARRAY parm1_taken = EXTENDED_INTEGER_TEMP_ALLOC(sizeof(EXT_TYPE) * length);
        EXT_ARRAY parm2_offset = EXTENDED_INTEGER_TEMP_ALLOC(sizeof(EXT_TYPE) * length);
        EXT_ARRAY temp = EXTENDED_INTEGER_TEMP_ALLOC(sizeof(EXT_TYPE) * length);
        
        extended_integer_copy(parm1, parm1_taken, length);
        
        bool downflow;
        
        LOOP {
            extended_integer_inline_bit_offset(parm2, parm2_offset, length, (int)offset);
            
            int compare_result = extended_integer_compare(parm1_taken, parm2_offset, length);
            
            if(compare_result > 0) {
                extended_integer_sbtract(parm1_taken, parm2_offset, temp, length, &downflow); DEBUG_ASSERT(!downflow);
                extended_integer_copy(temp, parm1_taken, length);
                extended_integer_inline_set_bit(result, length, offset, true);
            }
            else if(compare_result == 0) {
                extended_integer_inline_set_bit(result, length, offset, true);
                break;
            }
            
            if(offset == 0) break;
            else offset = offset - 1;
        }
        
        // break exit
        
        LABLE_QUICK_FINISH:
        DEBUG_ASSERT(!is_error);    // divide zero breakpoint
        if(error_flag != NULL) error_flag[0] = is_error;
        
    } DEBUG_ELSE
}

#pragma mark - Bit Operation

bool extended_integer_get_bit(EXT_ARRAY _Nonnull array,
                              size_t length,
                              size_t bit_index) {
    return extended_integer_inline_get_bit(array, length, bit_index);
}

void extended_integer_set_bit(EXT_ARRAY _Nonnull array,
                              size_t length,
                              size_t bit_index,
                              bool value) {
    extended_integer_inline_set_bit(array, length, bit_index, value);
}

void extended_integer_bit_offset(EXT_ARRAY _Nonnull restrict array,
                                 EXT_ARRAY _Nonnull restrict result,
                                 size_t length,
                                 int offset) {
    extended_integer_inline_bit_offset(array, result, length, offset);
}

static inline bool extended_integer_inline_get_bit(EXT_ARRAY _Nonnull array,
                                                   size_t length,
                                                   size_t bit_index) {
    if(array != NULL && length > 0 && bit_index < length * EXT_BIT) {
        size_t index = bit_index / EXT_BIT;
        size_t offset = bit_index % EXT_BIT;
        return array[index] & (EXT_TYPE_C(1) << offset);
    } DEBUG_ELSE
    return false;
}

static inline void extended_integer_inline_set_bit(EXT_ARRAY _Nonnull array,
                                                   size_t length,
                                                   size_t bit_index,
                                                   bool value) {
    if(array != NULL && length > 0 && bit_index < length * EXT_BIT) {
        size_t index = bit_index / EXT_BIT;
        size_t offset = bit_index % EXT_BIT;
        if(value) {
            EXT_TYPE temp = (EXT_TYPE_C(1) << offset);
            array[index] |= temp;
        }
        else {
            EXT_TYPE temp = ~ (EXT_TYPE_C(1) << offset);
            array[index] &= temp;
        }
    } DEBUG_ELSE
}

/*
 
 // Deprecated implementation [2019.12.10]
 
 static inline void extended_integer_inline_bit_offset_ori(EXT_ARRAY _Nonnull restrict array,
                                                           EXT_ARRAY _Nonnull restrict result,
                                                           size_t length,
                                                           int offset) {
     if(array != NULL && result != NULL && length > 0) {
         if(offset != 0) {
             size_t bit_amount = length * EXT_BIT;
             bool negative = false;
             if(offset < 0) {
                 negative = true;
                 offset = - offset;
             }
             extended_integer_inline_all_zero(result, length);
             if(offset < bit_amount) {
                 size_t need_move_amount = bit_amount - offset;
                 if(negative)
                     for(size_t index = 0; index < need_move_amount; index++) {
                         bool value = extended_integer_inline_get_bit(array, length, index + offset);
                         extended_integer_inline_set_bit(result, length, index, value);
                     }
                 else
                     for(size_t index = 0; index < need_move_amount; index++) {
                         bool value = extended_integer_inline_get_bit(array, length, bit_amount - 1 - index - offset);
                         extended_integer_inline_set_bit(result, length, bit_amount - 1 - index, value);
                     }
             } DEVELOP_ELSE_BREAKPOINT
         } else extended_integer_copy(array, result, length);
     } DEBUG_ELSE
 }
 
 */

static inline void extended_integer_inline_bit_offset(EXT_ARRAY _Nonnull restrict array,
                                                      EXT_ARRAY _Nonnull restrict result,
                                                      size_t length,
                                                      int offset) {
    if(array != NULL && result != NULL && length > 0) {     // length != 0
        if(offset != 0) {
            extended_integer_inline_all_zero(result, length);
            bool negative = false;
            if(offset < 0) {
                negative = true;
                offset = - offset;
            }
            size_t offset_bit   = offset % EXT_BIT;
            size_t offset_bit_complement = EXT_BIT - offset_bit;
            size_t offset_count = offset / EXT_BIT;
            if(offset_count < length) {
                if(negative) {
                    if(offset_bit == 0) {       // optimized
                        for(size_t index = 0; index <= length - 1 - offset_count; index++)
                            result[index] = array[index + offset_count];
                    }
                    else {
#define TRAIL_TO_HEAD(value, destination) (destination) |= (value) << offset_bit_complement
#define HEAD_TO_TRAIL(value, destination) (destination) |= (value) >> (offset_bit)
                        size_t last_origin_index = offset_count;
                        HEAD_TO_TRAIL(array[last_origin_index], result[0]);
                        if(last_origin_index < length - 1) {
                            size_t remain_count = length - 1 - offset_count;
                            for(size_t count = 1; count <= remain_count; count++) {
                                size_t current_origin_index = last_origin_index + count;
                                size_t result_head_index = count - 1;
                                size_t result_trail_index = count;
                                TRAIL_TO_HEAD(array[current_origin_index], result[result_head_index]);
                                HEAD_TO_TRAIL(array[current_origin_index], result[result_trail_index]);
                            }
                        }
#undef TRAIL_TO_HEAD
#undef HEAD_TO_TRAIL
                    }
                }
                else {
                    if(offset_bit == 0) {       // optimized
                        for(size_t index = 0; index <= length - 1 - offset_count; index++)
                            result[index + offset_count] = array[index];
                    }
                    else {
#define TRAIL_TO_HEAD(value, destination) (destination) |= (value) << offset_bit
#define HEAD_TO_TRAIL(value, destination) (destination) |= (value) >> (offset_bit_complement)
                        size_t first_origin_index = length - 1 - offset_count;
                        TRAIL_TO_HEAD(array[first_origin_index], result[length - 1]);
                        if(first_origin_index > 0) {
                            size_t remain_count = first_origin_index;
                            for(size_t count = 1; count <= remain_count; count++) {
                                size_t current_origin_index = first_origin_index - count;
                                size_t result_trail_index = length - count;
                                size_t result_head_index = length - count - 1;
                                HEAD_TO_TRAIL(array[current_origin_index], result[result_trail_index]);
                                TRAIL_TO_HEAD(array[current_origin_index], result[result_head_index]);
                            }
                        }
#undef TRAIL_TO_HEAD
#undef HEAD_TO_TRAIL
                    }
                }
            } DEVELOP_ELSE_BREAKPOINT
        } else extended_integer_copy(array, result, length);
    } DEBUG_ELSE
}

#pragma mark - Quick Function

void extended_integer_quick_multiply(EXT_ARRAY _Nonnull array,
                                     uint32_t value,
                                     size_t length,
                                     bool * _Nullable overflow) {
    if(array != NULL && length > 0) {
        
        bool is_overflow = false;
        
        EXT_ARRAY result = EXTENDED_INTEGER_TEMP_ALLOC(length * sizeof(EXT_TYPE));
        
        // anyway to all zero result
        extended_integer_inline_all_zero(result, length);
        
        bool array_all_zero;
        size_t array_highest_bit_index;
        extended_integer_query_info(array, length, &array_all_zero, &array_highest_bit_index, NULL);
        if(array_all_zero) goto LABLE_QUICK_FINISH;
        
        EXT_ARRAY temp1 = EXTENDED_INTEGER_TEMP_ALLOC(length * sizeof(EXT_TYPE));
        EXT_ARRAY temp2 = EXTENDED_INTEGER_TEMP_ALLOC(length * sizeof(EXT_TYPE));
        
        DEBUG_ASSERT(sizeof(uint32_t) * CHAR_BIT == 32);
        for(size_t index = 0; index < sizeof(uint32_t) * CHAR_BIT; index++) {
            uint32_t query = UINT32_C(1) << index;
            if(query & value) {
                if(!is_overflow) {
                    size_t highest_bit_index_after_offset = array_highest_bit_index + index;
                    size_t highest_bit_count_possible = EXT_BIT * length;
                    if(highest_bit_index_after_offset >= highest_bit_count_possible)
                        is_overflow = true;
                }
                extended_integer_inline_bit_offset(array, temp1, length, (int)index);
                bool this_time_overflow;
                extended_integer_add(temp1, result, temp2, length, &this_time_overflow);
                DEVELOP_ASSERT(!this_time_overflow);
                if(!is_overflow && this_time_overflow) is_overflow = true;
                extended_integer_copy(temp2, result, length);
            }
        }
        
        LABLE_QUICK_FINISH:
        if(overflow != NULL) overflow[0] = is_overflow;
        extended_integer_copy(result, array, length);
        
    } DEBUG_ELSE
}

uint32_t extended_integer_quick_divide_modulo(EXT_ARRAY _Nonnull restrict dividend,
                                              EXT_ARRAY _Nonnull restrict divisor,
                                              size_t length,
                                              bool * _Nullable error_flag) {
    if(dividend != NULL && divisor != NULL && length != 0) {
        bool error = false; uint32_t result = 0;
        
        bool divisor_all_zero; size_t divisor_highest_bit_index;
        extended_integer_query_info(divisor, length, &divisor_all_zero, &divisor_highest_bit_index, NULL);
        if(divisor_all_zero) { error = true; goto LABLE_QUICK_FINISH; }
        
        // quick finish flag
        if(extended_integer_compare(dividend, divisor, length) < 0)
            goto LABLE_QUICK_FINISH;
        
        // divide
        bool should_be_false;
        EXT_ARRAY quotient = EXTENDED_INTEGER_TEMP_ALLOC(length * sizeof(EXT_TYPE));
        extended_integer_divide(dividend, divisor, quotient, length, &should_be_false);
        DEBUG_ASSERT(!should_be_false);
        if(should_be_false) { error = true; goto LABLE_QUICK_FINISH; }
        
        // query
        bool quotient_all_zero; size_t quotient_highest_bit_index;
        extended_integer_query_info(quotient, length, &quotient_all_zero, &quotient_highest_bit_index, NULL);
        DEBUG_ASSERT(!quotient_all_zero);
        if(quotient_all_zero) { error = true; goto LABLE_QUICK_FINISH; }
        
        // result intial is zero
        
        COMILE_ASSERT(sizeof(uint32_t) * CHAR_BIT == 32);
        if(quotient_highest_bit_index >= sizeof(uint32_t) * CHAR_BIT)
            error = true;   // overflow uint32_t
        
        if (sizeof(EXT_TYPE) == sizeof(uint32_t))
            result = quotient[0];
        else {
            DEBUG_LOG("[STDC] extended_integer_quick_divide_modulo optimization code may required");
            size_t to_index = sizeof(uint32_t) * CHAR_BIT - 1;
            if(quotient_highest_bit_index < to_index)
                to_index = quotient_highest_bit_index;
            for(size_t index = 0; index <= to_index; index++)
                if(extended_integer_inline_get_bit(quotient, length, index))
                    result |= (UINT32_C(1) << index);
        }
        EXT_ARRAY product = EXTENDED_INTEGER_TEMP_ALLOC(length * sizeof(EXT_TYPE));
        extended_integer_multiply(divisor, quotient, product, length, &should_be_false);
        DEBUG_ASSERT(!should_be_false);
        if(should_be_false) { error = true; goto LABLE_QUICK_FINISH; }
        EXT_ARRAY difference = EXTENDED_INTEGER_TEMP_ALLOC(length * sizeof(EXT_TYPE));
        extended_integer_sbtract(dividend, product, difference, length,  &should_be_false);
        DEBUG_ASSERT(!should_be_false);
        if(should_be_false) { error = true; goto LABLE_QUICK_FINISH; }
        extended_integer_copy(difference, dividend, length);
        
        LABLE_QUICK_FINISH:
        if(error_flag != NULL) error_flag[0] = error;
        return result;
    }
    if(error_flag != NULL) error_flag[0] = true;
    DEBUG_RETURN(0);
}

#pragma mark - Static Definition

static void extended_integer_query_info(EXT_ARRAY _Nonnull restrict array,
                                        size_t length,
                                        bool * _Nonnull restrict all_zero,
                                        size_t * _Nullable restrict highest_bit_index,
                                        size_t * _Nullable restrict lowest_bit_index) {
    if(array != NULL && length > 0) { DEBUG_ASSERT(all_zero != NULL);
        bool is_all_zero = true;
        size_t high_index = 0;      // only valid if is_all_zero (false)
        size_t low_index = 0;       // only valid if is_all_zero (false)
        if(all_zero != NULL) {
            size_t serach_index;
            for(serach_index = 0; serach_index < length; serach_index++)
                if(array[serach_index] != 0) {
                    is_all_zero = false;
                    low_index = serach_index;
                    break;
                }
            DEBUG_ASSERT(serach_index >= length ? is_all_zero : !is_all_zero);
            if(serach_index < length) {
                if(serach_index + 1 == length) high_index = serach_index;
                else {
                    size_t reverse_search_index;
                    for(reverse_search_index = length - 1; reverse_search_index > serach_index; reverse_search_index--)
                        if(array[reverse_search_index] != 0)
                            break;
                    DEBUG_ASSERT(reverse_search_index >= serach_index);
                    high_index = reverse_search_index;
                }
            }
            all_zero[0] = is_all_zero;
        }
        if(highest_bit_index != NULL) {
            if(all_zero != NULL) {
                if(is_all_zero) highest_bit_index[0] = 0;
                else {
                    size_t offset;
                    for(offset = 0; offset < EXT_BIT; offset++) {
                        EXT_TYPE value = (EXT_TYPE_C(1) << (EXT_BIT - offset - 1));
                        if(array[high_index] & value) {
                            highest_bit_index[0] = (high_index * EXT_BIT + (EXT_BIT - offset - 1));
                            break;
                        }
                    }
                    if(offset >= EXT_BIT) {
                        highest_bit_index[0] = 0;
                        DEBUG_POINT;    // should not be reached
                    }
                }
            } else { highest_bit_index[0] = 0; DEBUG_POINT; }
        }
        if(lowest_bit_index != NULL) {
            if(all_zero != NULL) {
                if(is_all_zero) lowest_bit_index[0] = 0;
                else {
                    size_t offset;
                    for(offset = 0; offset < EXT_BIT; offset++) {
                        EXT_TYPE value = (EXT_TYPE_C(1) << offset);
                        if(array[low_index] & value) {
                            lowest_bit_index[0] = (low_index * EXT_BIT + offset);
                            break;
                        }
                    }
                    if(offset >= EXT_BIT) {
                        lowest_bit_index[0] = 0;
                        DEBUG_POINT;    // should not be reached
                    }
                }
            } else { lowest_bit_index[0] = 0; DEBUG_POINT; }
        }
    } DEBUG_ELSE
}

// compiler comm optimization for __platfrom_bzero code
static inline void extended_integer_inline_all_zero(EXT_ARRAY _Nonnull array, size_t length) {
    if(array != NULL && length > 0) {
        for(size_t index = 0; index < length; index++)
            array[index] = 0u;
    } DEBUG_ELSE
}

#pragma mark - DEBUG

#ifdef DEBUG

void extended_integer_debug(EXT_ARRAY _Nullable array, size_t length) {
    fprintf(stdout, "[DEBUG] extended_integer %p length %zu\n", array, length);
    if(array == NULL) {
        fprintf(stdout, "[INFO] array point to null\n");
        return;
    }
    if(length == 0) {
        fprintf(stdout, "[INFO] array length zero\n");
        return;
    }
    for(size_t index = 0; index < length; index++)
        fprintf(stdout, "[%2zu] %0*" EXT_PRIx "\n", index, (EXT_BIT / CHAR_BIT) * 2, array[index]);
}

#endif

#pragma mark - Auto Test

#ifdef DEBUG

GNU_ATTRIBUTE_CONSTRUCTOR
static void  __HIDDEN__extended_integer_auto_test(void) {
    /* constant offset test */
    EXT_TYPE arr1[10];
    EXT_TYPE arr2[10];
    extended_integer_inline_bit_offset(arr1, arr2, 10, EXT_BIT);
    for(size_t index = 0; index <= 8; index++)
        if(arr1[index] != arr2[index + 1]) DEBUG_POINT;
    EXT_TYPE arr3[10];
    EXT_TYPE arr4[10];
    extended_integer_inline_bit_offset(arr3, arr4, 10, - EXT_BIT);
    for(size_t index = 1; index <= 9; index++)
        if(arr3[index] != arr4[index - 1]) DEBUG_POINT;
    /* EXT_BIT */
    COMILE_ASSERT(sizeof(EXT_TYPE) * CHAR_BIT == EXT_BIT);
}

#endif
