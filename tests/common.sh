SRCDIR=`dirname $0`
SRCDIR=`cd $SRCDIR && pwd`
TOPSRCDIR=`cd $srcdir && pwd`
export SRCDIR TOPSRCDIR
cd tests || exit 1
