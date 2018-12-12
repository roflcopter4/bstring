/*
 * Copyright 2002-2010 Paul Hsieh
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

#include "private.h"
#include <assert.h>
#include <inttypes.h>
#include <talloc.h>

#include "bstring.h"

/* 
 * This file was broken off from the main bstrlib.c file in a forlorn effort to
 * keep things a little neater.
 */

/*============================================================================*/
/* General operations */
/*============================================================================*/

b_list *
b_list_create(void)
{
        b_list *sl = talloc(NULL, b_list);
        sl->lst    = talloc_zero_array(sl, bstring *, 4);
        sl->qty    = 0;
        sl->mlen   = 1;
        if (!sl->lst)
                FATAL_ERROR("calloc failed");

        return sl;
}

b_list *
b_list_create_alloc(const uint msz)
{
        const int safesize = (msz == 0) ? 1 : msz;

        b_list *sl = talloc(NULL, b_list);
        sl->lst    = talloc_zero_array(sl, bstring *, safesize);
        sl->qty    = 0;
        sl->mlen   = safesize;
        sl->lst[0] = NULL;

        return sl;
}

int
b_list_destroy(b_list *sl)
{
        if (!sl)
                return BSTR_ERR;
        for (uint i = 0; i < sl->qty; ++i)
                if (sl->lst[i])
                        b_destroy(sl->lst[i]);

        sl->qty  = 0;
        sl->mlen = 0;
        talloc_free(sl->lst);
        sl->lst  = NULL;
        talloc_free(sl);

        return BSTR_OK;
}

int
b_list_alloc(b_list *sl, const uint msz)
{
        bstring **blen;
        uint smsz;
        if (!sl || msz == 0 || !sl->lst || sl->mlen == 0 || sl->qty > sl->mlen)
                RUNTIME_ERROR();
        if (sl->mlen >= msz)
                return BSTR_OK;

        smsz = snapUpSize(msz);

        blen = talloc_realloc(NULL, sl->lst, bstring *, smsz);
#if 0
        blen = realloc(sl->lst, nsz);
        if (!blen) {
                smsz = msz;
                nsz = ((size_t)smsz) * sizeof(bstring *);
                blen = realloc(sl->lst, nsz);
                if (!blen)
                        ALLOCATION_ERROR(BSTR_ERR);
        }
#endif

        sl->mlen = smsz;
        sl->lst  = blen;
        return BSTR_OK;
}

int
b_list_allocmin(b_list *sl, uint msz)
{
        bstring **blen;
        size_t nsz;

        if (!sl || msz == 0 || !sl->lst || sl->mlen == 0 || sl->qty > sl->mlen)
                RUNTIME_ERROR();
        if (msz < sl->qty)
                msz = sl->qty;
        if (sl->mlen == msz)
                return BSTR_OK;

        nsz = ((size_t)msz) * sizeof(bstring *);

        if (nsz < (size_t)msz)
                RUNTIME_ERROR();

        blen = talloc_realloc_size(NULL, sl->lst, nsz);
        if (!blen)
                RUNTIME_ERROR();

        sl->mlen = msz;
        sl->lst  = blen;

        return BSTR_OK;
}

/*============================================================================*/

bstring *
b_join(const b_list *bl, const bstring *sep)
{
        if (!bl || INVALID(sep))
                RETURN_NULL();
        int64_t total = 1;

        for (uint i = 0; i < bl->qty; ++i) {
                const uint v = bl->lst[i]->slen;
                total += v;
                if (total > UINT32_MAX)
                        RETURN_NULL();
        }

        if (sep)
                total += (bl->qty - 1) * sep->slen;

        bstring *bstr = talloc(NULL, bstring);

        bstr->mlen  = total;
        bstr->slen  = total - 1;
        bstr->data  = talloc_size(bstr, total);
        bstr->flags = BSTR_STANDARD;
        total       = 0;
        talloc_set_destructor(bstr, b_free);

        for (uint i = 0; i < bl->qty; ++i) {
                if (i > 0 && sep) {
                        memcpy(bstr->data + total, sep->data, sep->slen);
                        total += sep->slen;
                }
                const uint v = bl->lst[i]->slen;
                memcpy(bstr->data + total, bl->lst[i]->data, v);
                total += v;
        }
        bstr->data[total] = (uchar)'\0';

        return bstr;
}


