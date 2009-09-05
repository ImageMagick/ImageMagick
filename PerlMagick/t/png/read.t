#!/usr/bin/perl
#
# Test reading PNG images
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
testRead( 'input_bw.png',
  '349c2ff9310d578051e40e80d42cfc36ca29ba93e353df175219f7448da5eeee' );

#
# 2) Test Monochrome PNG
# 
++$test;
print( "8-bit grayscale PNG ...\n" );
testRead( 'input_mono.png',
  'cda5c7a8ba8250de624af6dc825ad6772ebba3a7fa6da756c5b1ca228b62f8ac' );

#
# 3) Test 16-bit Portable Network Graphics
# 
++$test;
print( "16-bit grayscale PNG ...\n" );
testRead( 'input_16.png',
  'fa6b164245b385b3dea5764074be2c959a503dde90ecb1d4ba9c76a46bb8e4e6',
  '82f48df83eec5bacbe2c38f13ce7e2219e5e318f4b2974d928d0ea7f7cec65fd' );
#
# 4) Test 256 color pseudocolor PNG
# 
++$test;
print( "8-bit indexed-color PNG ...\n" );
testRead( 'input_256.png',
  '066c0047c6e7e3f4cda1c86224441bfd5f522b5805b2a9190dcfa5294d94e4bd' );

#
# 5) Test TrueColor PNG
# 
++$test;
print( "24-bit Truecolor PNG ...\n" );
testRead( 'input_truecolor.png',
  '55913611798c087b9300b14d3baeda08a142910ad120379a9308a6b8c8b2f6e8' );

#
# 6) Test Multiple-image Network Graphics
# 
++$test;
print( "MNG with 24-bit Truecolor PNGs...\n" );
testRead( 'input.mng',
  '030111e35491010550814468283f13a8d3d621efb0031bae005bd86e9d0038c5' );

