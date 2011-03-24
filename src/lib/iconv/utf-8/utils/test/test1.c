/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2010, 2011, Oracle and/or its affiliates. All rights reserved.
 *
 * This program tests various aspects of Unicode iconv code conversions.
 *
 */
#include <stdio.h>
#include <iconv.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>

#define	MY_OUTBUF_LEN	1024

int
ti(char *tname, char *f, char *t,
	char *inbuf, size_t inbuflen, char *outbuf, size_t outbuflen,
	size_t expected_ret1,
	char *inbuf2, size_t inbuflen2, char *outbuf2, size_t outbuflen2,
	size_t expected_ret2,
	boolean_t check_reset, boolean_t check_alias_only)
{
	iconv_t cd;
	char *ibp, *obp, *ibp2, *obp2;
	size_t ibl, obl, ibl2, obl2;
	char ob[MY_OUTBUF_LEN];
	size_t ret;
	int i;

	printf("%s started:\n", tname);

	cd = iconv_open(t, f);
	if (cd == (iconv_t)-1) {
		printf("\ticonv_open(%s, %s) FAILED. err = %d\n", f, t, errno);
		goto FAILED_RET2;
	} else {
		printf("\ticonv_open(%s, %s) successful.\n", f, t);
	}

	if (check_alias_only)
		goto SUCCESSFUL_RET;

	errno = 0;
	ibp = inbuf;
	ibl = inbuflen;
	obp = ob;
	obl = MY_OUTBUF_LEN;

	ret = iconv(cd, (const char **)&ibp, &ibl, &obp, &obl);

	if (ret != expected_ret1) {
		printf("\ticonv(%s, %s) FAILED. ret = %d, errno = %d\n",
			f, t, ret, errno);
		goto FAILED_RET1;
	}

	printf("\ticonv(%s, %s) returned expected return = %d.\n", f, t,
		(int)ret);

	if (ret == (size_t)-1)
		goto SUCCESSFUL_RET;

	for (i = 0; i < (outbuflen - MY_OUTBUF_LEN + obl); i++)
		if (outbuf[i] != ob[i]) {
			printf("\ticonv(%s, %s) FAILED at byte %d.\n", f, t, i);
			printf("\t\texpected = 0x%02X, actual = 0x%02X\n",
				(unsigned int)outbuf[i], (unsigned int)ob[i]);

			goto FAILED_RET1;
		}

	printf("\ticonv(%s, %s) converted as expected.\n", f, t);

	if (check_reset) {
		errno = 0;
		ibp = NULL;
		ibl = 0;
		obp = ob;
		obl = MY_OUTBUF_LEN;

		ret = iconv(cd, (const char **)&ibp, &ibl, &obp, &obl);

		if (ret == (size_t)-1) {
			printf("\ticonv(%s, %s) reset FAILED.\n", f, t);
			goto FAILED_RET1;
		}

		printf("\ticonv-reset(%s, %s) reset successful.\n", f, t);

		errno = 0;
		ibp = inbuf2;
		ibl = inbuflen2;
		obp = ob;
		obl = MY_OUTBUF_LEN;
	
		ret = iconv(cd, (const char **)&ibp, &ibl, &obp, &obl);
	
		if (ret != expected_ret2) {
			printf("\ticonv-reset(%s, %s) FAILED. errno = %d\n",
				f, t, errno);
			goto FAILED_RET1;
		}

		for (i = 0; i < (outbuflen2 - MY_OUTBUF_LEN + obl); i++)
			if (outbuf2[i] != ob[i]) {
				printf("\ticonv-reset(%s, %s) FAILED at byte "
					"%d.\n", f, t, i);
				printf("\t\texpected=0x%02X, actual=0x%02X\n",
					(unsigned int)outbuf2[i],
					(unsigned int)ob[i]);
	
				goto FAILED_RET1;
			}
	
		printf("\ticonv-reset(%s, %s) converted as expected.\n", f, t);
	}

SUCCESSFUL_RET:
	iconv_close(cd);

	printf("\ticonv_close(%s, %s) done.\n", f, t);

	printf("%s ended.\n", tname);

	return (1);

FAILED_RET1:
	iconv_close(cd);

	printf("\ticonv_close(%s, %s) done.\n", f, t);

FAILED_RET2:
	printf("%s ended with FAILURE.\n", tname);

	return (0);
}


