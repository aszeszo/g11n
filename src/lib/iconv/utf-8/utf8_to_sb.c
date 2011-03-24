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
#include <errno.h>
#include <sys/types.h>
#include "utf8_to_sb.h"


void *
_icv_open()
{
	ucs_state_t *cd = (ucs_state_t *)calloc(1, sizeof(ucs_state_t));
	if (cd == (ucs_state_t *)NULL) {
		errno = ENOMEM;
		return((void *)-1);
	}

	return((void *)cd);
}


void
_icv_close(ucs_state_t *cd)
{
	if (! cd)
		errno = EBADF;
	else
		free((void *)cd);
}


size_t
_icv_iconv(ucs_state_t *cd, char **inbuf, size_t *inbufleft, char **outbuf,
                size_t *outbufleft)
{
	size_t ret_val = 0;
	unsigned char *ib;
	unsigned char *ob;
	unsigned char *ibtail;
	unsigned char *obtail;
       	register int i, l, h;
       	signed char sz;
       	unsigned long u8;
	size_t len;

	if (! cd) {
		errno = EBADF;
		return((size_t)-1);
	}

	if (!inbuf || !(*inbuf)) {
		cd->bom_written = false;
		return((size_t)0);
	}

	ib = (unsigned char *)*inbuf;
	ob = (unsigned char *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;

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
		sz = number_of_bytes_in_utf8_char[*ib];
		if (sz == ICV_TYPE_ILLEGAL_CHAR) {
			errno = EILSEQ;
			ret_val = (size_t)-1;
			break;
		}

		if (ob >= obtail) {
			errno = E2BIG;
			ret_val = (size_t)-1;
			break;
		}

		if (sz == 1) {
			*ob++ = *ib++;
		} else {
			if ((ibtail - ib) < sz) {
				errno = EINVAL;
				ret_val = (size_t)-1;
				break;
			}

			u8 = *ib++;
			for (i = 1; i < sz; i++) {
				if (i == 1) {
					if (((uchar_t)*ib) <
						valid_min_2nd_byte[u8] ||
					    ((uchar_t)*ib) >
						valid_max_2nd_byte[u8]) {
						ib--;
						errno = EILSEQ;
						ret_val = (size_t)-1;
						goto illegal_char_err;
					}
				} else if (((uint_t)*ib) < 0x80 ||
					   ((uint_t)*ib) > 0xbf) {
					ib -= i;
					errno = EILSEQ;
					ret_val = (size_t)-1;
					goto illegal_char_err;
				}
				u8 = (u8 << 8) | ((unsigned int)*ib);
				ib++;
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
				ib -= sz;
				errno = EILSEQ;
				ret_val = (size_t)-1;
			       	goto illegal_char_err;
			}

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
			 * provide a replacement character.
			 */
			if (u8_sb_tbl[i].u8 == u8) {
				*ob++ = u8_sb_tbl[i].sb;
			} else {
				*ob++ = ICV_CHAR_ASCII_REPLACEMENT;
				ret_val++;
			}
		}
	}

illegal_char_err:
	*inbuf = (char *)ib;
	*inbufleft = ibtail - ib;
	*outbuf = (char *)ob;
	*outbufleft = obtail - ob;

	return(ret_val);
}
