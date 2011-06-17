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

#include "common_defs.h"

/*
 * Common implementation of _icv_open.
 */
void *_icv_open(const char *strp);

/*
 * Common implementation of _icv_close.
 */
void _icv_close(iconv_t *cd);

/*
 * Common implementation of _icv_iconvstr.
 */
size_t _icv_ciconvstr(char *inarray, size_t *inlen, char *outarray,
	size_t *outlen, int flags, int chsz);

/*
 * Perform the RESTORE_HEX action for ICONV_CONV_ILLEGAL_RESTORE_HEX
 * or ICONV_CONV_NON_IDENTICAL_RESTORE_HEX, adjust inbuf and outbuf.
 *
 * This function should be called when a valid prefix of the RESTORE_HEX
 * sequence (the first 4 characters, e.g. { 'I', 'L', '-', '-' }) was
 * found in the inbuf. The function skips such prefix and creates the byte
 * from the following 2 inbuf characters into the outbuf. See iconvctl(3c).
 *
 * Returns:
 * 	1	valid sequence found and restored
 * 	0	no valid sequence
 * 	-1 	error (set errno)
 * 
 */
int _icv_restore_hex(char **inbuf, size_t inbufleft, char **outbuf,
	size_t outbufleft);

/*
 * Multibyte variant of _icv_restore_hex, using given _getc() to
 * obtain input characters.
 */
int _icv_ucs_restore_hex(int (*_getc)(int, char **, size_t, uint_t *, uint_t *),
	char **inbuf, size_t inbufleft, char **outbuf, size_t outbufleft,
	int chsz, int little_endian); 


/*
 * Flag operation on iconv_t flags for _icv_iconvctl.
 * The 'attributes' argument allows for disabling features of the conversion.
 */
int _icv_flag_action(int *flags, int req, int *arg, int attributes);