char *aliases[] = {
	"CP720",
	"Cp720",
	"cP720",
	"cp720",
	"CP-720",
	"CP_720",
	"720",

	"UCS-2-BIG-ENDIAN",
	"UCS2-BIG-ENDIAN",
	"UCS_2-BIG-ENDIAN",
	"ucs-2-big-endian",
	"UCS2BIGENDIAN",
	"ucs2bigendian",

	"UCS-2-LITTLE-ENDIAN",
	"UCS2-LITTLE-ENDIAN",
	"UCS_2-LITTLE-ENDIAN",
	"ucs-2-little-endian",
	"UCS2LITTLEENDIAN",
	"ucs2littleendian",

	"UTF-16-BIG-ENDIAN",
	"UTF16-BIG-ENDIAN",
	"UTF_16-BIG-ENDIAN",
	"utf-16-big-endian",
	"UTF16BIGENDIAN",
	"utf16bigendian",

	"UTF-16-LITTLE-ENDIAN",
	"UTF16-LITTLE-ENDIAN",
	"UTF_16-LITTLE-ENDIAN",
	"utf-16-little-endian",
	"UTF16LITTLEENDIAN",
	"utf16littleendian",

	"UCS-4-BIG-ENDIAN",
	"UCS4-BIG-ENDIAN",
	"UCS_4-BIG-ENDIAN",
	"ucs-4-big-endian",
	"UCS4BIGENDIAN",
	"ucs4bigendian",

	"UCS-4-LITTLE-ENDIAN",
	"UCS4-LITTLE-ENDIAN",
	"UCS_4-LITTLE-ENDIAN",
	"ucs-4-little-endian",
	"UCS4LITTLEENDIAN",
	"ucs4littleendian",

	"UTF-32-BIG-ENDIAN",
	"UTF32-BIG-ENDIAN",
	"UTF_32-BIG-ENDIAN",
	"utf-32-big-endian",
	"UTF32BIGENDIAN",
	"utf32bigendian",

	"UTF-32-LITTLE-ENDIAN",
	"UTF32-LITTLE-ENDIAN",
	"UTF_32-LITTLE-ENDIAN",
	"utf-32-little-endian",
	"UTF32LITTLEENDIAN",
	"utf32littleendian",

	NULL
};

char *l1[] = {
	"UCS-2-BIG-ENDIAN",
	"UCS-2-LITTLE-ENDIAN",
	"UTF-16-BIG-ENDIAN",
	"UTF-16-LITTLE-ENDIAN",
	NULL
};

typedef struct cs {
	char *name;
	char *out;
	size_t len;
} cs_t;

cs_t ucs1[] = {
	{ "UCS-2-BIG-ENDIAN", "\xfe\xff\x00\x41\x00\x42\x00\x43", 8 },
	{ "UCS-2-LITTLE-ENDIAN", "\xff\xfe\x41\x00\x42\x00\x43\x00", 8 },
	{ "UTF-16-BIG-ENDIAN", "\xfe\xff\x00\x41\x00\x42\x00\x43", 8 },
	{ "UTF-16-LITTLE-ENDIAN", "\xff\xfe\x41\x00\x42\x00\x43\x00", 8 },
	{ "UCS-4-BIG-ENDIAN",
	  "\x00\x00\xfe\xff\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43",
	  16 },
	{ "UCS-4-LITTLE-ENDIAN",
	  "\xff\xfe\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43\x00\x00\x00",
	  16 },
	{ "UTF-32-BIG-ENDIAN",
	  "\x00\x00\xfe\xff\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43",
	  16 },
	{ "UTF-32-LITTLE-ENDIAN",
	  "\xff\xfe\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43\x00\x00\x00",
	  16 },
	{ NULL, NULL, 0 }
};

