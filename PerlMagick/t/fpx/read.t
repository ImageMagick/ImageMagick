#!/usr/bin/perl
#
# Test reading FPX images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..5\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/fpx' || die 'Cd failed';

#
# 1) Test Black-and-white, bit_depth=1 FPX
# 
print( "1-bit grayscale FPX ...\n" );
testRead( 'input_bw.fpx',
  '164b30b0e46fab4b60ea891a0f13c1ec2e3c9558e647c75021f7bd2935fe1e46' );

#
# 2) Test grayscale FPX
# 
++$test;
print( "8-bit grayscale FPX ...\n" );
testRead( 'input_grayscale.fpx',
  '74416d622acf60c213b8dd0a4ba9ab4a46581daa8b7b4a084658fb5ae2ad1e4b' );

#
# 3) Test 256 color pseudocolor FPX
# 
++$test;
print( "8-bit indexed-color FPX ...\n" );
testRead( 'input_256.fpx',
  '772ef079906aa47951a09cd4ce6d62b740a391935710e7076a6716423a92db4f' );

#
# 4) Test TrueColor FPX
# 
++$test;
print( "24-bit Truecolor FPX ...\n" );
testRead( 'input_truecolor.fpx',
  'a698f2fe0c6c31f83d19554a6ec02bac79c961dd9a87e7ed217752e75eb615d7' );

#
# 5) Test JPEG FPX
# 
++$test;
print( "24-bit JPEG FPX ...\n" );
testRead( 'input_jpeg.fpx',
  '8c02bf8e953893cbd65b8a0a1fb574de50ac4cdeb2a88dbf702c8b65d82aa41b' );

