/*
 * Do I need a copyright here?
 */

#include "private.h"
#include <assert.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <talloc.h>

#include "bstring.h"


/*============================================================================*/
/*============================================================================*/
#ifdef _WIN32
/*-
* SPDX-License-Identifier: BSD-3-Clause
*
* Copyright (c) 1990, 1993
*	The Regents of the University of California.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of the University nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
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
*/

static inline char *
BSTRING_strsep(char **stringp, const char *delim)
{
        const char *delimp;
        char       *ptr, *tok;
        char        src_ch, del_ch;

        if ((ptr = tok = *stringp) == NULL)
                return (NULL);

        for (;;) {
                src_ch = *ptr++;
                delimp = delim;
                do {
                        if ((del_ch = *delimp++) == src_ch) {
                                if (src_ch == '\0')
                                        ptr = NULL;
                                else
                                        ptr[-1] = '\0';
                                *stringp = ptr;
                                return (tok);
                        }
                } while (del_ch != '\0');
        }
        /* NOTREACHED */
}

#define strsep BSTRING_strsep

int
BSTRING_dprintf(int fd, char *fmt, ...)
{
        FILE   *fds;
        va_list ap;
        int     ret;
        int     fdx = _open_osfhandle(fd, 0);

        va_start(ap, fmt);
        fdx = dup(fdx);

        if ((fds = fdopen(fdx, "w")) == NULL) {
                va_end(ap);
                return (-1);
        }
        ret = vfprintf(fds, fmt, ap);
        fclose(fds);
        va_end(ap);

        return (ret);
}
#endif

#ifndef HAVE_ERR
__attribute__((__format__(gnu_printf, 2, 3))) void
BSTRING_warn_(bool print_err, const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        char buf[8192];
        snprintf(buf, 8192, "%s\n", fmt);
        va_end(ap);

        /* if (print_err)
                snprintf(buf, 8192, "%s: %s\n", fmt, strerror(errno));
        else
                snprintf(buf, 8192, "%s\n", fmt); */
        /* vfprintf(stderr, buf, ap); */
        if (print_err)
                perror(buf);
        else
                fputs(buf, stderr);
#  ifdef _WIN32
        fflush(stderr);
#  endif
}
#endif

/*============================================================================*/
/*============================================================================*/

int
b_memsep(bstring *dest, bstring *stringp, const char delim)
{
        if (!dest || !stringp || NO_WRITE(stringp))
                errx(1, "invalid input strings");

        dest->data = stringp->data;

        if (!stringp->data || stringp->slen == 0)
                return 0;

        const int64_t pos = b_strchr(stringp, delim);
        int           ret = 1;

        if (pos >= 0) {
                dest->data[pos] = '\0';
                dest->slen      = pos;
                stringp->data   = stringp->data + pos + 1U;
                stringp->slen  -= pos + 1U;
        } else {
                dest->slen    = stringp->slen;
                stringp->data = NULL;
                stringp->slen = 0;
        }

        return ret;
}

b_list *
b_split_char(bstring *tosplit, const int delim, const bool destroy)
{
        if (INVALID(tosplit) || (!destroy && NO_WRITE(tosplit)))
                RETURN_NULL();

        bstring *split = destroy ? tosplit : b_strcpy(tosplit);
        b_list  *ret   = b_list_create();

        bstring *tok = BSTR_NULL_INIT;
        bstring *buf = &(bstring){split->data, split->slen, 0, split->flags};

        while (b_memsep(tok, buf, (char)delim)) {
                b_list_append(ret, b_fromblk(tok->data, tok->slen));
        }

        b_destroy(split);
        return ret;
}

/*============================================================================*/
/* SOME CRAPPY ADDITIONS! */
/*============================================================================*/


void
_b_fwrite(FILE *fp, bstring *bstr, ...)
{
        va_list va;
        va_start(va, bstr);
        for (;;bstr = va_arg(va, bstring *)) {
                if (bstr) {
                        if (bstr->flags & BSTR_LIST_END)
                                break;
                        if (bstr->data && bstr->slen > 0)
                                fwrite(bstr->data, 1, bstr->slen, fp);
                }
        }
        va_end(va);
}