cs_t cs1[] = {
	{ "646", "\x41\x42\x43", 3 },
	{ "8859-10", "\x41\x42\x43", 3 },
	{ "8859-13", "\x41\x42\x43", 3 },
	{ "8859-14", "\x41\x42\x43", 3 },
	{ "8859-15", "\x41\x42\x43", 3 },
	{ "8859-16", "\x41\x42\x43", 3 },
	{ "8859-1", "\x41\x42\x43", 3 },
	{ "8859-2", "\x41\x42\x43", 3 },
	{ "8859-3", "\x41\x42\x43", 3 },
	{ "8859-4", "\x41\x42\x43", 3 },
	{ "8859-5", "\x41\x42\x43", 3 },
	{ "8859-6", "\x41\x42\x43", 3 },
	{ "8859-7", "\x41\x42\x43", 3 },
	{ "8859-8", "\x41\x42\x43", 3 },
	{ "8859-9", "\x41\x42\x43", 3 },
	{ "CP1250", "\x41\x42\x43", 3 },
	{ "CP1251", "\x41\x42\x43", 3 },
	{ "CP1252", "\x41\x42\x43", 3 },
	{ "CP1253", "\x41\x42\x43", 3 },
	{ "CP1254", "\x41\x42\x43", 3 },
	{ "CP1255", "\x41\x42\x43", 3 },
	{ "CP1256", "\x41\x42\x43", 3 },
	{ "CP1257", "\x41\x42\x43", 3 },
	{ "CP1258", "\x41\x42\x43", 3 },
	{ "CP437", "\x41\x42\x43", 3 },
	{ "CP720", "\x41\x42\x43", 3 },
	{ "CP737", "\x41\x42\x43", 3 },
	{ "CP775", "\x41\x42\x43", 3 },
	{ "CP850", "\x41\x42\x43", 3 },
	{ "CP852", "\x41\x42\x43", 3 },
	{ "CP855", "\x41\x42\x43", 3 },
	{ "CP857", "\x41\x42\x43", 3 },
	{ "CP860", "\x41\x42\x43", 3 },
	{ "CP861", "\x41\x42\x43", 3 },
	{ "CP862", "\x41\x42\x43", 3 },
	{ "CP863", "\x41\x42\x43", 3 },
	{ "CP864", "\x41\x42\x43", 3 },
	{ "CP865", "\x41\x42\x43", 3 },
	{ "CP866", "\x41\x42\x43", 3 },
	{ "CP869", "\x41\x42\x43", 3 },
	{ "CP874", "\x41\x42\x43", 3 },
	{ "KOI8-R", "\x41\x42\x43", 3 },
	{ "KOI8-U", "\x41\x42\x43", 3 },
	{ "PTCP154", "\x41\x42\x43", 3 },
	{ "UCS-4BE", "\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43", 12 },
	{ "UCS-4-BIG-ENDIAN",
	  "\x00\x00\xfe\xff\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43",
	  16 },
	{ "UCS-4LE", "\x41\x00\x00\x00\x42\x00\x00\x00\x43\x00\x00\x00", 12 },
	{ "UCS-4-LITTLE-ENDIAN",
	  "\xff\xfe\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43\x00\x00\x00",
	  16 },
	{ "UCS-4",
	  "\x00\x00\xfe\xff\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43",
	  16 },
	{ "UTF-32BE", "\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43", 12 },
	{ "UTF-32-BIG-ENDIAN",
	  "\x00\x00\xfe\xff\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43",
	  16 },
	{ "UTF-32LE", "\x41\x00\x00\x00\x42\x00\x00\x00\x43\x00\x00\x00", 12 },
	{ "UTF-32-LITTLE-ENDIAN",
	  "\xff\xfe\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43\x00\x00\x00",
	  16 },
	{ "UTF-32",
	  "\x00\x00\xfe\xff\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43",
	  16 },
	{ "UTF-8", "\x41\x42\x43", 3 },
	{ NULL, NULL, 0 }
};

