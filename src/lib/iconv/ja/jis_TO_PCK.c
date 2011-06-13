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
 * Copyright (c) 1991, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>
#include "japanese.h"
#include "jfp_iconv_common.h"

static unsigned short lookuptbl(unsigned short);

iconv_t
_icv_open_attr(int flag, void *reserved)
{
	__icv_state_t *st;

	if ((st = __icv_open_attr(flag)) != (__icv_state_t *)-1) {
		st->_st_cset = st->_st_cset_sav = CS_0;
		st->replacement = PGETA; /* default is PCK GETA */
	}

	return((iconv_t)st);
}

size_t
_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
	__icv_state_t	*st;

	int		cset;
	int		stat = ST_INIT;
	unsigned char	*op, ic, ic2;
	char		*ip;
	size_t		ileft, oleft;
	size_t		retval;

	st = (__icv_state_t *)cd;

	/*
	 * If inbuf and/or *inbuf are NULL, reset conversion descriptor
	 * and put escape sequence if needed.
	 */
	if ((inbuf == NULL) || (*inbuf == NULL)) {
		st->_st_cset_sav = st->_st_cset = CS_0;
		return ((size_t)0);
	}

	cset = st->_st_cset;

	ip = (char *)*inbuf;
	op = (unsigned char *)*outbuf;
	ileft = *inbytesleft;
	oleft = *outbytesleft;

	/*
	 * Main loop; basically 1 loop per 1 input byte
	 */

	while ((int)ileft > 0) {
		GET(ic);
		if (stat == ST_INIT) {
			goto text;
		}
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
				cset = CS_2;
				stat = ST_INIT;
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
text:
		/*
		 * Break through chars or ESC sequence
		 */
		if (ic == ESC) {
			if ((int)ileft > 0) {
				stat = ST_ESC;
				continue;
			} else {
				UNGET_ERRRET_STATELESS(1, EINVAL)
			}
		} else if (ic == SO) {
			cset = CS_2;
			stat = ST_INIT;
			continue;
		} else if (ic == SI) {
			cset = st->_st_cset_sav;
			stat = ST_INIT;
			continue;
		}
		if (!(ic & CMSB)) {
			if (cset == CS_0) {
				/* ASCII or JIS roman : may be 8bit chars */
				RESTORE_HEX_ASCII_CONTINUE(ic)
				if (oleft < SJISW0) {
					UNGET();
					errno = E2BIG;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
				PUT(ic);
				continue;
			} else if (cset == CS_1) { /* CS_1 Kanji starts */
				if ((int)ileft > 0) {
					if (oleft < SJISW1) {
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					if ((ic < 0x21) || (ic == 0x7f)) {
						/* 1st byte check failed */
						UNGET_ERRRET_STATELESS(1, EILSEQ)
					}
					GET(ic2);
					if ((ic2 < 0x21) || (ic2 == 0x7f)) {
						/* 2nd byte check failed */
						UNGET_ERRRET_STATELESS(2, EILSEQ)
					}
					PUT(jis208tosj1[ic]);
					if ((ic % 2) == 0)
						ic2 += 0x80;
					PUT(jistosj2[ic2]);
					continue;
				} else {	/* input fragment of Kanji */
					UNGET_ERRRET_STATELESS(1, EINVAL)
				}
			} else if (cset == CS_2) { /* Hankaku Katakana */
				if (oleft < SJISW2) {
					UNGET();
					errno = E2BIG;
					retval = (size_t)ERR_RETURN;
					goto ret;
				}
				PUT(ic | CMSB);
				continue;
			} else if (cset == CS_3) { /* CS_3 Kanji starts */
				unsigned short dest;
				if ((int)ileft > 0) {
					if (oleft < SJISW1) {
						UNGET();
						errno = E2BIG;
						retval = (size_t)ERR_RETURN;
						goto ret;
					}
					if ((ic < 0x21) || (ic == 0x7f)) {
						/* 1st byte check failed */
						UNGET_ERRRET_STATELESS(1, EILSEQ)
					}
					GET(ic2);
					if ((ic2 < 0x21) || (ic2 == 0x7f)) {
						/* 2nd byte check failed */
						UNGET_ERRRET_STATELESS(2, EILSEQ)
					}
					if (ic < 0x75) { /* check IBM area */
						dest = (ic << 8);
						dest += ic2;
						dest = lookuptbl(dest);
						if (dest == 0xffff) {
							/*
							 * Illegal code points
							 * in G3 plane.
							 */
							UNGET_ERRRET_STATELESS(2, EILSEQ)
						} else {
							if ((dest == PGETA) &&
							(st->_icv_flag & __ICONV_CONV_NON_IDENTICAL))
							{
								CALL_NON_IDENTICAL_UNGET(2)
							} else {
								PUT((dest >> 8) & 0xff);
								PUT(dest & 0xff);
							}
						}
						continue;
					} else {
						if ((ic % 2) == 0)
							ic2 += 0x80;
						PUT(jis212tosj1[ic]);
						PUT(jistosj2[ic2]);
						continue;
					}
				} else {	/* input fragment of Kanji */
					UNGET_ERRRET_STATELESS(1, EINVAL)
				}
			}
		} else {
			if (oleft < UNKNOWNW) {
				UNGET();
				errno = E2BIG;
				retval = (size_t)ERR_RETURN;
				goto ret;
			}
			PUT(ic);
			continue;
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
