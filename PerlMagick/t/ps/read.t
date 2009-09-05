#!/usr/bin/perl
#
# Test Reading Postscript images
#
# Contributed by Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#

BEGIN { $| = 1; $test=1; print "1..3\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't/ps' || die 'Cd failed';

#
# 1) Test reading Postscript
#
$image=Image::Magick->new;
$x=$image->ReadImage('input.ps');
if( "$x" ) {
  print "ReadImage: $x\n";
  print "not ok $test\n";
} else {
    print "ok $test\n";
}
undef $image;


#
# 2) Test reading Encapsulated Postscript
#
++$test;
$image=Image::Magick->new;
$x=$image->ReadImage('input.eps');
if( "$x" ) {
  print "ReadImage: $x\n";
  print "not ok $test\n";
} else {
    print "ok $test\n";
}
undef $image;

#
# 3) Test rendering using a Postscript font
#
++$test;
$font   = 'helvetica';

$image=Image::Magick->new;
$x=$image->Set(font=>"$font", pen=>'#0000FF', dither=>'False');
if( "$x" ) {
  print "$x\n";
  print "not ok $test\n";
} else {
  $x=$image->ReadImage('label:The quick brown fox jumps over the lazy dog.');
  if ( "$x" ) {
    print "ReadImage: $x\n";
    # If server can't be accessed, ImageMagick returns this warning
    # Warning 305: Unable to open X server
    $x =~ /(\d+)/;
    my $errorCode = $1;
    if ( $errorCode > 0 ) {
      print "not ok $test\n";
    } else {
      print "ok $test\n";
    }
  } else {
    print "ok $test\n";
  }
}
undef $image;