cs_t cs2[] = {
	{ "646", "\x41\x42\x43", 3 },
	{ "8859-10", "\x41\x42\x43", 3 },
	{ "8859-13", "\x41\x42\x43", 3 },
	{ "8859-14", "\x41\x42\x43", 3 },
	{ "8859-15", "\x41\x42\x43", 3 },
	{ "8859-16", "\x41\x42\x43", 3 },
	{ "8859-1", "\x41\x42\x43", 3 },
	{ "8859-2", "\x41\x42\x43", 3 },
	{ "8859-3", "\x41\x42\x43", 3 },
	{ "8859-4", "\x41\x42\x43", 3 },
	{ "8859-5", "\x41\x42\x43", 3 },
	{ "8859-6", "\x41\x42\x43", 3 },
	{ "8859-7", "\x41\x42\x43", 3 },
	{ "8859-8", "\x41\x42\x43", 3 },
	{ "8859-9", "\x41\x42\x43", 3 },
	{ "CP1250", "\x41\x42\x43", 3 },
	{ "CP1251", "\x41\x42\x43", 3 },
	{ "CP1252", "\x41\x42\x43", 3 },
	{ "CP1253", "\x41\x42\x43", 3 },
	{ "CP1254", "\x41\x42\x43", 3 },
	{ "CP1255", "\x41\x42\x43", 3 },
	{ "CP1256", "\x41\x42\x43", 3 },
	{ "CP1257", "\x41\x42\x43", 3 },
	{ "CP1258", "\x41\x42\x43", 3 },
	{ "CP437", "\x41\x42\x43", 3 },
	{ "CP720", "\x41\x42\x43", 3 },
	{ "CP737", "\x41\x42\x43", 3 },
	{ "CP775", "\x41\x42\x43", 3 },
	{ "CP850", "\x41\x42\x43", 3 },
	{ "CP852", "\x41\x42\x43", 3 },
	{ "CP855", "\x41\x42\x43", 3 },
	{ "CP857", "\x41\x42\x43", 3 },
	{ "CP860", "\x41\x42\x43", 3 },
	{ "CP861", "\x41\x42\x43", 3 },
	{ "CP862", "\x41\x42\x43", 3 },
	{ "CP863", "\x41\x42\x43", 3 },
	{ "CP864", "\x41\x42\x43", 3 },
	{ "CP865", "\x41\x42\x43", 3 },
	{ "CP866", "\x41\x42\x43", 3 },
	{ "CP869", "\x41\x42\x43", 3 },
	{ "CP874", "\x41\x42\x43", 3 },
	{ "KOI8-R", "\x41\x42\x43", 3 },
	{ "KOI8-U", "\x41\x42\x43", 3 },
	{ "PTCP154", "\x41\x42\x43", 3 },
	{ "UCS-2BE", "\x00\x41\x00\x42\x00\x43", 6 },
	{ "UCS-2-BIG-ENDIAN", "\xfe\xff\x00\x41\x00\x42\x00\x43", 8 },
	{ "UCS-2LE", "\x41\x00\x42\x00\x43\x00", 6 },
	{ "UCS-2-LITTLE-ENDIAN", "\xff\xfe\x41\x00\x42\x00\x43\x00", 8 },
	{ "UCS-2", "\xfe\xff\x00\x41\x00\x42\x00\x43", 8 },
	{ "UTF-16BE", "\x00\x41\x00\x42\x00\x43", 6 },
	{ "UTF-16-BIG-ENDIAN", "\xfe\xff\x00\x41\x00\x42\x00\x43", 8 },
	{ "UTF-16LE", "\x41\x00\x42\x00\x43\x00", 6 },
	{ "UTF-16-LITTLE-ENDIAN", "\xff\xfe\x41\x00\x42\x00\x43\x00", 8 },
	{ "UTF-16", "\xfe\xff\x00\x41\x00\x42\x00\x43", 8 },
	{ "UTF-8", "\x41\x42\x43", 3 },
	{ NULL, NULL, 0 }
};

