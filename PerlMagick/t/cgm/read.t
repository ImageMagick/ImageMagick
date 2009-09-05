#!/usr/bin/perl
#
# Test reading CGM files
#
# Written by Bob Friesenhahn
#
# Whenever a new test is added/removed, be sure to update the
# 1..n ouput.
#
BEGIN { $| = 1; $test=1; print "1..1\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/cgm' || die 'Cd failed';

testReadCompare('CGM:input.cgm', '../reference/cgm/read.miff', q//, 0.0 0.0);

