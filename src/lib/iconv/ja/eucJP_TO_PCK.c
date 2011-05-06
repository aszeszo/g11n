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

static unsigned short lookuptbl(unsigned short);

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *st;

	if ((st = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		st->replacement = PGETA; /* default is PCK GETA */
	}

	return ((iconv_t)st);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	int		stat;
	unsigned char	*ip, ic, ic2, ic3, *op;
	size_t		ileft, oleft;
	size_t		retval;

	st = (__icv_state_t *)cd;

	stat = ST_INIT;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		/* nothing to do here for this module */
		return ((size_t)0);
	}

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
			if (oleft < SJISW0) {
				UNGET();
				errno = E2BIG;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
			PUT(ic);
			continue;
		}
		if (ISCS1(ic)) {			/* Kanji starts */
			if ((int)ileft > 0) {
				GET(ic2);
				if (ISCS1(ic2)) {
					if (oleft < SJISW1) {
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					ic &= 0x7f;
					PUT(jis208tosj1[ic]);
					ic2 &= 0x7f;
					if ((ic % 2) == 0)
						ic2 += 0x80;
					PUT(jistosj2[ic2]);
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_EILSEQ_STATELESS(2)
				}
			} else {		/* input fragment of Kanji */
				UNGET();
				errno = EINVAL;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
		} else if (ic == SS2) {	/* Kana starts */
			if ((int)ileft > 0) {
				GET(ic2);
				if (ISCS2(ic2)) {
					if (oleft < SJISW2) {
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					PUT(ic2);
					continue;
				} else {	/* 2nd byte is illegal */
					UNGET_EILSEQ_STATELESS(2)
				}
			} else {		/* input fragment of Kana */
				UNGET();
				errno = EINVAL;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
		} else if (ic == SS3) { /* CS_3 Kanji starts */
			unsigned short dest;
			if (ileft >= EUCW3) {
				GET(ic2);
				GET(ic3);
				if (ISCS3(ic2) && ISCS3(ic3)) {
					if (oleft < SJISW1) {
						UNGET();
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					if (ic2 < 0xf5) { /* check IBM area */
						dest = (ic2 << 8);
						dest += ic3;
						dest = lookuptbl(dest);
						if (dest == 0xffff) {
							/*
							 * Illegal code points
							 * in G3 plane.
							 */
							UNGET_EILSEQ_STATELESS(3)
						} else {
							if ((dest == PGETA) &&
							(st->_icv_flag & __ICONV_CONV_NON_IDENTICAL))
							{
								CALL_NON_IDENTICAL_UNGET(3)
							} else {
								PUT((dest >> 8) & 0xff);
								PUT(dest & 0xff);
							}
						}
						continue;
					} else {
						ic2 &= 0x7f;
						ic3 &= 0x7f;
						if ((ic2 % 2) == 0)
							ic3 += 0x80;
						ic2 = jis212tosj1[ic2];
						ic3 = jistosj2[ic3];
						if ((ic2 != 0xff) &&
							(ic3 != 0xff)) {
							PUT(ic2);
							PUT(ic3);
							continue;
						}
						if (st->_icv_flag & 
							__ICONV_CONV_NON_IDENTICAL) {
							CALL_NON_IDENTICAL_UNGET(3)
						} else {
							PUT((PGETA >> 8) & 0xff);
							PUT(PGETA & 0xff);
						}
						continue;
					}
				} else { /* 2nd and 3rd byte check failed */
					UNGET_EILSEQ_STATELESS(3)
				}
			} else {	/* input fragment of JISX0212 */
				UNGET();
				errno = EINVAL;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
		} else { /* 1st byte check failed */
			UNGET_EILSEQ_STATELESS(1)
		}
	}
	retval = ileft;
ret:
	*inbuf = (char *)ip;
	*inbytesleft = ileft;
	*outbuf = (char *)op;
	*outbytesleft = oleft;

	return (retval);
}

/*
 * lookuptbl()
 * Return the index number if its index-ed number
 * is the same as dest value.
 */
static unsigned short
lookuptbl(unsigned short dest)
{
	unsigned short tmp;
	int i;
	int sz = (sizeof (sjtoibmext) / sizeof (sjtoibmext[0]));

	for (i = 0; i < sz; i++) {
		tmp = sjtoibmext[i];
		if (tmp == dest)
			return ((i + 0xfa40 + ((i / 0xc0) * 0x40)));
	}
	return (PGETA);
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
