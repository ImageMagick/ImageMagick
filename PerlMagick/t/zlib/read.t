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

require 't/subroutines.pl';

chdir 't/zlib' || die 'Cd failed';

#
# 1) Test reading Zip compressed MIFF
# 
testRead( 'input.miff',
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33' );

#
# 3) Test reading Zip stream-compressed MIFF (.gz extension)
#
print("Reading Zip stream-compressed MIFF (.gz extension) ...\n");
++$test;
testRead( 'input.miff.gz',
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33' );
