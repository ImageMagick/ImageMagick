#!/usr/bin/perl
#
# Make simple beveled button.
#
use Image::Magick;

$q=Image::Magick->new;
$q->Set(size=>'30x106');
$q->Read('gradient:#00f685-#0083f8');
$q->Rotate(-90);
$q->Raise('6x6');
$q->Annotate(font=>'Generic.ttf',text=>'Push Me',fill=>'black',
  gravity=>'Center',pointsize=>18);
$q->Write('button.pam');
$q->Write(magick=>'SHOW',title=>"Button");
