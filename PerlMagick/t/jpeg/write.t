#!/usr/bin/perl
#
# Test reading JPEG images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/jpeg' || die 'Cd failed';

#
# 1) Test with non-interlaced image
#
print( "Non-interlaced JPEG ...\n" );
testReadWriteCompare( 'input.jpg', 'output_tmp.jpg',
                      '../reference/jpeg/write_non_interlaced.miff',
                      q//, q//, 0.02, 0.27);

#
# 2) Test with plane-interlaced image
#
++$test;
print( "Plane-interlaced JPEG ...\n" );
testReadWriteCompare( 'input.jpg', 'output_plane_tmp.jpg',
                      '../reference/jpeg/write_plane_interlaced.miff',
                      q//, q//, 0.02, 0.27);

