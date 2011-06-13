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
 * Copyright (c) 1997, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_unicode_enhance.h"

/* Note: JFP_J2U_ICONV_RFC1468 macro pass through hankaku katakata. */
#ifdef  RFC1468_MODE
#define	JFP_J2U_ICONV_RFC1468
#else
#define	JFP_J2U_ICONV
#endif
#include "jfp_jis_to_ucs2.h"

/*
 * PUTU_UNGET macro
 *
 * Write unicode to output buffer. When attribute ICONV_CONV_NON_IDENTICAL_*
 * is specified and writing unicode is a replacement character (0xfffd),
 * call __icv_non_identical() to process non-identical char. Otherwise,
 * call write_unicode(). If any errors are encountered, move back the pointer
 * of input buffer, and return.
 *
 * This macro is used only from this program so far.
 */
#define PUTU_UNGET(u32, msg, num_of_bytes) \
	if ((st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) \
		&& (u32 == 0xfffd)) { \
		if(__icv_non_identical((unsigned char **)(&ip), &op, &oleft, \
			st, num_of_bytes) == (size_t)-1) { \
			UNGET(); \
			retval = ((size_t)-1); \
			goto ret; \
		} \
	} else if (write_unicode(u32, &op, &oleft, st, msg) \
			< (size_t)0) { \
		UNGET(); \
		retval = ((size_t)-1); \
		goto ret; \
	}

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *st;

	if ((st = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		st->_st_cset_sav = st->_st_cset = CS_0;
		st->replacement = 0xefbfbd; /* UTF-8 of U+fffd */
	}

	return ((iconv_t)st);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	int cset, stat;
	char *ip, ic, ic2;
	size_t ileft;
	size_t retval;
	unsigned int index;
	char		*op;
	size_t		oleft;

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
	stat = ST_INIT;

	ip = (char *)*inbuf;
	op = *outbuf;
	ileft = *inbytesleft;
	oleft = *outbytesleft;

	/*
	 * Main loop; 1 loop per 1 input byte
	 */

	while ((int)ileft > 0) {
		GETB(ic);
		if (stat == ST_ESC) {
			if (ic == MBTOG0_1) {
				if ((int)ileft > 0) {
					stat = ST_MBTOG0_1;
					continue;
				} else {
					UNGET_ERRRET_STATELESS(2, EINVAL)
				}
			} else if (ic == SBTOG0_1) {
				if ((int)ileft > 0) {
					stat = ST_SBTOG0;
					continue;
				} else {
					UNGET_ERRRET_STATELESS(2, EINVAL)
				}
			} else if (ic == X208REV_1) {
				if ((int)ileft > 0) {
					stat = ST_208REV_1;
					continue;
				} else {
					UNGET_ERRRET_STATELESS(2, EINVAL)
				}
			} else {
				UNGET_ERRRET_STATELESS(2, EILSEQ)
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
					UNGET_ERRRET_STATELESS(3, EINVAL)
				}
			} else if (ic == F_X0212_90) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_3;
				continue;
			} else {
				UNGET_ERRRET_STATELESS(3, EILSEQ)
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
				UNGET_ERRRET_STATELESS(4, EILSEQ)
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
				UNGET_ERRRET_STATELESS(3, EILSEQ)
			}
		} else if (stat == ST_208REV_1) {
			if (ic == X208REV_2) {
				if ((int)ileft > 0) {
					stat = ST_208REV_2;
					continue;
				} else {
					UNGET_ERRRET_STATELESS(3, EINVAL)
				}
			} else {
				UNGET_ERRRET_STATELESS(3, EILSEQ)
			}
		} else if (stat == ST_208REV_2) {
			if (ic == ESC) {
				if ((int)ileft > 0) {
					stat = ST_REV_AFT_ESC;
					continue;
				} else {
					UNGET_ERRRET_STATELESS(4, EINVAL)
				}
			} else {
				UNGET_ERRRET_STATELESS(4, EILSEQ)
			}
		} else if (stat == ST_REV_AFT_ESC) {
			if (ic == MBTOG0_1) {
				if ((int)ileft > 0) {
					stat = ST_REV_AFT_MBTOG0_1;
					continue;
				} else {
					UNGET_ERRRET_STATELESS(5, EINVAL)
				}
			} else {
				UNGET_ERRRET_STATELESS(5, EILSEQ)
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
					UNGET_ERRRET_STATELESS(6, EINVAL)
				}
			} else {
				UNGET_ERRRET_STATELESS(6, EILSEQ)
			}
		} else if (stat == ST_REV_AFT_MBTOG0_2) {
			if (ic == F_X0208_83_90) {
				stat = ST_INIT;
				st->_st_cset_sav = cset = CS_1;
				continue;
			} else {
				UNGET_ERRRET_STATELESS(7, EILSEQ)
			}
		}
		/*
		 * Break through chars or ESC sequence
		 * if (stat == ST_INIT)
		 */
		if (ic == ESC) {
			if ((int)ileft > 0) {
				stat = ST_ESC;
				continue;
			} else {
				UNGET_ERRRET_STATELESS(1, EINVAL)
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
			if ((cset == CS_0) || (cset == CS_2)){
				if (cset == CS_0) {
					RESTORE_HEX_ASCII_CONTINUE(ic)
					index = (int)_jfp_tbl_jisx0201roman_to_ucs2[ic];
				} else if (cset == CS_2) {
					index = 
					(int)_jfp_tbl_jisx0201kana_to_ucs2[(ic - 0x21)];
				}
				PUTU_UNGET(index, "writing CS_0/2", 2)
				stat = ST_INIT;
				continue;
			} else if ((cset == CS_1) || (cset == CS_3)) {
				if ((int)ileft > 0) {
					if ((ic < 0x21) || (ic == 0x7f)) {
						/* 1st byte check failed */
						UNGET_ERRRET_STATELESS(1, EILSEQ)
					}
					GET(ic2);
					if ((ic2 < 0x21) || (ic2 == 0x7f)) {
						/* 2nd byte check failed */
						UNGET_ERRRET_STATELESS(2, EILSEQ)
					}
					index = ((ic - 0x21) * 94)
							+ (ic2 - 0x21);
					if (cset == CS_1) {
#ifdef  RFC1468_MODE /* Convert VDC and UDC to GETA(DEFC_U in jis%UTF-8.h) */
						if ((ic == 0x2d) || 
						(0x75 <= ic))
							index = 0x3013;
						else
							index = (int)
							_jfp_tbl_jisx0208_to_ucs2[index];
#else   /* ISO-2022-JP.UIOSF */
						index = (int)
							_jfp_tbl_jisx0208_to_ucs2[index];
#endif  /* RFC1468_MODE */
					} else if (cset == CS_3) {
#ifdef  RFC1468_MODE /* Convert JIS X 0212 to GETA(DEFC_U in jis%UTF-8.h) */
						index = 0x3013;
#else   /* ISO-2022-JP.UIOSF */
						index = 
						(int)_jfp_tbl_jisx0212_to_ucs2[index];
#endif  /* RFC1468_MODE */
					}
					PUTU_UNGET(index, "writing CS_1/3", 2)
					stat = ST_INIT;
					continue;
				} else {
					UNGET_ERRRET_STATELESS(1, EINVAL)
				}
			}
		} else {
			UNGET_ERRRET_STATELESS(1, EILSEQ)
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
