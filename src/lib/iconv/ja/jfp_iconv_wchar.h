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

#ifndef _JFP_ICONV_WCHAR_H
#define _JFP_ICONV_WCHAR_H

/*
 * macro and definition for wchar_t support
 */

#define E32GETA		0x3000112e /* GETA in 32bit euc */
#define PCKWGETA	0x0000016c /* GETA in PCK wchar */

/* used in __get_pckwchar */
#define __ASCII		0
#define	__PCK_KANA	1
#define	__PCK_KANJI	2

/* prototypes */
wchar_t
__get_eucwchar(int codeset,
	unsigned char ic1, unsigned char ic2);

wchar_t
__get_pckwchar(int codeset,
	unsigned char ic1, unsigned char ic2);

size_t					/* return #bytes read, or -1 */
__read_eucwchar(
	unsigned short	*p,		/* point variable to store euc16 */
	unsigned char	**pip,		/* point pointer to input buf */
	size_t		*pileft,	/* point #bytes left in input buf */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*st);		/* BOM state and endian */

size_t					/* return #bytes read, or -1 */
__read_pckwchar(
	unsigned short	*p,		/* point variable to store PCK value */
	unsigned char	**pip,		/* point pointer to input buf */
	size_t		*pileft,	/* point #bytes left in input buf */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*st);		/* BOM state and endian */

size_t
__replace_hex_wchar(
	unsigned char	hex,	 /* hex value to be replaced */
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd,	 /* state */
	int		caller); /* caller */

size_t
__replace_invalid_wchar(
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd);	 /* state */

size_t
__restore_hex_wchar(
	unsigned char	**pip,	 /* point pointer to input buf */
	size_t		*pileft, /* point #bytes left in input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
        __icv_state_t   *st);	 /* state */

/* macro */

#ifdef __sparc
#define NPUT_WCHAR(wchar, msg) { \
		oc = (unsigned char)((wchar >> 24) & 0x000000ff); \
		NPUT(oc, msg); \
		oc = (unsigned char)((wchar >> 16) & 0x000000ff); \
		NPUT(oc, msg); \
		oc = (unsigned char)((wchar >> 8) & 0x000000ff); \
		NPUT(oc, msg); \
		oc = (unsigned char)((wchar >> 0) & 0x000000ff); \
		NPUT(oc, msg); \
	}

#define NPUT_WASCII(c, msg) { \
		NPUT('\0', msg); \
		NPUT('\0', msg); \
		NPUT('\0', msg); \
		NPUT((c), msg); \
	}
#else
#define NPUT_WCHAR(wchar, msg) { \
		oc = (unsigned char)((wchar >> 0) & 0x000000ff); \
		NPUT(oc, msg); \
		oc = (unsigned char)((wchar >> 8) & 0x000000ff); \
		NPUT(oc, msg); \
		oc = (unsigned char)((wchar >> 16) & 0x000000ff); \
		NPUT(oc, msg); \
		oc = (unsigned char)((wchar >> 24) & 0x000000ff); \
		NPUT(oc, msg); \
	}

#define NPUT_WASCII(c, msg) { \
		NPUT((c), msg); \
		NPUT('\0', msg); \
		NPUT('\0', msg); \
		NPUT('\0', msg); \
	}
#endif

/* see jfp_iconv_common.h */
#define RESTORE_HEX_WCHAR(w) { \
		if ((st->_icv_flag & __ICONV_CONV_RESTORE_HEX) && \
				ileft + sizeof(wchar_t) >= ((PREFIX_LENGTH + 2) * \
					sizeof(wchar_t)) && \
				(((w) == 'I') || ((w) == 'N'))) { \
			switch (__restore_hex_wchar(&ip, &ileft, \
					&op, &oleft, st)) { \
				case (size_t)1: \
					rv = ((size_t)0); \
					goto next; \
				case (size_t)-1: \
					rv = ((size_t)-1); \
					goto ret; \
				default: \
					break; \
			} \
		} \
	}

/* codeset is eucwchar or pckwchar */
#define	GET_WCHAR(__read_codeset, pw) \
	switch ((__read_codeset)((pw), &ip, &ileft, \
		&op, &oleft, st)) { \
	case (size_t)-1: \
		/* errno has been set in __read_codeset() */ \
		rv = (size_t)-1; \
		goto ret; \
	case (size_t)0: \
		/* character read was handled in the __read_codeset() */ \
		/* no further evaluation needed in caller side */ \
		rv = (size_t)0; \
		goto next; \
	default: \
		/* return if null-character is detected, and */ \
		/* ICONV_IGNORE_NULL is not specified */ \
		if ((*pw == 0U) && !(st->_icv_flag & ICONV_IGNORE_NULL)) \
			goto ret; \
		break; \
	}

#define	GET_EUCWCHAR(pw) \
	switch (__read_eucwchar(pw, &ip, &ileft, \
		&op, &oleft, st)) { \
	case (size_t)-1: \
		/* errno has been set in __read_eucwchar() */ \
		rv = (size_t)-1; \
		goto ret; \
	case (size_t)0: \
		/* character read was handled in the __read_eucwchar() */ \
		/* no further evaluation needed in caller side */ \
		rv = (size_t)0; \
		goto next; \
	default: \
		break; \
	}

#endif /* !_JFP_ICONV_WCHAR_H */
