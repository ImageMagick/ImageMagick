#!/bin/bash -eu

MAGICK_COMPILER_FLAGS="$MAGICK_COMPILER_FLAGS -DMAGICKCORE_HDRI_ENABLE=1 -DMAGICKCORE_QUANTUM_DEPTH=16"

$MAGICK_COMPILER $MAGICK_COMPILER_FLAGS -std=c++11 -I$MAGICK_INCLUDE "$MAGICK_SRC/encoder_list.cc" \
    -o "$MAGICK_SRC/encoder_list" $MAGICK_LIBS

for f in $MAGICK_SRC/*_fuzzer.cc; do
    fuzzer=$(basename "$f" _fuzzer.cc)
    # encoder_fuzzer is special
    if [ "$fuzzer" = "encoder" ]; then
        continue
    fi
    $MAGICK_COMPILER $MAGICK_COMPILER_FLAGS -std=c++11 -I$MAGICK_INCLUDE \
        "$f" -o "$MAGICK_OUTPUT/${fuzzer}_fuzzer" $MAGICK_LIBS
done

for encoder in $("$MAGICK_SRC/encoder_list"); do
    encoder_flags="-DFUZZ_IMAGEMAGICK_ENCODER=$encoder"
    $MAGICK_COMPILER $MAGICK_COMPILER_FLAGS -std=c++11 -I$MAGICK_INCLUDE \
        "$MAGICK_SRC/encoder_fuzzer.cc" -o "$MAGICK_OUTPUT/encoder_${encoder,,}_fuzzer" \
         $encoder_flags $MAGICK_LIBS
done