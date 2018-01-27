#include <Magick++/ResourceLimits.h>
#include <Magick++/SecurityPolicy.h>

class FuzzingLimits {
public:
  FuzzingLimits() {
    Magick::SecurityPolicy::maxMemoryRequest(256000000);
    Magick::ResourceLimits::memory(1000000000);
    Magick::ResourceLimits::map(500000000);
    Magick::ResourceLimits::width(2048);
    Magick::ResourceLimits::height(2048);
    Magick::ResourceLimits::listLength(32);
  }
};

FuzzingLimits fuzzingLimits;

#if BUILD_MAIN
#include "encoder_format.h"

EncoderFormat encoderFormat;

#define FUZZ_ENCODER encoderFormat.get()
#endif // BUILD_MAIN
