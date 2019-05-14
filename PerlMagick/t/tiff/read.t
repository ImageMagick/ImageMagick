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
	'24fc9ae8a8c00a01ba44a4f902230ca1a5841e283a5ec35e81815c194a344954' );

#
# 6) Test reading PseudoColor using contiguous planar packing
#
++$test;
print("PseudoColor (256 color) contiguous planes ...\n");
testRead( 'input_256_planar_contig.tiff',
  'b2644ac928730aa1d28e754aeb17b4731b57daea28c9fb89b1b50623e87215b5' );

#
# 7) Test reading PseudoColor using seperate planes
#
++$test;
print("PseudoColor (256 color) seperate planes ...\n");
testRead( 'input_256_planar_separate.tiff',
  'b2644ac928730aa1d28e754aeb17b4731b57daea28c9fb89b1b50623e87215b5' );

#
# 8) Test Reading TrueColor (8-bit)
# 
++$test;
print("TrueColor (8-bit) image ...\n");
testRead( 'input_truecolor.tiff',
  'ab90f892242d254e4c50dee17a7c8981bc7d46c9534bbb838cf5653c287886c8' );

#
# 9) Test Reading TrueColor (16-bit)
#
++$test;
print("TrueColor (16-bit) image ...\n");
testRead( 'input_truecolor_16.tiff',
  '562a32f51f620139402c1cc2336fb03f655da47dedf9fdbccfd4d23df55dd3b6' );

#
# 10) Test Reading 8-bit TrueColor Tiled (32x32 tiles)
# 
++$test;
print("TrueColor (8-bit) tiled image, 32x32 tiles ...\n");
testRead( 'input_truecolor_tiled32x32.tiff',
  'ab90f892242d254e4c50dee17a7c8981bc7d46c9534bbb838cf5653c287886c8' );

#
# 11) Test Reading 8-bit TrueColor Tiled (8 rows per strip)
# 
++$test;
print("TrueColor (8-bit) stripped, image, 8 rows per strip ...\n");
testRead( 'input_truecolor_stripped.tiff',
  'ab90f892242d254e4c50dee17a7c8981bc7d46c9534bbb838cf5653c287886c8' );

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
  'cdeea215166c095ef42557d63da3d037d16f4e7884e94b7db21e18e241d84f86');

#
# 14) Test Reading Grayscale 8-bit + matte
# 
++$test;
print("Grayscale (8-bit + matte) ...\n");
testRead( 'input_gray_8bit_matte.tiff',
  'fd4bf0cae6a978c301452178ae645a08cbd115659296a3fcfd5e07421bbaeb19' );

#
# 15) Test Reading Grayscale 12-bit
# 
++$test;
print("Grayscale (12-bit) ...\n");
testRead( 'input_gray_12bit.tiff',
  '638d5287bb0e6b585525334332ac348ab54903ad0104b789f9335413a8c59276');

#
# 16) Test Reading Grayscale 16-bit
# 
++$test;
print("Grayscale (16-bit) ...\n");
testRead( 'input_gray_16bit.tiff',
  '0cde228a8b2385f005fce6e3f027195a140971d2db9d122e0530e9e1fa75ea81');
