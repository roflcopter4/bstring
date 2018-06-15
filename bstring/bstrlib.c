/* Copyright 2002-2010 Paul Hsieh
 * This file is part of Bstrlib.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. Neither the name of bstrlib nor the names of its contributors may be
 *       used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * GNU General Public License Version 2 (the "GPL").
 */

/*
 * This file is the core module for implementing the bstring * functions.
 */

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined(_MSC_VER)
   /* These warnings from MSVC++ are totally pointless. */
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <err.h>

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bstrlib.h"

#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

/* Just a length safe wrapper for memmove. */
#define b_BlockCopy(D, buf, blen)                    \
        do {                                         \
                if ((blen) > 0)                      \
                        memmove((D), (buf), (blen)); \
        } while (0);

/* There were some pretty horrifying if statements in this file. I've tried to
 * make them at least somewhat saner with these macros that at least explain
 * what the checks are trying to accomplish. */
#define IS_NULL(BSTR)  (!(BSTR) || !(BSTR)->data)
#define INVALID(BSTR)  (IS_NULL(BSTR) || (BSTR)->slen < 0)
#define NO_WRITE(BSTR) ((BSTR)->mlen <= 0 || (BSTR)->mlen < (BSTR)->slen)

/* #if defined(DEBUG) && 0
#  define RUNTIME_ERROR errx(1, "Runtime error at file %s, line %d", __FILE__, __LINE__)
#  define RETURN_NULL errx(1, "Null return at file %s, line %d", __FILE__, __LINE__)
#else */
#  define RUNTIME_ERROR return BSTR_ERR
#  define RETURN_NULL return NULL
/* #endif */

/* #define RUNTIME_ERROR ({warnx("Runtime error at file %s, line %d", __FILE__, __LINE__); return BSTR_ERR;}) */
/* #define RETURN_NULL ({warnx("Null return at file %s, line %d", __FILE__, __LINE__); return NULL;}) */


/**
 * Compute the snapped size for a given requested size.
 * By snapping to powers of 2 like this, repeated reallocations are avoided.
 */
static int
snapUpSize(int i)
{
        if (i < 8)
                i = 8;
        else {
                unsigned j = (unsigned)i;
                j |= (j >> 1);
                j |= (j >> 2);
                j |= (j >> 4);
                j |= (j >> 8);  /* Ok, since int >= 16 bits */
#if (UINT_MAX != 0xFFFF)
                j |= (j >> 16); /* For 32 bit int systems */
#  if (UINT_MAX > 0xFFFFFFFFllu)
                j |= (j >> 32); /* For 64 bit int systems */
#  endif
#endif
                /* Least power of two greater than i */
                j++;
                if ((int)j >= i)
                        i = (int)j;
        }
        return i;
}


int
b_alloc(bstring *bstr, const int olen)
{
        if (INVALID(bstr) || NO_WRITE(bstr) || olen <= 0)
                RUNTIME_ERROR;
        /* assert(!(INVALID(bstr) || NO_WRITE(bstr) || olen <= 0)); */

        if (olen >= bstr->mlen) {
                uchar *tmp;
                int len = snapUpSize(olen);
                if (len <= bstr->mlen)
                        return BSTR_OK;

                /* Assume probability of a non-moving realloc is 0.125 */
                if (7 * bstr->mlen < 8 * bstr->slen) {
                        /* If slen is close to mlen in size then use realloc
                         * to reduce the memory defragmentation */
                retry:
                        tmp = realloc(bstr->data, len);
                        if (!tmp) {
                                /* Since we failed, try mallocating the tighest
                                 * possible allocation */
                                len = olen;
                                tmp = realloc(bstr->data, len);
                                if (!tmp)
                                        RUNTIME_ERROR;
                        }
                } else {
                        /* If slen is not close to mlen then avoid the penalty
                         * of copying the extra bytes that are mallocated, but
                         * not considered part of the string */
                        tmp = malloc(len);

                        /* Perhaps there is no available memory for the two
                         * mallocations to be in memory at once */
                        if (!tmp)
                                goto retry;

                        if (bstr->slen)
                                memcpy(tmp, bstr->data, bstr->slen);
                        free(bstr->data);
                }
                bstr->data = tmp;
                bstr->mlen = len;
                bstr->data[bstr->slen] = (uchar)'\0';
        }
        return BSTR_OK;
}


int
b_allocmin(bstring *bstr, int len)
{
        uchar *buf;
        if (IS_NULL(bstr) || (bstr->slen + 1) < 0 || NO_WRITE(bstr) || len <= 0)
                RUNTIME_ERROR;

        if (len < bstr->slen + 1)
                len = bstr->slen + 1;

        if (len != bstr->mlen) {
                buf = realloc(bstr->data, (size_t)len);
                if (!buf)
                        RUNTIME_ERROR;
                buf[bstr->slen] = (uchar)'\0';
                bstr->data = buf;
                bstr->mlen = len;
        }

        return BSTR_OK;
}


bstring *
b_fromcstr(const char *const str)
{
        bstring *bstr;
        if (!str)
                RETURN_NULL;

        const size_t size = strlen(str);
        const int    max  = snapUpSize((int)(size + (2 - (size != 0))));

        if (max <= (int)size)
                RETURN_NULL;
        if (!(bstr = malloc(sizeof *bstr)))
                RETURN_NULL;

        bstr->slen = (int)size;
        bstr->mlen = max;
        if (!(bstr->data = malloc(bstr->mlen))) {
                free(bstr);
                RETURN_NULL;
        }

        memcpy(bstr->data, str, size + 1);
        return bstr;
}


bstring *
b_fromcstr_alloc(const int mlen, const char *const str)
{
        bstring *bstr;
        if (!str)
                RETURN_NULL;

        const size_t size = strlen(str);
        int          max  = snapUpSize((int)(size + (2 - (size != 0))));

        if (max <= (int)size)
                RETURN_NULL;
        if (!(bstr = malloc(sizeof *bstr)))
                RETURN_NULL;

        bstr->slen = (int)size;
        if (max < mlen)
                max = mlen;

        bstr->mlen = max;
        bstr->data = malloc(bstr->mlen);
        if (!bstr->data) {
                free(bstr);
                RETURN_NULL;
        }

        memcpy(bstr->data, str, size + 1);
        return bstr;
}


bstring *
b_blk2bstr(const void *blk, const int len)
{
        bstring *bstr;
        if (!blk || len < 0)
                RETURN_NULL;
        if (!(bstr = malloc(sizeof(bstring))))
                RETURN_NULL;

        bstr->slen = len;
        const int i = snapUpSize(len + (2 - (len != 0)));
        bstr->mlen = i;
        bstr->data = malloc(bstr->mlen);

        if (!bstr->data) {
                free(bstr);
                RETURN_NULL;
        }

        if (len > 0)
                memcpy(bstr->data, blk, len);
        bstr->data[len] = (uchar)'\0';

        return bstr;
}


char *
b_bstr2cstr(const bstring *bstr, const char nul)
{
        char *buf;

        if (INVALID(bstr))
                RETURN_NULL;
        if (!(buf = malloc(bstr->slen + 1)))
                return buf;
        for (int i = 0; i < bstr->slen; ++i)
                buf[i] = (bstr->data[i] == '\0') ? nul : (char)(bstr->data[i]);

        buf[bstr->slen] = (uchar)'\0';

        return buf;
}


int
b_cstrfree(char *buf)
{
        free(buf);
        return BSTR_OK;
}


int
b_concat(bstring *b0, const bstring *b1)
{
        int len, d;
        bstring *aux = (bstring *)b1;

        if (INVALID(b0) || NO_WRITE(b0) || INVALID(b1))
                RUNTIME_ERROR;

        d = b0->slen;
        len = b1->slen;
        if ((d | (b0->mlen - d) | len | (d + len)) < 0)
                RUNTIME_ERROR;

        if (b0->mlen <= d + len + 1) {
                const ptrdiff_t pd = b1->data - b0->data;
                if (0 <= pd && pd < b0->mlen) {
                        aux = b_strcpy(b1);
                        if (!aux)
                                RUNTIME_ERROR;
                }
                if (b_alloc(b0, d + len + 1) != BSTR_OK) {
                        if (aux != b1)
                                b_destroy(aux);
                        RUNTIME_ERROR;
                }
        }

        b_BlockCopy(&b0->data[d], &aux->data[0], len);
        b0->data[d + len] = (uchar)'\0';
        b0->slen = d + len;
        if (aux != b1)
                b_destroy(aux);

        return BSTR_OK;
}


int
b_conchar(bstring *bstr, const char c)
{
        if (!bstr)
                RUNTIME_ERROR;
        const int d = bstr->slen;
        if ((d | (bstr->mlen - d)) < 0 || b_alloc(bstr, d + 2) != BSTR_OK)
                RUNTIME_ERROR;

        bstr->data[d] = (uchar)c;
        bstr->data[d + 1] = (uchar)'\0';
        bstr->slen++;

        return BSTR_OK;
}


int
b_catcstr(bstring *bstr, const char *buf)
{
        if (INVALID(bstr) || NO_WRITE(bstr) || !buf)
                RUNTIME_ERROR;

        /* Optimistically concatenate directly */
        int i;
        const int blen = bstr->mlen - bstr->slen;
        char *d = (char *)(&bstr->data[bstr->slen]);

        for (i = 0; i < blen; ++i) {
                if ((*d++ = *buf++) == '\0') {
                        bstr->slen += i;
                        return BSTR_OK;
                }
        }

        bstr->slen += i;

        /* Need to explicitely resize and concatenate tail */
        return b_catblk(bstr, buf, strlen(buf));
}


