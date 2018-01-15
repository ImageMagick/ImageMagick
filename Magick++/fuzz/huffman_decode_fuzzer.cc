#include <cstdint>

#include <Magick++/Image.h>

#include "utils.cc"

namespace MagickCore
{
  extern "C" void AttachBlob(BlobInfo *,const void *,const size_t);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  Magick::Image image;
  MagickCore::AttachBlob(image.image()->blob,(const void *) Data,Size);

  Magick::ExceptionInfo *exceptionInfo;
  exceptionInfo=MagickCore::AcquireExceptionInfo();
  (void) HuffmanDecodeImage(image.image(), exceptionInfo);
  (void) MagickCore::DestroyExceptionInfo(exceptionInfo);
  return 0;
}

#include "travis.cc"
