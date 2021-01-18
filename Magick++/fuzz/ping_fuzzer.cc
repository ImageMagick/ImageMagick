#include <cstdint>
#include <cstring>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"

#define FUZZ_ENCODER_STRING_LITERAL_X(name) FUZZ_ENCODER_STRING_LITERAL(name)
#define FUZZ_ENCODER_STRING_LITERAL(name) #name

#ifndef FUZZ_ENCODER
#define FUZZ_ENCODER FUZZ_ENCODER_STRING_LITERAL_X(FUZZ_IMAGEMAGICK_ENCODER)
#endif

#ifndef FUZZ_IMAGEMAGICK_INITIALIZER
#define FUZZ_IMAGEMAGICK_INITIALIZER ""
#endif
#define FUZZ_ENCODER_INITIALIZER FUZZ_ENCODER_STRING_LITERAL_X(FUZZ_IMAGEMAGICK_INITIALIZER)

static ssize_t EncoderInitializer(const uint8_t *Data, const size_t Size, Magick::Image &image)
{
  if (strcmp(FUZZ_ENCODER_INITIALIZER, "interlace") == 0) {
    Magick::InterlaceType interlace = (Magick::InterlaceType) *reinterpret_cast<const char *>(Data);
    if (interlace > Magick::PNGInterlace)
      return -1;
    image.interlaceType(interlace);
    return 1;
  }

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  Magick::Image image;
  const ssize_t offset = EncoderInitializer(Data, Size, image);
  if (offset < 0)
    return 0;
  std::string encoder = FUZZ_ENCODER;
  image.magick(encoder);
  image.fileName(std::string(encoder) + ":");
  const Magick::Blob blob(Data + offset, Size - offset);
  try {
    image.ping(blob);
  }
  catch (Magick::Exception &e) {
    return 0;
  }
  return 0;
}

#include "travis.cc"
