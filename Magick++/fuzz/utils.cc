#include <Magick++/ResourceLimits.h>


class FuzzingResourceLimits {
    FuzzingResourceLimits() {
        Magick::ResourceLimits::memory(1500000000);
    }
}
