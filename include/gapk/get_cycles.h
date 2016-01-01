/*
 * Copyright (c) 2005 Mellanox Technologies Ltd.,
 *	     (c) 2005 Harald Welte <laforge@gnumonks.org>, All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * $Id$
 */

#ifndef GET_CLOCK_H
#define GET_CLOCK_H

#if 0

#define _POSIX_C_SOURCE 199506L
#include <unistd.h>
#include <time.h>

/* Ideally we would be using clock_getres() and clock_gettime(). 
 * glibc manpage says CLOCK_PROCESS_CPUTIME_ID is only defined if it is
 * actually present.  however, on ppc64 it is defined but not implemented. */
#ifdef CLOCK_PROCESS_CPUTIME_ID
typedef long cycles_t;
static inline cycles_t get_cycles()
{
	struct timespec ts;

#if defined (__x86_64__) || defined(__i386__)
	asm volatile ("cpuid" : : : "eax", "ebx", "ecx", "edx" ); /* flush pipeline */
#endif
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	return ts.tv_nsec;
}
#endif

#endif

#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
typedef unsigned long long cycles_t;
static inline cycles_t get_cycles()
{
	unsigned low, high;
	unsigned long long val;
	asm volatile ("cpuid" : : : "eax", "ebx", "ecx", "edx" ); /* flush pipeline */
	asm volatile ("rdtsc" : "=a" (low), "=d" (high));
	val = high;
	val = (val << 32) | low;
	return val;
}
#elif defined(__PPC64__)
/* Note: only PPC CPUs which have mftb instruction are supported. */
typedef unsigned long long cycles_t;
static inline cycles_t get_cycles()
{
	cycles_t ret;

	asm volatile ("mftb %0" : "=r" (ret) : );
	return ret;
}
#elif defined(__sparc__)
/* Note: only sparc64 supports this register */
typedef unsigned long long cycles_t;
#define TICK_PRIV_BIT  (1ULL << 63)
static inline cycles_t get_cycles()
{
	cycles_t ret;

#if defined(__sparcv9) || defined(__arch64__)
	asm volatile ("rd %%tick, %0" : "=r" (ret));
#else
	asm volatile ("rd %%tick, %%g1\n\t"
		      "srlx %%g1, 32, %H0\n\t"
		      "srl  %%g1,  0, %L0"
		      : "=r" (ret)
		      : /* no inputs */
		      : "g1");
#endif
	return ret & ~TICK_PRIV_BIT;
}
#elif defined(__PPC__)
#define CPU_FTR_601                  0x00000100
typedef unsigned long cycles_t;
static inline cycles_t get_cycles()
{
	cycles_t ret;

	asm volatile (
		"98:	mftb %0\n"
		"99:\n"
		".section __ftr_fixup,\"a\"\n"
		"	.long %1\n"
		"	.long 0\n"
		"	.long 98b\n"
		"	.long 99b\n"
		".previous"
		: "=r" (ret) : "i" (CPU_FTR_601));
	return ret;
}
#elif defined(__ia64__) || defined(__mips__) || \
      defined(__s390__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
/* PPC64 has mftb */
#include <asm/timex.h>
#else
#warning get_cycles not implemented for this architecture: attempt asm/timex.h
#include <asm/timex.h>
#endif

extern double get_cpu_mhz(void);

#endif /* GET_CLOCK_H */
