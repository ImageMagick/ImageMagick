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

#define MagickDNGHeaders \
  MagickCoderHeader("CR2", 0, "\115\115\000\052\000\020\000\000\122\103\002") \
  MagickCoderHeader("CR2", 0, "\111\111\052\000\020\000\000\000\103\122\002") \
  MagickCoderHeader("CR3", 4, "ftypcrx ") \
  MagickCoderHeader("CRW", 0, "II\x1a\x00\x00\x00HEAPCCDR") \
  MagickCoderHeader("ORF", 0, "IIRO\x08\x00\x00\x00") \
  MagickCoderHeader("MRW", 0, "\x00MRM") \
  MagickCoderHeader("RAF", 0, "FUJIFILMCCD-RAW ")

#define MagickDNGAliases \
  MagickCoderAlias("DNG", "3FR") \
  MagickCoderAlias("DNG", "ARW") \
  MagickCoderAlias("DNG", "CR2") \
  MagickCoderAlias("DNG", "CR3") \
  MagickCoderAlias("DNG", "CRW") \
  MagickCoderAlias("DNG", "DCR") \
  MagickCoderAlias("DNG", "DCRAW") \
  MagickCoderAlias("DNG", "ERF") \
  MagickCoderAlias("DNG", "IIQ") \
  MagickCoderAlias("DNG", "KDC") \
  MagickCoderAlias("DNG", "K25") \
  MagickCoderAlias("DNG", "MEF") \
  MagickCoderAlias("DNG", "MRW") \
  MagickCoderAlias("DNG", "NEF") \
  MagickCoderAlias("DNG", "NRW") \
  MagickCoderAlias("DNG", "ORF") \
  MagickCoderAlias("DNG", "PEF") \
  MagickCoderAlias("DNG", "RAF") \
  MagickCoderAlias("DNG", "RAW") \
  MagickCoderAlias("DNG", "RMF") \
  MagickCoderAlias("DNG", "RW2") \
  MagickCoderAlias("DNG", "SRF") \
  MagickCoderAlias("DNG", "SR2") \
  MagickCoderAlias("DNG", "X3F")

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickCoderExports(DNG)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
