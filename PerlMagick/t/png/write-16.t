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
  'c45a7f8b2d978f5d92f70ddc40e0a7fec30dc3243facdb293f2245952ed68de1' );

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
  '4c8ba149f3b22a9d846e72e8317834871f5fb173799620d4d059e62f69576846' );

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
  'fa6b164245b385b3dea5764074be2c959a503dde90ecb1d4ba9c76a46bb8e4e6',
  '106f0647ae10a6516b1ab2968038161e287ef40d1b22ca047531ed768e594ef1');