int
_b_write(const int fd, bstring *bstr, ...)
{
        va_list ap;
        va_start(ap, bstr);
        for (;;bstr = va_arg(ap, bstring *)) {
                if (bstr) {
                        if (bstr->flags & BSTR_LIST_END)
                                break;
                        if (bstr->data && bstr->slen > 0) {
                                ssize_t n, total = 0;
                                int     tmp;
                                errno = 0;
                                do {
                                        n = write(fd, bstr->data, bstr->slen);
                                } while (n > 0 && (unsigned)(total += n) < bstr->slen);
                                if ((tmp = errno)) {
                                        va_end(ap);
                                        return tmp;
                                }
                        }
                }
        }
        va_end(ap);

        return BSTR_OK;
}

void
_b_free_all(bstring **bstr, ...)
{
        va_list va;
        va_start(va, bstr);
        for (;;bstr = va_arg(va, bstring **)) {
                if (bstr && *bstr) {
                        if ((*bstr)->flags & BSTR_LIST_END)
                                break;
                        if ((*bstr)->data) {
                                b_destroy(*bstr);
                                *bstr = NULL;
                        }
                }
        }
        va_end(va);
}

bstring *
_b_concat_all(const bstring *join, const int join_end, ...)
{
        unsigned       size   = 0;
        const unsigned j_size = (join && join->data) ? join->slen : 0;

        va_list va, va2;
        va_start(va, join_end);
        for (;;) {
                const bstring *src = va_arg(va, const bstring *);
                if (src) {
                        if (src->flags & BSTR_LIST_END)
                                break;
                        if (src->data)
                                size += src->slen + j_size;
                }
        }
        va_end(va);

        bstring *dest = b_alloc_null(size + 1 + j_size);
        dest->slen = 0;
        va_start(va2, join_end);

        for (;;) {
                const bstring *src = va_arg(va2, const bstring *);
                if (src) {
                        if (src->flags & BSTR_LIST_END)
                                break;
                        if (src->data) {
                                memcpy((dest->data + dest->slen), src->data, src->slen);
                                dest->slen += src->slen;
                                if (j_size) {
                                        memcpy((dest->data + dest->slen), join->data,
                                               join->slen);
                                        dest->slen += j_size;
                                }
                        }
                }
        }
        va_end(va2);

        if (dest->slen != size) {
                b_destroy(dest);
                RETURN_NULL();
        }

        if (join_end || !join) 
                dest->data[dest->slen] = '\0';
        else
                dest->data[(dest->slen -= join->slen)] = '\0';
        if (join && !join_end)
                dest->slen -= join->slen;

        return dest;
}

int
_b_append_all(bstring *dest, const bstring *join, const int join_end, ...)
{
        unsigned       size   = dest->slen;
        const unsigned j_size = (join && join->data) ? join->slen : 0;

        va_list va, va2;
        va_start(va, join_end);
        /* va_copy(va2, va); */

        for (;;) {
                const bstring *src = va_arg(va, const bstring *);
                if (src) {
                        if (src->flags & BSTR_LIST_END)
                                break;
                        if (src->data)
                                size += src->slen + j_size;
                }
        }
        va_end(va);

        b_alloc(dest, size + 1);
        va_start(va2, join_end);

        for (;;) {
                const bstring *src = va_arg(va2, const bstring *);
                if (src) {
                        if (src->flags & BSTR_LIST_END)
                                break;
                        if (src->data) {
                                memcpy((dest->data + dest->slen), src->data, src->slen);
                                dest->slen += src->slen;
                                if (j_size) {
                                        memcpy((dest->data + dest->slen), join->data,
                                               join->slen);
                                        dest->slen += join->slen;
                                }
                        }
                }
        }
        va_end(va2);

        if (join_end || !join) 
                dest->data[dest->slen] = '\0';
        else
                dest->data[(dest->slen -= join->slen)] = '\0';
        if (join && !join_end)
                size -= join->slen;

        return (dest->slen == size) ? BSTR_OK : BSTR_ERR;
}

/*============================================================================*/

int 
b_strcmp_fast(const bstring *a, const bstring *b)
{
        if (a->slen == b->slen)
                return memcmp(a->data, b->data, a->slen);
        else
                return a->slen - b->slen;
}

