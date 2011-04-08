#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright (c) 2009, 2011, Oracle and/or its affiliates.  All rights reserved.
# Use is subject to license terms.
#

#FIXME: removed openwin/lib/locale from the build targets, since we're using Xorg's Xi18n locale database files.
#SUBDIRS = lib locale_src/base openwin/lib/locale openwin/lib/X11/fonts/ share/lib/ openwin/keytables xorg/keytables uts cmd
SUBDIRS = lib kernel locale_src/base locale_src/charmap share/lib/ xorg/keytables xorg/fonts uts cmd

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber


all install clean: $(SUBDIRS)

$(SUBDIRS): FRC
	@cd $@; pwd; $(MAKE) $(MFLAGS) $(TARGET)

FRC:

