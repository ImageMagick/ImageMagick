#!/usr/bin/perl
#
# Test accessing X11 server
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..2\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/x11' || die 'Cd failed';


# 1) Test reading and displaying an image
#
if ( 0 && defined($ENV{'DISPLAY'}) && ($^O ne 'MSWin32') ) {
  $image=Image::Magick->new;
  $x=$image->ReadImage('congrats.miff');
  if( "$x" ) {
    print "not ok $test\n";
  } else {
    $x = $image->Display(delay=>800);
    if( "$x" ) {
      print "not ok $test\n";
    } else {
      print "ok $test\n";
    }
  }
  undef $image;
} else {
  print "ok $test\n";
}

# 2) Test XWD image file
#
print("X Windows system window dump file (color) ...\n");
++$test;
testReadWrite( 'XWD:input.xwd',
  'XWD:output.xwd',
  q//,
  '6a4a257921582768b774aeeac549b7c0c0b51f665395eddf921cce53a0ad2a33');
