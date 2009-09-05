#!/usr/bin/perl
#
# Test Reading HP GL images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..1\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/hpgl' || die 'Cd failed';

#
# 1) Test reading HP GL
#
$image=Image::Magick->new;
$x=$image->ReadImage('input.hpgl');
if( "$x" ) {
  print "ReadImage: $x\n";
  print "not ok $test\n";
} else {
    print "ok $test\n";
}

undef $image;
