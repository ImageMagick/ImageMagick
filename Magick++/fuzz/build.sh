#!/bin/bash -eu

# build bzip2
pushd $SRC
tar zxf bzip2-1.0.6.tar.gz
popd
pushd "$SRC/bzip2-1.0.6"
make -j$(nproc)
make install PREFIX="$WORK"
popd

# build zlib
pushd "$SRC/zlib"
./configure --static --prefix="$WORK"
make -j$(nproc)
make install
popd

# Build libtiff
pushd "$SRC/libtiff"
./autogen.sh
./configure --enable-static --disable-shared --prefix="$WORK"
make -j$(nproc)
make install
popd

# Build libde265
pushd "$SRC/libde265"
./autogen.sh
./configure --disable-shared --prefix="$WORK"
make -j$(nproc)
make install
popd

# Build webp
pushd "$SRC/libwebp"
./autogen.sh
./configure --disable-shared --prefix="$WORK"
make -j$(nproc)
make install
popd

# Build openjpg
pushd "$SRC/openjpeg"
cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DBUILD_SHARED_LIBS:bool=off -DBUILD_CODEC:bool=off -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
make install
popd


# Build ImageMagick
./configure --prefix="$WORK" --disable-shared --disable-docs LIBS="-lc++ -lubsan" LDFLAGS="${LDFLAGS:-} -L$WORK/lib" CFLAGS="$CFLAGS -I$WORK/include" PKG_CONFIG_PATH="$WORK/lib/pkgconfig"
make "-j$(nproc)"
make install

MAGICK_COMPILER=$CXX
MAGICK_COMPILER_FLAGS=$CXXFLAGS
MAGICK_INCLUDE="$WORK/include/ImageMagick-7"
MAGICK_SRC="$SRC/imagemagick/Magick++/fuzz"
MAGICK_LIBS="-lFuzzingEngine $WORK/lib/libMagick++-7.Q16HDRI.a $WORK/lib/libMagickWand-7.Q16HDRI.a $WORK/lib/libMagickCore-7.Q16HDRI.a $WORK/lib/libz.a $WORK/lib/libbz2.a $WORK/lib/libtiff.a $WORK/lib/libde265.a $WORK/lib/libopenjp2.a $WORK/lib/libwebp.a"
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
