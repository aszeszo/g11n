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

/*
 * libc iconv enhancement PSARC/2010/160 
 * two include files to implement PSARC/2010/160.
 *
 *   jfp_iconv_common.h (this file)
 *   jfp_iconv_common.c
 *
 * jfp_iconv_common.h and jfp_iconv_common.c
 * When implementing to support PSARC/2010/160, common definition and
 * common functions are implemented these two files.
 */

#ifndef _JFP_ICONV_COMMON_H
#define _JFP_ICONV_COMMON_H

#include <sys/types.h>
#include <iconv.h>

/*
 * flag to detect ICONV_CONV_ILLEGAL_* and ICONV_REPLACE_INVALID
 * When it's detected, EILSEQ is not reported and conversion is not
 * aborted by illegal input byte.
 * ICONV_CONV_ILLEGAL_RESTORE_HEX is not included because it's 
 * not related with EILSEQ.
 */
#define __ICONV_CONV_ILLEGAL (ICONV_CONV_ILLEGAL_DISCARD \
		| ICONV_CONV_ILLEGAL_REPLACE_HEX \
		| ICONV_REPLACE_INVALID)

/*
 * flag to detect ICONV_CONV_NON_IDENTICAL_* 
 * When it's detected, non identical character is not replaced with
 * pre-defined default (e.g. U+fffd, 0x3f '?', or jis 0x222e GETA)
 * ICONV_CONV_NON_IDENTICAL_RESTORE_HEX is not included because
 * it's not related with replacement.
 */
#define __ICONV_CONV_NON_IDENTICAL (ICONV_CONV_NON_IDENTICAL_DISCARD \
		| ICONV_CONV_NON_IDENTICAL_REPLACE_HEX \
		| ICONV_CONV_NON_IDENTICAL_TRANSLITERATE)

/*
 * flag to detect *_REPLACE_HEX
 * In case tocode is stateful (e.g ISO-2022-JP), the current mode
 * needs to be changed to CS_0 when it's detected.
 */
#define __ICONV_CONV_REPLACE_HEX (ICONV_CONV_ILLEGAL_REPLACE_HEX \
		| ICONV_CONV_NON_IDENTICAL_REPLACE_HEX)

/* flag to detect *_RESTORE_HEX */
#define __ICONV_CONV_RESTORE_HEX (ICONV_CONV_ILLEGAL_RESTORE_HEX \
		| ICONV_CONV_NON_IDENTICAL_RESTORE_HEX)

/*
 * flag to get/set ICONV_CONV_ILLEGAL_DISCARD and 
 * ICONV_CONV_NON_IDENTICAL_DISCARD
 */
#define __ICONV_CONV_DISCARD_ILSEQ (ICONV_CONV_ILLEGAL_DISCARD \
		| ICONV_CONV_NON_IDENTICAL_DISCARD)

/* symbol to return for ICONV_TRIVALP in _iconvctl() */
#define	__NOT_TRIVIALP	0
#define	__TRIVIALP	1

/*
 * symbol to be used to check which function call
 * __icv_replace_hex().
 */
#define __ICV_ILLEGAL		0
#define __ICV_NON_IDENTICAL	1

/*
 * prefix of illegal byte or non identical byte when
 * converting *_REPLACE_HEX flag. Those definition is used in
 * __restore_hex_ascii(). The different definition is in
 * __restore_hex_utf32(), and __restore_hex_utf16()
 */
#define PREFIX_ILLEGAL		"IL--"
#define	PREFIX_NON_IDENTICAL	"NI--"
#define PREFIX_LENGTH		4

/*
 * structure to keep state
 * Members have been merged from past implementation. And new member
 * _icv_flag and replacement are added to support attribute from 
 * _icv_open_attr(), and _icv_iconvctr().
 */
typedef struct {
	int		_st_cset;	/* codeset */
	int		_st_cset_sav;	/* previous codeset */
	boolean_t	bom_written;	/* true if BOM exists */
	boolean_t	little_endian;	/* true if Little Endian */

	int		_icv_flag;	/* flag ICONV_* in iconv.h */
	unsigned int	replacement;	/* only for _icv_iconvstr() */
	int		trivialp;	/* 1: trivial, 0: otherwise */
} __icv_state_t;

/*
 * prototype for common method
 */
iconv_t	_icv_open(const char *strp);
iconv_t	_icv_open_attr(int flag, void *reserved);
size_t	_icv_iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
		char **outbuf, size_t *outbytesleft);
