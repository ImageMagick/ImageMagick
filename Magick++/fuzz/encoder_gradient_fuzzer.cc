#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"
#include "encoder_utils.cc"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  return fuzzEncoderWithStringFilename("gradient", Data, Size);
}

#include "travis.cc"
