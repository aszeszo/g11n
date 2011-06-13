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
 * Copyright (c) 1999, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#if	defined(DEBUG)
#include <stdio.h>
#endif	/* DEBUG */
#include <stdlib.h>
#include <errno.h>

#define	JFP_ICONV_STATELESS
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_unicode_enhance.h"

iconv_t
_icv_open_attr(int flag, void *reserve)
{
	__icv_state_t *cd;

	if ((cd = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		cd->replacement = 0xefbfbd; /* UTF-8 of U+fffd */
		cd->trivialp = __TRIVIALP; /* trivial conversion */
	}

	return ((iconv_t)cd);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	size_t		rv = (size_t)0;
	unsigned int	ucs4;

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
		/* nothing to do here for this module */
		return ((size_t)0);
	}

	ip = (unsigned char *)*inbuf;
	ileft = *inbytesleft;
	op = *outbuf;
	oleft = *outbytesleft;

	while (ileft != 0) {
		GETU(&ucs4)

		if (ucs4 == 0xff5e)		/* FULLWIDTH TILDE */
			ucs4 = 0x301c;		/* WAVE DASH */
		else if (ucs4 == 0x2225)	/* PARALLEL TO */
			ucs4 = 0x2016;		/* DOUBLE VERTICAL BAR/LINE */
		else if (ucs4 == 0xff0d)	/* FULLWIDTH MINUS SIGN */
			ucs4 = 0x2212;		/* MINUS SIGN */
		else if (ucs4 == 0xffe0)	/* FULLWIDTH CENT SIGN */
			ucs4 = 0x00a2;		/* CENT SIGN */
		else if (ucs4 == 0xffe1)	/* FULLWIDTH POUND SIGN */
			ucs4 = 0x00a3;		/* POUND SIGN */
		else if (ucs4 == 0xffe2)	/* FULLWIDTH NOT SIGN */
			ucs4 = 0x00ac;		/* NOT SIGN */

		PUTUCS2((unsigned short)ucs4, "E2BIG");
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

#if	defined(DEBUG)
	if (rv == (size_t)-1) {
		fprintf(stderr, "DEBUG: errno=%d: %s\n", errno, debugmsg);
	}
#endif	/* DEBUG */

	/*
	 * When successfully converted, return number of non-identical
	 * conversion as described in iconv(3C) and iconvstr(3C)
	 */
	return ((rv == (size_t)-1) ? rv : st->num_of_ni);
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
	return (__replace_hex_utf32(hex, pip, pop, poleft, cd, caller));
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
