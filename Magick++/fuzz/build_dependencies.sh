#!/bin/bash -eu

# build zlib
pushd "$SRC/zlib"
./configure --static --prefix="$WORK"
make -j$(nproc) CFLAGS="$CFLAGS -fPIC"
make install
popd

# build deflate
pushd "$SRC/libdeflate"
cmake .  -DCMAKE_INSTALL_PREFIX=$WORK -DLIBDEFLATE_BUILD_SHARED_LIB=off -DLIBDEFLATE_BUILD_GZIP=off
make -j$(nproc)
make install
popd

# Build xz
pushd "$SRC/xz"
./autogen.sh --no-po4a --no-doxygen
./configure --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-ifunc --disable-scripts --disable-doc --disable-shared --with-pic=yes --prefix="$WORK"
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
./configure --disable-shared --prefix="$WORK" CFLAGS="$CFLAGS -I$WORK/include" LIBS="-L$WORK/lib"
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
pushd "$SRC/freetype"
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
cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DBUILD_SHARED_LIBS=off -DBUILD_TESTING=off -DWITH_EXAMPLES=off -DENABLE_PLUGIN_LOADING=off -DWITH_JPEG_DECODER=off -DWITH_JPEG_ENCODER=off -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
make install
popd

# Build webp
pushd "$SRC/libwebp"
./autogen.sh
./configure --disable-shared  --disable-png --disable-jpeg --disable-tiff --prefix="$WORK"
make -j$(nproc)
make install
popd

# Build openjpg
pushd "$SRC/openjpeg"
cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DBUILD_SHARED_LIBS=off -DBUILD_CODEC=off -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
make install
popd

# Build libjxl
pushd "$SRC/libjxl"
cmake . -DCMAKE_INSTALL_PREFIX=$WORK -DBUILD_SHARED_LIBS=off -DBUILD_TESTING=false  -DJPEGXL_ENABLE_TOOLS=false -DJPEGXL_ENABLE_SKCMS=false -DJPEGXL_ENABLE_DOXYGEN=false -DJPEGXL_ENABLE_MANPAGES=false -DJPEGXL_ENABLE_SJPEG=false -DJPEGXL_ENABLE_EXAMPLES=false -DJPEGXL_ENABLE_BENCHMARK=false -DJPEGXL_ENABLE_FUZZERS=false -DJPEGXL_BUNDLE_LIBPNG=false -DJPEGXL_ENABLE_JPEGLI_LIBJPEG=false -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS"
make -j$(nproc)
make install
popd
