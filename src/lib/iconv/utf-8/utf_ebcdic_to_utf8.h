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
 * Copyright (c) 1999, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#ifndef	UTF_EBCDIC_TO_UTF8_H
#define	UTF_EBCDIC_TO_UTF8_H

#include "common.h"

#define STATE_T			icv_state_t

#define	ICV_RHEX_PREFIX_IL_EBCDIC	"\xc9\xd3\x60\x60"
#define	ICV_RHEX_PREFIX_NI_EBCDIC	"\xd5\xc9\x60\x60"

#define XPUTC(ob, u4)	\
		if (u4 <= 0x7f) { \
			CHECK_OB(1); \
			*ob++ = (uchar_t)u4; \
		} else if (u4 <= 0x7ff) { \
			CHECK_OB(2); \
			*ob++ = (uchar_t)(0xc0 | ((u4 & 0x07c0) >> 6)); \
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x003f)); \
		} else if (u4 <= 0xd7ff) { \
			CHECK_OB(3); \
			*ob++ = (uchar_t)(0xe0 | ((u4 & 0x0f000) >> 12)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00fc0) >> 6)); \
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x0003f)); \
		} else if (u4 <= 0x00dfff) { \
			/* S zone */ \
			goto ILLEGAL_CHAR; \
		} else if (u4 <= 0x00fffd) { \
			CHECK_OB(3); \
			*ob++ = (uchar_t)(0xe0 | ((u4 & 0x0f000) >> 12)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00fc0) >> 6)); \
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x0003f)); \
		} else if (u4 <= 0x00ffff) { \
			goto ILLEGAL_CHAR; \
		} else if (u4 <= 0x1fffff) { \
			CHECK_OB(4); \
			*ob++ = (uchar_t)(0xf0 | ((u4 & 0x01c0000) >> 18)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x003f000) >> 12)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x0000fc0) >> 6)); \
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x000003f)); \
		} else if (u4 <= 0x3ffffff) { \
			CHECK_OB(5); \
			*ob++ = (uchar_t)(0xf8 | ((u4 & 0x03000000) >> 24)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00fc0000) >> 18)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x0003f000) >> 12)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00000fc0) >> 6)); \
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x0000003f)); \
		} else if (u4 <= 0x7fffffff) { \
			CHECK_OB(6); \
			*ob++ = (uchar_t)(0xfc | ((u4 & 0x40000000) >> 30)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x3f000000) >> 24)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00fc0000) >> 18)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x0003f000) >> 12)); \
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00000fc0) >> 6)); \
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x0000003f)); \
		} else { \
			goto ILLEGAL_CHAR; \
		}

#undef PUTC
#define PUTC(ob, u4)	XPUTC(ob, u4)


static 	unsigned int utf_ebcdic_to_i8[0x100] = {

#include "txt_ebcdic_utf/utf_ebcdic_to_i8.txt"

};


/*
 * shadow flag defined in specification.
 */
static 	signed char number_of_bytes_in_utf_ebcdic[0x100] = {

#include "txt_ebcdic_utf/shadow.txt"

};

#define UTF_EBCDIC_LEAD_OCTET_MAX 7
#define UTF_EBCDIC_LEAD_OCTET_MIN 0 /* Control Character */
#define UTF_EBCDIC_TRAILING_OCTET 9


#define UTF_EBCDIC_I8(utfe)	utf_ebcdic_to_i8[(utfe)]

/*
 * Following is a vector of bit-masks to get used bits in the first byte of
 * a UTF-EBCDIC character.  Index is 0 for control character or the number 
 * of bytes in the UTF-EBCDIC character.
 * and the index value comes from above table.
 */
static const uchar_t utf_ebcdic_masks_tbl[UTF_EBCDIC_LEAD_OCTET_MAX+1] =
	{ 0xff, 0xff, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x01};
/*	     0     1     2     3     4     5     6     7	*/

#define	UTF_EBCDIC_BIT_SHIFT		5
#define	UTF_EBCDIC_BIT_MASK		0x1f



/* Hex conversion macro */
#define HEX2DEC(c)	((c) >= 'a' ? (c)-'a'+10 : \
			 (c) >= 'A' ? (c)-'A'+10 : (c)-'0')

/* Custom restore hex function. */
int _icv_ebcdic_restore_hex(char **inbuf, size_t inbufleft, char **outbuf,
	size_t outbufleft);


#endif	/* UTF_EBCDIC_TO_UTF8_H */
