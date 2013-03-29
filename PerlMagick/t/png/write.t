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
  '349c2ff9310d578051e40e80d42cfc36ca29ba93e353df175219f7448da5eeee');

#
# 2) Test monochrome image
#
++$test;
print( "8-bit grayscale PNG ...\n" );
testReadWrite( 'input_mono.png',
  'output_mono.png', '',
  'cda5c7a8ba8250de624af6dc825ad6772ebba3a7fa6da756c5b1ca228b62f8ac');
#
# 3) Test 16-bit Portable Network Graphics
# 
++$test;
print( "16-bit grayscale PNG ...\n" );
testReadWrite( 'input_16.png',
  'output_16.png',
  q/quality=>55/,
  '6f2b7a49124d65616f37d4dbebec3e46b6a4a4b53abc6b197dc6f7e9b4cdeb33',
  '82f48df83eec5bacbe2c38f13ce7e2219e5e318f4b2974d928d0ea7f7cec65fd' );
#
# 4) Test pseudocolor image
#
++$test;
print( "8-bit indexed-color PNG ...\n" );
testReadWrite( 'input_256.png',
  'output_256.png',
  q/quality=>54/,
  '066c0047c6e7e3f4cda1c86224441bfd5f522b5805b2a9190dcfa5294d94e4bd' );
#
# 5) Test truecolor image
#
++$test;
print( "24-bit Truecolor PNG ...\n" );
testReadWrite( 'input_truecolor.png',
  'output_truecolor.png',
  q/quality=>55/,
  '55913611798c087b9300b14d3baeda08a142910ad120379a9308a6b8c8b2f6e8' );
#
# 6) Test Multiple-image Network Graphics
#
++$test;
print( "MNG with 24-bit Truecolor PNGs ...\n" );
testReadWrite( 'input.mng',
  'output.mng',
  q/quality=>55/,
  '40805ef3db6e3a94c85e30e591e5881dc660ff863591d6f56605dba64d03d83d' );
