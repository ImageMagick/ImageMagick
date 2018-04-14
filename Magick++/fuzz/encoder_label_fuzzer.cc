#include <cstdint>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"
#include "encoder_utils.cc"

static bool validateFileName(const std::string &fileName)
{
  // Signature: this will most likely cause a timeout.
  if (fileName.find("%#") != -1)
    return false;

  return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  return fuzzEncoderWithStringFilename("label", Data, Size, validateFileName);
}

#include "travis.cc"
