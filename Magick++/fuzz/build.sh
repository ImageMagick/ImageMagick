#!/bin/bash -eu

./configure --prefix="$WORK" --disable-shared --disable-docs
make "-j$(nproc)"
make install

MAGICK_COMPILER=$CXX
MAGICK_COMPILER_FLAGS=$CXXFLAGS
MAGICK_INCLUDE="$WORK/include/ImageMagick-7"
MAGICK_SRC="$SRC/imagemagick/Magick++/fuzz"
MAGICK_LIBS="-lFuzzingEngine $WORK/lib/libMagick++-7.Q16HDRI.a $WORK/lib/libMagickWand-7.Q16HDRI.a $WORK/lib/libMagickCore-7.Q16HDRI.a"
MAGICK_OUTPUT=$OUT
MAGICK_FAST_BUILD=0

. $MAGICK_SRC/build_fuzzers.sh

mkdir afl_testcases
(cd afl_testcases; tar xvf "$SRC/afl_testcases.tgz")
for format in gif jpg png bmp ico webp tif; do
    mkdir $format
    find afl_testcases -type f -name '*.'$format -exec mv -n {} $format/ \;
    zip -rj $format.zip $format/
    cp $format.zip "$OUT/encoder_${format}_fuzzer_seed_corpus.zip"
done