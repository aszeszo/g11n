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
 * Copyright (c) 1998, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/isa_defs.h>
#include "utf8_to_utf_ebcdic.h"


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
_icv_iconv(STATE_T *cd, char **inbuf, size_t *inbufleft,
	char **outbuf, size_t *outbufleft)
{
	size_t ret_val = 0;
	uchar_t *ib;
	uchar_t *ob;
	uchar_t *ibtail;
	uchar_t *obtail;
	int i, f;


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
		signed char sz;
		signed char obsz;


		sz = number_of_bytes_in_utf8_char[*ib];

		if (sz == ICV_TYPE_ILLEGAL_CHAR) {
			sz = 1;
			goto ILLEGAL_CHAR;
		}

		/* Handle RESTORE_HEX */

		if (f) {
			char *prefix = NULL;
			if (f & ICONV_CONV_ILLEGAL_RESTORE_HEX &&
			    *ib == ICV_RHEX_PREFIX_IL[0]) {
				prefix = ICV_RHEX_PREFIX_IL;
			} else if (f & ICONV_CONV_NON_IDENTICAL_RESTORE_HEX &&
			    *ib == ICV_RHEX_PREFIX_NI[0]) {
				prefix = ICV_RHEX_PREFIX_NI;
			}
			if (prefix &&
			    ibtail - ib >= ICV_RHEX_LEN &&
			    memcmp(ib, prefix, ICV_RHEX_PREFIX_ASCII_SZ) == 0) {

				i = _icv_restore_hex((char **)&ib, ibtail - ib,
				    (char **)&ob, obtail - ob);
				if (i == 1)
					continue;
				if (i == -1) {
					ret_val = (size_t)-1;
					break;
				}
			}
		}

		if ((ibtail - ib) < sz) {
			if (f & ICONV_REPLACE_INVALID) {
				sz = ibtail - ib;
				goto INCOMPLETE_CHAR;
			}
			ERR_INT(EINVAL);
		}

		u4 = (uint_t)(*ib & masks_tbl[sz]);
		for (i = 1; i < sz; i++) {
			uchar_t cib = (uchar_t)*(ib + i);
			if (i == 1) {
				if (cib < valid_min_2nd_byte[*ib] ||
				    cib > valid_max_2nd_byte[*ib]) {
					sz = 2;
					goto ILLEGAL_CHAR;
				}
			} else if (cib < 0x80 || cib > 0xbf) {
				sz = i+1;
				goto ILLEGAL_CHAR;
			}
			u4 = (u4 << ICV_UTF8_BIT_SHIFT) |
				(((uint_t)cib) & ICV_UTF8_BIT_MASK);
		}

		/* Check against known non-characters. */
		if ((u4 & ICV_UTF32_NONCHAR_mask) == ICV_UTF32_NONCHAR_fffe ||
		    (u4 & ICV_UTF32_NONCHAR_mask) == ICV_UTF32_NONCHAR_ffff ||
		    u4 > ICV_UTF32_LAST_VALID_CHAR ||
		    (u4 >= ICV_UTF32_SURROGATE_START_d800 &&
		    u4 <= ICV_UTF32_SURROGATE_END_dfff) ||
		    (u4 >= ICV_UTF32_ARABIC_NONCHAR_START_fdd0 &&
		    u4 <= ICV_UTF32_ARABIC_NONCHAR_END_fdef))
			goto ILLEGAL_CHAR;

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
				int l;

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
