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
 * Copyright (c) 1998, 2011, Oracle and/or its affiliates. All rights reserved.
 *
 * Following is how we process BOM and subsequent bytes in this program:
 * - UCS-2BE, UTF-16BE, UCS-4BE, UTF-32BE, UCS-2LE, UTF-16LE, UCS-4LE, and
 *   UTF-32LE don't care about BOM. From the beginning, they are properly
 *   serialized without the BOM character; any BOM is treated as ZWNBSP.
 * - In other encodings, UCS-2, UCS-4, UTF-16, and UTF-32, the initial byte
 *   ordering is of the current processor's byte ordering. During the first
 *   iconv() call, if BOM appears as the first character of the entier
 *   iconv input stream, the byte order will be changed accordingly.
 *   We will use 'bom_written' data field of the conversion descriptor to
 *   save this particular information, in other words, whether we've been
 *   encountered the first character as the BOM.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/isa_defs.h>
#include "ucs_to_utf8.h"


void *
_icv_open_attr(int flag, void *reserved)
{
	STATE_T *cd = (STATE_T *)calloc(1, sizeof(STATE_T));

	if (cd == (STATE_T *)NULL) {
		errno = ENOMEM;
		return ((void *)-1);
	}
	cd->flags = flag;

#if defined(UTF_16_BIG_ENDIAN) || defined(UCS_2_BIG_ENDIAN) || \
	defined(UCS_4_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN)
	cd->little_endian = false;
#elif defined(UTF_16_LITTLE_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN) || \
	defined(UCS_4_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
	cd->little_endian = true;
#elif defined(UTF_16BE) || defined(UCS_2BE) || defined(UCS_4BE) || \
	defined(UTF_32BE)
	cd->little_endian = false;
	cd->bom_written = true;
#elif defined(UTF_16LE) || defined(UCS_2LE) || defined(UCS_4LE) || \
	defined(UTF_32LE)
	cd->little_endian = true;
	cd->bom_written = true;
#elif defined(_LITTLE_ENDIAN)
	cd->little_endian = true;
#endif

	return ((void *)cd);
}


size_t
_icv_iconv(STATE_T *cd, char **inbuf, size_t *inbufleft,
	char **outbuf, size_t *outbufleft)
{
	size_t ret_val = 0;
	uchar_t *ib;
	uchar_t *ob;
	uchar_t *ibtail;
	uchar_t *obtail;
	uint_t u4;
	uint_t u4_2;
	int i, f;
	signed char sz;


	if (! cd) {
		errno = EBADF;
		return ((size_t)-1);
	}

	/* Reset the state as if it is called right after the iconv_open(). */
	if (!inbuf || !(*inbuf)) {
#if defined(UCS_2) || defined(UCS_4) || defined(UTF_16) || defined(UTF_32)
#if defined(_LITTLE_ENDIAN)
		cd->little_endian = true;
#else
		cd->little_endian = false;
#endif
		cd->bom_written = false;
#elif defined(UTF_16_BIG_ENDIAN) || defined(UCS_2_BIG_ENDIAN) || \
	defined(UCS_4_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN)
		cd->little_endian = false;
		cd->bom_written = false;
#elif defined(UTF_16_LITTLE_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN) || \
	defined(UCS_4_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
		cd->little_endian = true;
		cd->bom_written = false;
#endif
		return ((size_t)0);
	}

	ib = (uchar_t *)*inbuf;
	ob = (uchar_t *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;
	f = cd->flags;

#if defined(UCS_2) || defined(UCS_4) || defined(UTF_16) || defined(UTF_32) || \
	defined(UCS_2_BIG_ENDIAN) || defined(UTF_16_BIG_ENDIAN) || \
	defined(UCS_4_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN) || \
	defined(UCS_2_LITTLE_ENDIAN) || defined(UTF_16_LITTLE_ENDIAN) || \
	defined(UCS_4_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
	if (! cd->bom_written) {
		if ((ibtail - ib) < ICV_FETCH_UCS_SIZE) {
			if (f & ICONV_REPLACE_INVALID) {
				sz = ibtail - ib;
				goto INCOMPLETE_CHAR;
			}
			ERR_INT(EINVAL);
		}

		for (u4 = 0, i = 0; i < ICV_FETCH_UCS_SIZE; i++)
			u4 = (u4 << 8) | ((uint_t)(*(ib + i)));

		/* Big endian, Little endian, or, not specified?? */
		if (u4 == ICV_BOM_IN_BIG_ENDIAN) {
			ib += ICV_FETCH_UCS_SIZE;
			cd->little_endian = false;
		} else if (u4 == ICV_BOM_IN_LITTLE_ENDIAN) {
			ib += ICV_FETCH_UCS_SIZE;
			cd->little_endian = true;
		}

		/*
		 * Once BOM checking is done, regardless of whether we had
		 * the BOM or not, we treat the BOM sequence as a ZWNBSP
		 * character from now on.
		 */
		cd->bom_written = true;
	}
#endif

	while (ib < ibtail) {

		i = _ucs_getc(cd->little_endian, (char **)&ib,
		    ibtail - ib, &u4, &u4_2);
		sz = (u4_2) ? ICV_FETCH_UCS_SIZE_TWO : ICV_FETCH_UCS_SIZE;
		if (i == -1) {
			if (errno == EINVAL && f & ICONV_REPLACE_INVALID) {
				sz = ibtail - ib;
				goto INCOMPLETE_CHAR;
			}
			return (-1);
		}
		if (i == -2)
			goto ILLEGAL_CHAR;


		/* Handle RESTORE_HEX */

		if (f) {
			char *prefix = NULL;
			if (f & ICONV_CONV_ILLEGAL_RESTORE_HEX &&
			    u4 == ICV_RHEX_PREFIX_IL[0]) {
				prefix = cd->little_endian ?
					ICV_RHEX_PREFIX_IL_LE :
					ICV_RHEX_PREFIX_IL_BE;
			} else if (f & ICONV_CONV_NON_IDENTICAL_RESTORE_HEX &&
			    u4 == ICV_RHEX_PREFIX_NI[0]) {
				prefix = cd->little_endian ?
					ICV_RHEX_PREFIX_NI_LE :
					ICV_RHEX_PREFIX_NI_BE;
			}
			if (prefix &&
			    ibtail - ib >= ICV_RHEX_LEN * ICV_FETCH_UCS_SIZE &&
			    memcmp(ib, prefix, ICV_RHEX_PREFIX_SZ) == 0) {

				i = _icv_ucs_restore_hex(&_ucs_getc,
				    (char **)&ib, ibtail - ib,
				    (char **)&ob, obtail - ob,
				    ICV_FETCH_UCS_SIZE, cd->little_endian);
				if (i == 1)
					continue;
				if (i == -1) {
					ret_val = (size_t)-1;
					break;
				}
			}
		}

		/*
		 * Once we reach here, the "u4" contains a valid character
		 * and thus we don't do any other error checking in
		 * the below.
		 */
		if (u4 <= 0x7f) { 
			CHECK_OB(1);
			*ob++ = (uchar_t)u4;
		} else if (u4 <= 0x7ff) {
			CHECK_OB(2);
			*ob++ = (uchar_t)(0xc0 | ((u4 & 0x07c0) >> 6));
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x003f));
		} else if (u4 <= 0x00ffff) {
			CHECK_OB(3);
			*ob++ = (uchar_t)(0xe0 | ((u4 & 0x0f000) >> 12));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00fc0) >> 6));
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x0003f));
		} else if (u4 <= 0x1fffff) {
			CHECK_OB(4);
			*ob++ = (uchar_t)(0xf0 | ((u4 & 0x01c0000) >> 18));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x003f000) >> 12));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x0000fc0) >> 6));
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x000003f));
		} else if (u4 <= 0x3ffffff) {
			CHECK_OB(5);
			*ob++ = (uchar_t)(0xf8 | ((u4 & 0x03000000) >> 24));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00fc0000) >> 18));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x0003f000) >> 12));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00000fc0) >> 6));
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x0000003f));
		} else {
			CHECK_OB(6);
			*ob++ = (uchar_t)(0xfc | ((u4 & 0x40000000) >> 30));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x3f000000) >> 24));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00fc0000) >> 18));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x0003f000) >> 12));
			*ob++ = (uchar_t)(0x80 | ((u4 & 0x00000fc0) >> 6));
			*ob++ = (uchar_t)(0x80 |  (u4 & 0x0000003f));
		}
		ib += ((u4_2) ? ICV_FETCH_UCS_SIZE_TWO : ICV_FETCH_UCS_SIZE);
		continue;

