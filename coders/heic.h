/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
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

#include "coders/coders-private.h"

#define MagickHEICHeaders \
  MagickCoderHeader("AVCI", 4, "ftypavci") \
  MagickCoderHeader("AVIF", 4, "ftypavif") \
  MagickCoderHeader("AVIF", 4, "ftypavis") \
  MagickCoderHeader("HEIC", 4, "ftypheic") \
  MagickCoderHeader("HEIC", 4, "ftypheix") \
  MagickCoderHeader("HEIC", 4, "ftyphevc") \
  MagickCoderHeader("HEIC", 4, "ftypheim") \
  MagickCoderHeader("HEIC", 4, "ftypheis") \
  MagickCoderHeader("HEIC", 4, "ftyphevm") \
  MagickCoderHeader("HEIC", 4, "ftyphevs") \
  MagickCoderHeader("HEIC", 4, "ftypmif1") \
  MagickCoderHeader("HEIC", 4, "ftypmsf1")

#define MagickHEICAliases \
  MagickCoderAlias("HEIC", "AVCI") \
  MagickCoderAlias("HEIC", "AVIF") \
  MagickCoderAlias("HEIC", "HEIF")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(HEIC)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
