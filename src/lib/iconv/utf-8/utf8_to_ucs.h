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

#ifndef	UTF8_TO_UCS_H
#define	UTF8_TO_UCS_H

#include "common.h"
#include "common_ucs.h"

#define STATE_T		utf8_state_t

#if defined(UCS_2) || defined(UCS_2BE) || defined(UCS_2LE) || \
    defined(UCS_2_BIG_ENDIAN) || defined(UCS_2_LITTLE_ENDIAN)

#define PUTC(u4)	PUTC_UCS2(u4, cd->little_endian)

#elif defined(UTF_16) || defined(UTF_16BE) || defined(UTF_16LE) || \
      defined(UTF_16_BIG_ENDIAN) || defined(UTF_16_LITTLE_ENDIAN)

#define PUTC(u4)	PUTC_UTF16(u4, u4_2, cd->little_endian)

#elif defined(UCS_4) || defined(UCS_4BE) || defined(UCS_4LE) || \
      defined(UCS_4_BIG_ENDIAN) || defined(UCS_4_LITTLE_ENDIAN) || \
      defined(UTF_32) || defined(UTF_32BE) || defined(UTF_32LE) || \
      defined(UTF_32_BIG_ENDIAN) || defined(UTF_32_LITTLE_ENDIAN)

#define PUTC(u4)	PUTC_UCS4(u4, cd->little_endian)

#else
#error	"Fatal: one of the UCS macros needs to be defined."
#endif

#endif	/* UTF8_TO_UCS_H */