ILLEGAL_CHAR:
		/*
		 * Handle ILLEGAL and REPLACE_INVALID
		 */
		if (f) {
			if (f & ICONV_CONV_ILLEGAL_DISCARD) {
				ib += sz;
				continue;

			} else if (f & ICONV_CONV_ILLEGAL_REPLACE_HEX) {
				int l;

				CHECK_OB(sz * ICV_RHEX_LEN);
				for (l=0; l < sz; l++) {
					PUT_RHEX(*(ib+l), ob, IL);
				}
				ib += sz;
				continue;

			} else if (f & ICONV_REPLACE_INVALID) {
				int obsz;
INCOMPLETE_CHAR:
				obsz = ICV_CHAR_UTF8_REPLACEMENT_SIZE;
				CHECK_OB(obsz);
				for (i = 1; i <= obsz; i++)
					*ob++ = (unsigned int)
					    ((ICV_CHAR_UTF8_REPLACEMENT >>
					    ((obsz - i) * 8)) & 0xff);
				ib += sz;
				ret_val++;
				continue;
			}
		}

		/* Default scenario */
		ERR_INT(EILSEQ);
	}

_INTERRUPT:
	*inbuf = (char *)ib;
	*inbufleft = ibtail - ib;
	*outbuf = (char *)ob;
	*outbufleft = obtail - ob;

	return (ret_val);
}


