#!/usr/bin/perl
#
# Test reading MPEG files
#
# Whenever a new test is added/removed, be sure to update the
# 1..n ouput.
#
BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/mpeg' || die 'Cd failed';

#
# Motion Picture Experts Group file interchange format (version 2)
#
testRead( 'input.m2v',
  'd3096f660b590223d1047d423f2911b3c5b5e66f4ac09a1a16825dcf389aecce' );

#
# Motion Picture Experts Group file interchange format
#
++$test;
testRead( 'input.mpg',
  'c8fa2aec1317786ed8b9d68f3353f812bf426699442c6d16768b46d95650ec1a' );

1;
