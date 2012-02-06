#!/usr/bin/perl
#
# Demonstration of some of the fancier Image Composition Methods
# including the 'rotate' parameter specific to PerlMagick Composite()
#
# NOTE: versions of IM older than IM v6.5.3-4 will need to rename the
# parameter  "args=>"   to  the mis-named "blend=>" parameter.
#
# Also not that "composite -watermark" is actually known as the compose
# method "Modulate".
#
# Essentually each image is equivelent to
#   convert logo: -crop 80x80+140+60 +repage \
#           -size 60x60 gradient:black-white \
#           -alpha set miff:- |\
#    composite -  -geometry +10+10 -virtual-pixel gray \
#              -dissolve 70x30   show:
# for various composition methods.
#
use strict;
use Image::Magick;

# Background or Destination image
my $dest=Image::Magick->new();
$dest->Read('logo:');
$dest->Crop('100x100+400+100');  # wizards hat
$dest->Set(page=>'0x0+0+0');
$dest->Set(alpha=>'Set');

# Source, Composite or Overlay image
my $src=Image::Magick->new();
$src->Set(size=>'80x80');
$src->Read('gradient:black-white');
$src->Set(alpha=>'Set');

my $offset="+10+10";

# Circle Mask Image (same size as Destination)
my $circle=Image::Magick->new();
$circle->Set(size=>'80x80');
$circle->Read('xc:black');
$circle->Draw(fill=>'white',primitive=>'circle',points=>'39.5,39.5 10,39.5');

my $texture=Image::Magick->new();
$texture->Read('pattern:checkerboard');

# List of images generated
my $results=Image::Magick->new();

# Working copy of Destination Image
my $clone;

# ----------------------------------------
# Normal Composition Methods

$clone=$dest->Clone();
$clone->Label('Over\n(normal compose)');
$clone->Composite(
  image=>$src,
  compose=>'over',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Multiply\n(add black)');
$clone->Composite(
  image=>$src,
  compose=>'multiply',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Screen\n(add white)');
$clone->Composite(
  image=>$src,
  compose=>'screen',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('HardLight\n(light effects)');
$clone->Composite(
  image=>$src,
  compose=>'hardlight',
  geometry=>$offset,
);
push(@$results, $clone);

# ---------------
# Masked and Blending Demonstartion

$clone=$dest->Clone();
$clone->Label('Circle Masked\n(three image)');
$clone->Composite(
  image=>$src,
  mask=>$circle,
  compose=>'over',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Blend 50x50\n(50% plus 50%)');
$clone->Composite(
  image=>$src,
  compose=>'blend',
  args=>'50x50',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Dissolve 50x50\n(50% over 50%)');
$clone->Composite(
  image=>$src,
  compose=>'dissolve',
  args=>'50x50',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Dissolve 50\n(50% over 100%)');
$clone->Composite(
  image=>$src,
  compose=>'dissolve',
  args=>'50',
  geometry=>$offset,
);
push(@$results, $clone);

# ---------------
# Displacement Demonstartion

$clone=$dest->Clone();
$clone->Label('Displace 50x0\n(displace horiz)');
$clone->Set('virtual-pixel'=>'gray');
$clone->Composite(
  image=>$src,
  compose=>'displace',
  args=>'50x0',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Displace 0x50\n(compress vert)');
$clone->Set('virtual-pixel'=>'gray');
$clone->Composite(
  image=>$src,
  compose=>'displace',
  args=>'0x50',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Displace 50x50\n(diagonal)');
$clone->Set('virtual-pixel'=>'gray');
$clone->Composite(
  image=>$src,
  compose=>'displace',
  args=>'50x50',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Displace 0,-80\n(displace flip)');
$clone->Set('virtual-pixel'=>'gray');
$clone->Composite(
  image=>$src,
  compose=>'displace',
  args=>'0,-80',
  geometry=>$offset,
);
push(@$results, $clone);

# ---------------
# Demonstrate rotation
# note that offset is automatically adjusted to keep rotated image
# centered relative to its '0' rotation position

$clone=$dest->Clone();
$clone->Label('Rotate 0\n');
$clone->Composite(
  image=>$src,
  compose=>'over',
  rotate=>0,
  background=>'none',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Rotate 10\n');
$clone->Composite(
  image=>$src,
  compose=>'over',
  rotate=>10,
  background=>'none',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Rotate 45\n');
$clone->Composite(
  image=>$src,
  compose=>'over',
  rotate=>45,
  background=>'none',
  geometry=>$offset,
);
push(@$results, $clone);

$clone=$dest->Clone();
$clone->Label('Rotate 90\n');
$clone->Composite(
  image=>$src,
  compose=>'over',
  rotate=>90,
  background=>'none',
  geometry=>$offset,
);
push(@$results, $clone);

# ----------------------------------------
# Output the changed pixels

# to every image underlay a checkboard pattern
# so as to show if any transparency is present
for my $image ( @$results ) {
  $image->Composite(
    image=>$texture,
    tile=>'True',
    compose=>'DstOver',
  );
}

my $montage=$results->Montage(
  geometry=>'+10+10',
  tile=>'4x',
  frame=>'6x6+2+2',
  shadow=>'True',
);
$montage->Write('show:');
$montage->Write('compose_specials.jpg');