int
b_catblk(bstring *bstr, const void *buf, const int len)
{
        int nl;
        if (INVALID(bstr) || NO_WRITE(bstr) || !buf || len < 0)
                RUNTIME_ERROR;
        if (0 > (nl = bstr->slen + len)) /* Overflow? */
                RUNTIME_ERROR;
        if (bstr->mlen <= nl && 0 > b_alloc(bstr, nl + 1))
                RUNTIME_ERROR;

        b_BlockCopy(&bstr->data[bstr->slen], buf, len);
        bstr->slen = nl;
        bstr->data[nl] = (uchar)'\0';

        return BSTR_OK;
}


bstring *
b_strcpy(const bstring *bstr)
{
        bstring *b0;

        /* Attempted to copy an invalid string? */
        if (INVALID(bstr))
                RETURN_NULL;
        /* assert(!(INVALID(bstr))); */

        if (!(b0 = malloc(sizeof(bstring))))
                RETURN_NULL;

        int i    = bstr->slen;
        int j    = snapUpSize(i + 1);
        b0->data = malloc(j);

        if (!b0->data) {
                j = i + 1;
                b0->data = malloc(j);
                if (!b0->data) {
                        /* Unable to mallocate memory for string data */
                        free(b0);
                        RETURN_NULL;
                }
        }
        b0->mlen = j;
        b0->slen = i;

        if (i)
                memcpy(b0->data, bstr->data, i);
        b0->data[b0->slen] = (uchar)'\0';

        return b0;
}


int
b_assign(bstring *a, const bstring *bstr)
{
        if (INVALID(bstr))
                RUNTIME_ERROR;
        if (bstr->slen != 0) {
                if (b_alloc(a, bstr->slen) != BSTR_OK)
                        RUNTIME_ERROR;
                memmove(a->data, bstr->data, bstr->slen);
        } else if (INVALID(a) || a->mlen < a->slen || a->mlen == 0)
                        RUNTIME_ERROR;

        a->data[bstr->slen] = (uchar)'\0';
        a->slen = bstr->slen;

        return BSTR_OK;
}


int
b_assign_midstr(bstring *a, const bstring *bstr, int left, int len)
{
        if (INVALID(bstr))
                RUNTIME_ERROR;
        if (left < 0) {
                len += left;
                left = 0;
        }
        if (len > bstr->slen - left)
                len = bstr->slen - left;
        if (INVALID(a) || a->mlen < a->slen || a->mlen == 0)
                RUNTIME_ERROR;

        if (len > 0) {
                if (b_alloc(a, len) != BSTR_OK)
                        RUNTIME_ERROR;
                memmove(a->data, bstr->data + left, len);
                a->slen = len;
        } else
                a->slen = 0;

        a->data[a->slen] = (uchar)'\0';

        return BSTR_OK;
}


int
b_assign_cstr(bstring *a, const char *str)
{
        int i;
        if (INVALID(a) || a->mlen < a->slen || a->mlen == 0 || !str)
                RUNTIME_ERROR;

        for (i = 0; i < a->mlen; ++i) {
                if ('\0' == (a->data[i] = str[i])) {
                        a->slen = i;
                        return BSTR_OK;
                }
        }

        a->slen = i;
        const size_t len = strlen(str + i);
        if (len > INT_MAX || i + len + 1 > INT_MAX ||
            0 > b_alloc(a, (int)(i + len + 1)))
                RUNTIME_ERROR;
        b_BlockCopy(a->data + i, str + i, (size_t)len + 1);
        a->slen += (int)len;

        return BSTR_OK;
}


int
b_assign_blk(bstring *a, const void *buf, const int len)
{
        if (INVALID(a) || a->mlen < a->slen || a->mlen == 0 || !buf || len + 1 < 1)
                RUNTIME_ERROR;
        if (len + 1 > a->mlen && 0 > b_alloc(a, len + 1))
                RUNTIME_ERROR;
        b_BlockCopy(a->data, buf, len);
        a->data[len] = (uchar)'\0';
        a->slen = len;

        return BSTR_OK;
}