bstring *
b_join_quote(const b_list *bl, const bstring *sep, const int ch)
{
        if (!bl || INVALID(sep) || !ch)
                RETURN_NULL();
        const uint sepsize = (sep) ? sep->slen : 0;
        int64_t    total   = 1;

        B_LIST_FOREACH(bl, bstr, i)
                total += bstr->slen + sepsize + 2u;
        if (total > UINT32_MAX)
                RETURN_NULL();

        bstring *bstr = talloc(NULL, bstring);
        bstr->mlen    = total - (2 * sepsize);
        bstr->slen    = 0;
        bstr->data    = talloc_size(bstr, total);
        bstr->flags   = BSTR_STANDARD;
        talloc_set_destructor(bstr, b_free);

        B_LIST_FOREACH(bl, cur, i) {
                if (sep && i > 0) {
                        memcpy(bstr->data + bstr->slen, sep->data, sep->slen);
                        bstr->slen += sep->slen;
                }

                bstr->data[bstr->slen++] = (uchar)ch;
                memcpy((bstr->data + bstr->slen), cur->data, cur->slen);
                bstr->slen += cur->slen;
                bstr->data[bstr->slen++] = (uchar)ch;
        }

        bstr->data[bstr->slen] = (uchar)'\0';

        if (bstr->slen != bstr->mlen)
                warnx("Failed operation, %u is smaller than %"PRId64"!", bstr->slen, total);

        return bstr;
}

/*============================================================================*/

void
_b_list_dump(FILE *fp, const b_list *list, const char *listname)
{
        fprintf(fp, "Dumping list \"%s\"\n", listname);
        for (unsigned i = 0; i < list->qty; ++i)
                b_fwrite(fp, list->lst[i], b_tmp("\n"));
        fputc('\n', fp);
}


void
_b_list_dump_fd(const int fd, const b_list *list, const char *listname)
{
        dprintf(fd, "Dumping list \"%s\"\n", listname);
        for (unsigned i = 0; i < list->qty; ++i) {
                if (!list->lst[i] || list->lst[i]->slen == 0)
                        b_write(fd, B("(NULL)\n"));
                else
                        b_write(fd, list->lst[i], b_tmp("\n"));
        }
        if (write(fd, "\n", 1) != 1)
                warn("write error");
}


int
b_list_append(b_list *list, bstring *bstr)
{
        if (!list || !list->lst)
                RUNTIME_ERROR();

        if (list->qty >= (list->mlen)) {
                bstring **tmp = talloc_realloc(NULL, list->lst, bstring *, list->mlen *= 2);
                list->lst = tmp;
        }
        list->lst[list->qty++] = bstr;
        talloc_steal(list, bstr);

        return BSTR_OK;
}

int
b_list_remove(b_list *list, const unsigned index)
{
        if (!list || !list->lst || index >= list->qty)
                RUNTIME_ERROR();

        b_destroy(list->lst[index]);
        list->lst[index] = NULL;

        memmove(list->lst + index, list->lst + index + 1, --list->qty - index);
        return BSTR_OK;
}

b_list *
b_list_copy(const b_list *list)
{
        if (!list || !list->lst)
                RETURN_NULL();

        b_list *ret = b_list_create_alloc(list->qty);

        for (unsigned i = 0; i < list->qty; ++i) {
                ret->lst[ret->qty] = b_strcpy(list->lst[i]);
                b_writeallow(ret->lst[ret->qty]);
                ++ret->qty;
        }

        return ret;
}

b_list *
b_list_clone(const b_list *const list)
{
        if (!list || !list->lst)
                RETURN_NULL();

        b_list *ret = b_list_create_alloc(list->qty);

        for (unsigned i = 0; i < list->qty; ++i) {
                ret->lst[ret->qty] = b_clone(list->lst[i]);
                ++ret->qty;
        }

        return ret;
}

b_list *
b_list_clone_swap(b_list *list)
{
        if (!list || !list->lst)
                RETURN_NULL();

        b_list *ret = b_list_create_alloc(list->qty);

        for (unsigned i = 0; i < list->qty; ++i) {
                ret->lst[ret->qty] = b_clone_swap(list->lst[i]);
                ++ret->qty;
        }

        return ret;
}

