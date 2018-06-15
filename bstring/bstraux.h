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

/**
 * \file
 * \brief C example that implements trivial additional functions
 *
 * This file is not a necessary part of the core bstring * library itself, but
 * is just an auxilliary module which includes miscellaneous or trivial
 * functions.
 */
 
#ifndef BSTRAUX_H
#  define BSTRAUX_H

#  include "bstrlib.h"
#  include <time.h>

#  ifdef __cplusplus
extern "C" {
#  endif

#ifndef BSTR_PUBLIC
#  define BSTR_PUBLIC extern
#endif

/* Safety mechanisms */
#define b_strDeclare(b) bstring *(b) = NULL
#define b_strFree(b)                                                           \
        do {                                                                   \
                if ((b) != NULL && (b)->slen >= 0 && (b)->mlen >= (b)->slen) { \
                        b_destroy(b);                                          \
                        (b) = NULL;                                            \
                }                                                              \
        } while (0)

#ifdef BSTRLIB_NAMESPACE_POLLUTE
/* Backward compatibilty with previous versions of Bstrlib */
#  define b_Assign(a, b)                   (b_assign((a), (b)))
#  define b_Base64Decode(b)                (b_Base64DecodeEx((b), NULL))
#  define b_CaselessCmp(a, b)              (b_stricmp((a), (b)))
#  define b_CaselessNCmp(a, b, n)          (b_strnicmp((a), (b), (n)))
#  define b_CatBlk(b, s, len)              (b_catblk((b), (s), (len)))
#  define b_CatCstr(b, s)                  (b_catcstr((b), (s)))
#  define b_CatStatic(b, s)                (b_CatBlk((b), ("" s ""), sizeof(s) - 1))
#  define b_Lowercase(b)                   (b_tolower(b))
#  define b_ReplaceAll(b, find, repl, pos) (b_findreplace((b), (find), (repl), (pos)))
#  define b_Strchr(b, c)                   (b_strchr((b), (c)))
#  define b_StrchrFast(b, c)               (b_strchr((b), (c)))
#  define b_Subs(b, pos, len, a, c)        (b_replace((b), (pos), (len), (a), (unsigned char)(c)))
#  define b_Trunc(b, n)                    (b_trunc((b), (n)))
#  define b_Uppercase(b)                   (b_toupper(b))
#  define b_UuDecode(b)                    (b_UuDecodeEx((b), NULL))
#endif

/*============================================================================*/
/* Unusual functions */

/**
 * Create a bStream whose contents are a copy of the bstring * passed in.
 *
 * This allows the use of all the bStream APIs with bstrings.
 */
BSTR_PUBLIC struct bStream *bs_FromBstr(const bstring *b);

/**
 * Return with a string of the last n characters of b.
 */
BSTR_PUBLIC bstring *b_Tail(bstring *b, int n);

/**
 * Return with a string of the first n characters of b.
 */
BSTR_PUBLIC bstring *b_Head(bstring *b, int n);

/**
 * Sets the character at position pos to the character c in the bstring * a.
 *
 * If the character c is NUL ('\0') then the string is truncated at this point.
 * Note: this does not enable any other '\0' character in the bstring * as
 * terminator indicator for the string. pos must be in the position between 0
 * and b->slen inclusive, otherwise BSTR_ERR will be returned.
 */
BSTR_PUBLIC int b_SetCstrChar(bstring *a, int pos, char c);

/**
 * Sets the character at position pos to the character c in the bstring * a.
 *
 * The string is not truncated if the character c is NUL ('\0'). pos must be in
 * the position between 0 and b->slen inclusive, otherwise BSTR_ERR will be
 * returned.
 */
BSTR_PUBLIC int b_SetChar(bstring *b, int pos, char c);

/**
 * Fill a given bstring * with the character in parameter c, for a length n.
 */
BSTR_PUBLIC int b_Fill(bstring *a, char c, int len);

/**
 * Replicate the contents of b end to end n times and replace it in b.
 */
BSTR_PUBLIC int b_Replicate(bstring *b, int n);

/**
 * Reverse the contents of b in place.
 */
BSTR_PUBLIC int b_Reverse(bstring *b);

/**
 * Insert a repeated sequence of a given character into the string at position
 * pos for a length len.
 */
BSTR_PUBLIC int b_InsertChrs(bstring *b, int pos, int len, unsigned char c, unsigned char fill);

/**
 * Takes a format string that is compatible with strftime and a struct tm
 * pointer, formats the time according to the format string and outputs the
 * bstring * as a result.
 *
 * Note that if there is an early generation of a '\0' character, the bstring *
 * will be truncated to this end point.
 */
BSTR_PUBLIC bstring *b_StrfTime(const char *fmt, const struct tm *timeptr);

#define b_AscTime(t) (b_StrfTime("%c\n", (t)))
#define b_CTime(t)   ((t) ? b_AscTime(localtime(t)) : NULL)


/*============================================================================*/
/* Spacing formatting */

/**
 * Left justify a string.
 */
BSTR_PUBLIC int b_JustifyLeft(bstring *b, int space);

/**
 * Right justify a string to within a given width.
 */
BSTR_PUBLIC int b_JustifyRight(bstring *b, int width, int space);

/**
 * Stretch a string to flush against left and right margins by evenly
 * distributing additional white space between words.
 *
 * If the line is too long to be margin justified, it is left justified.
 */
BSTR_PUBLIC int b_JustifyMargin(bstring *b, int width, int space);

/**
 * Center a string's non-white space characters to within a given width by
 * inserting whitespaces at the beginning.
 */
BSTR_PUBLIC int b_JustifyCenter(bstring *b, int width, int space);

/* Esoteric standards specific functions */

/**
 * Convert a bstring * to a netstring.
 *
 * See http://cr.yp.to/proto/netstrings.txt for a description of netstrings.
 *
 * Note: 1) The value returned should be freed with a call to b_cstrfree() at
 *          the point when it will no longer be referenced to avoid a memory
 *          leak.
 *       2) If the returned value is non-NULL, then it also '\0' terminated
 *          in the character position one past the "," terminator.
 */
BSTR_PUBLIC char *b_Str2NetStr(const bstring *b);

/**
 * Convert a netstring to a bstring *.
 *
 * See http://cr.yp.to/proto/netstrings.txt for a description of netstrings.
 *
 * Note that the terminating "," *must* be present, however a following '\0'
 * is *not* required.
 */
BSTR_PUBLIC bstring *b_NetStr2Bstr(const char *buf);

/**
 * Generate a base64 encoding.
 *
 * See: RFC1341
 */
BSTR_PUBLIC bstring *b_Base64Encode(const bstring *b);

/**
 * Decode a base64 block of data.
 *
 * All MIME headers are assumed to have been removed.
 *
 * See: RFC1341
 */
BSTR_PUBLIC bstring *b_Base64DecodeEx(const bstring *b, int *boolTruncError);

/**
 * Creates a bStream which performs the UUDecode of an an input stream.
 *
 * If there are errors in the decoding, they are counted up and returned in
 * "badlines", if badlines is not NULL. It is assumed that the "begin" and
 * "end" lines have already been stripped off. The potential security problem
 * of writing the filename in the begin line is something that is beyond the
 * scope of a portable library.
 */
BSTR_PUBLIC struct bStream *bsUuDecode(struct bStream *sInp, int *badlines);

/**
 * Performs a UUDecode of a block of data.
 *
 * If there are errors in the decoding, they are counted up and returned in
 * "badlines", if badlines is not NULL. It is assumed that the "begin" and
 * "end" lines have already been stripped off. The potential security problem
 * of writing the filename in the begin line is something that is beyond the
 * scope of a portable library.
 */

BSTR_PUBLIC bstring *b_UuDecodeEx(const bstring *src, int *badlines);

/**
 * Performs a UUEncode of a block of data.
 *
 * The "begin" and "end" lines are not appended.
 */
BSTR_PUBLIC bstring *b_UuEncode(const bstring *src);

/**
 * Performs a YEncode of a block of data.
 *
 * No header or tail info is appended.
 *
 * See: http://www.yenc.org/whatis.htm, http://www.yenc.org/yenc-draft.1.3.txt
 */
BSTR_PUBLIC bstring *b_YEncode(const bstring *src);

/**
 * Performs a YDecode of a block of data.
 *
 * See: http://www.yenc.org/whatis.htm, http://www.yenc.org/yenc-draft.1.3.txt
 */
BSTR_PUBLIC bstring *b_YDecode(const bstring *src);


/*============================================================================*/
/* Writable stream */
typedef int (*bNwrite)(const void *buf, size_t elsize, size_t nelem,
                       void *parm);

/**
 * Wrap a given open stream (described by a fwrite work-a-like function pointer
 * and stream handle) into an open bwriteStream suitable for write streaming
 * functions.
 */
BSTR_PUBLIC struct bwriteStream *b_wsOpen(bNwrite writeFn, void *parm);

/**
 * Send a bstring * to a bwriteStream.
 *
 * If the stream is at EOF BSTR_ERR is returned. Note that there is no
 * deterministic way to determine the exact cut off point where the core stream
 * stopped accepting data.
 */
BSTR_PUBLIC int b_wsWriteBstr(struct bwriteStream *stream, const bstring *b);

/**
 * Send a block of data a bwriteStream.
 *
 * If the stream is at EOF BSTR_ERR is returned.
 */
BSTR_PUBLIC int b_wsWriteBlk(struct bwriteStream *stream, void *blk, int len);

/**
 * Force any pending data to be written to the core stream.
 */
BSTR_PUBLIC int b_wsWriteFlush(struct bwriteStream *stream);

/**
 * Returns 0 if the stream is currently writable, 1 if the core stream has
 * responded by not accepting the previous attempted write.
 */
BSTR_PUBLIC int b_wsIsEOF(const struct bwriteStream *stream);

/**
 * Set the length of the buffer used by the bwsStream.
 *
 * If sz is zero, the length is not set. This function returns with the
 * previous length.
 */
BSTR_PUBLIC int b_wsBuffLength(struct bwriteStream *stream, int sz);

/**
 * Close the bwriteStream, and return the handle to the stream that was
 * originally used to open the given stream.
 *
 * Note that even if the stream is at EOF it still needs to be closed with a
 * call to bwsClose.
 */
BSTR_PUBLIC void *b_wsClose(struct bwriteStream *stream);


/*============================================================================*/
/* Security functions */
#define b_SecureDestroy(b)                                              \
        do {                                                           \
                if ((b) && (b)->mlen > 0 && (b)->data) {               \
                        (void)memset((b)->data, 0, (size_t)(b)->mlen); \
                }                                                      \
                (void)b_destroy((b));                                   \
        } while (0)

#define b_SecureWriteProtect(t)                                               \
        do {                                                                 \
                if ((t).mlen >= 0) {                                         \
                        if ((t).mlen > (t).slen)                             \
                                {                                            \
                                        (void)memset((t).data + (t).slen, 0, \
                                                     (size_t)(t).mlen -      \
                                                         (t).slen);          \
                                }                                            \
                        (t).mlen = -1;                                       \
                }                                                            \
        } while (0)

/**
 * Read input from an abstracted input interface, for a length of at most
 * maxlen characters.
 *
 * If maxlen <= 0, then there is no length limit put on the input. The result
 * is terminated early if vgetchar() return EOF or the user specified value
 * termchar.
 *
 */
BSTR_PUBLIC bstring *b_SecureInput(int maxlen, int termchar, bNgetc vgetchar, void *vgcCtx);

#ifdef __cplusplus
}
#endif

#endif /* BSTRAUX_H */
