#!/usr/bin/perl
#
# Test reading Radiance file format
#
# Whenever a new test is added/removed, be sure to update the
# 1..n ouput.
#
BEGIN { $| = 1; $test=1; print "1..1\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/rad' || die 'Cd failed';

testRead( 'RAD:input.rad',
  '5927b5f4dcf084e5f5fd841df8e4dfb40e4fe637419d022fc6a0b52f02600881');
