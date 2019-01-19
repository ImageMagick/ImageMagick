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

#define MagickCALSHeaders \
  MagickCoderHeader("CALS", 0, "srcdocid:") \
  MagickCoderHeader("CALS", 8, "rorient:") \
  MagickCoderHeader("CALS", 9, "rorient:") \
  MagickCoderHeader("CALS", 21, "version: MIL-STD-1840")

#define MagickCALSAliases \
  MagickCoderAlias("CALS", "CAL")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(CALS)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif