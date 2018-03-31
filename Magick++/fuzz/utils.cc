#include <Magick++/ResourceLimits.h>
#include <Magick++/SecurityPolicy.h>

#ifndef FUZZ_MAX_SIZE
#define FUZZ_MAX_SIZE 2048
#endif

class FuzzingLimits {
public:
  FuzzingLimits() {
    Magick::SecurityPolicy::maxMemoryRequest(256000000);
    Magick::ResourceLimits::memory(1000000000);
    Magick::ResourceLimits::map(500000000);
    Magick::ResourceLimits::width(FUZZ_MAX_SIZE);
    Magick::ResourceLimits::height(FUZZ_MAX_SIZE);
    Magick::ResourceLimits::listLength(32);
  }
};

FuzzingLimits fuzzingLimits;

#if BUILD_MAIN
#include "encoder_format.h"

EncoderFormat encoderFormat;

#define FUZZ_ENCODER encoderFormat.get()
#endif // BUILD_MAIN
