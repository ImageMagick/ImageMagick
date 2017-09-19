#!/usr/bin/perl

use Image::Magick;

$image = Image::Magick->new();
$x = 100;
$y = 100;
for ($angle=0; $angle < 360; $angle+=30)
{
  my ($label);

  print "angle $angle\n";
  $label=Image::Magick->new(size=>"600x600",pointsize=>24);
  $label->Read("xc:white");
  $label->Draw(primitive=>'line',points=>"300,100 300,500",stroke=>'#600');
  $label->Draw(primitive=>'line',points=>"100,300 500,300",stroke=>'#600');
  $label->Draw(primitive=>'rectangle',points=>"100,100 500,500",fill=>'none',
    stroke=>'#600');
  $label->Annotate(text=>"North West",gravity=>"NorthWest",x=>$x,y=>$y,
    undercolor=>'yellow',rotate=>$angle);
  $label->Annotate(text=>"North",gravity=>"North",y=>$y,rotate=>$angle);
  $label->Annotate(text=>"North East",gravity=>"NorthEast",x=>$x,y=>$y,
    rotate=>$angle);
  $label->Annotate(text=>"West",gravity=>"West",x=>$x,rotate=>$angle);
  $label->Annotate(text=>"Center",gravity=>"Center",rotate=>$angle);
  $label->Annotate(text=>"East",gravity=>"East",x=>$x,rotate=>$angle);
  $label->Annotate(text=>"South West",gravity=>"SouthWest",x=>$x,y=>$y,
    rotate=>$angle);
  $label->Annotate(text=>"South",gravity=>"South",y=>$y,rotate=>$angle);
  $label->Annotate(text=>"South East",gravity=>"SouthEast",x=>$x,y=>$y,
    rotate=>$angle);
  push(@$image,$label);
}
$image->Set(delay=>20);
$image->Write("annotate.miff");
$image->Animate();
