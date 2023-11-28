#!/usr/bin/perl
#
# Make simple text with a shadow.
#
use Image::Magick;

$image=Image::Magick->new(size=>'525x125');
$image->Read('xc:white');
$image->Annotate(font=>'Generic.ttf',fill=>'rgba(100,100,100,0.8)',
  pointsize=>60,text=>'Works like magick!',geometry=>'+8+90');
$image->Blur('0x1');
$image->Annotate(font=>'Generic.ttf',fill=>'red',stroke=>'blue',pointsize=>60,
  text=>'Works like magick!',geometry=>'+4+86');
$image->Write('shadow.pam');
$image->Write(magick=>'SHOW',title=>"Shadow Text");
