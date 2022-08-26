#!/bin/bash -eu

autoreconf -fiv
./configure --prefix="$WORK" --disable-shared --disable-docs LDFLAGS="${LDFLAGS:-} -L$WORK/lib -lubsan -stdlib=libc++" CFLAGS="$CFLAGS -I$WORK/include" PKG_CONFIG_PATH="$WORK/lib/pkgconfig"
make "-j$(nproc)"
make install
