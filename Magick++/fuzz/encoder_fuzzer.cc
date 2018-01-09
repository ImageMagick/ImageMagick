#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#define FUZZ_ENCODER_STRING_LITERAL(name) #name
#define FUZZ_ENCODER FUZZ_ENCODER_STRING_LITERAL(FUZZ_IMAGEMAGICK_ENCODER)

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  const Magick::Blob blob(Data, Size);
  Magick::Image image;
  try {
    image.read(blob);
  } catch (Magick::Exception &e) {
    return 0;
  }

  Magick::Blob outBlob;
  try {
    image.write(&outBlob, FUZZ_ENCODER);
  } catch (Magick::Exception &e) {
  }
  return 0;
}