int	_icv_iconvctl(iconv_t cd, int request, void *arg);
size_t 	_icv_iconvstr(char *inarray, size_t *inlen, char *outarray,
		size_t *outlen, int flag);
void	_icv_close(iconv_t cd);

/*
 * macro to check if value is representation of hex value.
 * It's used in __restore_hex_ascii() and __restore_hex_ucs() function
 */
#define	ISHEXNUM(c)	(((c >= '0') && (c <= '9')) \
			|| ((c >= 'A') && (c <= 'F')))

/* 
 * macro to change hex ascii one character to hex value. The value 'c'
 * must be in the range of defined in above ISHEXNUM(c).
 */
#define __ATOI(c) { \
		if (c <= 0x39) { \
			c -= 0x30; \
		} else { \
			c -= 0x37; \
		} \
	}

/*
 * RESTORE_HEX_ASCII macro
 *
 * __restore_hex_ascii() should be called only when 
 * __ICONV_CONV_RESTORE_HEX is specified, and the left bytes in input
 * buffer is greater than the  6bytes. "IL--XX" or "NI--XX"
 *
 * When __restore_hex_ascii is return 1, it means restored hex value has
 * been put into output. Pointers have been updated already by 
 * __restore_hex_ascii. Program should jump to the next loop in this
 * case.
 *
 * When __restore_hex_ascii is return -1, it means error (E2BIG).
 * Program should jump to abort conversion. The errno has been set in
 * __restore_hex_ascii (actually, NPUT() macro).
 *
 * Three macros are defined in hear:
 *   RESTORE_HEX_ASCII_JUMP
 *   RESTORE_HEX_ASCII_CONTINUE
 *   RESTORE_HEX_UNICODE
 *
 * RESTORE_HEX_ASCII_JUMP and RESTORE_HEX_ASCII_CONTINUE are prepared
 * to fit to the loop implemented in existing conversion modules. 
 * RESTORE_HEX_UNICODE is provided just for calling __restore_hex_ucs()
 * in Unicode based conversion modules.
 */
#define RESTORE_HEX_ASCII_JUMP(ic) { \
		if ((st->_icv_flag & __ICONV_CONV_RESTORE_HEX) && \
				ileft + 1 >= (PREFIX_LENGTH + 2) && \
				(((ic) == 'I') || ((ic) == 'N'))) { \
			switch (__restore_hex_ascii(&ip, &ileft, \
					&op, &oleft, st)) { \
				case (size_t)1: \
					goto cont; \
				case (size_t)-1: \
					rv = ((size_t)-1); \
					goto ret; \
				default: \
					break; \
			} \
		} \
	}

#define RESTORE_HEX_ASCII_CONTINUE(ic) { \
		if ((st->_icv_flag & __ICONV_CONV_RESTORE_HEX) && \
				ileft + 1 >= (PREFIX_LENGTH + 2) && \
				(((ic) == 'I') || ((ic) == 'N'))) { \
			switch (__restore_hex_ascii( \
				(unsigned char **)&ip, &ileft, \
					(char **)&op, &oleft, st)) { \
				case (size_t)1: \
					stat = ST_INIT; \
					continue; \
				case (size_t)-1: \
					retval = (size_t)ERR_RETURN; \
					goto ret; \
				default: \
					break; \
			} \
		} \
	}

#define RESTORE_HEX_UNICODE(ucs) { \
		if ((st->_icv_flag & __ICONV_CONV_RESTORE_HEX) && \
				ileft + __SIZE_OF_UCS >= ((PREFIX_LENGTH + 2) * \
					__SIZE_OF_UCS) && \
				(((ucs) == 'I') || ((ucs) == 'N'))) { \
			switch (__restore_hex_ucs(&ip, &ileft, \
					&op, &oleft, st)) { \
				case (size_t)1: \
					rv = ((size_t)0); \
					goto cont; \
				case (size_t)-1: \
					rv = ((size_t)-1); \
					goto ret; \
				default: \
					break; \
			} \
		} \
	}

