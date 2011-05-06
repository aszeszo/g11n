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
# Copyright (c) 2006, 2011, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master

SUBDIRS		= tbls

all		:=	TARGET = all
clean		:=	TARGET = clean
clobber		:=	TARGET = clobber

all install lint clean clobber: $(SUBDIRS)

$(SUBDIRS): FRC
	cd $@; $(MAKE) $(TARGET)

# let the .so compilation be driven by present recoding tables
TABLES:sh =	cd tbls/ && ls *tbl

ALL_SOS	= 	$(TABLES:%.tbl=%.so)


CFLAGS		+= $(XREGSFLAG) -KPIC -G -dy -D_REENTRANT
CFLAGS64	+= $(XREGSFLAG64) -KPIC -G -dy -D_REENTRANT

LDFLAGS		= -G -z defs
LDLIBS		= -lc

ROOT_DIR	= $(FILEROOT)/usr/lib/iconv
ROOT_DIR64	= $(ROOT_DIR)/$(MACH64)

CMN		= ../utf-8
GEN_INCLUDE	= ./gen-include

.NO_PARALLEL:

.PARALLEL: $(ALL_SOS)


all: $(ALL_SOS)

install: all $(ROOT_DIR) $(ROOT_DIR64)
	for f in $(ALL_SOS) ; do \
		TMP=`echo $$f | $(TR) "_" "%"` ; \
		echo installing $$TMP to $(ROOT_DIR) ; \
		$(RM) $(ROOT_DIR)/$$TMP ; \
		$(CP) $$f $(ROOT_DIR)/$$TMP ; \
		echo installing $$TMP to $(ROOT_DIR)/$(MACH64) ; \
		$(CP) $(MACH64)/$$f $(ROOT_DIR)/$(MACH64)/$$TMP ; \
	done
	-@echo "done."

$(ROOT_DIR) $(ROOT_DIR64) $(MACH64): FRC
	/usr/bin/test -d $@ || $(INSDIR) $@

common.o: $(CMN)/common.c $(CMN)/common.h $(CMN)/common_defs.h
	$(CC) $(CFLAGS) -c -o $@ $(CMN)/common.c

$(MACH64)/common.o: $(CMN)/common.c $(CMN)/common.h $(CMN)/common_defs.h $(MACH64)
	$(CC) $(CFLAGS64) -c -o $@ $(CMN)/common.c

CMNDEPS		= $(CMN)/common_defs.h common.o $(MACH64)/common.o
LDLIBS32	= common.o $(LDLIBS)
LDLIBS64	= $(MACH64)/common.o $(LDLIBS)

$(ALL_SOS): $(MACH64) euro.h euro.c tbl.h $(CMNDEPS)
	TABLE=`echo $@ | $(TR) -d "-" "" | /usr/bin/cut -d. -f1` ; \
	$(CC) $(CFLAGS) -DT_$$TABLE euro.c -c -o $@.o ; \
	$(CC) $(CFLAGS64) -DT_$$TABLE euro.c -c -o $(MACH64)/$@.o
	$(LD) $(LDFLAGS) -o $@ $@.o $(LDLIBS32)
	$(LD) $(LDFLAGS) -o $(MACH64)/$@ $(MACH64)/$@.o $(LDLIBS64)


tbl.h: $(SUBDIRS) $(GEN_INCLUDE) $(TABLES:%=tbls/%)
	$(GEN_INCLUDE) > $@

clean: 
	$(RM) *.o *.so *.s tbl.h
	$(RM) -r $(MACH64)

clobber: clean

FRC:

