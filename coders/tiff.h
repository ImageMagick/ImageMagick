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

#define MagickTIFFHeaders \
  MagickCoderHeader("TIFF", 0, "\115\115\000\052") \
  MagickCoderHeader("TIFF", 0, "\111\111\052\000") \
  MagickCoderHeader("TIFF64", 0, "\115\115\000\053\000\010\000\000") \
  MagickCoderHeader("TIFF64", 0, "\111\111\053\000\010\000\000\000")

#define MagickTIFFAliases \
  MagickCoderAlias("TIFF", "GROUP4") \
  MagickCoderAlias("TIFF", "PTIF") \
  MagickCoderAlias("TIFF", "TIF") \
  MagickCoderAlias("TIFF", "TIFF64")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(TIFF)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif