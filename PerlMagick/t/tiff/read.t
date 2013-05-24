#!/usr/bin/perl
#
# Test reading TIFF images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..16\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/tiff' || die 'Cd failed';

#
# 1) Test Reading Monochrome
# 
print("Monochrome ...\n");
testRead ( 'input_mono.tiff',
  'dedb5873a990158f0e5abdebda8c8dfb32de0be16b2b191fcb476b754e000a7b' );

#
# 2) Test reading PseudoColor (16 color)
#
++$test;
print("PseudoColor (16 color)...\n");
testRead( 'input_16.tiff',
  '58a82998d620aba54b86ab8dca7cbfeb726f3e86842369d5a0292a8522e95dab' );

#
# 3) Test reading PseudoColor (16 color + matte channel)
#
++$test;
print("PseudoColor (16 color + matte channel)...\n");
testRead( 'input_16_matte.tiff',
  '58a82998d620aba54b86ab8dca7cbfeb726f3e86842369d5a0292a8522e95dab' );

#
# 4) Test reading PseudoColor (256 color)
#
++$test;
print("PseudoColor (256 color) ...\n");
testRead( 'input_256.tiff',
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '59c97ab49c16b8664f1362242548399ad9e902b96959db98540ec820484380b1' );

#
# 5) Test reading PseudoColor (256 color + matte channel)
#
++$test;
print("PseudoColor (256 color + matte channel) ...\n");
testRead( 'input_256_matte.tiff',
        '824af58cdd8a8accffee3dab1ed9d28b34a8b183d3e5f5f13caeaab03bcadd13',
	'64b8429356cf9ea2b717faaa28a85b0f7ca174ea1a72063c1d4b2270084e4881' );

#
# 6) Test reading PseudoColor using contiguous planar packing
#
++$test;
print("PseudoColor (256 color) contiguous planes ...\n");
testRead( 'input_256_planar_contig.tiff',
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '59c97ab49c16b8664f1362242548399ad9e902b96959db98540ec820484380b1' );

#
# 7) Test reading PseudoColor using seperate planes
#
++$test;
print("PseudoColor (256 color) seperate planes ...\n");
testRead( 'input_256_planar_separate.tiff',
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '59c97ab49c16b8664f1362242548399ad9e902b96959db98540ec820484380b1' );

#
# 8) Test Reading TrueColor (8-bit)
# 
++$test;
print("TrueColor (8-bit) image ...\n");
testRead( 'input_truecolor.tiff',
  '2c5f5bcc5168543b807bf50476e472cd38e8d1a7d2c68df91e25ae7cd001166a' );

#
# 9) Test Reading TrueColor (16-bit)
#
++$test;
print("TrueColor (16-bit) image ...\n");
testRead( 'input_truecolor_16.tiff',
  '9897466dce6a47db3530821056c0a1c6e20f20d5bbfce837addfbede63bdecab',
  '768d8c7d0a52108f1f8dc12fb10412f42bc18f07b0a537dd77c7774bec04a273' );

#
# 10) Test Reading 8-bit TrueColor Tiled (32x32 tiles)
# 
++$test;
print("TrueColor (8-bit) tiled image, 32x32 tiles ...\n");
testRead( 'input_truecolor_tiled32x32.tiff',
  '2c5f5bcc5168543b807bf50476e472cd38e8d1a7d2c68df91e25ae7cd001166a' );

#
# 11) Test Reading 8-bit TrueColor Tiled (8 rows per strip)
# 
++$test;
print("TrueColor (8-bit) stripped, image, 8 rows per strip ...\n");
testRead( 'input_truecolor_stripped.tiff',
  '2c5f5bcc5168543b807bf50476e472cd38e8d1a7d2c68df91e25ae7cd001166a' );

#
# 12) Test Reading Grayscale 4-bit
#
++$test;
print("Grayscale (4-bit) ...\n");
testRead( 'input_gray_4bit.tiff',
  '3d58e49ad202f2b171214f7a0e8ebdc2ac2c7e45b68a2249502f9339ca7efc6e');

#
# 13) Test Reading Grayscale 8-bit
# 
++$test;
print("Grayscale (8-bit) ...\n");
testRead( 'input_gray_8bit.tiff',
  '76f4dd783661899ede132a87a7c68132462d0a60efe92906388c3aca1fb76130');

#
# 14) Test Reading Grayscale 8-bit + matte
# 
++$test;
print("Grayscale (8-bit + matte) ...\n");
testRead( 'input_gray_8bit_matte.tiff',
  '30206f5082b53a8f81d1b0e5dfec94b3513b15ee3fe87fb646a7fd5bf9c94c04' );

#
# 15) Test Reading Grayscale 12-bit
# 
++$test;
print("Grayscale (12-bit) ...\n");
testRead( 'input_gray_12bit.tiff',
  '8784d89a246384f42210e980cfccb4e6c98de9dade262984bf756e16232e6c83');

#
# 16) Test Reading Grayscale 16-bit
# 
++$test;
print("Grayscale (16-bit) ...\n");
testRead( 'input_gray_16bit.tiff',
  'f056659e30e514325b8843d88f7bfa7a59c8b0496134ad0e66ea46eeece068d6',
  'c8428037f92e6ef6c9fca343f3b6206dd9404304b310d20782346db874292e1f',
  'c8428037f92e6ef6c9fca343f3b6206dd9404304b310d20782346db874292e1f');
