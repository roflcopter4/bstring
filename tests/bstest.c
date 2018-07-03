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
 * This file is the C unit test for Bstrlib.
 */

/* #include "bstraux.h" */
/* #include "bstrlib.h" */

#include <bstring/bstraux.h>
#include <bstring/bstrlib.h>

#include <check.h>
#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void
test0_0(const char *s, const char *res)
{
        int ret = 0;
        bstring *b0 = b_fromcstr(s);
        if (s) {
                ck_assert_ptr_nonnull(b0);
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b0->slen, ret);
                ret = memcmp(res, b0->data, b0->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b0->data[b0->slen], '\0');
                ret = b_destroy(b0);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ck_assert_ptr_null(res);
        }
}

static void
test0_1(const char *s, int len, const char *res)
{
        int ret = 0;
        bstring *b0 = b_fromcstr_alloc(len, s);
        if (s) {
                ck_assert_ptr_nonnull(b0);
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b0->slen, ret);
                ret = memcmp(res, b0->data, b0->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b0->data[b0->slen], '\0');
                ck_assert(b0->mlen >= len);
                ret = b_destroy(b0);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ck_assert_ptr_null(res);
        }
}

#define EMPTY_STRING ""

#define SHORT_STRING "bogus"

#define LONG_STRING                                     \
        "This is a bogus but reasonably long string.  " \
        "Just long enough to cause some mallocing."

START_TEST(core_000)
{
        /* tests with NULL */
        test0_0(NULL, NULL);
        /* normal operation tests */
        test0_0(EMPTY_STRING, EMPTY_STRING);
        test0_0(SHORT_STRING, SHORT_STRING);
        test0_0(LONG_STRING, LONG_STRING);
        /* tests with NULL */
        test0_1(NULL, 0, NULL);
        test0_1(NULL, 30, NULL);
        /* normal operation tests */
        test0_1(EMPTY_STRING, 0, EMPTY_STRING);
        test0_1(EMPTY_STRING, 30, EMPTY_STRING);
        test0_1(SHORT_STRING, 0, SHORT_STRING);
        test0_1(SHORT_STRING, 30, SHORT_STRING);
        test0_1(LONG_STRING, 0, LONG_STRING);
        test0_1(LONG_STRING, 30, LONG_STRING);
}
END_TEST

static void
test1_0(const void *blk, int len, const char *res)
{
        int ret = 0;
        bstring *b0 = b_blk2bstr(blk, len);
        if (res) {
                ck_assert_ptr_nonnull(b0);
                ck_assert_int_eq(b0->slen, len);
                ret = memcmp(res, b0->data, len);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b0->data[b0->slen], '\0');
                ret = b_destroy(b0);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ck_assert_ptr_null(b0);
        }
}

START_TEST(core_001)
{
        /* tests with NULL */
        test1_0(NULL, 10, NULL);
        test1_0(NULL, 0, NULL);
        test1_0(NULL, -1, NULL);
        /* normal operation tests */
        test1_0(SHORT_STRING, sizeof(SHORT_STRING) - 1, SHORT_STRING);
        test1_0(LONG_STRING, sizeof(LONG_STRING) - 1, LONG_STRING);
        test1_0(LONG_STRING, 5, "This ");
        test1_0(LONG_STRING, 0, "");
        test1_0(LONG_STRING, -1, NULL);
}
END_TEST

static void
test2_0(const bstring *b, char z, const unsigned char *res)
{
        int ret = 0;
        char *s = b_bstr2cstr(b, z);
        if (res) {
                ck_assert_ptr_nonnull(s);
                ret = strlen(s);
                ck_assert_int_eq(b->slen, ret);
                ret = memcmp(res, b->data, b->slen);
                ck_assert_int_eq(ret, 0);
        } else {
                /* warnx("s is %s", (s ? s : "null")); */
                ck_assert_ptr_null(s);
        }
        free(s);
}

/* test input used below */
bstring emptyBstring = bt_init(EMPTY_STRING);

bstring shortBstring = bt_init(SHORT_STRING);

bstring longBstring = bt_init(LONG_STRING);

bstring badBstring1 = {.mlen = 8, .slen = 4, .data = (unsigned char *)NULL};

bstring badBstring2 = {.mlen = 2, .slen = -5, .data = (unsigned char *)SHORT_STRING};

bstring badBstring3 = {.mlen = 2, .slen = 5, .data = (unsigned char *)SHORT_STRING};

bstring xxxxxBstring = bt_init("xxxxx");

START_TEST(core_002)
{
        /* tests with NULL */
        test2_0(NULL, '?', NULL);
        /* normal operation tests */
        test2_0(&emptyBstring, '?', emptyBstring.data);
        test2_0(&shortBstring, '?', shortBstring.data);
        test2_0(&longBstring, '?', longBstring.data);
        test2_0(&badBstring1, '?', NULL);
        test2_0(&badBstring2, '?', NULL);
}
END_TEST

static void
test3_0(const bstring *b)
{
        int ret = 0;
        bstring *b0 = b_strcpy(b);
        if (!b || !b->data || b->slen < 0) {
                ck_assert_ptr_null(b0);
        } else {
                ck_assert_ptr_nonnull(b0);
                ck_assert_int_eq(b0->slen, b->slen);
                ret = memcmp(b->data, b0->data, b->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b0->data[b0->slen], '\0');
                ret = b_destroy(b0);
                ck_assert_int_eq(ret, BSTR_OK);
        }
}

START_TEST(core_003)
{
        /* tests with NULL to make sure that there is NULL propogation */
        test3_0(NULL);
        test3_0(&badBstring1);
        test3_0(&badBstring2);
        /* normal operation tests */
        test3_0(&emptyBstring);
        test3_0(&shortBstring);
        test3_0(&longBstring);
}
END_TEST

static void
test4_0(const bstring *b, int left, int len, const char *res)
{
        static unsigned short int x = 0;
        warnx("Count test 4_0 - %u", x++);
        int ret = 0;
        bstring *b0 = b_midstr(b, left, len);
        if (!b0) {
                ck_assert(!b || !b->data || b->slen < 0 || len < 0);
        } else {
                warnx("b0 is %p", (void*)b0);
                ck_assert_ptr_nonnull(b);
                ck_assert_ptr_nonnull(res);
                if (len >= 0) {
                        ck_assert(b0->slen <= len);
                }
                ret = strlen(res);
                ck_assert_int_eq(b0->slen, ret);
                ret = memcmp(res, b0->data, b0->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b0->data[b0->slen], '\0');
                ret = b_destroy(b0);
                ck_assert_int_eq(ret, BSTR_OK);
        }
}

START_TEST(core_004)
{
        /* tests with NULL to make sure that there is NULL propogation */
        test4_0(NULL, 0, 0, NULL);
        test4_0(NULL, 0, 2, NULL);
        test4_0(NULL, 0, -2, NULL);
        test4_0(NULL, -5, 2, NULL);
        test4_0(NULL, -5, -2, NULL);
        test4_0(&badBstring1, 1, 3, NULL);
        test4_0(&badBstring2, 1, 3, NULL);

        /* normal operation tests on all sorts of subranges */
        test4_0(&emptyBstring, 0, 0, "");
        test4_0(&emptyBstring, 0, -1, "");
        test4_0(&emptyBstring, 1, 3, "");
        test4_0(&shortBstring, 0, 0, "");
        test4_0(&shortBstring, 0, -1, "");
        test4_0(&shortBstring, 1, 3, "ogu");
        test4_0(&shortBstring, -1, 3, "bo");
        test4_0(&shortBstring, -1, 9, "bogus");
        test4_0(&shortBstring, 3, -1, "");
        test4_0(&shortBstring, 9, 3, "");
}
END_TEST

static void
test5_0(bstring *b0, const bstring *b1, const char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0 && b1 && b1->data &&
            b1->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_concat(b2, b1);
                ck_assert_int_ne(ret, 0);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_concat(b2, b1);
                ck_assert_int_eq(b2->slen, b0->slen + b1->slen);
                ck_assert_ptr_nonnull(res);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_concat(b0, b1);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

static void
test5_1(void)
{
        bstring *b, *c;
        bstring t;
        int i, ret;
        for (i = 0; i < longBstring.slen; i++) {
                b = b_strcpy(&longBstring);
                ck_assert_ptr_nonnull(b);
                c = b_strcpy(&longBstring);
                ck_assert_ptr_nonnull(c);
                bmid2tbstr(t, b, i, longBstring.slen);
                ret = b_concat(c, &t);
                ck_assert_int_eq(ret, 0);
                ret = b_concat(b, &t);
                ck_assert_int_eq(ret, 0);
                ret = b_iseq(b, c);
                ck_assert_int_eq(ret, 1);
                ret = b_destroy(b);
                ck_assert_int_eq(ret, BSTR_OK);
                ret = b_destroy(c);
                ck_assert_int_eq(ret, BSTR_OK);
        }
        b = b_fromcstr("abcd");
        ck_assert_ptr_nonnull(b);
        c = b_fromcstr("abcd");
        ck_assert_ptr_nonnull(c);
        for (i = 0; i < 100; i++) {
                bmid2tbstr(t, b, 0, 3);
                ret = b_catcstr(c, "abc");
                ck_assert_int_eq(ret, 0);
                ret = b_concat(b, &t);
                ck_assert_int_eq(ret, 0);
                ret = b_iseq(b, c);
                ck_assert_int_eq(ret, 1);
        }
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
}

START_TEST(core_005)
{
        /* tests with NULL */
        test5_0(NULL, NULL, NULL);
        test5_0(NULL, &emptyBstring, NULL);
        test5_0(&emptyBstring, NULL, "");
        test5_0(&emptyBstring, &badBstring1, NULL);
        test5_0(&emptyBstring, &badBstring2, NULL);
        test5_0(&badBstring1, &emptyBstring, NULL);
        test5_0(&badBstring2, &emptyBstring, NULL);
        /* normal operation tests on all sorts of subranges */
        test5_0(&emptyBstring, &emptyBstring, "");
        test5_0(&emptyBstring, &shortBstring, "bogus");
        test5_0(&shortBstring, &emptyBstring, "bogus");
        test5_0(&shortBstring, &shortBstring, "bogusbogus");
        test5_1();
}
END_TEST

static void
test6_0(bstring *b, char c, const char *res)
{
        bstring *b0;
        int ret = 0;
        if (b && b->data && b->slen >= 0) {
                b0 = b_strcpy(b);
                ck_assert_ptr_nonnull(b0);
                b_writeprotect(*b0);
                ret = b_conchar(b0, c);
                ck_assert_int_ne(ret, 0);
                ret = b_iseq(b0, b);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b0);
                ret = b_conchar(b0, c);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b0->slen, b->slen + 1);
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b0->slen, ret);
                ret = memcmp(b0->data, res, b0->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b0->data[b0->slen], '\0');
                ret = b_destroy(b0);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_conchar(b, c);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_006)
{
        /* tests with NULL */
        test6_0(NULL, 'x', NULL);
        test6_0(&badBstring1, 'x', NULL);
        test6_0(&badBstring2, 'x', NULL);
        /* normal operation tests on all sorts of subranges */
        test6_0(&emptyBstring, 'x', "x");
        test6_0(&shortBstring, 'x', "bogusx");
}
END_TEST

static void
test7x8_0(int (*fnptr)(const bstring *, const bstring *), const bstring *b0,
          const bstring *b1, int res)
{
        int ret = 0;
        ret = fnptr(b0, b1);
        ck_assert_int_eq(ret, res);
}

static void
test7x8(int (*fnptr)(const bstring *, const bstring *), int retFail, int retLT,
        int retGT, int retEQ)
{
        /* tests with NULL */
        test7x8_0(fnptr, NULL, NULL, retFail);
        test7x8_0(fnptr, &emptyBstring, NULL, retFail);
        test7x8_0(fnptr, NULL, &emptyBstring, retFail);
        test7x8_0(fnptr, &shortBstring, NULL, retFail);
        test7x8_0(fnptr, NULL, &shortBstring, retFail);
        test7x8_0(fnptr, &badBstring1, &badBstring1, retFail);
        test7x8_0(fnptr, &badBstring2, &badBstring2, retFail);
        test7x8_0(fnptr, &shortBstring, &badBstring2, retFail);
        test7x8_0(fnptr, &badBstring2, &shortBstring, retFail);
        /* normal operation tests on all sorts of subranges */
        test7x8_0(fnptr, &emptyBstring, &emptyBstring, retEQ);
        test7x8_0(fnptr, &shortBstring, &emptyBstring, retGT);
        test7x8_0(fnptr, &emptyBstring, &shortBstring, retLT);
        test7x8_0(fnptr, &shortBstring, &shortBstring, retEQ);
        bstring *b = b_strcpy(&shortBstring);
        ck_assert_ptr_nonnull(b);
        b->data[1]++;
        test7x8_0(fnptr, b, &shortBstring, retGT);
        int ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        if (fnptr == b_iseq) {
                test7x8_0(fnptr, &shortBstring, &longBstring, retGT);
                test7x8_0(fnptr, &longBstring, &shortBstring, retLT);
        } else {
                test7x8_0(fnptr, &shortBstring, &longBstring, 'b' - 'T');
                test7x8_0(fnptr, &longBstring, &shortBstring, 'T' - 'b');
        }
}

START_TEST(core_007)
{
        test7x8(b_iseq, -1, 0, 0, 1);
}
END_TEST

START_TEST(core_008)
{
        test7x8(b_strcmp, SHRT_MIN, -1, 1, 0);
}
END_TEST

static void
test9_0(const bstring *b0, const bstring *b1, int n, int res)
{
        static int count = 0;
        warnx("This is count %d", count++);
        int ret = 0;
        ret = b_strncmp(b0, b1, n);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_009)
{
        /* tests with NULL */
        test9_0(NULL, NULL, 0, SHRT_MIN);
        test9_0(NULL, NULL, -1, SHRT_MIN);
        test9_0(NULL, NULL, 1, SHRT_MIN);
        test9_0(&emptyBstring, NULL, 0, SHRT_MIN);
        test9_0(NULL, &emptyBstring, 0, SHRT_MIN);
        test9_0(&emptyBstring, NULL, 1, SHRT_MIN);
        test9_0(NULL, &emptyBstring, 1, SHRT_MIN);
        test9_0(&badBstring1, &badBstring1, 1, SHRT_MIN);
        test9_0(&badBstring2, &badBstring2, 1, SHRT_MIN);
        test9_0(&emptyBstring, &badBstring1, 1, SHRT_MIN);
        test9_0(&emptyBstring, &badBstring2, 1, SHRT_MIN);
        test9_0(&badBstring1, &emptyBstring, 1, SHRT_MIN);
        test9_0(&badBstring2, &emptyBstring, 1, SHRT_MIN);
        /* normal operation tests on all sorts of subranges */
        test9_0(&emptyBstring, &emptyBstring, -1, 0);
        test9_0(&emptyBstring, &emptyBstring, 0, 0);
        test9_0(&emptyBstring, &emptyBstring, 1, 0);
        test9_0(&shortBstring, &shortBstring, -1, 0);
        test9_0(&shortBstring, &shortBstring, 0, 0);
        test9_0(&shortBstring, &shortBstring, 1, 0);
        test9_0(&shortBstring, &shortBstring, 9, 0);
}
END_TEST

static void
test10_0(bstring *b, int res, int nochange)
{
        bstring sb = bt_init("<NULL>");
        int x, ret = 0;
        if (b) {
                sb = *b;
        }
        ret = b_destroy(b);
        if (b) {
                if (ret >= 0) {
                        /* If the b_destroy was successful we have to assume
                         * the contents were "changed"
                         */
                        x = 1;
                } else {
                        x = memcmp(&sb, b, sizeof sb);
                }
        } else {
                x = !nochange;
        }
        ck_assert_int_eq(ret, res);
        ck_assert_int_ne(!nochange, !x);
}

START_TEST(core_010)
{
        bstring *c = b_strcpy(&shortBstring);
        bstring *b = b_strcpy(&emptyBstring);
        ck_assert_ptr_nonnull(c);
        ck_assert_ptr_nonnull(b);
        /* tests with NULL */
        test10_0(NULL, BSTR_ERR, 1);
        /* protected, constant and regular instantiations on empty or not */
        b_writeprotect(*b);
        b_writeprotect(*c);
        test10_0(b, BSTR_ERR, 1);
        test10_0(c, BSTR_ERR, 1);
        b_writeallow(*b);
        b_writeallow(*c);
        test10_0(b, BSTR_OK, 0);
        test10_0(c, BSTR_OK, 0);
        test10_0(&emptyBstring, BSTR_ERR, 1);
        b_writeallow(emptyBstring);
        test10_0(&emptyBstring, BSTR_ERR, 1);
        test10_0(&shortBstring, BSTR_ERR, 1);
        b_writeallow(emptyBstring);
        test10_0(&shortBstring, BSTR_ERR, 1);
        test10_0(&badBstring1, BSTR_ERR, 1);
        test10_0(&badBstring2, BSTR_ERR, 1);
}
END_TEST

static void
test11_0(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_instr(s1, pos, s2);
        ck_assert_int_eq(ret, res);
}

