#!/bin/bash -eu

MAGICK_COMPILER_FLAGS="$MAGICK_COMPILER_FLAGS -fuse-ld=lld -DMAGICKCORE_HDRI_ENABLE=1 -DMAGICKCORE_QUANTUM_DEPTH=16"

$MAGICK_COMPILER $MAGICK_COMPILER_FLAGS -std=c++11 -I$MAGICK_INCLUDE "$MAGICK_SRC/encoder_list.cc" \
    -o "$MAGICK_SRC/encoder_list" $MAGICK_LIBS_NO_FUZZ

for f in $MAGICK_SRC/*_fuzzer.cc; do
    fuzzer=$(basename "$f" _fuzzer.cc)
    # encoder_fuzzer is special
    if [ "$fuzzer" == "encoder" ]; then
        continue
    fi
    $MAGICK_COMPILER $MAGICK_COMPILER_FLAGS -std=c++11 -I$MAGICK_INCLUDE \
        "$f" -o "$MAGICK_OUTPUT/${fuzzer}_fuzzer" $MAGICK_LIBS
    echo -e "[libfuzzer]\nclose_fd_mask=3" > "$MAGICK_OUTPUT/${fuzzer}_fuzzer.options"
done

for item in $("$MAGICK_SRC/encoder_list"); do
    info=${item:1}
    encoder=${info%:*}
    initializer=${info##*:}
    encoder_flags="-DFUZZ_IMAGEMAGICK_ENCODER=$encoder"
    if [ "$initializer" != "" ]; then
      encoder_flags="$encoder_flags -DFUZZ_IMAGEMAGICK_ENCODER_INITIALIZER=$initializer"
    fi

    if [ "${item:0:1}" == "+" ]; then
        encoder_flags="$encoder_flags -DFUZZ_IMAGEMAGICK_ENCODER_WRITE=1"
    fi

    $MAGICK_COMPILER $MAGICK_COMPILER_FLAGS -std=c++11 -I$MAGICK_INCLUDE \
        "$MAGICK_SRC/encoder_fuzzer.cc" -o "$MAGICK_OUTPUT/encoder_${encoder,,}_fuzzer" \
         $encoder_flags $MAGICK_LIBS

    echo -e "[libfuzzer]\nclose_fd_mask=3" > "$MAGICK_OUTPUT/encoder_${encoder,,}_fuzzer.options"

    if [ -f "$MAGICK_SRC/dictionaries/${encoder,,}.dict" ]; then
        cp "$MAGICK_SRC/dictionaries/${encoder,,}.dict" "$MAGICK_OUTPUT/encoder_${encoder,,}_fuzzer.dict"
    fi

    if [ $MAGICK_FAST_BUILD -eq 1 ]; then
        break
    fi
done
