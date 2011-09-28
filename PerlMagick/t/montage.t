#!/usr/bin/perl
#  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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
  '977b812cb06e16d0ee7da6c80a9470cb85c2d33ba33f5919f26b05058f0eb177',
  '4aac8269bd31defb14971e475d23e0774c5fc929a60babddf8c3ecbcb01bfabd',
  '8de04216e6f60ad3bf63d5a2c54386cb824579e1c4a677af726e0306e0c7fb09');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'Center'/,
  '9a3f16a11fa4173f43642e7759070b32ce66a21c72312a7c99ed5dab711dc377',
  'a1825dab28dcb3f3fad641a2ec688199ad4c43253f5ea3fb13fafffe49a74818',
  '2a381bc2cc7dc751c95df0f6a88c5f04232e82d06094b65b3909f488e4347f93');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  '1ace55dbec2814d4d12e911f6ab21e0eac769cf12fbdcf516376f356222b74e1',
  'd41d7fb9cf59ab0d8f86545b4561fd267f555c0ab434a23d9b4d52e7b55fe64b',
  'f3be8f6f4ecabc14d6da8a3d8b6a2e369ad005d045f381df6f24ca2410406380');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'North'/,
  '02840dc5ac270f2e26422b8c90c5e5c3af81fd6e6739e9554977115e152a6872',
  '0c04c47f966d7769e99f99b0be4d8d1679e42a71543ce24efd2296afa9491073',
  'abd7f530c44e953ac799a04846a3eef613d0ab7b08aa92bb6cabe8abf275d5b6');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  'c5a10e80cb4f3b1c866d03e8cad4154f1139758ec86ffeef4fe2a802ac118ae2',
  '7110c0432bfd90f2173e547dbefe4c693757e7bf6bf731c8a97f72c71c723830',
  'bb35ddfaab468d869f2a3bafb16b3751015f2bb94e87438c48c7b294831d2bb8');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'West'/,
  '60f394bc620fba50b97dfe55c6026eed616a26ceee7a97fa66485b1853182ff8',
  '2a6f10fc376136ec74d53d7dd52c60e420edf851548377868bfb7f47d0e8a1e6',
  '09bb863d97be728ee75411ff87f1d09a5ef272b03fd23edca07b692239ce9346');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'East'/,
  '48e3b6f793ef83ff702a4a164a6d91f1e3711ff9d7eee9f2eb7f8ce704783355',
  '6a1517b10841950912b5a4e9fb852559268a3c64e86d6b391024c0e3ea7cc3e3',
  '932cf84bf09b180ed7e01976dabf6ba7088fc374c539991ea42600ae40cedcbd');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  'a7bdd145d3a98e1ba42ccff175ef3ff91c04ccff47cab1b0c8ae8ad21315f680',
  '6df59cabd0ccb84da56cf4a13516cfd8fc0bc215c438c8ee5bdbe6db82d52830',
  'ef5b118bfa9dfbed332c1cf349b8cc792280f535a1dcba053e878657cb060198');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'South'/,
  '35c31352e889dc7912ddb0ae23af45c3776f36bc77a874b751d2594415cee1e6',
  '227064c0fdd574d6c659a71bc24591d67fd604b4bb44c5466774446d9c7592f1',
  '3fd57e40caef146868326f177db65265e8970c88ce20ba099152a1c5e8027e1f');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  'd46836aea9de6ca6441a856b103edbe1bee6ceeeab189ac8bdf61bac35556774',
  '1b1eadf70f68f9ddea212b75d68d3109fcafe4e7db988cb90e148ca33be4691f',
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
  '5110ae8eb21ded2d8e6b6d3f5cc80317a6c58304f84731810a318bd76473427a',
  'a2b51a92bfa701b5121930caba4c39fca5de2d5d539570667b6a14aed1a1d0bd',
  '7e6adf7d0f1dd962c66e1be4069d0b808fc5ba81d00cb473080e6b742e209cc3');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  '61645a8c80a7220a4260133a2b1ae720bf2755f2ed880a3fef0e9453641f5b79',
  'e4e2f293481151f49888cca594b38beaa37e168ba1efa6a590e166521490b338',
  '105eede4ff37f8067fed104dd42feea9eff899caf872e7f11316d33ca02ecbf6');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  '4655b59218afbd58d24a9ecf162a42868b3de40ecc8e9f9671dccf8b17e1aa33',
  'e4e2f293481151f49888cca594b38beaa37e168ba1efa6a590e166521490b338',
  'd2413f59b630f2fd5ad4416f441cdb3b839765c4da8f2fe4210ceecefc0c8716');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '5b5b6e7667055dee87a282cac637bceeec605644d8063972f79a01b2c07f9872',
  '2b108aac04c0d339dbae55161a050dd788d2fd09a35f2ad5a0810b7c546ada44',
  '438478e6380519b065e4e24e33bab0e6dd6c4ab5a8c6449e22537fd55269babb');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'fadebb098990a6230d30c55f45c4f1a31effc70055d4eb66c8f46c913257e1fb',
  'eb2db2e5e0558563d814da22c8ca9be993a3217744eaba53d6921b223c6e6f3f',
  '535b6d2dd565ee5d01c3500569e60112812f5cd4d88525d528d35a5d02bbe681');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  'fadebb098990a6230d30c55f45c4f1a31effc70055d4eb66c8f46c913257e1fb',
  'eb2db2e5e0558563d814da22c8ca9be993a3217744eaba53d6921b223c6e6f3f',
  '535b6d2dd565ee5d01c3500569e60112812f5cd4d88525d528d35a5d02bbe681');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  'bead47d8f45327614e1a91f3537443317f59eebb960839f613365a1231b163ff',
  '26ba497f978db17009459907d6634abe0084f4dc26f2edc4435e15087a5a7918',
  'ad05ecff14b56693e2785eb4b6f06215c215eb1309eb19591d9380027aacfe21');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  '73dba1cf6a2077fca9c3d6a4f82ee5aa4481d64481423cffbb676b92e3f3c7dd',
  '3f42ab1a8a19594ab6a0c7990a35a34d92d9fff2a4ed59c5202a660d27c3ef36',
  '1fdca151dfe00fdc106832696815eff00b7e32a5fb0af64b41cb08610661880e');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  '73dba1cf6a2077fca9c3d6a4f82ee5aa4481d64481423cffbb676b92e3f3c7dd',
  '3f42ab1a8a19594ab6a0c7990a35a34d92d9fff2a4ed59c5202a660d27c3ef36',
  '1fdca151dfe00fdc106832696815eff00b7e32a5fb0af64b41cb08610661880e');
