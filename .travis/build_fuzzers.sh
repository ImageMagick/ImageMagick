#!/bin/bash

set -e
set -x

MAGICKINCLUDE="appdir/usr/include/ImageMagick-7"
MAGICKSTATICLIBS="appdir/usr/lib/libMagick++-7.Q16.a appdir/usr/lib/libMagickWand-7.Q16.a appdir/usr/lib/libMagickCore-7.Q16.a libfuzzer/libFuzzer.a"

# Checkout and build libFuzzer
svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/fuzzer libfuzzer
pushd libfuzzer
./build.sh
popd


clang++ -std=c++11 -I$MAGICKINCLUDE "Magick++/fuzz/encoder_list.cc" \
    -o "encoder_list" \
    -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 $MAGICKSTATICLIBS -lpthread -lfreetype

for f in Magick++/fuzz/*_fuzzer.cc; do
    fuzzer=$(basename "$f" _fuzzer.cc)
    # encoder_fuzzer is special
    if [ "$fuzzer" = "encoder" ]; then
        continue
    fi
    clang++ -std=c++11 -I$MAGICKINCLUDE \
        "$f" -o "${fuzzer}_fuzzer" \
        -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 $MAGICKSTATICLIBS -lpthread-lfreetype
done

for encoder in $("./encoder_list"); do
    clang++ -std=c++11 -I$MAGICKINCLUDE \
        "Magick++/fuzz/encoder_fuzzer.cc" -o "encoder_${encoder,,}_fuzzer" \
        -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 \
        "-DFUZZ_IMAGEMAGICK_ENCODER=$encoder" $MAGICKSTATICLIBS -lpthread -lfreetype
done
