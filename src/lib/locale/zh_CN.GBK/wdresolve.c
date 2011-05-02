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
 * Copyright (c) 1991, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#ident	"$Id: wdresolve.c,v 1.3 2003/12/23 21:22:20 fzhang Exp $"


#include <stdlib.h>
#include <widec.h>
#include <wctype.h>
/* #include <zh.GBK/xctype.h> */
#include <locale.h>

#define iscalpha(C)     _iswctype((C), _E3)     /* 1 Roman chars in cs1 */
#define iscblank(C)     iscspace(C)             /* 2 space char */
#define iscdigit(C)     _iswctype((C), _N)      /* 3 GB cs1 numeric. */
#define iscgb(C)        _iswctype((C), _E7)     /* 4 GB char */
#define iscgbk(C)               _iswctype((C), _E9)
#define iscgen(C)       _iswctype((C), _E19)    /* 5 General symbols.*/
#define iscgreek(C)     _iswctype((C), _E13)    /* 6 Greek chars in cs1 */
#define ischanzi(C)     _iswctype((C), _E2)     /* 7 Hanzi: 0xb0a1 - 0xfr7 */
#define ischira(C)      _iswctype((C),  _E11)   /* 8 Hiragana char in GB */
#define isckata(C)      _iswctype((C),  _E12)   /* 9 katakana char in GB */
#define iscline(C)      _iswctype((C), _E16)    /* 10 Ruled line symbols */
#define isclower(C)     _iswctype((C), _L)      /* 11 Lowercase char */
#define iscnumber(C)    _iswctype((C), _E4)     /* 12 number chars */
#define iscparen(C)     _iswctype((C), _E10)    /* 13 parenthese chars */
#define iscaccent(C)    _iswctype((C), _E14)    /* 14 phonetic accent */
#define iscphonetic(C)  _iswctype((C), _E21)    /* 15 phonetic symbols */
#define iscpinyin(C)    _iswctype((C), _E20)    /* 16 pinyin symbols */
#define iscpunct(C)     _iswctype((C), _P)      /* 17 punctuation */
#define iscrussian(C)   _iswctype((C), _E15)    /* 18 Russian character */
#define iscsci(C)       _iswctype((C), _E18)    /* 19 Scientific symbols */
#define iscspace(C)     _iswctype((C), _S)      /* 20 space char */

#define iscspecial(C)   _iswctype((C), _E6)     /* 21 special characters */
#define iscunit(C)      _iswctype((C), _E17)    /* 22 unit symbols */
#define iscupper(C)     _iswctype((C), _U)      /* 23 uppercase chars */

static int wd_bind_strength[][10] = {
	2, 3, 3, 3, 3, 3, 3, 3, 3, -1,
	3, 6, 2, 2, 6, 3, 2, 2, 5, -1,
	1, 3, 3, 3, 5, 2, 2, 2, 2, -1,
	1, 2, 3, 6, 2, 2, 2, 2, 2, -1,
	5, 5, 5, 6, 5, 5, 3, 2, 2, -1,
	2, 2, 2, 2, 2, 6, 2, 3, 2, -1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, -1,
	3, 3, 3, 3, 3, 3, 3, 6, 3, -1,
	2, 5, 3, 3, 6, 2, 2, 2, 5, -1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1, -1,
	0
};

static int	initialized = 0;
static int	conservative_edit = 0;
static char	*multicolumn_filler;

static void
_init(void)
{
	register int i, n;
	char env_name[64];

	/* differenciate text editor from text formatter (user's option) */
	if (getenv("MB_CONSERVATIVE_EDIT"))
		conservative_edit = 1;

	/* get filler character for multicolumn character at right margin */
	multicolumn_filler = getenv("MC_FILLER");

	initialized = 1;
}

int
_wdchkind_(wc)
wchar_t wc;
{

	if (!initialized)
		_init();

	/* Chinese ideograms */
	if ( ischanzi(wc) || iscgbk(wc))
                return (2);

	/* zhuyin symbols */
	if ( iscpinyin(wc) || iscphonetic(wc) )
                return (3);

	if (iscparen(wc) || iscpunct(wc) )
		return(4);

	/* Greek alphabet */
	if ( iscgreek(wc) )
		return(5);

	if ( iscunit(wc) || iscline(wc) || iscsci(wc) || iscgen(wc) )
		return(6);
			
	if ( iscspace(wc) )
		return(7);

	if ( iscdigit(wc) || iscupper(wc) || isclower(wc) )
		return(8);

	if (wc > 0L && wc < 0x7f)
		return (isalpha(wc) || isdigit(wc) || wc == '_');

	return (0);
}

int
_wdbindf_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
	int	i, j;

	if (!initialized)
		_init();

	i = _wdchkind_(wc1);
	j = _wdchkind_(wc2);

	return( wd_bind_strength[i][j] );
}

wchar_t *
_wddelim_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
	static wchar_t delim[2] = {0};
	int	i, j;

	if (!initialized)
		_init();

	if (conservative_edit && type == 2) {
		delim[0] = ' ';
		delim[1] = 0;
		return (&delim[0]);
	}

	i = _wdchkind_(wc1);
	j = _wdchkind_(wc2);

	if ( (i==1 && j==1) || (i==3 && j==3) || (i==10 && j==10)
			|| (i==4 && j==4) || (i==7 && j==7) || (i==8 && j==8) )
		delim[0] = 0;
	else {
		delim[0] = ' ';
		delim[1] = 0;
	}

	return (&delim[0]);
}

wchar_t
_mcfiller_()
{
	wchar_t fillerchar;

	if (!initialized)
		_init();
	if (!multicolumn_filler) {
		fillerchar = '~';
	} else {
		if (mbtowc(&fillerchar, multicolumn_filler, MB_CUR_MAX) <= 0)
			fillerchar = '~';
		if (!iswprint(fillerchar))
			fillerchar = '~';
	}
	return (fillerchar);
}
