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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _FSEXAM_HISTORY_
#define _FSEXAM_HISTORY_

typedef struct _History_info History_info;

History_info *fsexam_history_open (char *);
gboolean fsexam_history_close (History_info *);
gboolean fsexam_history_update (History_info *);

gboolean fsexam_history_undo (History_info *, guint, ConvType *, char **, char **, char **);
guint fsexam_history_put (History_info *, ConvType, char *, char *, char *, gboolean);
gchar *fsexam_history_get_reverse (History_info *, ConvType *, char *, char *);
gchar *fsexam_history_get_reverse_by_value (History_info *, ConvType *, char *, char *);
gchar *fsexam_history_get_reverse_by_value2 (History_info *, ConvType *, char *, char *);

#endif
