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

TABLES:sh	= ls *tbl

# non-identical fallback character definitions
FALLBACKS	= $(TABLES:%.tbl=%.fb)

all: $(FALLBACKS)


CP1250_dhn.fb \
CP852_dhn.fb \
dhn_maz.fb \
maz_dhn.fb \
8859-2_dhn.fb:
	echo "0x20" > $@

8859-1_IBM-037.fb \
8859-1_IBM-500.fb \
8859-16_IBM-870.fb:
	echo "0x6F" > $@

%.fb:
	echo "0x3F" > $@


clean: 
	$(RM) *.fb

clobber: clean

FRC:

