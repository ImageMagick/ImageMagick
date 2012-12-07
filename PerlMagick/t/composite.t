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
  'input.miff', q//, q/, gravity=>'Center', compose=>'Add'/,
  'reference/composite/Add.miff', 0.2, 1.03);
#
# Atop
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Atop'/,
  'reference/composite/Atop.miff', 0.1, 1.03);

#
# Bumpmap
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//,q/, gravity=>'Center', compose=>'Bumpmap'/,
  'reference/composite/Bumpmap.miff', 0.1, 1.03);

#
# Clear
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Clear'/,
  'reference/composite/Clear.miff', 0.3, 1.03);

#
# Copy
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Copy'/,
  'reference/composite/Copy.miff', 0.1, 1.03);

#
# CopyBlue
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyBlue'/,
  'reference/composite/CopyBlue.miff', 0.1, 1.03);

#
# CopyGreen
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyGreen'/,
  'reference/composite/CopyGreen.miff', 0.1, 1.03);

#
# CopyRed
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyRed'/,
  'reference/composite/CopyRed.miff', 0.1, 1.03);

#
# CopyOpacity
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'CopyOpacity'/,
  'reference/composite/CopyOpacity.miff', 0.1, 1.03);

#
# Difference
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Difference'/,
  'reference/composite/Difference.miff', 0.1, 1.03);

#
# In
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'In'/,
  'reference/composite/In.miff', 0.1, 1.03);

#
# Minus
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Minus'/,
  'reference/composite/Minus.miff', 0.3, 1.03);

#
# Multiply
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Multiply'/,
  'reference/composite/Multiply.miff', 0.1, 1.03);

#
# Out
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Out'/,
  'reference/composite/Out.miff', 0.3, 1.03);

#
# Over
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Over'/,
  'reference/composite/Over.miff', 0.1, 1.03);

#
# Plus
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Plus'/,
  'reference/composite/Plus.miff', 0.1, 1.03);

#
# Subtract
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Subtract'/,
  'reference/composite/Subtract.miff', 0.1, 1.03);

#
# Xor
#
++$test;
testCompositeCompare('gradient:white-black',q/size=>"100x80"/,
  'input.miff', q//, q/, gravity=>'Center', compose=>'Xor'/,
  'reference/composite/Xor.miff', 0.3, 1.03);

1;
