#!/bin/bash -eu

MAGICK_COMPILER=$CC
MAGICK_COMPILER_FLAGS="-DBUILD_TRAVIS=1"
MAGICK_INCLUDE="appdir/usr/include/ImageMagick-7"
MAGICK_SRC="Magick++/fuzz"
MAGICK_LIBS="-Lappdir/usr/lib -lz -lpng -lfontconfig -lxcb -lX11 -lXext -lbz2 -ljpeg -ljbig -lIlmImf -ldjvulibre -ltiff -llqr-1 -lwmf -lwmflite -lraqm -lxml2 -llzma -llcms2 -lpthread -lfreetype -lMagick++-7.Q16 -lMagickWand-7.Q16 -lMagickCore-7.Q16 -lstdc++"
MAGICK_OUTPUT="Magick++/fuzz"
MAGICK_FAST_BUILD=1

export LD_LIBRARY_PATH=appdir/usr/lib

. $MAGICK_SRC/build_fuzzers.sh