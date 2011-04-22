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
 * This particular file is to cover conversions from various UCS formats,
 * especially, UCS-2, UCS-2BE, UCS-2LE, UTF-16, UTF-16BE, and, UTF-16LE to
 * another various UCS formats, UCS-4, UCS-4BE, UCS-4LE, UTF-32, UTF-32BE,
 * and, UTF-32LE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/isa_defs.h>
#include "ucs_to_ucs4.h"


void *
_icv_open_attr(int flag, void *reserved)
{
	STATE_T *cd;

	cd = (STATE_T *)calloc(1, sizeof(STATE_T));
	if (cd == (STATE_T *)NULL) {
		errno = ENOMEM;
		return ((void *)-1);
	}
	cd->flags = flag;

#if defined(UTF_16_BIG_ENDIAN) || defined(UCS_2_BIG_ENDIAN)
	cd->input.little_endian = false;
#elif defined(UTF_16_LITTLE_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN)
	cd->input.little_endian = true;
#elif defined(UTF_16BE) || defined(UCS_2BE)
	cd->input.little_endian = false;
	cd->input.bom_written = true;
#elif defined(UTF_16LE) || defined(UCS_2LE)
	cd->input.little_endian = true;
	cd->input.bom_written = true;
#elif defined(_LITTLE_ENDIAN)
	cd->input.little_endian = true;
#endif

#if defined(UCS_4_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN)
	cd->output.little_endian = false;
#elif defined(UCS_4_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
	cd->output.little_endian = true;
#elif defined(UCS_4BE) || defined(UTF_32BE)
	cd->output.little_endian = false;
	cd->output.bom_written = true;
#elif defined(UCS_4LE) || defined(UTF_32LE)
	cd->output.little_endian = true;
	cd->output.bom_written = true;
#elif defined(_LITTLE_ENDIAN)
	cd->output.little_endian = true;
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


	if (! cd) {
		errno = EBADF;
		return ((size_t)-1);
	}

	/* Reset the state as if it is called right after the iconv_open(). */
	if (!inbuf || !(*inbuf)) {
#if defined(UCS_2) || defined(UTF_16)
#if defined(_LITTLE_ENDIAN)
		cd->input.little_endian = true;
#else
		cd->input.little_endian = false;
#endif
		cd->input.bom_written = false;
#elif defined(UTF_16_BIG_ENDIAN) || defined(UCS_2_BIG_ENDIAN)
		cd->input.little_endian = false;
		cd->input.bom_written = false;
#elif defined(UTF_16_LITTLE_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN)
		cd->input.little_endian = true;
		cd->input.bom_written = false;
#endif

#if defined(UCS_4) || defined(UTF_32) || \
	defined(UCS_4_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN) || \
	defined(UCS_4_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
		cd->output.bom_written = false;
#endif
		return ((size_t)0);
	}

	ib = (uchar_t *)*inbuf;
	ob = (uchar_t *)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;
	f = cd->flags;

#if defined(UCS_2) || defined(UTF_16) || \
	defined(UCS_2_BIG_ENDIAN) || defined(UTF_16_BIG_ENDIAN) || \
	defined(UCS_2_LITTLE_ENDIAN) || defined(UTF_16_LITTLE_ENDIAN)
	if (! cd->input.bom_written) {
		if ((ibtail - ib) < ICV_FETCH_UCS_SIZE)
			ERR_INT(EINVAL);

		for (u4 = 0, i = 0; i < ICV_FETCH_UCS_SIZE; i++)
			u4 = (u4 << 8) | ((uint_t)(*(ib + i)));

		if (u4 == ICV_BOM_IN_BIG_ENDIAN) {
			ib += ICV_FETCH_UCS_SIZE;
			cd->input.little_endian = false;
		} else if (u4 == ICV_BOM_IN_LITTLE_ENDIAN) {
			ib += ICV_FETCH_UCS_SIZE;
			cd->input.little_endian = true;
		}

		cd->input.bom_written = true;
	}
#endif


	while (ib < ibtail) {
		signed char obsz;


		i = _ucs_getc(cd->input.little_endian, (char **)&ib,
		    ibtail - ib, &u4, &u4_2);
		if (i == -1)
			return (-1);
		if (i == -2)
			goto ILLEGAL_CHAR;


		/* Handle RESTORE_HEX */

		if (f) {
			char *prefix = NULL;
			if (f & ICONV_CONV_ILLEGAL_RESTORE_HEX &&
			    u4 == ICV_RHEX_PREFIX_IL[0]) {
				prefix = cd->input.little_endian ?
					ICV_RHEX_PREFIX_IL_2LE :
					ICV_RHEX_PREFIX_IL_2BE;
			} else if (f & ICONV_CONV_NON_IDENTICAL_RESTORE_HEX &&
			    u4 == ICV_RHEX_PREFIX_NI[0]) {
				prefix = cd->input.little_endian ?
					ICV_RHEX_PREFIX_NI_2LE :
					ICV_RHEX_PREFIX_NI_2BE;
			}
			if (prefix &&
			    ibtail - ib >= ICV_RHEX_LEN * ICV_FETCH_UCS_SIZE &&
			    memcmp(ib, prefix, ICV_RHEX_PREFIX_2SZ) == 0) {

				obsz = (cd->output.bom_written) ? 1 : 5;
				CHECK_OB_AND_BOM(obsz, cd->output.bom_written);

				i = _icv_ucs_restore_hex(&_ucs_getc,
				    (char **)&ib, ibtail - ib,
				    (char **)&ob, obtail - ob,
				    ICV_FETCH_UCS_SIZE, cd->input.little_endian);
				if (i == 1)
					continue;
				if (i == -1) {
					ret_val = (size_t)-1;
					break;
				}
			}
		}

		obsz = (cd->output.bom_written) ? 4 : 8;
		CHECK_OB_AND_BOM(obsz, cd->output.bom_written);
		PUTC(u4);
		ib += ((u4_2) ? ICV_FETCH_UCS_SIZE_TWO : ICV_FETCH_UCS_SIZE);
		continue;

ILLEGAL_CHAR:
		/*
		 * Handle ILLEGAL and REPLACE_INVALID
		 */
		if (f) {
			int sz = (u4_2) ? ICV_FETCH_UCS_SIZE_TWO :
				ICV_FETCH_UCS_SIZE;

			if (f & ICONV_CONV_ILLEGAL_DISCARD) {
				ib += sz;
				continue;

			} else if (f & ICONV_CONV_ILLEGAL_REPLACE_HEX) {
				int l;

				/* sz bytes input char, 4 bytes output char */
				obsz = sz * ICV_RHEX_LEN * 4;
				if (! cd->output.bom_written)
					obsz += 4;
				CHECK_OB_AND_BOM(obsz, cd->output.bom_written);
				for (l=0; l < sz; l++) {
					PUT_RHEX(*(ib+l), IL);
				}
				ib += sz;
				continue;

			} else if (f & ICONV_REPLACE_INVALID) {
				obsz = (cd->output.bom_written) ? 4 : 8;
				CHECK_OB_AND_BOM(obsz, cd->output.bom_written);
				PUTC(ICV_CHAR_UCS2_REPLACEMENT);
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
#elif defined(UCS_4) || defined(UCS_4BE) || defined(UCS_4LE) || \
      defined(UTF_32) || defined(UTF_32BE) || defined(UTF_32LE) || \
      defined(UCS_4_BIG_ENDIAN) || defined(UTF_32_BIG_ENDIAN) || \
      defined(UCS_4_LITTLE_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)
	/*
	 * We do nothing here since these if expressions are
	 * only for input characters particularly of
	 * UCS-2, UCS-2BE, UCS-2LE, UTF-16, UTF-16BE, and
	 * UTF-16LE.
	 */
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

int _icv_iconvctl(STATE_T *cd, int req, void *arg)
{
	return _icv_flag_action(&cd->flags, req, (int *)arg,
	    ICONVCTL_NO_TRANSLIT);
}

