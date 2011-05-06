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
#
# Copyright (c) 2011, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master

SRC_TBLDIR	= ../../tbls
TOOLS		= .

# only work with tables that use ICV_TYPE_NON_IDENTICAL_CHAR
TABLES:sh	= cd ../../tbls && /usr/bin/grep -l NON_IDENTICAL *tbl 

REWRITE_TBL	= $(TOOLS)/rewrite-table.pl
TR_TABLE	= $(TOOLS)/translit.accents-any

# need to make fallback (*.fb) files first
SUBDIRS		= $(SRC_TBLDIR)

install		:=	TARGET = install
all		:=	TARGET = all
lint		:=	TARGET = lint
clean		:=	TARGET = clean
clobber		:=	TARGET = clobber

all install lint clean clobber: $(SUBDIRS)

$(SUBDIRS): FRC
	cd $@; $(MAKE) $(TARGET)


all: $(TABLES)

# create final tables
%.tbl: $(SRC_TBLDIR)/%.tbl $(SRC_TBLDIR)/%.fb $(REWRITE_TBL) $(TR_TABLE)
	$(REWRITE_TBL) $< $(TR_TABLE) `cat $(@:%.tbl=$(SRC_TBLDIR)/%.fb)` > $@ || ( $(RM) $@; false )

# compare with tables currently in use
diff:
	@for f in `ls *tbl`; do \
		echo $$f:; \
		diff $$f $(SRC_TBLDIR)/$$f; \
	done

clean: 
	$(RM) *.tbl

clobber: clean

FRC:

