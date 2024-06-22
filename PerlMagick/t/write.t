#!/usr/bin/perl
#  Copyright 1999 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
# Test writing formats supported directly by ImageMagick
#

BEGIN { $| = 1; $test=1; print "1..32\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';

print("AVS X image file ...\n");
testReadWrite( 'AVS:input.avs',
  'AVS:output.avs',
  q//,
  '74136c90d3e699ea5bcbf4aa733aff0dc822b6af72fce00f0c7647bcb0d49f66');

print("Microsoft Windows bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp',
  'BMP:output.bmp',
  q//,
  'd7324c919f04f4c118da68061a5dbb3f07ebab76b471ecfb0ac822453f677983');

print("Microsoft Windows 24-bit bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp24',
  'BMP:output.bmp24',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("ZSoft IBM PC multi-page Paintbrush file ...\n");
++$test;
testReadWrite( 'DCX:input.dcx',
  'DCX:output.dcx',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Microsoft Windows 3.X DIB file ...\n");
++$test;
testReadWrite( 'DIB:input.dib',
  'DIB:output.dib',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Flexible Image Transport System ...\n");
++$test;
testReadWrite( 'FITS:input.fits',
  'FITS:output.fits',
  q//,
  '1c773aeac90d47c684c5170fcee16e0c8d4b399f76809c97bcd92ea7e47b1ab4' );

print("CompuServe graphics interchange format ...\n");
++$test;
testReadWrite( 'GIF:input.gif',
  'GIF:output.gif',
  q//,
  'd7324c919f04f4c118da68061a5dbb3f07ebab76b471ecfb0ac822453f677983');

print("CompuServe graphics interchange format (1987) ...\n");
++$test;
testReadWrite( 'GIF87:input.gif87',
  'GIF87:output.gif87',
  q//,
  '153b1c806e673a635edc645a92c60d565b58a2aec2417cee1f2e507d8ede27e4');

print("Magick image file format ...\n");
++$test;
testReadWrite( 'MIFF:input.miff',
  'MIFF:output.miff',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("MTV Raytracing image format ...\n");
++$test;
testReadWrite( 'MTV:input.mtv',
  'MTV:output.mtv',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Portable bitmap format (black and white), ASCII format ...\n");
++$test;
testReadWrite( 'PBM:input_p1.pbm',
  'PBM:output_p1.pbm',
  q/compression=>'None'/,
  '71e1a6be223e307b1dbf732860792b15adba662b7a7ef284daf7f982f874ccf1');

print("Portable bitmap format (black and white), binary format ...\n");
++$test;
testReadWrite( 'PBM:input_p4.pbm',
  'PBM:output_p4.pbm',
  q//,
  '71e1a6be223e307b1dbf732860792b15adba662b7a7ef284daf7f982f874ccf1');

print("ZSoft IBM PC Paintbrush file ...\n");
++$test;
testReadWrite( 'PCX:input.pcx',
  'PCX:output.pcx',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Portable graymap format (gray scale), ASCII format ...\n");
++$test;
testReadWrite( 'PGM:input_p2.pgm',
  'PGM:output_p2.pgm',
  q/compression=>'None'/,
  'f345fd06540c055028fd51b1d97a2144065dda8036ff23234313ed66f0b87254');

print("Apple Macintosh QuickDraw/PICT file ...\n");
++$test;
testReadWrite( 'PICT:input.pict',
  'PICT:output.pict',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Portable pixmap format (color), ASCII format ...\n");
++$test;
testReadWrite( 'PPM:input_p3.ppm',
  'PPM:output_p3.ppm',
  q/compression=>'None'/,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Portable graymap format (gray scale), binary format ...\n");
++$test;
testReadWrite( 'PGM:input_p5.pgm',
  'PGM:output_p5.pgm',
  q//,
  'f345fd06540c055028fd51b1d97a2144065dda8036ff23234313ed66f0b87254');

print("Portable pixmap format (color), binary format ...\n");
++$test;
testReadWrite( 'PPM:input_p6.ppm',
  'PPM:output_p6.ppm',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Adobe Photoshop bitmap file ...\n");
++$test;
testReadWrite( 'PSD:input.psd',
  'PSD:output.psd',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52' );

print("Irix RGB image file ...\n");
++$test;
testReadWrite( 'SGI:input.sgi',
  'SGI:output.sgi',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("SUN 1-bit Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im1',
  'SUN:output.im1',
  q//,
  '49d4c40abae73a1d6169dc1f0262e89ad5dc8a9f64e7feef3430090768e629c4');

print("SUN 8-bit Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im8',
  'SUN:output.im8',
  q//,
  '8ac3392ac643d8a852a4ac23dbf25f2124cb13627dbc60bf887b76ecb89cbb20');

print("SUN True-Color Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im24',
  'SUN:output.im24',
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52');

print("Truevision Targa image file ...\n");
++$test;
testReadWrite( 'TGA:input.tga',
  'TGA:output.tga',
  q//,
  'ff78a650ff5f4adfdf6ef34e8cc3369de44e71ed4eef1807dc88372352ddac90');

print("Khoros Visualization image file ...\n");
++$test;
testReadWrite( 'VIFF:input.viff',
  'VIFF:output.viff',
  q//,
  'b9ff3e1dbb1a4cd376e95645c0f0f950e3ae73973780bb1dfbc849b211fc3925');

print("WBMP (Wireless Bitmap (level 0) image) ...\n");
++$test;
testReadWrite( 'WBMP:input.wbmp',
  'WBMP:output.wbmp',
  q//,
  '8833a92cbe11a3b925a1b7edffd6508d7b12dd50e3f4907ca8d77917f6e4e697');

print("X Windows system bitmap (black and white only) ...\n");
++$test;
testReadWrite( 'XBM:input.xbm',
  'XBM:output.xbm',
  q//,
  '49d4c40abae73a1d6169dc1f0262e89ad5dc8a9f64e7feef3430090768e629c4');

print("X Windows system pixmap file (color) ...\n");
++$test;
testReadWrite( 'XPM:input.xpm',
  'XPM:output.xpm',
  q//,
  '8ac3392ac643d8a852a4ac23dbf25f2124cb13627dbc60bf887b76ecb89cbb20');

print("CMYK format ...\n");
++$test;
testReadWriteSized( 'CMYK:input_70x46.cmyk',
  'CMYK:output_70x46.cmyk',
  '70x46',
  8,
  q//,
  '215166c965254211b75dcaadbb587b4c2947d7cb3de1420b13b6539cd815a90d');

print("GRAY format ...\n");
++$test;
testReadWriteSized( 'GRAY:input_70x46.gray',
  'GRAY:output_70x46.gray',
  '70x46',
  8,
  q//,
  '2f3d94bebb0feec1a2f0dcc295cbcf074ceb58e7e59262c3d23f0f26fd5e6267' );

print("RGB format ...\n");
++$test;
testReadWriteSized( 'RGB:input_70x46.rgb',
  'RGB:output_70x46.rgb',
  '70x46',
  8,
  q//,
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52' );

print("RGBA format ...\n");
++$test;
testReadWriteSized( 'RGBA:input_70x46.rgba',
  'RGBA:output_70x46.rgba',
  '70x46',
  8,
  q//,
  '74136c90d3e699ea5bcbf4aa733aff0dc822b6af72fce00f0c7647bcb0d49f66' );

1;
