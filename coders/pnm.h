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

#define MagickPNMHeaders \
  MagickCoderHeader("PBM", 0, "P1") \
  MagickCoderHeader("PGM", 0, "P2") \
  MagickCoderHeader("PPM", 0, "P3") \
  MagickCoderHeader("PBM", 0, "P4") \
  MagickCoderHeader("PGM", 0, "P5") \
  MagickCoderHeader("PPM", 0, "P6") \
  MagickCoderHeader("PAM", 0, "P7") \
  MagickCoderHeader("PFM", 0, "PF") \
  MagickCoderHeader("PFM", 0, "Pf")

#define MagickPNMAliases \
  MagickCoderAlias("PNM", "PAM") \
  MagickCoderAlias("PNM", "PBM") \
  MagickCoderAlias("PNM", "PFM") \
  MagickCoderAlias("PNM", "PGM") \
  MagickCoderAlias("PNM", "PPM")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(PNM)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif