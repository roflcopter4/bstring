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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bstraux.h"


bstring *
b_Tail(bstring *b, int n)
{
        if (b == NULL || n < 0 || (b->mlen < b->slen && b->mlen > 0))
                return NULL;
        if (n >= b->slen)
                return b_strcpy(b);
        return b_midstr(b, b->slen - n, n);
}

bstring *
b_Head(bstring *b, int n)
{
        if (b == NULL || n < 0 || (b->mlen < b->slen && b->mlen > 0))
                return NULL;
        if (n >= b->slen)
                return b_strcpy(b);
        return b_midstr(b, 0, n);
}

int
b_Fill(bstring *b, char c, int len)
{
        if (b == NULL || len < 0 || (b->mlen < b->slen && b->mlen > 0))
                return -__LINE__;
        b->slen = 0;
        return b_setstr(b, len, NULL, c);
}

int
b_Replicate(bstring *b, int n)
{
        return b_pattern(b, n * b->slen);
}

int
b_Reverse(bstring *b)
{
        int i, n, m;
        uchar t;
        if (b == NULL || b->slen < 0 || b->mlen < b->slen)
                return -__LINE__;
        n = b->slen;
        if (2 <= n) {
                m = ((unsigned int)n) >> 1;
                n--;
                for (i = 0; i < m; i++) {
                        t = b->data[n - i];
                        b->data[n - i] = b->data[i];
                        b->data[i] = t;
                }
        }
        return 0;
}

int
b_InsertChrs(bstring *b, int pos, int len, uchar c, uchar fill)
{
        if (b == NULL || b->slen < 0 || b->mlen < b->slen || pos < 0 ||
            len <= 0)
                return -__LINE__;
        if (pos > b->slen && 0 > b_setstr(b, pos, NULL, fill))
                return -__LINE__;
        if (0 > b_alloc(b, b->slen + len))
                return -__LINE__;
        if (pos < b->slen)
                memmove(b->data + pos + len, b->data + pos, b->slen - pos);
        memset(b->data + pos, c, len);
        b->slen += len;
        b->data[b->slen] = (uchar)'\0';
        return BSTR_OK;
}

int
b_JustifyLeft(bstring *b, int space)
{
        int j, i, s, t;
        uchar c = (uchar)space;
        if (b == NULL || b->slen < 0 || b->mlen < b->slen)
                return -__LINE__;
        if (space != (int)c)
                return BSTR_OK;
        for (s = j = i = 0; i < b->slen; i++) {
                t = s;
                s = c != (b->data[j] = b->data[i]);
                j += (t | s);
        }
        if (j > 0 && b->data[j - 1] == c)
                --j;
        b->data[j] = (uchar)'\0';
        b->slen = j;
        return BSTR_OK;
}

int
b_JustifyRight(bstring *b, int width, int space)
{
        int ret;
        if (width <= 0)
                return -__LINE__;
        if (0 > (ret = b_JustifyLeft(b, space)))
                return ret;
        if (b->slen <= width) {
                return b_InsertChrs(b, 0, width - b->slen, (uchar)space,
                                   (uchar)space);
        }
        return BSTR_OK;
}

int
b_JustifyCenter(bstring *b, int width, int space)
{
        int ret;
        if (width <= 0)
                return -__LINE__;
        if (0 > (ret = b_JustifyLeft(b, space)))
                return ret;
        if (b->slen <= width) {
                return b_InsertChrs(b, 0, (width - b->slen + 1) >> 1,
                                   (uchar)space, (uchar)space);
        }
        return BSTR_OK;
}

int
b_JustifyMargin(bstring *b, int width, int space)
{
        b_list *sl;
        int i, l, c;
        if (b == NULL || b->slen < 0 || b->mlen == 0 || b->mlen < b->slen)
                return -__LINE__;
        if (NULL == (sl = b_split(b, (uchar)space)))
                return -__LINE__;
        for (l = c = i = 0; i < sl->qty; i++) {
                if (sl->entry[i]->slen > 0) {
                        c++;
                        l += sl->entry[i]->slen;
                }
        }
        if (l + c >= width || c < 2) {
                b_strListDestroy(sl);
                return b_JustifyLeft(b, space);
        }
        b->slen = 0;
        for (i = 0; i < sl->qty; i++) {
                if (sl->entry[i]->slen > 0) {
                        if (b->slen > 0) {
                                int s = (width - l + (c / 2)) / c;
                                b_InsertChrs(b, b->slen, s, (uchar)space,
                                            (uchar)space);
                                l += s;
                        }
                        b_concat(b, sl->entry[i]);
                        c--;
                        if (c <= 0)
                                break;
                }
        }
        b_strListDestroy(sl);
        return BSTR_OK;
}

static size_t
readNothing(BSTR_UNUSED void *buff, BSTR_UNUSED size_t elsize,
            BSTR_UNUSED size_t nelem, BSTR_UNUSED void *parm)
{
        return 0; /* Immediately indicate EOF. */
}

struct bStream *
bs_FromBstr(const bstring *b)
{
        struct bStream *s = bs_open((bNread)readNothing, NULL);
        bs_unread(s, b); /* Push the bstring * data into the empty bStream. */
        return s;
}

static size_t
readRef(void *buff, size_t elsize, size_t nelem, void *parm)
{
        bstring *t = (bstring *)parm;
        size_t tsz = elsize * nelem;
        if (tsz > (size_t)t->slen)
                tsz = (size_t)t->slen;
        if (tsz > 0) {
                memcpy(buff, t->data, tsz);
                t->slen -= (int)tsz;
                t->data += tsz;
                return tsz / elsize;
        }
        return 0;
}

/**
 * The "by reference" version of the above function.
 *
 * This function puts a number of restrictions on the call site (the passed in
 * bstring *will* be modified by this function, and the source data
 * must remain alive and constant for the lifetime of the bStream). Hence it
 * is not presented as an extern.
 */
static struct bStream *
bsFromBstrRef(bstring *t)
{
        if (!t)
                return NULL;
        return bs_open((bNread)readRef, t);
}

char *
b_Str2NetStr(const bstring *b)
{
        char strnum[sizeof(b->slen) * 3 + 1];
        bstring *s;
        uchar *buff;
        if (b == NULL || b->data == NULL || b->slen < 0)
                return NULL;
        sprintf(strnum, "%d:", b->slen);
        if (NULL == (s = b_fromcstr(strnum)) || b_concat(s, b) == BSTR_ERR ||
            b_conchar(s, (char)',') == BSTR_ERR) {
                b_destroy(s);
                return NULL;
        }
        buff = s->data;
        b_cstrfree((char *)s);
        return (char *)buff;
}

bstring *
b_NetStr2Bstr(const char *buff)
{
        int i, x;
        bstring *b;
        if (buff == NULL)
                return NULL;
        x = 0;
        for (i = 0; buff[i] != ':'; ++i) {
                unsigned int v = buff[i] - '0';
                if (v > 9 || x > ((INT_MAX - (signed int)v) / 10))
                        return NULL;
                x = (x * 10) + v;
        }
        /* This thing has to be properly terminated */
        if (buff[i + 1 + x] != ',')
                return NULL;
        if (NULL == (b = b_fromcstr("")))
                return NULL;
        if (b_alloc(b, x + 1) != BSTR_OK) {
                b_destroy(b);
                return NULL;
        }
        memcpy(b->data, buff + i + 1, x);
        b->data[x] = (uchar)'\0';
        b->slen = x;
        return b;
}

static char b64ETable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefghijklmnopqrstuvwxyz"
                          "0123456789+/";

