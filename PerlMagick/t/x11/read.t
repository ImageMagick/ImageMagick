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

#
# 1) Test rendering text using common X11 font
#

$font   = '-*-courier-bold-r-normal-*-14-*-*-*-*-*-iso8859-1';

# Ensure that Ghostscript is out of the picture
$SAVEDPATH=$ENV{'PATH'};
$ENV{'PATH'}='';

$image=Image::Magick->new;
$x=$image->Set(font=>"$font", pen=>'#0000FF', dither=>'False');
if( "$x" ) {
  print "$x\n";
  print "not ok $test\n";
} else {
  $x=$image->ReadImage('label:The quick brown fox jumps over the lazy dog.');
  if( "$x" ) {
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
    #$image->Display();
    print "ok $test\n";
  }
}
undef $image;

$ENV{'PATH'}=$SAVEDPATH;

print("X Windows system window dump file (color) ...\n");
++$test;
testReadCompare('input.xwd', '../reference/read/input_xwd.miff', q//, 0.0, 0.0);
