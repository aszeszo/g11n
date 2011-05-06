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
 * Copyright (c) 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdlib.h>
#include <errno.h>
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_wchar.h"

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *cd;

	if ((cd = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		cd->replacement = PCKWGETA;
	}

	return ((iconv_t)cd);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	wchar_t		pckwchar;	/* PCK wchar */
	unsigned int	index;		/* index for table lookup */
	unsigned char	ic1, ic2;	/* 1st and 2nd bytes of a char */
	size_t		rv = (size_t)0;	/* return value of this function */
	unsigned char	oc;

	unsigned char	*ip;
	size_t		ileft, pre_ileft;
	char		*op;
	size_t		oleft;

	st = (__icv_state_t *)cd;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		return ((size_t)0);
	}

	ip = (unsigned char *)*inbuf;
	ileft = *inbytesleft;
	op = *outbuf;
	oleft = *outbytesleft;

	while (ileft != 0) {
		pre_ileft = ileft; /* value before reading input bytes */
		NGET(ic1, "never fail here"); /* get 1st byte */

		if (ISASC((int)ic1)) {	/* ASCII; 1 byte */
			RESTORE_HEX_ASCII_JUMP(ic1)
			pckwchar = __get_pckwchar(__ASCII, ic1, NULL);
			NPUT_WCHAR(pckwchar, "ASCII");
		} else if (ISSJKANA(ic1)) { /* JIS X 0201 Kana; 1 byte */
			pckwchar = __get_pckwchar(__PCK_KANA, ic1, NULL);
			NPUT_WCHAR(pckwchar, "KANA");
		} else if (ISSJMB_1(ic1)) { /* valid SJIS 1st byte */
			NGET(ic2, "CS1-2");
			/*
			 * mbtowc(3C) return -1 and set EILSEQ when
			 * the first byte is 0xeb or 0xec. But in
			 * iconv(3C), it's processed not EILSEQ, but
			 * non-identical conversion, because it's not
			 * a detection of illegal byte. The detected
			 * 0xeb or 0xec should not be discarded.
			 */
			if (ISSJKANJI2(ic2)) {
				if ((ic1 == 0xeb) || (ic1 == 0xec)) {
					if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
						CALL_NON_IDENTICAL()
					} else {
						NPUT((unsigned char)PCKWGETA, "replacement area");
					}
				} else {
					pckwchar = __get_pckwchar(__PCK_KANJI, ic1, ic2);
					NPUT_WCHAR(pckwchar, "KANJI");
				}
			} else { /* 2nd byte check failed */
				RET_EILSEQ("EILSEQ at CS1-2", 2)
			}
		} else { /* 1st byte check failed */
			RET_EILSEQ("EILSEQ at 1st", 1)
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
	 * Return value for successful return is not defined by XPG
	 * so return same as *inbytesleft as existing codes do.
	 */
	return ((rv == (size_t)-1) ? rv : *inbytesleft);
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
	return (__replace_hex_wchar(hex, pip, pop, poleft, cd, caller));
}

/* see jfp_iconv_common.h */
size_t __replace_invalid(
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	return (__replace_invalid_wchar(pop, poleft, cd));
}
