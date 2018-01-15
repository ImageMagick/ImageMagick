#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"

#define FUZZ_ENCODER_STRING_LITERAL_X(name) FUZZ_ENCODER_STRING_LITERAL(name)
#define FUZZ_ENCODER_STRING_LITERAL(name) #name
#ifndef FUZZ_ENCODER
#define FUZZ_ENCODER FUZZ_ENCODER_STRING_LITERAL_X(FUZZ_IMAGEMAGICK_ENCODER)
#endif
#define FUZZ_IMAGEMAGICK_ENCODER_WRITE 1

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  const Magick::Blob blob(Data, Size);
  Magick::Image image;
  image.magick(FUZZ_ENCODER);
  image.fileName(std::string(FUZZ_ENCODER) + ":");
  try {
    image.read(blob);
  }
  catch (Magick::Exception &e) {
    return 0;
  }

#if FUZZ_IMAGEMAGICK_ENCODER_WRITE

  Magick::Blob outBlob;
  try {
    image.write(&outBlob, FUZZ_ENCODER);
  }
  catch (Magick::Exception &e) {
  }
#endif
  return 0;
}
