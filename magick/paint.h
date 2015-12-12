/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image paint methods.
*/
#ifndef _MAGICKCORE_PAINT_H
#define _MAGICKCORE_PAINT_H

#include "magick/color.h"
#include "magick/draw.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport Image
  *OilPaintImage(const Image *,const double,ExceptionInfo *);

extern MagickExport MagickBooleanType
  FloodfillPaintImage(Image *,const ChannelType,const DrawInfo *,
    const MagickPixelPacket *,const ssize_t,const ssize_t,
    const MagickBooleanType),
  GradientImage(Image *,const GradientType,const SpreadMethod,
    const PixelPacket *,const PixelPacket *),
  OpaquePaintImage(Image *,const MagickPixelPacket *,const MagickPixelPacket *,
    const MagickBooleanType),
  OpaquePaintImageChannel(Image *,const ChannelType,const MagickPixelPacket *,
    const MagickPixelPacket *,const MagickBooleanType),
  TransparentPaintImage(Image *,const MagickPixelPacket *,
    const Quantum,const MagickBooleanType),
  TransparentPaintImageChroma(Image *,const MagickPixelPacket *,
    const MagickPixelPacket *,const Quantum,const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
