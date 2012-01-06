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
  '1502b4e5747ed8805fe9103f56f9c4097f60440b3a867fa6b1ae0ba81f12a90d' );

#
# 2) Test 256 color pseudocolor PNG
# 
++$test;
testRead( 'input_256.png',
  '4404bce58d768dda28165b81ad6618e6fd6553996a44e62486f4d46c6ac7e593' );

#
# 3) Test TrueColor PNG
# 
++$test;
testRead( 'input_truecolor.png',
  '610257576e33bcbf79aa1edb7f56ad2b5cfa1d9b7413db632d0b29f412a7e194' );

#
# 4) Test Multiple-image Network Graphics
# 
++$test;
testRead( 'input.mng',
  '70e2aabf6c94d5fb8660c3a4cec5ecce72f5b6b3f98a8617a2db5d3519db6ae0' );

#
# 5) Test 16-bit Portable Network Graphics
# 
++$test;
testRead( 'input_16.png',
  '6b6761c8108b1616e9411c4ef2564505715a37b93e86d2c824c9a4bca31bf47b',
  '30e14955ebf1352266dc2ff8067e68104607e750abb9d3b36582b8af909fcb58');

