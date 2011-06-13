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
 * COPYRIGHT AND PERMISSION NOTICE
 *
 * Copyright (c) 1991-2005 Unicode, Inc. All rights reserved. Distributed
 * under the Terms of Use in http://www.unicode.org/copyright.html.
 *
 * This file has been modified by Oracle and/or its affiliates.
 */
/*
 * Copyright (c) 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include	<sys/types.h>
#include	<string.h>
#include 	"jfp_iconv_predefine.h"

#define	BOM	0xfeff
#define	BSBOM16	0xfffe
#define	BSBOM32	0xfffe0000
#define	REPLACE	0xfffd
#define	IFHISUR(x)	((0xd800 <= (x)) && ((x) <= 0xdbff))
#define	IFLOSUR(x)	((0xdc00 <= (x)) && ((x) <= 0xdfff))

/* 
 * size of character. It's used in _replace_hex_ucs() function to
 * look for the prefix to be replaced with hex value.
 */
#if     defined(JFP_ICONV_FROMCODE_UTF32)
#define __SIZE_OF_UCS	4
#else   /* JFP_ICONV_FROMCODE_UTF16 or JFP_ICONV_FROMCODE_UCS2 */
#define __SIZE_OF_UCS	2
#endif

#if	defined(JFP_ICONV_FROMCODE_UTF32)

