/*
  Copyright @ 2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

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

static ssize_t EncoderInitializer(const uint8_t *Data,const size_t magick_unused(Size),Magick::Image &image)
{
  magick_unreferenced(Size);

  if (strcmp(FUZZ_ENCODER_INITIALIZER,"interlace") == 0)
    {
      Magick::InterlaceType
        interlace=(Magick::InterlaceType) *reinterpret_cast<const char *>(Data);

      if (interlace > Magick::PNGInterlace)
        return(-1);
      image.interlaceType(interlace);
      return(1);
    }
  if (strcmp(FUZZ_ENCODER_INITIALIZER,"png") == 0)
    image.defineValue("png","ignore-crc","1");
  return(0);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data,size_t Size)
{
  Magick::Image
    image;

  ssize_t
    offset;

  std::string
    encoder=FUZZ_ENCODER;

  if (IsInvalidSize(Size))
    return(0);
  offset=EncoderInitializer(Data,Size,image);
  if (offset < 0)
    return(0);
  image.magick(encoder);
  image.fileName(std::string(encoder)+":");
  try
  {
    const Magick::Blob
      blob(Data+offset,Size-offset);

#if defined(BUILD_MAIN)
    std::string
      image_data;

    image_data=blob.base64();
#endif

    image.read(blob);
  }
#if defined(BUILD_MAIN)
  catch (Magick::Exception &e)
  {
    std::cout << "Exception when reading: " << e.what() << std::endl;
    return(0);
  }
#else
  catch (Magick::Exception)
  {
    return(0);
  }
#endif

#if FUZZ_IMAGEMAGICK_ENCODER_WRITE || defined(BUILD_MAIN)
  try
  {
    Magick::Blob
      outBlob;

    image.write(&outBlob,encoder);
  }
#if defined(BUILD_MAIN)
  catch (Magick::Exception &e)
  {
    std::cout << "Exception when writing: " << e.what() << std::endl;
  }
#else
  catch (Magick::Exception)
  {
  }
#endif
#endif
  return(0);
}
