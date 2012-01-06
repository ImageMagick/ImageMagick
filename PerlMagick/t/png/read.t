#!/usr/bin/perl
#
# Test reading PNG images
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
testRead( 'input_bw.png',
  'b0ed82ae119a2baa1e8bf714fab26415708c47b8f07771fb5506ff101a550dbc' );

#
# 2) Test Monochrome PNG
# 
++$test;
print( "8-bit grayscale PNG ...\n" );
testRead( 'input_mono.png',
  '1502b4e5747ed8805fe9103f56f9c4097f60440b3a867fa6b1ae0ba81f12a90d' );

#
# 3) Test 16-bit Portable Network Graphics
# 
++$test;
print( "16-bit grayscale PNG ...\n" );
testRead( 'input_16.png',
  '6b6761c8108b1616e9411c4ef2564505715a37b93e86d2c824c9a4bca31bf47b',
  '30e14955ebf1352266dc2ff8067e68104607e750abb9d3b36582b8af909fcb58' );
#
# 4) Test 256 color pseudocolor PNG
# 
++$test;
print( "8-bit indexed-color PNG ...\n" );
testRead( 'input_256.png',
  '4404bce58d768dda28165b81ad6618e6fd6553996a44e62486f4d46c6ac7e593' );

#
# 5) Test TrueColor PNG
# 
++$test;
print( "24-bit Truecolor PNG ...\n" );
testRead( 'input_truecolor.png',
  '610257576e33bcbf79aa1edb7f56ad2b5cfa1d9b7413db632d0b29f412a7e194' );

#
# 6) Test Multiple-image Network Graphics
# 
++$test;
print( "MNG with 24-bit Truecolor PNGs...\n" );
testRead( 'input.mng',
  '70e2aabf6c94d5fb8660c3a4cec5ecce72f5b6b3f98a8617a2db5d3519db6ae0' );

