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

#include <iconv.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"


/* These are implemented in the individual modules. */
size_t _icv_iconv(iconv_t *cd, const char **inbuf,
	size_t *inbufleft, char **outbuf, size_t *outbufleft);

void *_icv_open_attr(int flag, void *reserved);



void *
_icv_open(const char *strp)
{
	return _icv_open_attr(0, NULL);
}

void
_icv_close(iconv_t *cd)
{
	if (! cd)
		errno = EBADF;
	else
		free(cd);
}

size_t
_icv_ciconvstr(char *inarray, size_t *inlen, char *outarray,
	size_t *outlen, int flags, int chsz)
{
	char *np;
	size_t len, t;
	size_t ret;
        iconv_t *cd;

	/*
	 * This block detects the string terminating null character,
	 * as per iconvstr(3C). It was placed here to simplify _icv_iconv
	 * in the individual modules.
	 */
	len = *inlen;
	if ((flags & ICONV_IGNORE_NULL) == 0) {
		if (chsz == 1) {
			np = (char *)memchr((const void *)inarray, 0, *inlen);
			if (np)
				len = np - inarray;
		} else if (*inlen >= chsz) { 
			static const char null_ch[4] =
			    { '\0', '\0', '\0', '\0' };
			int i;
	
			for (i=0; i <= len - chsz; i += chsz) {
				if (memcmp(inarray + i, null_ch, chsz) == 0) {
					len = i;
					break;
				}
			}
		}
	}

	t = len;
	flags &= (ICONV_IGNORE_NULL | ICONV_REPLACE_INVALID);

	if ((cd = (iconv_t *)_icv_open_attr(flags, 0)) == (iconv_t *)-1)
		return (size_t)-1;
	ret = _icv_iconv(cd, (const char **)(&inarray), &len, &outarray, outlen);

	/* Write out remaining state data */
	if (ret != (size_t)-1 &&
	    _icv_iconv(cd, NULL, &len, &outarray, outlen) == (size_t)-1) {
		ret = -1;
	} else if (ret == (size_t)-1 && errno == EILSEQ) {
		_icv_iconv(cd, NULL, &len, &outarray, outlen);
		errno = EILSEQ;
	}

	_icv_close(cd);

	*inlen -= (t - len);

	return (ret);
}

int
_icv_flag_action(int *flags, int req, int *arg, int attributes)
{
	int a = *arg;
	long f = *flags;
	
	switch (req) {
	case ICONV_GET_CONVERSION_BEHAVIOR:
		a = f;
		break;

	case ICONV_SET_CONVERSION_BEHAVIOR:
		if (a & ICONV_CONV_NON_IDENTICAL_TRANSLITERATE &&
		    attributes & ICONVCTL_NO_TRANSLIT) {
			errno = ENOTSUP;
			return -1;
		}
		f = a;
		break;

	case ICONV_GET_DISCARD_ILSEQ:
		a = (f & ICONV_CONV_ILLEGAL_DISCARD &&
		    f & ICONV_CONV_NON_IDENTICAL_DISCARD) ? 1 : 0;
		break;

	case ICONV_SET_DISCARD_ILSEQ:
		if (a)
			f |= (ICONV_CONV_ILLEGAL_DISCARD |
				ICONV_CONV_NON_IDENTICAL_DISCARD);
		else
			f &= ~(ICONV_CONV_ILLEGAL_DISCARD |
				ICONV_CONV_NON_IDENTICAL_DISCARD);
		break;

	case ICONV_GET_TRANSLITERATE:
		a = (f & ICONV_CONV_NON_IDENTICAL_TRANSLITERATE) ? 1 : 0;
		break;

	case ICONV_SET_TRANSLITERATE:
		if (attributes & ICONVCTL_NO_TRANSLIT) {
			errno = ENOTSUP;
			return -1;
		}
		if (a)
			f |= ICONV_CONV_NON_IDENTICAL_TRANSLITERATE;
		else
			f &= ~ICONV_CONV_NON_IDENTICAL_TRANSLITERATE;
		break;

	case ICONV_TRIVIALP:
		a = (attributes & ICONVCTL_NON_TRIVIAL) ? 0 : 1;
		break;

	default:
		/* Unknown request */
		errno = EINVAL;
		return -1;
	}

	*flags = f;
	*arg = a;

	return 0;
}


/* Hex conversion macro */
#define HEX2DEC(c)	((c) >= 'a' ? (c)-'a'+10 : \
			 (c) >= 'A' ? (c)-'A'+10 : (c)-'0')


int
_icv_restore_hex(char **inbuf, size_t inbufleft, char **outbuf,
	size_t outbufleft)
{
	unsigned char *ib = (unsigned char *)*inbuf;

	if (inbufleft < ICV_RHEX_LEN) 
		return 0;

	if (outbufleft < 1) {
		errno = E2BIG;
		return -1;
	}

	if (! (isxdigit(ib[4]) && isxdigit(ib[5])))
		return 0;

	**outbuf = (unsigned char)(HEX2DEC(ib[4]) * 16 + HEX2DEC(ib[5]));

	(*outbuf)++;
	*inbuf += ICV_RHEX_LEN;

	return 1;
}


int
_icv_ucs_restore_hex(int (*_getc)(int, char **, size_t, uint_t *, uint_t *),
	char **inbuf, size_t inbufleft, char **outbuf,
	size_t outbufleft, int chsz, int little_endian)
{
	uint_t u4;
	uint_t u4_2;
	unsigned char *ib;
	unsigned char c0, c;
	int i;

	ib = (unsigned char *)*inbuf;

	if (inbufleft < chsz * ICV_RHEX_LEN)
		return 0;

	if (outbufleft < 1) {
		errno = E2BIG;
		return -1;
	}

	/* Consume the prefix */
	ib += chsz * 4;
	inbufleft -= chsz * 4;

	i = _getc(little_endian, (char **)&ib, inbufleft, &u4, &u4_2);
	if (i == -1)
		return (-1);
	if (i == -2 || u4_2)
		return 0;

	c0 = (unsigned char) u4;
	ib += chsz;
	inbufleft -= chsz;

	i = _getc(little_endian, (char **)&ib, inbufleft, &u4, &u4_2);
	if (i == -1)
		return (-1);
	if (i == -2 || u4_2)
		return 0;
	
	c = (unsigned char) u4;
	ib += chsz;

	if (! (isxdigit(c0) && isxdigit(c)))
		return 0;

	**outbuf = (unsigned char)(HEX2DEC(c0) * 16 + HEX2DEC(c));

	(*outbuf)++;
	*inbuf = (char *)ib;

	return 1;
}

