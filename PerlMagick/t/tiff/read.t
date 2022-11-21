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
  '71e1a6be223e307b1dbf732860792b15adba662b7a7ef284daf7f982f874ccf1' );

#
# 2) Test reading PseudoColor (16 color)
#
++$test;
print("PseudoColor (16 color)...\n");
testRead( 'input_16.tiff',
  '0de2dcbf667c69ae6735d1a701b4038c1eeea25cc86981a496bb26fc82541835' );

#
# 3) Test reading PseudoColor (16 color + matte channel)
#
++$test;
print("PseudoColor (16 color + matte channel)...\n");
testRead( 'input_16_matte.tiff',
  '0de2dcbf667c69ae6735d1a701b4038c1eeea25cc86981a496bb26fc82541835' );

#
# 4) Test reading PseudoColor (256 color)
#
++$test;
print("PseudoColor (256 color) ...\n");
testRead( 'input_256.tiff',
  'b2644ac928730aa1d28e754aeb17b4731b57daea28c9fb89b1b50623e87215b5' );

#
# 5) Test reading PseudoColor (256 color + matte channel)
#
++$test;
print("PseudoColor (256 color + matte channel) ...\n");
testRead( 'input_256_matte.tiff',
	'c8e5089f89ed3b7d067222e187ccd95da0a586f3a7f669876188fe8bfa04e6d9' );

#
# 6) Test reading PseudoColor using contiguous planar packing
#
++$test;
print("PseudoColor (256 color) contiguous planes ...\n");
testRead( 'input_256_planar_contig.tiff',
  'b2644ac928730aa1d28e754aeb17b4731b57daea28c9fb89b1b50623e87215b5' );

#
# 7) Test reading PseudoColor using separate planes
#
++$test;
print("PseudoColor (256 color) separate planes ...\n");
testRead( 'input_256_planar_separate.tiff',
  'b2644ac928730aa1d28e754aeb17b4731b57daea28c9fb89b1b50623e87215b5' );

#
# 8) Test Reading TrueColor (8-bit)
# 
++$test;
print("TrueColor (8-bit) image ...\n");
testRead( 'input_truecolor.tiff',
  'f72b63be472e5e730ee2635463c6643d11057d251709ffe1f2027f69b57449df' );

#
# 9) Test Reading TrueColor (16-bit)
#
++$test;
print("TrueColor (16-bit) image ...\n");
testRead( 'input_truecolor_16.tiff',
  '7de73152fd38276a12bd4e137854b9dd27ae89dcd597e8789442e4d44df31e61',
  '81def436d1dea0ee118164ff4f017c62ad7a5a37bf97a820244a4e2c86c338ab' );

#
# 10) Test Reading 8-bit TrueColor Tiled (32x32 tiles)
# 
++$test;
print("TrueColor (8-bit) tiled image, 32x32 tiles ...\n");
testRead( 'input_truecolor_tiled32x32.tiff',
  'f72b63be472e5e730ee2635463c6643d11057d251709ffe1f2027f69b57449df' );

#
# 11) Test Reading 8-bit TrueColor Tiled (8 rows per strip)
# 
++$test;
print("TrueColor (8-bit) stripped, image, 8 rows per strip ...\n");
testRead( 'input_truecolor_stripped.tiff',
  'f72b63be472e5e730ee2635463c6643d11057d251709ffe1f2027f69b57449df' );

#
# 12) Test Reading Grayscale 4-bit
#
++$test;
print("Grayscale (4-bit) ...\n");
testRead( 'input_gray_4bit.tiff',
  'e55c01b0d28b0a19431ba27203db7cb6ada189c9519d4466c44a764aad5e185a');

#
# 13) Test Reading Grayscale 8-bit
# 
++$test;
print("Grayscale (8-bit) ...\n");
testRead( 'input_gray_8bit.tiff',
  'b51e862fcc24d439870da413c664dfefc36cea1260d807b3208d6f091566263c');

#
# 14) Test Reading Grayscale 8-bit + matte
# 
++$test;
print("Grayscale (8-bit + matte) ...\n");
testRead( 'input_gray_8bit_matte.tiff',
  '6002e57537cd54733551f8c4269e8104f2b14f8fcc58a07eda61f5911eb11c80' );

#
# 15) Test Reading Grayscale 12-bit
# 
++$test;
print("Grayscale (12-bit) ...\n");
testRead( 'input_gray_12bit.tiff',
  'f343adc420b5fc7353cddecf48e6836d8ab8a91a6c78e316e903aec2d3f7293a',
  '638d5287bb0e6b585525334332ac348ab54903ad0104b789f9335413a8c59276' );

#
# 16) Test Reading Grayscale 16-bit
# 
++$test;
print("Grayscale (16-bit) ...\n");
testRead( 'input_gray_16bit.tiff',
  '5d7d94a836efc6be6dc6a84be6017b19a0a5486cc9311b86462cd5e75abb9398',
  '9acab3f8b02e461149decd6dbb99d4b91be81a129e5f4cafc229e2f393173819' );
