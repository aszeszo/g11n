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
 * Copyright (c) 1999, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/isa_defs.h>
#include "utf_ebcdic_to_utf8.h"

void *
_icv_open_attr(int flag, void *reserved)
{
	STATE_T *cd = (STATE_T *)calloc(1, sizeof(STATE_T));

	if (cd == (STATE_T *)NULL) {
		errno = ENOMEM;
		return ((void *)-1);
	}
	cd->flags = flag;

	return ((void *)cd);
}


size_t
_icv_iconv(STATE_T *cd, const char **inbuf, size_t *inbufleft,
	char **outbuf, size_t *outbufleft)
{
	size_t ret_val = 0;
	uchar_t *ib;
	uchar_t *ob;
	uchar_t *ibtail;
	uchar_t *obtail;
	int f;

	if (! cd) {
		errno = EBADF;
		return ((size_t)-1);
	}

	if (!inbuf || !(*inbuf))
		return ((size_t)0);

	ib = (uchar_t *)*inbuf;
	ob = (uchar_t *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;
	f = cd->flags;

	while (ib < ibtail) {
		uint_t u4;
		int i, l;
		signed char sz; /* must be signed for loop condition */
		signed char obsz;


		sz = number_of_bytes_in_utf_ebcdic[*ib];

		if ((sz > UTF_EBCDIC_LEAD_OCTET_MAX) ||
		    (sz < UTF_EBCDIC_LEAD_OCTET_MIN)) {
			sz = 1;
			goto ILLEGAL_CHAR;
		}

		/* Handle RESTORE_HEX */

		if (f) {
			char *prefix = NULL;
			if (f & ICONV_CONV_ILLEGAL_RESTORE_HEX &&
			    *ib == (unsigned char) ICV_RHEX_PREFIX_IL_EBCDIC[0]) {
				prefix = ICV_RHEX_PREFIX_IL_EBCDIC;
			} else if (f & ICONV_CONV_NON_IDENTICAL_RESTORE_HEX &&
			    *ib == (unsigned char) ICV_RHEX_PREFIX_NI_EBCDIC[0]) {
				prefix = ICV_RHEX_PREFIX_NI_EBCDIC;
			}
			if (prefix &&
			    ibtail - ib >= ICV_RHEX_LEN &&
			    memcmp(ib, prefix, ICV_RHEX_PREFIX_ASCII_SZ) == 0) {

				i = _icv_ebcdic_restore_hex((char **)&ib,
				    ibtail - ib, (char **)&ob, obtail - ob);
				if (i == 1)
					continue;
				if (i == -1) {
					ret_val = (size_t)-1;
					break;
				}
			}
		}

		/* sz == 0 means control character. and it needs 1 byte */
		if ((ibtail - ib) < ((sz == 0) ? 1 : sz)) {
			if (f & ICONV_REPLACE_INVALID) {
				sz = ibtail - ib;
				goto INCOMPLETE_CHAR;
			}
			ERR_INT(EINVAL);
		}

		u4 = (UTF_EBCDIC_I8(*ib) & utf_ebcdic_masks_tbl[sz]);

		/* correct size */
		if (sz == 0)
			sz = 1;

		for (i = 1; i < sz; i++) {
			uchar_t cib = (uchar_t)*(ib + i);
			if (number_of_bytes_in_utf_ebcdic[cib] !=
			    UTF_EBCDIC_TRAILING_OCTET) {
				sz = i+1;
				goto ILLEGAL_CHAR;
			}
			u4 = ((u4 << UTF_EBCDIC_BIT_SHIFT) |
			    (((uint_t)(UTF_EBCDIC_I8(cib)))
			    & UTF_EBCDIC_BIT_MASK));
		}

		XPUTC(ob, u4);
		ib += sz;
		continue;

ILLEGAL_CHAR:

		/*
		 * Handle ILLEGAL and REPLACE_INVALID
		 * Here ib has the illegal character of sz bytes.
		 */
		if (f) {
			if (f & ICONV_CONV_ILLEGAL_DISCARD) {
				ib += sz;
				continue;

			} else if (f & ICONV_CONV_ILLEGAL_REPLACE_HEX) {
				CHECK_OB(sz * ICV_RHEX_LEN);
				for (l=0; l < sz; l++) {
					PUT_RHEX(*(ib+l), ob, IL);
				}
				ib += sz;
				continue;

			} else if (f & ICONV_REPLACE_INVALID) {
INCOMPLETE_CHAR:
				XPUTC(ob, ICV_CHAR_UCS2_REPLACEMENT);
				ib += sz;
				ret_val++;
				continue;
			}
		}

		/* Default scenario */
		ERR_INT(EILSEQ);
	}

_INTERRUPT:
	*inbuf = (char *)ib;
	*inbufleft = ibtail - ib;
	*outbuf = (char *)ob;
	*outbufleft = obtail - ob;

	return (ret_val);
}

int
_icv_iconvctl(STATE_T *cd, int req, void *arg)
{
	return _icv_flag_action(&cd->flags, req, (int *)arg, 0);
}

size_t
_icv_iconvstr(char *inarray, size_t *inlen, char *outarray,
	size_t *outlen, int flags)
{
	return _icv_ciconvstr(inarray, inlen, outarray, outlen, flags, 1);
}

int
_icv_ebcdic_restore_hex(char **inbuf, size_t inbufleft, char **outbuf,
	size_t outbufleft)
{
	unsigned char *ib = (unsigned char *)*inbuf;
	unsigned char c[2];
	int i;


	if (inbufleft < ICV_RHEX_LEN) 
		return 0;

	if (outbufleft < 1) {
		errno = E2BIG;
		return -1;
	}

	c[0] = ib[4];
	c[1] = ib[5];

	for (i=0; i < 2; i++) {
		if (c[i] >= 0xF0 && c[i] <= 0xF9) {	 /* 0 - 9 */
			c[i] -= 0xF0 - '0';
		} else if (c[i] >= 0xC1 && c[i] <= 0xC6) { /* A - F */
			c[i] -= 0xC1 - 'A';
		} else if (c[i] >= 0x81 && c[i] <= 0x86) { /* a - f */
			c[i] -= 0x81 - 'a';
		} else
			return 0;
	}

	**outbuf = (unsigned char)(HEX2DEC(c[0]) * 16 + HEX2DEC(c[1]));

	(*outbuf)++;
	*inbuf += ICV_RHEX_LEN;

	return 1;
}

