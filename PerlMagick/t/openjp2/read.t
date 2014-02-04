#!/usr/bin/perl
#
# Test read image method on non-interlaced JPEG.
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..3\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/openjp2' || die 'Cd failed';

#
# 1) JPEG-2000 JP2 File Format Syntax (ISO/IEC 15444-1)
# 
print( "JPEG-2000 JP2 File Format Syntax ...\n" );
testReadCompare('input.jp2', '../reference/openjp2/read_jp2.miff', q//, 0.0, 0.0);

#
# 2) JPEG-2000 Code Stream Syntax (ISO/IEC 15444-1)
# 
++$test;
print( " ...\n" );
testReadCompare('input.jpc', '../reference/openjp2/read_jpc.miff', q//, 0.0, 0.0);

#
# 3) JPEG-2000 VM Format
# 
++$test;
print( " ...\n" );
testReadCompare('input.j2k', '../reference/openjp2/read_j2k.miff', q//, 0.0, 0.0);
