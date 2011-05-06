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

#include <stdlib.h>
#include <errno.h>
#include <euc.h>
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_unicode_enhance.h"

#define	JFP_J2U_ICONV_X0213
#include "jfp_jis_to_ucs2.h"

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *cd;

	if ((cd = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		_icv_reset_unicode((void *)cd);
		cd->_st_cset = CS_0;
	}

	return ((iconv_t)cd);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	unsigned int	u32;		/* UTF-32 */
	unsigned short	e16;		/* 16-bit EUC */
	unsigned char	ic1, ic2;	/* bytes in a char or an esc seq */
	unsigned char	ic3, ic4;	/* bytes in an esc seq */
	size_t		rv = (size_t)0;	/* return value of this function */

	unsigned char	*ip;
        size_t		ileft;
	char		*op;
        size_t		oleft;

	__tmpbuf_t	tmpbuf;

	st = (__icv_state_t *)cd;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		st->_st_cset = CS_0;
		_icv_reset_unicode(st);
		return ((size_t)0);
	}

	ip = (unsigned char *)*inbuf;
	ileft = *inbytesleft;
	op = *outbuf;
	oleft = *outbytesleft;

	/*
	 * clear temporary buffer, and set buffer address into the
	 * conversion descriptor
	 */
	__clearbuf(&tmpbuf); /* clear temporary buffer */
	st->tmpbuf = &tmpbuf;

	while (ileft != 0) {
		NGETB(ic1, "never fail here"); /* get 1st byte */

		if (ic1 == ESC) { /* Escape */
			NGETB(ic2, "ESC-2");
			switch (ic2) {
			case 0x24: /* $ */
				NGETB(ic3, "ESC$-3");
				switch (ic3) {
				case 0x28: /* $( */
					NGETB(ic4, "ESC$(-4");
					switch (ic4) {
					case 0x4f: /* 24-28-4F ESC$(O */
						st->_st_cset = CS_1;
						break;
					case 0x50: /* 24-28-50 ESC$(P */
						st->_st_cset = CS_3;
						break;
					case 0x51: /* 24-28-51 ESC$(Q */
						st->_st_cset = CS_1;
						break;
					default:
						RET_EILSEQ("Unknown ESC$(?", 4)
					}
					break;
				case 0x42: /* 24-42 ESC$B */
					st->_st_cset = CS_1;
					break;
				default:
					RET_EILSEQ("Unknown ESC$?", 3)
				}
				break;
			case 0x28: /* ( */
				NGETB(ic3, "ESC(-3");
				switch (ic3) {
				case 0x42: /* 28-42 ESC(B */
					st->_st_cset = CS_0;
					break;
				default:
					RET_EILSEQ("Unknown ESC(?", 3)
				}
				break;
			default:
				RET_EILSEQ("Unknown ESC?", 2)
			}
		} else if (st->_st_cset == CS_0) { /* IRV */
			if ((ic1 == 0x0e) || (ic1 == 0x0f) || (ic1 > 0x7f)) {
				RET_EILSEQ("IRV-1", 1)
			}
			RESTORE_HEX_ASCII_JUMP(ic1)
			u32 = (unsigned int)_jfp_tbl_jisx0201roman_to_ucs2[ic1];
			PUTU(u32, "IRV", 1);
		} else if (st->_st_cset == CS_1) { /* Plane 1 */
			if ((ic1 < 0x21) || (ic1 > 0x7e)) {
				RET_EILSEQ("PLANE1-1", 1)
			}
			NGETB(ic2, "PLANE1-2");
			if ((ic2 < 0x21) || (ic2 > 0x7e)) {
				RET_EILSEQ("PLANE1-2", 2)
			}
			e16 = ((ic1 << 8) | ic2) | 0x8080;
			u32 = (unsigned int)_jfp_tbl_jisx0208_to_ucs2[
				(ic1 - 0x21) * 94 + (ic2 - 0x21)];
			if (IFHISUR(u32)) {
				u32 = _jfp_lookup_x0213_nonbmp(e16, u32);
				PUTU(u32, "PLANE1->NONBMP", 2);
			} else if (u32 == 0xffff) {
				/* need to compose */
				unsigned int	u32_2;
				u32 = _jfp_lookup_x0213_compose(e16, &u32_2);
				PUTU(u32, "PLANE1->CP1", 2);
				PUTU(u32_2, "PLANE1->CP2", 2);
			} else {
				PUTU(u32, "PLANE1->BMP", 2);
			}
		} else if (st->_st_cset == CS_3) { /* Plane 2 */
			if ((ic1 < 0x21) || (ic1 > 0x7e)) {
				RET_EILSEQ("PLANE2-1", 1)
			}
			NGETB(ic2, "PLANE2-2");
			if ((ic2 < 0x21) || (ic2 > 0x7e)) {
				RET_EILSEQ("PLANE2-2", 2)
			}
			e16 = ((ic1 << 8) | ic2) | 0x8000;
			u32 = (unsigned int)_jfp_tbl_jisx0213p2_to_ucs2[
				(ic1 - 0x21) * 94 + (ic2 - 0x21)];
			if (IFHISUR(u32)) {
				u32 = _jfp_lookup_x0213_nonbmp(e16, u32);
				PUTU(u32, "PLANE2->NONBMP", 2);
			} else {
				PUTU(u32, "PLANE2->BMP", 2);
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
	return (__replace_hex_utf32(hex, pip, pop, poleft, cd, caller));
}

/* see jfp_iconv_common.h */
size_t __replace_invalid(
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	return (__replace_invalid_utf32(pip, pop, poleft, cd));
}
