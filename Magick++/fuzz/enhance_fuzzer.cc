#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  const Magick::Blob blob(Data, Size);
  Magick::Image image;
  try {
    image.read(blob);
    image.enhance();
  } catch (Magick::Exception &e) {
    return 0;
  }
  return 0;
}

#include "travis.cc"
