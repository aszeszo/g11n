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

TARGETDIR =	$(FILEROOT)/usr/kernel/strmod

SOURCES =	u8sb.c common.h iso-8859-1_to_utf8.h iso-8859-2_to_utf8.h koi8-r_to_utf8.h utf8_to_iso-8859-1.h utf8_to_iso-8859-2.h utf8_to_koi8-r.h

MODULES	= 	u8koi8 u8lat1 u8lat2

FILES =		$(MODULES:%=$(MACH64)/%)

SUBDIRS = 

TARGETDIRS =	$(TARGETDIR) $(TARGETDIR)/$(MACH64)


# 64bit kernel modules compilation flags
# (32bit kernel is EOF)
CFLAGS		= -xO3 -xc99=%all -D_KERNEL -D_SYSCALL32 -D_ASM_INLINES -Xa -xspace -xdebugformat=stabs  -D_SYSCALL32_IMPL -D_ELF64 -D_DDI_STRICT -Dsun -D__sun -D__SVR4 -v -g -m64 -errtags=yes -errwarn=%all -W0,-noglobal -W0,-xglobalstatic -xstrconst 
CFLAGS_sparcv9	= $(CFLAGS) -xchip=ultra -Wc,-xcode=abs32 -Wc,-Qiselect-regsym=0 -W0,-Lt -Wc,-Qiselect-T1 -Wd,-xsafe=unboundsym -Wc,-Qassembler-ounrefsym=0 -W2,-xwrap_int -xregs=no%float 
CFLAGS_amd64	= $(CFLAGS) -Ui386 -U__i386 -xmodel=kernel -Wu,-O~Z -xvector=none -Wu,-save_args -xildoff 
LDFLAGS		= -r -zstrip-class=annotate -zstrip-class=comment -zstrip-class=debug -zstrip-class=exclude

$(MACH64)/u8lat1.o := CFLAGS += -DISO_8859_1                                                                                                                                                                                     
$(MACH64)/u8lat2.o := CFLAGS += -DISO_8859_2                                                                                                                                                                                     
$(MACH64)/u8koi8.o := CFLAGS += -DKOI8_R  


all: $(MODULES) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

install: $(FILES.obj) $(SUBDIRS)

clean: $(SUBDIRS)
	$(RM) -r $(MACH64) 

$(MODULES:%=$(MACH64)/%.o): $(MACH64) $(SOURCES)
	$(CC) $(CFLAGS_$(MACH64)) -c -o $@ u8sb.c

$(MODULES:%=$(MACH64)/%): $$(@:%=%.o)
	$(LD) $(LDFLAGS) -o $@ $^

$(MACH64):
	$(INSDIR) $(MACH64)


FRC:

