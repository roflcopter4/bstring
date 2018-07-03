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
 * This file is the C unit test for the bstraux module of Bstrlib.
 */

/* #include "bstraux.h" */
/* #include "bstrlib.h" */
#include "bstrlib.h"
#include "bstraux.h"
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

static int
tWrite(const void *buf, size_t elsize, size_t nelem, void *parm)
{
	bstring * b = (bstring *) parm;
	size_t i;
	if (NULL == b || NULL == buf || 0 == elsize || 0 == nelem) {
		return -__LINE__;
	}
	for (i = 0; i < nelem; ++i) {
		if (0 > b_catblk(b, buf, elsize)) {
			break;
		}
		buf = (const void *)(elsize + (const char *)buf);
	}
	return (int)i;
}

START_TEST(core_000)
{
	int ret = 0;
	bstring *s, *t;
	struct bwriteStream *ws;
	s = b_fromcstr("");
	ck_assert(s != NULL);
	ws = b_wsOpen((bNwrite)tWrite, s);
	ck_assert(ws != NULL);
	(void)b_wsBuffLength(ws, 8);
	ret = b_wsBuffLength(ws, 0);
	ck_assert_int_eq(ret, 8);
	ret = b_wsWriteBlk(ws, b_staticBlkParms("Hello "));
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_iseq_cstr(s, "");
	ck_assert_int_eq(ret, 1);
	ret = b_wsWriteBlk(ws, b_staticBlkParms("World\n"));
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_iseq_cstr(s, "Hello Wo");
	ck_assert_int_eq(ret, 1);
	t = b_wsClose(ws);
	ck_assert(t == s);
	ret = b_iseq_cstr(s, "Hello World\n");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(s);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_001)
{
	bstring t = b_static("Hello world");
	bstring *b, *c, *d;
	int ret = 0;
	b = b_Tail(&t, 5);
	ck_assert(b != NULL);
	c = b_Head(&t, 5);
	ck_assert(c != NULL);
	ret = b_iseq_cstr(b, "world");
	ck_assert_int_eq(ret, 1);
	ret = b_iseq_cstr(c, "Hello");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_destroy(c);
	ck_assert_int_eq(ret, BSTR_OK);
	b = b_Tail(&t, 0);
	ck_assert(b != NULL);
	c = b_Head(&t, 0);
	ck_assert(c != NULL);
	ret = b_iseq_cstr(b, "");
	ck_assert_int_eq(ret, 1);
	ret = b_iseq_cstr(c, "");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_destroy(c);
	ck_assert_int_eq(ret, BSTR_OK);
	d = b_strcpy(&t);
	ck_assert(d != NULL);
	b = b_Tail(d, 5);
	ck_assert(b != NULL);
	c = b_Head(d, 5);
	ck_assert(c != NULL);
	ret = b_iseq_cstr(b, "world");
	ck_assert_int_eq(ret, 1);
	ret = b_iseq_cstr(c, "Hello");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_destroy(c);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_destroy(d);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_002)
{
	bstring t = b_static("Hello world");
	int ret = 0;
	bstring * b;
	ret = b_SetChar(&t, 4, ',');
	ck_assert_int_eq(ret, BSTR_ERR);
	ret = b_SetChar(b = b_strcpy(&t), 4, ',');
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "Hell, world");
	ck_assert_int_eq(ret, 1);
	ret = b_SetChar(b, -1, 'x');
	ck_assert_int_eq(ret, BSTR_ERR);
	b->slen = 2;
	ret = b_SetChar(b, 1, 'i');
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "Hi");
	ck_assert_int_eq(ret, 1);
	ret = b_SetChar(b, 2, 's');
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "His");
	ck_assert_int_eq(ret, 1);
	ret = b_SetChar(b, 1, '\0');
	ck_assert_int_eq(ret, 0);
	ret = b_length(b);
	ck_assert_int_eq(ret, 3);
	ret = b_chare(b, 0, '?');
	ck_assert_int_eq(ret, 'H');
	ret = b_chare(b, 1, '?');
	ck_assert_int_eq(ret, '\0');
	ret = b_chare(b, 2, '?');
	ck_assert_int_eq(ret, 's');
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = 0;
	ret = b_SetCstrChar(&t, 4, ',');
	ck_assert_int_eq(ret, BSTR_ERR);
	b = b_strcpy(&t);
	ck_assert(b != NULL);
	ret = b_SetCstrChar(b, 4, ',');
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "Hell, world");
	ck_assert_int_eq(ret, 1);
	ret = b_SetCstrChar(b, -1, 'x');
	ck_assert_int_eq(ret, BSTR_ERR);
	b->slen = 2;
	ret = b_SetCstrChar(b, 1, 'i');
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "Hi");
	ck_assert_int_eq(ret, 1);
	ret = b_SetCstrChar(b, 2, 's');
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "His");
	ck_assert_int_eq(ret, 1);
	ret = b_SetCstrChar(b, 1, '\0');
	ck_assert_int_eq(ret, 0);
	ret = b_length(b);
	ck_assert_int_eq(ret, 1);
	ret = b_chare(b, 0, '?');
	ck_assert_int_eq(ret, 'H');
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_003)
{
	bstring t = b_static("Hello world");
	bstring * b;
	int ret = 0;
	ret = b_Fill(&t, 'x', 7);
	ck_assert_int_eq(ret, BSTR_ERR);
	b = b_strcpy(&t);
	ck_assert(b != NULL);
	ret = b_Fill(b, 'x', 7);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "xxxxxxx");
	ck_assert_int_eq(ret, 1);
	ret = b_Fill(b, 'x', -1);
	ck_assert(ret < 0);
	ret = b_Fill(b, 'x', 0);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_004)
{
	bstring t = b_static("foo");
	int ret = 0;
	bstring * b;
	ret = b_Replicate(&t, 4);
	ck_assert_int_eq(ret, BSTR_ERR);
	b = b_strcpy(&t);
	ck_assert(b != NULL);
	ret = b_Replicate(b, -1);
	ck_assert_int_eq(ret, BSTR_ERR);
	ret = b_Replicate(b, 4);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "foofoofoofoo");
	ck_assert_int_eq(ret, 1);
	ret = b_Replicate(b, 0);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_005)
{
	bstring t = b_static("Hello world");
	int ret = 0;
	bstring * b;
	ret = b_Reverse(&t);
	ck_assert(ret < 0);
	b = b_strcpy(&t);
	ck_assert(b != NULL);
	ret = b_Reverse(b);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "dlrow olleH");
	ck_assert_int_eq(ret, 1);
	b->slen = 0;
	ret = b_Reverse(b);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_006)
{
	bstring t = b_static("Hello world");
	int ret = 0;
	bstring * b;
	ret = b_InsertChrs(&t, 6, 4, 'x', '?');
	ck_assert(ret < 0);
	b = b_strcpy(&t);
	ck_assert(b != NULL);
	ret = b_InsertChrs(b, 6, 4, 'x', '?');
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "Hello xxxxworld");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_007)
{
	bstring t = b_static("  i am  ");
	int ret = 0;
	bstring *b;
	ret = b_JustifyLeft(&t, ' ');
	ck_assert(ret < 0);
	ret = b_JustifyRight(&t, 8, ' ');
	ck_assert(ret < 0);
	ret = b_JustifyMargin(&t, 8, ' ');
	ck_assert(ret < 0);
	ret = b_JustifyCenter(&t, 8, ' ');
	ck_assert(ret < 0);
	b = b_strcpy(&t);
	ck_assert(b != NULL);
	ret = b_JustifyLeft(b, ' ');
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_iseq_cstr(b, "i am");
	ck_assert_int_eq(ret, 1);
	ret = b_JustifyRight(b, 8, ' ');
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_iseq_cstr(b, "    i am");
	ck_assert_int_eq(ret, 1);
	ret = b_JustifyMargin(b, 8, ' ');
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_iseq_cstr(b, "i     am");
	ck_assert_int_eq(ret, 1);
	ret = b_JustifyCenter(b, 8, ' ');
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_iseq_cstr(b, "  i am");
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_008)
{
	bstring t = b_static("Hello world");
	int ret = 0;
	bstring *b;
	char *c;
	c = b_Str2NetStr(&t);
	ck_assert(c != NULL);
	ret = strcmp(c, "11:Hello world,");
	ck_assert_int_eq(ret, 0);
	b = b_NetStr2Bstr(c);
	ck_assert(b != NULL);
	ret = b_iseq(b, &t);
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_cstrfree(c);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_009)
{
	bstring t = b_static("Hello world");
	int err, ret = 0;
	bstring *b, *c;
	b = b_Base64Encode(&t);
	ck_assert(b != NULL);
	ret += 0 >= b_iseq_cstr(b, "SGVsbG8gd29ybGQ=");
	c = b_Base64DecodeEx(b, &err);
	ck_assert(b != NULL);
	ck_assert_int_eq(err, 0);
	ret += 0 >= b_iseq(c, &t);
	ck_assert_int_eq(ret, 0);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_destroy(c);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_010)
{
	bstring t = b_static("Hello world");
	int err, ret = 0;
	bstring *b, *c;
	b = b_UuEncode(&t);
	ck_assert(b != NULL);
	ret = b_iseq_cstr(b, "+2&5L;&\\@=V]R;&0`\r\n");
	ck_assert_int_eq(ret, 1);
	c = b_UuDecodeEx(b, &err);
	ck_assert(c != NULL);
	ck_assert_int_eq(err, 0);
	ret = b_iseq(c, &t);
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_destroy(c);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_011)
{
	bstring t = b_static("Hello world");
	unsigned char Ytstr[] = {
		0x72, 0x8f, 0x96, 0x96, 0x99, 0x4a,
		0xa1, 0x99, 0x9c, 0x96, 0x8e
	};
	bstring *b, *c;
	int ret = 0;
	b = b_YEncode(&t);
	ck_assert(b != NULL);
	ck_assert_int_eq(ret, 0);
	ret = b_is_stem_eq_blk(b, Ytstr, 11);
	ck_assert_int_eq(ret, 1);
	c = b_YDecode(b);
	ck_assert(c != NULL);
	ret = b_iseq(c, &t);
	ck_assert_int_eq(ret, 1);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
	ret = b_destroy(c);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

START_TEST(core_012)
{
	bstring t = b_static("Hello world");
	struct bStream *s;
	int ret = 0;
	bstring *b, *c;
	s = bs_FromBstr(&t);
	ck_assert(s != NULL);
	b = b_fromcstr("");
	ck_assert(b != NULL);
	ret = bs_read(b, s, 6);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "Hello ");
	ck_assert_int_eq(ret, 1);
	if (b) {
		b->slen = 0;
	}
	ret = bs_read(b, s, 6);
	ck_assert_int_eq(ret, 0);
	ret = b_iseq_cstr(b, "world");
	ck_assert_int_eq(ret, 1);
	c = bs_close(s);
	ck_assert(c == NULL);
	ret = b_destroy(b);
	ck_assert_int_eq(ret, BSTR_OK);
}
END_TEST

