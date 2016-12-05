#!/usr/bin/perl
#  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
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
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Microsoft Windows bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp',
  'BMP:output.bmp',
  q//,
  'e9b00f8a25976955cf8264391fc63f554396f4ac03d65cd0b1a2becbd667bc0b');

print("Microsoft Windows 24-bit bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp24',
  'BMP:output.bmp24',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("ZSoft IBM PC multi-page Paintbrush file ...\n");
++$test;
testReadWrite( 'DCX:input.dcx',
  'DCX:output.dcx',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Microsoft Windows 3.X DIB file ...\n");
++$test;
testReadWrite( 'DIB:input.dib',
  'DIB:output.dib',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Flexible Image Transport System ...\n");
++$test;
testReadWrite( 'FITS:input.fits',
  'FITS:output.fits',
  q//,
  '74f6153fb577d01d852458d6e43d8fb639e203f6c9a6e55acce2ca8a493dfe35' );

print("CompuServe graphics interchange format ...\n");
++$test;
testReadWrite( 'GIF:input.gif',
  'GIF:output.gif',
  q//,
  '4db1c9f8cf10c1a9a7e80397b4cf060d2d31caae13ba712712e6341fb96bd6b0',
  'e9b00f8a25976955cf8264391fc63f554396f4ac03d65cd0b1a2becbd667bc0b');

print("CompuServe graphics interchange format (1987) ...\n");
++$test;
testReadWrite( 'GIF87:input.gif87',
  'GIF87:output.gif87',
  q//,
  'a06fe5ec382d10ef6dce8d2bd729c4a57c66d82ed695e2786f1d1f280aaa17fb',
  '5c45e316eba35ac44cbe55c74b81259a1419f85264e5bb35b79db4a91bf0e3f1');

print("Magick image file format ...\n");
++$test;
testReadWrite( 'MIFF:input.miff',
  'MIFF:output.miff',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("MTV Raytracing image format ...\n");
++$test;
testReadWrite( 'MTV:input.mtv',
  'MTV:output.mtv',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Portable bitmap format (black and white), ASCII format ...\n");
++$test;
testReadWrite( 'PBM:input_p1.pbm',
  'PBM:output_p1.pbm',
  q/compression=>'None'/,
  'dedb5873a990158f0e5abdebda8c8dfb32de0be16b2b191fcb476b754e000a7b');

print("Portable bitmap format (black and white), binary format ...\n");
++$test;
testReadWrite( 'PBM:input_p4.pbm',
  'PBM:output_p4.pbm',
  q//,
  'dedb5873a990158f0e5abdebda8c8dfb32de0be16b2b191fcb476b754e000a7b');

print("ZSoft IBM PC Paintbrush file ...\n");
++$test;
testReadWrite( 'PCX:input.pcx',
  'PCX:output.pcx',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Portable graymap format (gray scale), ASCII format ...\n");
++$test;
testReadWrite( 'PGM:input_p2.pgm',
  'PGM:output_p2.pgm',
  q/compression=>'None'/,
  '63e162830260bb9892eb5a0e96301920e9cb4c6ed9016204dd58ededbb11923f');

print("Apple Macintosh QuickDraw/PICT file ...\n");
++$test;
testReadWrite( 'PICT:input.pict',
  'PICT:output.pict',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Portable pixmap format (color), ASCII format ...\n");
++$test;
testReadWrite( 'PPM:input_p3.ppm',
  'PPM:output_p3.ppm',
  q/compression=>'None'/,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Portable graymap format (gray scale), binary format ...\n");
++$test;
testReadWrite( 'PGM:input_p5.pgm',
  'PGM:output_p5.pgm',
  q//,
  '63e162830260bb9892eb5a0e96301920e9cb4c6ed9016204dd58ededbb11923f');

print("Portable pixmap format (color), binary format ...\n");
++$test;
testReadWrite( 'PPM:input_p6.ppm',
  'PPM:output_p6.ppm',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Adobe Photoshop bitmap file ...\n");
++$test;
testReadWrite( 'PSD:input.psd',
  'PSD:output.psd',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7' );

print("Irix RGB image file ...\n");
++$test;
testReadWrite( 'SGI:input.sgi',
  'SGI:output.sgi',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("SUN 1-bit Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im1',
  'SUN:output.im1',
  q//,
  '678af4d3e2f78a1ef30cb1df2bd6f00b347082f5b3560257aacd9ac40fb47d63');

print("SUN 8-bit Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im8',
  'SUN:output.im8',
  q//,
  'a4c13fd97d6b9b32c016793d6ae2b01cee048b5f2790de8daaacccdf1c4b6956',
  '8702000d509b897e48dc4834cf7fa1c2bf1a72ecc0d74a703e780f29b0835250');

print("SUN True-Color Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im24',
  'SUN:output.im24',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Truevision Targa image file ...\n");
++$test;
testReadWrite( 'TGA:input.tga',
  'TGA:output.tga',
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7');

print("Khoros Visualization image file ...\n");
++$test;
testReadWrite( 'VIFF:input.viff',
  'VIFF:output.viff',
  q//,
  'c211b4dd4e0c3b1d42a96682b0d290ae6bed5652670abe3c7fcca54503611b3c',
  '43783afb71012dcc4860d856a171246019f555d402dad04f61a27e8db09ef58b');

print("WBMP (Wireless Bitmap (level 0) image) ...\n");
++$test;
testReadWrite( 'WBMP:input.wbmp',
  'WBMP:output.wbmp',
  q//,
  '49279b862c8528fd47023b65361c6fc7081677311893d6c80ce577d463a948d8');

print("X Windows system bitmap (black and white only) ...\n");
++$test;
testReadWrite( 'XBM:input.xbm',
  'XBM:output.xbm',
  q//,
  '38e65935f34b9814ce2c4d839ccd0438a3d8c2cfed93e6e0eb881787c28a91a4');

print("X Windows system pixmap file (color) ...\n");
++$test;
testReadWrite( 'XPM:input.xpm',
  'XPM:output.xpm',
  q//,
  'a4c13fd97d6b9b32c016793d6ae2b01cee048b5f2790de8daaacccdf1c4b6956',
  '8702000d509b897e48dc4834cf7fa1c2bf1a72ecc0d74a703e780f29b0835250');

print("CMYK format ...\n");
++$test;
testReadWriteSized( 'CMYK:input_70x46.cmyk',
  'CMYK:output_70x46.cmyk',
  '70x46',
  8,
  q//,
  'f39e32b55a8ed4b2cc12c431cfe64fd0462f9aa0fb0122066f4010b562d5fe47');

print("GRAY format ...\n");
++$test;
testReadWriteSized( 'GRAY:input_70x46.gray',
  'GRAY:output_70x46.gray',
  '70x46',
  8,
  q//,
  '6e885bb6b3f0edd30266f9c59f453f93452dd551bf4b2618938a377b8c8d0b66' );

print("RGB format ...\n");
++$test;
testReadWriteSized( 'RGB:input_70x46.rgb',
  'RGB:output_70x46.rgb',
  '70x46',
  8,
  q//,
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7' );


print("RGBA format ...\n");
++$test;
testReadWriteSized( 'RGBA:input_70x46.rgba',
  'RGBA:output_70x46.rgba',
  '70x46',
  8,
  q//,
  '646ac633d5b5553721e032d2c9f8f54ffc19d315832bbf808c2b7321b1067293' );

1;
