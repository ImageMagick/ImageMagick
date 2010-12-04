#!/bin/sh
# Copyright (C) 1999-2011 ImageMagick Studio LLC
#
# This program is covered by multiple licenses, which are described in
# LICENSE. You should have received a copy of LICENSE with this
# package; otherwise see http://www.imagemagick.org/script/license.php.
#

set -e # Exit on any error
. ${srcdir}/wand/common.sh

${MEMCHECK} ./drawtest drawtest_out.miff
