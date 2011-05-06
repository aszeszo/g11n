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
		cd->replacement = '?';
	}

	return ((iconv_t)cd);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t *st;
	int stat;
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
		/* nothing to do here for this module */
		return ((size_t)0);
	}

	ip = (unsigned char *)*inbuf;
	op = *outbuf;
	ileft = *inbytesleft;
	oleft = *outbytesleft;

	/*
	 * Main loop; basically 1 loop per 1 input byte
	 */

	while ((int)ileft > 0) {
		GET(ic);
		if (ISASC((int)ic)) { /* ASCII */
			UNGET_EILSEQ(1);
		} else if (ISCS1(ic)) { /* CS_1 starts */
			if ((int)ileft > 0) {
				GET(ic2);
				if (ISCS1(ic) && ISCS1(ic2)) {
					if (oleft < JISW1) {
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					PUT(((ic & CMASK) - 0x20));
					PUT(((ic2 & CMASK) - 0x20));
					continue;
				} else { /* 2nd byte check failed */
					UNGET_EILSEQ(2)
				}
			} else {		/* input fragment of Kanji */
				UNGET();
				errno = EINVAL;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
		} else if (ic == SS2) {	/* Kana starts */
			UNGET_EILSEQ(1)
		} else if (ic == SS3) {	/* JISX0212 starts */
			if (ileft >= EUCW3) {
				GET(ic2);
				GET(ic3);
				if (ISCS3(ic2) && ISCS3(ic3)) {
					if (oleft < JISW3) {
						UNGET();
						UNGET();
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					PUT(((ic2 & CMASK) - 0x20));
					PUT(((ic3 & CMASK) - 0x20));
					continue;
				} else { /* 2nd and 3rd byte check failed */
					UNGET_EILSEQ(3)
				}
			} else {	/* input fragment of JISX0212 */
				UNGET();
				errno = EINVAL;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
		} else { /* 1st byte check failed */
			UNGET_EILSEQ(1)
		}
	}
	retval = ileft;
ret:
	*inbuf = (char *)ip;
	*inbytesleft = ileft;
	*outbuf = op;
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
	return (__replace_invalid_iso2022jp(pop, poleft, cd));
}
