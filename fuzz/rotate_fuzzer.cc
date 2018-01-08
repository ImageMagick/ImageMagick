#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size < sizeof(double)) {
    return 0;
  }
  double Degrees = *reinterpret_cast<const double *>(Data);
  if (!isfinite(Degrees)) {
    return 0;
  }
  const Magick::Blob blob(Data + sizeof(Degrees), Size - sizeof(Degrees));
  Magick::Image image;
  try {
    image.read(blob);
  } catch (Magick::Exception &e) {
    return 0;
  }
  image.rotate(Degrees);
  return 0;
}
