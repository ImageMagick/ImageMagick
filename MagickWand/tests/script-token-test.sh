#!/bin/sh
#
# Basic testing of ScriptToken parser.
#
#    script-token-test.sh | diff - script-token-test-results.txt
#
./script-token-test script-token-test-data.txt
echo ""

echo -n "\"Next token bad quotes\" \"unfinished quotes ->" |\
   ./script-token-test
echo ""

perl -e 'print "\"Binary input follows\"\n", "abc\006xyz\n"' |\
   ./script-token-test
echo ""

( echo '"Very BIG Token Tests"'
  dd if=/dev/zero bs=80   count=1    2>/dev/null | tr '\0' 'a'; echo ""
  dd if=/dev/zero bs=500  count=1    2>/dev/null | tr '\0' 'b'; echo ""
  dd if=/dev/zero bs=4000 count=1    2>/dev/null | tr '\0' 'c'; echo ""
  dd if=/dev/zero bs=5000 count=1    2>/dev/null | tr '\0' 'd'; echo ""
  dd if=/dev/zero bs=10k  count=1    2>/dev/null | tr '\0' 'e'; echo ""
  dd if=/dev/zero bs=13k  count=1    2>/dev/null | tr '\0' 'f'; echo ""
  dd if=/dev/zero bs=8k   count=1024 2>/dev/null | tr '\0' 'e'; echo ""
  echo '"and all is well!"'
) | ./script-token-test
echo ""

