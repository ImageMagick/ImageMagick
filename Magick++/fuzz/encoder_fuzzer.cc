#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"

#define FUZZ_ENCODER_STRING_LITERAL_X(name) FUZZ_ENCODER_STRING_LITERAL(name)
#define FUZZ_ENCODER_STRING_LITERAL(name) #name
#ifndef FUZZ_ENCODER
#define FUZZ_ENCODER FUZZ_ENCODER_STRING_LITERAL_X(FUZZ_IMAGEMAGICK_ENCODER)
#endif

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::string encoder = FUZZ_ENCODER;
  const Magick::Blob blob(Data, Size);
  Magick::Image image;
  image.magick(encoder);
  image.fileName(std::string(encoder) + ":");
  try {
    image.read(blob);
  }
  catch (Magick::Exception &e) {
    return 0;
  }

#if FUZZ_IMAGEMAGICK_ENCODER_WRITE || BUILD_MAIN

  Magick::Blob outBlob;
  try {
    image.write(&outBlob, encoder);
  }
  catch (Magick::Exception &e) {
  }
#endif
  return 0;
}

#include "travis.cc"
