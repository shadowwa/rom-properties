/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * TextFuncs_libc.c: Reimplementations of libc functions that aren't       *
 * present on this system.                                                 *
 *                                                                         *
 * Copyright (c) 2009-2019 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "TextFuncs_libc.h"

/** Reimplementations of libc functions that aren't present on this system. **/

#ifndef HAVE_STRNLEN
/**
 * String length with limit. (8-bit strings)
 * @param str The string itself
 * @param maxlen Maximum length of the string
 * @returns equivivalent to min(strlen(str), maxlen) without buffer overruns
 */
size_t strnlen(const char *str, size_t maxlen)
{
	const char *ptr = memchr(str, 0, maxlen);
	if (!ptr)
		return maxlen;
	return ptr - str;
}
#endif /* HAVE_STRNLEN */

#ifndef HAVE_MEMMEM
/**
 * Find a string within a block of memory.
 * @param haystack Block of memory.
 * @param haystacklen Length of haystack.
 * @param needle String to search for.
 * @param needlelen Length of needle.
 * @return Location of needle in haystack, or NULL if not found.
 */
void *memmem(const void *haystack, size_t haystacklen,
	     const void *needle, size_t needlelen)
{
	// Reference: https://opensource.apple.com/source/Libc/Libc-1044.1.2/string/FreeBSD/memmem.c
	// NOTE: haystack was originally 'l'; needle was originally 's'.
	register const char *cur, *last;
	const char *cl = (const char *)haystack;
	const char *cs = (const char *)needle;

	/* we need something to compare */
	if (haystacklen == 0 || needlelen == 0)
		return NULL;

	/* "s" must be smaller or equal to "l" */
	if (haystacklen < needlelen)
		return NULL;

	/* special case where s_len == 1 */
	if (needlelen == 1)
		return (void*)memchr(haystack, (int)*cs, needlelen);

	/* the last position where its possible to find "s" in "l" */
	last = (const char *)cl + haystacklen - needlelen;

	for (cur = (const char *)cl; cur <= last; cur++) {
		if (cur[0] == cs[0] && memcmp(cur, cs, needlelen) == 0)
			return (void*)cur;
	}

	return NULL;
}
#endif /* HAVE_MEMMEM */
