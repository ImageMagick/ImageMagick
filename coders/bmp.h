/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
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

#define MagickBMPHeaders \
  MagickCoderHeader("BMP", 0, "BA") \
  MagickCoderHeader("BMP", 0, "BM") \
  MagickCoderHeader("BMP", 0, "CI") \
  MagickCoderHeader("BMP", 0, "CP") \
  MagickCoderHeader("BMP", 0, "IC") \
  MagickCoderHeader("BMP", 0, "IP")

#define MagickBMPAliases \
  MagickCoderAlias("BMP", "BMP2") \
  MagickCoderAlias("BMP", "BMP3")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(BMP)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif