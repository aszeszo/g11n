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
 */

#ifndef	UCS_TO_SB_H
#define	UCS_TO_SB_H


#include "common_defs.h"


/*
 * IMPORTANT:
 * Since we are using binary search on the table, especially, on u8 data
 * field, the table should be sorted by ascending order on u8 data field.
 */
static const to_sb_table_component_t u4_sb_tbl[] = {
#if defined(US_ASCII)
#include "tbls/ucs4_to_us-ascii.tbl"

#elif defined(ISO_8859_1)
#include "tbls/ucs4_to_iso-8859-1.tbl"

#elif defined(ISO_8859_2)
#include "tbls/ucs4_to_iso-8859-2.tbl"

#elif defined(ISO_8859_3)
#include "tbls/ucs4_to_iso-8859-3.tbl"

#elif defined(ISO_8859_4)
#include "tbls/ucs4_to_iso-8859-4.tbl"

#elif defined(ISO_8859_5)
#include "tbls/ucs4_to_iso-8859-5.tbl"

#elif defined(ISO_8859_6)
#include "tbls/ucs4_to_iso-8859-6.tbl"

#elif defined(ISO_8859_7)
#include "tbls/ucs4_to_iso-8859-7.tbl"

#elif defined(ISO_8859_8)
#include "tbls/ucs4_to_iso-8859-8.tbl"

#elif defined(ISO_8859_9)
#include "tbls/ucs4_to_iso-8859-9.tbl"

#elif defined(ISO_8859_10)
#include "tbls/ucs4_to_iso-8859-10.tbl"

#elif defined(ISO_8859_13)
#include "tbls/ucs4_to_iso-8859-13.tbl"

#elif defined(ISO_8859_14)
#include "tbls/ucs4_to_iso-8859-14.tbl"

#elif defined(ISO_8859_15)
#include "tbls/ucs4_to_iso-8859-15.tbl"

#elif defined(ISO_8859_16)
#include "tbls/ucs4_to_iso-8859-16.tbl"

#elif defined(KOI8_R)
#include "tbls/ucs4_to_koi8-r.tbl"

#elif defined(KOI8_U)
#include "tbls/ucs4_to_koi8-u.tbl"

#elif defined(PTCP154)
#include "tbls/ucs4_to_ptcp154.tbl"

#elif defined(CP437)
#include "tbls/ucs4_to_cp437.tbl"

#elif defined(CP720)
#include "tbls/ucs4_to_cp720.tbl"

#elif defined(CP720)
#include "tbls/ucs4_to_cp720.tbl"

#elif defined(CP737)
#include "tbls/ucs4_to_cp737.tbl"

#elif defined(CP775)
#include "tbls/ucs4_to_cp775.tbl"

#elif defined(CP850)
#include "tbls/ucs4_to_cp850.tbl"

#elif defined(CP852)
#include "tbls/ucs4_to_cp852.tbl"

#elif defined(CP855)
#include "tbls/ucs4_to_cp855.tbl"

#elif defined(CP857)
#include "tbls/ucs4_to_cp857.tbl"

#elif defined(CP860)
#include "tbls/ucs4_to_cp860.tbl"

#elif defined(CP861)
#include "tbls/ucs4_to_cp861.tbl"

#elif defined(CP862)
#include "tbls/ucs4_to_cp862.tbl"

#elif defined(CP863)
#include "tbls/ucs4_to_cp863.tbl"

#elif defined(CP864)
#include "tbls/ucs4_to_cp864.tbl"

#elif defined(CP865)
#include "tbls/ucs4_to_cp865.tbl"

#elif defined(CP866)
#include "tbls/ucs4_to_cp866.tbl"

#elif defined(CP869)
#include "tbls/ucs4_to_cp869.tbl"

#elif defined(CP874)
#include "tbls/ucs4_to_cp874.tbl"

#elif defined(CP1250)
#include "tbls/ucs4_to_cp1250.tbl"

#elif defined(CP1251)
#include "tbls/ucs4_to_cp1251.tbl"

#elif defined(CP1252)
#include "tbls/ucs4_to_cp1252.tbl"

#elif defined(CP1253)
#include "tbls/ucs4_to_cp1253.tbl"

#elif defined(CP1254)
#include "tbls/ucs4_to_cp1254.tbl"

#elif defined(CP1255)
#include "tbls/ucs4_to_cp1255.tbl"

#elif defined(CP1256)
#include "tbls/ucs4_to_cp1256.tbl"

#elif defined(CP1257)
#include "tbls/ucs4_to_cp1257.tbl"

#elif defined(CP1258)
#include "tbls/ucs4_to_cp1258.tbl"

#else
#error	"Error - nothing defined."
#endif
};

#endif	/* UCS_TO_SB_H */
