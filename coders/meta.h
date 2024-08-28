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

#define MagickMETAHeaders \
  MagickCoderHeader("8BIMWTEXT", 0, "8\000B\000I\000M\000#") \
  MagickCoderHeader("8BIMTEXT", 0, "8BIM#") \
  MagickCoderHeader("8BIM", 0, "8BIM") \
  MagickCoderHeader("IPTCWTEXT", 0, "\062\000#\000\060\000=\000\042\000&\000#\000\060\000;\000&\000#\000\062\000;\000\042\000") \
  MagickCoderHeader("IPTCTEXT", 0, "2#0=\042&#0;&#2;\042") \
  MagickCoderHeader("IPTC", 0, "\034\002")

#define MagickMETAAliases \
  MagickCoderAlias("META", "8BIM") \
  MagickCoderAlias("META", "8BIMTEXT") \
  MagickCoderAlias("META", "8BIMWTEXT") \
  MagickCoderAlias("META", "APP1") \
  MagickCoderAlias("META", "APP1JPEG") \
  MagickCoderAlias("META", "EXIF") \
  MagickCoderAlias("META", "XMP") \
  MagickCoderAlias("META", "ICM") \
  MagickCoderAlias("META", "ICC") \
  MagickCoderAlias("META", "IPTC") \
  MagickCoderAlias("META", "IPTCTEXT") \
  MagickCoderAlias("META", "IPTCWTEXT")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(META)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
