#!/usr/bin/perl
#  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.imagemagick.org/script/license.php
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
  'e7d406ec41fe69ba2bd88dd59e5eb17a83f17c0a99519def02c020041144f5b3');

print("Microsoft Windows bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp',
  'BMP:output.bmp',
  q//,
  '4db1c9f8cf10c1a9a7e80397b4cf060d2d31caae13ba712712e6341fb96bd6b0',
  'ae2007d0e05933a72294a6f8c7b59fc54d5fa3039f0f13bf8d65d05044ef2f39');

print("Microsoft Windows 24-bit bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp24',
  'BMP:output.bmp24',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("ZSoft IBM PC multi-page Paintbrush file ...\n");
++$test;
testReadWrite( 'DCX:input.dcx',
  'DCX:output.dcx',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Microsoft Windows 3.X DIB file ...\n");
++$test;
testReadWrite( 'DIB:input.dib',
  'DIB:output.dib',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Flexible Image Transport System ...\n");
++$test;
testReadWrite( 'FITS:input.fits',
  'FITS:output.fits',
  q//,
  '04b3cafea6030665fbedcc3463711475bcda2ad35254e5a632d8772905f59ab9' );

print("CompuServe graphics interchange format ...\n");
++$test;
testReadWrite( 'GIF:input.gif',
  'GIF:output.gif',
  q//,
  '4db1c9f8cf10c1a9a7e80397b4cf060d2d31caae13ba712712e6341fb96bd6b0',
  'ae2007d0e05933a72294a6f8c7b59fc54d5fa3039f0f13bf8d65d05044ef2f39');

print("CompuServe graphics interchange format (1987) ...\n");
++$test;
testReadWrite( 'GIF87:input.gif87',
  'GIF87:output.gif87',
  q//,
  'a06fe5ec382d10ef6dce8d2bd729c4a57c66d82ed695e2786f1d1f280aaa17fb',
  'a037bce490abf358b3c8e1f471a83d15434da26d6c91aa6d761407c0df573f1d');

print("Magick image file format ...\n");
++$test;
testReadWrite( 'MIFF:input.miff',
  'MIFF:output.miff',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("MTV Raytracing image format ...\n");
++$test;
testReadWrite( 'MTV:input.mtv',
  'MTV:output.mtv',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Portable bitmap format (black and white), ASCII format ...\n");
++$test;
testReadWrite( 'PBM:input_p1.pbm',
  'PBM:output_p1.pbm',
  q/compression=>'None'/,
  'c8c4f812d902693d1de6c74a6cffaaef7506bd868df65cae63b06707f2c9f3ac');

print("Portable bitmap format (black and white), binary format ...\n");
++$test;
testReadWrite( 'PBM:input_p4.pbm',
  'PBM:output_p4.pbm',
  q//,
  'c8c4f812d902693d1de6c74a6cffaaef7506bd868df65cae63b06707f2c9f3ac');

print("ZSoft IBM PC Paintbrush file ...\n");
++$test;
testReadWrite( 'PCX:input.pcx',
  'PCX:output.pcx',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Portable graymap format (gray scale), ASCII format ...\n");
++$test;
testReadWrite( 'PGM:input_p2.pgm',
  'PGM:output_p2.pgm',
  q/compression=>'None'/,
  'a52f1015dcd3290c136b892874b0ef06516d28a846a2a7383d896267a5299aba');

print("Apple Macintosh QuickDraw/PICT file ...\n");
++$test;
testReadWrite( 'PICT:input.pict',
  'PICT:output.pict',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Portable pixmap format (color), ASCII format ...\n");
++$test;
testReadWrite( 'PPM:input_p3.ppm',
  'PPM:output_p3.ppm',
  q/compression=>'None'/,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Portable graymap format (gray scale), binary format ...\n");
++$test;
testReadWrite( 'PGM:input_p5.pgm',
  'PGM:output_p5.pgm',
  q//,
  'a52f1015dcd3290c136b892874b0ef06516d28a846a2a7383d896267a5299aba');

print("Portable pixmap format (color), binary format ...\n");
++$test;
testReadWrite( 'PPM:input_p6.ppm',
  'PPM:output_p6.ppm',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Adobe Photoshop bitmap file ...\n");
++$test;
testReadWrite( 'PSD:input.psd',
  'PSD:output.psd',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33' );

print("Irix RGB image file ...\n");
++$test;
testReadWrite( 'SGI:input.sgi',
  'SGI:output.sgi',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("SUN 1-bit Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im1',
  'SUN:output.im1',
  q//,
  'cf6e645339d1bb82131ad658f2e7521f1e8aac69eb6e7add728f6157489972cd');

print("SUN 8-bit Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im8',
  'SUN:output.im8',
  q//,
  'a4c13fd97d6b9b32c016793d6ae2b01cee048b5f2790de8daaacccdf1c4b6956',
  '6b894bae0411f6f21e76724047cc93e28a2b04176df31d0632b656e98ce9b6e8');

print("SUN True-Color Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im24',
  'SUN:output.im24',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Truevision Targa image file ...\n");
++$test;
testReadWrite( 'TGA:input.tga',
  'TGA:output.tga',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Khoros Visualization image file ...\n");
++$test;
testReadWrite( 'VIFF:input.viff',
  'VIFF:output.viff',
  q//,
  'c211b4dd4e0c3b1d42a96682b0d290ae6bed5652670abe3c7fcca54503611b3c',
  'bbeaa063bd13c74414a9962f21a795844661b596bd89f753f064ba1db2624d03');

print("WBMP (Wireless Bitmap (level 0) image) ...\n");
++$test;
testReadWrite( 'WBMP:input.wbmp',
  'WBMP:output.wbmp',
  q//,
  '6d1374fa22aa5cb8fa9e98b928b8dea6e5fc6606ed2dc07c5f27d1e03e494d0e');

print("X Windows system bitmap (black and white only) ...\n");
++$test;
testReadWrite( 'XBM:input.xbm',
  'XBM:output.xbm',
  q//,
  'cf6e645339d1bb82131ad658f2e7521f1e8aac69eb6e7add728f6157489972cd');

print("X Windows system pixmap file (color) ...\n");
++$test;
testReadWrite( 'XPM:input.xpm',
  'XPM:output.xpm',
  q//,
  'a4c13fd97d6b9b32c016793d6ae2b01cee048b5f2790de8daaacccdf1c4b6956',
  '6b894bae0411f6f21e76724047cc93e28a2b04176df31d0632b656e98ce9b6e8');

print("CMYK format ...\n");
++$test;
testReadWriteSized( 'CMYK:input_70x46.cmyk',
  'CMYK:output_70x46.cmyk',
  '70x46',
  8,
  q//,
  '7e704fc1a99118630a92374ba27adf5baf69f30019016be2ed70eac79629e8b4');

print("GRAY format ...\n");
++$test;
testReadWriteSized( 'GRAY:input_70x46.gray',
  'GRAY:output_70x46.gray',
  '70x46',
  8,
  q//,
  '04b3cafea6030665fbedcc3463711475bcda2ad35254e5a632d8772905f59ab9' );

print("RGB format ...\n");
++$test;
testReadWriteSized( 'RGB:input_70x46.rgb',
  'RGB:output_70x46.rgb',
  '70x46',
  8,
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33' );


print("RGBA format ...\n");
++$test;
testReadWriteSized( 'RGBA:input_70x46.rgba',
  'RGBA:output_70x46.rgba',
  '70x46',
  8,
  q//,
  'e7d406ec41fe69ba2bd88dd59e5eb17a83f17c0a99519def02c020041144f5b3' );

1;
