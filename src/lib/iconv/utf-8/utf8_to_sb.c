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
 *
 * This particular file is to cover conversions from UTF-8 to various single
 * byte codesets.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <sys/types.h>
#include "utf8_to_sb.h"

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
       	int f;
	size_t len;


	if (! cd) {
		errno = EBADF;
		return ((size_t)-1);
	}

	if (!inbuf || !(*inbuf)) {
		cd->bom_written = false;
		return ((size_t)0);
	}

	ib = (uchar_t *)*inbuf;
	ob = (uchar_t *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;
	f = cd->flags;

	/* We skip the first signature of UTF-8 BOM if any. */
	if (! cd->bom_written) {
		len = ibtail - ib;
		if (len >= ICV_FETCH_UTF8_BOM_SIZE) {
			if (ib[0] == 0xef && ib[1] == 0xbb && ib[2] == 0xbf) {
				ib += ICV_FETCH_UTF8_BOM_SIZE;
			}
			cd->bom_written = true;
		} else if ((len == 1 && ib[0] != 0xef) ||
			   (len == 2 && (ib[0] != 0xef || ib[1] != 0xbb))) {
			cd->bom_written = true;
		}
	}


	while (ib < ibtail) {
		int i, l, h;
		unsigned long u8;
		signed char sz;


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

		if (sz == 1) {
			CHECK_OB(1);
			*ob++ = *ib++;
			continue;
		}

		/* sz > 1 */
		if ((ibtail - ib) < sz)
			ERR_INT(EINVAL);

		u8 = *ib;
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
			u8 = (u8 << 8) | ((unsigned int)*(ib+i));
		}

		if ((u8 & ICV_UTF8_REPRESENTATION_ffff_mask) ==
		    ICV_UTF8_REPRESENTATION_fffe ||
		    (u8 & ICV_UTF8_REPRESENTATION_ffff_mask) ==
		    ICV_UTF8_REPRESENTATION_ffff ||
		    u8 > ICV_UTF8_REPRESENTATION_10fffd ||
		    (u8 >= ICV_UTF8_REPRESENTATION_d800 &&
		    u8 <= ICV_UTF8_REPRESENTATION_dfff) ||
		    (u8 >= ICV_UTF8_REPRESENTATION_fdd0 &&
		    u8 <= ICV_UTF8_REPRESENTATION_fdef))
			goto ILLEGAL_CHAR;

		i = l = 0;
		h = (sizeof(u8_sb_tbl) /
		     sizeof(to_sb_table_component_t)) - 1;
		while (l <= h) {
			i = (l + h) / 2;
			if (u8_sb_tbl[i].u8 == u8)
				break;
			else if (u8_sb_tbl[i].u8 < u8)
				l = i + 1;
			else
				h = i - 1;
		}

		/*
		 * We just assume that either we found it or it is
		 * a non-identical character that we need to
		 * provide a replacement character or process
		 * according to flags.
		 */
		if (u8_sb_tbl[i].u8 == u8) {
			*ob++ = u8_sb_tbl[i].sb;
			ib += sz;
			continue;
		}

NON_IDENTICAL_CHAR:

		/*
		 * Handle NON_IDENTICAL
		 * Count non-identicals in return value
		 */
		ret_val++;

		if (f) {
			if (f & ICONV_CONV_NON_IDENTICAL_DISCARD) {
				ib += sz;
				continue;

			} else if (f & ICONV_CONV_NON_IDENTICAL_REPLACE_HEX) {
				CHECK_OB(sz * ICV_RHEX_LEN);
				for (l=0; l < sz; l++) {
					PUT_RHEX(*(ib+l), ob, NI);
				}
				ib += sz;
				continue;

			}
		}

		/* Default scenario */
		*ob++ = ICV_CHAR_ASCII_REPLACEMENT;
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
				CHECK_OB(1);
				*ob++ = ICV_CHAR_ASCII_REPLACEMENT;
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


int _icv_iconvctl(STATE_T *cd, int req, void *arg)
{
	return _icv_flag_action(&cd->flags, req, (int *)arg,
	    ICONVCTL_NO_TRANSLIT);
}

