#!/usr/bin/perl
#
# Make simple text with a shadow.
#
use Image::Magick;

$image=Image::Magick->new(size=>'500x120');
$image->Read('xc:white');
$image->Annotate(font=>'Generic.ttf',fill=>'rgba(100,100,100,0.8)',
  pointsize=>60, text=>'Works like magick!',geometry=>'+30+90');
$image->Blur('0x1');
$image->Annotate(font=>'Generic.ttf',fill=>'red',stroke=>'blue',pointsize=>60,
  text=>'Works like magick!',geometry=>'+26+86');
$image->Write('shadow.gif');
$image->Write('win:');
