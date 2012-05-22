#!/usr/bin/perl
#  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
# Test montage method.
#
BEGIN { $| = 1; $test=1, print "1..19\n"; }
END {print "not ok 1\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';

#
# 1) Test montage defaults (except no label that requires an exact font)
#
testMontage( q//,
  q/background=>'#696e7e', label=>''/,
  'bf4dd13fca96be6b3605e8cfdf65d3d09485d2c063bd88d967432836723be297');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'Center'/,
  'd67f32498d2d172813a37cba59566bc6ad0cbb22813a7d94b3ccaa4f183b611a');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  '9a5b3dfc243f7d9e5abb2de0043679aa95a620313f0723d3297fad569b1e65ec');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'North'/,
  'ec73186569d5d922ebd48a244172beae71c6edd4fd3bec741519650d6244b829');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  '4b6de9121b3996c4b9ced91a3a4bda412456da17fc46e21e1bfa7f230df4660e');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'West'/,
  'f2fc5232bf3a756fd93e7ea24de0f01764d218d80da970a0479fc01297d4b07f');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'East'/,
  '2fdc1ceba82a853a02a55fc57eb1c3da9183fdf0d34cdd2deada0bd40329c1e1');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  '2b151fd9ed8192c326f5917f2cb9621fbb66fbac585ded776625d29a2bb1d690');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'South'/,
  'afba8a6b6f06f446cff8a0c819eaa11683eaf8f16b3a2c89b33de3f388a6563f');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  '6440e72ddadf6d84cac94f8c722adb406bb3e41d6ae1af04707f6f61dfd5334e');

#
# 11) Test Framed Montage
#
# Image border color 'bordercolor' controls frame background color
# Image matte color 'mattecolor' controls frame color
# Image pen color 'pen' controls label text foreground color
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+3+3>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  'f303eb837e796f3ecf69bc97a8067804113d5c54420d1e3af76f5099d3808b9e',
  'f303eb837e796f3ecf69bc97a8067804113d5c54420d1e3af76f5099d3808b9e',
  '5672f2813d7e032fa4306c0d501d1938dbf56ccb2daeab555b2f9a4e0395e149');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  '8dd9e9293ea3567a6b66ace613aa53d855120369c118a490075253b870458a7f',
  '8dd9e9293ea3567a6b66ace613aa53d855120369c118a490075253b870458a7f',
  '27b154dd3ce8392e2d0581179f51ce3d08abcd30854125a87f791ab46fba64d5');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  '8dd9e9293ea3567a6b66ace613aa53d855120369c118a490075253b870458a7f');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  'ec51611e630b07b472f4acfbfb102892bfa79b0708356b88f59aeb7a4dc2fae5',
  'ec51611e630b07b472f4acfbfb102892bfa79b0708356b88f59aeb7a4dc2fae5',
  '70853287289b478e27e4b5cc634b9d51efcee2cf1af5805323a3723a276acdfb');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'bfa74ec158d85faeb57feda41da3d5405f5b52f3a96a58942df474cafbc27415',
  'bfa74ec158d85faeb57feda41da3d5405f5b52f3a96a58942df474cafbc27415',
  '3e45e0316f3013dcae5c56de2f31a40e7bb523a607da7d82588b8f2c9283c43a');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'bfa74ec158d85faeb57feda41da3d5405f5b52f3a96a58942df474cafbc27415',
  'bfa74ec158d85faeb57feda41da3d5405f5b52f3a96a58942df474cafbc27415',
  '3e45e0316f3013dcae5c56de2f31a40e7bb523a607da7d82588b8f2c9283c43a');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '123c0dd02a97643fae81450adb10f5f4f1109a74e938083ce17df11f3a8e68a6',
  '123c0dd02a97643fae81450adb10f5f4f1109a74e938083ce17df11f3a8e68a6',
  '2e6bb7aeaf2c6f01c7a26105649bfada3b68508355c3433c2d6c3550e4f2d988');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  '1967a3499a09a58d27acdabb13bd5d97d0f29fee91b5cc8ee8445bdc9940f677');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  '1967a3499a09a58d27acdabb13bd5d97d0f29fee91b5cc8ee8445bdc9940f677',
  '1967a3499a09a58d27acdabb13bd5d97d0f29fee91b5cc8ee8445bdc9940f677',
  '4ecd63033a7b15dbb7391e2781058863e69240be500784c24f6026acede57889');
