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
  '4404bce58d768dda28165b81ad6618e6fd6553996a44e62486f4d46c6ac7e593' );

#
# 2) Test truecolor image
#
++$test;
testReadWrite( 'input_truecolor.png',
  'output_truecolor.png',
  q/quality=>55/,
  '610257576e33bcbf79aa1edb7f56ad2b5cfa1d9b7413db632d0b29f412a7e194' );

#
# 3) Test monochrome image
#
++$test;
testReadWrite( 'input_mono.png',
  'output_mono.png', '',
  '7e704fc1a99118630a92374ba27adf5baf69f30019016be2ed70eac79629e8b4' );

#
# 4) Test Multiple-image Network Graphics
#
++$test;
testReadWrite( 'input.mng',
  'output.mng',
  q/quality=>55/,
  'ece756f9de4c618819cf88c8561630518a9cf39ce09a81bf7c78445d9f00e09d' );

#
# 5) Test 16-bit Portable Network Graphics
# 
++$test;
testReadWrite( 'input_16.png',
  'output_16.png',
  q/quality=>55/,
  'd4bed86abb1849f69f1a5afb7c5cf8798e8192ba228357f189c277198c14f5a0',
  '5647f05ec18958947d32874eeb788fa396a05d0bab7c1b71f112ceb7e9b31eee');