int
b_strcmp_fast_wrap(const void *vA, const void *vB)
{
        const bstring *sA = *(bstring const*const*const)(vA);
        const bstring *sB = *(bstring const*const*const)(vB);

        return b_strcmp_fast(sA, sB);
}

int
b_strcmp_wrap(const void *const vA, const void *const vB)
{
        return b_strcmp((*(bstring const*const*const)(vA)),
                        (*(bstring const*const*const)(vB)));
}

bstring *
b_steal(void *blk, const unsigned len)
{
        if (!blk || len == 0)
                RETURN_NULL();
        bstring *ret = talloc(NULL, bstring);
        *ret = (bstring){
            .slen  = len,
            .mlen  = len + 1,
            .data  = (uchar *)blk,
            .flags = BSTR_STANDARD,
        };
        talloc_steal(ret, blk);
        talloc_set_destructor(ret, b_free);
        return ret;
}

bstring *
b_refblk(void *blk, const unsigned len)
{
        if (!blk || len == 0)
                RETURN_NULL();
        bstring *ret = talloc(NULL, bstring);
        *ret = (bstring){
            .slen  = len,
            .mlen  = len,
            .data  = (uchar *)blk,
            .flags = BSTR_WRITE_ALLOWED | BSTR_FREEABLE
        };
        talloc_set_destructor(ret, b_free);
        return ret;
}

bstring *
b_clone(const bstring *const src)
{
        if (INVALID(src))
                RETURN_NULL();

        bstring *ret = talloc(NULL, bstring);
        *ret = (bstring){.slen  = src->slen,
                         .mlen  = src->mlen,
                         .flags = (src->flags & (~((uint8_t)BSTR_DATA_FREEABLE))),
                         .data  = src->data};
        ret->flags |= BSTR_CLONE;

        b_writeprotect(ret);
        talloc_set_destructor(ret, b_free);
        return ret;
}

bstring *
b_clone_swap(bstring *src)
{
        if (INVALID(src) || NO_WRITE(src))
                RETURN_NULL();

        bstring *ret = talloc(NULL, bstring);
        *ret = (bstring){.slen  = src->slen,
                         .mlen  = src->mlen,
                         .flags = src->flags,
                         .data  = src->data};
        talloc_steal(ret, src->data);

        src->flags &= (~((uint8_t)BSTR_DATA_FREEABLE));
        src->flags |= BSTR_CLONE;

        b_writeprotect(src);
        talloc_set_destructor(ret, b_free);
        return ret;
}

/*============================================================================*/
/* Proper strstr replacements */
/*============================================================================*/


int64_t
b_strstr(const bstring *const haystack, const bstring *needle, const unsigned pos)
{
        if (INVALID(haystack) || INVALID(needle))
                RUNTIME_ERROR();
        if (haystack->slen < needle->slen || pos > haystack->slen)
                return (-1);

        char *ptr = strstr((char *)(haystack->data + pos), BS(needle));

        if (!ptr)
                return (-1);

        return (int64_t)psub(ptr, haystack->data);
}

b_list *
b_strsep(bstring *ostr, const char *const delim, const int refonly)
{
        if (INVALID(ostr) || NO_WRITE(ostr) || !delim)
                RETURN_NULL();

        b_list  *ret  = b_list_create();
        bstring *tok = BSTR_NULL_INIT;
        bstring *str = &(bstring){ostr->data, ostr->slen, 0, ostr->flags};

        if (refonly)
                while (b_memsep(tok, str, delim[0]))
                        b_list_append(ret, b_refblk(tok->data, tok->slen));
        else
                while (b_memsep(tok, str, delim[0]))
                        b_list_append(ret, b_fromblk(tok->data, tok->slen));

        return ret;
}

/*============================================================================*/
/* strpbrk */
/*============================================================================*/


int64_t
b_strpbrk_pos(const bstring *bstr, const unsigned pos, const bstring *delim)
{
        if (INVALID(bstr) || INVALID(delim) || bstr->slen == 0 ||
                    delim->slen == 0 || pos > bstr->slen)
                RUNTIME_ERROR();

        for (unsigned i = pos; i < bstr->slen; ++i)
                for (unsigned x = 0; x < delim->slen; ++x)
                        if (bstr->data[i] == delim->data[x])
                                return (int64_t)i;

        return (-1LL);
}