char *m1[] = {
	"646",
	"8859-1",
	"8859-10",
	"8859-13",
	"8859-14",
	"8859-15",
	"8859-16",
	"8859-2",
	"8859-3",
	"8859-4",
	"8859-5",
	"8859-6",
	"8859-7",
	"8859-8",
	"8859-9",
	"CP1250",
	"CP1251",
	"CP1252",
	"CP1253",
	"CP1254",
	"CP1255",
	"CP1256",
	"CP1257",
	"CP1258",
	"CP437",
	"CP720",
	"CP737",
	"CP775",
	"CP850",
	"CP852",
	"CP855",
	"CP857",
	"CP860",
	"CP861",
	"CP862",
	"CP863",
	"CP864",
	"CP865",
	"CP866",
	"CP869",
	"CP874",
	"KOI8-R",
	"KOI8-U",
	"PTCP154",
	"UCS-2",
	"UCS-2-BIG-ENDIAN",
	"UCS-2-LITTLE-ENDIAN",
	"UCS-2BE",
	"UCS-2LE",
	"UCS-4",
	"UCS-4-BIG-ENDIAN",
	"UCS-4-LITTLE-ENDIAN",
	"UCS-4BE",
	"UCS-4LE",
	"UTF-16",
	"UTF-16-BIG-ENDIAN",
	"UTF-16-LITTLE-ENDIAN",
	"UTF-16BE",
	"UTF-16LE",
	"UTF-32",
	"UTF-32-BIG-ENDIAN",
	"UTF-32-LITTLE-ENDIAN",
	"UTF-32BE",
	"UTF-32LE",
	"UTF-8",
	"UTF-EBCDIC",
	NULL
};

char *l2[] = {
	"UCS-4-BIG-ENDIAN",
	"UCS-4-LITTLE-ENDIAN",
	"UTF-32-BIG-ENDIAN",
	"UTF-32-LITTLE-ENDIAN",
	NULL
};

char *m2[] = {
	"646",
	"8859-10",
	"8859-13",
	"8859-14",
	"8859-15",
	"8859-16",
	"8859-1",
	"8859-2",
	"8859-3",
	"8859-4",
	"8859-5",
	"8859-6",
	"8859-7",
	"8859-8",
	"8859-9",
	"CP1250",
	"CP1251",
	"CP1252",
	"CP1253",
	"CP1254",
	"CP1255",
	"CP1256",
	"CP1257",
	"CP1258",
	"CP437",
	"CP720",
	"CP737",
	"CP775",
	"CP850",
	"CP852",
	"CP855",
	"CP857",
	"CP860",
	"CP861",
	"CP862",
	"CP863",
	"CP864",
	"CP865",
	"CP866",
	"CP869",
	"CP874",
	"KOI8-R",
	"KOI8-U",
	"PTCP154",
	"UCS-2BE",
	"UCS-2-BIG-ENDIAN",
	"UCS-2LE",
	"UCS-2-LITTLE-ENDIAN",
	"UCS-2",
	"UTF-16BE",
	"UTF-16-BIG-ENDIAN",
	"UTF-16LE",
	"UTF-16-LITTLE-ENDIAN",
	"UTF-16",
	"UTF-32BE",
	"UTF-32-BIG-ENDIAN",
	"UTF-32LE",
	"UTF-32-LITTLE-ENDIAN",
	"UTF-32",
	"UTF-8",
	NULL
};


