#!/usr/bin/perl
#  Copyright 1999-20.0 ImageMagick Studio LLC, a non-profit organization
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
BEGIN { $| = 1; $test=1; print "1..18\n"; }
END {print "not ok $test\n" unless $loaded;}
use Image::Magick;
$loaded=1;

require 't/subroutines.pl';

chdir 't' || die 'Cd failed';

#
# Add
#
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'ModulusAdd'/,
  'reference/composite/Add.miff', 0.003, 1.0);
#
# Atop
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Atop'/,
  'reference/composite/Atop.miff', 0.00003, 0.009);

#
# Bumpmap
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"70x46"/,
  'input.miff', q//,q/, gravity=>'Center', compose=>'Bumpmap'/,
  'reference/composite/Bumpmap.miff', 0.03, 0.3);

#
# Clear
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//,
  q/, gravity=>'Center', 'clip-to-self'=>True, compose=>'Clear'/,
  'reference/composite/Clear.miff', 0.00003, 0.009);

#
# Copy
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Copy'/,
  'reference/composite/Copy.miff', 0.00003, 0.009);

#
# CopyBlue
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyBlue'/,
  'reference/composite/CopyBlue.miff', 0.00003, 0.009);

#
# CopyGreen
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyGreen'/,
  'reference/composite/CopyGreen.miff', 0.00003, 0.009);

#
# CopyRed
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyRed'/,
  'reference/composite/CopyRed.miff', 0.00003, 0.009);

#
# CopyAlpha
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"70x46"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyAlpha'/,
  'reference/composite/CopyAlpha.miff', 0.00003, 0.009);

#
# Difference
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Difference'/,
  'reference/composite/Difference.miff', 0.00003, 0.009);

#
# In
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'In'/,
  'reference/composite/In.miff', 0.00003, 0.009);

#
# Minus
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Minus'/,
  'reference/composite/Minus.miff', 0.00003, 0.009);

#
# Multiply
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Multiply'/,
  'reference/composite/Multiply.miff', 0.00003, 0.009);

#
# Out
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"70x46"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Out'/,
  'reference/composite/Out.miff', 0.00003, 0.009);

#
# Over
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Over'/,
  'reference/composite/Over.miff', 0.00003, 0.009);

#
# Plus
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Plus'/,
  'reference/composite/Plus.miff', 0.03, 0.7);

#
# Subtract
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'ModulusSubtract'/,
  'reference/composite/Subtract.miff', 0.0026, 1.0);

#
# Xor
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"70x46"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Xor'/,
  'reference/composite/Xor.miff', 0.00003, 0.009);

1;
