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
 * This is for conversions from UTF-8 to various UCS forms, esp.,
 * UCS-2, UCS-2BE, UCS-2LE, UTF-16, UTF-16BE, UTF-16LE, UCS-4, UCS-4BE,
 * UCS-4LE, UTF-32, UTF-32BE, and UTF-32LE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/isa_defs.h>
#include "utf8_to_ucs.h"


void *
_icv_open_attr(int flag, void *reserved)
{
	STATE_T *cd = (STATE_T *)calloc(1, sizeof(STATE_T));

	if (cd == (STATE_T *)NULL) {
		errno = ENOMEM;
		return ((void *)-1);
	}
	cd->flags = flag;

#if defined(UTF_16_BIG_ENDIAN) || defined(UCS_2_BIG_ENDIAN) || \
	defined(UCS_4_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN)
	cd->little_endian = false;
#elif defined(UTF_16_LITTLE_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN) || \
	defined(UCS_4_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
	cd->little_endian = true;
#elif defined(UTF_16BE) || defined(UCS_2BE) || defined(UCS_4BE) || \
	defined(UTF_32BE)
	cd->little_endian = false;
	cd->bom_written = true;
#elif defined(UTF_16LE) || defined(UCS_2LE) || defined(UCS_4LE) || \
	defined(UTF_32LE)
	cd->little_endian = true;
	cd->bom_written = true;
#elif defined(_LITTLE_ENDIAN)
	cd->little_endian = true;
#endif

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
	size_t len;


	if (! cd) {
		errno = EBADF;
		return ((size_t)-1);
	}

	if (!inbuf || !(*inbuf)) {
#if defined(UCS_2) || defined(UCS_4) || defined(UTF_16) || defined(UTF_32) || \
	defined(UCS_2_BIG_ENDIAN) || defined(UCS_4_BIG_ENDIAN) || \
	defined(UTF_16_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN) || \
	defined(UCS_2_LITTLE_ENDIAN) || defined(UCS_4_LITTLE_ENDIAN) || \
	defined(UTF_16_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
		cd->bom_written = false;
		cd->bom_processed = false;
#endif
		return ((size_t)0);
	}

	ib = (uchar_t *)*inbuf;
	ob = (uchar_t *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;
	f = cd->flags;

	/* We skip the first signature of UTF-8 BOM if any. */
	if (! cd->bom_processed) {
		len = ibtail - ib;
		if (len >= ICV_FETCH_UTF8_BOM_SIZE) {
			if (ib[0] == 0xef && ib[1] == 0xbb && ib[2] == 0xbf) {
				ib += ICV_FETCH_UTF8_BOM_SIZE;
			}
			cd->bom_processed = true;
		} else if ((len == 1 && ib[0] != 0xef) ||
			   (len == 2 && (ib[0] != 0xef || ib[1] != 0xbb))) {
			cd->bom_processed = true;
		}
	}


	while (ib < ibtail) {
		uint_t u4;
		uint_t u4_2 = 0;
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

				obsz = (cd->bom_written) ? 1 :
				    ICV_FETCH_UCS_SIZE + 1;
				CHECK_OB_AND_BOM(obsz, cd->bom_written);

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

		if (u4 == ICV_BOM_IN_BIG_ENDIAN)
			cd->bom_written = true;

#if defined(UCS_4) || defined(UCS_4BE) || defined(UCS_4LE) || \
	defined(UCS_4_BIG_ENDIAN) || defined(UCS_4_LITTLE_ENDIAN)
		obsz = (cd->bom_written) ? 4 : 8;
#elif defined(UTF_32) || defined(UTF_32BE) || defined(UTF_32LE) || \
	defined(UTF_32_BIG_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
		obsz = (cd->bom_written) ? 4 : 8;
		if (u4 > 0x10ffff)
			goto NON_IDENTICAL_CHAR;
#elif defined(UCS_2) || defined(UCS_2BE) || defined(UCS_2LE) || \
	defined(UCS_2_BIG_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN)
		obsz = (cd->bom_written) ? 2 : 4;
		if (u4 > 0x00ffff)
			goto NON_IDENTICAL_CHAR;
#elif defined(UTF_16) || defined(UTF_16BE) || defined(UTF_16LE) || \
	defined(UTF_16_BIG_ENDIAN) || defined(UTF_16_LITTLE_ENDIAN)
		obsz = (cd->bom_written) ? 2 : 4;
		if (u4 > 0x10ffff) {
			goto NON_IDENTICAL_CHAR;
		} else if (u4 > 0x00ffff) {
			u4_2 = ((u4 - 0x010000) % 0x400) + 0x00dc00;
			u4   = ((u4 - 0x010000) / 0x400) + 0x00d800;
			obsz += 2;
		}
#else
#error	"Fatal: one of the UCS macros need to be defined."
#endif

		CHECK_OB_AND_BOM(obsz, cd->bom_written);
		PUTC(u4);
		ib += sz;
		continue;

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
				int l;

				obsz = ICV_FETCH_UCS_SIZE * ICV_RHEX_LEN;
				if (! cd->bom_written)
					obsz += ICV_FETCH_UCS_SIZE;
				CHECK_OB_AND_BOM(obsz, cd->bom_written);
				for (l=0; l < sz; l++) {
					PUT_RHEX(*(ib+l), NI);
				}
				ib += sz;
				continue;
			}
		}

		/* Default scenario */
		obsz = (cd->bom_written) ? ICV_FETCH_UCS_SIZE
		    : ICV_FETCH_UCS_SIZE_TWO;
		CHECK_OB_AND_BOM(obsz, cd->bom_written);
		PUTC(ICV_CHAR_UCS2_REPLACEMENT);
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

				obsz = ICV_FETCH_UCS_SIZE * ICV_RHEX_LEN;
				if (! cd->bom_written)
					obsz += ICV_FETCH_UCS_SIZE;
				CHECK_OB_AND_BOM(obsz, cd->bom_written);
				for (l=0; l < sz ; l++) {
					PUT_RHEX(*(ib+l), IL);
				}
				ib += sz;
				continue;

			} else if (f & ICONV_REPLACE_INVALID) {
INCOMPLETE_CHAR:
				u4_2 = 0;
				obsz = (cd->bom_written) ? ICV_FETCH_UCS_SIZE
				    : ICV_FETCH_UCS_SIZE_TWO;
				CHECK_OB_AND_BOM(obsz, cd->bom_written);
				PUTC(ICV_CHAR_UCS2_REPLACEMENT);
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

size_t
_icv_iconvstr(char *inarray, size_t *inlen, char *outarray,
	size_t *outlen, int flags)
{
	return _icv_ciconvstr(inarray, inlen, outarray, outlen, flags, 1);
}
