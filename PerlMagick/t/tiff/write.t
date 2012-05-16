#!/usr/bin/perl
#
# Test writing TIFF images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..10\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/tiff' || die 'Cd failed';

#
# 1) Test 4-bit pseudocolor image
#
print("PseudoColor image (4 bits/sample) ...\n");
testReadWrite( 'input_16.tiff',
  'output_16.tiff',
  q//,
  '7e704fc1a99118630a92374ba27adf5baf69f30019016be2ed70eac79629e8b4');

#
# 2) Test 8-bit pseudocolor image
#
++$test;
print("PseudoColor image (8 bits/sample) ...\n");
testReadWrite( 'input_256.tiff',
  'output_256.tiff',
  q//,
  'ec6408aba63b43dfc594b4bd766e43457754bb2382a02c170e3d085366e9a6f4',
  '1280e7ed7094aaae47c0be1cb0b6d33660e59483a5500f5f40e34940346f7847');

#
# 3) Test 4-bit pseudocolor + matte channel image
#
++$test;
print("PseudoColor image (4 bits/sample + matte channel) ...\n");
testReadWrite( 'input_16_matte.tiff',
  'output_16_matte.tiff',
  q//,
  '7e704fc1a99118630a92374ba27adf5baf69f30019016be2ed70eac79629e8b4' );

#
# 4) Test 8-bit pseudocolor + matte channel image
#
++$test;
print("PseudoColor image (8 bits/sample + matte channel) ...\n");
testReadWrite( 'input_256_matte.tiff',
  'output_256_matte.tiff',
  q//,
  '824af58cdd8a8accffee3dab1ed9d28b34a8b183d3e5f5f13caeaab03bcadd13',
  'f3dc959e76f722bbc0a4338e2ed6650d73be3a81774c55210118531333fe6daa' );

#
# 5) Test truecolor image
#
++$test;
print("TrueColor image (8 bits/sample) ...\n");
testReadWrite( 'input_truecolor.tiff',
  'output_truecolor.tiff',
  q/quality=>55/,
  '359291f6da6c9118bef6d75604be979b3267e4df0716e1bfc357f13cafd0acb8' );

#
# 6) Test monochrome image
#
++$test;
print("Gray image (1 bit per sample) ...\n");
testReadWrite(  'input_mono.tiff',
  'output_mono.tiff',
  q//,
  '57fc672e7e231d3f92793d9b2073132def273f4be3115bcbed1c49a1c3131222' );

#
# 7) Test gray 4 bit image
#
++$test;
print("Gray image (4 bits per sample) ...\n");
testReadWrite(  'input_gray_4bit.tiff',
  'output_gray_4bit.tiff',
  q//,
  'a3ae7f6908bb538751f59565dd17f28f83201620ca3ccc8a87a388b3d4c50232' );

#
# 8) Test gray 8 bit image
#
++$test;
print("Gray image (8 bits per sample) ...\n");
testReadWrite(  'input_gray_8bit.tiff',
  'output_gray_8bit.tiff',
  q//,
  '63783c30b21fca4cc94bb6c02ae37df722224a466d997db39bb7ddece5e236a8' );

#
# 9) Test gray 4 bit image (with matte channel)
#
++$test;
print("Gray image (4 bits per sample + matte channel) ...\n");
testReadWrite(  'input_gray_4bit_matte.tiff',
  'output_gray_4bit_matte.tiff',
  q//,
  'e87923c84a9140824abbb3683d603c19e8b1b08b926d78f794c5ea813db53615' );

#
# 10) Test gray 8 bit image (with matte channel)
#
++$test;
print("Gray image (8 bits per sample + matte channel) ...\n");
testReadWrite(  'input_gray_8bit_matte.tiff',
  'output_gray_8bit_matte.tiff',
  q//,
  'e87923c84a9140824abbb3683d603c19e8b1b08b926d78f794c5ea813db53615' );
