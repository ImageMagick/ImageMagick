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
  'e9455ad714f3be6dc4c0d6bf4cf85ce81625d1a8120562c4e584a2c8551613af',
  '93afa0ad47528ea284a170f6564e9cbb0b824959daf49ada35c43d9e8ca40f79',
  '8de04216e6f60ad3bf63d5a2c54386cb824579e1c4a677af726e0306e0c7fb09');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'Center'/,
  '51de44a7295de9dead16b8ec2186543913f6012b5aae4ad99d9db02c73ec07e9',
  '28537343abbee6a9e29c9ee4e11ead61351085b8a9281794db5fbbaaf3d141a9',
  '2a381bc2cc7dc751c95df0f6a88c5f04232e82d06094b65b3909f488e4347f93');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  '6c93662b887d13b132577e14e6dfe73f9a2247e2488d257ff9d19dca11269f92',
  'bf94b426dcb94c2c60470bc26be6d14442eea770e2ba32578145f230f40cae91',
  'f3be8f6f4ecabc14d6da8a3d8b6a2e369ad005d045f381df6f24ca2410406380');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'North'/,
  '6f851b77d28020a3afb89fc472de505cac95dbc1876e7c47c4022bbad9bd9884',
  '50e2dedd7bd218ca18da007f602f60024a6c944b9e5dffbc619b50a4533df1ab',
  'abd7f530c44e953ac799a04846a3eef613d0ab7b08aa92bb6cabe8abf275d5b6');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  'b718edc7cf14dfbf50255505452bbc3ea2da60b7f7b4a7ab3e452a2b5c8aa5fa',
  'b96307ebfc16f394da5e56c20c327de04945651931496a16633f604ba495c97c',
  'bb35ddfaab468d869f2a3bafb16b3751015f2bb94e87438c48c7b294831d2bb8');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'West'/,
  'ceacbd28df856c0285e50e76d69d73282b7b0308e4d3a68b0764190a5c5b1aed',
  '725374b2776657c3fcac22dbc6ae5d6b77027f6675067217c15f90f2aebb3879',
  '09bb863d97be728ee75411ff87f1d09a5ef272b03fd23edca07b692239ce9346');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'East'/,
  '4bf8bf1c8f5b91e6c940d0f5c85fad66a327b058a3a7d195f90ad2b59e8f9b21',
  '378382a287282b4800bbcf63bebed1f8ed8ad591aac70871cc558f4d50c67924',
  '932cf84bf09b180ed7e01976dabf6ba7088fc374c539991ea42600ae40cedcbd');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  'ef4188d71d0bb9e0b23c414adee0b2fa3f6adc960bbbe0c6f9637460bf61db4f',
  '7aa1f23b23771917e9b0e40da79ef98ad74ff57f692d9a3070af830c2a43b456',
  'ef5b118bfa9dfbed332c1cf349b8cc792280f535a1dcba053e878657cb060198');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'South'/,
  '03e3e711a7dbf46f2f52d245aecd5a8b5bba0f6469e2c17be36536df99f3e4eb',
  '393142e3e319751408a85d4ee677e744ca0bb8fab5024243d84ef23694a4d99f',
  '3fd57e40caef146868326f177db65265e8970c88ce20ba099152a1c5e8027e1f');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  '56c93dbbae5f329a1fd5f1eff764cff13bbe9dcfe77c24f7a5ba2a1352218c2e',
  '030228bcfccbfd8536fdab88500312c4cf1ec218ac647f32d2d5498bef9c15ae',
  'c94a37024d7f8ff664c5aae0977ed6ff5d3aa92f46425b09b179c2ab32494e0c');

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
  '0fb2fe82827cd5bfa876f462eeb70f9e1e3699b8cf73e05653308d7ca502dff2',
  '337e0666b183cadeb861cba8681228e8644f522aeb337dd872c14c122bff4946',
  '7e6adf7d0f1dd962c66e1be4069d0b808fc5ba81d00cb473080e6b742e209cc3');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  'a3afa3d7b85fd6fe5197d230d013395f47e51712dba6f31bc583e22d96c94521',
  '1af94b1faa3a4a4533add7d02cb4af13a67aa844d8f11773d7737f0d9b69120c',
  '105eede4ff37f8067fed104dd42feea9eff899caf872e7f11316d33ca02ecbf6');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  '3b4da5983e29ae3a4ee58aec8d2f44caf822e3416751ac15a970620b70c7a69c',
  '29c23db0f64c55f4f21f1e117edd8ce785f3ee2f7024e81c39ac2be0946e0fe4',
  'd2413f59b630f2fd5ad4416f441cdb3b839765c4da8f2fe4210ceecefc0c8716');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  'e80438193bd5d114faa77f884d1c5626dfad037f318b6a716c73b0338b1c3960',
  '80e14d97aa951296605f82117fbe3bbf16cd20b1f1ebfc13134bba8a2457f47d',
  '438478e6380519b065e4e24e33bab0e6dd6c4ab5a8c6449e22537fd55269babb');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '791a92e561a321341a4c25aa9b1249db37015a94787440ec84f7c0413eba01fa',
  '6a34f5ad3aa966259b9254653187e57e3ee884d5ecb96957e7969304336940fc',
  '535b6d2dd565ee5d01c3500569e60112812f5cd4d88525d528d35a5d02bbe681');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '791a92e561a321341a4c25aa9b1249db37015a94787440ec84f7c0413eba01fa',
  '6a34f5ad3aa966259b9254653187e57e3ee884d5ecb96957e7969304336940fc',
  '535b6d2dd565ee5d01c3500569e60112812f5cd4d88525d528d35a5d02bbe681');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  'd2c6ef7bf4bdff6308757dad856f14ec8b76d82dc0783e5d9bd7fd6b05d07ab5',
  '47cee9e742e71b695e72d874d4dbdc3d03f9ceed37862e3d365a0718b3edbc18',
  'ad05ecff14b56693e2785eb4b6f06215c215eb1309eb19591d9380027aacfe21');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  '21c855a24c46dc46b5e4e98ca38e87b124d62786b400d439a6c26bbce186aa67',
  'a5548990f7ade3ddc3ac8979c613739017fe30311331bc4c80f6c0e20ca3b195',
  '1fdca151dfe00fdc106832696815eff00b7e32a5fb0af64b41cb08610661880e');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  '33f2ed062f1f3f7ca95f48c8235f62c0675e0e80dff09d7ad117a9ca016bd11c',
  'c0893819eb2e2104b11a3ddd886c89c334e29b3a341c4d9880364ac32cc7b857',
  '1fdca151dfe00fdc106832696815eff00b7e32a5fb0af64b41cb08610661880e');
