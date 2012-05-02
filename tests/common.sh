SRCDIR=`dirname $0`
SRCDIR=`cd $SRCDIR && pwd`
TOPSRCDIR=`cd $srcdir && pwd`
REFERENCE_IMAGE="${TOPSRCDIR}/images/rose.pnm"
[ "X$CONVERT" = "X" ] && CONVERT=convert
[ "X$MAGICK" = "X" ] && MAGICK=magick
[ "X$IDENTIFY" = "X" ] && IDENTIFY=identify
export SRCDIR TOPSRCDIR
cd tests || exit 1
