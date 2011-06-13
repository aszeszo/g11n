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
#include "japanese.h"
#include "jfp_iconv_common.h"

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *st;

	if ((st = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		st->_st_cset = CS_0;
		st->replacement = JGETA; /* default is JIS GETA */
	}

	return ((iconv_t)st);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t *st;
	int cset;
	unsigned char *ip, ic, ic2, ic3;
	char *op;
	size_t ileft, oleft;
	size_t retval;

	int stat; /* not used, but it's needed for UNGET_ERRRET() */

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
				if (oleft < SEQ_SBTOG0) {
					errno = E2BIG;
					return ((size_t)-1);
				}
				PUT(ESC);
				PUT(SBTOG0_1);
				PUT(F_X0201_RM);
				*outbuf = (char *)op;
				*outbytesleft = oleft;
			}
			st->_st_cset = CS_0;
		}
		return ((size_t)0);
	}

	cset = st->_st_cset;

	ip = (unsigned char *)*inbuf;
	op = *outbuf;
	ileft = *inbytesleft;
	oleft = *outbytesleft;

	/*
	 * Main loop; basically 1 loop per 1 output char
	 */
	while ((int)ileft > 0) {
		st->_st_cset = cset;
		GET(ic);
		if (ISASC((int)ic)) { /* ASCII */
			RESTORE_HEX_ASCII_CONTINUE(ic)
			if (cset != CS_0) {
				if (oleft < SEQ_SBTOG0) {
					UNGET();
					errno = E2BIG;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
				PUT(ESC);	/* Kanji Out */
				PUT(SBTOG0_1);
				PUT(F_X0201_RM);
			}
			cset = CS_0;
			if (oleft < JISW0) {
				UNGET();
				errno = E2BIG;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
			/* Put ASCII character */
			PUT(ic);
			continue;
		} else if (ISCS1((int)ic)) { /* CS_1 starts */
			if ((int)ileft > 0) {
				GET(ic2); /* get 2nd byte */
				if (ISCS1(ic) && ISCS1(ic2)) {
					if (cset != CS_1) {
						if (oleft < SEQ_MBTOG0_O) {
							UNGET();
							UNGET();
							errno = E2BIG;
							retval =
							(size_t)ERR_RETURN;
							goto ret;
						}
						cset = CS_1;
						PUT(ESC);
						PUT(MBTOG0_1);
						PUT(F_X0208_83_90);
					}
					if (oleft < JISW1) {
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
#ifdef  RFC1468_MODE /* Convert VDC and UDC to GETA */
					if ((ic == 0xad) || (0xf5 <= ic )){
						PUT((JGETA >> 8) & CMASK);
						PUT(JGETA & CMASK);
						continue;
					}
#endif  /* RFC1468_MODE */
					/* Put JIS X 0208 character */
					PUT(ic & CMASK);
					PUT(ic2 & CMASK);
					continue;
				} else { /* 2nd byte check failed */
					UNGET_ERRRET(2, EILSEQ)
				}
			} else {		/* input fragment of Kanji */
				UNGET_ERRRET(1, EINVAL)
			}
		} else if (ic == SS2) {	/* Kana starts */
			if ((int)ileft > 0) {
				GET(ic2);
				if (ISCS2(ic2)) {

#ifdef  RFC1468_MODE	/* Substitute JIS X 0208 for JIS X 0201 Katakana */
        			unsigned short  zenkaku;
					if (cset != CS_1) {
						if (oleft < SEQ_MBTOG0_O) {
							UNGET();
							UNGET();
							errno = E2BIG;
							retval = 
							(size_t)ERR_RETURN;
							goto ret;
						}
						PUT(ESC);
						PUT(MBTOG0_1);
						PUT(F_X0208_83_90);
						cset = CS_1;
					}
					if (oleft < JISW1) {
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					zenkaku = halfkana2zenkakuj[ic - 0xA1];
					ic2 = (unsigned char)((zenkaku >> 8) & 
					CMASK);
					PUT(ic2);
					ic = (unsigned char)(zenkaku & CMASK);
					PUT(ic2);
#else   /* ISO-2022-JP.UIOSF */
					if (cset != CS_2) {
						if (oleft < SEQ_SBTOG0) {
							UNGET();
							UNGET();
							errno = E2BIG;
							retval =
							(size_t)ERR_RETURN;
							goto ret;
						}
						PUT(ESC);
						PUT(SBTOG0_1);
						PUT(F_X0201_KN);
						cset = CS_2;
					}
					if (oleft < JISW2) {
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					/* Put Kana character */
					PUT(ic2 & CMASK);
#endif  /* RFC1468_MODE */
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_ERRRET(2, EILSEQ)
				}
			} else {		/* input fragment of Kana */
				UNGET_ERRRET(1, EINVAL)
			}
		} else if (ic == SS3) {	/* JISX0212 starts */
			if (ileft >= EUCW3) {
				GET(ic2);
				GET(ic3);
				if (ISCS3(ic2) && ISCS3(ic3)) {

/* ISO-2022-JP.RFC1468 or ISO-2022-JP.UIOSF */
#ifdef  RFC1468_MODE	/* Substitute JIS X 0208 "Geta" for JIS X 0212 */
					if (cset != CS_1) {
						if (oleft < SEQ_MBTOG0_O) {
							UNGET();
							UNGET();
							UNGET();
							errno = E2BIG;
							retval = 
							(size_t)ERR_RETURN;
							goto ret;
						}
						cset = CS_1;
						PUT(ESC);
						PUT(MBTOG0_1);
						PUT(F_X0208_83_90);
					}
					if (oleft < JISW1) {
						UNGET();
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					/* Put GETA (0x222e) */
					ic = (unsigned char)((JGETA >> 8) & 
					CMASK);
					PUT(ic);
					ic = (unsigned char)(JGETA & CMASK);
					PUT(ic);
#else   /* ISO-2022-JP.UIOSF */
					if (cset != CS_3) {
						if (oleft < SEQ_MBTOG0) {
							UNGET();
							UNGET();
							UNGET();
							errno = E2BIG;
							retval =
							(size_t)ERR_RETURN;
							goto ret;
						}
						cset = CS_3;
						PUT(ESC);
						PUT(MBTOG0_1);
						PUT(MBTOG0_2);
						PUT(F_X0212_90);
					}
					if (oleft < JISW3) {
						UNGET();
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					/* Put JIS X 0212 character */
					PUT(ic2 & CMASK);
					PUT(ic3 & CMASK);
#endif  /* RFC1468_MODE */
					continue;
				} else { /* 2nd and 3rd byte check failed */
					UNGET_ERRRET(3, EILSEQ)
				}
			} else {	/* input fragment of JISX0212 */
				UNGET_ERRRET(1, EINVAL)
			}
		} else { /* 1st byte check failed */
			UNGET_ERRRET(1, EILSEQ)
		}
	}
	retval = ileft;
ret:
	*inbuf = (char *)ip;
	*inbytesleft = ileft;
	*outbuf = op;
	*outbytesleft = oleft;
	st->_st_cset = cset;

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
