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

#define MagickMPEGHeaders \
  MagickCoderHeader("MPEG", 0, "\000\000\001\263") \
  MagickCoderHeader("MPEG", 0, "RIFF")

#define MagickMPEGAliases \
  MagickCoderAlias("MPEG", "3GP") \
  MagickCoderAlias("MPEG", "3G2") \
  MagickCoderAlias("MPEG", "AVI") \
  MagickCoderAlias("MPEG", "FLV") \
  MagickCoderAlias("MPEG", "MKV") \
  MagickCoderAlias("MPEG", "MOV") \
  MagickCoderAlias("MPEG", "MPG") \
  MagickCoderAlias("MPEG", "MP4") \
  MagickCoderAlias("MPEG", "M2V") \
  MagickCoderAlias("MPEG", "M4V") \
  MagickCoderAlias("MPEG", "WMV")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(MPEG)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif