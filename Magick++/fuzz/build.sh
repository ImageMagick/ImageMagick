#!/bin/bash -eu

# build zlib
pushd "$SRC/zlib"
./configure --static --prefix="$WORK"
make -j$(nproc) CFLAGS="$CFLAGS -fPIC"
make install
popd

# Build xz
pushd "$SRC/xz"
./autogen.sh
./configure --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts --disable-doc --disable-shared --with-pic=yes --prefix="$WORK"
make -j$(nproc)
make install
popd

# Build png
pushd "$SRC/libpng"
cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DPNG_SHARED=off
make -j$(nproc)
make install
popd

# Build libjpeg-turbo
pushd "$SRC/libjpeg-turbo"
CFLAGS="$CFLAGS -fPIC" cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DENABLE_STATIC=on -DENABLE_SHARED=off
make -j$(nproc)
make install
popd

# Build libtiff
pushd "$SRC/libtiff"
autoreconf -fiv
./configure --prefix="$WORK"
make -j$(nproc)
make install
popd

# Build liblcms2
pushd "$SRC/Little-CMS"
autoreconf -fiv
./configure --disable-shared --prefix="$WORK"
make -j$(nproc)
make install
popd

# build libraw
pushd "$SRC/libraw"
autoreconf -fiv
./configure --prefix="$WORK" --disable-shared --with-pic=yes --disable-examples PKG_CONFIG_PATH="$WORK/lib/pkgconfig" CXXFLAGS="$CXXFLAGS -DLIBRAW_USE_CALLOC_INSTEAD_OF_MALLOC=on"
make -j$(nproc)
make install
popd

# Build freetype2
pushd "$SRC/freetype2"
./autogen.sh
./configure --prefix="$WORK" --disable-shared PKG_CONFIG_PATH="$WORK/lib/pkgconfig"
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

# Build libheif
pushd "$SRC/libheif"
./autogen.sh
./configure --disable-shared --prefix="$WORK" PKG_CONFIG_PATH="$WORK/lib/pkgconfig"
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
cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DBUILD_SHARED_LIBS=off -DBUILD_CODEC=off -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
make install
popd


# Build ImageMagick
./configure --prefix="$WORK" --disable-shared --disable-docs --with-utilities=no LDFLAGS="${LDFLAGS:-} -L$WORK/lib" CFLAGS="$CFLAGS -I$WORK/include" PKG_CONFIG_PATH="$WORK/lib/pkgconfig"
make "-j$(nproc)"
make install

MAGICK_COMPILER=$CXX
MAGICK_COMPILER_FLAGS=$CXXFLAGS
MAGICK_INCLUDE="$WORK/include/ImageMagick-7"
MAGICK_SRC="$SRC/imagemagick/Magick++/fuzz"
MAGICK_LIBS="-lFuzzingEngine $WORK/lib/libMagick++-7.Q16HDRI.a $WORK/lib/libMagickWand-7.Q16HDRI.a $WORK/lib/libMagickCore-7.Q16HDRI.a $WORK/lib/libpng.a $WORK/lib/libtiff.a $WORK/lib/libheif.a $WORK/lib/libde265.a $WORK/lib/libopenjp2.a $WORK/lib/libwebp.a $WORK/lib/libturbojpeg.a $WORK/lib/libjpeg.a $WORK/lib/libfreetype.a $WORK/lib/libraw.a $WORK/lib/liblzma.a $WORK/lib/liblcms2.a $WORK/lib/libz.a"
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

zip -rj "$OUT/encoder_heic_fuzzer_seed_corpus.zip" "$SRC/heic_corpus"
