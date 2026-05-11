/*
  Copyright @ 2026 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/license/

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

static const char *kProfileNames[] = {
  "exif", "xmp", "iptc", "icc", "8bim", "app1", "app13"
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data,size_t Size)
{
  if (IsInvalidSize(Size,2))
    return(0);

  const unsigned int sel=Data[0] % (sizeof(kProfileNames) /
    sizeof(kProfileNames[0]));
  const Magick::Blob blob(Data + 1, Size - 1);

  try
  {
    Magick::Image image("16x16","white");
    image.profile(kProfileNames[sel],blob);
  }
#if defined(BUILD_MAIN)
  catch (Magick::Exception &e)
  {
    std::cout << "Exception when reading profile: " << e.what() << std::endl;
  }
#else
  catch (Magick::Exception)
  {
  }
#endif
  return(0);
}
