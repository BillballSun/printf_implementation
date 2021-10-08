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
*    printf_core.h 2019/10/15
*/

#ifndef printf_core_h
#define printf_core_h

#include <stdio.h>
#include <stddef.h>

#ifdef __GNUC__
#define SNPRINTF_ATTR __attribute__ ((format(printf, 3, 4)))
#define  FPRINTF_ATTR __attribute__ ((format(printf, 2, 3)))
#define   PRINTF_ATTR __attribute__ ((format(printf, 1, 2)))
#else
#define SNPRINTF_ATTR
#define  FPRINTF_ATTR
#define   PRINTF_ATTR
#endif

#pragma mark - Decleraion

int CA_snprintf(char * _Nullable store, size_t n, const char * _Nonnull format, ...) SNPRINTF_ATTR;

int CA_fprintf(FILE * _Nonnull fp, const char * _Nonnull format, ...) FPRINTF_ATTR;

int CA_printf(const char * _Nonnull format, ...) PRINTF_ATTR;

#ifdef DEBUG

#endif

#endif /* printf_core_h */