int
main(int ac, char **av)
{
	int i, j;
	char tn[80];
	char *ib = "\xfe\xff\x00\x41\x00\x42\x00\x43";
	char *ib2 = "\xff\xfe\x41\x00\x42\x00\x43\x00";
	char *ib3 = "\x00\x00\xfe\xff\x00\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43";
	char *ib4 = "\xff\xfe\x00\x00\x41\x00\x00\x00\x42\x00\x00\x00\x43\x00\x00\x00";
	char *ob = "ABC";
	char *ibu8a = "\xf4\xa0\x80\x80";
	char *ibu8b = "\xc2\x80\xe0\x80\x80";
	char *ibu8c = "\xe0\xa0\x80\xf5\x80\x80\x80";
	char *ibu8d = "\xff\x80\x80\x80";
	char temp[48];

	system("date");
	putchar('\n');

	/* UCS-2-* and UTF-16-* tests: */
	for (i = 0; l1[i]; i++) {
		for (j = 0; cs1[j].name; j++) {
			sprintf(tn, "%s to %s test with big endian data",
				l1[i], cs1[j].name); 
			ti(tn, l1[i], cs1[j].name,
				ib, 8, cs1[j].out, cs1[j].len, 0,
				ib, 8, cs1[j].out, cs1[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	for (i = 0; l1[i]; i++) {
		for (j = 0; cs1[j].name; j++) {
			sprintf(tn, "%s to %s test with little endian data",
				l1[i], cs1[j].name); 
			ti(tn, l1[i], cs1[j].name,
				ib2, 8, cs1[j].out, cs1[j].len, 0,
				ib2, 8, cs1[j].out, cs1[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	for (i = 0; l1[i]; i++) {
		for (j = 0; cs1[j].name; j++) {
			sprintf(tn, "%s to %s test with mixed endian data 1",
				l1[i], cs1[j].name); 
			ti(tn, l1[i], cs1[j].name,
				ib, 8, cs1[j].out, cs1[j].len, 0,
				ib2, 8, cs1[j].out, cs1[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	for (i = 0; l1[i]; i++) {
		for (j = 0; cs1[j].name; j++) {
			sprintf(tn, "%s to %s test with mixed endian data 2",
				l1[i], cs1[j].name); 
			ti(tn, l1[i], cs1[j].name,
				ib2, 8, cs1[j].out, cs1[j].len, 0,
				ib, 8, cs1[j].out, cs1[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	/* UCS-4-* and UTF-32-* tests: */
	for (i = 0; l2[i]; i++) {
		for (j = 0; cs2[j].name; j++) {
			sprintf(tn, "%s to %s test with big endian data",
				l2[i], cs2[j].name); 
			ti(tn, l2[i], cs2[j].name,
				ib3, 16, cs2[j].out, cs2[j].len, 0,
				ib3, 16, cs2[j].out, cs2[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	for (i = 0; l2[i]; i++) {
		for (j = 0; cs2[j].name; j++) {
			sprintf(tn, "%s to %s test with little endian data",
				l2[i], cs2[j].name); 
			ti(tn, l2[i], cs2[j].name,
				ib4, 16, cs2[j].out, cs2[j].len, 0,
				ib4, 16, cs2[j].out, cs2[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	for (i = 0; l2[i]; i++) {
		for (j = 0; cs2[j].name; j++) {
			sprintf(tn, "%s to %s test with mixed endian data 1",
				l2[i], cs2[j].name); 
			ti(tn, l2[i], cs2[j].name,
				ib3, 16, cs2[j].out, cs2[j].len, 0,
				ib4, 16, cs2[j].out, cs2[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	for (i = 0; l2[i]; i++) {
		for (j = 0; cs2[j].name; j++) {
			sprintf(tn, "%s to %s test with mixed endian data 2",
				l2[i], cs2[j].name); 
			ti(tn, l2[i], cs2[j].name,
				ib4, 16, cs2[j].out, cs2[j].len, 0,
				ib3, 16, cs2[j].out, cs2[j].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	/* Now reverse tests: */
	for (j = 0; cs1[j].name; j++) {
		for (i = 0; ucs1[i].name; i++) {
			if (strncmp(cs1[j].name, "UCS-4", 5) == 0 &&
			    strncmp(ucs1[i].name, "UCS-4", 5) == 0)
				continue;
			if (strncmp(cs1[j].name, "UTF-32", 6) == 0 &&
			    strncmp(ucs1[i].name, "UTF-32", 6) == 0)
				continue;

			sprintf(tn, "%s to %s (reverse) test",
				cs1[j].name, ucs1[i].name);
			ti(tn, cs1[j].name, ucs1[i].name,
				cs1[j].out, cs1[j].len,
					ucs1[i].out, ucs1[i].len, 0,
				cs1[j].out, cs1[j].len,
					ucs1[i].out, ucs1[i].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}
	for (j = 0; cs2[j].name; j++) {
		for (i = 0; ucs1[i].name; i++) {
			if (strncmp(cs2[j].name, "UCS-2", 5) == 0 &&
			    strncmp(ucs1[i].name, "UCS-2", 5) == 0)
				continue;
			if (strncmp(cs2[j].name, "UCS-2", 5) == 0 &&
			    strncmp(ucs1[i].name, "UTF-16", 6) == 0)
				continue;
			if (strncmp(cs2[j].name, "UTF-16", 6) == 0 &&
			    strncmp(ucs1[i].name, "UCS-2", 5) == 0)
				continue;
			if (strncmp(cs2[j].name, "UTF-16", 6) == 0 &&
			    strncmp(ucs1[i].name, "UTF-16", 6) == 0)
				continue;

			sprintf(tn, "%s to %s (reverse) test",
				cs2[j].name, ucs1[i].name);
			ti(tn, cs2[j].name, ucs1[i].name,
				cs2[j].out, cs2[j].len,
					ucs1[i].out, ucs1[i].len, 0,
				cs2[j].out, cs2[j].len,
					ucs1[i].out, ucs1[i].len, 0,
				B_TRUE, B_FALSE);
			putchar('\n');
		}
	}

	/* Test aliases: */
	for (i = 0; aliases[i]; i++) {
		for (j = 0; cs1[j].name; j++) {
			sprintf(tn, "%s to UTF-8 alias test", aliases[i]);
			ti(tn, aliases[i], "UTF-8",
				NULL, 0, NULL, 0, 0,
				NULL, 0, NULL, 0, 0,
				B_FALSE, B_TRUE);
			putchar('\n');
		}
	}

	/* Test UTF-8 illegal sequences. */
	for (i = 0; m1[i]; i++) {
		sprintf(tn, "UTF-8 to %s test on illegal UTF-8 bytes 1", m1[i]);
		ti(tn, "UTF-8", m1[i],
			ibu8a, strlen(ibu8a), temp, 48, (size_t)-1,
			NULL, 0, NULL, 0, 0,
			B_FALSE, B_FALSE);
		putchar('\n');
	}

	for (i = 0; m1[i]; i++) {
		sprintf(tn, "UTF-8 to %s test on illegal UTF-8 bytes 2", m1[i]);
		ti(tn, "UTF-8", m1[i],
			ibu8b, strlen(ibu8b), temp, 48, (size_t)-1,
			NULL, 0, NULL, 0, 0,
			B_FALSE, B_FALSE);
		putchar('\n');
	}

	for (i = 0; m1[i]; i++) {
		sprintf(tn, "UTF-8 to %s test on illegal UTF-8 bytes 3", m1[i]);
		ti(tn, "UTF-8", m1[i],
			ibu8c, strlen(ibu8c), temp, 48, (size_t)-1,
			NULL, 0, NULL, 0, 0,
			B_FALSE, B_FALSE);
		putchar('\n');
	}

	for (i = 0; m1[i]; i++) {
		sprintf(tn, "UTF-8 to %s test on illegal UTF-8 bytes4 ", m1[i]);
		ti(tn, "UTF-8", m1[i],
			ibu8d, strlen(ibu8d), temp, 48, (size_t)-1,
			NULL, 0, NULL, 0, 0,
			B_FALSE, B_FALSE);
		putchar('\n');
	}

	putchar('\n');
	system("date");
}
