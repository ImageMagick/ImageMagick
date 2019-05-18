#!/usr/bin/perl
#  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
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
  'ad77fef2815f06678757ba79d14f752cfb639f078b32b54122714e25ede2bc5e');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'Center'/,
  '030de7b70860f11a66d7081df97a96d311641fc69f48aafdbde27da53ac61277');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  'd139d942510d9ca5762799acad940b7539e56f9610ab57a26e22c18fe35a01b5');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'North'/,
  '5f02d04d24bfbeb2dc82a056cf595ec8ef1601a0e68aaae541c116cc7c627916');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  'd1e9ee7a612ef19b360c923795a0f0c7bc819983475adf6787e39ee8167f75f1');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'West'/,
  '084e12bf9e3325140d8df2fb52ac481aff408bb3ed1940d379c14d1e07b58599');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'East'/,
  'ff8cee733142654135e8baebaf5e036eaf5de1d419476c0d0a8bee9c88370066');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  'e7109f6f3291cb0d576a79cb70e7b3db6dde851f1cfc6e5d63fbbf96100264bb');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'South'/,
  '287fc09873957d91ef2a29cf09c7d20a477aaab2104c3ba9fd6046ea0ee633de');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  '7f2745dfeb6b94f059e521601cd41cda2d548b7fff5eabd4781b102c2581c4f4');

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
  'd65a3fcf9119a59414d52b6b11c0d131579341dbae03e4f18776dc8a01da2931',
  '2aa66c8d3eb0c3b3e765df6ffc5977c026dd65f7839b763a48192cdd45d27474');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  'ff43a8c5dd980438c10436a71b47afc538ae3839ba57251768fa07add1229d8a',
  '885d043b63810d030697104629d79f2141fab28cb561cf84b4f323209cee4f42');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  '0de019cb1e17fb9d3e60e6ae3931c6d384859613c3fc930cfef46720cae8237e',
  '4774caea304139483a3f50a8b94690c7503ccbea89c0ae698e8bb8996d0cfda8');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '1ac572b14256896360176430b0e571dd53bfa083214a0ef35069b7a4a1e56b2d');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '08ce60c0e00ab7415c181a3fefcf2cb7e42d81aa4c630559d11073a4bcb95cad');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/ tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '08ce60c0e00ab7415c181a3fefcf2cb7e42d81aa4c630559d11073a4bcb95cad');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '6de83e358b3b0939d0a0cb196514de31f4ded673f867e155b63f03c7e04e8170');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  '8d857590f8f13bd4a893bbee811c3e801a97a6f6a98e8735b97c6812f1e35b11');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  '8d857590f8f13bd4a893bbee811c3e801a97a6f6a98e8735b97c6812f1e35b11');
