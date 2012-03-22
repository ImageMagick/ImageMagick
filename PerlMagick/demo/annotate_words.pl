#!/usr/bin/perl
#
# annotate_words.pl
#
# Take the internal string, split it into words and try to annotate each
# individual word correctly, so as to control spacing between the words
# under program control.
#
# A demonstration of using QueryFontMetrics(), by passing it exactly the same
# arguments as you would for Annotate(), to determine the location of the
# text that is/was drawn.
#
# Example script from   Zentara
#    http://zentara.net/Remember_How_Lucky_You_Are.html
#
use warnings;
use strict;
use Image::Magick;

my $image = Image::Magick->new;
$image->Set(size=>'500x200');
my $rc = $image->Read("xc:white");

my $str = 'Just Another Perl Hacker';
my (@words) = split ' ',$str;
#print join "\n",@words,"\n";

my ($x,$y) = (50,50);

foreach my $word (@words){

  $image->Annotate(
         pointsize => 24,
         fill      => '#000000ff', #last 2 digits transparency in hex ff=max
         text      => $word,
         gravity   => 'NorthWest',
         align     => 'left',
         x         => $x,
         y         => $y,
    );

  my ( $character_width,$character_height,$ascender,$descender,$text_width,
      $text_height,$maximum_horizontal_advance, $boundsx1, $boundsy1,
      $boundsx2, $boundsy2,$originx,$originy) =
          $image->QueryFontMetrics(
             pointsize => 24,
             text      => $word,
             gravity   => 'NorthWest',
             align     => 'left',
             x         => $x,
             y         => $y,
           );

  print "$word ( $character_width, $character_height,
         $ascender,$descender,
         $text_width, $text_height,
         $maximum_horizontal_advance,
         $boundsx1, $boundsy1,
         $boundsx2, $boundsy2,
         $originx,$originy)\n";

  my $n = $x + $originx + $character_width/3;  # add a space
  print "Next word at: $x + $originx + $character_width/3 => $n\n";
  $x = $n;

}

$image->Write("show:");

exit;