bstring *
b_Base64Encode(const bstring *b)
{
        int i, c0, c1, c2, c3;
        bstring *out;
        if (b == NULL || b->slen < 0 || b->data == NULL)
                return NULL;
        out = b_fromcstr("");
        for (i = 0; i + 2 < b->slen; i += 3) {
                if (i && ((i % 57) == 0)) {
                        if (b_conchar(out, (char)'\015') < 0 ||
                            b_conchar(out, (char)'\012') < 0) {
                                b_destroy(out);
                                return NULL;
                        }
                }
                c0 = b->data[i] >> 2;
                c1 = ((b->data[i] << 4) | (b->data[i + 1] >> 4)) & 0x3F;
                c2 = ((b->data[i + 1] << 2) | (b->data[i + 2] >> 6)) & 0x3F;
                c3 = b->data[i + 2] & 0x3F;
                if (b_conchar(out, b64ETable[c0]) < 0 ||
                    b_conchar(out, b64ETable[c1]) < 0 ||
                    b_conchar(out, b64ETable[c2]) < 0 ||
                    b_conchar(out, b64ETable[c3]) < 0) {
                        b_destroy(out);
                        return NULL;
                }
        }
        if (i && ((i % 57) == 0)) {
                if (b_conchar(out, (char)'\015') < 0 ||
                    b_conchar(out, (char)'\012') < 0) {
                        b_destroy(out);
                        return NULL;
                }
        }
        switch (i + 2 - b->slen) {
        case 0:
                c0 = b->data[i] >> 2;
                c1 = ((b->data[i] << 4) | (b->data[i + 1] >> 4)) & 0x3F;
                c2 = (b->data[i + 1] << 2) & 0x3F;
                if (b_conchar(out, b64ETable[c0]) < 0 ||
                    b_conchar(out, b64ETable[c1]) < 0 ||
                    b_conchar(out, b64ETable[c2]) < 0 ||
                    b_conchar(out, (char)'=') < 0) {
                        b_destroy(out);
                        return NULL;
                }
                break;
        case 1:
                c0 = b->data[i] >> 2;
                c1 = (b->data[i] << 4) & 0x3F;
                if (b_conchar(out, b64ETable[c0]) < 0 ||
                    b_conchar(out, b64ETable[c1]) < 0 ||
                    b_conchar(out, (char)'=') < 0 ||
                    b_conchar(out, (char)'=') < 0) {
                        b_destroy(out);
                        return NULL;
                }
                break;
        case 2: break;
        }
        return out;
}

#define B64_PAD (-2)
#define B64_ERR (-1)

static int
base64DecodeSymbol(uchar alpha)
{
        if ((alpha >= 'A') && (alpha <= 'Z'))
                return (int)(alpha - 'A');

        if ((alpha >= 'a') && (alpha <= 'z'))
                return 26 + (int)(alpha - 'a');

        if ((alpha >= '0') && (alpha <= '9'))
                return 52 + (int)(alpha - '0');

        if (alpha == '+')
                return 62;

        if (alpha == '/')
                return 63;

        if (alpha == '=')
                return B64_PAD;

        return B64_ERR;
}

bstring *
bBase64DecodeEx(const bstring *b, int *boolTruncError)
{
        int i, v;
        uchar c0, c1, c2;
        bstring *out;
        if (b == NULL || b->slen < 0 || b->data == NULL)
                return NULL;
        if (boolTruncError)
                *boolTruncError = 0;
        out = b_fromcstr("");
        i = 0;
        while (1) {
                do {
                        if (i >= b->slen)
                                return out;
                        if (b->data[i] == '=') {
                                /* Bad "too early" truncation */
                                if (boolTruncError) {
                                        *boolTruncError = 1;
                                        return out;
                                }
                                b_destroy(out);
                                return NULL;
                        }
                        v = base64DecodeSymbol(b->data[i]);
                        i++;
                } while (v < 0);
                c0 = (uchar)(v << 2);
                do {
                        if (i >= b->slen || b->data[i] == '=') {
                                /* Bad "too early" truncation */
                                if (boolTruncError) {
                                        *boolTruncError = 1;
                                        return out;
                                }
                                b_destroy(out);
                                return NULL;
                        }
                        v = base64DecodeSymbol(b->data[i]);
                        i++;
                } while (v < 0);
                c0 |= (uchar)(v >> 4);
                c1 = (uchar)(v << 4);
                do {
                        if (i >= b->slen) {
                                if (boolTruncError) {
                                        *boolTruncError = 1;
                                        return out;
                                }
                                b_destroy(out);
                                return NULL;
                        }
                        if (b->data[i] == '=') {
                                i++;
                                if (i >= b->slen || b->data[i] != '=' ||
                                    b_conchar(out, c0) < 0) {
                                        if (boolTruncError) {
                                                *boolTruncError = 1;
                                                return out;
                                        }
                                        /* Missing "=" at the end. */
                                        b_destroy(out);
                                        return NULL;
                                }
                                return out;
                        }
                        v = base64DecodeSymbol(b->data[i]);
                        i++;
                } while (v < 0);
                c1 |= (uchar)(v >> 2);
                c2 = (uchar)(v << 6);
                do {
                        if (i >= b->slen) {
                                if (boolTruncError) {
                                        *boolTruncError = 1;
                                        return out;
                                }
                                b_destroy(out);
                                return NULL;
                        }
                        if (b->data[i] == '=') {
                                if (b_conchar(out, c0) < 0 ||
                                    b_conchar(out, c1) < 0) {
                                        if (boolTruncError) {
                                                *boolTruncError = 1;
                                                return out;
                                        }
                                        b_destroy(out);
                                        return NULL;
                                }
                                if (boolTruncError)
                                        *boolTruncError = 0;
                                return out;
                        }
                        v = base64DecodeSymbol(b->data[i]);
                        i++;
                } while (v < 0);
                c2 |= (uchar)(v);
                if (b_conchar(out, c0) < 0 || b_conchar(out, c1) < 0 ||
                    b_conchar(out, c2) < 0) {
                        if (boolTruncError) {
                                *boolTruncError = -1;
                                return out;
                        }
                        b_destroy(out);
                        return NULL;
                }
        }
}

