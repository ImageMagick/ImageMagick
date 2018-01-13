#include <Magick++/ResourceLimits.h>


class FuzzingResourceLimits {
public:
    FuzzingResourceLimits() {
        Magick::ResourceLimits::memory(1000000000);
    }
};
