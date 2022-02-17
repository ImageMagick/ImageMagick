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

#define MagickVIDEOHeaders \
  MagickCoderHeader("VIDEO", 0, "\000\000\001\263")

#define MagickVIDEOAliases \
  MagickCoderAlias("VIDEO", "3GP") \
  MagickCoderAlias("VIDEO", "3G2") \
  MagickCoderAlias("VIDEO", "APNG") \
  MagickCoderAlias("VIDEO", "AVI") \
  MagickCoderAlias("VIDEO", "FLV") \
  MagickCoderAlias("VIDEO", "MKV") \
  MagickCoderAlias("VIDEO", "MOV") \
  MagickCoderAlias("VIDEO", "MPEG") \
  MagickCoderAlias("VIDEO", "MPG") \
  MagickCoderAlias("VIDEO", "MP4") \
  MagickCoderAlias("VIDEO", "M2V") \
  MagickCoderAlias("VIDEO", "M4V") \
  MagickCoderAlias("VIDEO", "WEBM") \
  MagickCoderAlias("VIDEO", "WMV")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(VIDEO)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