static size_t				/* return #bytes read, or -1 */
read_unicode(
	unsigned int	*p,		/* point variable to store UTF-32 */
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
	size_t		rv = (size_t)0; /* return value */
	unsigned char	ic1, ic2, ic3, ic4;	/* bytes read */
	unsigned int	u32;		/* resulted UTF-32 */

	NGETR(ic1, "UTF32-1");
	NGETR(ic2, "UTF32-2");
	NGETR(ic3, "UTF32-3");
	NGETR(ic4, "UTF32-4");

	if (st->bom_written == B_FALSE) {
		u32 = 0U;
		u32 |= (unsigned int)ic1 << 24;
		u32 |= (unsigned int)ic2 << 16;
		u32 |= (unsigned int)ic3 << 8;
		u32 |= (unsigned int)ic4 << 0;
		if (u32 == BOM) {
			st->bom_written = B_TRUE;
			st->little_endian = B_FALSE;
			*p = BOM;
			rv = (size_t)0;
			goto ret;
		} else if (u32 == BSBOM32) {
			st->bom_written = B_TRUE;
			st->little_endian = B_TRUE;
			*p = BOM;
			rv = (size_t)0;
			goto ret;
		} else {
			st->bom_written = B_TRUE;
		}
	}

	if (st->little_endian == B_TRUE) {
		u32 = 0U;
		u32 |= (unsigned int)ic1 << 0;
		u32 |= (unsigned int)ic2 << 8;
		u32 |= (unsigned int)ic3 << 16;
		u32 |= (unsigned int)ic4 << 24;
	} else {
		u32 = 0U;
		u32 |= (unsigned int)ic1 << 24;
		u32 |= (unsigned int)ic2 << 16;
		u32 |= (unsigned int)ic3 << 8;
		u32 |= (unsigned int)ic4 << 0;
	}

	if (u32 == BSBOM32) {
		RET_EILSEQ("byte-swapped BOM detected", 4)
	}

	if ((u32 == 0xfffe) || (u32 == 0xffff) || (u32 > 0x10ffff)
			|| IFHISUR(u32) || IFLOSUR(u32)) {
		RET_EILSEQ("illegal in UTF-32", 4)
	}

	RESTORE_HEX_UNICODE(u32)

	*p = u32;
	rv = *pileft - ileft;

next:
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

#elif	defined(JFP_ICONV_FROMCODE_UTF16) || defined(JFP_ICONV_FROMCODE_UCS2)

static size_t				/* return #bytes read, or -1 */
read_unicode(
	unsigned int	*p,		/* point variable to store UTF-32 */
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
	size_t		rv = (size_t)0; /* return value */
	unsigned char	ic1, ic2;	/* bytes read */
	unsigned int	u32;		/* resulted UTF-32 */
	unsigned int	losur;		/* low surrogate */

	NGETR(ic1, "UTF16-1");	/* read 1st byte */
	NGETR(ic2, "UTF16-2");	/* read 2nd byte */

	if (st->bom_written == B_FALSE) {
		u32 = 0U;
		u32 |= (unsigned int)ic1 << 8;
		u32 |= (unsigned int)ic2 << 0;
		if (u32 == BOM) {
			st->bom_written = B_TRUE;
			st->little_endian = B_FALSE;
			*p = BOM;
			rv = (size_t)0;
			goto ret;
		} else if (u32 == BSBOM16) {
			st->bom_written = B_TRUE;
			st->little_endian = B_TRUE;
			*p = BOM;
			rv = (size_t)0;
			goto ret;
		} else {
			st->bom_written = B_TRUE;
		}
	}

	if (st->little_endian == B_TRUE) {
		u32 = (((unsigned int)ic2) << 8) | ic1;
	} else {
		u32 = (((unsigned int)ic1) << 8) | ic2;
	}

	if (u32 == BSBOM16) {
		RET_EILSEQ("byte-swapped BOM detected", 2)
	}

	if ((u32 == 0xfffe) || (u32 == 0xffff) || (u32 > 0x10ffff)
			|| (IFLOSUR(u32))) {
		RET_EILSEQ("illegal in UTF16", 2)
	}

	RESTORE_HEX_UNICODE(u32)

	if (IFHISUR(u32)) {
#if	defined(JFP_ICONV_FROMCODE_UCS2)
		RET_EILSEQ("surrogate is illegal in UCS2", 2)
#else	/* !defined(JFP_ICONV_FROMCODE_UCS2) */
		NGETR(ic1, "LOSUR-1");
		NGETR(ic2, "LOSUR-2");

		if (st->little_endian == B_TRUE) {
			losur = (((unsigned int)ic2) << 8) | ic1;
		} else {
			losur = (((unsigned int)ic1) << 8) | ic2;
		}

		if (IFLOSUR(losur)) {
			u32 = ((u32 - 0xd800) * 0x400)
				+ (losur - 0xdc00) + 0x10000;
		} else {
			RET_EILSEQ("low-surrogate expected", 4)
		}
#endif	/* defined(JFP_ICONV_FROMCODE_UCS2) */
	}

	*p = u32;
	rv = *pileft - ileft;

next:
ret:
	if (rv != (size_t)-1) {
		/* update pointers only on sucessful return */
		*pip = ip;
		*pileft = ileft;
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

#else	/* JFP_ICONV_FROMCODE_UTF8 (default) */

/*
 * The following vector shows remaining bytes in a UTF-8 character.
 * Index will be the first byte of the character.
 */
static const char remaining_bytes_tbl[0x100] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,

   /*  C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  CA  CB  CC  CD  CE  CF  */
	0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 

   /*  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  DA  DB  DC  DD  DE  DF  */
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 

   /*  E0  E1  E2  E3  E4  E5  E6  E7  E8  E9  EA  EB  EC  ED  EE  EF  */
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 

   /*  F0  F1  F2  F3  F4  F5  F6  F7  F8  F9  FA  FB  FC  FD  FE  FF  */
	3,  3,  3,  3,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

    
/*
 * The following is a vector of bit-masks to get used bits in
 * the first byte of a UTF-8 character.  Index is remaining bytes at above of
 * the character.
 */
static const char masks_tbl[6] = { 0x00, 0x1f, 0x0f, 0x07, 0x03, 0x01 };


/*
 * The following two vectors are to provide valid minimum and
 * maximum values for the 2'nd byte of a multibyte UTF-8 character for
 * better illegal sequence checking. The index value must be the value of
 * the first byte of the UTF-8 character.
 */
static const unsigned char valid_min_2nd_byte[0x100] = {
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
     /*  C0    C1    C2    C3    C4    C5    C6    C7  */
	0,    0,    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
     /*  C8    C9    CA    CB    CC    CD    CE    CF  */
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
     /*  D0    D1    D2    D3    D4    D5    D6    D7  */
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
     /*  D8    D9    DA    DB    DC    DD    DE    DF  */
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
     /*  E0    E1    E2    E3    E4    E5    E6    E7  */
	0xa0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
     /*  E8    E9    EA    EB    EC    ED    EE    EF  */
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
     /*  F0    F1    F2    F3    F4    F5    F6    F7  */
	0x90, 0x80, 0x80, 0x80, 0x80, 0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
};

static const unsigned char valid_max_2nd_byte[0x100] = {
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
     /*  C0    C1    C2    C3    C4    C5    C6    C7  */
	0,    0,    0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 
     /*  C8    C9    CA    CB    CC    CD    CE    CF  */
	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 
     /*  D0    D1    D2    D3    D4    D5    D6    D7  */
	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 
     /*  D8    D9    DA    DB    DC    DD    DE    DF  */
	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 
     /*  E0    E1    E2    E3    E4    E5    E6    E7  */
	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 
     /*  E8    E9    EA    EB    EC    ED    EE    EF  */
	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0x9f, 0xbf, 0xbf, 
     /*  F0    F1    F2    F3    F4    F5    F6    F7  */
	0xbf, 0xbf, 0xbf, 0xbf, 0x8f, 0,    0,    0,    
	0,    0,    0,    0,    0,    0,    0,    0,    
};

static size_t
utf8_ucs(
	unsigned int	*p,
	unsigned char	**pip,
	size_t		*pileft,
	char		**pop,
	size_t		*poleft,
	__icv_state_t	*st)
{
	unsigned int	l;	/* to be copied to *p on successful return */
	unsigned char	ic;	/* current byte */
	unsigned char	ic1;	/* 1st byte */
	unsigned char	*ip = *pip;	/* next byte to read */
	size_t		ileft = *pileft; /* number of bytes available */
	size_t		rv = (size_t)0; /* return value of this function */
	int		remaining_bytes, i;

	char		*op = *pop;
	size_t		oleft = *poleft;

	NGETR(ic, "no bytes available");	/* read 1st byte */
	ic1 = ic;
	l = ic1; /* get bits from 1st byte to UCS value */

	if (ic1 < 0x80) {
		RESTORE_HEX_ASCII_JUMP(ic1)
		/* successfully converted */
		*p = l;
		rv = *pileft - ileft;
		goto ret;
	}

	remaining_bytes = remaining_bytes_tbl[ic1];

	if (remaining_bytes != 0) {
		l &= masks_tbl[remaining_bytes];

		/*
		 * loop counter 'i' starts from 2 since 1st byte has been 
		 * already read. loop counter is used to process __icv_illegal()
		 * that need to know which byte has been detected as illegal.
		 */
		for (i = 2; remaining_bytes > 0; remaining_bytes--, i++) {
			if (ic1 != 0U) {
				NGETR(ic, "2nd byte of UTF-8");
				if ((ic < valid_min_2nd_byte[ic1]) ||
					(ic > valid_max_2nd_byte[ic1])) {
					RET_EILSEQ("2nd byte is invalid", 2)
				}
				ic1 = 0U; /* 2nd byte check done */
			} else {
				NGETR(ic, "3rd or later byte of UTF-8");
				if ((ic < 0x80) || (ic > 0xbf)) {
				RET_EILSEQ("3rd or later byte is invalid", i)
				}
			}
			l = (l << 6) | (ic & 0x3f);
		}
		/* successfully converted */
		*p = l;
		rv = *pileft - ileft;
		goto ret;
	} else {
		RET_EILSEQ("1st byte is invalid", 1)
	}
next:
ret:
	if (rv != (size_t)-1) {
		/*
		 * update pointers on successful return
		 */
		*pip = ip;
		*pileft = ileft;
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

/* for UTF-8 */
static size_t				/* return #bytes read, or -1 */
read_unicode(
	unsigned int	*p,		/* point variable to store UTF-32 */
	unsigned char	**pip,		/* point pointer to input buf */
	size_t		*pileft,	/* point #bytes left in input buf */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*state)		/* BOM state and endian - unused */
{
	return (utf8_ucs(p, pip, pileft, pop, poleft, state));
}

#endif

#if	defined(JFP_ICONV_TOCODE_UTF32)

static size_t
write_unicode(
	unsigned int	u32,		/* UTF-32 to write */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*state,		/* BOM state and endian */
	const char	*msg)		/* debug message */
{
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* return value */
	unsigned char	ic1, ic2, ic3, ic4;	/* bytes to be written */

	if (state->bom_written == B_FALSE) {
		if (state->little_endian == B_TRUE) {
			ic1 = (unsigned char)((BOM >> 0) & 0xff);
			ic2 = (unsigned char)((BOM >> 8) & 0xff);
			ic3 = (unsigned char)((BOM >> 16) & 0xff);
			ic4 = (unsigned char)((BOM >> 24) & 0xff);
		} else {
			ic1 = (unsigned char)((BOM >> 24) & 0xff);
			ic2 = (unsigned char)((BOM >> 16) & 0xff);
			ic3 = (unsigned char)((BOM >> 8) & 0xff);
			ic4 = (unsigned char)((BOM >> 0) & 0xff);
		}
		rv += 4;
		NPUT(ic1, "BOM32-1")
		NPUT(ic2, "BOM32-2")
		NPUT(ic3, "BOM32-3")
		NPUT(ic4, "BOM32-4")
	}

	if (state->little_endian == B_TRUE) {
		ic1 = (unsigned char)((u32 >> 0) & 0xff);
		ic2 = (unsigned char)((u32 >> 8) & 0xff);
		ic3 = (unsigned char)((u32 >> 16) & 0xff);
		ic4 = (unsigned char)((u32 >> 24) & 0xff);
		rv += 4;
	} else {
		ic1 = (unsigned char)((u32 >> 24) & 0xff);
		ic2 = (unsigned char)((u32 >> 16) & 0xff);
		ic3 = (unsigned char)((u32 >> 8) & 0xff);
		ic4 = (unsigned char)((u32 >> 0) & 0xff);
		rv += 4;
	}

	NPUT(ic1, "UTF32-1")
	NPUT(ic2, "UTF32-2")
	NPUT(ic3, "UTF32-3")
	NPUT(ic4, "UTF32-4")

ret:
	if (rv != (size_t)-1) {
		/* update *pop and *poleft only on successful return */
		*pop = op;
		*poleft = oleft;
		if (state->bom_written == B_FALSE)
			state->bom_written = B_TRUE;
	}

	return (rv);
}

#elif	defined(JFP_ICONV_TOCODE_UTF16) || defined(JFP_ICONV_TOCODE_UCS2)

static size_t
write_unicode(
	unsigned int	u32,		/* UTF-32 to write */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*state,		/* BOM state and endian */
	const char	*msg)		/* debug message */
{
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;	/* return value */
	unsigned char	ic1, ic2;	/* bytes to be written */
	unsigned int	losur = 0U;		/* Hi/Lo surrogates */

	if (state->bom_written == B_FALSE) {
		if (state->little_endian == B_TRUE) {
			ic1 = (unsigned char)((BOM >> 0) & 0xff);
			ic2 = (unsigned char)((BOM >> 8) & 0xff);
		} else {
			ic1 = (unsigned char)((BOM >> 8) & 0xff);
			ic2 = (unsigned char)((BOM >> 0) & 0xff);
		}
		rv += 2;
		NPUT(ic1, "BOM16-1")
		NPUT(ic2, "BOM16-2")
	}

	if (u32 > 0xffff) {
#if	defined(JFP_ICONV_TOCODE_UCS2)
		u32 = REPLACE;
#else	/* !defined(JFP_ICONV_TOCODE_UCS2) */
		losur = ((u32 - 0x10000) % 0x400) + 0xdc00;
		u32 = ((u32 - 0x10000) / 0x400) + 0xd800;
#endif	/* defined(JFP_ICONV_TOCODE_UCS2) */
	}
	
	if (state->little_endian == B_TRUE) {
		ic1 = (unsigned char)(u32 & 0xff);
		ic2 = (unsigned char)((u32 >> 8) & 0xff);
		rv += 2;
	} else {
		ic1 = (unsigned char)((u32 >> 8) & 0xff);
		ic2 = (unsigned char)(u32 & 0xff);
		rv += 2;
	}

	NPUT(ic1, "UTF16-1")
	NPUT(ic2, "UTF16-2")

	if (losur != 0U) {
		if (state->little_endian == B_TRUE) {
			ic1 = (unsigned char)(losur & 0xff);
			ic2 = (unsigned char)((losur >> 8) & 0xff);
			rv += 2;
		} else {
			ic1 = (unsigned char)((losur >> 8) & 0xff);
			ic2 = (unsigned char)(losur & 0xff);
			rv += 2;
		}

		NPUT(ic1, "LOSUR-1")
		NPUT(ic2, "LOSUR-2")
	}


ret:
	if (rv != (size_t)-1) {
		/* update *pop and *poleft only on successful return */
		*pop = op;
		*poleft = oleft;
		if (state->bom_written == B_FALSE)
			state->bom_written = B_TRUE;
	}

	return (rv);
}

#else	/* JFP_ICONV_TOCODE_UTF8 (default) */

static size_t
write_unicode(
	unsigned int	u32,		/* UTF-32 to write */
	char		**pop,		/* point pointer to output buf */
	size_t		*poleft,	/* point #bytes left in output buf */
	__icv_state_t	*state,		/* BOM state and endian - unused */
	const char	*msg)		/* debug message */
{
	char	*op = *pop;
	size_t	oleft = *poleft;
	size_t	rv = 0;			/* return value */

	if (u32 <= 0x7f) {
		NPUT((unsigned char)(u32), msg);
		rv = 1;
	} else if (u32 <= 0x7ff) {
		NPUT((unsigned char)((((u32)>>6) & 0x1f) | 0xc0), msg);
		NPUT((unsigned char)((((u32)>>0) & 0x3f) | 0x80), msg);
		rv = 2;
	} else if ((u32 >= 0xd800) && (u32 <= 0xdfff)) {
		RETERROR(EILSEQ, "surrogate in UTF-8")
	} else if (u32 <= 0xffff) {
		NPUT((unsigned char)((((u32)>>12) & 0x0f) | 0xe0), msg);
		NPUT((unsigned char)((((u32)>>6) & 0x3f) | 0x80), msg);
		NPUT((unsigned char)((((u32)>>0) & 0x3f) | 0x80), msg);
		rv = 3;
	} else if (u32 <= 0x10ffff) {
		NPUT((unsigned char)((((u32)>>18) & 0x07) | 0xf0), msg);
		NPUT((unsigned char)((((u32)>>12) & 0x3f) | 0x80), msg);
		NPUT((unsigned char)((((u32)>>6) & 0x3f) | 0x80), msg);
		NPUT((unsigned char)((((u32)>>0) & 0x3f) | 0x80), msg);
		rv = 4;
	} else {
		RETERROR(EILSEQ, "beyond range of UTF-8")
	}

ret:
	if (rv != (size_t)-1) {
		/* update *pop and *poleft only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

#endif

#define	GETU(pu32) \
	switch (read_unicode(pu32, &ip, &ileft, \
		&op, &oleft, st)) { \
	case (size_t)-1: \
		/* errno has been set in read_unicode() */ \
		rv = (size_t)-1; \
		goto ret; \
	case (size_t)0: \
		/* character read was handled in the read_unicode() */ \
		/* no further evaluation needed in caller side */ \
		rv = (size_t)0; \
		goto next; \
	default: \
		/* return if null-character is detected, and */ \
		/* ICONV_IGNORE_NULL is specified */ \
		if ((*pu32 == 0U) && !(st->_icv_flag & ICONV_IGNORE_NULL)) \
			goto ret; \
		break; \
	}

#if	defined(JFP_ICONV_TOCODE_UCS2)
#define	PUTU(u32, msg, num_of_bytes)	\
	if ((st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) \
		&& ((u32 == 0xfffd) || (u32 > 0xffff))) { \
		if (__icv_non_identical(&ip, &op, &oleft, st, num_of_bytes) \
			== (size_t)-1) { \
			rv = ((size_t)-1);\
			goto ret; \
		} \
	} else if (write_unicode(u32, &op, &oleft, st, msg) \
			== (size_t)-1) { \
		rv = ((size_t)-1);\
		goto ret; \
	}
#else
#define	PUTU(u32, msg, num_of_bytes)	\
	if ((st->_icv_flag & __ICONV_CONV_NON_IDENTICAL) \
		&& (u32 == 0xfffd)) { \
		if (__icv_non_identical(&ip, &op, &oleft, st, num_of_bytes) \
			== (size_t)-1) { \
			rv = ((size_t)-1);\
			goto ret; \
		} \
	} else if (write_unicode(u32, &op, &oleft, st, msg) \
			== (size_t)-1) { \
		rv = ((size_t)-1);\
		goto ret; \
	}
#endif

#include	<stdlib.h>

static void
_icv_reset_unicode(void *cd)
{
	__icv_state_t	*state = (__icv_state_t *)cd;

#if	defined(JFP_ICONV_FROMCODE_UTF32BE) || \
	defined(JFP_ICONV_TOCODE_UTF32BE) || \
	defined(JFP_ICONV_FROMCODE_UTF16BE) || \
	defined(JFP_ICONV_TOCODE_UTF16BE) || \
	defined(JFP_ICONV_FROMCODE_UCS2BE) || \
	defined(JFP_ICONV_TOCODE_UCS2BE)
	state->little_endian = B_FALSE;
	state->bom_written = B_TRUE;
#elif	defined(JFP_ICONV_FROMCODE_UTF32LE) || \
	defined(JFP_ICONV_TOCODE_UTF32LE) || \
	defined(JFP_ICONV_FROMCODE_UTF16LE) || \
	defined(JFP_ICONV_TOCODE_UTF16LE) || \
	defined(JFP_ICONV_FROMCODE_UCS2LE) || \
	defined(JFP_ICONV_TOCODE_UCS2LE)
	state->little_endian = B_TRUE;
	state->bom_written = B_TRUE;
#elif	defined(_LITTLE_ENDIAN)
	state->little_endian = B_TRUE;
	state->bom_written = B_FALSE;
#endif

	/* set default replacement char U+fffd */
	state->replacement = 0xfffd;

	return;
}

/*
 * __replace_hex_utf32()
 *
 * Replace illegal, or non-identical hex value to UTF-32, and then
 * call PUTU macro (call write_unicode() function) to write
 * appropriate Unicode representative like UTF-8 into output buffer
 *
 * This function is called from __replace_hex() function defined in
 * each conversion modules. In case Unicode based conversion module,
 * this should be called.
 */
size_t
__replace_hex_utf32(
	unsigned char   hex,            /* hex to write */
	unsigned char   **pip,          /* point pointer to input buf */
	char            **pop,          /* point pointer to output buf */
	size_t          *poleft,        /* point #bytes left in output buf */
	__icv_state_t	*st,		/* state */
	int  		caller)	        /* caller */
{
	unsigned char	*ip = *pip;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* return value */
	
	unsigned char	first_half = ((hex >> 4) & 0x0f);
	unsigned char	second_half = (hex & 0x0f);

	if ((st->tmpbuf != (__tmpbuf_t *)NULL) && 
			!(caller & __NEXT_OF_ESC_SEQ)) {
		if((rv = __next_of_esc_seq(&ip, &op, &oleft,
				st, caller)) == -1) {
			goto ret;
		}
	}

	if(first_half < 0xa) {
		first_half += 0x30;
	} else {
		first_half += 0x37;
	}

	if(second_half < 0xa) {
		second_half += 0x30;
	} else {
		second_half += 0x37;
	}

	if ((caller & ~__NEXT_OF_ESC_SEQ) == __ICV_ILLEGAL) {
		PUTU('I', "REPLACE_HEX", 1);
		PUTU('L', "REPLACE_HEX", 1);
	} else { /* __ICV_NON_IDENTICAL */
		PUTU('N', "REPLACE_HEX", 1);
		PUTU('I', "REPLACE_HEX", 1);
	}
	PUTU('-', "REPLACE_HEX", 1);
	PUTU('-', "REPLACE_HEX", 1);
	PUTU((unsigned int)first_half, "REPLACE_HEX", 1);
	PUTU((unsigned int)second_half, "REPLACE_HEX", 1);

ret:
	if (rv != (size_t)-1) {
		/* update *pop and *poleft only on successful return */
		*pop = op;
		*poleft = oleft;
	}

	return (rv);
}

/*
 * __replace_invalid_utf32
 *
 * Replace invalid character with pre-defined replacement character.
 * pre-defined replacement character is set in conversion descriptor
 * when _icv_open() or _icv_open_attr() is called.
 */
size_t
__replace_invalid_utf32(
	unsigned char   **pip,          /* point pointer to input buf */
	char            **pop,          /* point pointer to output buf */
	size_t          *poleft,        /* point #bytes left in output buf */
	__icv_state_t	*st)		/* state */
{
	unsigned char	*ip = *pip;
	char		*op = *pop;
	size_t		oleft = *poleft;
	size_t		rv = (size_t)0;		/* retrun value */

	PUTU(st->replacement, "REPLACE_INVALID", 1);

ret:
	if (rv != (size_t)-1) {
		/* update *pop and *poleft only on successful return */
		*pop = op;
		*poleft = oleft;
	}
	return (rv);
}

/*
 * __restore_hex_ucs()
 *
 * Restore hex value when "IL--XX" or "NI--XX' is encountered.
 * return value:
 * 	0: done nothing 
 * 	1: done restoring ascii value, go to next loop
 *	-1: error, conversion should abort
 */
size_t
__restore_hex_ucs(
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
	unsigned char	hexval;

	int		rv = 0; /* return value */
	int		i;
	unsigned char	*ptr;

	unsigned int	ucs[PREFIX_LENGTH];
	unsigned int	illegal_prefix[PREFIX_LENGTH] = {'I', 'L', '-', '-'};
	unsigned int	non_identical_prefix[PREFIX_LENGTH] = {'N', 'I', '-', '-'};
	unsigned int	first_half, second_half;

	/*
	 * check if the string equal with prefix. The start address is
	 * an addres of previous character.
	 */
	ptr = ip - __SIZE_OF_UCS;
	for (i = 0; i < PREFIX_LENGTH; i++) {
#if     defined(JFP_ICONV_FROMCODE_UTF32)

		if (st->little_endian == B_TRUE) {
			ucs[i] = 0U;
			ucs[i] |= (unsigned int)*(ptr++) << 0;
			ucs[i] |= (unsigned int)*(ptr++) << 8;
			ucs[i] |= (unsigned int)*(ptr++) << 16;
			ucs[i] |= (unsigned int)*(ptr++) << 24;
		} else {
			ucs[i] = 0U;
			ucs[i] |= (unsigned int)*(ptr++) << 24;
			ucs[i] |= (unsigned int)*(ptr++) << 16;
			ucs[i] |= (unsigned int)*(ptr++) << 8;
			ucs[i] |= (unsigned int)*(ptr++) << 0;
		}

#else   /* JFP_ICONV_FROMCODE_UTF16 or JFP_ICONV_FROMCODE_UCS2 */

		if (st->little_endian == B_TRUE) {
			ucs[i] = 0U;
			ucs[i] |= (unsigned int)*(ptr++) << 0;
			ucs[i] |= (unsigned int)*(ptr++) << 8;
		} else {
			ucs[i] = 0U;
			ucs[i] |= (unsigned int)*(ptr++) << 8;
			ucs[i] |= (unsigned int)*(ptr++) << 0;
		}

#endif
	}

	/* prefix has been detected, process to get hex value */
	if (!memcmp(ucs, illegal_prefix, sizeof(ucs)) ||
		!memcmp(ucs, non_identical_prefix, sizeof(ucs))) {

#if     defined(JFP_ICONV_FROMCODE_UTF32)

		if (st->little_endian == B_TRUE) {
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
		} else {
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
		}

#else   /* JFP_ICONV_FROMCODE_UTF16 or JFP_ICONV_FROMCODE_UCS2 */

		if (st->little_endian == B_TRUE) {
			first_half = 0U;
			first_half |= (unsigned int)*(ptr++) << 0;
			first_half |= (unsigned int)*(ptr++) << 8;
			second_half = 0U;
			second_half |= (unsigned int)*(ptr++) << 0;
			second_half |= (unsigned int)*(ptr++) << 8;
		} else {
			first_half = 0U;
			first_half |= (unsigned int)*(ptr++) << 8;
			first_half |= (unsigned int)*(ptr++) << 0;
			second_half = 0U;
			second_half |= (unsigned int)*(ptr++) << 8;
			second_half |= (unsigned int)*(ptr++) << 0;
		}
#endif
		/* if hex value is detected, put it to output */
		if (ISHEXNUM(first_half) && ISHEXNUM(second_half)) {
			__ATOI(first_half);
			__ATOI(second_half);
			hexval = (unsigned char)((first_half << 4) +
				 second_half);
			NPUT(hexval, "RESTORE_HEX");

			/* move pointer to the end of replacement */
			ip += (PREFIX_LENGTH + 1) * __SIZE_OF_UCS;
			ileft -= (PREFIX_LENGTH + 1) * __SIZE_OF_UCS;

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

