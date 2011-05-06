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
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include "euro.h"


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
	register int f;

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
		register int i;
		unsigned char c;
		signed char sz;


		c = *ib;
		sz = tbl[c].sz;

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
				CHECK_OB(1);
				*ob++ = ICV_NON_IDENTICAL_REPLACEMENT_CHAR;
				ib++;
				ret_val++;
				continue;

			} else {
				ERR_INT(EILSEQ);
			}

		} else if (sz == ICV_TYPE_NON_IDENTICAL_CHAR) {

			/* Count non-identicals in return value */
			ret_val++;

			if (f & ICONV_CONV_NON_IDENTICAL_DISCARD) {
				ib++;
				continue;

			} else if (f & ICONV_CONV_NON_IDENTICAL_REPLACE_HEX) {
				CHECK_OB(ICV_RHEX_LEN)
				PUT_RHEX(*ib, ob, NI);
				ib++;
				continue;

			} else if (f & ICONV_CONV_NON_IDENTICAL_TRANSLITERATE) {
				unsigned int u8 = tbl[c].ch;

				if (!u8) {
					CHECK_OB(1);
					/* No transliteration available. */
					*ob++ = ICV_NON_IDENTICAL_REPLACEMENT_CHAR;
					ib++;
					continue;
				}

				for (i=1, sz=4; i <= 4; i++)
					if ((u8 >> (4 - i)*8) == 0)
						sz--;
				CHECK_OB(sz);
				for (i=1; i <= sz; i++)
					*ob++ = (unsigned char)
					    ((u8 >> ((sz - i) * 8)) & 0xff);
				ib++;
				continue;

			} else {
				CHECK_OB(1);
				/* Implementation-defined non-identical conversion. */
				*ob++ = ICV_NON_IDENTICAL_REPLACEMENT_CHAR;
				ib++;
				continue;
			}
		}

		CHECK_OB(1);
		*ob++ = tbl[c].ch;
		ib++;
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