#define UU_DECODE_BYTE(b) (((b) == (signed int)'`') ? 0 : (b) - (signed int)' ')

struct bUuInOut {
        bstring *src, *dst;
        int *badlines;
};

#define UU_MAX_LINELEN 45

static int
b_UuDecLine(void *parm, int ofs, int len)
{
        struct bUuInOut *io = (struct bUuInOut *)parm;
        bstring *s = io->src;
        bstring *t = io->dst;
        int i, llen, otlen, ret, c0, c1, c2, c3, d0, d1, d2, d3;
        if (len == 0)
                return 0;
        llen = UU_DECODE_BYTE(s->data[ofs]);
        ret = 0;
        otlen = t->slen;
        if (((unsigned)llen) > UU_MAX_LINELEN) {
                ret = -__LINE__;
                goto exit;
        }
        llen += t->slen;
        for (i = 1; i < s->slen && t->slen < llen; i += 4) {
                uchar outoctet[3];
                c0 = UU_DECODE_BYTE(d0 = (int)b_chare(s, i + ofs + 0, ' ' - 1));
                c1 = UU_DECODE_BYTE(d1 = (int)b_chare(s, i + ofs + 1, ' ' - 1));
                c2 = UU_DECODE_BYTE(d2 = (int)b_chare(s, i + ofs + 2, ' ' - 1));
                c3 = UU_DECODE_BYTE(d3 = (int)b_chare(s, i + ofs + 3, ' ' - 1));
                if (((unsigned)(c0 | c1) >= 0x40)) {
                        if (!ret)
                                ret = -__LINE__;
                        if (d0 > 0x60 || (d0 < (' ' - 1) && !isspace(d0)) ||
                            d1 > 0x60 || (d1 < (' ' - 1) && !isspace(d1))) {
                                t->slen = otlen;
                                goto exit;
                        }
                        c0 = c1 = 0;
                }
                outoctet[0] = (uchar)((c0 << 2) | ((unsigned)c1 >> 4));
                if (t->slen + 1 >= llen) {
                        if (0 > b_conchar(t, (char)outoctet[0]))
                                return -__LINE__;
                        break;
                }
                if ((unsigned)c2 >= 0x40) {
                        if (!ret)
                                ret = -__LINE__;
                        if (d2 > 0x60 || (d2 < (' ' - 1) && !isspace(d2))) {
                                t->slen = otlen;
                                goto exit;
                        }
                        c2 = 0;
                }
                outoctet[1] = (uchar)((c1 << 4) | ((unsigned)c2 >> 2));
                if (t->slen + 2 >= llen) {
                        if (0 > b_catblk(t, outoctet, 2))
                                return -__LINE__;
                        break;
                }
                if ((unsigned)c3 >= 0x40) {
                        if (!ret)
                                ret = -__LINE__;
                        if (d3 > 0x60 || (d3 < (' ' - 1) && !isspace(d3))) {
                                t->slen = otlen;
                                goto exit;
                        }
                        c3 = 0;
                }
                outoctet[2] = (uchar)((c2 << 6) | ((unsigned)c3));
                if (0 > b_catblk(t, outoctet, 3))
                        return -__LINE__;
        }
        if (t->slen < llen) {
                if (0 == ret)
                        ret = -__LINE__;
                t->slen = otlen;
        }
exit:
        if (ret && io->badlines) {
                (*io->badlines)++;
                return 0;
        }
        return ret;
}

