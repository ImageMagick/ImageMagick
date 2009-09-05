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
cd Magick++/demo || exit 1

base_test=`echo $0 | sed -e 's:.*/::g;s:test_::;s:\.sh::'`
executable=`echo $base_test | sed -e 's:_.*$::'`
filter=`echo $base_test | sed -e 's:.*_::'`

outfile="test_${base_test}.out"

rm -f $outfile
${MEMCHECK} ./$executable -filter $filter -geometry 600x600 ${SRCDIR}/model.miff  ${executable}_${filter}_out.miff 2>&1 > $outfile
status=$?
if test $status -eq 1
then
  cat $outfile
fi
rm -f $outfile
exit $status


