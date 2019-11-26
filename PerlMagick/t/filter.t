#!/usr/bin/perl
#
#  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
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
#  Test image filters.
#
BEGIN { $| = 1; $test=1, print "1..58\n"; }
END {print "not ok 1\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';
use FileHandle;
autoflush STDOUT 1;
autoflush STDERR 1;

$fuzz=int(0.05*(Image::Magick->new()->QuantumRange));

testFilterCompare('input.miff',  q//, 'reference/filter/AdaptiveThreshold.miff', 'AdaptiveThreshold', q/'5x5+5%'/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Annotate.miff', 'Annotate', q/text=>'Magick',geometry=>'+0+20',font=>'Generic.ttf',fill=>'gold',gravity=>'North',pointsize=>14/, 0.05, 1.00);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Blur.miff', 'Blur', q/'5x2'/, 0.007, 0.7);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Border.miff', 'Border', q/geometry=>'6x6',color=>'gold'/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Channel.miff', 'Channel', q/channel=>'red'/, 0.2, 0.8);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Chop.miff', 'Chop', q/geometry=>'80x80+5+10'/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Charcoal.miff', 'Charcoal', q/'0x1'/, 0.3, 1.01);
++$test;

testFilterCompare('input.miff', "fuzz=>$fuzz", 'reference/filter/ColorFloodfill.miff', 'ColorFloodfill', q/geometry=>"+25+45"/, 0.15, 1.0);
++$test;

testFilterCompare('input.miff', "fuzz=>$fuzz", 'reference/filter/Colorize.miff', 'Colorize', q/fill=>"red", blend=>"50%"/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Contrast.miff', 'Contrast', q//, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Convolve.miff', 'Convolve', q/[0.0625, 0.0625, 0.0625, 0.0625, 0.5, 0.0625, 0.0625, 0.0625, 0.0625]/, 0.1, 0.7);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Crop.miff', 'Crop', q/geometry=>'80x80+5+10'/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Set.miff', 'Set', q/page=>'0x0+0+0'/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Despeckle.miff', 'Despeckle', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Draw.miff', 'Draw', q/fill=>'none',stroke=>'gold',primitive=>'circle',points=>'60,90 60,120',strokewidth=>2/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Edge.miff', 'Edge', q//, 0.31, 1.01);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Emboss.miff', 'Emboss', q/'0x1'/, 0.2, 1.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Equalize.miff', 'Equalize', q//, 0.06, 0.5);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Implode.miff', 'Implode', q/0.0/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Flip.miff', 'Flip', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Flop.miff', 'Flop', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Frame.miff', 'Frame', q/'15x15+3+3'/, 0.02, 0.5);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Gamma.miff', 'Gamma', q/1.6/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/GaussianBlur.miff', 'GaussianBlur', q/'0.0x1.5'/, 0.07, 0.9);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Implode.miff', 'Implode', q/0.0/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Level.miff', 'Level', q/'20%x'/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Magnify.miff', 'Magnify', q//, 0.003, 0.3);
++$test;

testFilterCompare('input.miff', "fuzz=>$fuzz", 'reference/filter/MatteFloodfill.miff', 'MatteFloodfill', q/geometry=>"+25+45"/, 0.25, 1.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/MedianFilter.miff', 'MedianFilter', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Minify.miff', 'Minify', q//, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Modulate.miff', 'Modulate', q/brightness=>110,saturation=>110,hue=>110/, 0.05, 0.5);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/QuantizeMono.miff', 'Quantize', q/colors=>256/, 0.2, 0.7);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/MotionBlur.miff', 'MotionBlur', q/'0x13+10-10'/, 0.002, 0.04);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Negate.miff', 'Negate', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Normalize.miff', 'Normalize', q//, 0.02, 0.2);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/OilPaint.miff', 'OilPaint', q//, 0.03, 1.0);
++$test;

testFilterCompare('input.miff', "fuzz=>$fuzz", 'reference/filter/Opaque.miff', 'Opaque', q/color=>"#e23834", fill=>"green"/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Quantize.miff', 'Quantize', q//, 0.2, 0.7);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/RadialBlur.miff', 'RadialBlur', q/10/, 0.004, 0.4);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Raise.miff', 'Raise', q/'10x10'/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/ReduceNoise.miff', 'ReduceNoise', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Resize.miff', 'Resize', q/'60%'/, 0.00007, 0.07);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Roll.miff', 'Roll', q/geometry=>'+20+10'/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Rotate.miff', 'Rotate', q/45/, 0.00004, 0.04);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Sample.miff', 'Sample', q/'60%'/, 0.006, 0.6);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Scale.miff', 'Scale', q/'60%'/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Segment.miff', 'Segment', q//, 0.09, 0.9);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Shade.miff', 'Shade', q/geometry=>'30x30',gray=>'true'/, 0.09, 0.9);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Sharpen.miff', 'Sharpen', q/'5x2'/, 0.1, 1.001);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Shave.miff', 'Shave', q/'10x10'/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Shear.miff', 'Shear', q/'-20x20'/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/SigmoidalContrast.miff', 'SigmoidalContrast', q/"3x50%"/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Solarize.miff', 'Solarize', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Swirl.miff', 'Swirl', q/90/, 0.00001, 0.004);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Threshold.miff', 'Threshold', q/90%/, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Trim.miff', 'Trim', q//, 0.0, 0.0);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/UnsharpMask.miff', 'UnsharpMask', q/'5x2+1'/, 0.004, 0.4);
++$test;

testFilterCompare('input.miff',  q//, 'reference/filter/Wave.miff', 'Wave', q/'25x150'/, 0.00001, 0.004);
++$test;

1;
