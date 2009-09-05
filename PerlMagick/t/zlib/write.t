#!/usr/bin/perl
#
# Test writing files using zlib-based compression
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..1\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/zlib' || die 'Cd failed';

#
# 1) Test writing Zip-compressed MIFF
#

testReadWrite( 'input.miff',
  'output.miff',
  q/compression=>'Zip'/,
  'a698f2fe0c6c31f83d19554a6ec02bac79c961dd9a87e7ed217752e75eb615d7' );

$test = 0;  # Quench PERL compliaint

