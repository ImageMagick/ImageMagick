#include <Magick++/ResourceLimits.h>

class FuzzingLimits {
public:
  FuzzingLimits() {
    Magick::ResourceLimits::memory(1000000000);
  }
};

FuzzingLimits fuzzingLimits;

#if BUILD_MAIN
#include "encoder_format.h"

EncoderFormat encoderFormat;

#define FUZZ_ENCODER encoderFormat.get()
#endif // BUILD_MAIN
