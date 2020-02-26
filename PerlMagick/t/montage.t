#!/usr/bin/perl
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
  q/background=>'#696e7e'/,
  '6f8304c88f6dae4ade2a09fb7b4562cf2c09978cb30b025afaf9bd5805a4e75e');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'Center'/,
  '4fa94e68dc41fa0fda3469e6b8ebc4c07be6ef656405a1ea8f9ed8b00aa07552');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  '24adcfa00f2c69d10fda815c9572bd7f621adb93a58a9c3c85657060929da4d0');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'North'/,
  '70bb1be5bad21d5dd545ffe62eeb90052a037fcc1613aaa3f3b3c4d4c6138e8a');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  '116668aedcc616527dabac2c07ea597d5cc26e3aaf33db14d8ce796892bc1f1c');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'West'/,
  'abb3104224d07ba50235e750c8009b231a0e6aaa596fb2ec3703b2061a59abc3');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'East'/,
  '8723423ac9151136571b6e8f053fc60b09a8ec18d94e4fbc29c492de032c9a7d');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  'abc713cef6d9923859854c164b83ed7b92f6c15ae6924207bb8a38063395650b');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'South'/,
  '9992d5631703aa64d4fb87c71881df39aeff1cc821320d0a43f00897e7cd6a17');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  'ae4322daebfbd3fe448afffe33aa599be518eaebb3515793e1efe6dc33939383');

#
# 11) Test Framed Montage
#
# Image border color 'bordercolor' controls frame background color
# Image matte color 'mattecolor' controls frame color
# Image pen color 'pen' controls label text foreground color
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+3+3>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '40835871b13232243ba4d2bcb244d06958556a24a9624e793f5c4c291d64f322',
  '531d0d485d55a5b585cab3f64b37da81a109fbaa91aa44a038fa916421780f14',
  '40835871b13232243ba4d2bcb244d06958556a24a9624e793f5c4c291d64f322');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  'c11aad0132d57f84ec42aad600f2c9f3fdb9ad5a64235cb08a296c0f1a509f6d',
  '51b8db221299cea2bea84b11247bfa4b41a0cc3a9af27ff4c20f9c23ee7f2117',
  '7b8d2dbf3ee22440fd5df36090922bb599ab6b4df216282cc51bb4328d03a211');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  '292b115e08fcef4888ee3f386c6c3aff1c0ed008e34697435c1e4116a75149ed',
  'e2fe0b56decf6fd791813e99d7b0f40646a479589e7519d97e2f92969dd17a1e',
  '2c6bc96010beb38d123cc712ded96ecbbef40265a80e9638e9ea9034d64227f3');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  'b9df49be9b056ed8fd4c4eb7621aa073df65740e4aaebc038c294cb3731f334f');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '9d7d0f1886ef8d48f680567163d3a4583897d114c7abbb9da6a98f96fb629ebb');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/ tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '9d7d0f1886ef8d48f680567163d3a4583897d114c7abbb9da6a98f96fb629ebb');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '9f6f0c7ff76283b0f8747c1e12bd810bda0c47ab8cf78a4ae584556a64c88213');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  'dfede51035a4ce942e3ba9909f85eb497c134a60bb8363afded0ae81f73f0e46');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  'dfede51035a4ce942e3ba9909f85eb497c134a60bb8363afded0ae81f73f0e46');
