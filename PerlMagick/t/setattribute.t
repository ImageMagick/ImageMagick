#!/usr/bin/perl
#  Copyright 1999 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
# Test setting & getting attributes.
#
BEGIN { $| = 1; $test=1, print "1..71\n"; }
END {print "not ok 1\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';

# Determine if QuantumMagick is defined
$image=Image::Magick->new;
my $depth = $image->Get('depth');

testSetAttribute('input.miff','adjoin','True');

++$test;
testSetAttribute('input.miff','adjoin','False');

++$test;
testSetAttribute('input.miff','antialias','True');

++$test;
testSetAttribute('input.miff','antialias','False');

++$test;
testSetAttribute('input.miff','compression','None');

++$test;
testSetAttribute('input.miff','compression','JPEG');

++$test;
testSetAttribute('input.miff','compression','LZW');

++$test;
testSetAttribute('input.miff','compression','RLE');

++$test;
testSetAttribute('input.miff','compression','Zip');

++$test;
testSetAttribute('input.miff','density','72x72');

++$test;
testSetAttribute('input.miff','dispose','Undefined');

++$test;
testSetAttribute('input.miff','dispose','None');

++$test;
testSetAttribute('input.miff','dispose','Background');

++$test;
testSetAttribute('input.miff','dispose','Previous');

++$test;
testSetAttribute('input.miff','delay',100);

++$test;
testSetAttribute('input.miff','dither','True');

++$test;
testSetAttribute('input.miff','dither','False');

++$test;
testSetAttribute('input.miff','display','bogus:0.0');

++$test;
testSetAttribute('input.miff','filename','bogus.jpg');

++$test;
testSetAttribute('input.miff','font',q/-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-*/);

++$test;
testSetAttribute('input.miff','iterations',10);

++$test;
testSetAttribute('input.miff','interlace','None');

++$test;
testSetAttribute('input.miff','interlace','Line');

++$test;
testSetAttribute('input.miff','interlace','Plane');

++$test;
testSetAttribute('input.miff','interlace','Partition');

++$test;
testSetAttribute('input.miff','loop',100);

++$test;
testSetAttribute('input.miff','magick','TIFF');

++$test;
testSetAttribute('input.miff','monochrome','True');

++$test;
testSetAttribute('input.miff','monochrome','False');

++$test;
testSetAttribute('input.miff','page','595x842+0+0');

++$test;
testSetAttribute('input.miff','pointsize',12);

++$test;
testSetAttribute('input.miff','preview','Rotate');

++$test;
testSetAttribute('input.miff','preview','Shear');

++$test;
testSetAttribute('input.miff','preview','Roll');

++$test;
testSetAttribute('input.miff','preview','Hue');

++$test;
testSetAttribute('input.miff','preview','Saturation');

++$test;
testSetAttribute('input.miff','preview','Brightness');

++$test;
testSetAttribute('input.miff','preview','JPEG');

++$test;
testSetAttribute('input.miff','preview','Spiff');

++$test;
testSetAttribute('input.miff','preview','Dull');

++$test;
testSetAttribute('input.miff','preview','Grayscale');

++$test;
testSetAttribute('input.miff','preview','Quantize');

++$test;
testSetAttribute('input.miff','preview','Despeckle');

++$test;
testSetAttribute('input.miff','preview','ReduceNoise');

++$test;
testSetAttribute('input.miff','preview','AddNoise');

++$test;
testSetAttribute('input.miff','preview','Sharpen');

++$test;
testSetAttribute('input.miff','preview','Blur');

++$test;
testSetAttribute('input.miff','preview','Threshold');

++$test;
testSetAttribute('input.miff','preview','EdgeDetect');

++$test;
testSetAttribute('input.miff','preview','Spread');

++$test;
testSetAttribute('input.miff','preview','Solarize');

++$test;
testSetAttribute('input.miff','preview','Shade');

++$test;
testSetAttribute('input.miff','preview','Raise');

++$test;
testSetAttribute('input.miff','preview','Segment');

++$test;
testSetAttribute('input.miff','preview','Solarize');

++$test;
testSetAttribute('input.miff','preview','Swirl');

++$test;
testSetAttribute('input.miff','preview','Implode');

++$test;
testSetAttribute('input.miff','preview','Wave');

++$test;
testSetAttribute('input.miff','preview','OilPaint');

++$test;
testSetAttribute('input.miff','preview','Charcoal');

++$test;
testSetAttribute('input.miff','quality',25);

++$test;
testSetAttribute('input.miff','scene',5);

++$test;
testSetAttribute('input.miff','subimage',9);

++$test;
testSetAttribute('input.miff','subrange',16);

++$test;
testSetAttribute('input.miff','server','mymachine:0.0');

++$test;
testSetAttribute('input.miff','size','25x25');

++$test;
testSetAttribute('input.miff','size','25x25');

# I have no idea what this does
++$test;
testSetAttribute('input.miff','tile','some value');

++$test;
testSetAttribute('input.miff','texture','granite:');

++$test;
testSetAttribute('input.miff','verbose','True');

++$test;
testSetAttribute('input.miff','verbose','False');

