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
#include <wchar.h>
#include <widec.h>
#include "japanese.h"
#include "jfp_iconv_common.h"
#include "jfp_iconv_wchar.h"

/*
 * This file contains implementation of common methods, and local
 * functions for wchar_t conversion support
 */

size_t
__replace_hex_wchar(
	unsigned char	hex,
	unsigned char	**pip,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd,
	int		caller)
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
		NPUT_WASCII('I', "REPLACE_HEX");
		NPUT_WASCII('L', "REPLACE_HEX");
	} else { /* __ICV_NON_IDENTICAL */
		NPUT_WASCII('N', "REPLACE_HEX");
		NPUT_WASCII('I', "REPLACE_HEX");
	}
	NPUT_WASCII('-', "REPLACE_HEX");
	NPUT_WASCII('-', "REPLACE_HEX");
	NPUT_WASCII((char)first_half, "REPLACE_HEX");
	NPUT_WASCII((char)second_half, "REPLACE_HEX");
ret:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

size_t
__replace_invalid_wchar(
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*cd)
{
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* retrun value */

	wchar_t		wcharbuf;
	unsigned char	oc;

	wcharbuf = (wchar_t)(cd->replacement);

	NPUT_WCHAR(wcharbuf, "REPLACE_INVALID");

ret:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

/*
 * __restore_hex_wchar
 *
 * Restore hex value when "IL--XX" or "NI--XX' is encountered.
 * return value:
 * 	0: done nothing 
 * 	1: done restoring ascii value, go to next loop
 *	-1: error, conversion should abort
 */
size_t
__restore_hex_wchar(
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

	int		rv = 0; /* return value */
	int		i, is_equal;
	wchar_t 	wcharval;
	unsigned char	*ptr;

	wchar_t 	illegal_prefix[PREFIX_LENGTH] 
				= {L'I', L'L', L'-', L'-'};
	wchar_t		non_identical_prefix[PREFIX_LENGTH] 
				= {L'N', L'I', L'-', L'-'};
	unsigned int	hexval, first_half, second_half;
	
	/*
         * We need to check if inputting wchar_t sequence is equal with
         * illegal_prefix, or non_identical_prefix. The start address is
         * an adreess of previous wchar_t character.
         */
	ptr = ip - sizeof(wchar_t);
	for (i = 0, is_equal = 0; i < PREFIX_LENGTH; i++) {
#ifdef __sparc
		wcharval = 0U;
		wcharval |= (unsigned int)*(ptr++) << 24;
		wcharval |= (unsigned int)*(ptr++) << 16;
		wcharval |= (unsigned int)*(ptr++) << 8;
		wcharval |= (unsigned int)*(ptr++) << 0;
#else
		wcharval = 0U;
		wcharval |= (unsigned int)*(ptr++) << 0;
		wcharval |= (unsigned int)*(ptr++) << 8;
		wcharval |= (unsigned int)*(ptr++) << 16;
		wcharval |= (unsigned int)*(ptr++) << 24;
#endif
		/* break immediately when difference is detected */
		if (((st->_icv_flag & ICONV_CONV_ILLEGAL_RESTORE_HEX) &&
			(wcharval == illegal_prefix[i])) || 
		    ((st->_icv_flag & ICONV_CONV_NON_IDENTICAL_RESTORE_HEX) &&
			(wcharval == non_identical_prefix[i]))){
			is_equal++;
		} else {
			break;
		}
	}


	/* prefix has been detected, process to get hex value */

	if (is_equal == PREFIX_LENGTH) {
#ifdef __sparc
		first_half = 0U;
		first_half |= (unsigned int)*(ptr++) << 24;
		first_half |= (unsigned int)*(ptr++) << 16;
		first_half |= (unsigned int)*(ptr++) << 8;
		first_half |= (unsigned int)*(ptr++) << 0;
		second_half = 0U;
		second_half |= (unsigned int)*(ptr++) << 24;
		second_half |= (unsigned int)*(ptr++) << 16;
		second_half |= (unsigned int)*(ptr++) << 8;
		second_half |= (unsigned int)*(ptr++) << 0;
#else
		first_half = 0U;
		first_half |= (unsigned int)*(ptr++) << 0;
		first_half |= (unsigned int)*(ptr++) << 8;
		first_half |= (unsigned int)*(ptr++) << 16;
		first_half |= (unsigned int)*(ptr++) << 24;
		second_half = 0U;
		second_half |= (unsigned int)*(ptr++) << 0;
		second_half |= (unsigned int)*(ptr++) << 8;
		second_half |= (unsigned int)*(ptr++) << 16;
		second_half |= (unsigned int)*(ptr++) << 24;
#endif

		/* if hex value is detected, put it to output */
		if (ISHEXNUM(first_half) && ISHEXNUM(second_half)) {
			__ATOI(first_half);
			__ATOI(second_half);
			hexval = (unsigned char)((first_half << 4) +
				 second_half);
			NPUT(hexval, "RESTORE_HEX");

			/* move pointer to the end of replacement */
			ip += (PREFIX_LENGTH + 1) * sizeof(wchar_t);
			ileft -= (PREFIX_LENGTH + 1) * sizeof(wchar_t);

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

wchar_t
__get_eucwchar(
	int codeset,		/* CS_0, CS_1, CS_2, or CS_3 */
	unsigned char ic1,	/* 1st byte */
	unsigned char ic2)	/* 2nd byte (NULL when CS_0 and CS_2) */
{
	unsigned int	first, second, eucwchar;

	first = (unsigned int)ic1;
	second = (unsigned int)ic2;

	if (codeset == CS_0) {
		eucwchar = (unsigned int)(first) & 0xff;
	} else if (codeset == CS_1) {
		eucwchar = WCHAR_CS1 |
				((first & 0x7f) << 7) |
				(second & 0x7f);
	} else if (codeset == CS_2) {
		eucwchar = WCHAR_CS2 |
				(first & 0x7f);
	} else { /* CS_3 */
		eucwchar = WCHAR_CS3 |
				((first & 0x7f) << 7) |
				(second & 0x7f);
	}
	return (eucwchar);
}

/*
 * wchar is not a byte sqeuene but a value.
 * sequence is not checked. The stored value is checked and
 * return -1 and EILSEQ when the value is out of range.
 */
size_t					/* return #bytes read, or -1 */
__read_eucwchar(
	unsigned short	*p,		/* point variable to store euc16 */
	unsigned char	**pip,		/* point pointer to input buf */
	size_t		*pileft,	/* point #bytes left in input buf */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*st)		/* BOM state and endian */
{
	unsigned char	*ip = *pip;
	size_t		ileft = *pileft;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* return value */
	unsigned char	ic1, ic2, ic3, ic4;	/* bytes read */

	wchar_t		eucwchar;		/* resulted wchar */
	unsigned short	wchar1, wchar2, wchar3;	/* 1st, 2nd and 3rd byte in wchar format*/
	unsigned short	euc16;			/* 16bit euc */

	NGET(ic1, "WCHAR-1");
	NGET(ic2, "WCHAR-2");
	NGET(ic3, "WCHAR-3");
	NGET(ic4, "WCHAR-4");

#ifdef __sparc	
	eucwchar = 0U;
	eucwchar |= (wchar_t)ic1 << 24;
	eucwchar |= (wchar_t)ic2 << 16;
	eucwchar |= (wchar_t)ic3 << 8;
	eucwchar |= (wchar_t)ic4 << 0;
#else
	eucwchar = 0U;
	eucwchar |= (wchar_t)ic1 << 0;
	eucwchar |= (wchar_t)ic2 << 8;
	eucwchar |= (wchar_t)ic3 << 16;
	eucwchar |= (wchar_t)ic4 << 24;
#endif
	wchar1 = (unsigned short)(WCHAR_BYTE_OF(eucwchar, 1));
	wchar2 = (unsigned short)(WCHAR_BYTE_OF(eucwchar, 2));
	wchar3 = (unsigned short)(WCHAR_BYTE_OF(eucwchar, 3));

	if ((eucwchar & WCHAR_CSMASK) == WCHAR_CS0) {
		if ((wchar1 == 0) && (wchar2 == 0) && ISASC(wchar3)) {
			euc16 = wchar3;
		} else if ((eucwchar >= 0x80) && (eucwchar <= 0xff)) { 
			/*
			 * wctomb(3C) pass all the 1 byte value. But
			 * it's a not valid euc16 value. This function
			 * return 0x10xx as a special case
			 */
			euc16 = eucwchar | 0x1000;
		} else {
			RET_EILSEQ("illegal wchar-euc CS0", 4)
		}
	} else if ((eucwchar & WCHAR_CSMASK) == WCHAR_CS1) {
		if ((wchar1 == 0) && 
				ISCS1(wchar2|0x80) && ISCS1(wchar3|0x80)) {
			euc16 = (wchar2|0x80) << 8;
			euc16 |= (wchar3|0x80);
		} else {
			RET_EILSEQ("illegal wchar-euc CS1", 4)
		}
	} else if ((eucwchar & WCHAR_CSMASK) == WCHAR_CS2) {
		if ((wchar1 == 0) && (wchar2 == 0) && 
				(wchar3 > 0x20) && (wchar3 < 0x7f)) {
			/* wctomb(3C) accept 0x21 - 0x7e as 3rd byte */
			euc16 = (wchar3|0x80);
		} else {
			RET_EILSEQ("illegal wchar-euc CS2", 4)
		}
	} else if ((eucwchar & WCHAR_CSMASK) == WCHAR_CS3) {
		if ((wchar1 == 0) &&
				ISCS3(wchar2|0x80) && ISCS3(wchar3|0x80)) {
			euc16 = (wchar2|0x80) << 8;
			euc16 |= wchar3;
		} else {
			RET_EILSEQ("illegal wchar-euc CS3", 4)
		}
	} else {
		RET_EILSEQ("illegal wchar-euc codeset", 4)
	}

	RESTORE_HEX_WCHAR(eucwchar);

	*p = euc16;
	rv = *pileft - ileft;
ret:
next:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pip = ip;
		*pileft = ileft;
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

/* check of the input bytes must be done before calling this fucntion */
wchar_t
__get_pckwchar(
	int codeset,		/* __ASCII, __PCK_KANA, or __PCK_KANJI */
	unsigned char ic1,	/* 1st byte */
	unsigned char ic2)	/* 2nd byte */
{
	wchar_t		pckwchar;
	unsigned short	pck;

	if ((codeset == __ASCII) || (codeset == __PCK_KANA)) {
		pckwchar = (wchar_t)ic1;
	} else {
		pck = (unsigned short)ic1 << 8;
		pck |= ((unsigned short)ic2 & 0x00ff);

		if ((pck >= 0x8140) && (pck <= 0x9ffc)) {
			pckwchar = pck - 0x8040;
		} else if ((pck >= 0xe040) && (pck <= 0xfcfc)) {
			pckwchar = pck - 0xc083;
		} else {
			return (-1);
		}
	}
	return (pckwchar);
}

size_t					/* return #bytes read, or -1 */
__read_pckwchar(
	unsigned short	*p,		/* point variable to store PCK value */
	unsigned char	**pip,		/* point pointer to input buf */
	size_t		*pileft,	/* point #bytes left in input buf */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*st)		/* BOM state and endian */
{
	unsigned char	*ip = *pip;
	size_t		ileft = *pileft;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* return value */
	unsigned char	ic1, ic2, ic3, ic4;	/* bytes read */

	wchar_t		pckwchar;	/* PCK wchar */
	unsigned short	pckval;		/* PCK value */
	unsigned char	first_byte;	/* 2byte PCK 1st byte */
	unsigned char	second_byte;	/* 2byte PCK 2nd byte */
	int		range = -1;	/* __PCK_KANJI when it's 2byte char */

	NGET(ic1, "WCHAR-1");
	NGET(ic2, "WCHAR-2");
	NGET(ic3, "WCHAR-3");
	NGET(ic4, "WCHAR-4");

#ifdef __sparc	
	pckwchar = 0U;
	pckwchar |= (wchar_t)ic1 << 24;
	pckwchar |= (wchar_t)ic2 << 16;
	pckwchar |= (wchar_t)ic3 << 8;
	pckwchar |= (wchar_t)ic4 << 0;
#else
	pckwchar = 0U;
	pckwchar |= (wchar_t)ic1 << 0;
	pckwchar |= (wchar_t)ic2 << 8;
	pckwchar |= (wchar_t)ic3 << 16;
	pckwchar |= (wchar_t)ic4 << 24;
#endif

	/* get pck value and return if out of range. */
	if ((pckwchar <= 0x7f) ||
	   ((pckwchar >= 0xa1) && (pckwchar <= 0xdf))) {
		pckval = (unsigned short)pckwchar;
	} else if ((pckwchar >= 0x100) && (pckwchar <= 0x1fbc)) {
		pckval = (unsigned short)pckwchar + 0x8040;
		range = __PCK_KANJI;
	} else if ((pckwchar >= 0x1fbd) && (pckwchar <= 0x3c79)) {
		pckval = (unsigned short)pckwchar + 0xc083;
		range = __PCK_KANJI;
	} else {
		RET_EILSEQ("out of valid range", 4)
	}

	/* check 1st byte and 2nd byte */
	if (range == __PCK_KANJI) {
		first_byte = (unsigned char)(pckval >> 8);
		second_byte = (unsigned char)(pckval & 0x00ff);
		if (ISSJMB_1(first_byte)) {
			if(!(ISSJKANJI2(second_byte)))
				RET_EILSEQ("converted 2nd byte is illegal", 4);
		} else
			RET_EILSEQ("converted 1st byte is illegal", 4);
	}

	RESTORE_HEX_WCHAR(pckwchar);

	*p = pckval;
	rv = *pileft - ileft;
ret:
next:
	if (rv != (size_t)-1) {
		/* update pointers only on successful return */
		*pip = ip;
		*pileft = ileft;
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

