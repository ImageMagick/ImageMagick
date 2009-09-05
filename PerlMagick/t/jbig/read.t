#!/usr/bin/perl
#
# Test read image method on JBIG image
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
BEGIN { $| = 1; $test=1; print "1..1\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/jbig' || die 'Cd failed';

testRead( 'input.jbig',
  'f7047e83432fbef872f3034a19efb95aa3b241d0be228ae0ccdc7814a9e27811');
