#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#define FUZZ_MAX_SIZE 128

#include "utils.cc"
#include "encoder_utils.cc"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  return fuzzEncoderWithStringFilename("plasma", Data, Size);
}

#include "travis.cc"
