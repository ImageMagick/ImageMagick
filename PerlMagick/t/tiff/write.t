#!/usr/bin/perl
#
# Test writing TIFF images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..10\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/tiff' || die 'Cd failed';

#
# 1) Test 4-bit pseudocolor image
#
print("PseudoColor image (4 bits/sample) ...\n");
testReadWrite( 'input_16.tiff',
  'output_16.tiff',
  q//,
  '58a82998d620aba54b86ab8dca7cbfeb726f3e86842369d5a0292a8522e95dab');

#
# 2) Test 8-bit pseudocolor image
#
++$test;
print("PseudoColor image (8 bits/sample) ...\n");
testReadWrite( 'input_256.tiff',
  'output_256.tiff',
  q//,
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '59c97ab49c16b8664f1362242548399ad9e902b96959db98540ec820484380b1');

#
# 3) Test 4-bit pseudocolor + matte channel image
#
++$test;
print("PseudoColor image (4 bits/sample + matte channel) ...\n");
testReadWrite( 'input_16_matte.tiff',
  'output_16_matte.tiff',
  q//,
  '58a82998d620aba54b86ab8dca7cbfeb726f3e86842369d5a0292a8522e95dab' );

#
# 4) Test 8-bit pseudocolor + matte channel image
#
++$test;
print("PseudoColor image (8 bits/sample + matte channel) ...\n");
testReadWrite( 'input_256_matte.tiff',
  'output_256_matte.tiff',
  q//,
  '824af58cdd8a8accffee3dab1ed9d28b34a8b183d3e5f5f13caeaab03bcadd13',
  '64b8429356cf9ea2b717faaa28a85b0f7ca174ea1a72063c1d4b2270084e4881' );

#
# 5) Test truecolor image
#
++$test;
print("TrueColor image (8 bits/sample) ...\n");
testReadWrite( 'input_truecolor.tiff',
  'output_truecolor.tiff',
  q/quality=>55/,
  '2c5f5bcc5168543b807bf50476e472cd38e8d1a7d2c68df91e25ae7cd001166a' );

#
# 6) Test monochrome image
#
++$test;
print("Gray image (1 bit per sample) ...\n");
testReadWrite(  'input_mono.tiff',
  'output_mono.tiff',
  q//,
  'dedb5873a990158f0e5abdebda8c8dfb32de0be16b2b191fcb476b754e000a7b' );

#
# 7) Test gray 4 bit image
#
++$test;
print("Gray image (4 bits per sample) ...\n");
testReadWrite(  'input_gray_4bit.tiff',
  'output_gray_4bit.tiff',
  q//,
  '3d58e49ad202f2b171214f7a0e8ebdc2ac2c7e45b68a2249502f9339ca7efc6e' );

#
# 8) Test gray 8 bit image
#
++$test;
print("Gray image (8 bits per sample) ...\n");
testReadWrite(  'input_gray_8bit.tiff',
  'output_gray_8bit.tiff',
  q//,
  '76f4dd783661899ede132a87a7c68132462d0a60efe92906388c3aca1fb76130' );

#
# 9) Test gray 4 bit image (with matte channel)
#
++$test;
print("Gray image (4 bits per sample + matte channel) ...\n");
testReadWrite(  'input_gray_4bit_matte.tiff',
  'output_gray_4bit_matte.tiff',
  q//,
  '817ebd9cb521eca754aa3add8100be1b7865dc54510b830d67a57c254832d3d5' );

#
# 10) Test gray 8 bit image (with matte channel)
#
++$test;
print("Gray image (8 bits per sample + matte channel) ...\n");
testReadWrite(  'input_gray_8bit_matte.tiff',
  'output_gray_8bit_matte.tiff',
  q//,
  '30206f5082b53a8f81d1b0e5dfec94b3513b15ee3fe87fb646a7fd5bf9c94c04' );
