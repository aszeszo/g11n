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

#ifndef	UTF8_TO_UTF_EBCDIC_H
#define	UTF8_TO_UTF_EBCDIC_H

#include "common.h"

#define STATE_T			icv_state_t

#define XPUTC(ob, u4)	\
		if (u4 <= 0x7f) { \
			CHECK_OB(1); \
			*ob++ = I8_UTFEBICDIC((unsigned char)u4); \
		} else if (u4 <= 0x9f) { \
			CHECK_OB(1); \
			*ob++ = I8_UTFEBICDIC((unsigned char)u4); \
		} else if (u4 <= 0x3ff) { \
			CHECK_OB(2); \
			*ob++ = I8_UTFEBICDIC(0xc0 | ((u4 & 0x03e0) >> 5)); \
			*ob++ = I8_UTFEBICDIC(0xa0 |  (u4 & 0x001f)); \
		} else if (u4 <= 0x3fff) { \
			CHECK_OB(3); \
			*ob++ = I8_UTFEBICDIC(0xe0 | ((u4 & 0x3c00) >> 10)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x03e0) >> 5)); \
			*ob++ = I8_UTFEBICDIC(0xa0 |  (u4 & 0x001f)); \
		} else if (u4 <= 0x3ffff) { \
			CHECK_OB(4); \
			*ob++ = I8_UTFEBICDIC(0xf0 | ((u4 & 0x38000) >> 15)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x07c00) >> 10)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x003e0) >> 5)); \
			*ob++ = I8_UTFEBICDIC(0xa0 |  (u4 & 0x0001f)); \
		} else if (u4 <= 0x3fffff) { \
			CHECK_OB(5); \
			*ob++ = I8_UTFEBICDIC(0xf8 | ((u4 & 0x300000) >> 20)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x0f8000) >> 15)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x007c00) >> 10)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x0003e0) >> 5)); \
			*ob++ = I8_UTFEBICDIC(0xa0 |  (u4 & 0x00001f)); \
		} else if (u4 <= 0x3ffffff) { \
			CHECK_OB(6); \
			*ob++ = I8_UTFEBICDIC(0xfc | ((u4 & 0x2000000) >> 25)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x1f00000) >> 20)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x00f8000) >> 15)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x0007c00) >> 10)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x00003e0) >> 5)); \
			*ob++ = I8_UTFEBICDIC(0xa0 |  (u4 & 0x000001f)); \
		} else if (u4 <= 0x7fffffff) { \
			CHECK_OB(7); \
			*ob++ = I8_UTFEBICDIC(0xfe | ((u4 & 0x40000000) >> 30)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x3e000000) >> 25)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x01f00000) >> 20)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x000f8000) >> 15)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x00007c00) >> 10)); \
			*ob++ = I8_UTFEBICDIC(0xa0 | ((u4 & 0x000003e0) >> 5)); \
			*ob++ = I8_UTFEBICDIC(0xa0 |  (u4 & 0x0000001f)); \
		} else { \
			goto ILLEGAL_CHAR; \
		}

#undef PUTC
#define PUTC(ob, u4)	XPUTC(ob, u4)


#define I8_UTFEBICDIC(i8)	i8_to_utf_ebcdic[(i8)]

static 	unsigned char   i8_to_utf_ebcdic[0x100] = {

#include "txt_ebcdic_utf/i8_to_utf_ebcdic.txt"

};

#endif	/* UTF8_TO_UTF_EBCDIC_H */
