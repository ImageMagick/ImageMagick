#!/bin/bash

set -e
set -x

MAGICKINCLUDE="appdir/usr/include/ImageMagick-7"
MAGICKFUZZERLIBS="-Lappdir/usr/lib -lz -lpng -lfontconfig -lxcb -lX11 -lXext -lbz2 -ljpeg -ljbig -lIlmImf -ldjvulibre -ltiff -llqr-1 -lwmf -lwmflite -lraqm -lxml2 -llzma -llcms2 -lpthread -lfreetype -lMagick++-7.Q16 -lMagickWand-7.Q16 -lMagickCore-7.Q16"

# only compile this against clang
if [[ "$CC" == 'clang' ]]; then
    clang++ -std=c++11 -I$MAGICKINCLUDE "Magick++/fuzz/encoder_list.cc" \
        -o "encoder_list" \
        -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 -DBUILD_TRAVIS=1 $MAGICKFUZZERLIBS

    for f in Magick++/fuzz/*_fuzzer.cc; do
        fuzzer=$(basename "$f" _fuzzer.cc)
        # encoder_fuzzer is special
        if [ "$fuzzer" = "encoder" ]; then
            continue
        fi
        clang++ -std=c++11 -I$MAGICKINCLUDE \
            "$f" -o "${fuzzer}_fuzzer" \
            -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 -DBUILD_TRAVIS=1 $MAGICKFUZZERLIBS
    done

    # Build one encoder to confirm it works
    clang++ -std=c++11 -I$MAGICKINCLUDE \
        "Magick++/fuzz/encoder_fuzzer.cc" -o "encoder_wmf_fuzzer" \
        -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 -DBUILD_TRAVIS=1 \
        "-DFUZZ_IMAGEMAGICK_ENCODER=WMF" $MAGICKFUZZERLIBS
fi
