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

PROJECT		= wqy-zenhei
VER 		= $(PROJECT)
TARBALL		= $(VER)-0.6.26-0.tar.gz
SOURCE_URL	= http://downloads.sourceforge.net/project/wqy/$(PROJECT)/0.6.26/$(TARBALL)

ROOT_FONT_DIR 	= $(FONTROOT)/wqy
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/$(PROJECT)
FONTCONF_DIR	= $(FILEROOT)/etc/fonts/conf.avail

FONTS =	wqy-zenhei.ttf \
	../fonts.dir \
	../fonts.scale

# customized 44-wqy-zenhei.conf
CONF_FILES = 	../44-wqy-zenhei.conf \
		66-wqy-zenhei-sharp.conf

DOCS =	AUTHORS \
	COPYING \
	ChangeLog \
	README

all: $(VER)/.unpacked

install: $(VER)/.installed $(VER)/.conf_installed

$(VER)/.conf_installed:
	$(INSDIR) -m 755 $(FONTCONF_DIR)
	( cd $(VER); \
	for f in $(CONF_FILES); do \
		$(INS) $(FONTCONF_DIR) $$f; \
	done; )
	touch $@

include ../Makefile.fonts
include $(SRC)/Makefile.targ
