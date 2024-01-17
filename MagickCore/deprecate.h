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

  MagickCore deprecated methods.
*/
#ifndef MAGICKCORE_DEPRECATE_H
#define MAGICKCORE_DEPRECATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

#include "MagickCore/magick.h"

typedef int
  *(*BlobFifo)(const Image *,const void *,const size_t);

extern MagickExport MagickBooleanType
  GetMagickSeekableStream(const MagickInfo *);

#if defined(MAGICKCORE_WINGDI32_DELEGATE)
extern MagickExport void
  *CropImageToHBITMAP(Image *,const RectangleInfo *,ExceptionInfo *),
  *ImageToHBITMAP(Image *,ExceptionInfo *);
#endif

extern MagickExport void 
  InitializePixelChannelMap(Image *) magick_attribute((deprecated));

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