int64_t
b_strrpbrk_pos(const bstring *bstr, const unsigned pos, const bstring *delim)
{
        if (INVALID(bstr) || INVALID(delim) || bstr->slen == 0 ||
                    delim->slen == 0 || pos > bstr->slen)
                RUNTIME_ERROR();

        unsigned i = pos;
        do {
                for (unsigned x = 0; x < delim->slen; ++x)
                        if (bstr->data[i] == delim->data[x])
                                return (int64_t)i;
        } while (i-- > 0);

        return (-1LL);
}

/*============================================================================*/
/* Path operations */
/*============================================================================*/


bstring *
b_dirname(const bstring *path)
{
        if (INVALID(path))
                RETURN_NULL();
        int64_t pos;

#ifdef _WIN32
        pos = b_strrpbrk(path, B("/\\"));
#else
        pos = b_strrchr(path, '/');
        if (pos == 0)
                ++pos;
#endif

        if (pos >= 0)
                return b_fromblk(path->data, pos);

        RETURN_NULL();
}

bstring *
b_basename(const bstring *path)
{
        if (INVALID(path))
                RETURN_NULL();
        int64_t pos;

#ifdef _WIN32
        pos = b_strrpbrk(path, B("/\\"));
#else
        pos = b_strrchr(path, '/');
        if (pos == 0)
                ++pos;
#endif

        if (pos >= 0)
                return b_fromblk(path->data + pos + 1U, path->slen - pos - 1U);

        RETURN_NULL();
}

bstring *
b_regularize_path(bstring *path)
{
        if (INVALID(path) || NO_WRITE(path))
                RETURN_NULL();
        
#if defined(_WIN32) || defined(_WIN64)
        for (unsigned i = 0; i < path->slen; ++i)
                if (path->data[i] == '/')
                        path->data[i] = '\\';
#endif

        return path;
}

bstring *
b_quickread(const char *const __restrict fmt, ...)
{
        if (!fmt)
                RETURN_NULL();
        va_list ap;
        char buf[PATH_MAX + 1];
        va_start(ap, fmt);
        vsnprintf(buf, PATH_MAX + 1, fmt, ap);
        va_end(ap);

        struct stat st;
        FILE       *fp = fopen(buf, "rb");
        if (!fp)
                RETURN_NULL();
        fstat(fileno(fp), &st);
        if (st.st_size == 0) {
                fclose(fp);
                return NULL;
        }

        bstring      *ret   = b_alloc_null(st.st_size + 1);
        const ssize_t nread = fread(ret->data, 1, st.st_size, fp);
        fclose(fp);
        if (nread < 0) {
                b_free(ret);
                RETURN_NULL();
        }

        ret->slen        = (unsigned)nread;
        ret->data[nread] = '\0';
        return ret;
}

#define INIT_READ ((size_t)(1LLU << 20))

bstring *
b_read_stdin(void)
{
        bstring *buf = b_create(INIT_READ + 1LLU);

        for (;;) {
                size_t nread = fread(buf->data + buf->slen, 1, INIT_READ, stdin);
                if (nread > 0) {
                        buf->slen += nread;
                        if (feof(stdin))
                                break;
                        /* buf = xrealloc(buf, total + INIT_READ); */
                        b_alloc(buf, buf->slen + INIT_READ);
                } else {
                        break;
                }
        }

        buf->data[buf->slen] = '\0';
        return buf;
}

#if 0
#define INIT_READ ((size_t)(1 << 20))

static size_t
getstdin(char **dest, FILE *fp)
{
        size_t  total = 0;
        char   *buf   = xmalloc(INIT_READ+1LLU);
        
        for (;;) {
                size_t nread = fread(buf + total, 1, INIT_READ, fp);
                if (nread > 0) {
                        total += nread;
                        if (feof(fp))
                                break;
                        char *tmp = realloc(buf, total + INIT_READ);
                        buf       = tmp;
                } else {
                        break;
                }
        }

        buf[total] = '\0';
        *dest      = buf;
        return total;
}

bstring *
b_read_fd(const int fd)
{
        FILE *fp = fdopen(fd, "rb");
        char *buf;
        size_t total = getstdin(&buf, fp);
        fclose(fp);

        bstring *ret = xmalloc(sizeof *ret);
        *ret = (bstring){total, total+1, (uchar *)buf, BSTR_STANDARD};
        return ret;
}
#endif

