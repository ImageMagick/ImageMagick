#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  // Allow a bit extra to make sure we do proper bounds checking in Magick++
  if (Size > MagickPathExtent)
    return 0;
  std::string color(reinterpret_cast<const char*>(Data));

  Magick::Image image;
  try {
    image.read("xc:" + color);
  }
  catch (Magick::Exception &e) {
  }
  return 0;
}

#include "travis.cc"