int
b_list_writeprotect(b_list *list)
{
        if (!list || !list->lst)
                RUNTIME_ERROR();

        B_LIST_FOREACH(list, bstr, i)
                if (!INVALID(bstr))
                        b_writeprotect(bstr);

        return BSTR_OK;
}

int
b_list_writeallow(b_list *list)
{
        if (!list || !list->lst)
                RUNTIME_ERROR();

        B_LIST_FOREACH(list, bstr, i)
                if (!INVALID(bstr))
                        b_writeallow(bstr);

        return BSTR_OK;
}

int
b_list_merge(b_list **dest, b_list *src, const int flags)
{
        if (!dest || !*dest || !(*dest)->lst)
                RUNTIME_ERROR();
        if (!src || !src->lst)
                RUNTIME_ERROR();
        if (src->qty == 0)
                return BSTR_ERR;

        const unsigned size = ((*dest)->qty + src->qty);
        if ((*dest)->mlen < size)
                (*dest)->lst = talloc_realloc(NULL, (*dest)->lst, bstring *, (size_t)((*dest)->mlen = size));

        for (unsigned i = 0; i < src->qty; ++i)
                (*dest)->lst[(*dest)->qty++] = src->lst[i];

        if (flags & BSTR_M_DEL_SRC) {
                talloc_free(src->lst);
                talloc_free(src);
        }
        if (flags & BSTR_M_DEL_DUPS)
                b_list_remove_dups(dest);
        else if (flags & BSTR_M_SORT_FAST)
                B_LIST_SORT_FAST(*dest);
        if (!(flags & BSTR_M_SORT_FAST) && (flags & BSTR_M_SORT))
                B_LIST_SORT(*dest);

        return BSTR_OK;
}

int
b_list_remove_dups(b_list **listp)
{
        if (!listp || !*listp || !(*listp)->lst || (*listp)->qty == 0)
                RUNTIME_ERROR();

        b_list *toks = b_list_create_alloc((*listp)->qty * 10);

        for (unsigned i = 0; i < (*listp)->qty; ++i) {
                b_list *tmp = b_strsep((*listp)->lst[i], " ", 0);
                if (!tmp)
                        continue;
                for (unsigned x = 0; x < tmp->qty; ++x)
                        b_list_append(toks, tmp->lst[x]);
                talloc_free(tmp->lst);
                talloc_free(tmp);
        }

        b_list_destroy(*listp);

        qsort(toks->lst, toks->qty, sizeof(bstring *), &b_strcmp_fast_wrap);

        b_list *uniq = b_list_create_alloc(toks->qty);
        uniq->lst[0] = toks->lst[0];
        uniq->qty    = 1;
        b_writeprotect(uniq->lst[0]);

        for (unsigned i = 1; i < toks->qty; ++i) {
                if (!b_iseq(toks->lst[i], toks->lst[i-1])) {
                        uniq->lst[uniq->qty] = toks->lst[i];
                        b_writeprotect(uniq->lst[uniq->qty]);
                        ++uniq->qty;
                }
        }

        b_list_destroy(toks);
        for (unsigned i = 0; i < uniq->qty; ++i)
                b_writeallow(uniq->lst[i]);

        *listp = uniq;
        return BSTR_OK;
}

bstring *
b_list_join(const b_list *list, const bstring *sep)
{
        if (!list || !list->lst || list->qty == 0)
                RETURN_NULL();
        unsigned total = 1;
        unsigned seplen = (sep != NULL) ? sep->slen : 0;

        B_LIST_FOREACH (list, str, i)
                total += str->slen + seplen;
        
        bstring *ret = b_alloc_null(total);

        B_LIST_FOREACH (list, str, i) {
                memcpy(ret->data + ret->slen, str->data, str->slen);
                ret->slen += str->slen;
                if (sep != NULL) {
                        memcpy(ret->data + ret->slen, sep->data, sep->slen);
                        ret->slen += sep->slen;
                }
        }

        ret->data[ret->slen] = '\0';
        return ret;
}
