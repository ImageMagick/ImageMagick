#!/usr/bin/perl
#  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
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
# Test image blobs.
#
BEGIN { $| = 1; $test=1, print "1..1\n"; }
END {print "not ok 1\n" unless $loaded;}
use Image::Magick;
$loaded=1;

chdir 't' || die 'Cd failed';

$image = new Image::Magick;
$image->Read( 'input.miff' );
@blob = $image->ImageToBlob();
undef $image;

$image=Image::Magick->new( magick=>'MIFF' );
$image->BlobToImage( @blob );

if ($image->Get('signature') ne 
    'f7b3db46d6f696ea8392f0ad0be945dd502a806e2c1e9c082efef517191758f7')
  { print "not ok $test\n"; }
else
  { print "ok $test\n"; }

1;
