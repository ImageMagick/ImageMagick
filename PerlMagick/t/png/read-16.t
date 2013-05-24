#!/usr/bin/perl
#
# Test reading PNG images when 16bit support is enabled
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..5\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/png' || die 'Cd failed';

#
# 1) Test Monochrome PNG
# 
testRead( 'input_mono.png',
  'a8d79029f6ced36f904659dda5ea67a6bab5e602b1033aa29c7f1b45b35c1178' );

#
# 2) Test 256 color pseudocolor PNG
# 
++$test;
testRead( 'input_256.png',
  '5d1041037358767b9ebd2d5398c50daeee2ae644e2d128006e2eb91e82cf4b16' );

#
# 3) Test TrueColor PNG
# 
++$test;
testRead( 'input_truecolor.png',
  '76f43f03e51c8608bfca7ff96d3936148ad855782968a017ea03cccbeebff64d' );

#
# 4) Test Multiple-image Network Graphics
# 
++$test;
testRead( 'input.mng',
  '5a883e67c29d7bb27897ec7a3f3cb3fb0330fb43aef4360e8314503cb37fae09' );

#
# 5) Test 16-bit Portable Network Graphics
# 
++$test;
testRead( 'input_16.png',
  'd4bed86abb1849f69f1a5afb7c5cf8798e8192ba228357f189c277198c14f5a0',
  '4036b56c00d731a2865470815eabc177dc158af2abacb34ef95c7e716ec2da60');

