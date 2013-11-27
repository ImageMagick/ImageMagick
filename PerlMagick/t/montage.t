#!/usr/bin/perl
#  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
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
  '977b812cb06e16d0ee7da6c80a9470cb85c2d33ba33f5919f26b05058f0eb177',
  'fea8915dc67163d86e93a6e611c900185f82bdc5fc67c7a3e53a4b76639aaa56',
  '8de04216e6f60ad3bf63d5a2c54386cb824579e1c4a677af726e0306e0c7fb09');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'Center'/,
  '9a3f16a11fa4173f43642e7759070b32ce66a21c72312a7c99ed5dab711dc377',
  '633267a42b30c1a5709d8929ef9ad24e6612f28e10299315a0ff386ccba7ec9a',
  '2a381bc2cc7dc751c95df0f6a88c5f04232e82d06094b65b3909f488e4347f93');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  '1ace55dbec2814d4d12e911f6ab21e0eac769cf12fbdcf516376f356222b74e1',
  '9d4854b04b3072c4f371a55289ed0583c5fab1a52758e514008462bfb890be41',
  'f3be8f6f4ecabc14d6da8a3d8b6a2e369ad005d045f381df6f24ca2410406380');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'North'/,
  '02840dc5ac270f2e26422b8c90c5e5c3af81fd6e6739e9554977115e152a6872',
  '67ee49456bfc3500aac4a2425b7b45c9cd32d8948232a97fabe62713ddd7f859',
  'abd7f530c44e953ac799a04846a3eef613d0ab7b08aa92bb6cabe8abf275d5b6');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  'c5a10e80cb4f3b1c866d03e8cad4154f1139758ec86ffeef4fe2a802ac118ae2',
  '66b8ac0676dbf6c48703cb16422e75174ee676003cacfd58de546a697075b469',
  'bb35ddfaab468d869f2a3bafb16b3751015f2bb94e87438c48c7b294831d2bb8');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'West'/,
  '60f394bc620fba50b97dfe55c6026eed616a26ceee7a97fa66485b1853182ff8',
  '46eed993d02f46dc553478fcdda6c0632b4d1161d478d5a9409b2925a8624c09',
  '09bb863d97be728ee75411ff87f1d09a5ef272b03fd23edca07b692239ce9346');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'East'/,
  '48e3b6f793ef83ff702a4a164a6d91f1e3711ff9d7eee9f2eb7f8ce704783355',
  '2285ea36a814317b190c43012f509a70a779cac61afc0dcd0a54f089cbd6b284',
  '932cf84bf09b180ed7e01976dabf6ba7088fc374c539991ea42600ae40cedcbd');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  'a7bdd145d3a98e1ba42ccff175ef3ff91c04ccff47cab1b0c8ae8ad21315f680',
  '6e4b779635b9dbd54490aa6a71aa45c4414c51554e449ccd88570ced42975b1b',
  'ef5b118bfa9dfbed332c1cf349b8cc792280f535a1dcba053e878657cb060198');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'South'/,
  '35c31352e889dc7912ddb0ae23af45c3776f36bc77a874b751d2594415cee1e6',
  'b72b1f24b49d9a3bb0507f3e5d5588b0c11cde499fab27d02df41d36dd87f099',
  '3fd57e40caef146868326f177db65265e8970c88ce20ba099152a1c5e8027e1f');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  'd46836aea9de6ca6441a856b103edbe1bee6ceeeab189ac8bdf61bac35556774',
  '192a8ee4da1c5d62d14ab7837378d06fec13c37cb097d7ee00f97534f86d8304',
  'c94a37024d7f8ff664c5aae0977ed6ff5d3aa92f46425b09b179c2ab32494e0c');

#
# 11) Test Framed Montage
#
# Image border color 'bordercolor' controls frame background color
# Image matte color 'mattecolor' controls frame color
# Image pen color 'pen' controls label text foreground color
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',, tile=>'4x4', geometry=>'90x80+3+3>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '5110ae8eb21ded2d8e6b6d3f5cc80317a6c58304f84731810a318bd76473427a',
  'ed4b38de6a8bc7408585a5b38efb677d6877244a80cff9ca2d95dbacdaf8df3f',
  '1419b60478f8f0d6f4da5fb87b0df3f16313a1e89a3192d9feeda2cdf2ac9860');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',, tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  'd4de864f9004b185297c9c2351f811aaeb779095bdd8fd5f29bcfbee79f09da4',
  'bcd96dabb454c5d25091422763b1cdecb6a69a9b02b84a5b7fa0a70f150b957c',
  '803c926764df2ad940f36fbbeea274e05467d729bfdfb2f28c479cfd27245a85');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',, tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  '8418407b6d56d2c1b67bc735004794d9eb20609d30115a93255eefcad3499e95',
  '9209b2db884fa4730eeab6c410b90e094fa305635baab7ede17270c13f6e80ad',
  '5793a1de15b5d73df297968af79fa01a110c1585cccd46dcbef794674ab5f174');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '5b5b6e7667055dee87a282cac637bceeec605644d8063972f79a01b2c07f9872',
  'd313d29183c31fb1aa91b0fb44ef04c8cd7ad6f7648a553fe0ec3c6ba7872fb4',
  '438478e6380519b065e4e24e33bab0e6dd6c4ab5a8c6449e22537fd55269babb');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e',, tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'fadebb098990a6230d30c55f45c4f1a31effc70055d4eb66c8f46c913257e1fb',
  '9468d6ff4312772656e980874be56ae69cf97946e78461de1f67e96c6c76675c',
  '535b6d2dd565ee5d01c3500569e60112812f5cd4d88525d528d35a5d02bbe681');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'fadebb098990a6230d30c55f45c4f1a31effc70055d4eb66c8f46c913257e1fb',
  '9468d6ff4312772656e980874be56ae69cf97946e78461de1f67e96c6c76675c',
  '535b6d2dd565ee5d01c3500569e60112812f5cd4d88525d528d35a5d02bbe681');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  'bead47d8f45327614e1a91f3537443317f59eebb960839f613365a1231b163ff',
  'ecc4b83e0b76129f1635c2e866bc641cce6a5d65a6c89bc497b5b81e4bb0f3b6',
  'ad05ecff14b56693e2785eb4b6f06215c215eb1309eb19591d9380027aacfe21');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e',, tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  '73dba1cf6a2077fca9c3d6a4f82ee5aa4481d64481423cffbb676b92e3f3c7dd',
  '108b50e6f8d5155f6c6f60dfe939e83ec465a917b3d8ec6fa1419d27ffa3cdb3',
  '1fdca151dfe00fdc106832696815eff00b7e32a5fb0af64b41cb08610661880e');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e',, tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  '73dba1cf6a2077fca9c3d6a4f82ee5aa4481d64481423cffbb676b92e3f3c7dd',
  '108b50e6f8d5155f6c6f60dfe939e83ec465a917b3d8ec6fa1419d27ffa3cdb3',
  '1fdca151dfe00fdc106832696815eff00b7e32a5fb0af64b41cb08610661880e');