/*
 * RET_EILSEQ macro 
 *
 * It's to return from conversion module with EILSEQ.
 *
 * When ICONV_CONV_ILLEGAL_* attribute is specified by _icv_open() or 
 * _icv_iconvctl(), EILSEQ is not reported, and detected illegal byte 
 * is processed as specified by _icv_flag in __icv_state_t structure. 
 *
 * When ICONV_REPLACE_INVALID attribute is specified by _icv_iconvstr(),
 * the illegal byte is replaced with replacement in __icv_state_t.
 *
 * The existing code ("foo" is debug message):
 *
 *    RETERROR(EILSEQ, "foo")
 *
 * is replaced with this macro as follows:
 *
 *    RET_EILSEQ("foo", byte_num)
 *
 * byte_num is an integer number which byte is currently being 
 * processed (if it's 2nd byte, num_of_byte should be 2).
 *
 * Once stepping in this code fragment, program do jump to 'cont:' 
 * or 'ret:'
 *
 * cont: go to top of the loop to get next byte to avoid EILSEQ
 * ret:	 error return with EILSEQ, conversion will be aborted
 *
 */
#define RET_EILSEQ(msg, byte_num) {\
		errno = EILSEQ; \
		DEBUGMSG(msg); \
		if (st->_icv_flag & __ICONV_CONV_ILLEGAL) { \
			if(__icv_illegal(&ip, &ileft, &op, &oleft, \
				st, (byte_num)) == (size_t)-1) { \
				rv = ((size_t)-1); \
				goto ret; \
			} \
			goto cont; \
		} else {\
			rv = ((size_t)-1); \
			goto ret; \
		}\
	}

/*
 * UNGET_EILSEQ macro
 *
 * This macro is used only in ISO-2022-JP_TO_*.c,
 * and *_TO_ISO-2022-JP.c Unexpected to be used in others.
 *
 * The program goes into this macro when:
 *   a) illegal byte is detected in escape sequence
 *   b) illegal byte is detected in text ( byte < 0x21, or byte == 7f)
 *
 * default:
 * Abort conversion (go to "ret") with EILSEQ, and move back to 
 * pointer as below:
 *   - Move back to <ESC> when it's detected in escape sequence
 *   - Move back to 1st byte when it's detected in multi-byte char
 *
 * When ICONV_CONV_ILLEGAL_* is specified:
 * Call __icv_illegal to process illegal byte, and go to next loop.
 * When illegal byte is detected in escape sequence, the pointer of 
 * input buffer move back to <ESC>, and clear status to ST_INIT. 
 * When illegal byte is detected in 2nd byte of multi-byte character, 
 * the pointer of input buffer move back to 1st byte.
 *
 * Status (stat), is not used when this code is extracted in
 * *_TO_ISO-2022-JP.c
 *
 */
#define UNGET_EILSEQ(byte) {\
		int __ix; \
		if (st->_icv_flag & __ICONV_CONV_ILLEGAL) { \
			if(__icv_illegal((unsigned char **)(&ip), \
				&ileft, (char **)(&op), &oleft, \
				st, (byte)) == (size_t)-1) { \
				retval = (size_t)ERR_RETURN; \
				goto ret; \
			} \
			stat = ST_INIT; \
			if(st->_icv_flag & __ICONV_CONV_REPLACE_HEX) { \
				cset = CS_0; \
			} else if (st->_icv_flag \
				& ICONV_REPLACE_INVALID) {\
				if (st->replacement < 0x7f) {\
					cset = CS_0; \
				} else { \
					cset = CS_1; \
				} \
			} \
			continue; \
		} else { \
			for(__ix = (byte); __ix > 0; __ix--) { \
				UNGET(); \
			} \
			errno = EILSEQ; \
			retval = (size_t)ERR_RETURN; \
			goto ret; \
		} \
	}

#define UNGET_EILSEQ_STATELESS(byte) { \
		int __ix; \
		if (st->_icv_flag & __ICONV_CONV_ILLEGAL) { \
			if(__icv_illegal((unsigned char **)(&ip), \
				&ileft, (char **)(&op), &oleft, \
				st, (byte)) == (size_t)-1) { \
				retval = (size_t)ERR_RETURN; \
				goto ret; \
			} \
			stat = ST_INIT; \
			continue; \
		} else { \
			for(__ix = (byte); __ix > 0; __ix--) { \
				UNGET(); \
			} \
			errno = EILSEQ; \
			retval = (size_t)ERR_RETURN; \
			goto ret; \
		} \
	}

/*
 * CALL_NON_IDENTICAL() macro
 * It's used to call __icv_non_identical() function from conversion
 * modules. Program jump to "ret" when return value of 
 * __icv_non_identical() is "-1". The "-1" is set when E2BIG in
 *  __icv_non_identical(). The errno was set in __icv_non_identical() 
 * before returning from that function.
 *
 * CALL_NON_IDENTICAL_UNGET() macro is similar with it, but it's
 * different because it call UNGET() when __icv_non_identical() return 
 * "-1". The argument is number of byte to pass __icv_non_identical(),
 * and that value is also used for a number of call UNGET()
 */
