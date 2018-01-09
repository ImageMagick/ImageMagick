#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  uint16_t Width;
  uint16_t Height;
  if (Size < (sizeof(Width) + sizeof(Height))) {
    return 0;
  }
  Width = *reinterpret_cast<const uint16_t *>(Data);
  Height = *reinterpret_cast<const uint16_t *>(Data + sizeof(Width));
  const Magick::Blob blob(Data + sizeof(Width) + sizeof(Height),
                          Size - (sizeof(Width) + sizeof(Height)));
  Magick::Image image;
  try {
    image.read(blob);
  } catch (Magick::Exception &e) {
    return 0;
  }
  image.crop(Magick::Geometry(Width, Height));
  return 0;
}
