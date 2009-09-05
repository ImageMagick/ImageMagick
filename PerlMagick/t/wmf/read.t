#!/usr/bin/perl
#
# Test reading WMF files
#
# Whenever a new test is added/removed, be sure to update the
# 1..n ouput.
#
BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/wmf' || die 'Cd failed';

testReadCompare('wizard.wmf', '../reference/wmf/wizard.miff',
                q//, 0.53, 1.0);
++$test;
testReadCompare('clock.wmf', '../reference/wmf/clock.miff',
                q//, 0.44, 1.0);