#define INIT_READ ((size_t)(8192LLU))
#ifdef DOSISH
#  define SSIZE_T size_t
#else
#  define SSIZE_T ssize_t
#endif

bstring *
b_read_fd(const int fd)
{
        bstring *ret = b_alloc_null(INIT_READ + 1U);

        for (;;) {
                SSIZE_T nread = read(fd, (ret->data + ret->slen), INIT_READ);
                if (nread > 0) {
                        ret->slen += nread;
                        if ((size_t)nread < INIT_READ) {
                                /* eprintf("breaking\n"); */
                                break;
                        }
                        b_growby(ret, INIT_READ);
                } else {
                        break;
                }
        }

        ret->data[ret->slen] = '\0';
        return ret;
}


#if 0
bstring *
b_read_fd(const int fd)
{
        /* size_t   total = 0; */
        bstring *ret   = b_alloc_null(INIT_READ+1LLU);
        /* FILE    *fp    = fdopen(fd, "rb"); */
        /* bstring *ret   = xmalloc(sizeof *ret); */
        /* *ret = (bstring){0, INIT_READ+1LLU, xcalloc(1, INIT_READ+1LLU), BSTR_STANDARD}; */

        int ch = 0;
        while (read(fd, &ch, 1) != EOF)
                b_conchar(ret, ch);
#if 0
        for (;;) {
                size_t nread = fread(ret->data + total, 1, INIT_READ, fp);
                /* ssize_t nread = read(fd, ret->data + total, INIT_READ); */
                /* if (nread >= 0) { */
                total += nread;
                if (feof(fp))
                        break;
                /* b_alloc(ret, total + INIT_READ); */
                b_growby(ret, INIT_READ);
                /* } else { */
                        /* break; */
                /* } */
        }
#endif

        /* fclose(fp); */
        /* ret->slen        = total; */
        return ret;
}
#endif

/*============================================================================*/
/* String Modiying  */
/*============================================================================*/

int
b_insert_char(bstring *str, const unsigned location, const int ch)
{
        if (location >= str->slen)
                RUNTIME_ERROR();
        if (INVALID(str) || NO_WRITE(str))
                RUNTIME_ERROR();
        if (str->mlen <= str->slen - 1)
                b_growby(str, 1);

        memmove((str->data + location + 1), (str->data + location),
                (str->slen - location + 1));
        str->data[location] = (uchar)ch;
        ++str->slen;

        return 0;
}

/*============================================================================*/
/* Minor helper functions */
/*============================================================================*/


/* A 64 bit integer is at most 19 decimal digits long. That, plus one for the
 * null byte and plus one for a '+' or '-' sign gives a max size of 21. */
#define INT64_MAX_CHARS 21

bstring *
b_ll2str(const long long value)
{
        /* Generate the (reversed) string representation. */
        uchar *rev, *fwd;
        uint64_t inv = (value < 0) ? (-value) : (value);
        bstring *ret = b_alloc_null(INT64_MAX_CHARS + 1);
        rev = fwd = ret->data;

        do {
                *rev++ = (uchar)('0' + (inv % 10));
                inv    = (inv / 10);
        } while (inv);

        if (value < 0)
                *rev++ = (uchar)'-';

        /* Compute length and add null term. */
        *rev--    = (uchar)'\0';
        ret->slen = psub(rev, ret->data) + 1U;

        /* Reverse the string. */
        while (fwd < rev) {
                const uchar swap = *fwd;
                *fwd++           = *rev;
                *rev--           = swap;
        }

        return ret;
}

static unsigned
_tmp_ll2bstr(bstring *bstr, const long long value)
{
        uchar *rev, *fwd;
        unsigned long long inv = (value < 0) ? (-value) : (value);
        rev = fwd = bstr->data;

        do {
                *rev++ = (uchar)('0' + (inv % 10LLU));
                inv    = (inv / 10LLU);
        } while (inv);
        if (value < 0)
                *rev++ = (uchar)'-';

        *rev--     = (uchar)'\0';
        bstr->slen = psub(rev, bstr->data) + 1U;
        while (fwd < rev) {
                const uchar swap = *fwd;
                *fwd++           = *rev;
                *rev--           = swap;
        }

        return bstr->slen;
}