int
_ucs_getc(int little_endian, char **inbuf, size_t inbufleft,
	uint_t *p1, uint_t *p2)
{
	size_t ret_val = 0;
	unsigned char *ib;
	unsigned char *ibtail;
	int i;
	uint_t u4,u4_2;


	ib = (unsigned char *)*inbuf;
	ibtail = ib + inbufleft;

	if ((ibtail - ib) < ICV_FETCH_UCS_SIZE)
		ERR_INT(EINVAL);

	u4 = u4_2 = 0;
	if (little_endian) {
		for (i = ICV_FETCH_UCS_SIZE - 1; i >= 0; i--)
			u4 = (u4 << 8) | ((uint_t)(*(ib + i)));
	} else {
		for (i = 0; i < ICV_FETCH_UCS_SIZE; i++)
			u4 = (u4 << 8) | ((uint_t)(*(ib + i)));
	}

#if defined(UCS_2) || defined(UCS_2BE) || defined(UCS_2LE) || \
    defined(UCS_2_BIG_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN)
	if (u4 >= 0x00fffe || (u4 >= 0x00d800 && u4 <= 0x00dfff)) {
		goto ILLEGAL_CHAR;
	}
#elif defined(UTF_16) || defined(UTF_16BE) || defined(UTF_16LE) || \
      defined(UTF_16_BIG_ENDIAN) || defined(UTF_16_LITTLE_ENDIAN)
	if ((u4 >= 0x00dc00 && u4 <= 0x00dfff) || u4 >= 0x00fffe)
		goto ILLEGAL_CHAR;

	if (u4 >= 0x00d800 && u4 <= 0x00dbff) {
		if ((ibtail - ib) < ICV_FETCH_UCS_SIZE_TWO)
			ERR_INT(EINVAL);;

		if (little_endian) {
			for (i = ICV_FETCH_UCS_SIZE_TWO - 1;
				i >= ICV_FETCH_UCS_SIZE;
					i--)
				u4_2 = (u4_2<<8)|((uint_t)(*(ib + i)));
		} else {
			for (i = ICV_FETCH_UCS_SIZE;
				i < ICV_FETCH_UCS_SIZE_TWO;
					i++)
				u4_2 = (u4_2<<8)|((uint_t)(*(ib + i)));
		}

		if (u4_2 < 0x00dc00 || u4_2 > 0x00dfff)
			goto ILLEGAL_CHAR;

		u4 = ((((u4 - 0x00d800) * 0x400) +
			(u4_2 - 0x00dc00)) & 0x0fffff) + 0x010000;
	}
#elif defined(UTF_32) || defined(UTF_32BE) || defined(UTF_32LE) || \
      defined(UTF_32_BIG_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
	if (u4 == 0x00fffe || u4 == 0x00ffff || u4 > 0x10ffff ||
	    (u4 >= 0x00d800 && u4 <= 0x00dfff)) {
		goto ILLEGAL_CHAR;
	}
#elif defined(UCS_4) || defined(UCS_4BE) || defined(UCS_4LE) || \
      defined(UCS_4_BIG_ENDIAN) || defined(UCS_4_LITTLE_ENDIAN)
	if (u4 == 0x00fffe || u4 == 0x00ffff || u4 > 0x7fffffff ||
	    (u4 >= 0x00d800 && u4 <= 0x00dfff)) {
		goto ILLEGAL_CHAR;
	}
#else
#error	"Fatal: one of the UCS macros need to be defined."
#endif

_INTERRUPT:
	*p1 = u4;
	*p2 = u4_2;
	return ret_val;

ILLEGAL_CHAR:
	*p1 = u4;
	*p2 = u4_2;
	return -2;
}


int
_icv_iconvctl(STATE_T *cd, int req, void *arg)
{
	return _icv_flag_action(&cd->flags, req, (int *)arg, 0);
}

size_t
_icv_iconvstr(char *inarray, size_t *inlen, char *outarray,
	size_t *outlen, int flags)
{
	return _icv_ciconvstr(inarray, inlen, outarray, outlen, flags,
		ICV_FETCH_UCS_SIZE);
}

