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
  'c8c4f812d902693d1de6c74a6cffaaef7506bd868df65cae63b06707f2c9f3ac' );

#
# 2) Test reading PseudoColor (16 color)
#
++$test;
print("PseudoColor (16 color)...\n");
testRead( 'input_16.tiff',
  '7e704fc1a99118630a92374ba27adf5baf69f30019016be2ed70eac79629e8b4' );

#
# 3) Test reading PseudoColor (16 color + matte channel)
#
++$test;
print("PseudoColor (16 color + matte channel)...\n");
testRead( 'input_16_matte.tiff',
  '7e704fc1a99118630a92374ba27adf5baf69f30019016be2ed70eac79629e8b4' );

#
# 4) Test reading PseudoColor (256 color)
#
++$test;
print("PseudoColor (256 color) ...\n");
testRead( 'input_256.tiff',
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '1280e7ed7094aaae47c0be1cb0b6d33660e59483a5500f5f40e34940346f7847' );

#
# 5) Test reading PseudoColor (256 color + matte channel)
#
++$test;
print("PseudoColor (256 color + matte channel) ...\n");
testRead( 'input_256_matte.tiff',
        '824af58cdd8a8accffee3dab1ed9d28b34a8b183d3e5f5f13caeaab03bcadd13',
	'f3dc959e76f722bbc0a4338e2ed6650d73be3a81774c55210118531333fe6daa' );

#
# 6) Test reading PseudoColor using contiguous planar packing
#
++$test;
print("PseudoColor (256 color) contiguous planes ...\n");
testRead( 'input_256_planar_contig.tiff',
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '1280e7ed7094aaae47c0be1cb0b6d33660e59483a5500f5f40e34940346f7847' );

#
# 7) Test reading PseudoColor using seperate planes
#
++$test;
print("PseudoColor (256 color) seperate planes ...\n");
testRead( 'input_256_planar_separate.tiff',
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '1280e7ed7094aaae47c0be1cb0b6d33660e59483a5500f5f40e34940346f7847' );

#
# 8) Test Reading TrueColor (8-bit)
# 
++$test;
print("TrueColor (8-bit) image ...\n");
testRead( 'input_truecolor.tiff',
  '359291f6da6c9118bef6d75604be979b3267e4df0716e1bfc357f13cafd0acb8' );

#
# 9) Test Reading TrueColor (16-bit)
#
++$test;
print("TrueColor (16-bit) image ...\n");
testRead( 'input_truecolor_16.tiff',
  '9897466dce6a47db3530821056c0a1c6e20f20d5bbfce837addfbede63bdecab',
  '4c5c847c9e40a3ffc082f9fabadc29f87279008c9092fa749a9504c61f5e172a' );

#
# 10) Test Reading 8-bit TrueColor Tiled (32x32 tiles)
# 
++$test;
print("TrueColor (8-bit) tiled image, 32x32 tiles ...\n");
testRead( 'input_truecolor_tiled32x32.tiff',
  '359291f6da6c9118bef6d75604be979b3267e4df0716e1bfc357f13cafd0acb8' );

#
# 11) Test Reading 8-bit TrueColor Tiled (8 rows per strip)
# 
++$test;
print("TrueColor (8-bit) stripped, image, 8 rows per strip ...\n");
testRead( 'input_truecolor_stripped.tiff',
  '359291f6da6c9118bef6d75604be979b3267e4df0716e1bfc357f13cafd0acb8' );

#
# 12) Test Reading Grayscale 4-bit
#
++$test;
print("Grayscale (4-bit) ...\n");
testRead( 'input_gray_4bit.tiff',
  'be370e06f1aad47490e88b5212002c89520b07af6764690b3cee4cb9f1343df9');

#
# 13) Test Reading Grayscale 8-bit
# 
++$test;
print("Grayscale (8-bit) ...\n");
testRead( 'input_gray_8bit.tiff',
  '9bd950a80339e260c491025f5c58a21ca70c38e2c498914feda6558bfa1ffe35');

#
# 14) Test Reading Grayscale 8-bit + matte
# 
++$test;
print("Grayscale (8-bit + matte) ...\n");
testRead( 'input_gray_8bit_matte.tiff',
  'c34ac18bc2c04aa5d2577c579a620a1223e2249018ed6303cf08282f578d59c9' );

#
# 15) Test Reading Grayscale 12-bit
# 
++$test;
print("Grayscale (12-bit) ...\n");
testRead( 'input_gray_12bit.tiff',
  'ff6335069b6e140eb47149d847aea80bf7e2b06bd80ae9708aa382efb3ae21ee');

#
# 16) Test Reading Grayscale 16-bit
# 
++$test;
print("Grayscale (16-bit) ...\n");
testRead( 'input_gray_16bit.tiff',
  '9bd950a80339e260c491025f5c58a21ca70c38e2c498914feda6558bfa1ffe35');
