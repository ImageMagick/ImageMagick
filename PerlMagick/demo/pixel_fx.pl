#!/usr/bin/perl
#
# Example of modifying all the pixels in an image (like -fx).
#
# Currently this is slow as each pixel is being one one by one. The better
# technique of extracting and modifing a whole row of pixels at a time has not
# been figured out, though functions are provided for this.
#
# Also access and controls for Area Re-sampling (EWA), beyond single pixel
# lookup (interpolated unscaled lookup), is also not available at this time.
#
# Anthony Thyssen   5 October 2007
#
use strict;
use Image::Magick;

# read original image
my $orig = Image::Magick->new();
my $w = $orig->Read('rose:');
warn("$w")  if $w;
exit  if $w =~ /^Exception/;


# make a clone of the image for modifications
my $dest = $orig->Clone();

# You could enlarge destination image here if you like.
# And it is posible to modify the existing image directly
# rather than modifying a clone as FX does.

# Iterate over destination image...
my ($width, $height) = $dest->Get('width', 'height');

for( my $j = 0; $j < $height; $j++ ) {
  for( my $i = 0; $i < $width; $i++ ) {

    # read original image color
    my @pixel = $orig->GetPixel( x=>$i, y=>$j );

    # modify the pixel values (as normalized floats)
    $pixel[0] = $pixel[0]/2;      # darken red

    # write pixel to destination
    # (quantization and clipping happens here)
    $dest->SetPixel(x=>$i,y=>$j,color=>\@pixel);
  }
}

# display the result (or you could save it)
$dest->Write('win:');
$dest->Write('pixel_fx.gif');