bstring *
b_UuDecodeEx(const bstring *src, int *badlines)
{
        struct bStream *s, *d;
        bstring t;
        bstring *b;

        if (!src)
                return NULL;
        t = *src;              /* Short lifetime alias to header of src */
        s = bsFromBstrRef(&t); /* t is undefined after this */
        if (!s)
                return NULL;
        d = bsUuDecode(s, badlines);
        b = b_fromcstr_alloc(256, "");
        if (NULL == b)
                goto error;
        if (0 > bs_read(b, d, INT_MAX))
                goto error;
exit:
        bs_close(d);
        bs_close(s);
        return b;
error:
        b_destroy(b);
        b = NULL;
        goto exit;
}

struct bsUuCtx {
        struct bUuInOut io;
        struct bStream *sInp;
};

static size_t
bsUuDecodePart(void *buff, size_t elsize, size_t nelem, void *parm)
{
        static bstring eol = bt_init("\r\n");
        struct bsUuCtx *ctx = (struct bsUuCtx *)parm;
        size_t tsz;
        int l, lret;
        if (NULL == buff || NULL == parm)
                return 0;
        tsz = elsize * nelem;
check:
        /* If internal buffer has sufficient data, just output it */
        if (((size_t)ctx->io.dst->slen) > tsz) {
                memcpy(buff, ctx->io.dst->data, tsz);
                b_delete(ctx->io.dst, 0, (int)tsz);
                return nelem;
        }
decode:
        if (0 <= (l = b_inchr(ctx->io.src, 0, &eol))) {
                int ol = 0;
                bstring t;
                bstring *s = ctx->io.src;
                ctx->io.src = &t;
                do {
                        if (l > ol) {
                                bmid2tbstr(t, s, ol, l - ol);
                                lret = b_UuDecLine(&ctx->io, 0, t.slen);
                                if (0 > lret) {
                                        ctx->io.src = s;
                                        goto done;
                                }
                        }
                        ol = l + 1;
                        if (((size_t)ctx->io.dst->slen) > tsz)
                                break;
                        l = b_inchr(s, ol, &eol);
                } while (BSTR_ERR != l);
                b_delete(s, 0, ol);
                ctx->io.src = s;
                goto check;
        }
        if (BSTR_ERR !=
            bs_reada(ctx->io.src, ctx->sInp,
                    bs_bufflength(ctx->sInp, BSTR_BS_BUFF_LENGTH_GET)))
                goto decode;
        b_UuDecLine(&ctx->io, 0, ctx->io.src->slen);
done:
        /* Output any lingering data that has been translated */
        if (((size_t)ctx->io.dst->slen) > 0) {
                if (((size_t)ctx->io.dst->slen) > tsz)
                        goto check;
                memcpy(buff, ctx->io.dst->data, ctx->io.dst->slen);
                tsz = ctx->io.dst->slen / elsize;
                ctx->io.dst->slen = 0;
                if (tsz > 0)
                        return tsz;
        }
        /* Deallocate once EOF becomes triggered */
        b_destroy(ctx->io.dst);
        b_destroy(ctx->io.src);
        free(ctx);
        return 0;
}

