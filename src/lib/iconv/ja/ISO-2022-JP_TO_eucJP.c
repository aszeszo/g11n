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
		st->_st_cset = st->_st_cset_sav = CS_0;
		st->replacement = EGETA; /* default is eucJP GETA */
	}

	return ((iconv_t)st);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	int		cset;
	int		stat = ST_INIT;
	unsigned char	*op;
	char		*ip, ic, ic2;
	size_t 		ileft, oleft;
	size_t 		retval;
        unsigned short  zenkaku;

	__tmpbuf_t	tmpbuf;

	st = (__icv_state_t *)cd;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		st->_st_cset_sav = st->_st_cset = CS_0;
		return ((size_t)0);
	}

	/*
	 * clear temporary buffer, and set buffer address into the
	 * conversion descriptor
	 */
	__clearbuf(&tmpbuf); /* clear temporary buffer */
	st->tmpbuf = &tmpbuf;

	cset = st->_st_cset;

	ip = (char *)*inbuf;
	op = (unsigned char *)*outbuf;
	ileft = *inbytesleft;
	oleft = *outbytesleft;

	/*
	 * Main loop; basically 1 loop per 1 input byte
	 */

	while ((int)ileft > 0) {
		GETB(ic);
		if (stat == ST_INIT) {
			goto text;
		}

		if (stat == ST_ESC) {
			if (ic == MBTOG0_1) {
				if ((int)ileft > 0) {
					stat = ST_MBTOG0_1;
					continue;
				} else {
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else if (ic == SBTOG0_1) {
				if ((int)ileft > 0) {
					stat = ST_SBTOG0;
					continue;
				} else {
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else if (ic == X208REV_1) {
				if ((int)ileft > 0) {
					stat = ST_208REV_1;
					continue;
				} else {
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else {
				UNGET_EILSEQ_STATELESS(2)
			}
		} else if (stat == ST_MBTOG0_1) {
			if ((ic == F_X0208_83_90) || (ic == F_X0208_78)) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_1;
				continue;
			} else if (ic == MBTOG0_2) {
				if ((int)ileft > 0) {
					stat = ST_MBTOG0_2;
					continue;
				} else {
					UNGET();
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else if (ic == F_X0212_90) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_3;
				continue;
			} else {
				UNGET_EILSEQ_STATELESS(3)
			}
		} else if (stat == ST_MBTOG0_2) {
			if ((ic == F_X0208_83_90) || (ic == F_X0208_78)) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_1;
				continue;
			} else if (ic == F_X0212_90) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_3;
				continue;
			} else {
				UNGET_EILSEQ_STATELESS(4)
			}
		} else if (stat == ST_SBTOG0) {
			if ((ic == F_ASCII) ||
				(ic == F_X0201_RM) ||
				(ic == F_ISO646)) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_0;
				continue;
			} else if (ic == F_X0201_KN) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_2;
				continue;
			} else {
				UNGET_EILSEQ_STATELESS(3)
			}
		} else if (stat == ST_208REV_1) {
			if (ic == X208REV_2) {
				if ((int)ileft > 0) {
					stat = ST_208REV_2;
					continue;
				} else {
					UNGET();
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else {
				UNGET_EILSEQ_STATELESS(3)
			}
		} else if (stat == ST_208REV_2) {
			if (ic == ESC) {
				if ((int)ileft > 0) {
					stat = ST_REV_AFT_ESC;
					continue;
				} else {
					UNGET();
					UNGET();
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else {
				UNGET_EILSEQ_STATELESS(4)
			}
		} else if (stat == ST_REV_AFT_ESC) {
			if (ic == MBTOG0_1) {
				if ((int)ileft > 0) {
					stat = ST_REV_AFT_MBTOG0_1;
					continue;
				} else {
					UNGET();
					UNGET();
					UNGET();
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else {
				UNGET_EILSEQ_STATELESS(5)
			}
		} else if (stat == ST_REV_AFT_MBTOG0_1) {
			if (ic == F_X0208_83_90) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_1;
				continue;
			} else if (ic == MBTOG0_2) {
				if ((int)ileft > 0) {
					stat = ST_REV_AFT_MBTOG0_2;
					continue;
				} else {
					UNGET();
					UNGET();
					UNGET();
					UNGET();
					UNGET();
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else {
				UNGET_EILSEQ_STATELESS(6)
			}
		} else if (stat == ST_REV_AFT_MBTOG0_2) {
			if (ic == F_X0208_83_90) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_1;
				continue;
			} else {
				UNGET_EILSEQ_STATELESS(7)
			}
		}
text:
		/*
		 * Break through chars or ESC sequence
		 */
		if (ic == ESC) {
			if ((int)ileft > 0) {
				stat = ST_ESC;
				continue;
			} else {
				UNGET();
				errno = EINVAL;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
		/*
		 * XXX- Because V3 mailtool uses SI/SO to switch
		 *	G0 and G1 sets while it puts "iso2022-7"
		 *	as its "X-Sun-Charset" tag. Though it
		 *	breaks ISO-2022-JP definition based on
		 *	UI-OSF, dtmail have handle them correctly.
		 *	Therefore, we have to following a few codes, UGH.
		 */
		} else if (ic == SO) {
			cset = CS_2;
			stat = ST_INIT;
			continue;
		} else if (ic == SI) {
			cset = st->_st_cset_sav;
			stat = ST_INIT;
			continue;
		} else if (!(ic & CMSB)) {
			if (cset == CS_0) {
				RESTORE_HEX_ASCII_CONTINUE(ic)
				CHECK2BIG(EUCW0, 1);
				PUT(ic);
				continue;
			} else if (cset == CS_1) {
				if ((int)ileft > 0) {
					CHECK2BIG(EUCW1, 1);
					if ((ic < 0x21) || (ic == 0x7f)) {
						/* 1st byte check failed */
						UNGET_EILSEQ_STATELESS(1)
					}
					GETB(ic2);
					if ((ic2 < 0x21) || (ic2 == 0x7f)) {
						/* 2nd byte check failed */
						UNGET_EILSEQ_STATELESS(2)
					}
#ifdef  RFC1468_MODE /* Convert VDC and UDC to GETA */
					if ((ic == 0x2d) || (0x75 <= ic )){
						PUT((EGETA >> 8) & 0xff);
						PUT(EGETA & 0xff);
						continue;
					}
#endif  /* RFC1468_MODE */
					PUT(ic | CMSB);
					PUT(ic2 | CMSB);
					stat = ST_INIT;
					continue;
				} else {
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			} else if (cset == CS_2) {
				if (!ISSJKANA((ic | CMSB))) {
					UNGET_EILSEQ_STATELESS(1)
				}
#ifdef  RFC1468_MODE /* Convert JIS X 0201 Kana to JIS X 0208 Kana */
				CHECK2BIG(EUCW1, 1);
				zenkaku = halfkana2zenkakue[(ic - 0x21)];
	                        ic = (unsigned char)((zenkaku >> 8) & 0xFF);
	                        PUT(ic);
	                        ic = (unsigned char)(zenkaku & 0xFF);
	                        PUT(ic);
#else   /* ISO-2022-JP.UIOSF */
				CHECK2BIG(EUCW2 + SEQ_SS, 1);
				PUT(SS2);
				PUT(ic | CMSB);
#endif  /* RFC1468_MODE */
				continue;
			} else if (cset == CS_3) {
				if ((int)ileft > 0) {
					if ((ic < 0x21) || (ic == 0x7f)) {
						/* 1st byte check failed */
						UNGET_EILSEQ_STATELESS(1)
					}
					GETB(ic2);
					if ((ic2 < 0x21) || (ic2 == 0x7f)) {
						/* 2nd byte check failed */
						UNGET_EILSEQ_STATELESS(2)
					}
#ifdef  RFC1468_MODE /* Convert JIS X 0212 to GETA */
					CHECK2BIG(EUCW1, 2);
					PUT((EGETA >> 8) | CMSB);
					PUT((EGETA & CMASK) | CMSB);
#else   /* ISO-2022-JP.UIOSF */
					CHECK2BIG(EUCW3 + SEQ_SS, 2);
					PUT(SS3);
					PUT(ic | CMSB);
					PUT(ic2 | CMSB);
#endif  /* RFC1468_MODE */
					stat = ST_INIT;
					continue;
				} else {
					UNGET();
					errno = EINVAL;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
			}
		} else {
			UNGET_EILSEQ_STATELESS(1)
		}
	}
	retval = ileft;
ret:
	*inbuf = ip;
	*inbytesleft = ileft;
	*outbuf = (char *)op;
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
