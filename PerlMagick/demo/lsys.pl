#!/usr/bin/perl

# Written by jreed@itis.com, adapted by Cristy.

use Image::Magick;
use Turtle;

sub flower
{
  my $flower = shift;
  my ($width, $height) = $flower->Get('width', 'height');
  my ($x, $y) = $turtle->state();
  my ($geometry);

  $geometry = '+' . int($x-$width/2) . '+' . int($y-$height/2);
  $im->Composite(image=>$flower, compose=>'over', geometry=>$geometry);
}

sub lsys_init
{
  my ($imagesize) = @_;
  
  %translate =
  (
    'S' => sub{ # Step forward
                $turtle->forward($changes->{"distance"},
                $changes->{"motionsub"});
              },
    '-' => sub{ $turtle->turn(-$changes->{"dtheta"}); },  # counter-clockwise
    '+' => sub{ $turtle->turn($changes->{"dtheta"}); },  # Turn clockwise
    'M' => sub{ $turtle->mirror(); },  # Mirror
    '[' => sub{ push(@statestack, [$turtle->state()]); },  # Begin branch
    ']' => sub{ $turtle->setstate(@{pop(@statestack)}); },  # End branch
    '{' => sub{ @poly = (); $changes=\%polychanges; },  # Begin polygon
    '}' => sub{ # End polygon
                $im->Draw (primitive=>'Polygon', points=>join(' ',@poly),
                           fill=>'light green');
                $changes = \%stemchanges;
              },
    'f' => sub{ flower($pink_flower); },  # Flower
    'g' => sub{ flower($red_flower); },  # Flower
    'h' => sub{ flower($yellow_flower); }  # Flower
  );

  # Create the main image
  $im = new Image::Magick;
  $im->Set(size=>$imagesize . 'x' . $imagesize);
  $im->Read('xc:white');
  
  # Create the flower images
  $pink_flower = new Image::Magick;
  $pink_flower->Read('pink-flower.gif');
  
  $red_flower = new Image::Magick;
  $red_flower->Read('red-flower.gif');
  
  $yellow_flower = new Image::Magick;
  $yellow_flower->Read('yellow-flower.gif');
  
  # Turtle:  the midpoint of the bottom edge of the image, pointing up.
  $turtle=new Turtle($imagesize/2, $imagesize, 0, 1);
}

sub lsys_execute
{
  my ($string, $repetitions, $filename, %rule) = @_;

  my ($command);

  # Apply the %rule to $string, $repetitions times.
  for (1..$repetitions)
  {
    $string =~ s/./defined ($rule{$&}) ? $rule{$&} : $&/eg;
  }
  foreach $command (split(//, $string))
  {
    if ($translate{$command}) { &{$translate{$command}}(); }
  }
  $im->Write($filename);
  $im->Write('win:');
}

1;
