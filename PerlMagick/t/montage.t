#!/usr/bin/perl
#  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
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
  'a7eaf25ec79757e02a706a3f25022ce5f11ae8ae8e57fa001fd036eea2de8ab9',
  'ea7cddf84b2109684c34896189bfb8b45950fb7ac835a733afae13182c9c368f',
  '1de064bfdd428c871f15f9605e8cb0240b866bf5bc174e80c7fa7cb78d177a2c');

#
# 2) Test Center gravity
#    Image should be centered in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'Center'/,
  '4d935b299bccea6ca30ecaa681d2c5924d49bd499d28429605ddb3ed319ed3ce',
  '188aa6113eacaa8ab62174215aa0f6c6cddd61b7d5352c07988e1ab33bca1134',
  'de920cc4d9235790371f74c3bd1fed53c973c861b3b48e111e8366f628ac9fde');

#
# 3) Test NorthWest gravity
#    Image should be at top-left in frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthWest'/,
  'a8ffb192e899ac49513488913a5b6eb739c7c315188248bd0ec285b9a1df4791',
  '4c9fa1d96fda955ae545aa111983a78180934fe7616dfcc861172a09d5b80007',
  '530c228f51c7f338e64693d99ba409a9191d9ffbf642a12d06d0a5d3c23fb150');

#
# 4) Test North gravity
#    Image should be at top-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'North'/,
  'ff0bbb9ee40aaef9b5f57989449560f4ce49ed3f6eba9f94d2b77ad477139eca',
  'c06e5e42b501c249c08470d2063e4f1507e3e2ef36db96cdb56b40d596f9b1ee',
  '3291d681b64d2dc8d34599e1af25c9be6bccc00aaef5f9b1223792ebb6960227');

#
# 5) Test NorthEast gravity
#    Image should be at top-right of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'NorthEast'/,
  '6e50b9ff8f3c110e727981008d05ffb01e427eec542faea7aa583a7181b57a6f',
  'bd067ee016a5ce115e61ec3a1d6e1253018f3bc12c7ded6720280eedce6fd86e',
  'fba9553f6b8f995b4874228c1d768dc68fe987e11a0c2317561e1e4751f1cdb0');

#
# 6) Test West gravity
#    Image should be at left-center of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'West'/,
  '36c59af9a7da82a344f13e4ae9248b20456d54f6c19bf191e048c94d15466b32',
  '56c8ad289db37dafb29b1b41e49c2860809359d5bced73b60755e0dd1394be63',
  '981a5799eec1a57640a2c26032ebac62a3ef53706810a4908b2bb5fd0ca534b4');

#
# 7) Test East gravity
#    Image should be at right-center of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'East'/,
  'd3d87130e57f0e5d39d45b80e6d3f074488e5a3bca80f51484202e7fe13cbaf1',
  'e04d8202bb03fd461f0fb19084bc12f2834a11df71048a27cd1020b6510d6be2',
  '3ec563b6762cbe1b708ffabc167c2490eb3d48274662752b165887da1d68aae6');

#
# 8) Test SouthWest gravity
#    Image should be at bottom-left of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthWest'/,
  '914bb90f88c1e1fc097f2a0f2853f1499500135bc637e5588ee8e06a2e350f4f',
  '6456910908ab97815ca6118b5c165b34ccde6f04adead09f6ba7faf3ee5d1a89',
  '5e18766bc47722d5542f324a0d87f36f6dd7ee0dcedcfc08769fa46cc706f5b0');

#
# 9) Test South gravity
#    Image should be at bottom of frame
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'South'/,
  '74f403700db063ff4ea0b64158bcaae6229fb8ee727b8a1f1ae294dfd3d4857e',
  '481c5f80a3d05feda792db2f730355dbdb63df624d2df9ee0a6eece502fd2a40',
  'f256f530988bf7587230d946f82df38fe6e2688e93be475d7fbf0946465e0e1f');

