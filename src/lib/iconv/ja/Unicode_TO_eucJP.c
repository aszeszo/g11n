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
 * Copyright (c) 2007, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>

#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_unicode.h"

#ifdef JAVA_CONV_COMPAT
#define	JFP_U2E_ICONV_JAVA
#elif	JFP_ICONV_MS932
#define	JFP_U2E_ICONV_MS932
#else
#define	JFP_U2E_ICONV
#endif
#include "jfp_ucs2_to_euc16.h"

#define	DEF_SINGLE	'?'

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *cd;

	if ((cd = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		_icv_reset_unicode((void *)cd);
		cd->replacement = DEF_SINGLE;
	}

	return ((iconv_t)cd);
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

	unsigned char	*ip;
        size_t		ileft, pre_ileft;
	char		*op;
        size_t		oleft;

	int		cset; /* not used but needed for GETU() */

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
		pre_ileft = ileft; /* value before reading input bytes */
		GETU(&ucs4);

		if (ucs4 > 0xffff) {
			if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
				CALL_NON_IDENTICAL()
			} else {
				/* non-BMP */
				ic = (unsigned char)DEF_SINGLE;
				NPUT(ic, "DEF for non-BMP");
			}
		} else {
			euc16 = _jfp_ucs2_to_euc16((unsigned short)ucs4);

			if(euc16 == 0xffff) {
				if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
					CALL_NON_IDENTICAL()
					goto next;
				} else {
					euc16 = DEF_SINGLE; /* replacement char */
				}
			}

			switch (euc16 & 0x8080) {
			case 0x0000:	/* CS0 */
				ic = (unsigned char)euc16;
				NPUT(ic, "CS0");
				break;
			case 0x8080:	/* CS1 */
				ic = (unsigned char)((euc16 >> 8) & 0xff);
				NPUT(ic, "CS1-1");
				ic = (unsigned char)(euc16 & 0xff);
				NPUT(ic, "CS1-2");
				break;
			case 0x0080:	/* CS2 */
				NPUT(SS2, "CS2-1");
				ic = (unsigned char)euc16;
				NPUT(ic, "CS2-2");
				break;
			case 0x8000:	/* CS3 */
				NPUT(SS3, "E2BIG at CS3-1");
				ic = (unsigned char)((euc16 >> 8) & 0xff);
				NPUT(ic, "CS3-2");
				ic = (unsigned char)(euc16 & CMASK);
				NPUT(ic | CMSB, "CS3-3");
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
size_t
__replace_hex(
	unsigned char	hex,
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd,
	int		caller)
{
	return (__replace_hex_ascii(hex, pip, pop, poleft, caller));
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
