#!/usr/bin/perl
#
# Test writing PNG images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..6\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/png' || die 'Cd failed';

#
# 1) Test Black-and-white, bit_depth=1 PNG
# 
print( "1-bit grayscale PNG ...\n" );
testReadWrite( 'input_bw.png', 'output_bw.png', q/quality=>95/,
  '7d12eaac6f41d3c2947022f382e158ea715e918ce0cd73649ec04db01239c88f');

#
# 2) Test monochrome image
#
++$test;
print( "8-bit grayscale PNG ...\n" );
testReadWrite( 'input_mono.png',
  'output_mono.png', '',
  'fa43f8c3d45c3efadab6791a6de83b5a303f65e2c1d58e0814803a4846e68593');
#
# 3) Test 16-bit Portable Network Graphics
# 
++$test;
print( "16-bit grayscale PNG ...\n" );
testReadWrite( 'input_16.png',
  'output_16.png',
  q/quality=>55/,
  'd4bed86abb1849f69f1a5afb7c5cf8798e8192ba228357f189c277198c14f5a0',
  '2d30a8bed1ae8bd19c8320e861f3140dfc7497ca8a05d249734ab41c71272f08' );
#
# 4) Test pseudocolor image
#
++$test;
print( "8-bit indexed-color PNG ...\n" );
testReadWrite( 'input_256.png',
  'output_256.png',
  q/quality=>54/,
  'e3930ba2c0d7813f21e9ac16b058c10904470853dc6a59f9f3b3f1f47da7dc2c' );
#
# 5) Test truecolor image
#
++$test;
print( "24-bit Truecolor PNG ...\n" );
testReadWrite( 'input_truecolor.png',
  'output_truecolor.png',
  q/quality=>55/,
  '7d40e88aa651fd6234780c61a6cef9f34ae8f579975240cf5d33c86217a348c9' );
#
# 6) Test Multiple-image Network Graphics
#
++$test;
print( "MNG with 24-bit Truecolor PNGs ...\n" );
testReadWrite( 'input.mng',
  'output.mng',
  q/quality=>55/,
  '0cd7b340ab0c0bceac4e95a4248b12987446a9d2df07bcb6e7e7ecd4ddc44b13' );