#
# 10) Test SouthEast gravity
#     Image should be at bottom-right of frame.
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', geometry=>'90x80+5+5>', gravity=>'SouthEast'/,
  'fa3b5372ab5b5a62bc553b9c9f6ce177588fa6540b42874a6332bfa5c6d6eae6',
  '67c5bce34915223c6d47119eebb737708fbbaa08f2773a46209685d3a47b5526',
  'c8bf258bd7ac7232215166c553570fd1d83f745544ca0f7522ae4030f265ce93');

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
  '5155225f259ce1aa4ff63516f364e266029301ee44a74b4429678eaf0f2929ca',
  '621b8edc661d1b68e7a72b233ce5b9d974e5202d20553c9011aa11af86558b35',
  'cc4e49b70cb6e0bd5746333e6fa2872ea0e16454990c47bdadd6a3daef59b501');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  '343db3a0a259856c28ef8fdef9544ff1723dfc77fd1d7c06fddebd5bf4d94416',
  '16586a1f9f9e23745a954ff1b9460fb59c81cd61d878e79ea37597f35abb8938',
  '8201c5e4df3a2088f2a84d88a0b1f554cf199622a9728d48c7a068ba69781c78');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  'bd359af477364055a691c8a781cd5c2184c3681998992ce4be437dbc02a48f26',
  '8417fd1533b0d055ff4bd40e9ef503859ba8566f4f3fc7ed002d831881a3d575',
  'a6f6e7e58ec8ab18d64863620408e9908da0ad366281f62da3d4f63935df73da');

#
# 14) Test Un-bordered, Un-framed Montage
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '2cb2597765e04541f6d9e651c5b8101f326fc02dce7cb500fd1bf8da0a8ab319',
  '4557584a91187dac730bac456e62522f4a9d470cf7f704e25e42ece0222db489',
  '82dcc2a143504c662cd0a36785c2f4083ac554164431d2bbc248ea6e5d1515ac');

#
# 15) Test Bordered, Un-framed Montage (mode=>'Unframe')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'Unframe',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '874c5f600bbf329b272dd8c210133ea4cb28b454177e3e4a23091f3719aff83f',
  '048b6d022215a31b7a9b3b16ea1e5532ff6d9c784c2ffee5a3ab8d5b7dca5f8a',
  '6b81664b58fe7d6c62b7c2f8388235142bdb5ff179fe39ee2eb7118e67e941b8');

#
# 16) Test Bordered, Un-framed Montage (mode=>'UnFrame')
#
++$test;
testMontage( q/bordercolor=>'red'/, 
  q/label=>'', tile=>'4x4', geometry=>'90x80+6+6>', mode=>'UnFrame',
  borderwidth=>'5', gravity=>'Center', background=>'gray'/,
  '874c5f600bbf329b272dd8c210133ea4cb28b454177e3e4a23091f3719aff83f',
  '048b6d022215a31b7a9b3b16ea1e5532ff6d9c784c2ffee5a3ab8d5b7dca5f8a',
  '6b81664b58fe7d6c62b7c2f8388235142bdb5ff179fe39ee2eb7118e67e941b8');

#
# 17) Test Un-bordered, Un-framed Montage with 16x1 tile
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'16x1', geometry=>'90x80+0+0>', mode=>'Unframe',
  borderwidth=>'0', gravity=>'Center', background=>'gray'/,
  '94c095fac1601253e9f1eab309aeb588f038f4ac47aae8089255709461e9b263',
  '0712c4cabf0db958853ca85ede6f4baebc62de4619576cfd6103c0e7d347f6b8',
  '26975a4a978a5529aad348fc77de5cf03c6d8bae02a96a4bb6c85f3c3f3aef5a');

#
# 18) Test concatenated thumbnail Montage (concatenated via special Concatenate mode)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//,
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'90x80>', mode=>'Concatenate'/,
  '863f5c1d06d3b46c579f34d6e5df73bcba3848751d02a3b7c2c61ec3fd3ea334',
  'dc94e4ff9acd98776b74e1f013a6e1f06b8be6df0e7178fb5351d2916f231d35',
  '2ddf734af40280d09f41d038d2a1a008e6b2b2f61cb2ff876030746263fe7934');
#
# 19) Test concatenated thumbnail Montage (concatentated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e', label=>'', tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  'd9bddfad7d74652b39b6a76eef54373380055343d75c7e9163686cf60c599875',
  '20c8568bd09b08987e66afc42a14f9b73cfaa4989c335369653aea60def92ff3',
  '6a9b96bc81980364691327c3e056c47f5a14dc905d61c7ae83200f7dcda9ea40');
