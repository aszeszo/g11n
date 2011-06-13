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
 * COPYRIGHT AND PERMISSION NOTICE
 *
 * Copyright (c) 1991-2005 Unicode, Inc. All rights reserved. Distributed
 * under the Terms of Use in http://www.unicode.org/copyright.html.
 *
 * This file has been modified by Oracle and/or its affiliates.
 */
/*
 * Copyright (c) 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdlib.h>
#include <errno.h>
#include <euc.h>
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_wchar.h"
#include "jfp_iconv_unicode_enhance.h"

#define	JFP_J2U_ICONV
#include "jfp_jis_to_ucs2.h"

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *cd;

	if ((cd = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		_icv_reset_unicode((void *)cd);
	}

	return ((iconv_t)cd);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	unsigned short	pckval;		/* PCK value */
	unsigned int	uni;		/* UTF-32 */
	unsigned int	index;		/* index for table lookup */
	unsigned char	ic1, ic2;	/* 1st and 2nd bytes of a char */
	size_t		rv = (size_t)0;	/* return value of this function */

	unsigned char	*ip;
	size_t		ileft;
	char		*op;
	size_t		oleft;

	st = (__icv_state_t *)cd;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		_icv_reset_unicode(st);
		return ((size_t)0);
	}

	ip = (unsigned char *)*inbuf;
	ileft = *inbytesleft;
	op = *outbuf;
	oleft = *outbytesleft;

	while (ileft != 0) {

		GET_WCHAR(__read_pckwchar, &pckval);

		if (ISASC(pckval)) {
			ic1 = (unsigned char)pckval;
			uni = _jfp_tbl_jisx0201roman_to_ucs2[ic1];
			PUTU(uni, "ASCII", 1);
			goto next;
		} else if (ISSJKANA(pckval)) {
			ic1 = (unsigned char)pckval;
			uni = _jfp_tbl_jisx0201kana_to_ucs2[(ic1 - 0xa1)];
			PUTU(uni, "KANA", 1);
			goto next;
		} else {
			ic1 = (unsigned char)(pckval >> 8);
			ic2 = (unsigned char)(pckval & 0x00ff);
		}

		if (ISSJKANJI1(ic1)) { /* JIS X 0208 or UDC; 2 bytes */
			ic1 = sjtojis1[(ic1 - 0x80)];
			if (ic2 >= 0x9f) {
				ic1++;
			}
			index = ((ic1 - 0x21) * 94)
				+ (sjtojis2[ic2] - 0x21);
			uni = _jfp_tbl_jisx0208_to_ucs2[index];
			PUTU(uni, "KANJI", 2);
		} else if (ISSJSUPKANJI1(ic1)) { /* VDC, 2 bytes */
			ic1 = sjtojis1[(ic1 - 0x80)];
			if (ic2 >= 0x9f) {
				ic1++;
			}
			index = ((ic1 - 0x21) * 94)
					+ (sjtojis2[ic2] - 0x21);
			uni = _jfp_tbl_jisx0212_to_ucs2[index];
			PUTU(uni, "SUPKANJI", 2);
		} else if (ISSJIBM(ic1) || /* Extended IBM char. area */
			ISSJNECIBM(ic1)) { /* NEC/IBM char. area */
		/*
		 * We need a special treatment for each codes.
		 * By adding some offset number for them, we
		 * can process them as the same way of that of
		 * extended IBM chars.
		 */
			unsigned short dest, upper, lower;
			dest = (ic1 << 8) + ic2;
			if ((0xed40 <= dest) && (dest <= 0xeffc)) {
				REMAP_NEC(dest);
				if (dest == 0xffff) {
					RET_EILSEQ("invalid NEC", 4)
				}
			}
			/*
			 * XXX: 0xfa54 and 0xfa5b must be mapped
			 *	to JIS0208 area. Therefore we
			 *	have to do special treatment.
			 */
			if ((dest == 0xfa54) || (dest == 0xfa5b)) {
				if (dest == 0xfa54) {
			/* map to JIS X 0208 row 2 cell 44 "NOT SIGN" */
			index = (2 - 1) * 94 + (44 - 1);
				} else {
			/* map to JIS X 0208 row 2 cell 72 "BECAUSE" */
			index = (2 - 1) * 94 + (72 - 1);
				}
				uni = _jfp_tbl_jisx0208_to_ucs2[index];
				PUTU(uni, "IBM", 2);
			} else {
				dest = dest - 0xfa40 -
					(((dest>>8) - 0xfa) * 0x40);
				dest = sjtoibmext[dest];
				if (dest == 0xffff) {
					RET_EILSEQ("invalid IBM", 4)
				}
				upper = ((dest >> 8) & 0x7f) - 0x21;
				lower = (dest & 0x7f) - 0x21;
				index = (unsigned int)(upper * 94 + 
					lower);
				uni = _jfp_tbl_jisx0212_to_ucs2[index];
				PUTU(uni, "IBM", 2);
			}

		} else if ((0xeb <= ic1) && (ic1 <= 0xec)) {
		/*
		 * Based on the draft convention of OSF-JVC CDEWG,
		 * characters in this area will be mapped to
		 * "CHIKAN-MOJI." (convertible character)
		 * We use U+FFFD in this case.
		 */
			uni = 0xfffd;
			PUTU(uni, "GAP", 2);
		}
next:
		/*
		 * One character successfully converted so update
		 * values outside of this function's stack.
		 */
		*inbuf = (char *)ip;
		*inbytesleft = ileft;
		*outbuf = op;
		*outbytesleft = oleft;
	}

ret:
	DEBUGPRINTERROR

	/*
	 * When successfully converted, return number of non-identical
	 * conversion as described in iconv(3C) and iconvstr(3C)
	 */
	return ((rv == (size_t)-1) ? rv : st->num_of_ni);
}

/* see jfp_iconv_common.h */
size_t __replace_hex(
	unsigned char	hex,
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd,
	int		caller)
{
	return (__replace_hex_utf32(hex, pip, pop, poleft, cd, caller));
}

/* see jfp_iconv_common.h */
size_t __replace_invalid(
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	return (__replace_invalid_utf32(pip, pop, poleft, cd));
}
