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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <iconv.h>
#include <errno.h>
#include "japanese.h"
#include "jfp_iconv_common.h"

/*
 * This file contains implementation of common methods, and local
 * functions to be called from conversion modules.
 */

/* 
 * implementation of common methods
 * The following methods are not implemented here because it's different
 * between each conversion modules. They are implemented in each 
 * modules.
 *
 * _icv_open_attr(), _icv_iconv()
 *
 */

iconv_t
_icv_open(const char *strp)
{
	return(_icv_open_attr(0, NULL));
}

void
_icv_close(iconv_t cd)
{
	if (cd == NULL) {
		errno = EBADF;
	} else {
		free(cd);
	}
	return;
}

int
_icv_iconvctl(
	iconv_t	st,
	int	request,
	void	*argv)
{
	int	rv = 0;	/* return value */
	int	*arg = argv;

	__icv_state_t *cd = (__icv_state_t *)st;

	switch (request) {
		case ICONV_GET_CONVERSION_BEHAVIOR:
			*arg = cd->_icv_flag;
			rv = cd->_icv_flag;
			break;

		case ICONV_SET_CONVERSION_BEHAVIOR:
			cd->_icv_flag = *arg;
			break;

		case ICONV_GET_DISCARD_ILSEQ:
			if (cd->_icv_flag & __ICONV_CONV_DISCARD_ILSEQ)
			{
				*arg = 1;
			} else {
				*arg = 0;
			}
			break;

		case ICONV_SET_DISCARD_ILSEQ:
			/*
			 * When ICONV_SET_DISCARD_ILSEQ is specified,
			 * ICONV_CONV_ILLEGAL_DISCARD and
			 * ICONV_CONV_NON_IDENTICAL_DISCARD are on, and
			 * all other bit should be off.
			 */
			if (*arg != 0) {
				cd->_icv_flag = 
					__ICONV_CONV_DISCARD_ILSEQ;
			/*
			 * It's specified to set off 
			 * ICONV_CONV_ILLEGAL_DISCARD and
			 * ICONV_CONV_NON_IDENTICAL_DISCARD, but other
			 * bit may already on.
			 */ 
			} else {
				cd->_icv_flag &= 
					~__ICONV_CONV_DISCARD_ILSEQ;
			}
			break;

		case ICONV_GET_TRANSLITERATE:
			if (cd->_icv_flag & 
				ICONV_CONV_NON_IDENTICAL_TRANSLITERATE) {
				*arg = 1;
			} else {
				*arg = 0;
			}
			break;

		case ICONV_SET_TRANSLITERATE:
			/* It has not yet been implemented in ja modules */
			errno = ENOTSUP;
			rv = -1;
			break;

		case ICONV_TRIVIALP:
			/*
			 * cd->trivialp is set when _icv_open() or 
			 * _icv_open_attr() is called to initialize conversion
			 * descriptor.
			 */
			*arg = cd->trivialp;
			break;
	}

	return(rv);
}

size_t
_icv_iconvstr(
	char	*inarray,
	size_t	*inlen,
	char	*outarray,
	size_t	*outlen,
	int	flag)
{
	char *np;
	size_t len, t;
	size_t ret;
        iconv_t cd;

	if (!(flag & ICONV_IGNORE_NULL) &&

#if	defined(JFP_ICONV_FROMCODE_UTF32) || \
	defined(JFP_ICONV_FROMCODE_UTF16) || \
	defined(JFP_ICONV_FROMCODE_UCS2) || \
	defined(JFP_ICONV_FROMCODE_WCHAR)

	    (np = (char *)__index_of_nullchar((const void *)inarray,
			*inlen)) != NULL)

#else /* ASCII compatible incl. UTF-8 */

	    (np = (char *)memchr((const void *)inarray, '\0', *inlen))
			!= NULL)

