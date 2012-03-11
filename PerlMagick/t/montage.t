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
  '4aac8269bd31defb14971e475d23e0774c5fc929a60babddf8c3ecbcb01bfabd');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'Center'/,
  'a1825dab28dcb3f3fad641a2ec688199ad4c43253f5ea3fb13fafffe49a74818');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  'd41d7fb9cf59ab0d8f86545b4561fd267f555c0ab434a23d9b4d52e7b55fe64b');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'North'/,
  '0c04c47f966d7769e99f99b0be4d8d1679e42a71543ce24efd2296afa9491073');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  '7110c0432bfd90f2173e547dbefe4c693757e7bf6bf731c8a97f72c71c723830');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'West'/,
  '2a6f10fc376136ec74d53d7dd52c60e420edf851548377868bfb7f47d0e8a1e6');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'East'/,
  '6a1517b10841950912b5a4e9fb852559268a3c64e86d6b391024c0e3ea7cc3e3');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  '6df59cabd0ccb84da56cf4a13516cfd8fc0bc215c438c8ee5bdbe6db82d52830');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'South'/,
  '227064c0fdd574d6c659a71bc24591d67fd604b4bb44c5466774446d9c7592f1');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  '1b1eadf70f68f9ddea212b75d68d3109fcafe4e7db988cb90e148ca33be4691f');

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
  'a2b51a92bfa701b5121930caba4c39fca5de2d5d539570667b6a14aed1a1d0bd',
  'a2b51a92bfa701b5121930caba4c39fca5de2d5d539570667b6a14aed1a1d0bd',
  '5672f2813d7e032fa4306c0d501d1938dbf56ccb2daeab555b2f9a4e0395e149');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  'e4e2f293481151f49888cca594b38beaa37e168ba1efa6a590e166521490b338',
  'e4e2f293481151f49888cca594b38beaa37e168ba1efa6a590e166521490b338',
  '27b154dd3ce8392e2d0581179f51ce3d08abcd30854125a87f791ab46fba64d5');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  'e4e2f293481151f49888cca594b38beaa37e168ba1efa6a590e166521490b338');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '2b108aac04c0d339dbae55161a050dd788d2fd09a35f2ad5a0810b7c546ada44',
  '2b108aac04c0d339dbae55161a050dd788d2fd09a35f2ad5a0810b7c546ada44',
  '70853287289b478e27e4b5cc634b9d51efcee2cf1af5805323a3723a276acdfb');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'eb2db2e5e0558563d814da22c8ca9be993a3217744eaba53d6921b223c6e6f3f',
  'eb2db2e5e0558563d814da22c8ca9be993a3217744eaba53d6921b223c6e6f3f',
  '3e45e0316f3013dcae5c56de2f31a40e7bb523a607da7d82588b8f2c9283c43a');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'eb2db2e5e0558563d814da22c8ca9be993a3217744eaba53d6921b223c6e6f3f',
  'eb2db2e5e0558563d814da22c8ca9be993a3217744eaba53d6921b223c6e6f3f',
  '3e45e0316f3013dcae5c56de2f31a40e7bb523a607da7d82588b8f2c9283c43a');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '26ba497f978db17009459907d6634abe0084f4dc26f2edc4435e15087a5a7918',
  '26ba497f978db17009459907d6634abe0084f4dc26f2edc4435e15087a5a7918',
  '2e6bb7aeaf2c6f01c7a26105649bfada3b68508355c3433c2d6c3550e4f2d988');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  '3f42ab1a8a19594ab6a0c7990a35a34d92d9fff2a4ed59c5202a660d27c3ef36');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  '3f42ab1a8a19594ab6a0c7990a35a34d92d9fff2a4ed59c5202a660d27c3ef36',
  '3f42ab1a8a19594ab6a0c7990a35a34d92d9fff2a4ed59c5202a660d27c3ef36',
  '4ecd63033a7b15dbb7391e2781058863e69240be500784c24f6026acede57889');