static unsigned
_tmp_ull2bstr(bstring *bstr, const unsigned long long value)
{
        uchar *rev, *fwd;
        unsigned long long inv = value;
        rev = fwd = bstr->data;

        do {
                *rev++ = (uchar)('0' + (inv % 10LLU));
                inv    = (inv / 10LLU);
        } while (inv);

        *rev--     = (uchar)'\0';
        bstr->slen = psub(rev, bstr->data) + 1U;
        while (fwd < rev) {
                const uchar swap = *fwd;
                *fwd++           = *rev;
                *rev--           = swap;
        }

        return bstr->slen;
}

/*============================================================================*/
/* Simple string manipulation.. */
/*============================================================================*/


int
b_chomp(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR();

        if (bstr->slen > 0) {
                if (bstr->data[bstr->slen - 1] == '\n')
                        bstr->data[--bstr->slen] = '\0';
                if (bstr->data[bstr->slen - 1] == '\r')
                        bstr->data[--bstr->slen] = '\0';
        }

        return BSTR_OK;
}

int
b_replace_ch(bstring *bstr, const int find, const int replacement)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR();

        unsigned  len = bstr->slen;
        uint8_t  *dat = bstr->data;
        uint8_t  *ptr = NULL;

        while ((ptr = memchr(dat, find, len))) {
                *ptr = replacement;
                len -= (unsigned)psub(dat, ptr);
                dat  = ptr + 1;
        }

        return BSTR_OK;
}

int
b_catblk_nonul(bstring *bstr, void *blk, const unsigned len)
{
        if (INVALID(bstr) || NO_WRITE(bstr) || !blk || len == 0)
                RUNTIME_ERROR();
        b_alloc(bstr, bstr->slen + 1 + len);
        memcpy(bstr->data + bstr->slen + 1, blk, len);
        return BSTR_OK;
}

bool
b_starts_with(const bstring *b0, const bstring *b1)
{
        if (INVALID(b0) || INVALID(b1))
                RUNTIME_ERROR();
        if (b0->slen < b1->slen)
                return false;
        return b_strncmp(b0, b1, b1->slen) == 0;
}

int
b_strip_leading_ws(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR();
        unsigned i;
        for (i = 0; i < bstr->slen; ++i)
                if (!isspace(bstr->data[i]))
                        break;
        if (i > 0) {
                memmove(bstr->data, bstr->data + i, bstr->slen + 1 - i);
                bstr->slen -= i;
        }
        return 0;
}

int
b_strip_trailing_ws(bstring *bstr)
{
        if (INVALID(bstr) || NO_WRITE(bstr))
                RUNTIME_ERROR();
        if (bstr->slen == 0)
                return 0;
        unsigned i;
        for (i = bstr->slen - 1; i > 0; --i)
                if (!isspace(bstr->data[i]))
                        break;
        if (i++ > 0) {
                bstr->data[i] = '\0';
                bstr->slen = i;
        }
        return 0;
}

/*============================================================================*/
/* Simple printf analogues. */
/*============================================================================*/


bstring *
_b_sprintf(const bstring *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        bstring *ret = _b_vsprintf(fmt, ap);
        va_end(ap);
        return ret;
}

