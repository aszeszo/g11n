#!/usr/bin/perl
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

#
#  Rewrite tbls/<TBL> to format incl. transliteration for
#  ICV_TYPE_NON_IDENTICAL_CHAR entries.
#
#  Works for ASCII-compatible encodings.
#

use strict;

sub usage {
	print STDERR "usage: $0 <tbl> <translit> <fallback>\n";
	die "\n";
}

sub read_file {
	my $in = @_[0];
	my @lines = [];
	open (FILE, "<$in") || return @lines;
	@lines = <FILE>;
	close (FILE);
	return @lines;
}

############################################################################

my $input = shift or usage();
my $translit = shift or usage();
my $fallback = shift or usage();
(my $fromcode = $input) =~ s/_.*//;
$fromcode =~ s/.*\///;

open INPUT, "<", $input or die $!;

my $counter = 0;
my @parts;

my @tr_lines = read_file ($translit);

while (my $line = <INPUT>) {

	chop $line;
	(my $prefix = $line) =~ s/(\/\* [0-9A-Fx]* \*\/\ *{).*/$1/;
	(my $ch = $line) =~ s/\/\* [0-9A-Fx]* \*\/\ *{\ \ "?([\\x0-9A-F]*)"?.*/$1/;
	undef @parts;
	@parts = split(/,\ ?/, $line);
	if ($#parts == 3) {
		$parts[2] = join(", ", ( $parts[2], $parts[3] ));
		$parts[3] = "";
	}

	if ($line =~ /^\/\* [0-9A-Fx]* \*\/.*ICV_TYPE_NON_IDENTICAL_CHAR/) {

		# get the character in hex
		(my $chid = $line) =~ s/\/\* 0x([0-9A-Fx]*) \*\/\ *{.*/$1/;

		# convert to string containing utf-8 integer (0x0000C280)
		my $utf8 = qx/printf \"\\x$chid\" | \/usr\/bin\/iconv -f $fromcode -t UTF-8 | xxd -i/;
		chop $utf8;
		$utf8 =~ s/0x//g;
		$utf8 =~ s/, //;
		$utf8 =~ s/^ *//;
		$utf8 =~ s/00*: //;
		$utf8 = sprintf("0x%08s", uc($utf8));
	
		#print "$chid -> '$utf8'\n";

		# grep transliteration table
		my @matching = grep(/{ $utf8,/, @tr_lines);
		my $translit = "0x00"; # $fallback;
		my $text = "";
	 	if ($#matching > -1) {
			my $tr_line = @matching[0];
			chop $tr_line;
			($translit = $tr_line) =~ s/{ 0x.*,  *(0x[^ ]*)  *}.*/$1/;
			($text = $tr_line) =~  s/.*},\s*(\/\*.*\*\/)/  $1/;

			#print $translit, " ", $text, "\n";
		}

		print "$prefix  $translit, ";
		if ($parts[2] eq "") {
			$parts[1] =~ s/}  *.*/}/;
		}
		print "$parts[1],$text";

	} elsif ($line =~ /^\/\* [0-9A-Fx]* \*\/.*ICV_TYPE_ILLEGAL_CHAR/) {

		# ILLEGAL -> use specified fallback

		print "$prefix  $fallback, ";
		print "$parts[1]";
		if ($parts[2] eq "") {
			print ",";
		}

	#} elsif ($line =~ /^\/\* [0-9A-Fx]* \*\//) {
		# standard conversion
	} else {
		print $line;
	}
	print "\n";
}

close INPUT;