int
b_trunc(bstring *bstr, const int n)
{
        if (n < 0 || INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;
        if (bstr->slen > n) {
                bstr->slen = n;
                bstr->data[n] = (uchar)'\0';
        }

        return BSTR_OK;
}


#define upcase(c)   (toupper((uchar)(c)))
#define downcase(c) (tolower((uchar)(c)))
#define wspace(c)   (isspace((uchar)(c)))

int
b_toupper(bstring *bstr)
{
        int i, len;
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;
        for (i = 0, len = bstr->slen; i < len; ++i)
                bstr->data[i] = (uchar)upcase(bstr->data[i]);

        return BSTR_OK;
}


int
b_tolower(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;
        for (int i = 0, len = bstr->slen; i < len; ++i)
                bstr->data[i] = (uchar)downcase(bstr->data[i]);

        return BSTR_OK;
}


int
b_stricmp(const bstring *b0, const bstring *b1)
{
        int n;
        if (INVALID(b0) || INVALID(b1))
                return SHRT_MIN;
        if ((n = b0->slen) > b1->slen)
                n = b1->slen;
        else if (b0->slen == b1->slen && b0->data == b1->data)
                return BSTR_OK;

        for (int i = 0; i < n; ++i) {
                const int v = (char)downcase(b0->data[i]) - (char)downcase(b1->data[i]);
                if (v != 0)
                        return v;
        }

        if (b0->slen > n) {
                const int v = (char)downcase(b0->data[n]);
                if (v)
                        return v;
                return UCHAR_MAX + 1;
        }

        if (b1->slen > n) {
                const int v = -(char)downcase(b1->data[n]);
                if (v)
                        return v;
                return -(int)(UCHAR_MAX + 1);
        }

        return BSTR_OK;
}


int
b_strnicmp(const bstring *b0, const bstring *b1, const int n)
{
        if (INVALID(b0) || INVALID(b1) || n < 0)
                return SHRT_MIN;
        int v, m = n;

        if (m > b0->slen)
                m = b0->slen;
        if (m > b1->slen)
                m = b1->slen;
        if (b0->data != b1->data) {
                for (int i = 0; i < m; ++i) {
                        v = (char)downcase(b0->data[i]);
                        v -= (char)downcase(b1->data[i]);
                        if (v != 0)
                                return b0->data[i] - b1->data[i];
                }
        }

        if (n == m || b0->slen == b1->slen)
                return BSTR_OK;
        if (b0->slen > m) {
                v = (char)downcase(b0->data[m]);
                if (v)
                        return v;
                return UCHAR_MAX + 1;
        }
        v = -(char)downcase(b1->data[m]);
        if (v)
                return v;

        return -(int)(UCHAR_MAX + 1);
}


int
b_iseq_caseless(const bstring *b0, const bstring *b1)
{
        if (INVALID(b0) || INVALID(b1))
                RUNTIME_ERROR;
        if (b0->slen != b1->slen)
                return BSTR_OK;
        if (b0->data == b1->data || b0->slen == 0)
                return 1;

        for (int i = 0, n = b0->slen; i < n; ++i) {
                if (b0->data[i] != b1->data[i]) {
                        const uchar c = (uchar)downcase(b0->data[i]);
                        if (c != (uchar)downcase(b1->data[i]))
                                return 0;
                }
        }

        return 1;
}


int
b_is_stem_eq_caseless_blk(const bstring *b0, const void *blk, const int len)
{
        if (INVALID(b0) || !blk || len < 0)
                RUNTIME_ERROR;
        if (b0->slen < len)
                return BSTR_OK;
        if (b0->data == (const uchar *)blk || len == 0)
                return 1;

        for (int i = 0; i < len; ++i)
                if (b0->data[i] != ((const uchar *)blk)[i])
                        if (downcase(b0->data[i]) != downcase(((const uchar *)blk)[i]))
                                return 0;

        return 1;
}


int
b_ltrimws(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;
        for (int len = bstr->slen, i = 0; i < len; ++i)
                if (!wspace(bstr->data[i]))
                        return b_delete(bstr, 0, i);

        bstr->data[0] = (uchar)'\0';
        bstr->slen = 0;

        return BSTR_OK;
}


int
b_rtrimws(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;
        for (int i = bstr->slen - 1; i >= 0; --i) {
                if (!wspace(bstr->data[i])) {
                        if (bstr->mlen > i)
                                bstr->data[i + 1] = (uchar)'\0';
                        bstr->slen = i + 1;
                        return BSTR_OK;
                }
        }

        bstr->data[0] = (uchar)'\0';
        bstr->slen = 0;

        return BSTR_OK;
}


int
b_trimws(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;
        for (int i = bstr->slen - 1; i >= 0; --i) {
                if (!wspace(bstr->data[i])) {
                        int j;
                        if (bstr->mlen > i)
                                bstr->data[i + 1] = (uchar)'\0';
                        bstr->slen = i + 1;
                        for (j = 0; wspace(bstr->data[j]); j++)
                                ;
                        return b_delete(bstr, 0, j);
                }
        }

        bstr->data[0] = (uchar)'\0';
        bstr->slen = 0;

        return BSTR_OK;
}


int
b_iseq(const bstring *b0, const bstring *b1)
{
        if (INVALID(b0) || INVALID(b1))
                RUNTIME_ERROR;
        if (b0->slen != b1->slen)
                return 0;
        if (b0->data == b1->data || b0->slen == 0)
                return 1;

        return !memcmp(b0->data, b1->data, b0->slen);
}


int
b_is_stem_eq_blk(const bstring *b0, const void *blk, const int len)
{
        if (INVALID(b0) || !blk || len < 0)
                RUNTIME_ERROR;
        if (b0->slen < len)
                return 0;
        if (b0->data == (const uchar *)blk || len == 0)
                return 1;
        for (int i = 0; i < len; ++i)
                if (b0->data[i] != ((const uchar *)blk)[i])
                        return 0;

        return 1;
}


int
b_iseq_cstr(const bstring *bstr, const char *buf)
{
        int i;
        if (!buf || INVALID(bstr))
                RUNTIME_ERROR;
        for (i = 0; i < bstr->slen; ++i)
                if (buf[i] == '\0' || bstr->data[i] != (uchar)buf[i])
                        return 0;

        return buf[i] == '\0';
}


int
b_iseq_cstr_caseless(const bstring *bstr, const char *buf)
{
        int i;
        if (!buf || INVALID(bstr))
                RUNTIME_ERROR;
        for (i = 0; i < bstr->slen; ++i)
                if (buf[i] == '\0' || (bstr->data[i] != (uchar)buf[i] && downcase(bstr->data[i]) != (uchar)downcase(buf[i])))
                        return 0;

        return buf[i] == '\0';
}


int
b_strcmp(const bstring *b0, const bstring *b1)
{
        if (INVALID(b0) || INVALID(b1))
                return SHRT_MIN;

        /* Return zero if two strings are both empty or point to the same data. */
        if (b0->slen == b1->slen && (b0->data == b1->data || b0->slen == 0))
                return 0;

        const int n = MIN(b0->slen, b1->slen);

        for (int i = 0; i < n; ++i) {
                int v = ((char)b0->data[i]) - ((char)b1->data[i]);
                if (v != 0)
                        return v;
                if (b0->data[i] == (uchar)'\0')
                        return 0;
        }

        if (b0->slen > n)
                return 1;
        if (b1->slen > n)
                return -1;

        return 0;

        /* return memcmp(b0->data, b1->data, n); */
}


int
b_strncmp(const bstring *b0, const bstring *b1, const int n)
{
        if (INVALID(b0) || INVALID(b1))
                return SHRT_MIN;

        const int m = MIN(n, MIN(b0->slen, b1->slen));

        if (b0->data != b1->data) {
                for (int i = 0; i < m; ++i) {
                        int v = ((char)b0->data[i]) - ((char)b1->data[i]);
                        if (v != 0)
                                return v;
                        if (b0->data[i] == (uchar)'\0')
                                return 0;
                }
        }

        if (n == m || b0->slen == b1->slen)
                return 0;
        if (b0->slen > m)
                return 1;

        /* return memcmp(b0->data, b1->data, m); */
}


bstring *
b_midstr(const bstring *bstr, int left, int len)
{
        if (INVALID(bstr))
                RETURN_NULL;
        if (left < 0) {
                len += left;
                left = 0;
        }
        if (len > bstr->slen - left)
                len = bstr->slen - left;
        if (len <= 0)
                return b_fromcstr("");

        return b_blk2bstr(bstr->data + left, len);
}


int
b_delete(bstring *bstr, int pos, int len)
{
        /* Clamp to left side of bstring * */
        if (pos < 0) {
                len += pos;
                pos = 0;
        }
        if (len < 0 || INVALID(bstr) ||
            NO_WRITE(bstr))
                RUNTIME_ERROR;
        if (len > 0 && pos < bstr->slen) {
                if (pos + len >= bstr->slen)
                        bstr->slen = pos;
                else {
                        b_BlockCopy((char *)(bstr->data + pos),
                                    (char *)(bstr->data + pos + len),
                                    bstr->slen - (pos + len));
                        bstr->slen -= len;
                }
                bstr->data[bstr->slen] = (uchar)'\0';
        }

        return BSTR_OK;
}


int
b_destroy(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;
        free(bstr->data);

        /* In case there is any stale usage, there is one more chance to notice this error. */
        *bstr = (bstring){ -1, -__LINE__, NULL };
        free(bstr);

        return BSTR_OK;
}


int
b_instr(const bstring *b1, const int pos, const bstring *b2)
{
        int j, ii, ll, lf;
        uchar *d0;
        uchar c0;
        register uchar *d1;
        register uchar c1;
        register int i;
        if (INVALID(b1) || INVALID(b2))
                RUNTIME_ERROR;
        if (b1->slen == pos)
                return (b2->slen == 0) ? pos : BSTR_ERR;
        if (b1->slen < pos || pos < 0)
                RUNTIME_ERROR;
        if (b2->slen == 0)
                return pos;

        /* No space to find such a string? */
        if ((lf = b1->slen - b2->slen + 1) <= pos)
                RUNTIME_ERROR;

        /* An obvious alias case */
        if (b1->data == b2->data && pos == 0)
                return 0;
        i = pos;
        d0 = b2->data;
        d1 = b1->data;
        ll = b2->slen;

        /* Peel off the b2->slen == 1 case */
        c0 = d0[0];
        if (1 == ll) {
                for (; i < lf; ++i)
                        if (c0 == d1[i])
                                return i;
                RUNTIME_ERROR;
        }

        c1 = c0;
        j = 0;
        lf = b1->slen - 1;
        ii = -1;
        if (i < lf) {
                do {
                        /* Unrolled current character test */
                        if (c1 != d1[i]) {
                                if (c1 != d1[1 + i]) {
                                        i += 2;
                                        continue;
                                }
                                i++;
                        }

                        /* Take note if this is the start of a potential
                         * match */
                        if (0 == j)
                                ii = i;

                        /* Shift the test character down by one */
                        j++;
                        i++;

                        /* If this isn't past the last character continue */
                        if (j < ll) {
                                c1 = d0[j];
                                continue;
                        }
                N0:
                        /* If no characters mismatched, then we matched */
                        if (i == ii + j)
                                return ii;

                        /* Shift back to the beginning */
                        i -= j;
                        j = 0;
                        c1 = c0;
                } while (i < lf);
        }

        /* Deal with last case if unrolling caused a misalignment */
        if (i == lf && ll == j + 1 && c1 == d1[i])
                goto N0;

        RUNTIME_ERROR;
}


int
b_instrr(const bstring *b1, const int pos, const bstring *b2)
{
        int blen;
        uchar *d0, *d1;
        if (INVALID(b1) || INVALID(b2))
                RUNTIME_ERROR;
        if (b1->slen == pos && b2->slen == 0)
                return pos;
        if (b1->slen < pos || pos < 0)
                RUNTIME_ERROR;
        if (b2->slen == 0)
                return pos;

        /* Obvious alias case */
        if (b1->data == b2->data && pos == 0 && b2->slen <= b1->slen)
                return 0;

        int i = pos;
        if ((blen = b1->slen - b2->slen) < 0)
                RUNTIME_ERROR;

        /* If no space to find such a string then snap back */
        if (blen + 1 <= i)
                i = blen;

        int j = 0;
        d0    = b2->data;
        d1    = b1->data;
        blen  = b2->slen;

        for (;;) {
                if (d0[j] == d1[i + j]) {
                        j++;
                        if (j >= blen)
                                return i;
                } else {
                        i--;
                        if (i < 0)
                                break;
                        j = 0;
                }
        }

        RUNTIME_ERROR;
}


int
b_instr_caseless(const bstring *b1, const int pos, const bstring *b2)
{
        int j, i, blen, ll;
        uchar *d0, *d1;
        if (INVALID(b1) || INVALID(b2))
                RUNTIME_ERROR;
        if (b1->slen == pos)
                return (b2->slen == 0) ? pos : BSTR_ERR;
        if (b1->slen < pos || pos < 0)
                RUNTIME_ERROR;
        if (b2->slen == 0)
                return pos;
        blen = b1->slen - b2->slen + 1;

        /* No space to find such a string? */
        if (blen <= pos)
                RUNTIME_ERROR;

        /* An obvious alias case */
        if (b1->data == b2->data && pos == 0)
                return BSTR_OK;
        i = pos;
        j = 0;
        d0 = b2->data;
        d1 = b1->data;
        ll = b2->slen;

        for (;;) {
                if (d0[j] == d1[i + j] ||
                    downcase(d0[j]) == downcase(d1[i + j])) {
                        j++;
                        if (j >= ll)
                                return i;
                } else {
                        i++;
                        if (i >= blen)
                                break;
                        j = 0;
                }
        }

        RUNTIME_ERROR;
}


int
b_instrr_caseless(const bstring *b1, const int pos, const bstring *b2)
{
        if (INVALID(b1) || INVALID(b2))
                RUNTIME_ERROR;
        if (b1->slen == pos && b2->slen == 0)
                return pos;
        if (b1->slen < pos || pos < 0)
                RUNTIME_ERROR;
        if (b2->slen == 0)
                return pos;

        /* Obvious alias case */
        if (b1->data == b2->data && pos == 0 && b2->slen <= b1->slen)
                return BSTR_OK;
        int blen, i = pos;
        if ((blen = b1->slen - b2->slen) < 0)
                RUNTIME_ERROR;

        /* If no space to find such a string then snap back */
        if ((blen + 1) <= i)
                i = blen;
        int j = 0;
        uchar *d0 = b2->data;
        uchar *d1 = b1->data;
        blen = b2->slen;

        for (;;) {
                if (d0[j] == d1[i + j] ||
                    downcase(d0[j]) == downcase(d1[i + j])) {
                        j++;
                        if (j >= blen)
                                return i;
                } else {
                        i--;
                        if (i < 0)
                                break;
                        j = 0;
                }
        }

        RUNTIME_ERROR;
}


int
b_strchrp(const bstring *bstr, const int ch, const int pos)
{
        if (IS_NULL(bstr) || bstr->slen <= pos || pos < 0)
                RUNTIME_ERROR;

        uchar *p = (uchar *)memchr((bstr->data + pos), (uchar)ch, (bstr->slen - pos));
        if (p)
                return (int)(p - bstr->data);

        RUNTIME_ERROR;
}


int
b_strrchrp(const bstring *bstr, int c, int pos)
{
        if (IS_NULL(bstr) || bstr->slen <= pos || pos < 0)
                RUNTIME_ERROR;
        for (int i = pos; i >= 0; --i)
                if (bstr->data[i] == (uchar)c)
                        return i;

        RUNTIME_ERROR;
}


#ifndef BSTRLIB_AGGRESSIVE_MEMORY_FOR_SPEED_TRADEOFF
#  define LONG_LOG_BITS_QTY (3)
#  define LONG_BITS_QTY (1 << LONG_LOG_BITS_QTY)
#  define LONG_TYPE uchar
#  define CFCLEN ((1 << CHAR_BIT) / LONG_BITS_QTY)
   struct char_field {
           LONG_TYPE content[CFCLEN];
   };
#  define testInCharField(cf, c)                   \
        ((cf)->content[(c) >> LONG_LOG_BITS_QTY] & \
         ((1ll) << ((c) & (LONG_BITS_QTY - 1))))

#  define setInCharField(cf, idx)                                   \
        do {                                                        \
                unsigned int c = (unsigned int)(idx);               \
                (cf)->content[c >> LONG_LOG_BITS_QTY] |=            \
                    (LONG_TYPE)(1llu << (c & (LONG_BITS_QTY - 1))); \
        } while (0)

#else

#  define CFCLEN (1 << CHAR_BIT)
   struct charField {
           uchar content[CFCLEN];
   };
#  define testInCharField(cf, c)  ((cf)->content[(uchar)(c)])
#  define setInCharField(cf, idx) (cf)->content[(unsigned int)(idx)] = ~0
#endif


/* Convert a bstring * to charField */
static int
build_char_field(struct char_field *cf, const bstring *bstr)
{
        if (IS_NULL(bstr) || bstr->slen <= 0)
                RUNTIME_ERROR;
        memset(cf->content, 0, sizeof(struct char_field));
        for (int i = 0; i < bstr->slen; ++i)
                setInCharField(cf, bstr->data[i]);

        return BSTR_OK;
}


static void
invert_char_field(struct char_field *cf)
{
        for (int i = 0; i < CFCLEN; ++i)
                cf->content[i] = ~cf->content[i];
}


/* Inner engine for binchr */
static int
b_inchrCF(const uchar *data, const int len, const int pos, const struct char_field *cf)
{
        for (int i = pos; i < len; ++i) {
                const uchar c = (uchar)data[i];
                if (testInCharField(cf, c))
                        return i;
        }

        RUNTIME_ERROR;
}


int
b_inchr(const bstring *b0, const int pos, const bstring *b1)
{
        struct char_field chrs;
        if (pos < 0 || INVALID(b0) || INVALID(b1))
                RUNTIME_ERROR;
        if (1 == b1->slen)
                return b_strchrp(b0, b1->data[0], pos);
        if (0 > build_char_field(&chrs, b1))
                RUNTIME_ERROR;

        return b_inchrCF(b0->data, b0->slen, pos, &chrs);
}


/* Inner engine for binchrr */
static int
b_inchrrCF(const uchar *data, const int pos, const struct char_field *cf)
{
        for (int i = pos; i >= 0; --i) {
                const unsigned c = (unsigned)data[i];
                if (testInCharField(cf, c))
                        return i;
        }

        RUNTIME_ERROR;
}


int
b_inchrr(const bstring *b0, int pos, const bstring *b1)
{
        struct char_field chrs;
        if (pos < 0 || INVALID(b0) || !b1)
                RUNTIME_ERROR;
        if (pos == b0->slen)
                pos--;
        if (1 == b1->slen)
                return b_strrchrp(b0, b1->data[0], pos);
        if (0 > build_char_field(&chrs, b1))
                RUNTIME_ERROR;
        return b_inchrrCF(b0->data, pos, &chrs);
}


int
b_ninchr(const bstring *b0, int pos, const bstring *b1)
{
        struct char_field chrs;
        if (pos < 0 || INVALID(b0))
                RUNTIME_ERROR;
        if (build_char_field(&chrs, b1) < 0)
                RUNTIME_ERROR;
        invert_char_field(&chrs);
        return b_inchrCF(b0->data, b0->slen, pos, &chrs);
}


int
b_ninchrr(const bstring *b0, int pos, const bstring *b1)
{
        struct char_field chrs;
        if (pos < 0 || INVALID(b0))
                RUNTIME_ERROR;
        if (pos == b0->slen)
                pos--;
        if (build_char_field(&chrs, b1) < 0)
                RUNTIME_ERROR;
        invert_char_field(&chrs);

        return b_inchrrCF(b0->data, pos, &chrs);
}


int
b_setstr(bstring *b0, int pos, const bstring *b1, uchar fill)
{
        int d, newlen;
        bstring *aux = (bstring *)b1;
        if (pos < 0 || INVALID(b0) || NO_WRITE(b0))
                RUNTIME_ERROR;
        if (b1 && (b1->slen < 0 || !b1->data))
                RUNTIME_ERROR;
        d = pos;

        /* Aliasing case */
        if (aux) {
                ptrdiff_t pd = (ptrdiff_t)(b1->data - b0->data);
                if (pd >= 0 && pd < (ptrdiff_t)b0->mlen)
                        if (!(aux = b_strcpy(b1)))
                                RUNTIME_ERROR;
                d += aux->slen;
        }

        /* Increase memory size if necessary */
        if (b_alloc(b0, d + 1) != BSTR_OK) {
                if (aux != b1)
                        b_destroy(aux);
                RUNTIME_ERROR;
        }
        newlen = b0->slen;

        /* Fill in "fill" character as necessary */
        if (pos > newlen) {
                memset(b0->data + b0->slen, (int)fill, (size_t)(pos - b0->slen));
                newlen = pos;
        }

        /* Copy b1 to position pos in b0. */
        if (aux) {
                b_BlockCopy((char *)(b0->data + pos), (char *)aux->data, aux->slen);
                if (aux != b1)
                        b_destroy(aux);
        }

        /* Indicate the potentially increased size of b0 */
        if (d > newlen)
                newlen = d;
        b0->slen = newlen;
        b0->data[newlen] = (uchar)'\0';

        return BSTR_OK;
}


int
b_insert(bstring *b1, int pos, const bstring *b2, uchar fill)
{
        int d, blen;
        ptrdiff_t pd;
        bstring *aux = (bstring *)b2;

        if (pos < 0 || INVALID(b1) || INVALID(b2) || NO_WRITE(b1))
                RUNTIME_ERROR;

        /* Aliasing case */
        if ((pd = (ptrdiff_t)(b2->data - b1->data)) >= 0 &&
            pd < (ptrdiff_t)b1->mlen) {
                if (!(aux = b_strcpy(b2)))
                        RUNTIME_ERROR;
        }

        /* Compute the two possible end pointers */
        d = b1->slen + aux->slen;
        blen = pos + aux->slen;
        if ((d | blen) < 0) {
                if (aux != b2)
                        b_destroy(aux);
                RUNTIME_ERROR;
        }

        if (blen > d) {
                /* Inserting past the end of the string */
                if (b_alloc(b1, blen + 1) != BSTR_OK) {
                        if (aux != b2)
                                b_destroy(aux);
                        RUNTIME_ERROR;
                }
                memset(b1->data + b1->slen, (int)fill,
                       (size_t)(pos - b1->slen));
                b1->slen = blen;
        } else {
                /* Inserting in the middle of the string */
                if (b_alloc(b1, d + 1) != BSTR_OK) {
                        if (aux != b2)
                                b_destroy(aux);
                        RUNTIME_ERROR;
                }
                b_BlockCopy(b1->data + blen, b1->data + pos, d - blen);
                b1->slen = d;
        }

        b_BlockCopy(b1->data + pos, aux->data, aux->slen);
        b1->data[b1->slen] = (uchar)'\0';
        if (aux != b2)
                b_destroy(aux);

        return BSTR_OK;
}


int
b_replace(bstring *b1, int pos, int len, const bstring *b2, uchar fill)
{
        int pl, ret;
        ptrdiff_t pd;
        bstring *aux = (bstring *)b2;

        if (pos < 0 || len < 0 || (pl = pos + len) < 0 || INVALID(b1) || INVALID(b2) || NO_WRITE(b1))
                RUNTIME_ERROR;

        /* Straddles the end? */
        if (pl >= b1->slen) {
                if ((ret = b_setstr(b1, pos, b2, fill)) < 0)
                        return ret;
                if (pos + b2->slen < b1->slen) {
                        b1->slen = pos + b2->slen;
                        b1->data[b1->slen] = (uchar)'\0';
                }
                return ret;
        }

        /* Aliasing case */
        pd = (ptrdiff_t)(b2->data - b1->data);
        if (pd >= 0 && pd < (ptrdiff_t)b1->slen) {
                aux = b_strcpy(b2);
                if (!aux)
                        RUNTIME_ERROR;
        }
        if (aux->slen > len) {
                if (b_alloc(b1, b1->slen + aux->slen - len) != BSTR_OK) {
                        if (aux != b2)
                                b_destroy(aux);
                        RUNTIME_ERROR;
                }
        }

        if (aux->slen != len)
                memmove(b1->data + pos + aux->slen, b1->data + pos + len, b1->slen - (pos + len));
        memcpy(b1->data + pos, aux->data, aux->slen);
        b1->slen += aux->slen - len;
        b1->data[b1->slen] = (uchar)'\0';
        if (aux != b2)
                b_destroy(aux);

        return BSTR_OK;
}


typedef int (*instr_fnptr)(const bstring *s1, int pos, const bstring *s2);

#define INITIAL_STATIC_FIND_INDEX_COUNT 32

/*
 *  findreplaceengine is used to implement bfindreplace and
 *  bfindreplacecaseless. It works by breaking the three cases of
 *  expansion, reduction and replacement, and solving each of these
 *  in the most efficient way possible.
 */
static int
findreplaceengine(bstring *bstr, const bstring *find, const bstring *repl,
                  int pos, instr_fnptr instr)
{
        int i, ret, slen, mlen, delta, acc;
        int *d;
        /* This +1 is unnecessary, but it shuts up LINT. */
        int static_d[INITIAL_STATIC_FIND_INDEX_COUNT + 1];
        ptrdiff_t pd;
        bstring *auxf = (bstring *)find;
        bstring *auxr = (bstring *)repl;

        if (IS_NULL(find) || find->slen <= 0 || INVALID(bstr) ||
            INVALID(repl) || NO_WRITE(bstr) || pos < 0)
                RUNTIME_ERROR;

        if (pos > bstr->slen - find->slen)
                return BSTR_OK;

        /* Alias with find string */
        pd = (ptrdiff_t)(find->data - bstr->data);
        if ((ptrdiff_t)(pos - find->slen) < pd && pd < (ptrdiff_t)bstr->slen) {
                auxf = b_strcpy(find);
                if (!auxf)
                        RUNTIME_ERROR;
        }

        /* Alias with repl string */
        pd = (ptrdiff_t)(repl->data - bstr->data);
        if ((ptrdiff_t)(pos - repl->slen) < pd && pd < (ptrdiff_t)bstr->slen) {
                auxr = b_strcpy(repl);
                if (!auxr) {
                        if (auxf != find)
                                b_destroy(auxf);
                        RUNTIME_ERROR;
                }
        }

        delta = auxf->slen - auxr->slen;
        /* in-place replacement since find and replace strings are of equal length */
        if (delta == 0) {
                while ((pos = instr(bstr, pos, auxf)) >= 0) {
                        memcpy(bstr->data + pos, auxr->data, auxr->slen);
                        pos += auxf->slen;
                }
                if (auxf != find)
                        b_destroy(auxf);
                if (auxr != repl)
                        b_destroy(auxr);
                return BSTR_OK;
        }

        /* shrinking replacement since auxf->slen > auxr->slen */
        if (delta > 0) {
                acc = 0;
                while ((i = instr(bstr, pos, auxf)) >= 0) {
                        if (acc && i > pos)
                                memmove(bstr->data + pos - acc, bstr->data + pos, i - pos);
                        if (auxr->slen)
                                memcpy(bstr->data + i - acc, auxr->data, auxr->slen);
                        acc += delta;
                        pos = i + auxf->slen;
                }

                if (acc) {
                        i = bstr->slen;
                        if (i > pos)
                                memmove((bstr->data + pos - acc), bstr->data + pos, i - pos);
                        bstr->slen -= acc;
                        bstr->data[bstr->slen] = (uchar)'\0';
                }

                if (auxf != find)
                        b_destroy(auxf);
                if (auxr != repl)
                        b_destroy(auxr);
                return BSTR_OK;
        }

        /* Expanding replacement since find->slen < repl->slen. Its a lot
         * more complicated. This works by first finding all the matches and
         * storing them to a growable array, then doing at most one resize of
         * the destination bstring * and then performing the direct memory
         * transfers of the string segment pieces to form the final result. The
         * growable array of matches uses a deferred doubling reallocing
         * strategy. What this means is that it starts as a reasonably fixed
         * sized auto array in the hopes that many if not most cases will never
         * need to grow this array. But it switches as soon as the bounds of
         * the array will be exceeded. An extra find result is always appended
         * to this array that corresponds to the end of the destination string,
         * so slen is checked against mlen - 1 rather than mlen before resizing. */

        mlen = INITIAL_STATIC_FIND_INDEX_COUNT;
        d = (int *)static_d; /* Avoid malloc for trivial/initial cases */
        acc = slen = 0;
        while ((pos = instr(bstr, pos, auxf)) >= 0) {
                if (slen >= mlen - 1) {
                        int sl, *t;
                        mlen += mlen;
                        sl = sizeof(int *) * mlen;
                        if (static_d == d) /* static_d cannot be realloced */
                                d = NULL;
                        if (mlen <= 0 || sl < mlen || !(t = realloc(d, sl))) {
                                ret = BSTR_ERR;
                                goto done;
                        }
                        if (!d)
                                memcpy(t, static_d, sizeof(static_d));
                        d = t;
                }
                d[slen] = pos;
                slen++;
                acc -= delta;
                pos += auxf->slen;
                if (pos < 0 || acc < 0) {
                        ret = BSTR_ERR;
                        goto done;
                }
        }
        /* slen <= INITIAL_STATIC_INDEX_COUNT-1 or mlen-1 here. */
        d[slen] = bstr->slen;
        ret = b_alloc(bstr, bstr->slen + acc + 1);
        if (BSTR_OK == ret) {
                bstr->slen += acc;
                for (i = slen - 1; i >= 0; --i) {
                        int buf, blen;
                        buf = d[i] + auxf->slen;
                        blen = d[i + 1] - buf; /* d[slen] may be accessed here. */
                        if (blen)
                                memmove(bstr->data + buf + acc,
                                        bstr->data + buf, blen);
                        if (auxr->slen)
                                memmove(bstr->data + buf + acc - auxr->slen,
                                        auxr->data, auxr->slen);
                        acc += delta;
                }
                bstr->data[bstr->slen] = (uchar)'\0';
        }
done:
        if (static_d == d)
                d = NULL;
        free(d);
        if (auxf != find)
                b_destroy(auxf);
        if (auxr != repl)
                b_destroy(auxr);

        return ret;
}


int
b_findreplace(bstring *bstr, const bstring *find, const bstring *repl, int pos)
{
        return findreplaceengine(bstr, find, repl, pos, b_instr);
}


int
b_findreplace_caseless(bstring *bstr, const bstring *find, const bstring *repl, int pos)
{
        return findreplaceengine(bstr, find, repl, pos, b_instr_caseless);
}


int
b_insertch(bstring *bstr, int pos, const int len, const uchar fill)
{
        if (pos < 0 || INVALID(bstr) || NO_WRITE(bstr) || len < 0)
                RUNTIME_ERROR;

        /* Compute the two possible end pointers */
        const int end    = bstr->slen + len;
        const int newlen = pos + len;
        if ((end | newlen) < 0)
                RUNTIME_ERROR;

        if (newlen > end) {
                /* Inserting past the end of the string */
                if (b_alloc(bstr, newlen + 1) != BSTR_OK)
                        RUNTIME_ERROR;
                pos = bstr->slen;
                bstr->slen = newlen;
        } else {
                /* Inserting in the middle of the string */
                if (b_alloc(bstr, end + 1) != BSTR_OK)
                        RUNTIME_ERROR;
                for (int i = end - 1; i >= newlen; --i)
                        bstr->data[i] = bstr->data[i - len];
                bstr->slen = end;
        }

        for (int i = pos; i < newlen; ++i)
                bstr->data[i] = fill;
        bstr->data[bstr->slen] = (uchar)'\0';

        return BSTR_OK;
}


int
b_pattern(bstring *bstr, const int len)
{
        const int d = b_length(bstr);

        if (d <= 0 || len < 0 || b_alloc(bstr, len + 1) != BSTR_OK)
                RUNTIME_ERROR;

        if (len > 0) {
                if (d == 1)
                        return b_setstr(bstr, len, NULL, bstr->data[0]);
                for (int i = d; i < len; ++i)
                        bstr->data[i] = bstr->data[i - d];
        }

        bstr->data[len] = (uchar)'\0';
        bstr->slen = len;

        return BSTR_OK;
}


#define BS_BUFF_SZ (1024)

int
b_reada(bstring *bstr, const bNread read_ptr, void *parm)
{
        if (INVALID(bstr) || NO_WRITE(bstr) || !read_ptr)
                RUNTIME_ERROR;

        int i = bstr->slen;
        for (int n = (i + 16);; n += ((n < BS_BUFF_SZ) ? n : BS_BUFF_SZ)) {
                if (BSTR_OK != b_alloc(bstr, n + 1))
                        RUNTIME_ERROR;
                const int bytes_read = (int)read_ptr((void *)(bstr->data + i), 1, n - i, parm);
                i += bytes_read;
                bstr->slen = i;
                if (i < n)
                        break;
        }
        bstr->data[i] = (uchar)'\0';

        return BSTR_OK;
}


bstring *
b_read(const bNread read_ptr, void *parm)
{
        bstring *buff;
        if (0 > b_reada((buff = b_fromcstr("")), read_ptr, parm)) {
                b_destroy(buff);
                RETURN_NULL;
        }
        return buff;
}


int
b_assign_gets(bstring *bstr, const bNgetc getc_ptr, void *parm, const char terminator)
{
        if (INVALID(bstr) || NO_WRITE(bstr) || !getc_ptr)
                RUNTIME_ERROR;
        int d = 0;
        int e = bstr->mlen - 2;
        int ch;

        while ((ch = getc_ptr(parm)) >= 0) {
                if (d > e) {
                        bstr->slen = d;
                        if (b_alloc(bstr, d + 2) != BSTR_OK)
                                RUNTIME_ERROR;
                        e = bstr->mlen - 2;
                }
                bstr->data[d] = (uchar)ch;
                d++;
                if (ch == terminator)
                        break;
        }

        bstr->data[d] = (uchar)'\0';
        bstr->slen = d;

        return (d == 0 && ch < 0);
}


int
b_getsa(bstring *bstr, const bNgetc getc_ptr, void *parm, const char terminator)
{
        if (INVALID(bstr) || NO_WRITE(bstr) || !getc_ptr)
                RUNTIME_ERROR;
        int ch;
        int d = bstr->slen;
        int e = bstr->mlen - 2;

        while ((ch = getc_ptr(parm)) >= 0) {
                if (d > e) {
                        bstr->slen = d;
                        if (b_alloc(bstr, d + 2) != BSTR_OK)
                                RUNTIME_ERROR;
                        e = bstr->mlen - 2;
                }
                bstr->data[d] = (uchar)ch;
                d++;
                if (ch == terminator)
                        break;
        }

        bstr->data[d] = (uchar)'\0';
        bstr->slen = d;

        return (d == 0 && ch < 0);
}


bstring *
b_gets(const bNgetc getc_ptr, void *parm, const char terminator)
{
        bstring *buff;
        if (0 > b_getsa(buff = b_fromcstr(""), getc_ptr, parm, terminator) || 0 >= buff->slen) {
                b_destroy(buff);
                buff = NULL;
        }
        return buff;
}


struct bStream {
        bstring *buff;    /* Buffer for over-reads */
        void *parm;       /* The stream handle for core stream */
        bNread readFnPtr; /* fread compatible fnptr for core stream */
        int isEOF;        /* track file'buf EOF state */
        int maxBuffSz;
};


struct bStream *
bs_open(const bNread read_ptr, void *parm)
{
        struct bStream *buf;
        if (!read_ptr)
                RETURN_NULL;
        if (!(buf = malloc(sizeof(struct bStream))))
                RETURN_NULL;

        buf->parm      = parm;
        buf->buff      = b_fromcstr("");
        buf->readFnPtr = read_ptr;
        buf->maxBuffSz = BS_BUFF_SZ;
        buf->isEOF     = 0;

        return buf;
}


int
bs_bufflength(struct bStream *buf, const int sz)
{
        int old_sz;
        if (!buf || sz < 0)
                RUNTIME_ERROR;
        old_sz = buf->maxBuffSz;
        if (sz > 0)
                buf->maxBuffSz = sz;

        return old_sz;
}


int
bs_eof(const struct bStream *buf)
{
        if (!buf || !buf->readFnPtr)
                RUNTIME_ERROR;
        return buf->isEOF && (buf->buff->slen == 0);
}


void *
bs_close(struct bStream *buf)
{
        void *parm;
        if (!buf)
                RETURN_NULL;
        buf->readFnPtr = NULL;
        if (buf->buff)
                b_destroy(buf->buff);
        buf->buff = NULL;
        parm = buf->parm;
        buf->parm = NULL;
        buf->isEOF = 1;
        free(buf);

        return parm;
}


int
bs_readlna(bstring *r, struct bStream *buf, const char terminator)
{
        int i, rlo;
        bstring tmp;

        if (!buf || !buf->buff || INVALID(r) || NO_WRITE(r))
                RUNTIME_ERROR;

        int blen = buf->buff->slen;
        if (BSTR_OK != b_alloc(buf->buff, buf->maxBuffSz + 1))
                RUNTIME_ERROR;
        char *str = (char *)buf->buff->data;
        tmp.data = (uchar *)str;

        /* First check if the current buffer holds the terminator */
        str[blen] = terminator; /* Set sentinel */
        for (i = 0; str[i] != terminator; ++i)
                ;
        if (i < blen) {
                tmp.slen = i + 1;
                const int ret = b_concat(r, &tmp);
                buf->buff->slen = blen;
                if (BSTR_OK == ret)
                        b_delete(buf->buff, 0, i + 1);
                return BSTR_OK;
        }
        rlo = r->slen;
        /* If not then just concatenate the entire buffer to the output */
        tmp.slen = blen;
        if (BSTR_OK != b_concat(r, &tmp))
                RUNTIME_ERROR;

        /* Perform direct in-place reads into the destination to allow for
         * the minimum of data-copies */
        for (;;) {
                if (BSTR_OK != b_alloc(r, r->slen + buf->maxBuffSz + 1))
                        RUNTIME_ERROR;
                str = (char *)(r->data + r->slen);
                blen = (int)buf->readFnPtr(str, 1, buf->maxBuffSz, buf->parm);
                if (blen <= 0) {
                        r->data[r->slen] = (uchar)'\0';
                        buf->buff->slen = 0;
                        buf->isEOF = 1;
                        /* If nothing was read return with an error message */
                        return BSTR_ERR & -(r->slen == rlo);
                }
                str[blen] = terminator; /* Set sentinel */
                for (i = 0; str[i] != terminator; ++i)
                        ;
                if (i < blen)
                        break;
                r->slen += blen;
        }

        /* Terminator found, push over-read back to buffer */
        r->slen += (++i);
        buf->buff->slen = blen - i;
        memcpy(buf->buff->data, str + i, blen - i);
        r->data[r->slen] = (uchar)'\0';

        return BSTR_OK;
}


int
bs_readlnsa(bstring *r, struct bStream *buf, const bstring *term)
{
        int i, blen, ret, rlo;
        uchar *bstr;
        bstring tmp;
        struct char_field cf;

        if (!buf || !buf->buff || INVALID(term) || INVALID(r) || NO_WRITE(r))
                RUNTIME_ERROR;
        if (term->slen == 1)
                return bs_readlna(r, buf, term->data[0]);
        if (term->slen < 1 || build_char_field(&cf, term))
                RUNTIME_ERROR;

        blen = buf->buff->slen;
        if (BSTR_OK != b_alloc(buf->buff, buf->maxBuffSz + 1))
                RUNTIME_ERROR;

        bstr = (uchar *)buf->buff->data;
        tmp.data = bstr;

        /* First check if the current buffer holds the terminator */
        bstr[blen] = term->data[0]; /* Set sentinel */
        for (i = 0; !testInCharField(&cf, bstr[i]); ++i)
                ;
        if (i < blen) {
                tmp.slen = i + 1;
                ret = b_concat(r, &tmp);
                buf->buff->slen = blen;
                if (BSTR_OK == ret)
                        b_delete(buf->buff, 0, i + 1);
                return BSTR_OK;
        }
        rlo = r->slen;

        /* If not then just concatenate the entire buffer to the output */
        tmp.slen = blen;
        if (BSTR_OK != b_concat(r, &tmp))
                RUNTIME_ERROR;

        /* Perform direct in-place reads into the destination to allow for
         * the minimum of data-copies */
        for (;;) {
                if (BSTR_OK != b_alloc(r, r->slen + buf->maxBuffSz + 1))
                        RUNTIME_ERROR;

                bstr = (uchar *)(r->data + r->slen);
                blen = (int)buf->readFnPtr(bstr, 1, buf->maxBuffSz, buf->parm);

                if (blen <= 0) {
                        r->data[r->slen] = (uchar)'\0';
                        buf->buff->slen = 0;
                        buf->isEOF = 1;
                        /* If nothing was read return with an error message */
                        return BSTR_ERR & -(r->slen == rlo);
                }

                bstr[blen] = term->data[0]; /* Set sentinel */
                for (i = 0; !testInCharField(&cf, bstr[i]); ++i)
                        ;
                if (i < blen)
                        break;
                r->slen += blen;
        }

        /* Terminator found, push over-read back to buffer */
        r->slen += (++i);
        buf->buff->slen = blen - i;
        memcpy(buf->buff->data, bstr + i, blen - i);
        r->data[r->slen] = (uchar)'\0';

        return BSTR_OK;
}


int
bs_reada(bstring *r, struct bStream *buf, int n)
{
        int blen, ret, orslen;
        char *bstr;
        bstring tmp;

        if (!buf || !buf->buff || INVALID(r) || NO_WRITE(r) || n <= 0)
                RUNTIME_ERROR;
        n += r->slen;

        if (n <= 0)
                RUNTIME_ERROR;
        blen = buf->buff->slen;
        orslen = r->slen;

        if (0 == blen) {
                if (buf->isEOF)
                        RUNTIME_ERROR;
                if (r->mlen > n) {
                        blen = (int)buf->readFnPtr((r->data + r->slen), 1, (n - r->slen), buf->parm);
                        if (0 >= blen || blen > n - r->slen) {
                                buf->isEOF = 1;
                                RUNTIME_ERROR;
                        }
                        r->slen += blen;
                        r->data[r->slen] = (uchar)'\0';
                        return 0;
                }
        }

        if (BSTR_OK != b_alloc(buf->buff, buf->maxBuffSz + 1))
                RUNTIME_ERROR;
        bstr = (char *)buf->buff->data;
        tmp.data = (uchar *)bstr;

        do {
                if (blen + r->slen >= n) {
                        tmp.slen = n - r->slen;
                        ret = b_concat(r, &tmp);
                        buf->buff->slen = blen;
                        if (BSTR_OK == ret)
                                b_delete(buf->buff, 0, tmp.slen);
                        return BSTR_ERR & -(r->slen == orslen);
                }
                tmp.slen = blen;
                if (BSTR_OK != b_concat(r, &tmp))
                        break;
                blen = n - r->slen;
                if (blen > buf->maxBuffSz)
                        blen = buf->maxBuffSz;
                blen = (int)buf->readFnPtr(bstr, 1, blen, buf->parm);

        } while (blen > 0);

        if (blen < 0)
                blen = 0;
        if (blen == 0)
                buf->isEOF = 1;
        buf->buff->slen = blen;

        return BSTR_ERR & -(r->slen == orslen);
}


int
bs_readln(bstring *r, struct bStream *buf, char terminator)
{
        if (!buf || !buf->buff || INVALID(r))
                RUNTIME_ERROR;
        if (BSTR_OK != b_alloc(buf->buff, buf->maxBuffSz + 1))
                RUNTIME_ERROR;
        r->slen = 0;

        return bs_readlna(r, buf, terminator);
}


int
bs_readlns(bstring *r, struct bStream *buf, const bstring *term)
{
        if (!buf || !buf->buff || INVALID(r) || INVALID(term))
                RUNTIME_ERROR;
        if (term->slen == 1)
                return bs_readln(r, buf, term->data[0]);
        if (term->slen < 1)
                RUNTIME_ERROR;
        if (BSTR_OK != b_alloc(buf->buff, buf->maxBuffSz + 1))
                RUNTIME_ERROR;
        r->slen = 0;

        return bs_readlnsa(r, buf, term);
}


int
bs_read(bstring *r, struct bStream *buf, int n)
{
        if (!buf || !buf->buff || !r || r->mlen <= 0 || n <= 0)
                RUNTIME_ERROR;
        if (BSTR_OK != b_alloc(buf->buff, buf->maxBuffSz + 1))
                RUNTIME_ERROR;
        r->slen = 0;

        return bs_reada(r, buf, n);
}


int
bs_unread(struct bStream *buf, const bstring *bstr)
{
        if (!buf || !buf->buff)
                RUNTIME_ERROR;
        return b_insert(buf->buff, 0, bstr, (uchar)'?');
}


int
bs_peek(bstring *r, const struct bStream *buf)
{
        if (!buf || !buf->buff)
                RUNTIME_ERROR;
        return b_assign(r, buf->buff);
}


bstring *
b_join(const b_list *bl, const bstring *sep)
{
        bstring *bstr;
        int i, c, v;
        if (!bl || bl->qty < 0)
                RETURN_NULL;
        if (INVALID(sep))
                RETURN_NULL;

        for (i = 0, c = 1; i < bl->qty; ++i) {
                v = bl->entry[i]->slen;
                if (v < 0)
                        RETURN_NULL; /* Invalid input */
                c += v;
                if (c < 0)
                        RETURN_NULL; /* Wrap around ?? */
        }

        if (sep)
                c += (bl->qty - 1) * sep->slen;
        if (!(bstr = malloc(sizeof(bstring))))
                RETURN_NULL;  /* Out of memory */
        if (!(bstr->data = malloc(c))) {
                free(bstr);
                RETURN_NULL;
        }

        bstr->mlen = c;
        bstr->slen = c - 1;
        for (i = 0, c = 0; i < bl->qty; ++i) {
                if (i > 0 && sep) {
                        memcpy(bstr->data + c, sep->data, sep->slen);
                        c += sep->slen;
                }
                v = bl->entry[i]->slen;
                memcpy(bstr->data + c, bl->entry[i]->data, v);
                c += v;
        }
        bstr->data[c] = (uchar)'\0';

        return bstr;
}


#define BSSSC_BUFF_LEN (256)

int
bs_splitscb(struct bStream *buf, const bstring *splitStr, bs_cbfunc cb, void *parm)
{
        struct char_field chrs;
        bstring *buff;
        int i, p, ret;
        if (!cb || !buf || !buf->readFnPtr || INVALID(splitStr))
                RUNTIME_ERROR;
        buff = b_fromcstr("");
        if (!buff)
                RUNTIME_ERROR;

        if (splitStr->slen == 0) {
                while (bs_reada(buff, buf, BSSSC_BUFF_LEN) >= 0)
                        ;
                if ((ret = cb(parm, 0, buff)) > 0)
                        ret = 0;
        } else {
                build_char_field(&chrs, splitStr);
                ret = p = i = 0;
                for (;;) {
                        if (i >= buff->slen) {
                                bs_reada(buff, buf, BSSSC_BUFF_LEN);
                                if (i >= buff->slen) {
                                        if (0 < (ret = cb(parm, p, buff)))
                                                ret = 0;
                                        break;
                                }
                        }
                        if (testInCharField(&chrs, buff->data[i])) {
                                uchar c;
                                bstring t = blk2tbstr((buff->data + i + 1),
                                                      buff->slen - (i + 1));
                                if ((ret = bs_unread(buf, &t)) < 0)
                                        break;
                                buff->slen = i;
                                c = buff->data[i];
                                buff->data[i] = (uchar)'\0';
                                if ((ret = cb(parm, p, buff)) < 0)
                                        break;
                                buff->data[i] = c;
                                buff->slen = 0;
                                p += i + 1;
                                i = -1;
                        }
                        i++;
                }
        }

        b_destroy(buff);
        return ret;
}


int
bs_splitstrcb(struct bStream *buf, const bstring *splitStr, bs_cbfunc cb, void *parm)
{
        bstring *buff;
        int ret;

        if (!cb || !buf || !buf->readFnPtr || INVALID(splitStr))
                RUNTIME_ERROR;
        if (splitStr->slen == 1)
                return bs_splitscb(buf, splitStr, cb, parm);
        if (!(buff = b_fromcstr("")))
                RUNTIME_ERROR;

        if (splitStr->slen == 0) {
                for (int i = 0; bs_reada(buff, buf, BSSSC_BUFF_LEN) >= 0; ++i) {
                        if ((ret = cb(parm, 0, buff)) < 0) {
                                b_destroy(buff);
                                return ret;
                        }
                        buff->slen = 0;
                }
                b_destroy(buff);
                return BSTR_OK;
        } else {
                for (int p = 0, i = 0;;) {
                        ret = b_instr(buff, 0, splitStr);
                        if (ret >= 0) {
                                bstring t = blk2tbstr(buff->data, ret);
                                i = ret + splitStr->slen;
                                ret = cb(parm, p, &t);
                                if (ret < 0)
                                        break;
                                p += i;
                                b_delete(buff, 0, i);
                        } else {
                                bs_reada(buff, buf, BSSSC_BUFF_LEN);
                                if (bs_eof(buf)) {
                                        ret = cb(parm, p, buff);
                                        if (ret > 0)
                                                ret = 0;
                                        break;
                                }
                        }
                }
        }

        b_destroy(buff);
        return ret;
}


b_list *
b_strListCreate(void)
{
        b_list *sl = malloc(sizeof(b_list));
        if (sl) {
                sl->entry = malloc(1 * sizeof(bstring *));
                if (!sl->entry) {
                        free(sl);
                        sl = NULL;
                } else {
                        sl->qty = 0;
                        sl->mlen = 1;
                }
        }
        return sl;
}


int
b_strListDestroy(b_list *sl)
{
        int i;
        if (!sl || sl->qty < 0)
                RUNTIME_ERROR;
        for (i = 0; i < sl->qty; ++i) {
                if (sl->entry[i]) {
                        b_destroy(sl->entry[i]);
                        sl->entry[i] = NULL;
                }
        }
        sl->qty = -1;
        sl->mlen = -1;
        free(sl->entry);
        sl->entry = NULL;
        free(sl);

        return BSTR_OK;
}


int
b_strListAlloc(b_list *sl, int msz)
{
        bstring **blen;
        int smsz;
        size_t nsz;
        if (!sl || msz <= 0 || !sl->entry || sl->qty < 0 || sl->mlen <= 0 || sl->qty > sl->mlen)
                RUNTIME_ERROR;
        if (sl->mlen >= msz)
                return BSTR_OK;
        smsz = snapUpSize(msz);
        nsz = ((size_t)smsz) * sizeof(bstring *);
        if (nsz < (size_t)smsz)
                RUNTIME_ERROR;
        blen = realloc(sl->entry, nsz);
        if (!blen) {
                smsz = msz;
                nsz = ((size_t)smsz) * sizeof(bstring *);
                blen = realloc(sl->entry, nsz);
                if (!blen)
                        RUNTIME_ERROR;
        }
        sl->mlen = smsz;
        sl->entry = blen;
        return BSTR_OK;
}


int
b_strListAllocMin(b_list *sl, int msz)
{
        bstring **blen;
        size_t nsz;
        if (!sl || msz <= 0 || !sl->entry || sl->qty < 0 || sl->mlen <= 0 || sl->qty > sl->mlen)
                RUNTIME_ERROR;
        if (msz < sl->qty)
                msz = sl->qty;
        if (sl->mlen == msz)
                return BSTR_OK;
        nsz = ((size_t)msz) * sizeof(bstring *);
        if (nsz < (size_t)msz)
                RUNTIME_ERROR;
        blen = realloc(sl->entry, nsz);
        if (!blen)
                RUNTIME_ERROR;
        sl->mlen = msz;
        sl->entry = blen;
        return BSTR_OK;
}


int
b_splitcb(const bstring *str, uchar splitChar, int pos, b_cbfunc cb, void *parm)
{
        int i, p, ret;
        if (!cb || INVALID(str) || pos < 0 || pos > str->slen)
                RUNTIME_ERROR;
        p = pos;

        do {
                for (i = p; i < str->slen; ++i)
                        if (str->data[i] == splitChar)
                                break;
                if ((ret = cb(parm, p, i - p)) < 0)
                        return ret;
                p = i + 1;
        } while (p <= str->slen);

        return BSTR_OK;
}


int
b_splitscb(const bstring *str, const bstring *splitStr, const int pos,
           b_cbfunc cb, void *parm)
{
        struct char_field chrs;
        int i, p, ret;
        if (!cb || INVALID(str) || INVALID(splitStr) || pos < 0 || pos > str->slen)
                RUNTIME_ERROR;
        if (splitStr->slen == 0) {
                if ((ret = cb(parm, 0, str->slen)) > 0)
                        ret = 0;
                return ret;
        }
        if (splitStr->slen == 1)
                return b_splitcb(str, splitStr->data[0], pos, cb, parm);

        build_char_field(&chrs, splitStr);
        p = pos;
        do {
                for (i = p; i < str->slen; ++i)
                        if (testInCharField(&chrs, str->data[i]))
                                break;
                if ((ret = cb(parm, p, i - p)) < 0)
                        return ret;
                p = i + 1;
        } while (p <= str->slen);

        return BSTR_OK;
}


int
b_splitstrcb(const bstring *str, const bstring *splitStr, int pos, b_cbfunc cb, void *parm)
{
        int i, p, ret;
        if (!cb || INVALID(str) || INVALID(splitStr) || pos < 0 || pos > str->slen)
                RUNTIME_ERROR;

        if (0 == splitStr->slen) {
                for (i = pos; i < str->slen; ++i) {
                        ret = cb(parm, i, 1);
                        if (ret < 0)
                                return ret;
                }
                return BSTR_OK;
        }
        if (splitStr->slen == 1)
                return b_splitcb(str, splitStr->data[0], pos, cb, parm);
        i = p = pos;
        while (i <= str->slen - splitStr->slen) {
                ret = memcmp(splitStr->data, str->data + i, splitStr->slen);
                if (0 == ret) {
                        ret = cb(parm, p, i - p);
                        if (ret < 0)
                                return ret;
                        i += splitStr->slen;
                        p = i;
                } else
                        i++;
        }
        ret = cb(parm, p, str->slen - p);
        if (ret < 0)
                return ret;

        return BSTR_OK;
}


struct gen_b_list {
        bstring *bstr;
        b_list *bl;
};


static int
b_scb(void *parm, int ofs, int len)
{
        struct gen_b_list *g = (struct gen_b_list *)parm;
        if (g->bl->qty >= g->bl->mlen) {
                int mlen = g->bl->mlen * 2;
                bstring **tbl;
                while (g->bl->qty >= mlen) {
                        if (mlen < g->bl->mlen)
                                RUNTIME_ERROR;
                        mlen += mlen;
                }
                tbl = realloc(g->bl->entry, sizeof(bstring *) * mlen);
                if (!tbl)
                        RUNTIME_ERROR;
                g->bl->entry = tbl;
                g->bl->mlen = mlen;
        }

        g->bl->entry[g->bl->qty] = b_midstr(g->bstr, ofs, len);
        g->bl->qty++;

        return BSTR_OK;
}


b_list *
b_split(const bstring *str, uchar splitChar)
{
        struct gen_b_list g;
        if (INVALID(str) || !(g.bl = malloc(sizeof(b_list))))
                RETURN_NULL;

        g.bl->mlen  = 4;
        g.bl->entry = malloc(g.bl->mlen * sizeof(bstring *));

        if (!g.bl->entry) {
                free(g.bl);
                RETURN_NULL;
        }

        g.bstr = (bstring *)str;
        g.bl->qty = 0;
        if (b_splitcb(str, splitChar, 0, b_scb, &g) < 0) {
                b_strListDestroy(g.bl);
                RETURN_NULL;
        }

        return g.bl;
}


b_list *
b_splitstr(const bstring *str, const bstring *splitStr)
{
        struct gen_b_list g;

        if (INVALID(str))
                RETURN_NULL;
        if (!(g.bl = malloc(sizeof(b_list))))
                RETURN_NULL;
        g.bl->mlen = 4;

        if (!(g.bl->entry = malloc(g.bl->mlen * sizeof(bstring *)))) {
                free(g.bl);
                RETURN_NULL;
        }
        g.bstr = (bstring *)str;
        g.bl->qty = 0;

        if (b_splitstrcb(str, splitStr, 0, b_scb, &g) < 0) {
                b_strListDestroy(g.bl);
                RETURN_NULL;
        }

        return g.bl;
}


b_list *
b_splits(const bstring *str, const bstring *splitStr)
{
        struct gen_b_list g;
        if (INVALID(str) || INVALID(splitStr))
                RETURN_NULL;
        if (!(g.bl = malloc(sizeof(b_list))))
                RETURN_NULL;
        g.bl->mlen = 4;
        if (!(g.bl->entry = malloc(g.bl->mlen * sizeof(bstring *)))) {
                free(g.bl);
                RETURN_NULL;
        }
        g.bstr = (bstring *)str;
        g.bl->qty = 0;

        if (b_splitscb(str, splitStr, 0, b_scb, &g) < 0) {
                b_strListDestroy(g.bl);
                RETURN_NULL;
        }

        return g.bl;
}


#define START_VSNBUFF (16)

/* On IRIX vsnprintf returns n-1 when the operation would overflow the target
 * buffer, WATCOM and MSVC both return -1, while C99 requires that the returned
 * value be exactly what the length would be if the buffer would be large
 * enough.  This leads to the idea that if the return value is larger than n,
 * then changing n to the return value will reduce the number of iterations
 * required. */

int
b_formata(bstring *bstr, const char *fmt, ...)
{
        int r;
        va_list arglist;
        bstring *buff;
        if (!fmt || INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;

#ifdef HAVE_ASPRINTF
        if (!(buff  = malloc(sizeof *buff)))
                RUNTIME_ERROR;

        va_start(arglist, fmt);
        int n = vasprintf((char **)(&buff->data), fmt, arglist);
        va_end(arglist);

        if (n < 0) {
                b_destroy(buff);
                RUNTIME_ERROR;
        }
        buff->slen = buff->mlen = n;
#else
        /* Since the length is not determinable beforehand, a search is
         * performed using the truncating "vsnprintf" call (to avoid buffer
         * overflows) on increasing potential sizes for the output result. */
        int n = (int)(2 * strlen(fmt));
        if (n < START_VSNBUFF)
                n = START_VSNBUFF;
        buff = b_fromcstr_alloc(n + 2, "");

        if (!buff) {
                n = 1;
                buff = b_fromcstr_alloc(n + 2, "");
                if (!buff)
                        RUNTIME_ERROR;
        }

        for (;;) {
                va_start(arglist, fmt);
                r = vsnprintf((char *)buff->data, n + 1, fmt, arglist);
                va_end(arglist);

                buff->data[n] = (uchar)'\0';
                buff->slen = (int)(strlen)((char *)buff->data);

                if (buff->slen < n)
                        break;
                if (r > n)
                        n = r;
                else
                        n += n;
                if (BSTR_OK != b_alloc(buff, n + 2)) {
                        b_destroy(buff);
                        RUNTIME_ERROR;
                }
        }
#endif

        r = b_concat(bstr, buff);
        b_destroy(buff);

        return r;
}


int
b_assign_format(bstring *bstr, const char *fmt, ...)
{
        va_list arglist;
        bstring *buff;
        int n, r;
        if (!fmt || INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;

#ifdef HAVE_ASPRINTF
        if (!(buff  = malloc(sizeof *buff)))
                RUNTIME_ERROR;

        va_start(arglist, fmt);
        n = vasprintf((char **)(&buff->data), fmt, arglist);
        va_end(arglist);

        if (n < 0) {
                b_destroy(buff);
                RUNTIME_ERROR;
        }
        buff->slen = buff->mlen = n;
#else
        /* Since the length is not determinable beforehand, a search is
         * performed using the truncating "vsnprintf" call (to avoid buffer
         * overflows) on increasing potential sizes for the output result. */
        n = (int)(2 * strlen(fmt));
        if (n < START_VSNBUFF)
                n = START_VSNBUFF;
        buff = b_fromcstr_alloc(n + 2, "");
        if (!buff) {
                n = 1;
                buff = b_fromcstr_alloc(n + 2, "");
                if (!buff)
                        RUNTIME_ERROR;
        }
        for (;;) {
                va_start(arglist, fmt);
                r = vsnprintf((char *)buff->data, n + 1, fmt, arglist);
                va_end(arglist);
                buff->data[n] = (uchar)'\0';
                buff->slen = (int)strlen((char *)buff->data);
                if (buff->slen < n)
                        break;
                if (r > n)
                        n = r;
                else
                        n += n;
                if (BSTR_OK != b_alloc(buff, n + 2)) {
                        b_destroy(buff);
                        RUNTIME_ERROR;
                }
        }
#endif

        r = b_assign(bstr, buff);
        b_destroy(buff);

        return r;
}


bstring *
b_format(const char *fmt, ...)
{
        va_list arglist;
        bstring *buff;
        if (!fmt)
                RETURN_NULL;

#ifdef HAVE_ASPRINTF
        if (!(buff  = malloc(sizeof *buff)))
                RETURN_NULL;

        va_start(arglist, fmt);
        int n = vasprintf((char **)(&buff->data), fmt, arglist);
        va_end(arglist);

        if (n < 0) {
                b_destroy(buff);
                RETURN_NULL;
        }
        buff->slen = buff->mlen = n;
#else
        /* Since the length is not determinable beforehand, a search is
         * performed using the truncating "vsnprintf" call (to avoid buffer
         * overflows) on increasing potential sizes for the output result. */
        int n = (int)(2 * strlen(fmt));
        if (n < START_VSNBUFF)
                n = START_VSNBUFF;
        buff = b_fromcstr_alloc(n + 2, "");
        if (!buff) {
                n = 1;
                buff = b_fromcstr_alloc(n + 2, "");
                if (!buff)
                        RETURN_NULL;
        }

        for (;;) {
                int r;
                va_start(arglist, fmt);
                r = vsnprintf((char *)buff->data, n + 1, fmt, arglist);
                va_end(arglist);
                buff->data[n] = (uchar)'\0';
                buff->slen = (int)strlen((char *)buff->data);
                if (buff->slen < n)
                        break;
                if (r > n)
                        n = r;
                else
                        n += n;
                if (BSTR_OK != b_alloc(buff, n + 2)) {
                        b_destroy(buff);
                        RETURN_NULL;
                }
        }
#endif

        return buff;
}


int
b_vcformata(bstring *bstr, const int count, const char *fmt, va_list arg)
{
        if (!fmt || count <= 0 || INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR;

#ifdef HAVE_ASPRINTF
        bstring *buff = malloc(sizeof *buff);
        if (!buff)
                RUNTIME_ERROR;

        int n = vasprintf((char **)(&buff->data), fmt, arg);

        if (n < 0) {
                b_destroy(buff);
                RUNTIME_ERROR;
        }
        buff->slen = buff->mlen = n;

        return b_concat(bstr, buff);
#else
        int n, r, blen;
        if (count > (n = bstr->slen + count) + 2)
                RUNTIME_ERROR;
        if (BSTR_OK != b_alloc(bstr, n + 2))
                RUNTIME_ERROR;

        r = vsnprintf((char *)bstr->data + bstr->slen, count + 2, fmt, arg);

        /* Did the operation complete successfully within bounds? */
        for (blen = bstr->slen; blen <= n; blen++) {
                if ('\0' == bstr->data[blen]) {
                        bstr->slen = blen;
                        return BSTR_OK;
                }
        }

        /* Abort, since the buffer was not large enough.  The return value
         * tries to help set what the retry length should be. */
        bstr->data[bstr->slen] = '\0';
        if (r > count + 1) {
                /* Does r specify a particular target length? */
                n = r;
        } else {
                /* If not, just double the size of count */
                n = count + count;
                if (count > n)
                        n = INT_MAX;
        }

        n = -n;
        if (n > BSTR_ERR - 1)
                n = BSTR_ERR - 1;

        return n;
#endif
}


void
__b_fputs(FILE *fp, bstring *bstr, ...)
{
        va_list va;
        va_start(va, bstr);
        do fwrite(bstr->data, 1, bstr->slen, fp);
        while ((bstr = va_arg(va, bstring *)));
        va_end(va);
}
