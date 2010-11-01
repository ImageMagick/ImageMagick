#!/usr/bin/perl
#
# An example of applying many settings in preparation for image creation.
#
# Extracted from PerlMagick Discussion forums..
# Gravity center, caption and wrapped text
#   http://www.imagemagick.org/discourse-server/viewtopic.php?f=7&t=17282
#
use strict;
use warnings;
use Image::Magick;

my $im = new Image::Magick;
my $e = $im->Set(
        background => 'none',
        fill => 'white',
        stroke => 'black',
        strokewidth => 2,
        Gravity => 'East',
        pointsize => 48,
        size => '200x300',
);
die $e if $e;

$e = $im->Read("caption:Lorem ipsum etc etc");
die $e if $e;

$e = $im->Trim();
die $e if $e;

$e = $im->Write('settings.png');
die $e if $e;
