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
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7' );

#
# 3) Test reading Zip stream-compressed MIFF (.gz extension)
#
print("Reading Zip stream-compressed MIFF (.gz extension) ...\n");
++$test;
testRead( 'input.miff.gz',
  'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7' );