bstring *
_b_vsprintf(const bstring *fmt, va_list args)
{
        static bstring nullstring = bt_init("(null)");
        if (INVALID(fmt))
                RETURN_NULL();

        b_list  *c_strings = NULL;
        unsigned c_str_ctr = 0;

        va_list  cpy;
        int64_t  pos[2048];
        unsigned len  = fmt->slen;
        int64_t  pcnt = 0;
        int64_t  i    = 0;
        memset(pos, 0, sizeof(pos));
        va_copy(cpy, args);

        for (; i < fmt->slen; ++pcnt) {
                int     islong = 0;
                pos[pcnt] = b_strchrp(fmt, '%', i) + 1LL;
                if (pos[pcnt] == 0)
                        break;

                int ch = fmt->data[pos[pcnt]];

        len_restart:
                switch (ch) {
                case 's': {
                        bstring *next = va_arg(cpy, bstring *);
                        if (!next || !next->data)
                                len = (len - 2U) + (sizeof("(null)") - 1U);
                        else
                                len = (len - 2U) + next->slen;
                        i   = pos[pcnt] + 2LL;

                        break;
                }
                case 'n': {
                        if (!c_strings)
                                c_strings = b_list_create();
                        const char *next = va_arg(cpy, const char *);
                        bstring    *tmp;
                        if (next)
                                tmp = b_fromcstr(next);
                        else
                                tmp = b_fromlit("(null)");
                        b_list_append(c_strings, tmp);

                        len = (len - 2U) + tmp->slen;
                        i   = pos[pcnt] + 2;
                        break;
                }
                case 'd':
                case 'u':
                        switch (islong) {
                        case 0:
                                (void)va_arg(cpy, int);
                                len = (len - 2U) + 11U;
                                i   = pos[pcnt] + 2;
                                break;
                        case 1: (void)va_arg(cpy, long);
#if INT_MAX == LONG_MAX
                                len = (len - 3U) + 11U;
#else
                                len = (len - 3U) + 21U;
#endif
                                i   = pos[pcnt] + 3;
                                break;
                        case 2:
                                (void)va_arg(cpy, long long);
                                len = (len - 4U) + 21U;
                                i   = pos[pcnt] + 4;
                                break;
                        case 3:
                                (void)va_arg(cpy, size_t);
                                len = (len - 3U) + 21U;
                                i   = pos[pcnt] + 3;
                                break;
                        default:
                                abort();
                        }

                        break;
                case 'l':
                        ++islong;
                        ch = fmt->data[pos[pcnt] + islong];
                        goto len_restart;
                case 'z':
                        islong = 3;
                        ch = fmt->data[pos[pcnt] + 1];
                        goto len_restart;
                case 'c':
                        (void)va_arg(cpy, int);
                        --len;
                        i = pos[pcnt] + 2;
                        break;
                case '%':
                        --len;
                        i = pos[pcnt] + 2;
                        break;
                default:
                        errx(1, "Value '%c' is not legal.", ch);
                }
        }

        va_end(cpy);
        bstring *ret = b_create(snapUpSize(len + 1U));
        int64_t  x;
        pcnt = i = x = 0;

        for (; ; ++pcnt) {
                const int64_t diff = (pos[pcnt] == 0) ? (int64_t)fmt->slen - x
                                                      : pos[pcnt] - x - 1LL;
                if (diff >= 0)
                        memcpy(ret->data + i, fmt->data + x, diff);
                
                int islong = 0;
                int ch     = fmt->data[pos[pcnt]];
                i += diff;
                x += diff;

                if (pos[pcnt] == 0)
                        break;

        str_restart:
                switch (ch) {
                case 's':
                case 'n': {
                        bstring *next;

                        if (ch == 's') {
                                next = va_arg(args, bstring *);
                                if (!next || !next->data)
                                        next = &nullstring;
                        } else {
                                if (!c_strings)
                                        abort();
                                (void)va_arg(args, const char *);
                                next = c_strings->lst[c_str_ctr++];
                        }

                        memcpy(ret->data + i, next->data, next->slen);
                        i += next->slen;
                        x = pos[pcnt] + 1;
                        break;
                }
                case 'd': {
                        uchar buf[INT64_MAX_CHARS + 1];
                        int n = 0;
                        bstring tmp = {buf, 0, 0, 0};

                        switch (islong) {
                        case 0: {
                                const int next = va_arg(args, int);
                                n = _tmp_ll2bstr(&tmp, (long long)next);
                                x = pos[pcnt] + 1;
                                break;
                        }
                        case 1: {
                                const long next = va_arg(args, long);
                                n = _tmp_ll2bstr(&tmp, (long long)next);
                                x = pos[pcnt] + 2;
                                break;
                        }
                        case 2: {
                                const long long next = va_arg(args, long long);
                                n = _tmp_ll2bstr(&tmp, next);
                                x = pos[pcnt] + 3;
                                break;
                        }
                        case 3: {
                                const ssize_t next = va_arg(args, ssize_t);
                                n = _tmp_ll2bstr(&tmp, next);
                                x = pos[pcnt] + 2;
                                break;
                        }
                        default:
                                abort();
                        }

                        memcpy(ret->data + i, buf, n);
                        i += n;
                        break;
                }
                case 'u': {
                        uchar buf[INT64_MAX_CHARS + 1];
                        int n = 0;
                        bstring *tmp = &(bstring){buf, 0, 0, 0};

                        switch (islong) {
                        case 0: {
                                const unsigned next = va_arg(args, unsigned);
                                n = _tmp_ull2bstr(tmp, (long long unsigned)next);
                                x = pos[pcnt] + 1;
                                break;
                        }
                        case 1: {
                                const long unsigned next = va_arg(args, long unsigned);
                                n = _tmp_ull2bstr(tmp, (long long unsigned)next);
                                x = pos[pcnt] + 2;
                                break;
                        }
                        case 2: {
                                const long long unsigned next = va_arg(args, long long unsigned);
                                n = _tmp_ull2bstr(tmp, next);
                                x = pos[pcnt] + 3;
                                break;
                        }
                        case 3: {
                                const size_t next = va_arg(args, size_t);
                                n = _tmp_ull2bstr(tmp, next);
                                x = pos[pcnt] + 2;
                                break;
                        }
                        default:
                                abort();
                        }

                        memcpy(ret->data + i, buf, n);
                        i += n;
                        break;
                }
                case 'c': {
                        const int next = va_arg(args, int);
                        ret->data[i++] = (uchar)next;
                        x += 2;
                        break;
                }
                case 'l':
                        ++islong;
                        ch = fmt->data[pos[pcnt] + islong];
                        goto str_restart;
                case 'z':
                        islong = 3;
                        ch = fmt->data[pos[pcnt] + 1];
                        goto str_restart;
                case '%':
                        ret->data[i++] = '%';
                        x += 2;
                        break;
                default:
                        errx(1, "Value '%c' is not legal.", ch);
                }
        }

        if (c_strings)
                b_list_destroy(c_strings);

        ret->data[(ret->slen = i)] = '\0';
        return ret;
}