#endif
	{
		len = np - inarray;
	} else {
		len = *inlen;
	}

	t = len;
	flag &= (ICONV_IGNORE_NULL | ICONV_REPLACE_INVALID);

	if ((cd = (iconv_t)_icv_open_attr(flag, NULL)) == (iconv_t)-1) {
		return (size_t)-1;
	}

	ret = _icv_iconv(cd, (const char **)(&inarray), 
		&len, &outarray, outlen);

	/*
	 * When above _icv_iconv() return without errors, the 
	 * conversion descriptor is reseted to clear the conversion
	 * state.
	 *
	 * When error is caused (return -1) and errno is EILSEQ,
	 * it also try to reset. It's probably possible and variable to
	 * reset. The return value is not checked in this trying, and
	 * _icv_iconvstr keep errno as EILSEQ even if reseting
	 * _icv_iconv() set another value while resetting.
	 *
	 * Other cases (EINVAL or E2BIG), no need to reset the 
	 * conversion descriptor.
	 */
	if (ret != -1) {
		/* reset conversion descriptor to clear the state */ 
		if (_icv_iconv(cd, NULL, &len, &outarray, outlen) 
			== (size_t)-1) {
			ret = (size_t)-1;
		}
	} else if ((ret == (size_t)-1) && (errno = EILSEQ)) {
		_icv_iconv(cd, NULL, &len, &outarray, outlen);
		errno = EILSEQ;
	}

	_icv_close(cd);

	*inlen -= (t - len);

	return(ret);
}

/*
 * local functions to be called from conversion modules. That name
 * starts with double under score '__'.
 */
 
/*
 * __icv_open_attr
 *
 * It's expected to be called from _icv_open_attr() implementation in
 * conversion modules. It does:
 *   - allocate data structure
 *   - initialize members
 */
__icv_state_t *
__icv_open_attr(int flag)
{
	__icv_state_t 	*st;

	if ((st = (__icv_state_t *)malloc(sizeof(__icv_state_t))) 
			== NULL) {
		errno = ENOMEM;
		return ((__icv_state_t*)-1);
	}

	/* set flag */
	st->_icv_flag = flag;

	/* set default value into trivialp */
	st->trivialp = __NOT_TRIVIALP;

	/* initialze all other values */
	st->_st_cset = ST_INIT;
	st->_st_cset_sav = ST_INIT;
	st->bom_written = B_FALSE;
	st->little_endian = B_FALSE;
	st->replacement = NULL;
	st->tmpbuf = NULL;

	return (st);
}

/*
 * __icv_illegal
 *
 * process input byte sequence as specified by _icv_flag. This function
 * is called only when ICONV_CONV_ILLEGAL_* is specified. It's needed 
 * to know which byte (1st, 2nd, 3rd, or 4th) is illegal. The bytes up 
 * to illegal byte is processed. For example,
 *
 *   - byte sequence is: WWXXYYZZ
 *   - WW is EUC-JP 1st byte
 *   - XX is detected as illegal byte of EUC-JP 2nd byte
 *
 * In above case, this function is called when 0xXX is detected as 
 * illegal byte. 0xXX is illegal EUC-JP 2nd byte, but it may be 
 * appropriate EUC-JP 1st byte. It's difficult to know what's going on 
 * (2nd byte is corrupted, or 2nd byte is truncated). 0xWW is processed
 * as illegal, and the next byte should be 0xXX. Pointer of input 
 * buffer is decremented, and number of byte-left is incremented.
 *
 * To support case like above, the 5th argument of this function is a 
 * number of bytes when detected. It's used to decrement the pointer of
 * input buffer.
 *
 * When this function is called:
 *
 *  ip - 2  : pointing WW
 *  ip - 1  : pointing XX
 *  ip      : pointing YY
 * 
 * When return from this function:
 * 
 *  WW is processed as illegal (discard, replace_hex, or restore_hex)
 *  ip      : pointing XX
 *  ileft   : incremented to point XX
 *
 * a corner case
 * When input buffer ends with incomplete multi-byte sequence
 * (in other words, not ends with last byte of multi-byte sequence),
 * _icv_iconv() return with EINVAL. It's not processed by this.
 * It may be possible to process the last byte as illegal byte
 * (discard, replace_hex, or restore_hex). However, EILSEQ and EINVAL
 * are defined for each purpose. This __icv_illegal is implemented
 * to process when EILSEQ is reporting, not process for EINVAL.
 *
 */
