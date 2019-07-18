#!/usr/bin/perl
#  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
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
  'cc6cd392e791804b817e825d48a37bc811f880f29e6d28e0ace3815eec687091');

print("Microsoft Windows bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp',
  'BMP:output.bmp',
  q//,
  'a8e92cecc7ca058cb432a030f070793cb5be8cd09e74d07f963c81cc472af910');

print("Microsoft Windows 24-bit bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp24',
  'BMP:output.bmp24',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("ZSoft IBM PC multi-page Paintbrush file ...\n");
++$test;
testReadWrite( 'DCX:input.dcx',
  'DCX:output.dcx',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Microsoft Windows 3.X DIB file ...\n");
++$test;
testReadWrite( 'DIB:input.dib',
  'DIB:output.dib',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Flexible Image Transport System ...\n");
++$test;
testReadWrite( 'FITS:input.fits',
  'FITS:output.fits',
  q//,
  '71f12148af77bf65bc7e0ff5fbe82b5d38416cd99c11470074436cfe42448b41' );

print("CompuServe graphics interchange format ...\n");
++$test;
testReadWrite( 'GIF:input.gif',
  'GIF:output.gif',
  q//,
  'a8e92cecc7ca058cb432a030f070793cb5be8cd09e74d07f963c81cc472af910');

print("CompuServe graphics interchange format (1987) ...\n");
++$test;
testReadWrite( 'GIF87:input.gif87',
  'GIF87:output.gif87',
  q//,
  '6ca156130053dcfd5515c531a2d43b5d166bb65ec9988bfbfd359f25b0a13f7e');

print("Magick image file format ...\n");
++$test;
testReadWrite( 'MIFF:input.miff',
  'MIFF:output.miff',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("MTV Raytracing image format ...\n");
++$test;
testReadWrite( 'MTV:input.mtv',
  'MTV:output.mtv',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

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
  '88ad5b37729ccd4fc292fbaf32776e8009f72abc6136827383231af3676ba956');

print("ZSoft IBM PC Paintbrush file ...\n");
++$test;
testReadWrite( 'PCX:input.pcx',
  'PCX:output.pcx',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Portable graymap format (gray scale), ASCII format ...\n");
++$test;
testReadWrite( 'PGM:input_p2.pgm',
  'PGM:output_p2.pgm',
  q/compression=>'None'/,
  '4385fd336eb1883fa41ad9b7f44a31c1a2923ae47b3d9650d395b9229b5e9198');

print("Apple Macintosh QuickDraw/PICT file ...\n");
++$test;
testReadWrite( 'PICT:input.pict',
  'PICT:output.pict',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Portable pixmap format (color), ASCII format ...\n");
++$test;
testReadWrite( 'PPM:input_p3.ppm',
  'PPM:output_p3.ppm',
  q/compression=>'None'/,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Portable graymap format (gray scale), binary format ...\n");
++$test;
testReadWrite( 'PGM:input_p5.pgm',
  'PGM:output_p5.pgm',
  q//,
  '4385fd336eb1883fa41ad9b7f44a31c1a2923ae47b3d9650d395b9229b5e9198');

print("Portable pixmap format (color), binary format ...\n");
++$test;
testReadWrite( 'PPM:input_p6.ppm',
  'PPM:output_p6.ppm',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Adobe Photoshop bitmap file ...\n");
++$test;
testReadWrite( 'PSD:input.psd',
  'PSD:output.psd',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e' );

print("Irix RGB image file ...\n");
++$test;
testReadWrite( 'SGI:input.sgi',
  'SGI:output.sgi',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

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
  '0249e226dd79884d3552aa50951c2689859e4c0c5f708da05fff9c6c93e66cb2');

print("SUN True-Color Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im24',
  'SUN:output.im24',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Truevision Targa image file ...\n");
++$test;
testReadWrite( 'TGA:input.tga',
  'TGA:output.tga',
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e');

print("Khoros Visualization image file ...\n");
++$test;
testReadWrite( 'VIFF:input.viff',
  'VIFF:output.viff',
  q//,
  'cc0595eb4b8096cf76f84ff0c03607f5112fbab9d4fb32ef9305f5ce4545d00a');

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
  '0249e226dd79884d3552aa50951c2689859e4c0c5f708da05fff9c6c93e66cb2');

print("CMYK format ...\n");
++$test;
testReadWriteSized( 'CMYK:input_70x46.cmyk',
  'CMYK:output_70x46.cmyk',
  '70x46',
  8,
  q//,
  '243b587fb295eb5c17b54a8b6768da139dd09b98bafcef569778ab941c6114ec');

print("GRAY format ...\n");
++$test;
testReadWriteSized( 'GRAY:input_70x46.gray',
  'GRAY:output_70x46.gray',
  '70x46',
  8,
  q//,
  'f03c7a969bf0d9f32821b0fa33895929b47f1994d6428822ae544d1a72c6a397' );

print("RGB format ...\n");
++$test;
testReadWriteSized( 'RGB:input_70x46.rgb',
  'RGB:output_70x46.rgb',
  '70x46',
  8,
  q//,
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e' );

print("RGBA format ...\n");
++$test;
testReadWriteSized( 'RGBA:input_70x46.rgba',
  'RGBA:output_70x46.rgba',
  '70x46',
  8,
  q//,
  'cc6cd392e791804b817e825d48a37bc811f880f29e6d28e0ace3815eec687091' );

1;
