#!/usr/bin/perl
#
# Methods for to Get and Set single pixels in images using PerlMagick
#
use strict;
use Image::Magick;

# read image
my $im=Image::Magick->new();
$im->Read('logo:');

# ---

# Get/Set a single pixel as a string
my $skin=$im->Get('pixel[400,200]');
print "Get('pixel[x,y]') = ", $skin, "\n";

$im->Set('pixel[1,1]'=>'0,0,0,0');
$im->Set('pixel[2,1]'=>$skin);
$im->Set('pixel[3,1]'=>'green');
$im->Set('pixel[4,1]'=>'rgb(255,0,255)');

# ---

# More direct single pixel access
my @pixel = $im->GetPixel( x=>400, y=>200 );
print "GetPixel() = ", "@pixel", "\n";

# modify the pixel values (as normalized floats)
$pixel[0] = $pixel[0]/2;      # darken red value
$pixel[1] = 0.0;              # junk green value
$pixel[2] = 0.0;              # junk blue value

# write pixel to destination
# (quantization and clipping happens here)
$im->SetPixel(x=>5,y=>1,color=>\@pixel);

# ---

# crop, scale, display the changed pixels
$im->Crop(geometry=>'7x3+0+0');
$im->Set(page=>'0x0+0+0');
$im->Scale('1000%');

# Output the changed pixels
$im->Write('single-pixels.pam');
$im->Write(magick=>'SHOW',title=>"Single Pixel");
