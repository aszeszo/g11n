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
 * Copyright (c) 1997, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_unicode_enhance.h"

#ifdef RFC1468_MODE
#define	JFP_U2E_ICONV_RFC1468
#else
#define	JFP_U2E_ICONV
#endif
#include "jfp_ucs2_to_euc16.h"

#define	DEF_SINGLE	'?'

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *st;

	if ((st = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		st->_st_cset = CS_0;
		st->replacement = DEF_SINGLE;
	}

	return ((iconv_t)st);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	unsigned char	ic;
	size_t		rv = (size_t)0;
	unsigned int	ucs4;
	unsigned short	euc16;

	int		cset;

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
		if (st->_st_cset != CS_0) {
			if ((outbuf != NULL) && (*outbuf != NULL)
					&& (outbytesleft != NULL)) {
				op = (char *)*outbuf;
				oleft = *outbytesleft;
				NPUT(ESC, "RESET-SEQ-ESC");
				NPUT(SBTOG0_1, "RESET-SEQ-1");
				NPUT(F_X0201_RM, "RESET-SEQ-2");
				*outbuf = (char *)op;
				*outbytesleft = oleft;
			}
			st->_st_cset = CS_0;
		}
		return ((size_t)0);
	}

	cset = st->_st_cset;

	ip = (unsigned char *)*inbuf;
	ileft = *inbytesleft;
	op = *outbuf;
	oleft = *outbytesleft;

	while (ileft != 0) {
		pre_ileft = ileft; /* value before reading input bytes */
		errno = 0;
		if (utf8_ucs(&ucs4, &ip, &ileft,
				&op, &oleft, st) == (size_t)-1) {
			/* errno has been set in utf8_ucs() */
			rv = (size_t)-1;
			goto ret;
		}
		/*
		 * When illegal byte is detected and __ICONV_CONV_ILLEGAL,
		 * utf8_ucs return with sucess and EILSEQ is set in
		 * errno. Detected illegal bytes have been processed
		 * already. It should go to the next loop.
		 * The above "errno = 0;" is required for here.
		 */
		if ((errno == EILSEQ) && 
			(st->_icv_flag & __ICONV_CONV_ILLEGAL)) {

			/* mode is ascii when illegal byte was replaced */
			if (st->_icv_flag &
			(__ICONV_CONV_REPLACE_HEX|ICONV_REPLACE_INVALID)) {
				cset = CS_0;
			}

			goto next;
		}

		if (ucs4 > 0xffff) {
			if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
				CALL_NON_IDENTICAL()
				if(st->_icv_flag & __ICONV_CONV_REPLACE_HEX) {
					cset = CS_0;
				}
			} else {
				/* non-BMP */
				if (cset != CS_0) {
					NPUT(ESC, "CS0-SEQ-ESC");
					NPUT(SBTOG0_1, "CS0-SEQ-1");
					NPUT(F_X0201_RM, "CS0-SEQ-2");
					cset = CS_0;
				}
				ic = (unsigned char)DEF_SINGLE;
				NPUT(ic, "DEF for non-BMP(replaced)");
			}
		} else {
			euc16 = _jfp_ucs2_to_euc16((unsigned short)ucs4);

			if(euc16 == 0xffff) {
				if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL)
				{
					CALL_NON_IDENTICAL()
					if(st->_icv_flag & __ICONV_CONV_REPLACE_HEX) {
						cset = CS_0;
					}
					goto next;
				} else {
					euc16 = DEF_SINGLE; /* replacement char */
				}
			}

			switch (euc16 & 0x8080) {
			case 0x0000:	/* CS0 */
				if (cset != CS_0) {
					NPUT(ESC, "CS0-SEQ-ESC");
					NPUT(SBTOG0_1, "CS0-SEQ-1");
					NPUT(F_X0201_RM, "CS0-SEQ-2");
					cset = CS_0;
				}
				ic = (unsigned char)euc16;
				NPUT(ic, "CS0-1");
				break;
			case 0x8080:	/* CS1 */
				if (cset != CS_1) {
					NPUT(ESC, "CS1-SEQ-ESC");
					NPUT(MBTOG0_1, "CS1-SEQ-1");
					NPUT(F_X0208_83_90, "CS1-SEQ-2");
					cset = CS_1;
				}
				ic = (unsigned char)((euc16 >> 8) & CMASK);
				NPUT(ic, "CS1-1");
				ic = (unsigned char)(euc16 & CMASK);
				NPUT(ic, "CS1-2");
				break;
			case 0x0080:	/* CS2 */
#ifdef  RFC1468_MODE	/* Substitute JIS X 0208 for JIS X 0201 Katakana */
				if (cset != CS_1) {
					NPUT(ESC, "CS2-SEQ-ESC(fullsized)");
					NPUT(MBTOG0_1, "CS2-SEQ-1(fullsized)");
					NPUT(F_X0208_83_90,
						"CS2-SEQ-2(fullsized)");
					cset = CS_1;
				}
				euc16 = halfkana2zenkakuj[euc16 - 0xa1];
				ic = (unsigned char)((euc16 >> 8) & CMASK);
				NPUT(ic, "CS2-1(fullsized)");
				ic = (unsigned char)(euc16 & CMASK);
				NPUT(ic, "CS2-2(fullsized)");
#else   /* ISO-2022-JP.UIOSF */
				if (cset != CS_2) {
					NPUT(ESC, "CS2-SEQ-ESC");
					NPUT(SBTOG0_1, "CS2-SEQ-1");
					NPUT(F_X0201_KN, "CS2-SEQ-2");
					cset = CS_2;
				}
				ic = (unsigned char)euc16;
				NPUT(ic & CMASK, "CS2-1");
#endif  /* RFC1468_MODE */
				break;
			case 0x8000:	/* CS3 */
				if (cset != CS_3) {
					NPUT(ESC, "CS3-SEQ-ESC");
					NPUT(MBTOG0_1, "CS3-SEQ-1");
					NPUT(MBTOG0_2, "CS3-SEQ-2");
					NPUT(F_X0212_90, "CS3-SEQ-3");
					cset = CS_3;
				}
				ic = (unsigned char)((euc16 >> 8) & CMASK);
				NPUT(ic, "CS3-1");
				ic = (unsigned char)(euc16 & CMASK);
				NPUT(ic, "CS3-2");
				break;
			}
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

		st->_st_cset = cset;
	}

ret:

#if	defined(DEBUG)
	if (rv == (size_t)-1) {
		fprintf(stderr, "DEBUG: errno=%d: %s\n", errno, debugmsg);
	}
#endif	/* DEBUG */

	/*
	 * Return value for successful return is not defined by XPG
	 * so return same as *inbytesleft as existing codes do.
	 */
	return ((rv == (size_t)-1) ? rv : *inbytesleft);
}

/* see jfp_iconv_common.h */
size_t
__replace_hex(
	unsigned char	hex,
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd,
	int		caller)
{
	return (__replace_hex_iso2022jp(hex, pip, pop, poleft, cd, caller));
}

/* see jfp_iconv_common.h */
size_t
__replace_invalid(
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	return (__replace_invalid_iso2022jp(pop, poleft, cd));
}
