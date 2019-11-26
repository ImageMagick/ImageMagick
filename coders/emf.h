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

#define MagickEMFHeaders \
  MagickCoderHeader("EMF", 40, "\040\105\115\106\000\000\001\000") \
  MagickCoderHeader("WMF", 0, "\327\315\306\232") \
  MagickCoderHeader("WMF", 0, "\001\000\011\000")

#define MagickEMFAliases \
  MagickCoderAlias("EMF", "WMF")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(EMF)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif