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
 * Copyright (c) 2010, 2011, Oracle and/or its affiliates. All rights reserved.
 */

#include "../utf-8/common.h"

#define STATE_T		icv_state_t

/* Conversion table structure */
typedef struct {
	unsigned int	ch;
	signed char	sz;
} table_component_t;

/*
 * Mapping tables
 * tbl.h generated from tbls/ files using ./gen-include script
 */
#include "tbl.h"

#ifndef ICV_NON_IDENTICAL_REPLACEMENT_CHAR
#error "ICV_NON_IDENTICAL_REPLACEMENT_CHAR not defined"
#endif

