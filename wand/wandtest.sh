#!/bin/sh
# Copyright (C) 1999-2009 ImageMagick Studio LLC
#
# This program is covered by multiple licenses, which are described in
# LICENSE. You should have received a copy of LICENSE with this
# package; otherwise see http://www.imagemagick.org/script/license.php.
#
#  Test for '${CONVERT}' utility.
#

set -e # Exit on any error
. ${srcdir}/wand/common.sh

${MEMCHECK} ./wandtest wandtest_out.miff