struct bStream *
bsUuDecode(struct bStream *sInp, int *badlines)
{
        struct bsUuCtx *ctx = (struct bsUuCtx *)malloc(sizeof(struct bsUuCtx));
        struct bStream *sOut;
        if (NULL == ctx)
                return NULL;
        ctx->io.src = b_fromcstr("");
        ctx->io.dst = b_fromcstr("");
        if (NULL == ctx->io.dst || NULL == ctx->io.src)
                goto error;
        ctx->io.badlines = badlines;
        if (badlines)
                *badlines = 0;
        ctx->sInp = sInp;
        sOut = bs_open((bNread)bsUuDecodePart, ctx);
        if (NULL == sOut)
                goto error;
        return sOut;
error:
        b_destroy(ctx->io.dst);
        b_destroy(ctx->io.src);
        free(ctx);
        return NULL;
}

#define UU_ENCODE_BYTE(b) ((char)(((b) == 0) ? '`' : ((b) + ' ')))

bstring *
b_UuEncode(const bstring *src)
{
        bstring *out;
        int i, j, jm;
        unsigned int c0, c1, c2;
        if (src == NULL || src->slen < 0 || src->data == NULL)
                return NULL;
        if ((out = b_fromcstr("")) == NULL)
                return NULL;
        for (i = 0; i < src->slen; i += UU_MAX_LINELEN) {
                if ((jm = i + UU_MAX_LINELEN) > src->slen)
                        jm = src->slen;
                if (b_conchar(out, UU_ENCODE_BYTE(jm - i)) < 0) {
                        b_strFree(out);
                        break;
                }
                for (j = i; j < jm; j += 3) {
                        c0 = (unsigned int)b_char(src, j);
                        c1 = (unsigned int)b_char(src, j + 1);
                        c2 = (unsigned int)b_char(src, j + 2);
                        if (b_conchar(out, UU_ENCODE_BYTE((c0 & 0xFC) >> 2)) <
                                0 ||
                            b_conchar(out, UU_ENCODE_BYTE(((c0 & 0x03) << 4) |
                                                         ((c1 & 0xF0) >> 4))) <
                                0 ||
                            b_conchar(out, UU_ENCODE_BYTE(((c1 & 0x0F) << 2) |
                                                         ((c2 & 0xC0) >> 6))) <
                                0 ||
                            b_conchar(out, UU_ENCODE_BYTE((c2 & 0x3F))) < 0) {
                                b_strFree(out);
                                goto exit;
                        }
                }
                if (b_conchar(out, (char)'\r') < 0 ||
                    b_conchar(out, (char)'\n') < 0) {
                        b_strFree(out);
                        break;
                }
        }
exit:
        return out;
}

bstring *
b_YEncode(const bstring *src)
{
        int i;
        bstring *out;
        uchar c;
        if (src == NULL || src->slen < 0 || src->data == NULL)
                return NULL;
        if ((out = b_fromcstr("")) == NULL)
                return NULL;
        for (i = 0; i < src->slen; ++i) {
                c = (uchar)(src->data[i] + 42);
                if (c == '=' || c == '\0' || c == '\r' || c == '\n') {
                        if (0 > b_conchar(out, (char)'=')) {
                                b_destroy(out);
                                return NULL;
                        }
                        c += (uchar)64;
                }
                if (0 > b_conchar(out, c)) {
                        b_destroy(out);
                        return NULL;
                }
        }
        return out;
}

#define MAX_OB_LEN (64)

bstring *
b_YDecode(const bstring *src)
{
        int i, obl;
        bstring *out;
        uchar c, octetbuff[MAX_OB_LEN];
        if (src == NULL || src->slen < 0 || src->data == NULL)
                return NULL;
        if ((out = b_fromcstr("")) == NULL)
                return NULL;
        obl = 0;
        for (i = 0; i < src->slen; i++) {
                if ('=' == (c = src->data[i])) {
                        /* The = escape mode */
                        ++i;
                        if (i >= src->slen) {
                                b_destroy(out);
                                return NULL;
                        }
                        c = (uchar)(src->data[i] - 64);
                } else {
                        if ('\0' == c) {
                                b_destroy(out);
                                return NULL;
                        }
                        /* Extraneous CR/LFs are to be ignored. */
                        if (c == '\r' || c == '\n')
                                continue;
                }
                octetbuff[obl] = (uchar)((int)c - 42);
                obl++;
                if (obl >= MAX_OB_LEN) {
                        if (0 > b_catblk(out, octetbuff, obl)) {
                                b_destroy(out);
                                return NULL;
                        }
                        obl = 0;
                }
        }

        if (0 > b_catblk(out, octetbuff, obl)) {
                b_destroy(out);
                out = NULL;
        }
        return out;
}

