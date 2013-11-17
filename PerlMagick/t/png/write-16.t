#!/usr/bin/perl
#
# Test writing PNG images when 16bit support is enabled
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
# 1) Test pseudocolor image
#
testReadWrite( 'input_256.png',
  'output_256.png',
  q/quality=>54/,
  '26687d6be46b4e1ca5201a47caa7dad660245c5f4517835ab6e19f2f84fc4a03' );

#
# 2) Test truecolor image
#
++$test;
testReadWrite( 'input_truecolor.png',
  'output_truecolor.png',
  q/quality=>55/,
  '76f43f03e51c8608bfca7ff96d3936148ad855782968a017ea03cccbeebff64d' );

#
# 3) Test monochrome image
#
++$test;
testReadWrite( 'input_mono.png',
  'output_mono.png', '',
  'a8d79029f6ced36f904659dda5ea67a6bab5e602b1033aa29c7f1b45b35c1178' );

#
# 4) Test Multiple-image Network Graphics
#
++$test;
testReadWrite( 'input.mng',
  'output.mng',
  q/quality=>55/,
  '5a883e67c29d7bb27897ec7a3f3cb3fb0330fb43aef4360e8314503cb37fae09' );

#
# 5) Test 16-bit Portable Network Graphics
# 
++$test;
testReadWrite( 'input_16.png',
  'output_16.png',
  q/quality=>55/,
  'd4bed86abb1849f69f1a5afb7c5cf8798e8192ba228357f189c277198c14f5a0',
  '4036b56c00d731a2865470815eabc177dc158af2abacb34ef95c7e716ec2da60');


