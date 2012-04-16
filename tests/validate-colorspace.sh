#!/bin/bash
#
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
. ${srcdir}/tests/common.sh

# how to generate a one pixel (average rose) color and output its values
in="rose: -scale 1x1"    # a one pixel image of the average color.
out="-format '%[fx:int(255*r+.5)],%[fx:int(255*g+.5)],%[fx:int(255*b+.5)]' info:-"

# ----------------

# Colors to compare results to.
error=false
average=`eval ${CONVERT} "$in" -noop "$out"`
too_light=`eval ${CONVERT} "$in" -colorspace RGB "$out"`
too_dark=`eval ${CONVERT} "$in" -set colorspace RGB -colorspace sRGB "$out"`
format='%-30s%s\n'        # results formating
format2='%-30s%-14s%s\n'

printf "$format2" "Average \"rose:\" Color"  "$average" "sRGB(rose)"
printf "$format2" "Too Light Color" "$too_light" "sRGB(rose)->RGB result"
printf "$format2" "Too Dark Color"  "$too_dark"  "RGB(rose)->sRGB result"
echo ''

#
# Sanity checks
#
# NOTE: as a extra validation on sanity checks below...
#    eval ${CONVERT} "$in" -gamma .454545 "$out"
# produces a value of  74,25,20   which is close to 73,26,21 below.
#    eval ${CONVERT} "$in" -gamma 2.2 "$out"
# produces a value of  198,158,151  whcih is close to 199,160,152 below.
#
# Actual values used below come from IM v6.5.4-7 colorspace conversions
#
if [ "X$average" != "X146,89,80" ]; then
  echo "Sanity Failure: Average expected to be 145,89,80 - ABORTING"
  error=true
fi
if [ "X$too_light" != "X73,26,21" ]; then
  echo "Sanity Failure: Too Light expected to be 73,26,21 - ABORTING"
  error=true
fi
if [ "X$too_dark" != "X199,160,152" ]; then
  echo "Sanity Failure: Too Dark expected to be 199,159,152 - ABORTING"
  error=true
fi
$error && exit 1

test_color() {
  test="sRGB(rose)"
  cs='';
  for i in "$@"; do
    test="${test}->$i"        # format of the test being performed
    cs="$cs -colorspace $i"   # colorspace operations to perform test
  done
  color=`eval ${CONVERT} "$in" $cs "$out"`

  if [ "X$color" = "X$average" ]; then
    printf "$format" "$test" "good"
    return
  fi
  error=false
  if [ "X$color" = "X$too_light" ]; then
    printf "$format" "$test" "TOO_LIGHT"
    return
  fi
  if [ "X$color" = "X$too_dark" ]; then
    printf "$format" "$test" "TOO_DARK"
    return
  fi
  printf "$format" "$test" "UNKNOWN COLOR (expect $average, got $color)"
}

# ----------------

test_color RGB sRGB
test_color XYZ sRGB
test_color RGB XYZ sRGB
test_color XYZ RGB sRGB

test_color CMY   sRGB
test_color CMYK  sRGB
test_color HSL   sRGB
test_color HSB   sRGB
test_color Lab   sRGB
test_color YIQ   sRGB
test_color YCbCr sRGB

eval $error   # return the overall error result
