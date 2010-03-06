SRCDIR=`dirname $0`
SRCDIR=`cd $SRCDIR && pwd`
TOPSRCDIR=`cd $srcdir && pwd`
REFERENCE_IMAGE="${TOPSRCDIR}/images/rose.pnm"
export SRCDIR TOPSRCDIR
cd tests || exit 1
