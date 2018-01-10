#include <Magick++/ResourceLimits.h>


class FuzzingResourceLimits {
public:
    FuzzingResourceLimits() {
        Magick::ResourceLimits::memory(1500000000);
    }
};
