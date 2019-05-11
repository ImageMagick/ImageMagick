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
  '0de2dcbf667c69ae6735d1a701b4038c1eeea25cc86981a496bb26fc82541835');

#
# 2) Test 8-bit pseudocolor image
#
++$test;
print("PseudoColor image (8 bits/sample) ...\n");
testReadWrite( 'input_256.tiff',
  'output_256.tiff',
  q//,
  'b2644ac928730aa1d28e754aeb17b4731b57daea28c9fb89b1b50623e87215b5');

#
# 3) Test 4-bit pseudocolor + matte channel image
#
++$test;
print("PseudoColor image (4 bits/sample + matte channel) ...\n");
testReadWrite( 'input_16_matte.tiff',
  'output_16_matte.tiff',
  q//,
  '0de2dcbf667c69ae6735d1a701b4038c1eeea25cc86981a496bb26fc82541835' );

#
# 4) Test 8-bit pseudocolor + matte channel image
#
++$test;
print("PseudoColor image (8 bits/sample + matte channel) ...\n");
testReadWrite( 'input_256_matte.tiff',
  'output_256_matte.tiff',
  q//,
  'c8e5089f89ed3b7d067222e187ccd95da0a586f3a7f669876188fe8bfa04e6d9' );

#
# 5) Test truecolor image
#
++$test;
print("TrueColor image (8 bits/sample) ...\n");
testReadWrite( 'input_truecolor.tiff',
  'output_truecolor.tiff',
  q/quality=>55/,
  'f72b63be472e5e730ee2635463c6643d11057d251709ffe1f2027f69b57449df' );

#
# 6) Test monochrome image
#
++$test;
print("Gray image (1 bit per sample) ...\n");
testReadWrite(  'input_mono.tiff',
  'output_mono.tiff',
  q//,
  '71e1a6be223e307b1dbf732860792b15adba662b7a7ef284daf7f982f874ccf1' );

#
# 7) Test gray 4 bit image
#
++$test;
print("Gray image (4 bits per sample) ...\n");
testReadWrite(  'input_gray_4bit.tiff',
  'output_gray_4bit.tiff',
  q//,
  'e55c01b0d28b0a19431ba27203db7cb6ada189c9519d4466c44a764aad5e185a' );

#
# 8) Test gray 8 bit image
#
++$test;
print("Gray image (8 bits per sample) ...\n");
testReadWrite(  'input_gray_8bit.tiff',
  'output_gray_8bit.tiff',
  q//,
  'b51e862fcc24d439870da413c664dfefc36cea1260d807b3208d6f091566263c' );

#
# 9) Test gray 4 bit image (with matte channel)
#
++$test;
print("Gray image (4 bits per sample + matte channel) ...\n");
testReadWrite(  'input_gray_4bit_matte.tiff',
  'output_gray_4bit_matte.tiff',
  q//,
  'aa1ccb94820722df3dbb8a84410b6e8f6d5a3e393e5b2204923a16182e0958b9' );

#
# 10) Test gray 8 bit image (with matte channel)
#
++$test;
print("Gray image (8 bits per sample + matte channel) ...\n");
testReadWrite(  'input_gray_8bit_matte.tiff',
  'output_gray_8bit_matte.tiff',
  q//,
  '6002e57537cd54733551f8c4269e8104f2b14f8fcc58a07eda61f5911eb11c80' );
