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

#define MagickPNGHeaders \
  MagickCoderHeader("PNG", 0, "\211PNG\r\n\032\n") \
  MagickCoderHeader("JNG", 0, "\213JNG\r\n\032\n") \
  MagickCoderHeader("MNG", 0, "\212MNG\r\n\032\n")

#define MagickPNGAliases \
  MagickCoderAlias("PNG", "MNG") \
  MagickCoderAlias("PNG", "PNG8") \
  MagickCoderAlias("PNG", "PNG24") \
  MagickCoderAlias("PNG", "PNG32") \
  MagickCoderAlias("PNG", "PNG48") \
  MagickCoderAlias("PNG", "PNG64") \
  MagickCoderAlias("PNG", "PNG00") \
  MagickCoderAlias("PNG", "JNG")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(PNG)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif