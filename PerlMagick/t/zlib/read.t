#!/usr/bin/perl
#
# Test reading an image which uses Zip compression
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

use Cwd;
use lib cwd;
require 't/subroutines.pl';

chdir 't/zlib' || die 'Cd failed';

#
# 1) Test reading Zip compressed MIFF
# 
testRead( 'input.miff',
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52' );

#
# 3) Test reading Zip stream-compressed MIFF (.gz extension)
#
print("Reading Zip stream-compressed MIFF (.gz extension) ...\n");
++$test;
testRead( 'input.miff.gz',
  'fb6fc68beb3b1001c5ebaa671c8ac8fddea06995027127765ff508f77723cc52' );
