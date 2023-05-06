#!/bin/bash -eu

autoreconf -fiv
./configure --prefix="$WORK" --disable-shared --disable-docs --with-jxl CFLAGS="$CFLAGS -I$WORK/include" LIBS="-L$WORK/lib -lhwy -lbrotlidec -lbrotlienc -lbrotlicommon -lde265" PKG_CONFIG_PATH="$WORK/lib/pkgconfig"
make "-j$(nproc)"
make install
