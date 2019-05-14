#!/usr/bin/perl
#
# Test writing PNG images when 16bit support is enabled
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..5\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/png' || die 'Cd failed';

#
# 1) Test pseudocolor image
#
testReadWrite( 'input_256.png',
  'output_256.png',
  q/quality=>54/,
  'e3930ba2c0d7813f21e9ac16b058c10904470853dc6a59f9f3b3f1f47da7dc2c' );

#
# 2) Test truecolor image
#
++$test;
testReadWrite( 'input_truecolor.png',
  'output_truecolor.png',
  q/quality=>55/,
  '7d40e88aa651fd6234780c61a6cef9f34ae8f579975240cf5d33c86217a348c9' );

#
# 3) Test monochrome image
#
++$test;
testReadWrite( 'input_mono.png',
  'output_mono.png', '',
  'fa43f8c3d45c3efadab6791a6de83b5a303f65e2c1d58e0814803a4846e68593' );

#
# 4) Test Multiple-image Network Graphics
#
++$test;
testReadWrite( 'input.mng',
  'output.mng',
  q/quality=>55/,
  '0cd7b340ab0c0bceac4e95a4248b12987446a9d2df07bcb6e7e7ecd4ddc44b13' );

#
# 5) Test 16-bit Portable Network Graphics
# 
++$test;
testReadWrite( 'input_16.png',
  'output_16.png',
  q/quality=>55/,
  'd4bed86abb1849f69f1a5afb7c5cf8798e8192ba228357f189c277198c14f5a0',
  '2d30a8bed1ae8bd19c8320e861f3140dfc7497ca8a05d249734ab41c71272f08');


