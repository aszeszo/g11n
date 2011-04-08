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
# Copyright (c) 2006, 2011, Oracle and/or its affiliates.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/Makefile.master


all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean


CHARMAPS = ja_JP.PCK ja_JP.eucJP ko ko_KR.EUC th_TH.TIS620 zh_CN.EUC zh_CN.GB18030 zh_CN.GBK zh_HK.BIG5HK zh_TW.BIG5 zh_TW.EUC

TARGETDIR =	$(FILEROOT)/usr/lib/localedef/src/charmaps

FILES =		$(CHARMAPS:%=%.charmap)

TARGETDIRS =	$(TARGETDIR)


all: $(TARGETDIRS) $(FILES)

include $(SRC)/src/Makefile.targ

install: $(FILES) $(TARGETDIRS) $(FILES.obj)
	$(INSDIR) $(FILEROOT)/lib/localedef/charmaps

clean:
	$(RM) $(FILES)