bstring *
b_StrfTime(const char *fmt, const struct tm *timeptr)
{
#if defined(__TURBOC__) && !defined(__BORLANDC__)
        static bstring ns = bt_init("bStrfTime Not supported");
        fmt = fmt;
        timeptr = timeptr;
        return &ns;
#else
        bstring *buff;
        int n;
        size_t r;
        if (fmt == NULL)
                return NULL;
        /* Since the length is not determinable beforehand, a search is
         * performed using the truncating "strftime" call on increasing
         * potential sizes for the output result.
         */
        if ((n = (int)(2 * strlen(fmt))) < 16)
                n = 16;
        buff = b_fromcstr_alloc(n + 2, "");
        while (1) {
                if (BSTR_OK != b_alloc(buff, n + 2)) {
                        b_destroy(buff);
                        return NULL;
                }
                r = strftime((char *)buff->data, n + 1, fmt, timeptr);
                if (r > 0) {
                        buff->slen = (int)r;
                        break;
                }
                n += n;
        }
        return buff;
#endif
}

int
b_SetCstrChar(bstring *b, int pos, char c)
{
        if (NULL == b || b->mlen <= 0 || b->slen < 0 || b->mlen < b->slen)
                return BSTR_ERR;
        if (pos < 0 || pos > b->slen)
                return BSTR_ERR;
        if (pos == b->slen) {
                if ('\0' != c)
                        return b_conchar(b, c);
                return 0;
        }
        b->data[pos] = (uchar)c;
        if ('\0' == c)
                b->slen = pos;
        return 0;
}

int
b_SetChar(bstring *b, int pos, char c)
{
        if (NULL == b || b->mlen <= 0 || b->slen < 0 || b->mlen < b->slen)
                return BSTR_ERR;
        if (pos < 0 || pos > b->slen)
                return BSTR_ERR;
        if (pos == b->slen)
                return b_conchar(b, c);
        b->data[pos] = (uchar)c;
        return 0;
}

#define INIT_SECURE_INPUT_LENGTH (256)

bstring *
b_SecureInput(int maxlen, int termchar, bNgetc vgetchar, void *vgcCtx)
{
        size_t i, m, c;
        bstring *b, *t;
        if (!vgetchar)
                return NULL;
        b = b_fromcstr_alloc(INIT_SECURE_INPUT_LENGTH, "");
        if (!b)
                return NULL;
        if ((c = UCHAR_MAX + 1) == (size_t)termchar)
                c++;
        for (i = 0;; i++) {
                if ((size_t)termchar == c ||
                    (maxlen > 0 && i >= (size_t)maxlen))
                        c = EOF;

                else
                        c = vgetchar(vgcCtx);
                if ((size_t)EOF == c)
                        break;
                if (i + 1 >= (size_t)b->mlen) {
                        /* Double size, but deal with unusual case of numeric
                         * overflows
                         */
                        if ((m = b->mlen << 1) <= (size_t)b->mlen &&
                            (m = b->mlen + 1024) <= (size_t)b->mlen &&
                            (m = b->mlen + 16) <= (size_t)b->mlen &&
                            (m = b->mlen + 1) <= (size_t)b->mlen)
                                t = NULL;

                        else
                                t = b_fromcstr_alloc(m, "");
                        if (t)
                                memcpy(t->data, b->data, i);
                        b_SecureDestroy(b); /* Cleanse previous buffer */
                        b = t;
                        if (!b)
                                return b;
                }
                b->data[i] = (uchar)c;
        }
        b->slen = i;
        b->data[i] = (uchar)'\0';
        return b;
}

#define BWS_BUFF_SZ (BUFSIZ)

struct bwriteStream {
        bstring *buff;   /* Buffer for underwrites */
        void *parm;      /* The stream handle for core stream */
        bNwrite writeFn; /* fwrite work-a-like fnptr for core stream */
        int isEOF;       /* track stream's EOF state */
        int minBuffSz;
};

struct bwriteStream *
b_wsOpen(bNwrite writeFn, void *parm)
{
        struct bwriteStream *ws;
        if (NULL == writeFn)
                return NULL;
        ws = (struct bwriteStream *)malloc(sizeof(struct bwriteStream));
        if (ws) {
                if (NULL == (ws->buff = b_fromcstr(""))) {
                        free(ws);
                        ws = NULL;
                } else {
                        ws->parm = parm;
                        ws->writeFn = writeFn;
                        ws->isEOF = 0;
                        ws->minBuffSz = BWS_BUFF_SZ;
                }
        }
        return ws;
}

#define internal_bwswriteout(ws, b)                                      \
        {                                                                \
                if ((b)->slen > 0) {                                     \
                        if (1 != ((ws)->writeFn((b)->data, (b)->slen, 1, \
                                              (ws)->parm))) {            \
                                (ws)->isEOF = 1;                         \
                                return BSTR_ERR;                         \
                        }                                                \
                }                                                        \
        }

int
b_wsWriteFlush(struct bwriteStream *ws)
{
        if (NULL == ws || ws->isEOF || 0 >= ws->minBuffSz ||
            NULL == ws->writeFn || NULL == ws->buff)
                return BSTR_ERR;
        internal_bwswriteout(ws, ws->buff);
        ws->buff->slen = 0;
        return 0;
}

int
b_wsWriteBstr(struct bwriteStream *ws, const bstring *b)
{
        bstring t;
        int l;
        if (NULL == ws || NULL == b || NULL == ws->buff || ws->isEOF ||
            0 >= ws->minBuffSz || NULL == ws->writeFn)
                return BSTR_ERR;
        /* Buffer prepacking optimization */
        if (b->slen > 0 && ws->buff->mlen - ws->buff->slen > b->slen) {
                static bstring empty = bt_init("");
                if (0 > b_concat(ws->buff, b))
                        return BSTR_ERR;
                return b_wsWriteBstr(ws, &empty);
        }
        if (0 > (l = ws->minBuffSz - ws->buff->slen)) {
                internal_bwswriteout(ws, ws->buff);
                ws->buff->slen = 0;
                l = ws->minBuffSz;
        }
        if (b->slen < l)
                return b_concat(ws->buff, b);
        if (0 > b_catblk(ws->buff, b->data, l))
                return BSTR_ERR;
        internal_bwswriteout(ws, ws->buff);
        ws->buff->slen = 0;
        bmid2tbstr(t, (bstring *)b, l, b->slen);
        if (t.slen >= ws->minBuffSz) {
                internal_bwswriteout(ws, &t);
                return 0;
        }
        return b_assign(ws->buff, &t);
}

int
b_wsWriteBlk(struct bwriteStream *ws, void *blk, int len)
{
        if (NULL == blk || len < 0)
                return BSTR_ERR;
        
        bstring t = blk2tbstr(blk, len);
        return b_wsWriteBstr(ws, &t);
}

int
b_wsIsEOF(const struct bwriteStream *ws)
{
        if (NULL == ws || NULL == ws->buff || 0 > ws->minBuffSz ||
            NULL == ws->writeFn)
                return BSTR_ERR;
        return ws->isEOF;
}

int
b_wsBuffLength(struct bwriteStream *ws, int sz)
{
        int oldSz;
        if (ws == NULL || sz < 0)
                return BSTR_ERR;
        oldSz = ws->minBuffSz;
        if (sz > 0)
                ws->minBuffSz = sz;
        return oldSz;
}

void *
b_wsClose(struct bwriteStream *ws)
{
        void *parm = NULL;
        if (ws) {
                if (NULL == ws->buff || 0 >= ws->minBuffSz || NULL == ws->writeFn)
                        return NULL;
                b_wsWriteFlush(ws);
                parm = ws->parm;
                ws->parm = NULL;
                ws->minBuffSz = -1;
                ws->writeFn = NULL;
                b_strFree(ws->buff);
                free(ws);
        }
        return parm;
}
