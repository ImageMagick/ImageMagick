#!/usr/bin/perl
#  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
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
# Test reading formats supported directly by ImageMagick.
#
BEGIN { $| = 1; $test=1; print "1..47\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';

print("AVS X image file ...\n");
testReadCompare('input.avs', 'reference/read/input_avs.miff', q//, 0.0, 0.0);

print("Microsoft Windows bitmap image file ...\n");
++$test;
testReadCompare('input.bmp', 'reference/read/input_bmp.miff', q//, 0.0, 0.0);

print("Microsoft Windows 24-bit bitmap image file ...\n");
++$test;
testReadCompare('input.bmp24', 'reference/read/input_bmp24.miff', q//, 0.0, 0.0);

print("ZSoft IBM PC multi-page Paintbrush file ...\n");
++$test;
testReadCompare('input.dcx', 'reference/read/input_dcx.miff', q//, 0.0, 0.0);

print("Microsoft Windows bitmap image file ...\n");
++$test;
testReadCompare('input.dib', 'reference/read/input_dib.miff', q//, 0.0, 0.0);

print("Flexible Image Transport System ...\n");
++$test;
testReadCompare('input.fits', 'reference/read/input_fits.miff', q//, 0.0, 0.0);

print("CompuServe graphics interchange format ...\n");
++$test;
testReadCompare('input.gif', 'reference/read/input_gif.miff', q//, 0.02, 1.02);

print("CompuServe graphics interchange format (1987) ...\n");
++$test;
testReadCompare('input.gif87', 'reference/read/input_gif87.miff', q//, 0.02, 1.02);

print("Gradient (gradual passing from one shade to another) ...\n");
++$test;
testReadCompare('gradient:red-blue', 'reference/read/gradient.miff',
  q/size=>"70x46"/, 0.2, 1.02);

print("GRANITE (granite texture) ...\n");
++$test;
testReadCompare('granite:', 'reference/read/granite.miff', q/size=>"70x46"/, 0.0, 0.0);

print("MAT (MatLab gray 8-bit LSB integer) ...\n");
++$test;
testReadCompare('input_gray_lsb_08bit.mat', 'reference/read/input_gray_lsb_08bit_mat.miff', q//, 0.2, 1.02);

print("MAT (MatLab gray 8-bit MSB integer) ...\n");
++$test;
testReadCompare('input_gray_msb_08bit.mat', 'reference/read/input_gray_msb_08bit_mat.miff', q//, 0.0, 0.0);

print("MAT (MatLab gray 64-bit LSB double) ...\n");
++$test;
testReadCompare('input_gray_lsb_double.mat', 'reference/read/input_gray_lsb_double_mat.miff', q//, 0.2, 1.02);

print("MAT (MatLab RGB 8-bit LSB integer) ...\n");
++$test;
testReadCompare('input_rgb_lsb_08bit.mat', 'reference/read/input_rgb_lsb_08bit_mat.miff', q//, 0.2, 1.02);

print("Microsoft icon ...\n");
++$test;
testReadCompare('input.ico', 'reference/read/input_ico.miff', q//, 0.0, 0.0);

print("Magick image file format ...\n");
++$test;
testReadCompare('input.miff', 'reference/read/input_miff.miff', q//, 0.0, 0.0);

print("MTV Raytracing image format ...\n");
++$test;
testReadCompare('input.mtv', 'reference/read/input_mtv.miff', q//, 0.0, 0.0);

print("NULL (white image) ...\n");
++$test;
testReadCompare('NULL:white', 'reference/read/input_null_white.miff', q/size=>"70x46"/, 0.0, 0.0);

print("NULL (black image) ...\n");
++$test;
testReadCompare('NULL:black', 'reference/read/input_null_black.miff', q/size=>"70x46"/, 0.0, 0.0);

print("NULL (DarkOrange image) ...\n");
++$test;
testReadCompare('NULL:DarkOrange', 'reference/read/input_null_DarkOrange.miff', q/size=>"70x46"/, 0.0, 0.0);

print("Portable bitmap format (black and white), ASCII format ...\n");
++$test;
testReadCompare('input_p1.pbm', 'reference/read/input_pbm_p1.miff', q//, 0.0, 0.0);

print("Portable bitmap format (black and white), binary format ...\n");
++$test;
testReadCompare('input_p4.pbm', 'reference/read/input_pbm_p4.miff', q//, 0.0, 0.0);

print("ZSoft IBM PC Paintbrush file ...\n");
++$test;
testReadCompare('input.pcx', 'reference/read/input_pcx.miff', q//, 0.0, 0.0);

print("Portable graymap format (gray scale), ASCII format ...\n");
++$test;
testReadCompare('input_p2.pgm', 'reference/read/input_pgm_p2.miff', q//, 0.0, 0.0);

print("Portable graymap format (gray scale), binary format ...\n");
++$test;
testReadCompare('input_p5.pgm', 'reference/read/input_pgm_p5.miff', q//, 0.0, 0.0);

print("Apple Macintosh QuickDraw/PICT file ...\n");
++$test;
testReadCompare('input.pict', 'reference/read/input_pict.miff', q//, 0.0, 0.0);

print("Alias/Wavefront RLE image format ...\n");
++$test;
testReadCompare('input.rle', 'reference/read/input_rle.miff', q//, 0.0, 0.0);

print("Portable pixmap format (color), ASCII format ...\n");
++$test;
testReadCompare('input_p3.ppm', 'reference/read/input_ppm_p3.miff', q//, 0.0, 0.0);

print("Portable pixmap format (color), binary format ...\n");
++$test;
testReadCompare('input_p6.ppm', 'reference/read/input_ppm_p6.miff', q//, 0.0, 0.0);

print("Adobe Photoshop bitmap file ...\n");
++$test;
testReadCompare('input.psd', 'reference/read/input_psd.miff', q//, 0.0, 0.0);

print("Irix RGB image file ...\n");
++$test;
testReadCompare('input.sgi', 'reference/read/input_sgi.miff', q//, 0.25, 1.1);

print("SUN 1-bit Rasterfile ...\n");
++$test;
testReadCompare('input.im1', 'reference/read/input_im1.miff', q//, 0.0, 0.0);

print("SUN 8-bit Rasterfile ...\n");
++$test;
testReadCompare('input.im8', 'reference/read/input_im8.miff', q//, 0.0, 0.0);

print("SUN TrueColor Rasterfile ...\n");
++$test;
testReadCompare('sun:input.im24', 'reference/read/input_im24.miff', q//, 0.0, 0.0);

print("Truevision Targa image file ...\n");
++$test;
testReadCompare('input.tga', 'reference/read/input_tga.miff', q//, 0.0, 0.0);

print("PSX TIM file ...\n");
++$test;
testReadCompare('input.tim', 'reference/read/input_tim.miff', q//, 0.0, 0.0);

print("Khoros Visualization image file ...\n");
++$test;
testReadCompare('input.viff', 'reference/read/input_viff.miff', q//, 0.0, 0.0);

print("WBMP (Wireless Bitmap (level 0) image) ...\n");
++$test;
testReadCompare('input.wbmp', 'reference/read/input_wbmp.miff', q//, 0.0, 0.0);

print("X Windows system bitmap (black and white only) ...\n");
++$test;
testReadCompare('input.xbm', 'reference/read/input_xbm.miff', q//, 0.0, 0.0);

print("XC: Constant image of X server color ...\n");
++$test;
testReadCompare('xc:black', 'reference/read/input_xc_black.miff', q/size=>"70x46",, depth=>8/, 0.0, 0.0);

print("X Windows system pixmap file (color) ...\n");
++$test;
testReadCompare('input.xpm', 'reference/read/input_xpm.miff', q//, 0.0, 0.0);

print("TILE (Tile image with a texture) ...\n");
# This is an internal generated format
# We will tile using the default image and a MIFF file
#
++$test;
testReadCompare('TILE:input.miff', 'reference/read/input_tile.miff',
                q/size=>"140x92", depth=>8/, 0.0, 0.0);

print("CMYK format ...\n");
++$test;
testReadCompare('input_70x46.cmyk', 'reference/read/input_cmyk.miff',
                q/size=>"70x46", depth=>8/, 0.0, 0.0);

print("GRAY format ...\n");
++$test;
testReadCompare('input_70x46.gray', 'reference/read/input_gray.miff',
                q/size=>"70x46", depth=>8/, 0.0, 0.0);

print("RGB format ...\n");
++$test;
testReadCompare('input_70x46.rgb', 'reference/read/input_rgb.miff',
                q/size=>"70x46", depth=>8/, 0.0, 0.0);

print("RGBA format ...\n");
++$test;
testReadCompare('input_70x46.rgba', 'reference/read/input_rgba.miff',
                q/size=>"70x46", depth=>8/, 0.0, 0.0);

print("UYVY format ...\n");
++$test;
testReadCompare('input_70x46.uyvy', 'reference/read/input_uyvy.miff',
                q/size=>"70x46", depth=>8/, 0.2, 1.02);
