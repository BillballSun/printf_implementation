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
*    printf_test.c 2019/10/15
*/

#ifdef DEBUG        // debug only

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <math.h>
#include "printf_test.h"
#include "printf_core.h"
#include "CFMacro.h"

#define INFO(format, ...) do {                                                  \
    fprintf(stdout, "✅ ");                                                     \
    fprintf(stdout, format, ## __VA_ARGS__);                                    \
    putc('\n', stdout);                                                         \
    } while(0)

static void correct_test(void);
static void strict_test(void);
static void speed_test(void);
static void gnuc_test(void);
static void n_test(void);

void printf_test(void) {
    setlocale(LC_CTYPE, "zh_CN");
    correct_test();
    strict_test();
    speed_test();
    gnuc_test();
    n_test();
    DEBUG_POINT;        // test passed
}

#define TEST_SAME(format, ...) do {                                             \
    fprintf(stdout, " [FORMAT] %s\n", format);                                  \
    int rt1 = snprintf(arr, ARRAY_SIZE(arr), format, ## __VA_ARGS__);           \
    fprintf(stdout, " SYS[%2d]: %s\n", rt1, arr);                               \
    int rt2 = CA_snprintf(arr, ARRAY_SIZE(arr), format, ## __VA_ARGS__);        \
    fprintf(stdout, "USER[%2d]: %s\n", rt2, arr);                               \
    DEBUG_ASSERT(rt1 == rt2); } while(0)

#define SHOW_DIFF(format, ...) do {                                             \
    fprintf(stdout, "[FORMAT] %s\n", format);                                   \
    int rt1 = snprintf(arr, ARRAY_SIZE(arr), format, ## __VA_ARGS__);           \
    fprintf(stdout, " SYS[%2d]: %s\n", rt1, arr);                               \
    int rt2 = CA_snprintf(arr, ARRAY_SIZE(arr), format, ## __VA_ARGS__);        \
    fprintf(stdout, "USER[%2d]: %s\n", rt2, arr);} while(0)                     \

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_FORMAT

static void correct_test(void) {
    char arr[100];
    INFO("width");
    TEST_SAME("%12.3f", 123.123);
    INFO("mixed");
    TEST_SAME("%0#12.3X", 0x123456);
    TEST_SAME("%-012f", 123.123);
    INFO("g specifier");
    TEST_SAME("%.6g", 0.1200001);
    TEST_SAME("%.6g", 0.01299989999999);
    TEST_SAME("%.6g", 123456.123456);
    TEST_SAME("%.6g", 1234567.1234567);
    INFO("NAN");
    TEST_SAME("%f", NAN);
    TEST_SAME("%F", NAN);
    INFO("INFINITY");
    TEST_SAME("%f", INFINITY);
    TEST_SAME("%F", INFINITY);
    TEST_SAME("%f", -INFINITY);
    TEST_SAME("%F", -INFINITY);
    INFO("specifier a");
    TEST_SAME("%050a", 0x1.234p+0);
    TEST_SAME("%20A", -0x1.2345p+0);
    TEST_SAME("%p", (void *)0x12345);
    TEST_SAME("%20p", (void *)0);
    TEST_SAME("%20%");
    TEST_SAME("%s", "我爱你中国");
    SHOW_DIFF("%.4s", "我爱你中国");
    SHOW_DIFF("%20s", "亲爱的母亲, 我为你流泪, 也为你自豪");
    INFO("ZERO");
    TEST_SAME("%f", 0.0);
    TEST_SAME("%-#12.0f", 0.0);
    TEST_SAME("%e", 0.0);
    TEST_SAME("%.2e", 0.0);
    TEST_SAME("%6.12g", 0.0);
    TEST_SAME("%a", 0.0);
    TEST_SAME("%12.3a", 0.0);
    TEST_SAME("%f", -0.0);
    TEST_SAME("%-#12.0f", -0.0);
    TEST_SAME("%e", -0.0);
    TEST_SAME("%.2e", -0.0);
    TEST_SAME("%6.12g", -0.0);
    TEST_SAME("%a", -0.0);
    TEST_SAME("%12.3a", -0.0);
    TEST_SAME("%.70f", 2333333333333333);
    TEST_SAME("%ls", L"我爱你中国");
    SHOW_DIFF("%.4ls", L"我爱你中国");
    SHOW_DIFF("%20ls", L"亲爱的母亲, 我为你流泪, 也为你自豪");
}

static void strict_test(void) {
    char arr1[2]; char arr2[2];
    arr1[1] = 0xF; arr2[1] = 0xF;
    int rt1 = CA_snprintf(arr1, 1, "%12.3f", 123.3456789);
    int rt2 = snprintf(arr2, 1, "%12.3f", 123.3456789);
    DEBUG_ASSERT(rt1 == rt2);
    DEBUG_ASSERT(strcmp(arr1, arr2) == 0);
    DEBUG_ASSERT(arr1[1] == 0xF);
    DEBUG_ASSERT(arr2[1] == 0xF);
    rt1 = CA_snprintf(arr1, 0, "%12.3f", 123.3456789);
    rt2 = snprintf(arr2, 0, "%12.3f", 123.3456789);
    DEBUG_ASSERT(rt1 == rt2);
    rt1 = CA_snprintf(NULL, 0, "%12.3f", 123.3456789);
    rt2 = snprintf(NULL, 0, "%12.3f", 123.3456789);
    DEBUG_ASSERT(rt1 == rt2);
}

static void speed_test(void) {
    char arr1[100]; char arr2[100];
    
    clock_t snprintf_start = clock();
    snprintf(arr1, 100, "%f\n", 0x1p-1022L);
    clock_t snprintf_end = clock();
    
    clock_t CA_snprintf_start = clock();
    CA_snprintf(arr2, 100, "%f\n", 0x1p-1022L);
    clock_t CA_snprintf_end = clock();
    
    fprintf(stdout, "SYS time: %.8f sec\n", (double)(snprintf_end - snprintf_start) / CLOCKS_PER_SEC);
    fprintf(stdout, "USR time: %.8f sec\n", (double)(CA_snprintf_end - CA_snprintf_start) / CLOCKS_PER_SEC);
}

#define GNUC_TEST1(format, ...) do {        \
    printf(format, ## __VA_ARGS__);         \
    CA_printf(format, ## __VA_ARGS__);      \
} while(0)

static void gnuc_test(void) {
    char arr[100];
    char shortstr[] = "Hi, Z.";
    char longstr[] = "Good morning, Doctor Chandra.  This is Hal. I am ready for my first lesson today.";
    TEST_SAME("decimal negative:\t\"%d\"\n", -2345);
    TEST_SAME("octal negative:\t\"%o\"\n", -2345);
    TEST_SAME("hex negative:\t\"%x\"\n", -2345);
    TEST_SAME("long decimal number:\t\"%ld\"\n", -123456L);
    TEST_SAME("long octal negative:\t\"%lo\"\n", -2345L);
    TEST_SAME("long unsigned decimal number:\t\"%lu\"\n", -123456L);
    TEST_SAME("zero-padded LDN:\t\"%010ld\"\n", -123456L);
    TEST_SAME("left-adjusted ZLDN:\t\"%-010ld\"\n", -123456L);
    TEST_SAME("space-padded LDN:\t\"%10ld\"\n", -123456L);
    TEST_SAME("left-adjusted SLDN:\t\"%-10ld\"\n", -123456L);
    TEST_SAME("zero-padded string:\t\"%010s\"\n", shortstr);
    TEST_SAME("left-adjusted Z string:\t\"%-010s\"\n", shortstr);
    TEST_SAME("space-padded string:\t\"%10s\"\n", shortstr);
    TEST_SAME("left-adjusted S string:\t\"%-10s\"\n", shortstr);
    TEST_SAME("null string:\t\"%s\"\n", (char *)NULL);
    TEST_SAME("limited string:\t\"%.22s\"\n", longstr);
    TEST_SAME("a-style max:\t\"%a\"\n", DBL_MAX);
    TEST_SAME("a-style -max:\t\"%a\"\n", -DBL_MAX);
    TEST_SAME("e-style >= 1:\t\"%e\"\n", 12.34);
    TEST_SAME("e-style >= .1:\t\"%e\"\n", 0.1234);
    TEST_SAME("e-style < .1:\t\"%e\"\n", 0.001234);
    TEST_SAME("e-style big:\t\"%.60e\"\n", 1e20);
    TEST_SAME("e-style == .1:\t\"%e\"\n", 0.1);
    TEST_SAME("f-style >= 1:\t\"%f\"\n", 12.34);
    TEST_SAME("f-style >= .1:\t\"%f\"\n", 0.1234);
    TEST_SAME("f-style < .1:\t\"%f\"\n", 0.001234);
    TEST_SAME("g-style >= 1:\t\"%g\"\n", 12.34);
    TEST_SAME("g-style >= .1:\t\"%g\"\n", 0.1234);
    TEST_SAME("g-style < .1:\t\"%g\"\n", 0.001234);
    TEST_SAME("g-style big:\t\"%.60g\"\n", 1e20);
    TEST_SAME(" %6.5f\n", .099999999860301614);
    TEST_SAME(" %6.5f\n", .1);
    TEST_SAME("x%5.4fx\n", .5);
    TEST_SAME("%#03x\n", 1);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 0.0, 0.0, 0.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 1.0, 1.0, 1.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", -1.0, -1.0, -1.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 100.0, 100.0, 100.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 1000.0, 1000.0, 1000.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 10000.0, 10000.0, 10000.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 12345.0, 12345.0, 12345.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 100000.0, 100000.0, 100000.0);
    TEST_SAME("|%12.4f|%12.4e|%12.4g|\n", 123456.0, 123456.0, 123456.0);
    TEST_SAME("%.8f\n", DBL_MAX);
    TEST_SAME("%.8f\n", -DBL_MAX);
    TEST_SAME("%e should be 1.234568e+06\n", 1234567.8);
    TEST_SAME("%f should be 1234567.800000\n", 1234567.8);
    TEST_SAME("%g should be 1.23457e+06\n", 1234567.8);
    TEST_SAME("%g should be 123.456\n", 123.456);
    TEST_SAME("%g should be 1e+06\n", 1000000.0);
    TEST_SAME("%g should be 10\n", 10.0);
    TEST_SAME("%g should be 0.02\n", 0.02);
    TEST_SAME("%Le\n", 0x1p-1022L);
    TEST_SAME("%Le\n", 0x1.0000000000001p-1022L);
    TEST_SAME("%Le\n", 0x1.00000000001e7p-1022L);
    TEST_SAME("%Le\n", 0x1.fffffffffffffp-1022L);
    TEST_SAME("%Le\n", 0x1p-1021L);
    TEST_SAME("%Le\n", 0x1.00000000000008p-1021L);
    TEST_SAME("%Le\n", 0x1.0000000000001p-1021L);
    TEST_SAME("%Le\n", 0x1.00000000000018p-1021L);
    TEST_SAME("%Le\n", 0x1.0000000000000f8p-1017L);
    TEST_SAME("%Le\n", 0x1.0000000000001p-1017L);
    TEST_SAME("%Le\n", 0x1.000000000000108p-1017L);
    TEST_SAME("%Le\n", 0x1.000000000000dcf8p-1013L);
    TEST_SAME("%Le\n", 0x1.000000000000ddp-1013L);
    TEST_SAME("%Le\n", 0x1.000000000000dd08p-1013L);
    TEST_SAME("%Le\n", 0x1.ffffffffffffffffffffffffffp-1L);
    TEST_SAME("%Le\n", 0x1.ffffffffffffffffffffffffff8p-1L);
    TEST_SAME("%Le\n", 0x1p+0L);
    TEST_SAME("%Le\n", 0x1.000000000000000000000000008p+0L);
    TEST_SAME("%Le\n", 0x1.00000000000000000000000001p+0L);
    TEST_SAME("%Le\n", 0x1.000000000000000000000000018p+0L);
    TEST_SAME("%Le\n", 0x1.23456789abcdef123456789abc8p+0L);
    TEST_SAME("%Le\n", 0x1.23456789abcde7123456789abc8p+0L);
    TEST_SAME("%Le\n", 0x1.23456789abcdef123456789abc8p+64L);
    TEST_SAME("%Le\n", 0x1.23456789abcde7123456789abc8p+64L);
    TEST_SAME("%Le\n", 0x1.123456789abcdef123456789p-969L);
}

static void n_test(void) {
    char arr1[100]; char arr2[100];
    int count1, count2;
    int rt1 = CA_snprintf(arr1, 100, "%12.3f%n", 123.3456789, &count1);
    int rt2 = snprintf(arr2, 100, "%12.3f%n", 123.3456789, &count2);
    DEBUG_ASSERT(count1 == count2);
    DEBUG_ASSERT(count1 == rt1);
    DEBUG_ASSERT(rt1 == rt2);
}

CLANG_DIAGNOSTIC_POP

#endif
