#!/usr/bin/perl
#
# Test reading WMF files
#
# Whenever a new test is added/removed, be sure to update the
# 1..n output.
#
BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/wmf' || die 'Cd failed';

testReadCompare('wizard.wmf', '../reference/wmf/wizard.gif', q//, 0.01, 1.0);
++$test;
testReadCompare('clock.wmf', '../reference/wmf/clock.gif', q//, 0.01, 1.0);

