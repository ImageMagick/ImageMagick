#!/usr/bin/perl
#
# Test writing Postscript images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/ps' || die 'Cd failed';

#
# 1) Test Postscript
#
testReadWriteNoVerify( 'input.miff',
		       'output.ps',
		       q// );
#
# 2) Test Encapsulated Postscript
#
++$test;
testReadWriteNoVerify( 'input.miff',
		       'output.eps',
		       q// );

