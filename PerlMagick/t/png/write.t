#!/usr/bin/perl
#
# Test writing PNG images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..6\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/png' || die 'Cd failed';

#
# 1) Test Black-and-white, bit_depth=1 PNG
# 
print( "1-bit grayscale PNG ...\n" );
testReadWrite( 'input_bw.png', 'output_bw.png', q/quality=>95/,
  '5ace96fd545d2f4479f2e7a8f6f8f6cb1fd7cd277ae35559dffc825fd2a670f6');

#
# 2) Test monochrome image
#
++$test;
print( "8-bit grayscale PNG ...\n" );
testReadWrite( 'input_mono.png',
  'output_mono.png', '',
  '4c8ba149f3b22a9d846e72e8317834871f5fb173799620d4d059e62f69576846');
#
# 3) Test 16-bit Portable Network Graphics
# 
++$test;
print( "16-bit grayscale PNG ...\n" );
testReadWrite( 'input_16.png',
  'output_16.png',
  q/quality=>55/,
  'fa6b164245b385b3dea5764074be2c959a503dde90ecb1d4ba9c76a46bb8e4e6',
  '106f0647ae10a6516b1ab2968038161e287ef40d1b22ca047531ed768e594ef1' );
#
# 4) Test pseudocolor image
#
++$test;
print( "8-bit indexed-color PNG ...\n" );
testReadWrite( 'input_256.png',
  'output_256.png',
  q/quality=>54/,
  'c45a7f8b2d978f5d92f70ddc40e0a7fec30dc3243facdb293f2245952ed68de1' );
#
# 5) Test truecolor image
#
++$test;
print( "24-bit Truecolor PNG ...\n" );
testReadWrite( 'input_truecolor.png',
  'output_truecolor.png',
  q/quality=>55/,
  '610257576e33bcbf79aa1edb7f56ad2b5cfa1d9b7413db632d0b29f412a7e194' );
#
# 6) Test Multiple-image Network Graphics
#
++$test;
print( "MNG with 24-bit Truecolor PNGs ...\n" );
testReadWrite( 'input.mng',
  'output.mng',
  q/quality=>55/,
  'ece756f9de4c618819cf88c8561630518a9cf39ce09a81bf7c78445d9f00e09d' );
