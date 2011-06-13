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
 * Copyright (c) 1994, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>
#define	JFP_ICONV_STATELESS
#include "japanese.h"
#include "jfp_iconv_common.h"

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *cd;

	if ((cd = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		cd->replacement = EGETA; /* default is eucJP GETA */
	}

	return ((iconv_t)cd);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;
	int		stat, cset;
	unsigned char	*ip, ic, ic2, *op;
	size_t		ileft, oleft;
	size_t		retval;

	st = (__icv_state_t *)cd;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		/* nothing to do here for this module */
		return ((size_t)0);
	}

	stat = ST_INIT;

	ip = (unsigned char *)*inbuf;
	op = (unsigned char *)*outbuf;
	ileft = *inbytesleft;
	oleft = *outbytesleft;

	/*
	 * Main loop; basically 1 loop per 1 input byte
	 */

	while ((int)ileft > 0) {
		GET(ic);
		if (ISASC((int)ic)) {		/* ASCII */
			RESTORE_HEX_ASCII_CONTINUE(ic)
			CHECK2BIG(EUCW0,1);
			PUT(ic);
			continue;
		} else if (ISSJKANA(ic)) {		/* kana start */
			CHECK2BIG((SS2W + EUCW2),1);
			PUT(SS2);
			PUT(ic);
			continue;
		} else if (ISSJKANJI1(ic)) {	/* CS_1 kanji starts */
			if ((int)ileft > 0) {
				GET(ic2); /* get 2nd byte */
				if (ISSJKANJI2(ic2)) {
					CHECK2BIG(EUCW1,2);
					ic = sjtojis1[(ic - 0x80)];
					if (ic2 >= 0x9f) {
						ic++;
					}
					PUT(ic | CMSB);
					ic2 = sjtojis2[ic2];
					PUT(ic2 | CMSB);
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_ERRRET_STATELESS(2, EILSEQ)
				}
			} else {		/* input fragment of Kanji */
				UNGET_ERRRET_STATELESS(1, EINVAL)
			}
		} else if (ISSJSUPKANJI1(ic)) {	/* CS_3 kanji starts */
			if ((int)ileft > 0) {
				GET(ic2);
				if (ISSJKANJI2(ic2)) {
					CHECK2BIG((SS3W + EUCW3),2);
					ic = sjtojis1[(ic - 0x80)];
					if (ic2 >= 0x9f) {
						ic++;
					}
					PUT(SS3);
					PUT(ic | CMSB);
					ic2 = sjtojis2[ic2];
					PUT(ic2 | CMSB);
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_ERRRET_STATELESS(2, EILSEQ)
				}
			} else {		/* input fragment of Kanji */
				UNGET_ERRRET_STATELESS(1, EINVAL)
			}
		} else if (ISSJIBM(ic) || /* Extended IBM char. area */
			ISSJNECIBM(ic)) { /* NEC/IBM char. area */
			/*
			 * We need a special treatment for each codes.
			 * By adding some offset number for them, we
			 * can process them as the same way of that of
			 * extended IBM chars.
			 */
			if ((int)ileft > 0) {
				GET(ic2);
				if (ISSJKANJI2(ic2)) {
					unsigned short dest;
					dest = (ic << 8);
					dest += ic2;
					if ((0xed40 <= dest) &&
						(dest <= 0xeffc)) {
						REMAP_NEC(dest);
						if (dest == 0xffff) {
							goto ill_ibm;
						}
					}
					if ((dest == 0xfa54) ||
						(dest == 0xfa5b)) {
						CHECK2BIG(EUCW1,2);
						if (dest == 0xfa54) {
							PUT(0xa2);
							PUT(0xcc);
						} else {
							PUT(0xa2);
							PUT(0xe8);
						}
						continue;
					}
					CHECK2BIG((SS3W + EUCW3),2);
					dest = dest - 0xfa40 -
						(((dest>>8) - 0xfa) * 0x40);
					dest = sjtoibmext[dest];
					if (dest == 0xffff) {
						/*
						 * Illegal code points
						 * in IBM-EXT area.
						 */
ill_ibm:
						UNGET_ERRRET_STATELESS(2, EILSEQ)
					}
					PUT(SS3);
					PUT((dest>>8) & 0xff);
					PUT(dest & 0xff);
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_ERRRET_STATELESS(2, EILSEQ)
				}
			} else {		/* input fragment of Kanji */
				UNGET_ERRRET_STATELESS(1, EINVAL)
			}
		} else if ((0xeb <= ic) && (ic <= 0xec)) {
		/*
		 * Based on the draft convention of OSF-JVC CDEWG,
		 * characters in this area will be mapped to
		 * "CHIKAN-MOJI." (convertible character)
		 * So far, we'll use (0x222e) for it.
		 */
			if ((int)ileft > 0) {
				GET(ic2); /* get 2nd byte */
				if (ISSJKANJI2(ic2)) {
					if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
						CALL_NON_IDENTICAL_UNGET(2)
					} else {
						CHECK2BIG(EUCW1,2);
						PUT((EGETA>>8) & 0xff);
						PUT(EGETA & 0xff);
						st->num_of_ni++;
					}
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_ERRRET_STATELESS(2, EILSEQ)
				}
			} else {		/* input fragment of Kanji */
				UNGET_ERRRET_STATELESS(1, EINVAL)
			}
		} else {			/* 1st byte is illegal */
			UNGET_ERRRET_STATELESS(1, EILSEQ)
		}
	}
	/*
	 * When successfully converted, return number of non-identical
	 * conversion as described in iconv(3C) and iconvstr(3C)
	 */
	retval = st->num_of_ni;
ret:
	*inbuf = (char *)ip;
	*inbytesleft = ileft;
	*outbuf = (char *)op;
	*outbytesleft = oleft;

	return (retval);
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
	return (__replace_hex_ascii(hex, pip, pop, poleft, cd, caller));
}

/* see jfp_iconv_common.h */
size_t
__replace_invalid(
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	return (__replace_invalid_ascii(pop, poleft, cd));
}
