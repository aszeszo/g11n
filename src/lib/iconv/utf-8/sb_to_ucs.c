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
 * In this program, we assume that each table entry provided will contain
 * a valid UCS character, an illegal character, or, a replacement character.
 * In other words, it is table provider's responsibility to provide
 * an appropriate mapping for each single byte character in the table since
 * the program in this file will not do any special checking on the table
 * component values.
 *
 * This particular file is to cover conversions from various single byte
 * codesets to UCS-2, UCS-2BE, UCS-2LE, UCS-4, UCS-4BE, UCS-4LE, UTF-16,
 * UTF-16BE, UTF-16LE, UTF-32, UTF-32BE, and UTF-32LE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/isa_defs.h>
#include "sb_to_ucs.h"


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
	unsigned char *ib;
	unsigned char *ob;
	unsigned char *ibtail;
	unsigned char *obtail;
	int f;


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
#endif
		return ((size_t)0);
	}

	ib = (unsigned char *)*inbuf;
	ob = (unsigned char *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;
	f = cd->flags;

	while (ib < ibtail) {
		int i;
		unsigned int u4;
		unsigned int u4_2;
		signed char obsz;

		u4 = sb_u4_tbl[*ib].u8;
		u4_2 = 0;

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

		if (sb_u4_tbl[*ib].size == ICV_TYPE_ILLEGAL_CHAR)
			goto ILLEGAL_CHAR;

		obsz = (cd->bom_written) ? ICV_FETCH_UCS_SIZE :
			ICV_FETCH_UCS_SIZE_TWO;

/* This doesn't occur in the tables:

#if defined(UCS_2) || defined(UCS_2BE) || defined(UCS_2LE) || \
	defined(UCS_2_BIG_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN)
		if (u4 > 0x00ffff) {
			u4 = ICV_CHAR_UCS2_REPLACEMENT;
			ret_val++;
			goto NON_IDENTICAL_CHAR;
		}
#elif defined(UTF_16) || defined(UTF_16BE) || defined(UTF_16LE) || \
       defined(UTF_16_BIG_ENDIAN) || defined(UTF_16_LITTLE_ENDIAN)
		if (u4 > 0x00ffff && u4 < 0x110000) {
			u4_2 = ((u4 - 0x010000) % 0x400) + 0x00dc00;
			u4   = ((u4 - 0x010000) / 0x400) + 0x00d800;
			obsz += 2;
		} else if (u4 > 0x10ffff) {
			u4 = ICV_CHAR_UCS2_REPLACEMENT;
			ret_val++;
			goto NON_IDENTICAL_CHAR;
		}
#elif defined(UTF_32) || defined(UTF_32BE) || defined(UTF_32LE) || \
       defined(UTF_32_BIG_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
		if (u4 > 0x10ffff) {
			u4 = ICV_CHAR_UCS2_REPLACEMENT;
			ret_val++;
			goto NON_IDENTICAL_CHAR;
		}
#else
#error "Fatal: one of the UCS macros need to be defined."
#endif
 */

		/*
		 * The target values in the conversion tables are in UCS-4
		 * without BOM and so the max target value possible would be
		 * U+7FFFFFFF.
		 */
		if (u4 == 0x00fffe || u4 == 0x00ffff || u4 > 0x7fffffff ||
		    (u4 >= 0x00d800 && u4 <= 0x00dfff)) {
			/*
			 * If conversion table is right, this should not
			 * happen.
			 */
			ERR_INT(EILSEQ);
		}

		CHECK_OB_AND_BOM(obsz, cd->bom_written);
		PUTC(u4);
		ib++;
		continue;

ILLEGAL_CHAR:

		/*
		 * Handle ILLEGAL and REPLACE_INVALID
		 */
		if (f) {
			if (f & ICONV_CONV_ILLEGAL_DISCARD) {
				ib++;
				continue;

			} else if (f & ICONV_CONV_ILLEGAL_REPLACE_HEX) {
				obsz = ICV_FETCH_UCS_SIZE * ICV_RHEX_LEN;
				if (! cd->bom_written)
					obsz += ICV_FETCH_UCS_SIZE;
				CHECK_OB_AND_BOM(obsz, cd->bom_written);
				PUT_RHEX(*ib, IL);
				ib++;
				continue;

			} else if (f & ICONV_REPLACE_INVALID) {
				obsz = (cd->bom_written) ? ICV_FETCH_UCS_SIZE :
					ICV_FETCH_UCS_SIZE_TWO;
				CHECK_OB_AND_BOM(obsz, cd->bom_written);
				PUTC(ICV_CHAR_UCS2_REPLACEMENT);
				ib++;
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
	return _icv_flag_action(&cd->flags, req, (int *)arg, 0);
}

size_t
_icv_iconvstr(char *inarray, size_t *inlen, char *outarray,
	size_t *outlen, int flags)
{
	return _icv_ciconvstr(inarray, inlen, outarray, outlen, flags, 1);
}

