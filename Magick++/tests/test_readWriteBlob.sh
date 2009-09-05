#!/bin/sh
#
# Copyright 2004 Bob Friesenhahn <bfriesen@simple.dallas.tx.us>
#
executable=`echo $0 | sed -e 's:.*/::g;s:test_::;s:\.sh::'`

outfile="test_${executable}.out"

rm -f $outfile
${MAGICK_ENV} ${MEMCHECK} "./${executable}" 2>&1 > $outfile
status=$?
if test $status -eq 1
then
  cat $outfile
fi
rm -f $outfile
exit $status


