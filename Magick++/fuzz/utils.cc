#include <Magick++/Functions.h>
#include <Magick++/ResourceLimits.h>
#include <Magick++/SecurityPolicy.h>

#ifndef FUZZ_MAX_SIZE
#define FUZZ_MAX_SIZE 2048
#endif

class FuzzingInitializer {
public:
  FuzzingInitializer() {

    // Disable SIMD in jpeg turbo.
    (void) putenv(const_cast<char *>("JSIMD_FORCENONE=1"));

    Magick::InitializeMagick((const char *) NULL);
    Magick::SecurityPolicy::anonymousCacheMemoryMap();
    Magick::SecurityPolicy::anonymousSystemMemoryMap();
    Magick::SecurityPolicy::maxMemoryRequest(256000000);
    Magick::ResourceLimits::memory(1000000000);
    Magick::ResourceLimits::map(500000000);
    Magick::ResourceLimits::width(FUZZ_MAX_SIZE);
    Magick::ResourceLimits::height(FUZZ_MAX_SIZE);
    Magick::ResourceLimits::listLength(16);
  }
};

FuzzingInitializer fuzzingInitializer;

#if BUILD_MAIN
#include "encoder_format.h"

EncoderFormat encoderFormat;

#define FUZZ_ENCODER encoderFormat.get()
#endif // BUILD_MAIN