#define CALL_NON_IDENTICAL() { \
		if(__icv_non_identical(&ip, &op, &oleft, \
			st, pre_ileft - ileft) == (size_t)-1) { \
			rv = ((size_t)-1); \
			goto ret; \
		} \
	}

#define CALL_NON_IDENTICAL_UNGET(num) { \
		int __ix; \
		if(__icv_non_identical( \
			(unsigned char **)(&ip), \
			(char **)(&op),  \
			&oleft, \
			st, (num)) == (size_t)-1) { \
			for (__ix = 0; __ix < (num); __ix++) { \
				UNGET(); \
			} \
			retval = (size_t)ERR_RETURN; \
			goto ret; \
		} \
	}

/*
 * prototype for local functions
 */
__icv_state_t *
__icv_open_attr(int flag);

size_t
__icv_illegal(
	unsigned char   **pip,	 /* point pointer to input buf */
	size_t		*pileft, /* point #bytes left in input buf */
	char            **pop,	 /* point pointer to output buf */
	size_t          *poleft, /* point #bytes left in output buf */
	__icv_state_t   *cd,	 /* state */
	int		num_of_bytes); /* byte number (1st, 2nd,..) */

size_t
__icv_non_identical(
        unsigned char   **pip,	 /* point pointer to input buf */
        char            **pop,	 /* point pointer to output buf */
        size_t          *poleft, /* point #bytes left in output buf */
        __icv_state_t   *cd,	 /* state */
        int             num_of_bytes); /* num of non-identical bytes */

/*
 * prototype related with  __replace_hex()
 *
 * __replace_hex
 *
 * It's called from __icv_illegal() or __icv_non_identical() when
 * ICONV_CONV_ILLEGAL_REPLACE_HEX or 
 * ICONV_CONV_NON_IDENTICAL_REPLACE_HEX is specified. __replace_hex 
 * should be an implementation to put replacement characters "IL--XX" or
 * "NI--XX" as appropriate tocode. When tocode is Unicode based 
 * encoding like UTF-8 or UCS-2, this function should call 
 * __replace_hex_utf32() defined in jfp_iconv_unicode.h
 *
 * __replace_hex is implemented in each conversion modules. It's to 
 * call __replace_hex_utf32() for Unicode based conversion, or to call
 * other routine for native conversion. 
 */
size_t __replace_hex(
	unsigned char	hex,	 /* hex to write */
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd,	 /* state */
	int		caller); /* caller */

size_t __replace_hex_utf32(
	unsigned char	hex,	 /* hex to write */
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd,	 /* state */
	int		caller); /* caller */

size_t __replace_hex_ascii(
	unsigned char	hex,	 /* hex to write */
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	int		caller); /* caller */

size_t __replace_hex_iso2022jp(
	unsigned char	hex,	 /* hex to write */
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd,	 /* state */
	int		caller); /* caller */

/*
 * prototype related with  __replace_invalid()
 *
 * __replace_invalid
 *
 * It's called from __icv_illegal when ICONV_REPLACE_INVALID is 
 * specified. It's put pre-defined replacement character for 
 * appropriate tocode. It's implemented in each modules. In each 
 * module, __replace_invalid should call one of 
 * __replace_invalid_utf32(), __replace_invalid_ascii, or
 * __replace_invalid_iso2022jp().
 *
 * The pre-defined replacement character is set by __icv_open_attr(). 
 * It's in __icv_state_t.
 */
size_t 
__replace_invalid(
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd);	 /* state */

size_t 
__replace_invalid_utf32(
	unsigned char	**pip,	 /* point pointer to input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd);	 /* state */

size_t 
__replace_invalid_ascii(
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd);	 /* state */

size_t 
__replace_invalid_iso2022jp(
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
	__icv_state_t	*cd);	 /* state */

/*
 * prototype of __restore_*()
 */
size_t
__restore_hex_ascii(
	unsigned char	**pip,	 /* point pointer to input buf */
	size_t		*pileft, /* point #bytes left in input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
        __icv_state_t   *st);	 /* state */

size_t
__restore_hex_ucs(
	unsigned char	**pip,	 /* point pointer to input buf */
	size_t		*pileft, /* point #bytes left in input buf */
	char		**pop,	 /* point pointer to output buf */
	size_t		*poleft, /* point #bytes left in output buf */
        __icv_state_t   *st);	 /* state */

#endif /* !_JFP_ICONV_COMMON_H */