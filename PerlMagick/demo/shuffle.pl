#!/usr/bin/perl
#
# Shuffle image sequence.
#
use strict;
use warnings;
use Image::Magick;
use List::Util qw(shuffle);

# Instantiate a new Image::Magick object
my $image = Image::Magick->new();

# Read the built-in images into the sequence
my $status = $image->Read('model.gif', 'red-flower.gif', 'tile.gif',
  'pink-flower.gif', 'smile.gif', 'yellow-flower.gif');
die "Error reading images: $status" if "$status";

# Extract the image sequence into a native Perl array, 
# shuffle it randomly, clear the object, and push them back.
my @images = @$image;
@$image = ();
push(@$image, shuffle(@images));

# Write the shuffled image sequence
print "Write...\n";
foreach my $img (@$image) {
  $status = $img->Resize(geometry => '320x320');
  die "Error resizing image: $status" if "$status";
}
$image->Write('ashlar:shuffle.pam');
print "Display...\n";
my $shuffle = Image::Magick->new();
my $status = $shuffle->Read('shuffle.pam');
$shuffle->Write(magick=>'SHOW',title=>"Shuffle");

