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
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e' );

#
# 3) Test reading Zip stream-compressed MIFF (.gz extension)
#
print("Reading Zip stream-compressed MIFF (.gz extension) ...\n");
++$test;
testRead( 'input.miff.gz',
  'd0b17026dc758a4088c96bc0f21a2bf14e3ab4af8a35638129f7731f2f1cbe7e' );
