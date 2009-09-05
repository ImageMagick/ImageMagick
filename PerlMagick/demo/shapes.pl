#!/usr/bin/perl
# GD example using PerlMagick methods.

use Image::Magick;

#
# Create a 300x300 white canvas.
#
$image=Image::Magick->new;
$image->Set(size=>'300x300');
$image->Read('xc:white');
#
# Draw shapes.
#
$tile=Image::Magick->new;
$tile->Read('tile.gif');
$image->Draw(primitive=>'Polygon',tile=>$tile,fill=>'none',
  points=>'30,30 100,10 190,290 30,290');
$image->Draw(stroke=>'red',primitive=>'Ellipse',stroke=>'black',fill=>'red',
  strokewidth=>5,points=>'100,100 50,75 0,360');
$image->Draw(primitive=>'Polygon',fill=>'none',stroke=>'black',strokewidth=>5,
  points=>'30,30 100,10 190,290 30,290');
$image->FloodfillPaint(geometry=>'+132+62',fill=>'blue',bordercolor=>'black',
  invert=>'true');
#
# Draw text.
#
$image->Annotate(fill=>'red',geometry=>'+150+20',font=>'Generic.ttf',
  pointsize=>18,text=>'Hello world!');
$image->Annotate(fill=>'blue',geometry=>'+150+38',font=>'Generic.ttf',
  pointsize=>14,text=>'Goodbye cruel world!');
$image->Annotate(fill=>'black',geometry=>'+280+120',font=>'Generic.ttf',
  pointsize=>14,text=>"I'm climbing the wall!",rotate=>90.0);
#
# Write image.
#
print "Write image...\n";
$image->Write('shapes.gif');
print "Display image...\n";
$image->Write('win:');