int
_b_fprintf(FILE *out_fp, const bstring *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        const int ret = _b_vfprintf(out_fp, fmt, ap);
        va_end(ap);

        return ret;
}

int
_b_vfprintf(FILE *out_fp, const bstring *fmt, va_list args)
{
        if (INVALID(fmt) || !out_fp)
                RUNTIME_ERROR();

        bstring *toprint = _b_vsprintf(fmt, args);
        if (!toprint)
                RUNTIME_ERROR();

        const int ret = fwrite(toprint->data, 1, toprint->slen, out_fp);
        b_free(toprint);
        return ret;
}

int
_b_dprintf(const int out_fd, const bstring *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        const int ret = _b_vdprintf(out_fd, fmt, ap);
        va_end(ap);

        return ret;
}

int
_b_vdprintf(const int out_fd, const bstring *fmt, va_list args)
{
        if (INVALID(fmt) || out_fd < 0)
                RUNTIME_ERROR();

        bstring *toprint = _b_vsprintf(fmt, args);
        if (!toprint)
                RUNTIME_ERROR();

        const int ret = write(out_fd, toprint->data, toprint->slen);
        b_free(toprint);
        return ret;
}

int
_b_sprintfa(bstring *dest, const bstring *fmt, ...)
{
        if (INVALID(dest) || NO_WRITE(dest) || INVALID(fmt))
                RUNTIME_ERROR();
        va_list ap;
        va_start(ap, fmt);
        const int ret = _b_vsprintfa(dest, fmt, ap);
        va_end(ap);

        return ret;
}

int
_b_vsprintfa(bstring *dest, const bstring *fmt, va_list args)
{
        if (INVALID(dest) || NO_WRITE(dest) || INVALID(fmt))
                RUNTIME_ERROR();

        bstring *app = _b_vsprintf(fmt, args);
        if (INVALID(app))
                RUNTIME_ERROR();

        const unsigned newlen = snapUpSize(dest->slen + app->slen + 1U);

        if (dest->mlen >= newlen) {
                memcpy(dest->data + dest->slen, app->data, app->slen);
        } else {
                uchar *buf = talloc_size(dest, newlen);
                memcpy(buf, dest->data, dest->slen);
                memcpy(buf + dest->slen, app->data, app->slen);
                talloc_free(dest->data);
                dest->data = buf;
        }

        dest->slen += app->slen;
        dest->data[dest->slen] = '\0';
        dest->mlen = newlen;
        b_free(app);

        return BSTR_OK;
}
