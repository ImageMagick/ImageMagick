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
  'd0d08bbb6ea2f9d5d76bc134a4785f3ab25d6b03d66c64bc43b6b2a9174582d7' );

#
# Motion Picture Experts Group file interchange format
#
++$test;
testRead( 'input.mpg',
  'bbffb8cfbd98a111f7ff625fbc2cf49485d92640de702a2e76c0852ead39c6f1' );

1;
