#!/bin/sh
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
#  Test for 'validate' utility.
#
set -e # Exit on any error
. ${srcdir}/tests/common.sh

${MAGICK} pnm:- null:   < ${REFERENCE_IMAGE}
${MAGICK} pnm:- info:   < ${REFERENCE_IMAGE}
${MAGICK} pnm:- miff:-  < ${REFERENCE_IMAGE} | ${IDENTIFY} -
${MAGICK} pnm:- -       < ${REFERENCE_IMAGE} | ${IDENTIFY} -
${MAGICK} ${REFERENCE_IMAGE} -write null:  null:
${MAGICK} ${REFERENCE_IMAGE} -write info:  null:
${MAGICK} ${REFERENCE_IMAGE} -write miff:- null: | ${IDENTIFY} -
${MAGICK} ${REFERENCE_IMAGE} -write -      null: | ${IDENTIFY} -

# IMv7 "magick" testing

# -exit can be used insted of implicit write
${MAGICK} ${REFERENCE_IMAGE} -write info: -exit
# null: does not require an image during write
${MAGICK} -write null: -exit
${MAGICK} ${REFERENCE_IMAGE} -write info: +delete null:
# Using file decriptors (write)
${MAGICK} ${REFERENCE_IMAGE} fd:6  6>&1 | ${IDENTIFY} -
# Using file decriptors (read)
exec 5<${REFERENCE_IMAGE}
${MAGICK} fd:5 info:
exec 5<&-
# pipelined magick script
echo "-read ${REFERENCE_IMAGE} -write info:" | ${MAGICK} -script -
# pipelined magick script, input image pre-read
echo "-write info:" | ${MAGICK} ${REFERENCE_IMAGE} -script -
# pipelined script from file descriptor, read image from stdin
echo "-read pnm:- -write info:" |\
   ${MAGICK} -script fd:5 5<&0 <${REFERENCE_IMAGE}