size_t
__icv_illegal(
	unsigned char   **pip,	 /* point pointer to input buf */
	size_t		*pileft, /* point #bytes left in input buf */
	char            **pop,	 /* point pointer to output buf */
	size_t          *poleft, /* point #bytes left in output buf */
	__icv_state_t   *cd,	 /* state */
	int		num_of_bytes) /* byte number (1st, 2nd,..) */
{
	unsigned char	*ip = *pip;
	size_t		ileft = *pileft;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* retrun value */

#if	defined(JFP_ICONV_FROMCODE_UTF32) || \
	defined(JFP_ICONV_FROMCODE_UTF16) || \
	defined(JFP_ICONV_FROMCODE_UCS2) || \
	defined(JFP_ICONV_FROMCODE_WCHAR)

	int i;
	if (cd->_icv_flag & ICONV_CONV_ILLEGAL_DISCARD) {
		/* nop */
	} else if (cd->_icv_flag & ICONV_CONV_ILLEGAL_REPLACE_HEX) {
		/* put replacement character for illegal byte */
		for (i = num_of_bytes; i > 0; i--) {
			rv = __replace_hex(*(ip - i), &ip, &op, &oleft,
				cd, __ICV_ILLEGAL);
		}
	} else if (cd->_icv_flag & ICONV_REPLACE_INVALID) {
		/* replace with pre-defined character */
		for (i = num_of_bytes; i > 0; i--) {
			rv = __replace_invalid(&ip, &op, &oleft, cd);
		}
	}
#else
	if (cd->_icv_flag & ICONV_CONV_ILLEGAL_DISCARD) {
		/* nop */
	} else if (cd->_icv_flag & ICONV_CONV_ILLEGAL_REPLACE_HEX) {
		/* put replacement character for illegal byte */
		rv = __replace_hex(*(ip - num_of_bytes), &ip, 
			&op, &oleft, cd, __ICV_ILLEGAL);
	} else if (cd->_icv_flag & ICONV_REPLACE_INVALID) {
		/* replace with pre-defined character */
		rv = __replace_invalid(&ip, &op, &oleft, cd);
	}

	/*
	 * 1 byte is processed as illegal, next processed byte is a
	 * next byte of this illegal one.
	 */
	ip = ip - num_of_bytes + 1;
	ileft = ileft + num_of_bytes - 1;
#endif

ret:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pip = ip;
		*pileft = ileft;
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

/*
 * __icv_non_identical()
 *
 * It's called when writing character is the representative of Unicode 
 * replacement char (0xfffd), or ascii question mark ('?'), or
 * "GETA" mark (jis 0x222e)
 *
 * When this function is called, ip is already pointing next byte,
 * not non-identical byte sequence. The fifth argument is the number
 * of non-identical bytes. For example, 
 *
 *   - byte sequence is: WWXXYYZZ
 *   - WWXXYY is appropriate byte sequence, but not exist in
 *     target code.
 *
 * When this function is called:
 *
 *   ip - 3: WW
 *   ip - 2: XX
 *   ip - 1: YY
 *   ip    : ZZ
 *
 * WWXXYY is processed as non-identical bytes.
 * 
 */
size_t
__icv_non_identical(
        unsigned char   **pip,	 /* point pointer to input buf */
        char            **pop,	 /* point pointer to output buf */
        size_t          *poleft, /* point #bytes left in output buf */
        __icv_state_t   *cd,	 /* state */
        int             num_of_bytes) /* num of non-identical bytes */
{
	unsigned char	*ip = *pip;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;

	int i;

	if (cd->_icv_flag & ICONV_CONV_NON_IDENTICAL_DISCARD) {
		return (rv);
	} else if (cd->_icv_flag & ICONV_CONV_NON_IDENTICAL_REPLACE_HEX) {
		for (i = num_of_bytes; i > 0; i--) {
			rv = __replace_hex(*(ip - i), &ip, &op, &oleft, 
				cd, __ICV_NON_IDENTICAL);
		}
	} 

ret:
	if (rv != (size_t)-1) {
		/* update *pop and *poleft only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

size_t
__replace_hex_ascii(
	unsigned char	hex,	 /* hex to write */
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd,	 /* state */
	int		caller)	 /* caller */
{
	unsigned char	*ip = *pip;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* retrun value */
	
	unsigned char	first_half = ((hex >> 4) & 0x0f);
	unsigned char	second_half = (hex & 0x0f);

	if ((cd->tmpbuf != NULL) && !(caller & __NEXT_OF_ESC_SEQ)) {
		if((rv = __next_of_esc_seq(&ip, &op, &oleft,
				cd, caller)) == -1) {
			goto ret;
		}
	}

	if (first_half < 0xa) {
		first_half += 0x30;
	} else {
		first_half += 0x37;
	}

	if (second_half < 0xa) {
		second_half += 0x30;
	} else {
		second_half += 0x37;
	}

	if ((caller & ~__NEXT_OF_ESC_SEQ) == __ICV_ILLEGAL) {
		NPUT('I', "REPLACE_HEX");
		NPUT('L', "REPLACE_HEX");
	} else { /* __ICV_NON_IDENTICAL */
		NPUT('N', "REPLACE_HEX");
		NPUT('I', "REPLACE_HEX");
	}
	NPUT('-', "REPLACE_HEX");
	NPUT('-', "REPLACE_HEX");
	NPUT((char)first_half, "REPLACE_HEX");
	NPUT((char)second_half, "REPLACE_HEX");

ret:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

size_t
__replace_hex_iso2022jp(
	unsigned char	hex,	 /* hex to write */
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
        __icv_state_t   *cd,	 /* state */
	int		caller)	 /* caller */
{
	unsigned char	*ip = *pip;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* retrun value */
	
	unsigned char	first_half = ((hex >> 4) & 0x0f);
	unsigned char	second_half = (hex & 0x0f);

	if (first_half < 0xa) {
		first_half += 0x30;
	} else {
		first_half += 0x37;
	}

	if (second_half < 0xa) {
		second_half += 0x30;
	} else {
		second_half += 0x37;
	}

	if (cd->_st_cset != CS_0) { /* not in ascii mode */
		NPUT(0x1b, "RELACE_HEX"); /* <ESC> */
		NPUT('(', "REPLACE_HEX");
#if defined(JFP_ICONV_TOCODE_2022JP2004)
		NPUT('B', "REPLACE_HEX");
#else
		NPUT('J', "REPLACE_HEX");
#endif
	}

	if (caller == __ICV_ILLEGAL) {
		NPUT('I', "REPLACE_HEX");
		NPUT('L', "REPLACE_HEX");
	} else { /* __ICV_NON_IDENTICAL */
		NPUT('N', "REPLACE_HEX");
		NPUT('I', "REPLACE_HEX");
	}
	NPUT('-', "REPLACE_HEX");
	NPUT('-', "REPLACE_HEX");
	NPUT((char)first_half, "REPLACE_HEX");
	NPUT((char)second_half, "REPLACE_HEX");

ret:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

size_t 
__replace_invalid_ascii(
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd)	 /* state */
{
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* retrun value */

	unsigned char	byte_buf;

	/*
	 * put each byte to op. cd->replacement is expected to be up
	 * to 0xffffff (3 byte char)
         */
	if (cd->replacement <= 0xff) {
		byte_buf = (unsigned char)cd->replacement;
		NPUT(byte_buf, "REPLACE_INVALID");
	} else if ((cd->replacement > 0xff) && 
			(cd->replacement <= 0xffff)) {
		byte_buf = ((cd->replacement >> 8) & 0x00ff);
		NPUT(byte_buf, "REPLACE_INVALID");
		byte_buf = (cd->replacement & 0x00ff);
		NPUT(byte_buf, "REPLACE_INVALID");
	} else { /* (cd->replacement > 0xffff) */
		byte_buf = ((cd->replacement >> 16) & 0x00ff);
		NPUT(byte_buf, "REPLACE_INVALID");
		byte_buf = ((cd->replacement >> 8) & 0x00ff);
		NPUT(byte_buf, "REPLACE_INVALID");
		byte_buf = (cd->replacement & 0x00ff);
		NPUT(byte_buf, "REPLACE_INVALID");
	}

ret:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

size_t
__replace_invalid_iso2022jp(
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd)	 /* state */
{
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* retrun value */

	unsigned char	byte_buf;

	if (cd->replacement < 0x7f) {
		if (cd->_st_cset != CS_0) { /* not in ascii mode */
			NPUT(0x1b, "RELACE_HEX"); /* <ESC> */
			NPUT('(', "REPLACE_HEX");
#if defined(JFP_ICONV_TOCODE_2022JP2004)
			NPUT('B', "REPLACE_HEX");
#else
			NPUT('J', "REPLACE_HEX");
#endif
		}
		byte_buf = (unsigned char)cd->replacement;
		NPUT(byte_buf, "REPLACE_INVALID");
	} else if (cd->replacement > 0xff) {
		/* not in 0208 or 0213-plane_1 mode */
		if (cd->_st_cset != CS_1) { 
			NPUT(0x1b, "RELACE_HEX"); /* <ESC> */
#if defined(JFP_ICONV_TOCODE_2022JP2004)
			NPUT('$', "REPLACE_HEX");
			NPUT('(', "REPLACE_HEX");
			NPUT('Q', "REPLACE_HEX");
#else
			NPUT('$', "REPLACE_HEX");
			NPUT('B', "REPLACE_HEX");
#endif
		}
		byte_buf = ((cd->replacement >> 8) & 0x00ff);
		NPUT(byte_buf, "REPLACE_INVALID");
		byte_buf = (cd->replacement & 0x00ff);
		NPUT(byte_buf, "REPLACE_INVALID");
	}

ret:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

/*
 * __restore_hex_ascii
 *
 * Restore hex value when "IL--XX" or "NI--XX' is encountered.
 * return value:
 * 	0: done nothing 
 * 	1: done restoring ascii value, go to next loop
 *	-1: error, conversion should abort
 */
size_t
__restore_hex_ascii(
	unsigned char	**pip,	 /* point pointer to input buf */
	size_t		*pileft, /* point #bytes left in input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
        __icv_state_t   *st)	 /* state */
{
	unsigned char	*ip = *pip;
	size_t		ileft = *pileft;
	char		*op = *pop;
	size_t		oleft = *poleft;

	char 		*prefix; 
	unsigned char	hexval, first_half, second_half;

	int		rv = 0; /* return value */

	char	restore_buf[PREFIX_LENGTH + 2]; /* "NI--XX" or "LI--XX" */

	memcpy(restore_buf, ip - 1, sizeof(restore_buf));

	if (((st->_icv_flag & ICONV_CONV_ILLEGAL_RESTORE_HEX) &&
		(strncmp(restore_buf, PREFIX_ILLEGAL, PREFIX_LENGTH) == 0)) ||
	    ((st->_icv_flag & ICONV_CONV_NON_IDENTICAL_RESTORE_HEX) &&
		(strncmp(restore_buf, PREFIX_NON_IDENTICAL, PREFIX_LENGTH) == 0))) {

		if (ISHEXNUM(restore_buf[PREFIX_LENGTH])
				 && ISHEXNUM(restore_buf[PREFIX_LENGTH + 1])) {

			/* get hex value and put it into output */
			first_half=restore_buf[PREFIX_LENGTH];
			second_half=restore_buf[PREFIX_LENGTH + 1];
			__ATOI(first_half);
			__ATOI(second_half);
			hexval = (first_half << 4) + second_half;
			NPUT(hexval, "RESTORE_HEX");

			/* move pointer to the end of replacement */
			ip += PREFIX_LENGTH + 1;
			ileft -= PREFIX_LENGTH + 1;

			/* notify to caller */
			rv = (size_t)1;
		}
	}
ret:
	/* pointers are updated only on successful return */
	if (rv != (size_t)-1) {
		*pip =  ip;
		*pileft = ileft;
		*pop =  op;
		*poleft = oleft;
	}

	return (rv);
}

/* see jfp_iconv_common.h */
void
__clearbuf(__tmpbuf_t *buf)
{
	int	i;

	buf->ix = 0;
	for (i = 0; i < __TMPBUF_SIZE; i++) {
		buf->data[i] = 0;
	}
	return;
}

/* see jfp_iconv_common.h */
void
__putbuf(__tmpbuf_t *buf, char ic)
{
	char	tmpbuf[__TMPBUF_SIZE];

	if (buf->ix < __TMPBUF_SIZE) {
		buf->data[buf->ix] = ic;
		buf->ix++;
	} else {
		memcpy(tmpbuf, buf->data + 1, __TMPBUF_SIZE - 1);
		tmpbuf[__TMPBUF_SIZE - 1] = ic;
		memcpy(buf->data, tmpbuf, __TMPBUF_SIZE);
	}
	return;
}

/* see jfp_iconv_common.h */
int
__cmpbuf(__tmpbuf_t *buf, char *str, size_t strlen)
{
	char	*ptr;
	int	i;

	if (buf->ix < __TMPBUF_SIZE) {
		i = buf->ix - strlen - 1;
	} else {
		i = __TMPBUF_SIZE - strlen - 1;
	}

	if (i < 0) {
		return i;
	}
	ptr = &(buf->data[i]);
	return(memcmp(ptr, str, strlen));
}


size_t 
__next_of_esc_seq(
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd,	 /* state */
	int		caller)	 /* caller */
{
	size_t		rv = 0;	 /* return value */

	int		i, j;

	char *esc_seq[12] = {
		"\x1b(B",
		"\x1b(I",
		"\x1b(J",
		"\x1b$@",
		"\x1b$B",
		"\x1b$(B",
		"\x1b$(D",
		"\x1b&@\x1b$B",
		"\x1b&@\x1b$(B",
		"\x1b$(O",
		"\x1b$(P",
		"\x1b$(Q"
	};

	for (i = 0; i < 12; i++) {
		if (__cmpbuf(cd->tmpbuf, esc_seq[i], strlen(esc_seq[i]))
				== 0) {
			for (j = 0; j < strlen(esc_seq[i]); j++) {
				caller |= __NEXT_OF_ESC_SEQ;
				if ((rv = __replace_hex(
					esc_seq[i][j], pip, pop, poleft,
						cd, caller)) == -1) {
					goto ret;
				}
			}
			goto ret;
		}
	}

ret:
	return(rv);
}

void*
__index_of_nullchar(const void *asp, size_t n)
{

#if	defined(JFP_ICONV_FROMCODE_UTF32) || \
	defined(JFP_ICONV_FROMCODE_WCHAR)

	register const uint32_t*	sp = (uint32_t *)asp;
	register const uint32_t*	ep = sp + n;

	while (sp < ep)
		if (*sp++ == '\0')
			return((void *)--sp);
	return(0);

#else /* JFP_ICONV_FROMCODE_UTF16 || JFP_ICONV_FROMCODE_UCS2 */

	register const uint16_t*	sp = (uint16_t *)asp;
	register const uint16_t*	ep = sp + n;

	while (sp < ep)
		if (*sp++ == '\0')
			return((void *)--sp);
	return(0);

#endif
}
