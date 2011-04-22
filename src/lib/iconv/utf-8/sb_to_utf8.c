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
 * Copyright (c) 2004, 2011, Oracle and/or its affiliates. All rights reserved.
 *
 * This program assumes that all single byte coded characters will be either
 * map to UTF-8 coded characters or illegal characters. Thus no replacement is
 * assumed at the moment.
 *
 * This particular file is to cover conversions from various single byte
 * codesets to UTF-8.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include "sb_to_utf8.h"


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
	unsigned char *ib;
	unsigned char *ob;
	unsigned char *ibtail;
	unsigned char *obtail;
	int f;

	if (! cd) {
		errno = EBADF;
		return ((size_t)-1);
	}

	if (!inbuf || !(*inbuf))
		return ((size_t)0);

	ib = (unsigned char *)*inbuf;
	ob = (unsigned char *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;
	f = cd->flags;

	while (ib < ibtail) {
		int i;
		unsigned long u8;
		signed char sz;

		u8 = (unsigned long)sb_u8_tbl[*ib].u8;
		sz = sb_u8_tbl[*ib].size;

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

		/* Handle ILLEGAL and REPLACE_INVALID */

		if (sz == ICV_TYPE_ILLEGAL_CHAR) {
			if (f & ICONV_CONV_ILLEGAL_DISCARD) {
				ib++;
				continue;

			} else if (f & ICONV_CONV_ILLEGAL_REPLACE_HEX) {
				CHECK_OB(ICV_RHEX_LEN);
				PUT_RHEX(*ib, ob, IL);
				ib++;
				continue;

			} else if (f & ICONV_REPLACE_INVALID) {
				u8 = ICV_CHAR_UTF8_REPLACEMENT;
				sz = ICV_CHAR_UTF8_REPLACEMENT_SIZE;
				ret_val++;

			} else {
				ERR_INT(EILSEQ);
			}
		}

		if ((u8 & ICV_UTF8_REPRESENTATION_ffff_mask) ==
		    ICV_UTF8_REPRESENTATION_fffe ||
		    (u8 & ICV_UTF8_REPRESENTATION_ffff_mask) ==
		    ICV_UTF8_REPRESENTATION_ffff ||
		    u8 > ICV_UTF8_REPRESENTATION_10fffd ||
		    (u8 >= ICV_UTF8_REPRESENTATION_d800 && 
		    u8 <= ICV_UTF8_REPRESENTATION_dfff) ||
		    (u8 >= ICV_UTF8_REPRESENTATION_fdd0 &&
		    u8 <= ICV_UTF8_REPRESENTATION_fdef)) {
			/* This should not happen, if sb_u8_tbl is right. */
       			ERR_INT(EILSEQ);
		}

		CHECK_OB(sz)
		for (i = 1; i <= sz; i++)
			*ob++ = (unsigned int)((u8 >> ((sz - i) * 8)) & 0xff);
		ib++;
	}

_INTERRUPT:
	*inbuf = (char *)ib;
	*inbufleft = ibtail - ib;
	*outbuf = (char *)ob;
	*outbufleft = obtail - ob;

	return (ret_val);
}

int _icv_iconvctl(STATE_T *cd, int req, void *arg)
{
	return _icv_flag_action(&cd->flags, req, (int *)arg,
	    ICONVCTL_NO_TRANSLIT);
}

