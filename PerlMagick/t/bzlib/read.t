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
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33' );

#
# 2) Test reading BZip stream-compressed MIFF (.bz2 extension)
#
print("Reading BZip stream-compressed MIFF (.bz2 extension) ...\n");
++$test;
testRead( 'input.miff.bz2',
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33' );

