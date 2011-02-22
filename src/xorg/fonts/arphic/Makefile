#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
# Copyright (c) 2009, 2011, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master

SUBDIRS = \
	ukai \
	uming

FILES = fonts.dir \
	fonts.scale \
	fonts.alias


ROOT_FONT_DIR 	= $(FILEROOT)/usr/share/fonts/TrueType/arphic
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/arphic

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

all: $(SUBDIRS)

install: $(SUBDIRS) .files_installed

clean: $(SUBDIRS)
	$(RM) .*installed

.files_installed:
	$(INSDIR) -m 755 $(ROOT_FONT_DIR)
	( for f in $(FILES); do \
		$(INS) $(ROOT_FONT_DIR) $$f; \
	done; )
	touch $@

$(SUBDIRS): FRC
	@cd $@; pwd; $(MAKE) $(MFLAGS) $(TARGET)

tarball:

FRC:

