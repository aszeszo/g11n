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
 * Copyright (c) 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>

#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_wchar.h"
#include "jfp_iconv_unicode_enhance.h"

#define	JFP_U2E_ICONV
#include "jfp_ucs2_to_euc16.h"

#define	DEF_SINGLE	'?'

static unsigned short lookuptbl(unsigned short);

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

	unsigned char	ic1, ic2, ku, ten, oc;
	size_t		rv = (size_t)0;
	unsigned int	ucs4;
	unsigned short	euc16;
	unsigned short	dest;
	wchar_t		pckwchar;

	unsigned char	*ip;
        size_t		ileft, pre_ileft;
	char		*op;
        size_t		oleft;

	int		cset; /* not used, but needed for GETU() */

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
				ic1 = (unsigned char)DEF_SINGLE;
				pckwchar = __get_pckwchar(__ASCII, ic1, NULL);
				NPUT_WCHAR(pckwchar, "DEF for non-BMP");
				st->num_of_ni++;
			}
		} else {
			euc16 = _jfp_ucs2_to_euc16((unsigned short)ucs4);

			if(euc16 == 0xffff) {
				if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
					CALL_NON_IDENTICAL()
					goto next;
				} else {
					euc16 = DEF_SINGLE; /* replacement char */
					st->num_of_ni++;
				}
			}

			switch (euc16 & 0x8080) {
			case 0x0000:	/* CS0 */
				if (ISC1CTRL((unsigned char)euc16)) {
					if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
						CALL_NON_IDENTICAL()
					} else {
						ic1 = (unsigned char)DEF_SINGLE;
						pckwchar = __get_pckwchar(__ASCII, 
									ic1, NULL);
						NPUT_WCHAR(pckwchar,
							"CS0-C1CTRL(replaced)")
						st->num_of_ni++;
					}
				} else {
					ic1 = (unsigned char)euc16;
					pckwchar = __get_pckwchar(__ASCII, ic1, NULL);
					NPUT_WCHAR(pckwchar, "CS0-1");
				}
				break;
			case 0x8080:	/* CS1 */
				ku = (unsigned short)((euc16 >> 8) & 0x7f);
				ic1 = jis208tosj1[ku];

				/*
				 * for even number row (Ku), add 0x80 to
				 * look latter half of jistosj2[] array
				 */
				ten = (unsigned char)((euc16 & 0x7f)
					+ (((ku % 2) == 0) ? 0x80 : 0x00));
				ic2 = jistosj2[ten];

				pckwchar = __get_pckwchar(__PCK_KANJI, ic1, ic2);
				NPUT_WCHAR(pckwchar, "CS1");
				break;
			case 0x0080:	/* CS2 */
				ic1 = (unsigned char)euc16;
				pckwchar = __get_pckwchar(__PCK_KANA, ic1, NULL);
				NPUT_WCHAR(pckwchar, "CS2");
				break;
			case 0x8000:	/* CS3 */
				ku = (unsigned short)((euc16 >> 8) & 0x7f);
				if (euc16 == 0xa271) {
					/* NUMERO SIGN */
					pckwchar = __get_pckwchar(__PCK_KANJI, 0x87, 0x82);
					NPUT_WCHAR(pckwchar, "CS3-NUMERO");
				} else if (ku < 0x75) { /* check if IBM VDC */
					dest = lookuptbl(euc16 & 0x7f7f);
					if (dest == 0xffff) {
						if (st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) {
							CALL_NON_IDENTICAL()
						} else {
							ic1 = (unsigned char)DEF_SINGLE;
							pckwchar = __get_pckwchar(__ASCII,
									ic1, NULL);
							NPUT_WCHAR(pckwchar,
								"CS3-NoSJIS(replaced)")
							st->num_of_ni++;
						}
					} else {
						/* avoid putting NUL ('\0') */
						if (dest > 0xff) {
							ic1 = (dest >> 8) & 0xff;
							ic2 = dest & 0xff;
							pckwchar = __get_pckwchar(__PCK_KANJI,
									ic1, ic2);
							NPUT_WCHAR(pckwchar, "CS3-IBM");
						} else {
							if ((dest == 0x3f)
								&& (st->_icv_flag 
									& __ICONV_CONV_NON_IDENTICAL)) {
								CALL_NON_IDENTICAL()
							} else {
								ic1 = dest & 0xff;
								pckwchar = __get_pckwchar(__PCK_KANA,
										ic1, NULL);
								NPUT_WCHAR(pckwchar, "CS3-IBM-1");
								if (dest == 0x3f) {
									st->num_of_ni++;
								}
							}
						}
					}
				} else {
					ic1 = jis212tosj1[ku];
					/*
					 * for even number row (Ku), add 0x80 to
					 * look latter half of jistosj2[] array
					 */
					ten = (unsigned short)((euc16 & 0x7f)
						+ (((ku % 2) == 0) ?
						0x80 : 0x00));
					ic2 = jistosj2[ten];

					pckwchar = __get_pckwchar(__PCK_KANJI,
								ic1, ic2);
					NPUT_WCHAR(pckwchar, "CS3");
				}
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
		tmp = (sjtoibmext[i] & 0x7f7f);
		if (tmp == dest)
			return ((i + 0xfa40 + ((i / 0xc0) * 0x40)));
	}
	return (0x3f);
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
	return (__replace_hex_wchar(hex, pip, pop, poleft, cd, caller));
}

/* see jfp_iconv_common.h */
size_t
__replace_invalid(
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	return (__replace_invalid_wchar(pop, poleft, cd));
}
