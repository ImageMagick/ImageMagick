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
  '11fabe4dc3114c91da9d4f905bc69a2ab30608b9a550c0d775f0be06f3b39cd0' );

#
# Motion Picture Experts Group file interchange format
#
++$test;
testRead( 'input.mpg',
  '11fabe4dc3114c91da9d4f905bc69a2ab30608b9a550c0d775f0be06f3b39cd0' );

1;
