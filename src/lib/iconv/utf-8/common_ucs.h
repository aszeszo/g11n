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
 *
 * Common output macros for the UCS-* and UTF-32* encodings.
 */


/*
 * Read in a multibyte character from the input.
 * This function doesn't consume the input buffer.
 * Return values:
 *	0	- character read ok
 *	1	- non-identical character
 *	-1	- incomplete input sequence (set EINVAL)
 *	-2	- illegal character
 */
static int _ucs_getc(int little_endian, char **inbuf, size_t inbufleft,
	uint_t *p1, uint_t *p2);

/*
 * Useful macros
 */
#define CHECK_OB_AND_BOM(sz,bom_var) \
				CHECK_OB(sz); \
				if (! (bom_var)) { \
					PUTC(ICV_BOM_IN_BIG_ENDIAN); \
					(bom_var) = true; \
				}

#ifdef BYTE_TO_HEX
#undef BYTE_TO_HEX
#endif

#define BYTE_TO_HEX(ch)		PUTC(HEX_ARR[ch/16]); \
				PUTC(HEX_ARR[ch%16]);

#ifdef PUT_RHEX
#undef PUT_RHEX
#endif

#define PUT_RHEX(ch,ID)		for (i=0; i < ICV_RHEX_LEN-2; i++) \
					PUTC(ICV_RHEX_PREFIX_##ID[i]); \
				BYTE_TO_HEX(ch)

/* Individual ucs modules redefine this. */
#ifdef PUTC
#undef PUTC
#endif

/* UCS_2, UCS_2BE, UCS_2LE, UCS_2_BIG_ENDIAN, UCS_2_LITTLE_ENDIAN */

#define PUTC_UCS2(u4,le)	if (le) { \
					*ob++ = (uchar_t)(u4 & 0xff); \
					*ob++ = (uchar_t)((u4 >> 8) & 0xff); \
				} else { \
					*ob++ = (uchar_t)((u4 >> 8) & 0xff); \
					*ob++ = (uchar_t)(u4 & 0xff); \
				}

/* UCS_4, UCS_4BE, UCS_4LE, UCS_4_BIG_ENDIAN, UCS_4_LITTLE_ENDIAN,
 * UTF_32, UTF_32BE, UTF_32LE, UTF_32_BIG_ENDIAN, UTF_32_LITTLE_ENDIAN */

#define PUTC_UCS4(u4,le)	if (le) { \
					*ob++ = (uchar_t)(u4 & 0xff); \
					*ob++ = (uchar_t)((u4 >> 8) & 0xff); \
					*ob++ = (uchar_t)((u4 >> 16) & 0xff); \
					*ob++ = (uchar_t)((u4 >> 24) & 0xff); \
				} else { \
					*ob++ = (uchar_t)((u4 >> 24) & 0xff); \
					*ob++ = (uchar_t)((u4 >> 16) & 0xff); \
					*ob++ = (uchar_t)((u4 >> 8) & 0xff); \
					*ob++ = (uchar_t)(u4 & 0xff); \
				}

/* UTF_16, UTF_16BE, UTF_16LE, UTF_16_BIG_ENDIAN, UTF_16_LITTLE_ENDIAN */

#define PUTC_UTF16(u4,u4_2,le)	\
		if (le) { \
			*ob++ = (uchar_t)(u4 & 0xff); \
			*ob++ = (uchar_t)((u4 >> 8) & 0xff); \
			if (u4_2) { \
				*ob++ = (uchar_t)(u4_2 & 0xff); \
				*ob++ = (uchar_t)((u4_2 >> 8) & 0xff); \
			} \
		} else { \
			*ob++ = (uchar_t)((u4 >> 8) & 0xff); \
			*ob++ = (uchar_t)(u4 & 0xff); \
			if (u4_2) { \
				*ob++ = (uchar_t)((u4_2 >> 8) & 0xff); \
				*ob++ = (uchar_t)(u4_2 & 0xff); \
			} \
		}

