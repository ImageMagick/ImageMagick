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
  'b41936e37b153bc3627167a6ff9520d92fa256e7cc967ba29acfbaad5812ff11' );

#
# Motion Picture Experts Group file interchange format
#
++$test;
testRead( 'input.mpg',
  '113b91329ec4ad7d147e910e5fd8340d06db419ce6d0258de8993ec4673fc7bc' );

1;
