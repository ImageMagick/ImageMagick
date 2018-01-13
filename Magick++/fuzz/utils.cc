#include <Magick++/ResourceLimits.h>

class FuzzingResourceLimits {
public:
    FuzzingResourceLimits() {
        Magick::ResourceLimits::memory(1000000000);
    }
};

#if BUILD_MAIN
#include "encoder_format.h"

EncoderFormat encoderFormat;

#define FUZZ_ENCODER encoderFormat.get()
#endif // BUILD_MAIN
