
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
  'dcf3971caf73a0d7a7e3dd5c231622f7350da6a02addfb2eb1a9ebce75246f88',
  '40835871b13232243ba4d2bcb244d06958556a24a9624e793f5c4c291d64f322',
  '531d0d485d55a5b585cab3f64b37da81a109fbaa91aa44a038fa916421780f14',
  '531d0d485d55a5b585cab3f64b37da81a109fbaa91aa44a038fa916421780f14',
  '531d0d485d55a5b585cab3f64b37da81a109fbaa91aa44a038fa916421780f14');

#
# 12) Test Framed Montage with drop-shadows
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True',background=>'gray'/,
  'f338c5218bf2dba47490d453bd845ed7b6e6c146079f662265338baedcd473ee',
  '40d6c29a9e85135eed35cf47b79cdfd8091f4a72459fbea0cc80f94bfff87d88',
  '420aecbcab063e2735156a2b4d0b96e89d1b89dd84d3b2f217c9cf980f25939c',
  '16d83700b4dabac353bcc557aa2581659a07f71e8098b550f5ef241b6d1ab9e6',
  'ab9588dcdd480f7a792ecc3202b0af5d9c8bd900eee1447f0c6ef1cedee3b140');

#
# 13) Test Framed Montage with drop-shadows and background texture
#
++$test;
testMontage( q/bordercolor=>'blue', mattecolor=>'red'/, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'90x80+6+6>', frame=>'8x10',
  borderwidth=>'0', gravity=>'Center', shadow=>'True', texture=>'granite:'/,
  'b2be2edbc68a4b828392f5fbf2683a16e63d81bcd5c2c77e8aa1573c4a37b23b',
  '9a23ef5366121cee8d101af2d6def5920b4984ce8e37f5e46db8fd5b18c47896',
  'c10b9d30b4a3312df15f4a55c195992670dd22d59dbf2c4e3436ad730d647ed2',
  '6a7b56ca2e9f7e7308f1794b1556c2f87c3ba6050f84bfbeb61a7ccad2b3de9b',
  '4a2b769973f9e0682004307cf8c999d1476404e5c85712e6fe8e77af6699289d');

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
# 19) Test concatenated thumbnail Montage (concatenated by setting params to zero)
#     Thumbnails should be compacted tightly together in a grid
#
++$test;
testMontage( q//, 
  q/background=>'#696e7e',  tile=>'4x4', geometry=>'+0+0', mode=>'Unframe', shadow=>'False',
  borderwidth=>'0', background=>'gray'/,
  'dfede51035a4ce942e3ba9909f85eb497c134a60bb8363afded0ae81f73f0e46');
