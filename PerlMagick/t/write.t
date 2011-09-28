#!/usr/bin/perl
#  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');

print("Microsoft Windows bitmap image file ...\n");
++$test;
testReadWrite( 'BMP:input.bmp',
  'BMP:output.bmp',
  q//,
  '32d82b4ab7a2527d0b886ccdd60990f6f7e4a411181337bd033760256c0d596c');

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
  'f28470f09857477a372a743665071cdc325613b963481d94b6dceabe292dd469' );

print("CompuServe graphics interchange format ...\n");
++$test;
testReadWrite( 'GIF:input.gif',
  'GIF:output.gif',
  q//,
  '32d82b4ab7a2527d0b886ccdd60990f6f7e4a411181337bd033760256c0d596c');

print("CompuServe graphics interchange format (1987) ...\n");
++$test;
testReadWrite( 'GIF87:input.gif87',
  'GIF87:output.gif87',
  q//,
  'd8938f29cbd56aa164bf8265f385d9ec872be75fe08401601b68eee159df8cd9');

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
  '57fc672e7e231d3f92793d9b2073132def273f4be3115bcbed1c49a1c3131222');

print("Portable bitmap format (black and white), binary format ...\n");
++$test;
testReadWrite( 'PBM:input_p4.pbm',
  'PBM:output_p4.pbm',
  q//,
  '57fc672e7e231d3f92793d9b2073132def273f4be3115bcbed1c49a1c3131222');

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
  'e2e1b058a09c2b9c0c696996163911dcce325def773cb2a1554c6f21a0391ff7');

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
  'e2e1b058a09c2b9c0c696996163911dcce325def773cb2a1554c6f21a0391ff7');

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
  '31d166e543d44963cc8a500212ae60c1c6f040fac3117748bcd54174727b45e1');

print("SUN 8-bit Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im8',
  'SUN:output.im8',
  q//,
  '88528b9fde504a6388f0db5a351e3373ea15607e18a0c2d0e7e0fe4c851cce10');

print("SUN True-Color Rasterfile ...\n");
++$test;
testReadWrite( 'SUN:input.im24',
  'SUN:output.im24',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33',
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33',
  '5a5f94a626ee1945ab1d4d2a621aeec4982cccb94e4d68afe4c784abece91b3e');

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
  '7f2c98e7ce98983509580eaeb3bb6a420e3f358b39fcec4cdd96982ae1e21882',
  '409e646b6ba024c597a6b2c34754bf23713857e367047e7ea15615d8bd0fd028',
  'aa4a6154f3c314d99c257280faf9097f3863a132ec8bddbc3b68209ce2c19487');

print("WBMP (Wireless Bitmap (level 0) image) ...\n");
++$test;
testReadWrite( 'WBMP:input.wbmp',
  'WBMP:output.wbmp',
  q//,
  'b7b682361e82d9d7cf2bed34f76af87576b97590b12d76b961104e53ee18ee74',
  'd386466607cfcec0625943f277454d66cce1d17696d482f50103a25a04bd1070',
  'd818195f73f8d5db624c8f87a706bbcb3179dbb7a7f08abbad5b12cd97de8fe6');

print("X Windows system bitmap (black and white only) ...\n");
++$test;
testReadWrite( 'XBM:input.xbm',
  'XBM:output.xbm',
  q//,
  '31d166e543d44963cc8a500212ae60c1c6f040fac3117748bcd54174727b45e1');

print("X Windows system pixmap file (color) ...\n");
++$test;
testReadWrite( 'XPM:input.xpm',
  'XPM:output.xpm',
  q//,
  '88528b9fde504a6388f0db5a351e3373ea15607e18a0c2d0e7e0fe4c851cce10');

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
  'f28470f09857477a372a743665071cdc325613b963481d94b6dceabe292dd469' );

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
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33' );

1;
