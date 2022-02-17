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

  MagickCore image paint methods.
*/
#ifndef MAGICKCORE_PAINT_H
#define MAGICKCORE_PAINT_H

#include "MagickCore/color.h"
#include "MagickCore/draw.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport Image
  *OilPaintImage(const Image *,const double,const double,ExceptionInfo *);

extern MagickExport MagickBooleanType
  FloodfillPaintImage(Image *,const DrawInfo *,const PixelInfo *,const ssize_t,
    const ssize_t,const MagickBooleanType,ExceptionInfo *),
  GradientImage(Image *,const GradientType,const SpreadMethod,const StopInfo *,
    const size_t,ExceptionInfo *),
  OpaquePaintImage(Image *,const PixelInfo *,const PixelInfo *,
    const MagickBooleanType,ExceptionInfo *),
  TransparentPaintImage(Image *,const PixelInfo *,
    const Quantum,const MagickBooleanType,ExceptionInfo *),
  TransparentPaintImageChroma(Image *,const PixelInfo *,
    const PixelInfo *,const Quantum,const MagickBooleanType,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
