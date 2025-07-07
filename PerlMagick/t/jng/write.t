#!/usr/bin/perl
#
# Test reading JPEG images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..11\n"; }
END {print "not ok $test\n" unless $loaded;}

use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/jng' || die 'Cd failed';

testReadWriteCompare( 'input_gray_idat.jng', 'gray_idat_tmp.jng', '../reference/jng/write_gray_idat.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_gray_jdaa.jng', 'gray_jdaa_tmp.jng', '../reference/jng/write_gray_jdaa.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_gray.jng', 'gray_tmp.jng', '../reference/jng/write_gray.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_gray_prog_idat.jng', 'gray_prog_idat_tmp.jng', '../reference/jng/write_gray_prog_idat.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_gray_prog_jdaa.jng', 'gray_prog_jdaa_tmp.jng', '../reference/jng/write_gray_prog_jdaa.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_gray_prog.jng', 'gray_prog_tmp.jng', '../reference/jng/write_gray_prog.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_idat.jng', 'idat_tmp.jng', '../reference/jng/write_idat.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_jdaa.jng', 'jdaa_tmp.jng', '../reference/jng/write_jdaa.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_prog_idat.jng', 'prog_idat_tmp.jng', '../reference/jng/write_prog_idat.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_prog_jdaa.jng', 'prog_jdaa_tmp.jng', '../reference/jng/write_prog_jdaa.miff', q//, q//, 0.5, 1.0);
++$test;
testReadWriteCompare( 'input_prog.jng', 'prog_tmp.jng', '../reference/jng/write_prog.miff', q//, q//, 0.5, 1.0);

