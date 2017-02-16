#!/usr/bin/perl
#  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
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
  q/background=>'#696e7e'/,
  'd29f905749426e5491bbd8c4f58b7c14dc1a72250632b7e7b542910e0cbb7c77');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'Center'/,
  '21ab4077722e45db67c945b2364cfc1365d3f939d57fc6fb2c7b77f161381c2e');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  'a518ad6aa1c3ad2b117864525d610481c1b3b9ead2c624c773fb427f85ef897b');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'North'/,
  '4126725283eb699545d755a6b30c5fc6cee9ac16a1e8310cdbbedcbd054bebf6');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  'e144c8d0da144864063753903838a286d46da56ca9ea0ce0b6080f84c6feac6c');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'West'/,
  '67286cf1ed572e836f6b67c5d733a039bba9e6a7ef18d25397e46c17223b206f');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'East'/,
  'decf855bf26dfc3f7401769a09e302d8bac53cba6c27fd456cdfbc35e494be01');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  '848fda241866b1a191ab4794b42272b5756dbeacac5b58ef5227a34a8c72817d');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'South'/,
  'f5cccb291b3bea8cb62e5cd7a6c8e154eedfd8e12c20409a8e663231e62411c4');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  'a19748d933fefe536b29b8ba98ce0024e3f2dc98102a1f56307a229bc3042f1c');

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
  'f0b9673408f0aa10549c3a54cc5baa8061a7a1dd57b2199dd887ce9447702644',
  'f0b9673408f0aa10549c3a54cc5baa8061a7a1dd57b2199dd887ce9447702644',
  '72a7587a058c71cd62226e59b64c335190a37d7dd990d68534cf05df61084783');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  '60ebc16a46ef5addda72c3f0776257af52eba009d6fb2331d10c83c5c26dd5da',
  '60ebc16a46ef5addda72c3f0776257af52eba009d6fb2331d10c83c5c26dd5da',
  '9fe18e11ac759d3a86362f16d0bdee4b6952d617daa3ad2c3511e19050dda71d');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  '71e7b38629ccddcb46dc5b37e62fa08cf4bec2cc28507f8375638aab30607616');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '245f9fbee901f99e08458aad797ade92ea17e68b230212510ad2eca96e169e39',
  '245f9fbee901f99e08458aad797ade92ea17e68b230212510ad2eca96e169e39',
  'ab07c5591848f1aa8bfffc46c094be40cf662e18f68b19229f1f7e9d9eec4692');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '611833bd0e13681a82d34c4c40b2a263bf245ac879de240114df874f9eaca7f0',
  '611833bd0e13681a82d34c4c40b2a263bf245ac879de240114df874f9eaca7f0',
  '65bb1cd11a0dbbd90ca46b07ce58b2ba5fcfadc0869356e5a08eda53510e0126');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/ tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '611833bd0e13681a82d34c4c40b2a263bf245ac879de240114df874f9eaca7f0',
  '611833bd0e13681a82d34c4c40b2a263bf245ac879de240114df874f9eaca7f0',
  '65bb1cd11a0dbbd90ca46b07ce58b2ba5fcfadc0869356e5a08eda53510e0126');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '49bf08b9ae34c7c8acee15b123c29e414ae6fb9bdf2a15dbdd3a33a753b8e7ff',
  '49bf08b9ae34c7c8acee15b123c29e414ae6fb9bdf2a15dbdd3a33a753b8e7ff',
  'c42d5f2bef5f30167f02d6ba85504ee82e06c16d9404bc6fc0f6e75c0f8f53e9');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  'b46f6d6c6305ab1d5f05f89030e775fca2f01dd5d40a8340372bd31dafaa4e83');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  'b46f6d6c6305ab1d5f05f89030e775fca2f01dd5d40a8340372bd31dafaa4e83',
  'b46f6d6c6305ab1d5f05f89030e775fca2f01dd5d40a8340372bd31dafaa4e83',
  '6c61a8ad7be5f8c368dcb591f766349e648b623cbc41e6699ba3ea53ef182510');
