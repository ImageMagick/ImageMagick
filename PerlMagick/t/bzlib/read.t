#!/usr/bin/perl
#
# Test reading an image which uses BZip compression
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/bzlib' || die 'Cd failed';

#
# Test reading BZip compressed MIFF
# 
testRead( 'input.miff',
  'a698f2fe0c6c31f83d19554a6ec02bac79c961dd9a87e7ed217752e75eb615d7' );

#
# 2) Test reading BZip stream-compressed MIFF (.bz2 extension)
#
print("Reading BZip stream-compressed MIFF (.bz2 extension) ...\n");
++$test;
testRead( 'input.miff.bz2',
  'a698f2fe0c6c31f83d19554a6ec02bac79c961dd9a87e7ed217752e75eb615d7' );

