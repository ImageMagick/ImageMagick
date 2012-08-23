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
  'cda5c7a8ba8250de624af6dc825ad6772ebba3a7fa6da756c5b1ca228b62f8ac' );

#
# 2) Test 256 color pseudocolor PNG
# 
++$test;
testRead( 'input_256.png',
  '066c0047c6e7e3f4cda1c86224441bfd5f522b5805b2a9190dcfa5294d94e4bd' );

#
# 3) Test TrueColor PNG
# 
++$test;
testRead( 'input_truecolor.png',
  '55913611798c087b9300b14d3baeda08a142910ad120379a9308a6b8c8b2f6e8' );

#
# 4) Test Multiple-image Network Graphics
# 
++$test;
testRead( 'input.mng',
  '40805ef3db6e3a94c85e30e591e5881dc660ff863591d6f56605dba64d03d83d' );

#
# 5) Test 16-bit Portable Network Graphics
# 
++$test;
testRead( 'input_16.png',
  '6b6761c8108b1616e9411c4ef2564505715a37b93e86d2c824c9a4bca31bf47b',
  '82f48df83eec5bacbe2c38f13ce7e2219e5e318f4b2974d928d0ea7f7cec65fd',
  'a6619f482fee91a315f76cdcd8705d39b6ce11077c435ccc696142e130c27762' );

