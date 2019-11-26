/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
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

// The pattern for JXL JPEG1 recompression for now, the main pattern will be
// added when full decoder support is added.
#define MagickJXLHeaders \
  MagickCoderHeader("JXL", 0, "\x0a\x04\x42\xd2\xd5\x4e")

#define MagickJXLAliases

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(JXL)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
