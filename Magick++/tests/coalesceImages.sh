#!/bin/sh
#
# Copyright 2004 Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
# This file is part of Magick++, the C++ API for ImageMagick and
# ImageMagick.  Please see the file "COPYING" included with Magick++
# for usage and copying restrictions.
#

SRCDIR=`dirname $0`
SRCDIR=`cd $SRCDIR; pwd`/
export SRCDIR
cd Magick++/tests || exit 1

executable=`echo $0 | sed -e 's:.*/::g;s:test_::;s:\.sh::'`

outfile="test_${executable}.out"

rm -f $outfile
${MEMCHECK} "./${executable}" 2>&1 > $outfile
status=$?
if test $status -eq 1
then
  cat $outfile
fi
rm -f $outfile
exit $status