struct vfgetc {
	int ofs;
	bstring * base;
};

static int
core13_fgetc(void *ctx)
{
	struct vfgetc * vctx = (struct vfgetc *) ctx;
	int c;
	if (NULL == vctx || NULL == vctx->base) {
		return EOF;
	}
	if (vctx->ofs >= b_length (vctx->base)) {
		return EOF;
	}
	c = b_chare(vctx->base, vctx->ofs, EOF);
	vctx->ofs++;
	return c;
}

START_TEST(core_013)
{
	bstring t0 = b_static("Random String");
	struct vfgetc vctx;
	bstring *b;
	int ret = 0;
	int i;
	for (i = 0; i < 1000; i++) {
		vctx.ofs = 0;
		vctx.base = &t0;
		b = b_SecureInput(INT_MAX, '\n', (bNgetc)core13_fgetc, &vctx);
		ret = b_iseq(b, &t0);
		ck_assert_int_eq(ret, 1);
		b_SecureDestroy(b);
	}
}
END_TEST

int
main(void)
{
	/* Build test suite */
	Suite *suite = suite_create("bstr-aux");
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
	suite_add_tcase(suite, core);
	/* Run tests */
	SRunner *runner = srunner_create(suite);
	srunner_run_all(runner, CK_ENV);
	int number_failed = srunner_ntests_failed(runner);
	srunner_free(runner);
	return (0 == number_failed) ? EXIT_SUCCESS : EXIT_FAILURE;
}
