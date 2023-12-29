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

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

#include "utils.cc"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data,size_t Size)
{
  Magick::Image
    image;

  uint16_t
    height,
    width;

  if (IsInvalidSize(Size,sizeof(width)+sizeof(height)))
    return(0);
  width=*reinterpret_cast<const uint16_t *>(Data);
  height=*reinterpret_cast<const uint16_t *>(Data+sizeof(width));
  try
  {
    const Magick::Blob
      blob(Data+sizeof(width)+sizeof(height),Size-(sizeof(width)+
        sizeof(height)));

    image.read(blob);
    image.crop(Magick::Geometry(width,height));
  }
  catch (Magick::Exception &e)
  {
  }
  return(0);
}
