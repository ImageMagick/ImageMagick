#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  const Magick::Blob blob(Data, Size);
  Magick::Image image;
  try {
    image.read(blob);
  } catch (Magick::Exception &e) {
    return 0;
  }
  Magick::ExceptionInfo ex;
  auto res = HuffmanDecodeImage(image.image(), &ex);
  return 0;
}
