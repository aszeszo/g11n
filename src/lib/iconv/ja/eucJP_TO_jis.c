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

	int cset, stat;
	unsigned char *ip, ic, ic2, ic3;
	char *op;
	size_t ileft, oleft;
	size_t retval;

	st = (__icv_state_t *)cd;

	stat = ST_INIT;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		if ((st->_st_cset == CS_1) || (st->_st_cset == CS_3)) {
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
		} else if (st->_st_cset == CS_2) {
			if ((outbuf != NULL) && (*outbuf != NULL)
					&& (outbytesleft != NULL)) {
				op = (char *)*outbuf;
				oleft = *outbytesleft;
				if (oleft < SEQ_SOSI) {
					errno = E2BIG;
					return ((size_t)-1);
				}
				PUT(SI);
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
	 * Main loop; basically 1 loop per 1 input byte
	 */

	while ((int)ileft > 0) {
		st->_st_cset = cset;
		GET(ic);
		if (ISASC((int)ic)) { /* ASCII */
			RESTORE_HEX_ASCII_CONTINUE(ic)
			if ((cset == CS_1) || (cset == CS_3)) {
				if (oleft < SEQ_SBTOG0) {
					UNGET();
					errno = E2BIG;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
				PUT(ESC);	/* Kanji Out */
				PUT(SBTOG0_1);
				PUT(F_X0201_RM);
			} else if (cset == CS_2) {
				if (oleft < SEQ_SOSI) {
					UNGET();
					errno = E2BIG;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
				PUT(SI);	/* Shift In */
			}
			cset = CS_0;
			if (oleft < JISW0) {
				UNGET();
				errno = E2BIG;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
			PUT(ic);
			continue;
		} else if (ISCS1(ic)) {
			if ((int)ileft > 0) {	/* Kanj starts */
				GET(ic2); /*get 2nd byte */
				if (ISCS1(ic2)) {
					if (cset == CS_2) {
						if (oleft < SEQ_SOSI) {
							UNGET();
							UNGET();
							errno = E2BIG;
							retval =
							(size_t)ERR_RETURN;
							goto ret;
						}
						cset = CS_0;
						PUT(SI);
					}
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
					PUT(ic & CMASK);
					PUT(ic2 & CMASK);
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_ERRRET(2, EILSEQ)
				}
			} else {		/* input fragment of Kanji */
				UNGET_ERRRET(1, EINVAL)
			}
		} else if (ic == SS2) {	/* Kana starts */
			if ((int)ileft > 0) {
				GET(ic2); /* get 2nd byte */
				if (ISCS2(ic2)) {
					if ((cset == CS_1) || (cset == CS_3)) {
						if (oleft < SEQ_SBTOG0) {
							UNGET();
							UNGET();
							errno = E2BIG;
							retval =
							(size_t)ERR_RETURN;
							goto ret;
						}
						cset = CS_0;
						PUT(ESC);
						PUT(SBTOG0_1);
						PUT(F_X0201_RM);
					}
					if (cset != CS_2) {
						if (oleft < SEQ_SOSI) {
							UNGET();
							UNGET();
							errno = E2BIG;
							retval =
							(size_t)ERR_RETURN;
							goto ret;
						}
						cset = CS_2;
						PUT(SO);
					}
					if (oleft < JISW2) {
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					PUT(ic2 & CMASK);
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_ERRRET(2, EILSEQ)
				}
			} else {		/* input fragment of Kana */
				UNGET_ERRRET(1, EINVAL)
			}
		} else if (ic == SS3) {	/* JISX0212 starts */
			if (ileft >= EUCW3) {
				GET(ic2); /* get 2nd byte */
				GET(ic3); /* get 3rd byte */
				if (ISCS3(ic2) && ISCS3(ic3)) {
					if (cset == CS_2) {
						if (oleft < SEQ_SOSI) {
							UNGET();
							UNGET();
							UNGET();
							errno = E2BIG;
							retval =
							(size_t)ERR_RETURN;
							goto ret;
						}
						cset = CS_0;
						PUT(SI);
					}
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
					PUT(ic2 & CMASK);
					PUT(ic3 & CMASK);
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
	/*
	 * When successfully converted, return number of non-identical
	 * conversion as described in iconv(3C) and iconvstr(3C)
	 */
	retval = st->num_of_ni;
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