static void
test11_1(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_instr_caseless(s1, pos, s2);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_011)
{
        int ret;
        bstring *b, *c;
        test11_0(NULL, 0, NULL, BSTR_ERR);
        test11_0(&emptyBstring, 0, NULL, BSTR_ERR);
        test11_0(NULL, 0, &emptyBstring, BSTR_ERR);
        test11_0(&emptyBstring, 0, &badBstring1, BSTR_ERR);
        test11_0(&emptyBstring, 0, &badBstring2, BSTR_ERR);
        test11_0(&badBstring1, 0, &emptyBstring, BSTR_ERR);
        test11_0(&badBstring2, 0, &emptyBstring, BSTR_ERR);
        test11_0(&badBstring1, 0, &badBstring2, BSTR_ERR);
        test11_0(&badBstring2, 0, &badBstring1, BSTR_ERR);
        test11_0(&emptyBstring, 0, &emptyBstring, 0);
        test11_0(&emptyBstring, 1, &emptyBstring, BSTR_ERR);
        test11_0(&shortBstring, 1, &shortBstring, BSTR_ERR);
        test11_0(&shortBstring, 5, &emptyBstring, 5);
        test11_0(&shortBstring, -1, &shortBstring, BSTR_ERR);
        test11_0(&shortBstring, 0, &shortBstring, 0);
        test11_0(&shortBstring, 0, b = b_strcpy(&shortBstring), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(&shortBstring, 0, b = b_fromcstr("BOGUS"), BSTR_ERR);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(&longBstring, 0, &shortBstring, 10);
        test11_0(&longBstring, 20, &shortBstring, BSTR_ERR);
        test11_0(c = b_fromcstr("sssssssssap"), 0, b = b_fromcstr("sap"), 8);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("sssssssssap"), 3, b = b_fromcstr("sap"), 8);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("ssssssssssap"), 3, b = b_fromcstr("sap"), 9);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("sssssssssap"), 0, b = b_fromcstr("s"), 0);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("sssssssssap"), 3, b = b_fromcstr("s"), 3);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("sssssssssap"), 0, b = b_fromcstr("a"), 9);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("sssssssssap"), 5, b = b_fromcstr("a"), 9);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("sasasasasap"), 0, b = b_fromcstr("sap"), 8);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_0(c = b_fromcstr("ssasasasasap"), 0, b = b_fromcstr("sap"), 9);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_1(NULL, 0, NULL, BSTR_ERR);
        test11_1(&emptyBstring, 0, NULL, BSTR_ERR);
        test11_1(NULL, 0, &emptyBstring, BSTR_ERR);
        test11_1(&emptyBstring, 0, &badBstring1, BSTR_ERR);
        test11_1(&emptyBstring, 0, &badBstring2, BSTR_ERR);
        test11_1(&badBstring1, 0, &emptyBstring, BSTR_ERR);
        test11_1(&badBstring2, 0, &emptyBstring, BSTR_ERR);
        test11_1(&badBstring1, 0, &badBstring2, BSTR_ERR);
        test11_1(&badBstring2, 0, &badBstring1, BSTR_ERR);
        test11_1(&emptyBstring, 0, &emptyBstring, 0);
        test11_1(&emptyBstring, 1, &emptyBstring, BSTR_ERR);
        test11_1(&shortBstring, 1, &shortBstring, BSTR_ERR);
        test11_1(&shortBstring, 5, &emptyBstring, 5);
        test11_1(&shortBstring, -1, &shortBstring, BSTR_ERR);
        test11_1(&shortBstring, 0, &shortBstring, 0);
        test11_1(&shortBstring, 0, b = b_strcpy(&shortBstring), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_1(&shortBstring, 0, b = b_fromcstr("BOGUS"), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test11_1(&longBstring, 0, &shortBstring, 10);
        test11_1(&longBstring, 20, &shortBstring, BSTR_ERR);
}
END_TEST

static void
test12_0(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_instrr(s1, pos, s2);
        ck_assert_int_eq(ret, res);
}

static void
test12_1(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_instrr_caseless(s1, pos, s2);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_012)
{
        bstring *b;
        int ret = 0;
        test12_0(NULL, 0, NULL, BSTR_ERR);
        test12_0(&emptyBstring, 0, NULL, BSTR_ERR);
        test12_0(NULL, 0, &emptyBstring, BSTR_ERR);
        test12_0(&emptyBstring, 0, &badBstring1, BSTR_ERR);
        test12_0(&emptyBstring, 0, &badBstring2, BSTR_ERR);
        test12_0(&badBstring1, 0, &emptyBstring, BSTR_ERR);
        test12_0(&badBstring2, 0, &emptyBstring, BSTR_ERR);
        test12_0(&badBstring1, 0, &badBstring2, BSTR_ERR);
        test12_0(&badBstring2, 0, &badBstring1, BSTR_ERR);
        test12_0(&emptyBstring, 0, &emptyBstring, 0);
        test12_0(&emptyBstring, 1, &emptyBstring, BSTR_ERR);
        test12_0(&shortBstring, 1, &shortBstring, 0);
        test12_0(&shortBstring, 5, &emptyBstring, 5);
        test12_0(&shortBstring, -1, &shortBstring, BSTR_ERR);
        test12_0(&shortBstring, 0, &shortBstring, 0);
        test12_0(&shortBstring, 0, b = b_strcpy(&shortBstring), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test12_0(&shortBstring, 0, b = b_fromcstr("BOGUS"), BSTR_ERR);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test12_0(&longBstring, 0, &shortBstring, BSTR_ERR);
        test12_0(&longBstring, 20, &shortBstring, 10);
        test12_1(NULL, 0, NULL, BSTR_ERR);
        test12_1(&emptyBstring, 0, NULL, BSTR_ERR);
        test12_1(NULL, 0, &emptyBstring, BSTR_ERR);
        test12_1(&emptyBstring, 0, &badBstring1, BSTR_ERR);
        test12_1(&emptyBstring, 0, &badBstring2, BSTR_ERR);
        test12_1(&badBstring1, 0, &emptyBstring, BSTR_ERR);
        test12_1(&badBstring2, 0, &emptyBstring, BSTR_ERR);
        test12_1(&badBstring1, 0, &badBstring2, BSTR_ERR);
        test12_1(&badBstring2, 0, &badBstring1, BSTR_ERR);
        test12_1(&emptyBstring, 0, &emptyBstring, 0);
        test12_1(&emptyBstring, 1, &emptyBstring, BSTR_ERR);
        test12_1(&shortBstring, 1, &shortBstring, 0);
        test12_1(&shortBstring, 5, &emptyBstring, 5);
        test12_1(&shortBstring, -1, &shortBstring, BSTR_ERR);
        test12_1(&shortBstring, 0, &shortBstring, 0);
        test12_1(&shortBstring, 0, b = b_strcpy(&shortBstring), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test12_1(&shortBstring, 0, b = b_fromcstr("BOGUS"), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test12_1(&longBstring, 0, &shortBstring, BSTR_ERR);
        test12_1(&longBstring, 20, &shortBstring, 10);
}
END_TEST

static void
test13_0(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_inchr(s1, pos, s2);
        warnx("ret -> %d, res -> %d", ret, res);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_013)
{
        bstring *b;
        int ret = 0;
        bstring multipleOs = bt_init("ooooo");
        test13_0(NULL, 0, NULL, BSTR_ERR);
        test13_0(&emptyBstring, 0, NULL, BSTR_ERR);
        test13_0(NULL, 0, &emptyBstring, BSTR_ERR);
        test13_0(&emptyBstring, 0, &badBstring1, BSTR_ERR);
        test13_0(&emptyBstring, 0, &badBstring2, BSTR_ERR);
        test13_0(&badBstring1, 0, &emptyBstring, BSTR_ERR);
        test13_0(&badBstring2, 0, &emptyBstring, BSTR_ERR);
        test13_0(&badBstring2, 0, &badBstring1, BSTR_ERR);
        test13_0(&badBstring1, 0, &badBstring2, BSTR_ERR);
        test13_0(&emptyBstring, 0, &emptyBstring, BSTR_ERR);
        test13_0(&shortBstring, 0, &emptyBstring, BSTR_ERR);
        test13_0(&shortBstring, 0, &shortBstring, 0);
        test13_0(&shortBstring, 0, &multipleOs, 1);
        test13_0(&shortBstring, 0, b = b_strcpy(&shortBstring), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test13_0(&shortBstring, -1, &shortBstring, BSTR_ERR);
        test13_0(&shortBstring, 10, &shortBstring, BSTR_ERR);
        test13_0(&shortBstring, 1, &shortBstring, 1);
        test13_0(&emptyBstring, 0, &shortBstring, BSTR_ERR);
        test13_0(&xxxxxBstring, 0, &shortBstring, BSTR_ERR);
        test13_0(&longBstring, 0, &shortBstring, 3);
        test13_0(&longBstring, 10, &shortBstring, 10);
}
END_TEST

static void
test14_0(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_inchrr(s1, pos, s2);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_014)
{
        bstring *b;
        int ret = 0;
        test14_0(NULL, 0, NULL, BSTR_ERR);
        test14_0(&emptyBstring, 0, NULL, BSTR_ERR);
        test14_0(NULL, 0, &emptyBstring, BSTR_ERR);
        test14_0(&emptyBstring, 0, &emptyBstring, BSTR_ERR);
        test14_0(&shortBstring, 0, &emptyBstring, BSTR_ERR);
        test14_0(&emptyBstring, 0, &badBstring1, BSTR_ERR);
        test14_0(&emptyBstring, 0, &badBstring2, BSTR_ERR);
        test14_0(&badBstring1, 0, &emptyBstring, BSTR_ERR);
        test14_0(&badBstring2, 0, &emptyBstring, BSTR_ERR);
        test14_0(&badBstring2, 0, &badBstring1, BSTR_ERR);
        test14_0(&badBstring1, 0, &badBstring2, BSTR_ERR);
        test14_0(&shortBstring, 0, &shortBstring, 0);
        test14_0(&shortBstring, 0, b = b_strcpy(&shortBstring), 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test14_0(&shortBstring, -1, &shortBstring, BSTR_ERR);
        test14_0(&shortBstring, 5, &shortBstring, 4);
        test14_0(&shortBstring, 4, &shortBstring, 4);
        test14_0(&shortBstring, 1, &shortBstring, 1);
        test14_0(&emptyBstring, 0, &shortBstring, BSTR_ERR);
        test14_0(&xxxxxBstring, 4, &shortBstring, BSTR_ERR);
        test14_0(&longBstring, 0, &shortBstring, BSTR_ERR);
        test14_0(&longBstring, 10, &shortBstring, 10);
}
END_TEST

static void
test15_0(bstring *b0, int pos, const bstring *b1, unsigned char fill, char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0 && b1 && b1->data &&
            b1->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_setstr(b2, pos, b1, fill);
                ck_assert_int_ne(ret, 0);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_setstr(b2, pos, b1, fill);
                if (b1) {
                        if (pos < 0) {
                                ck_assert_int_eq(b2->slen, b0->slen);
                        } else {
                                if (b2->slen != b0->slen + b1->slen) {
                                        ck_assert_int_eq(b2->slen,
                                                         pos + b1->slen);
                                }
                        }
                }
                ck_assert_ptr_nonnull(res);
                ck_assert(!((ret == 0) != (pos >= 0)));
                ret = strlen(res);
                ck_assert(b2->slen >= ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_setstr(b0, pos, b1, fill);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_015)
{
        /* tests with NULL */
        test15_0(NULL, 0, NULL, (unsigned char)'?', NULL);
        test15_0(NULL, 0, &emptyBstring, (unsigned char)'?', NULL);
        test15_0(&badBstring1, 0, NULL, (unsigned char)'?', NULL);
        test15_0(&badBstring1, 0, &badBstring1, (unsigned char)'?', NULL);
        test15_0(&emptyBstring, 0, &badBstring1, (unsigned char)'?', NULL);
        test15_0(&badBstring1, 0, &emptyBstring, (unsigned char)'?', NULL);
        test15_0(&badBstring2, 0, NULL, (unsigned char)'?', NULL);
        test15_0(&badBstring2, 0, &badBstring2, (unsigned char)'?', NULL);
        test15_0(&emptyBstring, 0, &badBstring2, (unsigned char)'?', NULL);
        test15_0(&badBstring2, 0, &emptyBstring, (unsigned char)'?', NULL);
        /* normal operation tests */
        test15_0(&emptyBstring, 0, &emptyBstring, (unsigned char)'?', "");
        test15_0(&emptyBstring, 5, &emptyBstring, (unsigned char)'?', "?????");
        test15_0(&emptyBstring, 5, &shortBstring, (unsigned char)'?',
                 "?????bogus");
        test15_0(&shortBstring, 0, &emptyBstring, (unsigned char)'?', "bogus");
        test15_0(&emptyBstring, 0, &shortBstring, (unsigned char)'?', "bogus");
        test15_0(&shortBstring, 0, &shortBstring, (unsigned char)'?', "bogus");
        test15_0(&shortBstring, -1, &shortBstring, (unsigned char)'?', "bogus");
        test15_0(&shortBstring, 2, &shortBstring, (unsigned char)'?',
                 "bobogus");
        test15_0(&shortBstring, 6, &shortBstring, (unsigned char)'?',
                 "bogus?bogus");
        test15_0(&shortBstring, 6, NULL, (unsigned char)'?', "bogus?");
}
END_TEST

static void
test16_0(bstring *b0, int pos, const bstring *b1, unsigned char fill, char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0 && b1 && b1->data &&
            b1->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_insert(b2, pos, b1, fill);
                ck_assert_int_ne(ret, 0);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_insert(b2, pos, b1, fill);
                if (b1) {
                        ck_assert(!((pos >= 0) &&
                                    (b2->slen != b0->slen + b1->slen) &&
                                    (b2->slen != pos + b1->slen)));
                        ck_assert(!((pos < 0) && (b2->slen != b0->slen)));
                        ck_assert(
                            !((ret == 0) != (pos >= 0 && pos <= b2->slen)));
                        ck_assert(
                            !((ret == 0) != (pos >= 0 && pos <= b2->slen)));
                }
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b2->slen, ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_insert(b0, pos, b1, fill);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_016)
{
        /* tests with NULL */
        test16_0(NULL, 0, NULL, (unsigned char)'?', NULL);
        test16_0(NULL, 0, &emptyBstring, (unsigned char)'?', NULL);
        test16_0(&badBstring1, 0, NULL, (unsigned char)'?', NULL);
        test16_0(&badBstring1, 0, &badBstring1, (unsigned char)'?', NULL);
        test16_0(&emptyBstring, 0, &badBstring1, (unsigned char)'?', NULL);
        test16_0(&badBstring1, 0, &emptyBstring, (unsigned char)'?', NULL);
        test16_0(&badBstring2, 0, NULL, (unsigned char)'?', NULL);
        test16_0(&badBstring2, 0, &badBstring2, (unsigned char)'?', NULL);
        test16_0(&emptyBstring, 0, &badBstring2, (unsigned char)'?', NULL);
        test16_0(&badBstring2, 0, &emptyBstring, (unsigned char)'?', NULL);
        /* normal operation tests */
        test16_0(&emptyBstring, 0, &emptyBstring, (unsigned char)'?', "");
        test16_0(&emptyBstring, 5, &emptyBstring, (unsigned char)'?', "?????");
        test16_0(&emptyBstring, 5, &shortBstring, (unsigned char)'?',
                 "?????bogus");
        test16_0(&shortBstring, 0, &emptyBstring, (unsigned char)'?', "bogus");
        test16_0(&emptyBstring, 0, &shortBstring, (unsigned char)'?', "bogus");
        test16_0(&shortBstring, 0, &shortBstring, (unsigned char)'?',
                 "bogusbogus");
        test16_0(&shortBstring, -1, &shortBstring, (unsigned char)'?', "bogus");
        test16_0(&shortBstring, 2, &shortBstring, (unsigned char)'?',
                 "bobogusgus");
        test16_0(&shortBstring, 6, &shortBstring, (unsigned char)'?',
                 "bogus?bogus");
        test16_0(&shortBstring, 6, NULL, (unsigned char)'?', "bogus");
}
END_TEST

static void
test17_0(bstring *s1, int pos, int len, char *res)
{
        bstring *b2;
        int ret = 0;
        if (s1 && s1->data && s1->slen >= 0) {
                b2 = b_strcpy(s1);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_delete(b2, pos, len);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(s1, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_delete(b2, pos, len);
                ck_assert(!((len >= 0) != (ret == 0)));
                ck_assert(!((b2->slen > s1->slen) ||
                            (b2->slen < pos && s1->slen >= pos)));
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b2->slen, ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_delete(s1, pos, len);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_017)
{
        /* tests with NULL */
        test17_0(NULL, 0, 0, NULL);
        test17_0(&badBstring1, 0, 0, NULL);
        test17_0(&badBstring2, 0, 0, NULL);
        /* normal operation tests */
        test17_0(&emptyBstring, 0, 0, "");
        test17_0(&shortBstring, 1, 3, "bs");
        test17_0(&shortBstring, -1, 3, "gus");
        test17_0(&shortBstring, 1, -3, "bogus");
        test17_0(&shortBstring, 3, 9, "bog");
        test17_0(&shortBstring, 3, 1, "bogs");
        test17_0(&longBstring, 4, 300, "This");
}
END_TEST

static void
test18_0(bstring *b, int len, int res, int mlen)
{
        int ret = 0;
        int ol = 0;
        if (b) {
                ol = b->mlen;
        }
        ret = b_alloc(b, len);
        ck_assert(!((b && b->data && b->slen >= 0 && ol > b->mlen)));
        ck_assert_int_eq(ret, res);
        ck_assert(!((b && (mlen > b->mlen || b->mlen == 0))));
}

static void
test18_1(bstring *b, int len, int res, int mlen)
{
        int ret = 0;
        ret = b_allocmin(b, len);
        if (b && b->data) {
                ck_assert_int_eq(b->mlen, mlen);
        }
        ck_assert_int_eq(ret, res);
}

START_TEST(core_018)
{
        int ret = 0;
        bstring *b = b_fromcstr("test");
        ck_assert_ptr_nonnull(b);
        /* tests with NULL */
        test18_0(NULL, 2, BSTR_ERR, 0);
        test18_0(&badBstring1, 2, BSTR_ERR, 0);
        test18_0(&badBstring2, 2, BSTR_ERR, 0);
        /* normal operation tests */
        test18_0(b, 2, 0, b->mlen);
        test18_0(b, -1, BSTR_ERR, b->mlen);
        test18_0(b, 9, 0, 9);
        test18_0(b, 2, 0, 9);
        b_writeprotect(*b);
        test18_0(b, 4, BSTR_ERR, b->mlen);
        b_writeallow(*b);
        test18_0(b, 2, 0, b->mlen);
        test18_0(&emptyBstring, 9, BSTR_ERR, emptyBstring.mlen);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        b = b_fromcstr("test");
        /* tests with NULL */
        test18_1(NULL, 2, BSTR_ERR, 0);
        test18_1(&badBstring1, 2, BSTR_ERR, 0);
        test18_1(&badBstring2, 2, BSTR_ERR, 2);
        /* normal operation tests */
        test18_1(b, 2, 0, b->slen + 1);
        test18_1(b, -1, BSTR_ERR, b->mlen);
        test18_1(b, 9, 0, 9);
        test18_1(b, 2, 0, b->slen + 1);
        test18_1(b, 9, 0, 9);
        b_writeprotect(*b);
        test18_1(b, 4, BSTR_ERR, -1);
        b_writeallow(*b);
        test18_1(b, 2, 0, b->slen + 1);
        test18_1(&emptyBstring, 9, BSTR_ERR, emptyBstring.mlen);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

static void
test19_0(bstring *b, int len, const char *res, int erv)
{
        int ret = 0;
        bstring *b1 = NULL;
        if (b && b->data && b->slen >= 0) {
                b1 = b_strcpy(b);
                ck_assert_ptr_nonnull(b1);
                b_writeprotect(*b1);
                ret = b_pattern(b1, len);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b1, b);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b1);
                ret = b_pattern(b1, len);
                ck_assert_int_eq(ret, erv);
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b1->slen, ret);
                ret = memcmp(b1->data, res, b1->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b1->data[b1->slen], '\0');
                ret = b_destroy(b1);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_pattern(b, len);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_019)
{
        /* tests with NULL */
        test19_0(NULL, 0, NULL, BSTR_ERR);
        test19_0(NULL, 5, NULL, BSTR_ERR);
        test19_0(NULL, -5, NULL, BSTR_ERR);
        test19_0(&badBstring1, 5, NULL, BSTR_ERR);
        test19_0(&badBstring2, 5, NULL, BSTR_ERR);
        /* normal operation tests */
        test19_0(&emptyBstring, 0, "", BSTR_ERR);
        test19_0(&emptyBstring, 10, "", BSTR_ERR);
        test19_0(&emptyBstring, -1, "", BSTR_ERR);
        test19_0(&shortBstring, 0, "", 0);
        test19_0(&shortBstring, 12, "bogusbogusbo", 0);
        test19_0(&shortBstring, -1, "bogus", BSTR_ERR);
}
END_TEST

START_TEST(core_020)
{
        int ret = 0;
        bstring *b, *c;
        /* tests with NULL */
        b = b_format(NULL, 1, 2);
        ck_assert_ptr_null(b);
        /* normal operation tests */
        b = b_format("%d %s", 1, "xy");
        ck_assert_ptr_nonnull(b);
        ret = b_iseq(c = b_fromcstr("1 xy"), b);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        b = b_format("%d %s(%s)", 6, c->data, shortBstring.data);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq(c = b_fromcstr("6 1 xy(bogus)"), b);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(c);
        ret = b_destroy(b);
        b = b_format("%s%s%s%s%s%s%s%s", longBstring.data, longBstring.data,
                     longBstring.data, longBstring.data, longBstring.data,
                     longBstring.data, longBstring.data, longBstring.data);
        c = b_strcpy(&longBstring);
        b_concat(c, c);
        b_concat(c, c);
        b_concat(c, c);
        ret = b_iseq(c, b);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(c);
        ret = b_destroy(b);
        b = b_fromcstr("");
        /* tests with NULL */
        ret = b_formata(b, NULL, 1, 2);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_formata(&badBstring1, "%d %d", 1, 2);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_formata(b, "%d %d", 1, 2);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq(c = b_fromcstr("1 2"), b);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_formata(b = b_fromcstr("x"), "%s%s%s%s%s%s%s%s",
                        longBstring.data, longBstring.data, longBstring.data,
                        longBstring.data, longBstring.data, longBstring.data,
                        longBstring.data, longBstring.data);
        ck_assert_int_eq(ret, BSTR_OK);
        c = b_strcpy(&longBstring);
        b_concat(c, c);
        b_concat(c, c);
        b_concat(c, c);
        b_insertch(c, 0, 1, 'x');
        ret = b_iseq(c, b);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        b = b_fromcstr("Initial");
        /* tests with NULL */
        ret = b_assign_format(b, NULL, 1, 2);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_assign_format(&badBstring1, "%d %d", 1, 2);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_assign_format(b, "%d %d", 1, 2);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq(c = b_fromcstr("1 2"), b);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_assign_format(b = b_fromcstr("x"), "%s%s%s%s%s%s%s%s",
                              longBstring.data, longBstring.data,
                              longBstring.data, longBstring.data,
                              longBstring.data, longBstring.data,
                              longBstring.data, longBstring.data);
        ck_assert_int_eq(ret, BSTR_OK);
        c = b_strcpy(&longBstring);
        ck_assert_ptr_nonnull(c);
        b_concat(c, c);
        b_concat(c, c);
        b_concat(c, c);
        ret = b_iseq(c, b);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(c);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

static void
test21_0(bstring *b, char sc, int ns)
{
        b_list *l;
        int ret = 0;
        if (b && b->data && b->slen >= 0) {
                bstring *c;
                bstring t;
                t = blk2tbstr(&sc, 1);
                l = b_split(b, sc);
                ck_assert_ptr_nonnull(l);
                ck_assert_int_eq(ns, l->qty);
                c = b_join(l, &t);
                ret = b_iseq(c, b);
                if (c && c->data)
                        warnx("c is %s", BS(c));
                ck_assert_int_eq(ret, 1);
                ret = b_destroy(c);
                ck_assert_int_eq(ret, BSTR_OK);
                ret = b_strListDestroy(l);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                l = b_split(b, sc);
                ck_assert_ptr_null(l);
        }
}

static void
test21_1(bstring *b, const bstring *sc, int ns)
{
        b_list *l;
        int ret = 0;
        if (b && b->data && b->slen >= 0) {
                bstring *c;
                l = b_splitstr(b, sc);
                ck_assert_ptr_nonnull(l);
                ck_assert_int_eq(ns, l->qty);
                c = b_join(l, sc);
                ck_assert_ptr_nonnull(c);
                ret = b_iseq(c, b);
                ck_assert_int_eq(ret, 1);
                ret = b_destroy(c);
                ck_assert_int_eq(ret, BSTR_OK);
                ret = b_strListDestroy(l);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                l = b_splitstr(b, sc);
                ck_assert_ptr_null(l);
        }
}

#define SAY (warnx("This is count %u", count++));

START_TEST(core_021)
{
        bstring is = bt_init("is");
        bstring ng = bt_init("ng");
        bstring delim = bt_init("aa");
        bstring beginWithDelim = bt_init("aaabcdaa1");
        bstring endWithDelim = bt_init("1aaabcdaa");
        bstring conseqDelim = bt_init("1aaaa1");
        bstring oneCharLeft = bt_init("aaaaaaa");
        bstring allDelim = bt_init("aaaaaa");
        int ret = 0;
        /* tests with NULL */
        short unsigned int count = 0;

SAY        test21_0(NULL, (char)'?', 0);
SAY        test21_0(&badBstring1, (char)'?', 0);
SAY        test21_0(&badBstring2, (char)'?', 0);
SAY        /* normal operation tests */
SAY        test21_0(&emptyBstring, (char)'?', 1);
SAY        test21_0(&shortBstring, (char)'o', 2);
SAY        test21_0(&shortBstring, (char)'s', 2);
SAY        test21_0(&shortBstring, (char)'b', 2);
SAY        test21_0(&longBstring, (char)'o', 9);
SAY        test21_1(NULL, NULL, 0);
SAY        test21_1(&badBstring1, &emptyBstring, 0);
SAY        test21_1(&badBstring2, &emptyBstring, 0);
SAY        /* normal operation tests */
SAY        test21_1(&shortBstring, &emptyBstring, 5);
SAY        test21_1(&longBstring, &is, 3);
SAY        test21_1(&longBstring, &ng, 5);
SAY        /* corner cases */
SAY        test21_1(&emptyBstring, &delim, 1);
SAY        test21_1(&delim, &delim, 2);
SAY        test21_1(&beginWithDelim, &delim, 3);
SAY        test21_1(&endWithDelim, &delim, 3);
SAY        test21_1(&conseqDelim, &delim, 3);
SAY        test21_1(&oneCharLeft, &delim, 4);
SAY        test21_1(&allDelim, &delim, 4);
        b_list *l;
        unsigned char c;
        bstring t;
        bstring *b;
        bstring *list[3] = {&emptyBstring, &shortBstring, &longBstring};
        int i;
        t = blk2tbstr(&c, 1);
        for (i = 0; i < 3; i++) {
                c = '\0';
                while (1) {
                        l = b_split(list[i], c);
                        ck_assert_ptr_nonnull(l);
                        b = b_join(l, &t);
                        ck_assert_ptr_nonnull(b);
                        ret = b_iseq(b, list[i]);
                        ck_assert_int_eq(ret, 1);
                        ret = b_destroy(b);
                        ck_assert_int_eq(ret, BSTR_OK);
                        ret = b_strListDestroy(l);
                        ck_assert_int_eq(ret, BSTR_OK);
                        l = b_splitstr(list[i], &t);
                        ck_assert_ptr_nonnull(l);
                        b = b_join(l, &t);
                        ck_assert_ptr_nonnull(b);
                        ret = b_iseq(b, list[i]);
                        ck_assert_int_eq(ret, 1);
                        ret = b_destroy(b);
                        ck_assert_int_eq(ret, BSTR_OK);
                        ret = b_strListDestroy(l);
                        ck_assert_int_eq(ret, BSTR_OK);
                        if (UCHAR_MAX == c) {
                                break;
                        }
                        c++;
                }
        }
}
END_TEST

static void
test22_0(const bstring *b, const bstring *sep, int ns, ...)
{
        va_list arglist;
        b_list *l;
        int ret = 0;
        if (b && b->data && b->slen >= 0 && sep && sep->data &&
            sep->slen >= 0) {
                l = b_splits(b, sep);
                if (l) {
                        int i;
                        va_start(arglist, ns);
                        for (i = 0; i < l->qty; i++) {
                                char *res;
                                res = va_arg(arglist, char *);
                                ck_assert_ptr_nonnull(res);
                                ret = strlen(res);
                                ck_assert(ret <= l->lst[i]->slen);
                                ret = memcmp(l->lst[i]->data, res,
                                             l->lst[i]->slen);
                                ck_assert_int_eq(ret, 0);
                                ck_assert_int_eq(
                                    l->lst[i]->data[l->lst[i]->slen], '\0');
                        }
                        va_end(arglist);
                        ck_assert_int_eq(ns, l->qty);
                        ret = b_strListDestroy(l);
                        ck_assert_int_eq(ret, BSTR_OK);
                } else {
                        ck_assert_int_eq(ns, 0);
                }
        } else {
                l = b_splits(b, sep);
                ck_assert_ptr_null(l);
        }
}

START_TEST(core_022)
{
        int ret = 0;
        bstring o = bt_init("o");
        bstring s = bt_init("s");
        bstring b = bt_init("b");
        bstring bs = bt_init("bs");
        bstring uo = bt_init("uo");
        /* tests with NULL */
        test22_0(NULL, &o, 0);
        test22_0(&o, NULL, 0);
        /* normal operation tests */
        test22_0(&emptyBstring, &o, 1, "");
        test22_0(&emptyBstring, &uo, 1, "");
        test22_0(&shortBstring, &emptyBstring, 1, "bogus");
        test22_0(&shortBstring, &o, 2, "b", "gus");
        test22_0(&shortBstring, &s, 2, "bogu", "");
        test22_0(&shortBstring, &b, 2, "", "ogus");
        test22_0(&shortBstring, &bs, 3, "", "ogu", "");
        test22_0(&longBstring, &o, 9, "This is a b", "gus but reas", "nably l",
                 "ng string.  Just l", "ng en", "ugh t", " cause s", "me mall",
                 "cing.");
        test22_0(&shortBstring, &uo, 3, "b", "g", "s");
        b_list *l;
        unsigned char c;
        bstring t;
        bstring *bb;
        bstring *list[3] = {&emptyBstring, &shortBstring, &longBstring};
        int i;
        t = blk2tbstr(&c, 1);
        for (i = 0; i < 3; i++) {
                c = '\0';
                while (1) {
                        bb = b_join(l = b_splits(list[i], &t), &t);
                        ret = b_iseq(bb, list[i]);
                        ck_assert_int_eq(ret, 1);
                        ret = b_destroy(bb);
                        ck_assert_int_eq(ret, BSTR_OK);
                        b_strListDestroy(l);
                        if (UCHAR_MAX == c) {
                                break;
                        }
                        c++;
                }
        }
}
END_TEST

struct sbstr {
        int ofs;
        bstring *b;
};

static size_t
test23_aux_read(void *buff, size_t elsize, size_t nelem, void *parm)
{
        struct sbstr *sb = (struct sbstr *)parm;
        int els, len;
        if (!parm || elsize == 0 || nelem == 0) {
                return 0;
        }
        len = (int)(nelem * elsize);
        if (len <= 0) {
                return 0;
        }
        if (len + sb->ofs > sb->b->slen) {
                len = sb->b->slen - sb->ofs;
        }
        els = (int)(len / elsize);
        len = (int)(els * elsize);
        if (len > 0) {
                memcpy(buff, sb->b->data + sb->ofs, len);
                sb->ofs += len;
        }
        return els;
}

static int
test23_aux_open(struct sbstr *sb, bstring *b)
{
        if (!sb || !b || !b->data) {
                return -__LINE__;
        }
        sb->ofs = 0;
        sb->b = b;
        return 0;
}

static int
test23_aux_splitcb(void *parm, BSTR_UNUSED int ofs, const bstring *lst)
{
        bstring *b = (bstring *)parm;
        if (b->slen > 0) {
                b_conchar(b, '|');
        }
        b_concat(b, lst);
        return 0;
}

struct tagBss {
        int first;
        unsigned char sc;
        bstring *b;
};

static int
test23_aux_splitcbx(void *parm, BSTR_UNUSED int ofs, const bstring *lst)
{
        struct tagBss *p = (struct tagBss *)parm;
        if (!p->first) {
                b_conchar(p->b, (char)p->sc);
        } else {
                p->first = 0;
        }
        b_concat(p->b, lst);
        return 0;
}

START_TEST(core_023)
{
        bstring space = bt_init(" ");
        struct sbstr sb;
        struct bStream *bs;
        bstring *b;
        int l, ret = 0;
        test23_aux_open(&sb, &longBstring);
        bs = bs_open((bNread)NULL, &sb);
        ck_assert_ptr_null(bs);
        bs = bs_open((bNread)test23_aux_read, &sb);
        ck_assert_ptr_nonnull(bs);
        ret = bs_eof(bs);
        ck_assert_int_eq(ret, 0);
        ret = bs_bufflength(NULL, -1);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = bs_bufflength(NULL, 1);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = bs_bufflength(bs, -1);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = bs_bufflength(bs, 1);
        ck_assert_int_ne(ret, BSTR_ERR);
        ret = bs_peek(NULL, bs);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = bs_readln(NULL, bs, (char)'?');
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = bs_readln(&emptyBstring, bs, (char)'?');
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = bs_peek(&emptyBstring, bs);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = bs_peek(b = b_fromcstr(""), bs);
        ck_assert_int_ne(ret, BSTR_ERR);
        ret = bs_readln(b, NULL, (char)'?');
        ck_assert_int_eq(ret, BSTR_ERR);
        b->slen = 0;
        ret = bs_readln(b, bs, (char)'?');
        ck_assert_int_ne(ret, BSTR_ERR);
        ret = bs_eof(bs);
        ck_assert_int_eq(ret, 1);
        ret = b_iseq(b, &longBstring);
        ck_assert_int_eq(ret, 1);
        ret = bs_unread(bs, b);
        ck_assert_int_ne(ret, BSTR_ERR);
        ret = bs_eof(bs);
        ck_assert_int_eq(ret, 0);
        b->slen = 0;
        ret = bs_peek(b, bs);
        ck_assert_int_ne(ret, BSTR_ERR);
        ret = b_iseq(b, &longBstring);
        ck_assert_int_eq(ret, 1);
        b->slen = 0;
        ret = bs_readln(b, bs, (char)'?');
        ck_assert_int_ne(ret, BSTR_ERR);
        ret = bs_eof(bs);
        ck_assert_int_eq(ret, 1);
        ret = b_iseq(b, &longBstring);
        ck_assert_int_eq(ret, 1);
        bs = bs_close(bs);
        ck_assert_ptr_nonnull(bs);
        sb.ofs = 0;
        bs = bs_open((bNread)test23_aux_read, &sb);
        ck_assert_ptr_nonnull(bs);
        b->slen = 0;
        ret = bs_readln(b, bs, (char)'.');
        ck_assert_int_ne(ret, BSTR_ERR);
        l = b->slen;
        ret = b_strncmp(b, &longBstring, l);
        ck_assert_int_eq(ret, 0);
        ck_assert_int_eq(longBstring.data[l - 1], '.');
        ret = bs_unread(bs, b);
        ck_assert_int_ne(ret, BSTR_ERR);
        b->slen = 0;
        ret = bs_peek(b, bs);
        ck_assert_int_ne(ret, BSTR_ERR);
        ret = b_iseq(b, &longBstring);
        ck_assert_int_eq(ret, 1);
        b->slen = 0;
        ret = bs_readln(b, bs, '.');
        ck_assert_int_ne(ret, BSTR_ERR);
        ck_assert_int_eq(b->slen, l);
        ret = b_strncmp(b, &longBstring, l);
        ck_assert_int_eq(ret, 0);
        ck_assert_int_eq(longBstring.data[l - 1], '.');
        bs = bs_close(bs);
        ck_assert_ptr_nonnull(bs);
        test23_aux_open(&sb, &longBstring);
        bs = bs_open((bNread)test23_aux_read, &sb);
        ck_assert_ptr_nonnull(bs);
        ret = bs_eof(bs);
        ck_assert_int_eq(ret, 0);
        b->slen = 0;
        l = bs_splitscb(bs, &space, test23_aux_splitcb, b);
        ret = bs_eof(bs);
        ck_assert_int_eq(ret, 1);
        bs = bs_close(bs);
        ck_assert_ptr_nonnull(bs);
        for (l = 1; l < 4; l++) {
                char *str;
                for (str = (char *)longBstring.data; *str; str++) {
                        test23_aux_open(&sb, &longBstring);
                        bs = bs_open((bNread)test23_aux_read, &sb);
                        ck_assert_ptr_nonnull(bs);
                        ret = bs_eof(bs);
                        ck_assert_int_eq(ret, 0);
                        ret = bs_bufflength(bs, l);
                        ck_assert(0 <= ret);
                        b->slen = 0;
                        while (0 == bs_readlna(b, bs, *str))
                                ;
                        ret = b_iseq(b, &longBstring);
                        ck_assert_int_eq(ret, 1);
                        ret = bs_eof(bs);
                        ck_assert_int_eq(ret, 1);
                        bs = bs_close(bs);
                        ck_assert_ptr_nonnull(bs);
                }
        }
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        unsigned char c;
        bstring t;
        bstring *list[3] = {&emptyBstring, &shortBstring, &longBstring};
        int i;
        t = blk2tbstr(&c, 1);
        for (i = 0; i < 3; i++) {
                c = '\0';
                while (1) {
                        struct tagBss bss;
                        bss.sc = c;
                        bss.b = b_fromcstr("");
                        ck_assert(bss.b != NULL);
                        bss.first = 1;
                        test23_aux_open(&sb, list[i]);
                        bs = bs_open((bNread)test23_aux_read, &sb);
                        ck_assert_ptr_nonnull(bs);
                        bs_splitscb(bs, &t, test23_aux_splitcbx, &bss);
                        bs = bs_close(bs);
                        ck_assert_ptr_nonnull(bs);
                        ret = b_iseq(bss.b, list[i]);
                        ck_assert_int_eq(ret, 1);
                        ret = b_destroy(bss.b);
                        ck_assert_int_eq(ret, BSTR_OK);
                        if (UCHAR_MAX == c) {
                                break;
                        }
                        c++;
                }
                while (1) {
                        struct tagBss bss;
                        bss.sc = c;
                        bss.b = b_fromcstr("");
                        ck_assert(bss.b != NULL);
                        bss.first = 1;
                        test23_aux_open(&sb, list[i]);
                        bs = bs_open((bNread)test23_aux_read, &sb);
                        ck_assert_ptr_nonnull(bs);
                        bs_splitstrcb(bs, &t, test23_aux_splitcbx, &bss);
                        bs = bs_close(bs);
                        ck_assert_ptr_nonnull(bs);
                        ret = b_iseq(bss.b, list[i]);
                        ck_assert_int_eq(ret, 1);
                        ret = b_destroy(bss.b);
                        ck_assert_int_eq(ret, BSTR_OK);
                        if (UCHAR_MAX == c) {
                                break;
                        }
                        c++;
                }
        }
}
END_TEST

static void
test24_0(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_ninchr(s1, pos, s2);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_024)
{
        bstring *b;
        int ret = 0;
        test24_0(NULL, 0, NULL, BSTR_ERR);
        test24_0(&emptyBstring, 0, NULL, BSTR_ERR);
        test24_0(NULL, 0, &emptyBstring, BSTR_ERR);
        test24_0(&shortBstring, 3, &badBstring1, BSTR_ERR);
        test24_0(&badBstring1, 3, &shortBstring, BSTR_ERR);
        test24_0(&emptyBstring, 0, &emptyBstring, BSTR_ERR);
        test24_0(&shortBstring, 0, &emptyBstring, BSTR_ERR);
        test24_0(&shortBstring, 0, &shortBstring, BSTR_ERR);
        test24_0(&shortBstring, 1, &shortBstring, BSTR_ERR);
        test24_0(&longBstring, 3, &shortBstring, 4);
        b = b_strcpy(&shortBstring);
        ck_assert_ptr_nonnull(b);
        test24_0(&longBstring, 3, b, 4);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test24_0(&longBstring, -1, &shortBstring, BSTR_ERR);
        test24_0(&longBstring, 1000, &shortBstring, BSTR_ERR);
        test24_0(&xxxxxBstring, 0, &shortBstring, 0);
        test24_0(&xxxxxBstring, 1, &shortBstring, 1);
        test24_0(&emptyBstring, 0, &shortBstring, BSTR_ERR);
        test24_0(&longBstring, 0, &shortBstring, 0);
        test24_0(&longBstring, 10, &shortBstring, 15);
}
END_TEST

static void
test25_0(bstring *s1, int pos, const bstring *s2, int res)
{
        int ret = b_ninchrr(s1, pos, s2);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_025)
{
        bstring *b;
        int ret = 0;
        test25_0(NULL, 0, NULL, BSTR_ERR);
        test25_0(&emptyBstring, 0, NULL, BSTR_ERR);
        test25_0(NULL, 0, &emptyBstring, BSTR_ERR);
        test25_0(&emptyBstring, 0, &emptyBstring, BSTR_ERR);
        test25_0(&shortBstring, 0, &emptyBstring, BSTR_ERR);
        test25_0(&shortBstring, 0, &badBstring1, BSTR_ERR);
        test25_0(&badBstring1, 0, &shortBstring, BSTR_ERR);
        test25_0(&shortBstring, 0, &shortBstring, BSTR_ERR);
        test25_0(&shortBstring, 4, &shortBstring, BSTR_ERR);
        test25_0(&longBstring, 10, &shortBstring, 9);
        b = b_strcpy(&shortBstring);
        ck_assert_ptr_nonnull(b);
        test25_0(&longBstring, 10, b, 9);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        test25_0(&xxxxxBstring, 4, &shortBstring, 4);
        test25_0(&emptyBstring, 0, &shortBstring, BSTR_ERR);
}
END_TEST

static void
test26_0(bstring *b0, int pos, int len, const bstring *b1, unsigned char fill,
         char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0 && b1 && b1->data &&
            b1->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_replace(b2, pos, len, b1, fill);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_replace(b2, pos, len, b1, fill);
                if (b1) {
                        ck_assert(
                            !(((ret == 0) != (pos >= 0 && pos <= b2->slen))));
                }
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert(b2->slen >= ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_replace(b0, pos, len, b1, fill);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_026)
{
        /* tests with NULL */
        test26_0(NULL, 0, 0, NULL, '?', NULL);
        test26_0(NULL, 0, 0, &emptyBstring, '?', NULL);
        test26_0(&badBstring1, 1, 3, &shortBstring, '?', NULL);
        test26_0(&shortBstring, 1, 3, &badBstring1, '?', NULL);
        /* normal operation tests */
        test26_0(&emptyBstring, 0, 0, &emptyBstring, '?', "");
        test26_0(&emptyBstring, 5, 0, &emptyBstring, '?', "?????");
        test26_0(&emptyBstring, 5, 0, &shortBstring, '?', "?????bogus");
        test26_0(&shortBstring, 0, 0, &emptyBstring, '?', "bogus");
        test26_0(&emptyBstring, 0, 0, &shortBstring, '?', "bogus");
        test26_0(&shortBstring, 0, 0, &shortBstring, '?', "bogusbogus");
        test26_0(&shortBstring, 1, 3, &shortBstring, '?', "bboguss");
        test26_0(&shortBstring, 3, 8, &shortBstring, '?', "bogbogus");
        test26_0(&shortBstring, -1, 0, &shortBstring, '?', "bogus");
        test26_0(&shortBstring, 2, 0, &shortBstring, '?', "bobogusgus");
        test26_0(&shortBstring, 6, 0, &shortBstring, '?', "bogus?bogus");
        test26_0(&shortBstring, 6, 0, NULL, '?', "bogus");
}
END_TEST

static void
test27_0(bstring *b0, const bstring *b1, const char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0 && b1 && b1->data &&
            b1->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_assign(b2, b1);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_assign(b2, b1);
                if (b1) {
                        ck_assert_int_eq(b2->slen, b1->slen);
                }
                ck_assert(!(((0 != ret) && (b1 != NULL)) ||
                            ((0 == ret) && (b1 == NULL))));
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b2->slen, ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_assign(b0, b1);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_027)
{
        /* tests with NULL */
        test27_0(NULL, NULL, NULL);
        test27_0(NULL, &emptyBstring, NULL);
        test27_0(&emptyBstring, NULL, "");
        test27_0(&badBstring1, &emptyBstring, NULL);
        test27_0(&badBstring2, &emptyBstring, NULL);
        test27_0(&emptyBstring, &badBstring1, NULL);
        test27_0(&emptyBstring, &badBstring2, NULL);
        /* normal operation tests on all sorts of subranges */
        test27_0(&emptyBstring, &emptyBstring, "");
        test27_0(&emptyBstring, &shortBstring, "bogus");
        test27_0(&shortBstring, &emptyBstring, "");
        test27_0(&shortBstring, &shortBstring, "bogus");
}
END_TEST

static void
test28_0(bstring *s1, int c, int res)
{
        int ret = b_strchr(s1, c);
        ck_assert_int_eq(ret, res);
}

static void
test28_1(bstring *s1, int c, int res)
{
        int ret = b_strrchr(s1, c);
        ck_assert_int_eq(ret, res);
}

static void
test28_2(bstring *s1, int c, int pos, int res)
{
        int ret = b_strchrp(s1, c, pos);
        ck_assert_int_eq(ret, res);
}

static void
test28_3(bstring *s1, int c, int pos, int res)
{
        int ret = b_strrchrp(s1, c, pos);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_028)
{
        test28_0(NULL, 0, BSTR_ERR);
        test28_0(&badBstring1, 'b', BSTR_ERR);
        test28_0(&badBstring2, 's', BSTR_ERR);
        test28_0(&emptyBstring, 0, BSTR_ERR);
        test28_0(&shortBstring, 0, BSTR_ERR);
        test28_0(&shortBstring, 'b', 0);
        test28_0(&shortBstring, 's', 4);
        test28_0(&shortBstring, 'q', BSTR_ERR);
        test28_0(&xxxxxBstring, 0, BSTR_ERR);
        test28_0(&xxxxxBstring, 'b', BSTR_ERR);
        test28_0(&longBstring, 'i', 2);
        test28_1(NULL, 0, BSTR_ERR);
        test28_1(&badBstring1, 'b', BSTR_ERR);
        test28_1(&badBstring2, 's', BSTR_ERR);
        test28_1(&emptyBstring, 0, BSTR_ERR);
        test28_1(&shortBstring, 0, BSTR_ERR);
        test28_1(&shortBstring, 'b', 0);
        test28_1(&shortBstring, 's', 4);
        test28_1(&shortBstring, 'q', BSTR_ERR);
        test28_1(&xxxxxBstring, 0, BSTR_ERR);
        test28_1(&xxxxxBstring, 'b', BSTR_ERR);
        test28_1(&longBstring, 'i', 82);
        test28_2(NULL, 0, 0, BSTR_ERR);
        test28_2(&badBstring1, 'b', 0, BSTR_ERR);
        test28_2(&badBstring2, 's', 0, BSTR_ERR);
        test28_2(&shortBstring, 'b', -1, BSTR_ERR);
        test28_2(&shortBstring, 'b', shortBstring.slen, BSTR_ERR);
        test28_2(&emptyBstring, 0, 0, BSTR_ERR);
        test28_2(&shortBstring, 0, 0, BSTR_ERR);
        test28_2(&shortBstring, 'b', 0, 0);
        test28_2(&shortBstring, 'b', 1, BSTR_ERR);
        test28_2(&shortBstring, 's', 0, 4);
        test28_2(&shortBstring, 'q', 0, BSTR_ERR);
        test28_3(NULL, 0, 0, BSTR_ERR);
        test28_3(&badBstring1, 'b', 0, BSTR_ERR);
        test28_3(&badBstring2, 's', 0, BSTR_ERR);
        test28_3(&shortBstring, 'b', -1, BSTR_ERR);
        test28_3(&shortBstring, 'b', shortBstring.slen, BSTR_ERR);
        test28_3(&emptyBstring, 0, 0, BSTR_ERR);
        test28_3(&shortBstring, 0, 0, BSTR_ERR);
        test28_3(&shortBstring, 'b', 0, 0);
        test28_3(&shortBstring, 'b', shortBstring.slen - 1, 0);
        test28_3(&shortBstring, 's', shortBstring.slen - 1, 4);
        test28_3(&shortBstring, 's', 0, BSTR_ERR);
}
END_TEST

static void
test29_0(bstring *b0, char *s, const char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_catcstr(b2, s);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_catcstr(b2, s);
                if (s) {
                        ck_assert_int_eq(b2->slen, b0->slen + (int)strlen(s));
                }
                ck_assert(!(((0 != ret) && (s != NULL)) ||
                            ((0 == ret) && (s == NULL))));
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b2->slen, ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_catcstr(b0, s);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_029)
{
        /* tests with NULL */
        test29_0(NULL, NULL, NULL);
        test29_0(NULL, "", NULL);
        test29_0(&emptyBstring, NULL, "");
        /* test29_0(&badBstring1, "bogus", NULL);
        test29_0(&badBstring2, "bogus", NULL); */
        /* normal operation tests on all sorts of subranges */
        test29_0(&emptyBstring, "", "");
        test29_0(&emptyBstring, "bogus", "bogus");
        test29_0(&shortBstring, "", "bogus");
        test29_0(&shortBstring, "bogus", "bogusbogus");
}
END_TEST

static void
test30_0(bstring *b0, const unsigned char *s, int len, const char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_catblk(b2, s, len);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_catblk(b2, s, len);
                if (s) {
                        if (len >= 0) {
                                ck_assert_int_eq(b2->slen, b0->slen + len);
                        } else {
                                ck_assert_int_eq(b2->slen, b0->slen);
                        }
                }
                ck_assert(!(((0 != ret) && (s && len >= 0)) ||
                            ((0 == ret) && (s == NULL || len < 0))));
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(ret, b2->slen);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_catblk(b0, s, len);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_030)
{
        /* tests with NULL */
        test30_0(NULL, NULL, 0, NULL);
        test30_0(NULL, (unsigned char *)"", 0, NULL);
        test30_0(&emptyBstring, NULL, 0, "");
        test30_0(&emptyBstring, NULL, -1, "");
        test30_0(&badBstring1, NULL, 0, NULL);
        test30_0(&badBstring2, NULL, 0, NULL);
        /* normal operation tests on all sorts of subranges */
        test30_0(&emptyBstring, (unsigned char *)"", -1, "");
        test30_0(&emptyBstring, (unsigned char *)"", 0, "");
        test30_0(&emptyBstring, (unsigned char *)"bogus", 5, "bogus");
        test30_0(&shortBstring, (unsigned char *)"", 0, "bogus");
        test30_0(&shortBstring, (unsigned char *)"bogus", 5, "bogusbogus");
        test30_0(&shortBstring, (unsigned char *)"bogus", -1, "bogus");
}
END_TEST

static void
test31_0(bstring *b0, const bstring *find, const bstring *replace, int pos,
         char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0 && find && find->data &&
            find->slen >= 0 && replace && replace->data && replace->slen >= 0)
        {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_findreplace(b2, find, replace, pos);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_findreplace(b2, find, replace, pos);
                ck_assert_int_eq(ret, BSTR_OK);
                /* if (res) { */
                        ret = strlen(res);
                        ck_assert(b2->slen >= ret);
                        ret = memcmp(b2->data, res, b2->slen);
                        ck_assert_int_eq(ret, 0);
                        ck_assert_int_eq(b2->data[b2->slen], '\0');
                /* } */
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_findreplace(b0, find, replace, pos);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

static void
test31_1(bstring *b0, const bstring *find, const bstring *replace, int pos,
         char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0 && find && find->data &&
            find->slen >= 0 && replace && replace->data && replace->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_findreplace_caseless(b2, find, replace, pos);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_findreplace_caseless(b2, find, replace, pos);
                ck_assert_int_eq(ret, BSTR_OK);
                /* if (res) { */
                        ret = strlen(res);
                        ck_assert(b2->slen >= ret);
                        ret = memcmp(b2->data, res, b2->slen);
                        ck_assert_int_eq(ret, 0);
                        ck_assert_int_eq(b2->data[b2->slen], '\0');
                /* } */
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_findreplace_caseless(b0, find, replace, pos);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

#define LOTS_OF_S                           \
        "sssssssssssssssssssssssssssssssss" \
        "sssssssssssssssssssssssssssssssss"

START_TEST(core_031)
{
        bstring t0 = bt_init("funny");
        bstring t1 = bt_init("weird");
        bstring t2 = bt_init("s");
        bstring t3 = bt_init("long");
        bstring t4 = bt_init("big");
        bstring t5 = bt_init("ss");
        bstring t6 = bt_init("sstsst");
        bstring t7 = bt_init("xx" LOTS_OF_S "xx");
        bstring t8 = bt_init("S");
        bstring t9 = bt_init("LONG");
        /* tests with NULL */
        test31_0(NULL, NULL, NULL, 0, NULL);
        test31_0(&shortBstring, NULL, &t1, 0, (char *)shortBstring.data);
        test31_0(&shortBstring, &t2, NULL, 0, (char *)shortBstring.data);
        test31_0(&badBstring1, &t2, &t1, 0, NULL);
        test31_0(&badBstring2, &t2, &t1, 0, NULL);
        /* normal operation tests */
        test31_0(&longBstring, &shortBstring, &t0, 0,
                 "This is a funny but reasonably long string.  "
                 "Just long enough to cause some mallocing.");
        test31_0(&longBstring, &t2, &t1, 0,
                 "Thiweird iweird a boguweird but reaweirdonably "
                 "long weirdtring.  Juweirdt long enough to cauweirde "
                 "weirdome mallocing.");
        test31_0(&shortBstring, &t2, &t1, 0, "boguweird");
        test31_0(&shortBstring, &t8, &t1, 0, "bogus");
        test31_0(&longBstring, &t2, &t1, 27,
                 "This is a bogus but reasonably long weirdtring.  "
                 "Juweirdt long enough to cauweirde weirdome mallocing.");
        test31_0(&longBstring, &t3, &t4, 0,
                 "This is a bogus but reasonably big string.  "
                 "Just big enough to cause some mallocing.");
        test31_0(&longBstring, &t9, &t4, 0,
                 "This is a bogus but reasonably long string.  "
                 "Just long enough to cause some mallocing.");
        test31_0(&t6, &t2, &t5, 0, "sssstsssst");
        test31_0(&t7, &t2, &t5, 0, "xx" LOTS_OF_S LOTS_OF_S "xx");
        /* tests with NULL */
        test31_1(NULL, NULL, NULL, 0, NULL);
        test31_1(&shortBstring, NULL, &t1, 0, (char *)shortBstring.data);
        test31_1(&shortBstring, &t2, NULL, 0, (char *)shortBstring.data);
        test31_1(&badBstring1, &t2, &t1, 0, NULL);
        test31_1(&badBstring2, &t2, &t1, 0, NULL);
        /* normal operation tests */
        test31_1(&longBstring, &shortBstring, &t0, 0,
                 "This is a funny but reasonably long string.  "
                 "Just long enough to cause some mallocing.");
        test31_1(&longBstring, &t2, &t1, 0,
                 "Thiweird iweird a boguweird but reaweirdonably "
                 "long weirdtring.  Juweirdt long enough to cauweirde "
                 "weirdome mallocing.");
        test31_1(&shortBstring, &t2, &t1, 0, "boguweird");
        test31_1(&shortBstring, &t8, &t1, 0, "boguweird");
        test31_1(&longBstring, &t2, &t1, 27,
                 "This is a bogus but reasonably long weirdtring.  "
                 "Juweirdt long enough to cauweirde weirdome mallocing.");
        test31_1(&longBstring, &t3, &t4, 0,
                 "This is a bogus but reasonably big string.  "
                 "Just big enough to cause some mallocing.");
        test31_1(&longBstring, &t9, &t4, 0,
                 "This is a bogus but reasonably big string.  "
                 "Just big enough to cause some mallocing.");
        test31_1(&t6, &t2, &t5, 0, "sssstsssst");
        test31_1(&t6, &t8, &t5, 0, "sssstsssst");
        test31_1(&t7, &t2, &t5, 0, "xx" LOTS_OF_S LOTS_OF_S "xx");
}
END_TEST

static void
test32_0(const bstring *b, const char *s, int res)
{
        int ret = b_iseq_cstr(b, s);
        ck_assert_int_eq(ret, res);
}

static void
test32_1(const bstring *b, const char *s, int res)
{
        int ret = b_iseq_cstr_caseless(b, s);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_032)
{
        int ret;
        bstring *b;
        /* tests with NULL */
        test32_0(NULL, NULL, BSTR_ERR);
        test32_0(&emptyBstring, NULL, BSTR_ERR);
        test32_0(NULL, "", BSTR_ERR);
        test32_0(&badBstring1, "", BSTR_ERR);
        test32_0(&badBstring2, "bogus", BSTR_ERR);
        /* normal operation tests on all sorts of subranges */
        test32_0(&emptyBstring, "", 1);
        test32_0(&shortBstring, "bogus", 1);
        test32_0(&emptyBstring, "bogus", 0);
        test32_0(&shortBstring, "", 0);
        b = b_strcpy(&shortBstring);
        ck_assert_ptr_nonnull(b);
        b->data[1]++;
        test32_0(b, (char *)shortBstring.data, 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
        /* tests with NULL */
        test32_1(NULL, NULL, BSTR_ERR);
        test32_1(&emptyBstring, NULL, BSTR_ERR);
        test32_1(NULL, "", BSTR_ERR);
        test32_1(&badBstring1, "", BSTR_ERR);
        test32_1(&badBstring2, "bogus", BSTR_ERR);
        /* normal operation tests on all sorts of subranges */
        test32_1(&emptyBstring, "", 1);
        test32_1(&shortBstring, "bogus", 1);
        test32_1(&shortBstring, "BOGUS", 1);
        test32_1(&emptyBstring, "bogus", 0);
        test32_1(&shortBstring, "", 0);
        b = b_strcpy(&shortBstring);
        ck_assert_ptr_nonnull(b);
        b->data[1]++;
        test32_1(b, (char *)shortBstring.data, 0);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

static void
test33_0(bstring *b0, const char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 && b0->data && b0->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_toupper(b2);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_toupper(b2);
                ck_assert_int_eq(ret, BSTR_OK);
                ck_assert_ptr_nonnull(b2);
                ck_assert_int_eq(b2->slen, b0->slen);
                /* if (res) { */
                        ret = strlen(res);
                        ck_assert_int_eq(b2->slen, ret);
                        ret = memcmp(b2->data, res, b2->slen);
                        ck_assert_int_eq(ret, 0);
                        ck_assert_int_eq(b2->data[b2->slen], '\0');
                /* } */
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_toupper(b0);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_033)
{
        /* tests with NULL */
        test33_0(NULL, NULL);
        test33_0(&badBstring1, NULL);
        test33_0(&badBstring2, NULL);
        /* normal operation tests on all sorts of subranges */
        test33_0(&emptyBstring, "");
        test33_0(&shortBstring, "BOGUS");
        test33_0(&longBstring, "THIS IS A BOGUS BUT REASONABLY LONG STRING.  "
                               "JUST LONG ENOUGH TO CAUSE SOME MALLOCING.");
}
END_TEST

static void
test34_0(bstring *b0, const char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 != NULL && b0->data != NULL && b0->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_tolower(b2);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_tolower(b2);
                ck_assert_int_eq(b2->slen, b0->slen);
                ck_assert_int_eq(ret, BSTR_OK);
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b2->slen, ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                b_destroy(b2);
        } else {
                ret = b_tolower(b0);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_034)
{
        /* tests with NULL */
        test34_0(NULL, NULL);
        test34_0(&badBstring1, NULL);
        test34_0(&badBstring2, NULL);
        /* normal operation tests on all sorts of subranges */
        test34_0(&emptyBstring, "");
        test34_0(&shortBstring, "bogus");
        test34_0(&longBstring, "this is a bogus but reasonably long string.  "
                               "just long enough to cause some mallocing.");
}
END_TEST

static void
test35_0(const bstring *b0, const bstring *b1, int res)
{
        int ret = b_stricmp(b0, b1);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_035)
{
        bstring t0 = bt_init("bOgUs");
        bstring t1 = bt_init("bOgUR");
        bstring t2 = bt_init("bOgUt");
        /* tests with NULL */
        test35_0(NULL, NULL, SHRT_MIN);
        test35_0(&emptyBstring, NULL, SHRT_MIN);
        test35_0(NULL, &emptyBstring, SHRT_MIN);
        test35_0(&emptyBstring, &badBstring1, SHRT_MIN);
        test35_0(&badBstring1, &emptyBstring, SHRT_MIN);
        test35_0(&shortBstring, &badBstring2, SHRT_MIN);
        test35_0(&badBstring2, &shortBstring, SHRT_MIN);
        /* normal operation tests on all sorts of subranges */
        test35_0(&emptyBstring, &emptyBstring, 0);
        test35_0(&shortBstring, &t0, 0);
        test35_0(&shortBstring, &t1,
                 tolower(shortBstring.data[4]) - tolower(t1.data[4]));
        test35_0(&shortBstring, &t2,
                 tolower(shortBstring.data[4]) - tolower(t2.data[4]));
        t0.slen++;
        test35_0(&shortBstring, &t0, -(UCHAR_MAX + 1));
        test35_0(&t0, &shortBstring, (UCHAR_MAX + 1));
}
END_TEST

static void
test36_0(const bstring *b0, const bstring *b1, int n, int res)
{
        int ret = b_strnicmp(b0, b1, n);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_036)
{
        bstring t0 = bt_init("bOgUs");
        bstring t1 = bt_init("bOgUR");
        bstring t2 = bt_init("bOgUt");
        /* tests with NULL */
        test36_0(NULL, NULL, 0, SHRT_MIN);
        test36_0(&emptyBstring, NULL, 0, SHRT_MIN);
        test36_0(NULL, &emptyBstring, 0, SHRT_MIN);
        test36_0(&emptyBstring, &badBstring1, 0, SHRT_MIN);
        test36_0(&badBstring1, &emptyBstring, 0, SHRT_MIN);
        test36_0(&shortBstring, &badBstring2, 5, SHRT_MIN);
        test36_0(&badBstring2, &shortBstring, 5, SHRT_MIN);
        /* normal operation tests on all sorts of subranges */
        test36_0(&emptyBstring, &emptyBstring, 0, 0);
        test36_0(&shortBstring, &t0, 0, 0);
        test36_0(&shortBstring, &t0, 5, 0);
        test36_0(&shortBstring, &t0, 4, 0);
        test36_0(&shortBstring, &t0, 6, 0);
        test36_0(&shortBstring, &t1, 5, shortBstring.data[4] - t1.data[4]);
        test36_0(&shortBstring, &t1, 4, 0);
        test36_0(&shortBstring, &t1, 6, shortBstring.data[4] - t1.data[4]);
        test36_0(&shortBstring, &t2, 5, shortBstring.data[4] - t2.data[4]);
        test36_0(&shortBstring, &t2, 4, 0);
        test36_0(&shortBstring, &t2, 6, shortBstring.data[4] - t2.data[4]);
        t0.slen++;
        test36_0(&shortBstring, &t0, 5, 0);
        test36_0(&shortBstring, &t0, 6, -(UCHAR_MAX + 1));
        test36_0(&t0, &shortBstring, 6, (UCHAR_MAX + 1));
}
END_TEST

static void
test37_0(const bstring *b0, const bstring *b1, int res)
{
        int ret = b_iseq_caseless(b0, b1);
        ck_assert_int_eq(ret, res);
}

START_TEST(core_037)
{
        bstring t0 = bt_init("bOgUs");
        bstring t1 = bt_init("bOgUR");
        bstring t2 = bt_init("bOgUt");
        /* tests with NULL */
        test37_0(NULL, NULL, BSTR_ERR);
        test37_0(&emptyBstring, NULL, BSTR_ERR);
        test37_0(NULL, &emptyBstring, BSTR_ERR);
        test37_0(&emptyBstring, &badBstring1, BSTR_ERR);
        test37_0(&badBstring1, &emptyBstring, BSTR_ERR);
        test37_0(&shortBstring, &badBstring2, BSTR_ERR);
        test37_0(&badBstring2, &shortBstring, BSTR_ERR);
        /* normal operation tests on all sorts of subranges */
        test37_0(&emptyBstring, &emptyBstring, 1);
        test37_0(&shortBstring, &t0, 1);
        test37_0(&shortBstring, &t1, 0);
        test37_0(&shortBstring, &t2, 0);
}
END_TEST

struct emuFile {
        int ofs;
        bstring *contents;
};

static int
test38_aux_bngetc(struct emuFile *f)
{
        int v = EOF;
        if (f) {
                v = b_chare(f->contents, f->ofs, EOF);
                if (EOF != v) {
                        f->ofs++;
                }
        }
        return v;
}

static size_t
test38_aux_bnread(void *buff, size_t elsize, size_t nelem, struct emuFile *f)
{
        char *b = (char *)buff;
        int v;
        size_t i, j, c = 0;
        if (!f || !b) {
                return c;
        }
        for (i = 0; i < nelem; i++) {
                for (j = 0; j < elsize; j++) {
                        v = test38_aux_bngetc(f);
                        if (EOF == v) {
                                *b = '\0';
                                return c;
                        } else {
                                *b = v;
                                b++;
                                c++;
                        }
                }
        }
        return c;
}

static int
test38_aux_bnopen(struct emuFile *f, bstring *b)
{
        if (!f || !b) {
                return -__LINE__;
        }
        f->ofs = 0;
        f->contents = b;
        return 0;
}

START_TEST(core_038)
{
        struct emuFile f;
        bstring *b0, *b1, *b2, *b3;
        int ret = test38_aux_bnopen(&f, &shortBstring);
        ck_assert_int_eq(ret, 0);
        /* Creation/reads */
        b0 = b_gets((bNgetc)test38_aux_bngetc, &f, 'b');
        ck_assert_ptr_nonnull(b0);
        b1 = b_read((bNread)test38_aux_bnread, &f);
        ck_assert_ptr_nonnull(b1);
        b2 = b_gets((bNgetc)test38_aux_bngetc, &f, '\0');
        ck_assert_ptr_null(b2);
        b3 = b_read((bNread)test38_aux_bnread, &f);
        ck_assert_ptr_nonnull(b3);
        ret = b_iseq_cstr(b0, "b");
        ck_assert_int_eq(ret, 1);
        ret = b_iseq_cstr(b1, "ogus");
        ck_assert_int_eq(ret, 1);
        ret = b_iseq_cstr(b3, "");
        ck_assert_int_eq(ret, 1);
        /* Bogus accumulations */
        f.ofs = 0;
        ret = b_getsa(NULL, (bNgetc)test38_aux_bngetc, &f, 'o');
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_reada(NULL, (bNread)test38_aux_bnread, &f);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_getsa(&shortBstring, (bNgetc)test38_aux_bngetc, &f, 'o');
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_reada(&shortBstring, (bNread)test38_aux_bnread, &f);
        ck_assert_int_eq(ret, BSTR_ERR);
        /* Normal accumulations */
        ret = b_getsa(b0, (bNgetc)test38_aux_bngetc, &f, 'o');
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_reada(b1, (bNread)test38_aux_bnread, &f);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq_cstr(b0, "bbo");
        ck_assert_int_eq(ret, 1);
        ret = b_iseq_cstr(b1, "ogusgus");
        ck_assert_int_eq(ret, 1);
        /* Attempt to append past end should do nothing */
        ret = b_getsa(b0, (bNgetc)test38_aux_bngetc, &f, 'o');
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_reada(b1, (bNread)test38_aux_bnread, &f);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq_cstr(b0, "bbo");
        ck_assert_int_eq(ret, 1);
        ret = b_iseq_cstr(b1, "ogusgus");
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(b0);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b1);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_destroy(b2);
        ck_assert_int_eq(ret, BSTR_ERR);
        b_destroy(b3);
        ck_assert_int_eq(ret, BSTR_ERR);
}
END_TEST

static void
test39_0(const bstring *b, const bstring *lt, const bstring *rt,
         const bstring *t)
{
        bstring *r;
        int ret = 0;
        ret = b_ltrimws(NULL);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_rtrimws(NULL);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_trimws(NULL);
        ck_assert_int_eq(ret, BSTR_ERR);
        r = b_strcpy(b);
        ck_assert_ptr_nonnull(r);
        b_writeprotect(*r);
        ret = b_ltrimws(r);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_rtrimws(r);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_trimws(r);
        ck_assert_int_eq(ret, BSTR_ERR);
        b_writeallow(*r);
        ret = b_ltrimws(r);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq(r, lt);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(r);
        ck_assert_int_eq(ret, BSTR_OK);
        r = b_strcpy(b);
        ck_assert_ptr_nonnull(r);
        ret = b_rtrimws(r);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq(r, rt);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(r);
        ck_assert_int_eq(ret, BSTR_OK);
        r = b_strcpy(b);
        ck_assert_ptr_nonnull(r);
        ret = b_trimws(r);
        ck_assert_int_eq(ret, BSTR_OK);
        ret = b_iseq(r, t);
        ck_assert_int_eq(ret, 1);
        ret = b_destroy(r);
        ck_assert_int_eq(ret, BSTR_OK);
}

START_TEST(core_039)
{
        bstring t0 = bt_init("   bogus string   ");
        bstring t1 = bt_init("bogus string   ");
        bstring t2 = bt_init("   bogus string");
        bstring t3 = bt_init("bogus string");
        bstring t4 = bt_init("     ");
        bstring t5 = bt_init("");
        test39_0(&t0, &t1, &t2, &t3);
        test39_0(&t1, &t1, &t3, &t3);
        test39_0(&t2, &t3, &t2, &t3);
        test39_0(&t3, &t3, &t3, &t3);
        test39_0(&t4, &t5, &t5, &t5);
        test39_0(&t5, &t5, &t5, &t5);
}
END_TEST

static void
test40_0(bstring *b0, const bstring *b1, int left, int len, const char *res)
{
        bstring *b2;
        int ret = 0;
        if (b0 != NULL && b0->data != NULL && b0->slen >= 0 && b1 != NULL &&
            b1->data != NULL && b1->slen >= 0) {
                b2 = b_strcpy(b0);
                ck_assert_ptr_nonnull(b2);
                b_writeprotect(*b2);
                ret = b_assign_midstr(b2, b1, left, len);
                ck_assert_int_ne(ret, 0);
                ret = b_iseq(b0, b2);
                ck_assert_int_eq(ret, 1);
                b_writeallow(*b2);
                ret = b_assign_midstr(b2, b1, left, len);
                if (b1) {
                        ck_assert(!((b2->slen > len) | (b2->slen < 0)));
                }
                ck_assert(!(((0 != ret) && (b1 != NULL)) ||
                            ((0 == ret) && (b1 == NULL))));
                ck_assert_ptr_nonnull(res);
                ret = strlen(res);
                ck_assert_int_eq(b2->slen, ret);
                ret = memcmp(b2->data, res, b2->slen);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b2->data[b2->slen], '\0');
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ret = b_assign_midstr(b0, b1, left, len);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
}

START_TEST(core_040)
{
        /* tests with NULL */
        test40_0(NULL, NULL, 0, 1, NULL);
        test40_0(NULL, &emptyBstring, 0, 1, NULL);
        test40_0(&emptyBstring, NULL, 0, 1, "");
        test40_0(&badBstring1, &emptyBstring, 0, 1, NULL);
        test40_0(&badBstring2, &emptyBstring, 0, 1, NULL);
        test40_0(&emptyBstring, &badBstring1, 0, 1, NULL);
        test40_0(&emptyBstring, &badBstring2, 0, 1, NULL);
        /* normal operation tests on all sorts of subranges */
        test40_0(&emptyBstring, &emptyBstring, 0, 1, "");
        test40_0(&emptyBstring, &shortBstring, 1, 3, "ogu");
        test40_0(&shortBstring, &emptyBstring, 0, 1, "");
        test40_0(&shortBstring, &shortBstring, 1, 3, "ogu");
        test40_0(&shortBstring, &shortBstring, -1, 4, "bog");
        test40_0(&shortBstring, &shortBstring, 1, 9, "ogus");
        test40_0(&shortBstring, &shortBstring, 9, 1, "");
}
END_TEST

static void
test41_0(bstring *b1, int left, int len)
{
        bstring t;
        bstring *b2, *b3;
        int ret = 0;
        if (b1 && b1->data && b1->slen >= 0) {
                b2 = b_fromcstr("");
                ck_assert_ptr_nonnull(b2);
                b_assign_midstr(b2, b1, left, len);
                bmid2tbstr(t, b1, left, len);
                b3 = b_strcpy(&t);
                ck_assert_ptr_nonnull(b3);
                ret = b_iseq(&t, b2);
                ck_assert_int_eq(ret, 1);
                ret = b_destroy(b2);
                ck_assert_int_eq(ret, BSTR_OK);
                ret = b_destroy(b3);
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                bmid2tbstr(t, b1, left, len);
                b3 = b_strcpy(&t);
                ck_assert_ptr_nonnull(b3);
                ck_assert_int_eq(t.slen, 0);
                ret = b_destroy(b3);
                ck_assert_int_eq(ret, BSTR_OK);
        }
}

START_TEST(core_041)
{
        /* tests with NULL */
        test41_0(NULL, 0, 1);
        test41_0(&emptyBstring, 0, 1);
        test41_0(NULL, 0, 1);
        test41_0(&emptyBstring, 0, 1);
        test41_0(&emptyBstring, 0, 1);
        test41_0(&badBstring1, 0, 1);
        test41_0(&badBstring2, 0, 1);
        /* normal operation tests on all sorts of subranges */
        test41_0(&emptyBstring, 0, 1);
        test41_0(&shortBstring, 1, 3);
        test41_0(&emptyBstring, 0, 1);
        test41_0(&shortBstring, 1, 3);
        test41_0(&shortBstring, -1, 4);
        test41_0(&shortBstring, 1, 9);
        test41_0(&shortBstring, 9, 1);
}
END_TEST

static void
test42_0(const bstring *bi, int len, const char *res)
{
        bstring *b;
        int ret = 0;
        ret = b_trunc(b = b_strcpy(bi), len);
        if (len >= 0) {
                ck_assert_int_eq(ret, BSTR_OK);
        } else {
                ck_assert_int_eq(ret, BSTR_ERR);
        }
        if (res) {
                ret = b_iseq_cstr(b, res);
                ck_assert_int_eq(ret, 1);
        }
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
}

START_TEST(core_042)
{
        int ret = 0;
        /* tests with NULL */
        ret = b_trunc(NULL, 2);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_trunc(NULL, 0);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_trunc(NULL, -1);
        ck_assert_int_eq(ret, BSTR_ERR);
        /* write protected */
        ret = b_trunc(&shortBstring, 2);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_trunc(&shortBstring, 0);
        ck_assert_int_eq(ret, BSTR_ERR);
        ret = b_trunc(&shortBstring, -1);
        ck_assert_int_eq(ret, BSTR_ERR);
        test42_0(&emptyBstring, 10, "");
        test42_0(&emptyBstring, 0, "");
        test42_0(&emptyBstring, -1, NULL);
        test42_0(&shortBstring, 10, "bogus");
        test42_0(&shortBstring, 3, "bog");
        test42_0(&shortBstring, 0, "");
        test42_0(&shortBstring, -1, NULL);
}
END_TEST

START_TEST(core_043)
{
        static bstring ts0 = bt_init("");
        static bstring ts1 = bt_init("    ");
        static bstring ts2 = bt_init(" abc");
        static bstring ts3 = bt_init("abc ");
        static bstring ts4 = bt_init(" abc ");
        static bstring ts5 = bt_init("abc");
        bstring *tstrs[6] = {&ts0, &ts1, &ts2, &ts3, &ts4, &ts5};
        int ret = 0;
        for (int i = 0; i < 6; i++) {
                bstring t;
                bstring *b;
                bt_fromblkltrimws(t, tstrs[i]->data, tstrs[i]->slen);
                b_ltrimws(b = b_strcpy(tstrs[i]));
                ret = b_iseq(b, &t);
                ck_assert_int_eq(ret, 1);
                ret = b_destroy(b);
                ck_assert_int_eq(ret, BSTR_OK);
                bt_fromblkrtrimws(t, tstrs[i]->data, tstrs[i]->slen);
                b_rtrimws(b = b_strcpy(tstrs[i]));
                ret = b_iseq(b, &t);
                ck_assert_int_eq(ret, 1);
                ret = b_destroy(b);
                ck_assert_int_eq(ret, BSTR_OK);
                bt_fromblktrimws(t, tstrs[i]->data, tstrs[i]->slen);
                b_trimws(b = b_strcpy(tstrs[i]));
                ret = b_iseq(b, &t);
                ck_assert_int_eq(ret, 1);
                ret = b_destroy(b);
                ck_assert_int_eq(ret, BSTR_OK);
        }
}
END_TEST

static void
test44_0(const char *str)
{
        int ret = 0;
        bstring *b = b_fromcstr("");
        ck_assert_ptr_nonnull(b);
        if (NULL == str) {
                ret = b_assign_cstr(NULL, "test");
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_cstr(b, NULL);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_cstr(&shortBstring, NULL);
                ck_assert_int_eq(ret, BSTR_ERR);
        } else {
                ret = b_assign_cstr(NULL, str);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_cstr(b, str);
                ck_assert_int_eq(ret, BSTR_OK);
                ret = strcmp(b_datae(b, ""), str);
                ck_assert_int_eq(ret, 0);
                ret = strlen(str);
                ck_assert_int_eq(b->slen, ret);
                ret = b_assign_cstr(b, "xxxxx");
                ck_assert_int_eq(ret, BSTR_OK);
                b_writeprotect(*b);
                ret = b_assign_cstr(b, str);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = strcmp(b_datae(b, ""), "xxxxx");
                ck_assert_int_eq(ret, 0);
                ret = strlen("xxxxx");
                ck_assert_int_eq(b->slen, ret);
                b_writeallow(*b);
                ret = b_assign_cstr(&shortBstring, str);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_cstr(&shortBstring, str);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
}

START_TEST(core_044)
{
        /* tests with NULL */
        test44_0(NULL);
        test44_0(EMPTY_STRING);
        test44_0(SHORT_STRING);
        test44_0(LONG_STRING);
}
END_TEST

static void
test45_0(const char *str)
{
        int ret = 0, len;
        bstring *b = b_fromcstr("");
        ck_assert_ptr_nonnull(b);
        if (!str) {
                ret = b_assign_blk(NULL, "test", 4);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_blk(b, NULL, 1);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_blk(&shortBstring, NULL, 1);
                ck_assert_int_eq(ret, BSTR_ERR);
        } else {
                len = strlen(str);
                ret = b_assign_blk(NULL, str, len);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_blk(b, str, len);
                ck_assert_int_eq(ret, BSTR_OK);
                ret = strcmp(b_datae(b, ""), str);
                ck_assert_int_eq(ret, 0);
                ck_assert_int_eq(b->slen, len);
                ret = b_assign_cstr(b, "xxxxx");
                ck_assert_int_eq(ret, BSTR_OK);
                b_writeprotect(*b);
                ret = b_assign_blk(b, str, len);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = strcmp(b_datae(b, ""), "xxxxx");
                ck_assert_int_eq(ret, 0);
                ret = strlen("xxxxx");
                ck_assert_int_eq(b->slen, ret);
                b_writeallow(*b);
                ret = b_assign_blk(&shortBstring, str, len);
                ck_assert_int_eq(ret, BSTR_ERR);
                ret = b_assign_blk(&shortBstring, str, len);
                ck_assert_int_eq(ret, BSTR_ERR);
        }
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
}

START_TEST(core_045)
{
        /* tests with NULL */
        test45_0(NULL);
        test45_0(EMPTY_STRING);
        test45_0(SHORT_STRING);
        test45_0(LONG_STRING);
}
END_TEST

static void
test46_0(const bstring *r, bstring *b, int count, const char *fmt, ...)
{
        int ret;
        va_list arglist;
        va_start(arglist, fmt);
        ret = b_vcformata(b, count, fmt, arglist);
        va_end(arglist);
        warnx("Ret is %d, r is %p", ret, r);
        if (ret < 0) {
                ck_assert_ptr_null(r);
        } else if (r == NULL) {
                ck_assert_int_eq(ret, 0);
        } else {
                ret = b_iseq(r, b);
                ck_assert_int_eq(ret, 1);
        }
}

static void
test46_1(bstring *b, const char *fmt, const bstring *r, ...)
{
        int ret;
        b_vformata(ret, b, fmt, r);
        if (ret < 0) {
                ck_assert_ptr_null(r);
        } else {
                ret = b_iseq(r, b);
                ck_assert_int_eq(ret, 1);
        }
}

START_TEST(core_046)
{
        bstring *b;
        int ret = 0;
        short unsigned int count = 0;
SAY        test46_0(NULL, NULL, 8, "[%d]", 15);
SAY        test46_0(NULL, &shortBstring, 8, "[%d]", 15);
SAY        test46_0(NULL, &badBstring1, 8, "[%d]", 15);
SAY        test46_0(NULL, &badBstring2, 8, "[%d]", 15);
SAY        test46_0(NULL, &badBstring3, 8, "[%d]", 15); b = b_fromcstr(""); ck_assert_ptr_nonnull(b);
SAY        test46_0(&shortBstring, b, shortBstring.slen, "%s", (char *)shortBstring.data); b->slen = 0;
SAY        test46_0(&shortBstring, b, shortBstring.slen + 1, "%s", (char *)shortBstring.data); b->slen = 0;
SAY        test46_0(NULL, b, shortBstring.slen - 1, "%s", (char *)shortBstring.data);
SAY        test46_1(NULL, "[%d]", NULL, 15);
SAY        test46_1(&shortBstring, "[%d]", NULL, 15);
SAY        test46_1(&badBstring1, "[%d]", NULL, 15);
SAY        test46_1(&badBstring2, "[%d]", NULL, 15);
SAY        test46_1(&badBstring3, "[%d]", NULL, 15); b->slen = 0;
SAY        test46_1(b, "%s", &shortBstring, (char *)shortBstring.data); b->slen = 0;
SAY        test46_1(b, "%s", &longBstring, (char *)longBstring.data);
        ret = b_destroy(b);
        ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

int
main(void)
{
        /* Build test suite */
        Suite *suite = suite_create("bstr-core");
        /* Core tests */
        TCase *core = tcase_create("Core");
        tcase_add_test(core, core_000);
        tcase_add_test(core, core_001);
        tcase_add_test(core, core_002);
        tcase_add_test(core, core_003);
        tcase_add_test(core, core_004);
        tcase_add_test(core, core_005);
        tcase_add_test(core, core_006);
        tcase_add_test(core, core_007);
        tcase_add_test(core, core_008);
        tcase_add_test(core, core_009);
        tcase_add_test(core, core_010);
        tcase_add_test(core, core_011);
        tcase_add_test(core, core_012);
        tcase_add_test(core, core_013);
        tcase_add_test(core, core_014);
        tcase_add_test(core, core_015);
        tcase_add_test(core, core_016);
        tcase_add_test(core, core_017);
        tcase_add_test(core, core_018);
        tcase_add_test(core, core_019);
        tcase_add_test(core, core_020);
        tcase_add_test(core, core_021);
        tcase_add_test(core, core_022);
        tcase_add_test(core, core_023);
        tcase_add_test(core, core_024);
        tcase_add_test(core, core_025);
        tcase_add_test(core, core_026);
        tcase_add_test(core, core_027);
        tcase_add_test(core, core_028);
        tcase_add_test(core, core_029);
        tcase_add_test(core, core_030);
        tcase_add_test(core, core_031);
        tcase_add_test(core, core_032);
        tcase_add_test(core, core_033);
        tcase_add_test(core, core_034);
        tcase_add_test(core, core_035);
        tcase_add_test(core, core_036);
        tcase_add_test(core, core_037);
        tcase_add_test(core, core_038);
        tcase_add_test(core, core_039);
        tcase_add_test(core, core_040);
        tcase_add_test(core, core_041);
        tcase_add_test(core, core_042);
        tcase_add_test(core, core_043);
        tcase_add_test(core, core_044);
        tcase_add_test(core, core_045);
        tcase_add_test(core, core_046);
        suite_add_tcase(suite, core);
        /* Run tests */
        SRunner *runner = srunner_create(suite);
        srunner_run_all(runner, CK_ENV);
        int number_failed = srunner_ntests_failed(runner);
        srunner_free(runner);
        return (0 == number_failed) ? EXIT_SUCCESS : EXIT_FAILURE;
}
