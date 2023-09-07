#!/bin/bash -eu

MAGICK_COMPILER=$CXX
MAGICK_COMPILER_FLAGS=$CXXFLAGS
MAGICK_INCLUDE="$WORK/include/ImageMagick-7"
MAGICK_SRC="$SRC/imagemagick/Magick++/fuzz"
MAGICK_LIBS_NO_FUZZ="$WORK/lib/libMagick++-7.Q16HDRI.a $WORK/lib/libMagickWand-7.Q16HDRI.a $WORK/lib/libMagickCore-7.Q16HDRI.a $WORK/lib/libpng.a $WORK/lib/libtiff.a $WORK/lib/libheif.a $WORK/lib/libde265.a $WORK/lib/libopenjp2.a $WORK/lib/libwebp.a $WORK/lib/libwebpmux.a $WORK/lib/libwebpdemux.a $WORK/lib/libsharpyuv.a $WORK/lib/libhwy.a $WORK/lib/libbrotlicommon.a $WORK/lib/libbrotlidec.a $WORK/lib/libbrotlienc.a $WORK/lib/libjxl_threads.a $WORK/lib/libjxl.a $WORK/lib/libturbojpeg.a $WORK/lib/libjpeg.a $WORK/lib/libfreetype.a $WORK/lib/libraw.a $WORK/lib/liblzma.a $WORK/lib/liblcms2.a $WORK/lib/libz.a"
MAGICK_LIBS="$LIB_FUZZING_ENGINE $MAGICK_LIBS_NO_FUZZ"
MAGICK_OUTPUT=$OUT
MAGICK_FAST_BUILD=0

. $MAGICK_SRC/build_dependencies.sh
. $MAGICK_SRC/build_imagemagick.sh
. $MAGICK_SRC/build_fuzzers.sh

mkdir afl_testcases
(cd afl_testcases; tar xvf "$SRC/afl_testcases.tgz")
for format in gif jpg png bmp ico webp tif; do
    mkdir $format
    find afl_testcases -type f -name '*.'$format -exec mv -n {} $format/ \;
    zip -rj $format.zip $format/
    cp $format.zip "$OUT/encoder_${format}_fuzzer_seed_corpus.zip"
done
