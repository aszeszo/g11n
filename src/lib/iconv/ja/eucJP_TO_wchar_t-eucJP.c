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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>
#include <wchar.h>
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_wchar.h"

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *cd;

	if ((cd = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		cd->replacement = E32GETA;
	}

	return ((iconv_t)cd);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	wchar_t		eucwchar;	/* EUC wide char representation */
	unsigned char	ic1, ic2, ic3;	/* 1st, 2nd, and 3rd bytes of a char */
	unsigned char	oc;		/* byte buffer for EUC wchar_t */
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
		/* nothing to do here for this module */
		return ((size_t)0);
	}

	ip = (unsigned char *)*inbuf;
	ileft = *inbytesleft;
	op = *outbuf;
	oleft = *outbytesleft;

	while ((int)ileft > 0) {
		NGET(ic1, "never fail here"); /* get 1st byte */

		if (ISASC((int)ic1)) { /* ASCII */
			RESTORE_HEX_ASCII_JUMP(ic1)
			eucwchar = __get_eucwchar(CS_0, ic1, NULL);
			NPUT_WCHAR(eucwchar, "CS0");
		} else if (ISCS1(ic1)) { /* JIS X 0208; 2 bytes */
			NGET(ic2, "CS1-1");
			if (ISCS1(ic2)) { /* 2nd byte check passed */
				eucwchar = __get_eucwchar(CS_1, ic1, ic2);
				NPUT_WCHAR(eucwchar, "CS1");
			} else { /* 2nd byte check failed */
				RET_EILSEQ("CS1-2", 2)
			}
		} else if (ic1 == SS2) {	/* JIS X 0201 Kana; 2 bytes */
			NGET(ic2, "CS2-2");
			if (ISCS2(ic2)) { /* 2nd byte check passed */
				eucwchar = __get_eucwchar(CS_2, ic2, NULL);
				NPUT_WCHAR(eucwchar, "CS2");
			} else { /* 2ndbyte check failed */
				RET_EILSEQ("CS2-2", 2)
			}
		} else if (ic1 == SS3) { /* JIS X 0212; 3 bytes */
			NGET(ic2, "CS3-2");
			if (ISCS3(ic2)) { /* 2nd byte check passed */
				NGET(ic3, "CS3-3");
				if (ISCS3(ic3)) { /* 3rd byte check passed */
					eucwchar = __get_eucwchar(CS_3, ic2, ic3);
					NPUT_WCHAR(eucwchar, "CS3");
				} else { /* 3rd byte check failed */
					RET_EILSEQ("CS3-3", 3)
				}
			} else { /* 2nd byte check failed */
				RET_EILSEQ("CS3-2", 2)
			}
		} else if (ISC1CTRLEUC(ic1)) { /* C1 control; 1 byte */
			eucwchar = __get_eucwchar(CS_0, ic1, NULL);
			NPUT_WCHAR(eucwchar, "E2BIG C1CTRL")
		} else { /* 1st byte check failed */
			RET_EILSEQ("CS?-1", 1)
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

	return ((rv == (size_t)-1) ? rv : *inbytesleft);
}

/* see jfp_iconv_enhance.h */
size_t
__replace_hex(
	unsigned char	hex,
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd,
	int		caller)
{
	return (__replace_hex_wchar(hex, pip, pop, poleft, cd, caller));
}

/* see jfp_iconv_enhance.h */
size_t
__replace_invalid(
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	return (__replace_invalid_wchar(pop, poleft, cd));
}
