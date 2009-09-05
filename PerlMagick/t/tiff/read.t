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
  '83175f7bcc43fb71212dee254c85e355c18bcd25f35d3b9caba66fff7341fa64' );

#
# 2) Test reading PseudoColor (16 color)
#
++$test;
print("PseudoColor (16 color)...\n");
testRead( 'input_16.tiff',
  'c33901f8a62814e6c2c1ecca0c7d1c95b3ecb089f4815ab630652e25cae85b44' );

#
# 3) Test reading PseudoColor (16 color + matte channel)
#
++$test;
print("PseudoColor (16 color + matte channel)...\n");
testRead( 'input_16_matte.tiff',
  'c33901f8a62814e6c2c1ecca0c7d1c95b3ecb089f4815ab630652e25cae85b44' );

#
# 4) Test reading PseudoColor (256 color)
#
++$test;
print("PseudoColor (256 color) ...\n");
testRead( 'input_256.tiff',
  '08fdfd88b1eb09649ef126c1fe5a8c5b958eb941653daa0b3615f1b9db9966df' );

#
# 5) Test reading PseudoColor (256 color + matte channel)
#
++$test;
print("PseudoColor (256 color + matte channel) ...\n");
testRead( 'input_256_matte.tiff',
	'f28f9d3620babcaf84c61ffbf3f92e83fcc0bc3d5904ac7b8a1318e8d796859f' );

#
# 6) Test reading PseudoColor using contiguous planar packing
#
++$test;
print("PseudoColor (256 color) contiguous planes ...\n");
testRead( 'input_256_planar_contig.tiff',
  '08fdfd88b1eb09649ef126c1fe5a8c5b958eb941653daa0b3615f1b9db9966df' );

#
# 7) Test reading PseudoColor using seperate planes
#
++$test;
print("PseudoColor (256 color) seperate planes ...\n");
testRead( 'input_256_planar_separate.tiff',
  '08fdfd88b1eb09649ef126c1fe5a8c5b958eb941653daa0b3615f1b9db9966df' );

#
# 8) Test Reading TrueColor (8-bit)
# 
++$test;
print("TrueColor (8-bit) image ...\n");
testRead( 'input_truecolor.tiff',
  '4002f066656ca5cdb12afa067769bfa432b1d45d0278d1c558cf4a64638eaa6e' );

#
# 9) Test Reading TrueColor (16-bit)
#
++$test;
print("TrueColor (16-bit) image ...\n");
testRead( 'input_truecolor_16.tiff',
  '0adb551c0d521ce9e502e7242040463543f1c84b55c6349d3aa4cbd093b1a410',
  '725e5f79cd2dea9fd6000df596a1e63790e8ecf48a4a7c303324c176e88c5757' );

#
# 10) Test Reading 8-bit TrueColor Tiled (32x32 tiles)
# 
++$test;
print("TrueColor (8-bit) tiled image, 32x32 tiles ...\n");
testRead( 'input_truecolor_tiled32x32.tiff',
  '4002f066656ca5cdb12afa067769bfa432b1d45d0278d1c558cf4a64638eaa6e' );

#
# 11) Test Reading 8-bit TrueColor Tiled (8 rows per strip)
# 
++$test;
print("TrueColor (8-bit) stripped, image, 8 rows per strip ...\n");
testRead( 'input_truecolor_stripped.tiff',
  '4002f066656ca5cdb12afa067769bfa432b1d45d0278d1c558cf4a64638eaa6e' );

#
# 12) Test Reading Grayscale 4-bit
#
++$test;
print("Grayscale (4-bit) ...\n");
testRead( 'input_gray_4bit.tiff',
  'aff256464aeb39a8fd5498d7e296362a11b827f6700b7ad1342b8be8a6304303');

#
# 13) Test Reading Grayscale 8-bit
# 
++$test;
print("Grayscale (8-bit) ...\n");
testRead( 'input_gray_8bit.tiff',
  '46542d79a23def43c94f4b07452e2d8466abd73a949569596d23bb7130b850f5');

#
# 14) Test Reading Grayscale 8-bit + matte
# 
++$test;
print("Grayscale (8-bit + matte) ...\n");
testRead( 'input_gray_8bit_matte.tiff',
  '49929da2adbe49c525a7e7f2210187ad496d14d0268c80cddcd201e389fe8171' );

#
# 15) Test Reading Grayscale 12-bit
# 
++$test;
print("Grayscale (12-bit) ...\n");
testRead( 'input_gray_12bit.tiff',
  'c29789db13969ddbfc9b588066d6578d87628566a60ffc33dbd43e6c4f747f51',
  '0f2c8dfde42ee59deddd126853a8d6f69f6d517cebd8fab28c35836d21227064',
  '0f2c8dfde42ee59deddd126853a8d6f69f6d517cebd8fab28c35836d21227064');

#
# 16) Test Reading Grayscale 16-bit
# 
++$test;
print("Grayscale (16-bit) ...\n");
testRead( 'input_gray_16bit.tiff',
  '7cc1f9e909cd671d0a4d32018fa885997a43de202eafdf4e0bec3dbff9f24a4e',
  '5ebfb789a15df77bdd86342617710eb2f685a0ab32886aa4818719c9a84683ed',
  '5ebfb789a15df77bdd86342617710eb2f685a0ab32886aa4818719c9a84683ed');
