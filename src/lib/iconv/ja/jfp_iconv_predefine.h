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

/*
 * definition to control pre-processing. A lots of code is defined with
 * JFP_ICONV_FROMCODE_UTF32, JFP_ICONV_FROMCODE_UTF16 or others. The
 * little-endian (LE) and big-endian (BE) 
 */

#ifndef __JFP_ICONV_PREDEFINE_H
#define __JFP_ICONV_PREDEFINE_H

#if	defined(JFP_ICONV_FROMCODE_UTF32BE) || \
	defined(JFP_ICONV_FROMCODE_UTF32LE)
#define	JFP_ICONV_FROMCODE_UTF32
#endif

#if	defined(JFP_ICONV_FROMCODE_UTF16BE) || \
	defined(JFP_ICONV_FROMCODE_UTF16LE)
#define	JFP_ICONV_FROMCODE_UTF16
#endif

#if	defined(JFP_ICONV_FROMCODE_UCS2BE) || \
	defined(JFP_ICONV_FROMCODE_UCS2LE)
#define	JFP_ICONV_FROMCODE_UCS2
#endif

#if	defined(JFP_ICONV_TOCODE_UTF32BE) || \
	defined(JFP_ICONV_TOCODE_UTF32LE)
#define	JFP_ICONV_TOCODE_UTF32
#endif

#if	defined(JFP_ICONV_TOCODE_UTF16BE) || \
	defined(JFP_ICONV_TOCODE_UTF16LE)
#define	JFP_ICONV_TOCODE_UTF16
#endif

#if	defined(JFP_ICONV_TOCODE_UCS2BE) || \
	defined(JFP_ICONV_TOCODE_UCS2LE)
#define	JFP_ICONV_TOCODE_UCS2
#endif

#endif /* !_JFP_ICONV_PREDEFINE_H */
