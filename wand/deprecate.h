/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore deprecated methods.
*/
#ifndef _MAGICKWAND_DEPRECATE_H
#define _MAGICKWAND_DEPRECATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

#include "wand/drawing-wand.h"
#include "wand/magick-wand.h"
#include "wand/pixel-iterator.h"
#include "wand/pixel-wand.h"

typedef struct _DrawingWand
  *DrawContext;

extern WandExport double
  DrawGetFillAlpha(const DrawingWand *) magick_attribute((deprecated)),
  DrawGetStrokeAlpha(const DrawingWand *) magick_attribute((deprecated));

extern WandExport DrawInfo
  *DrawPeekGraphicWand(const DrawingWand *) magick_attribute((deprecated));

extern WandExport char
  *MagickDescribeImage(MagickWand *) magick_attribute((deprecated)),
  *MagickGetImageAttribute(MagickWand *,const char *)
    magick_attribute((deprecated)),
  *PixelIteratorGetException(const PixelIterator *,ExceptionType *)
    magick_attribute((deprecated));

extern WandExport long
  MagickGetImageIndex(MagickWand *) magick_attribute((deprecated));

extern WandExport MagickBooleanType
  MagickClipPathImage(MagickWand *,const char *,const MagickBooleanType)
    magick_attribute((deprecated)),
  MagickColorFloodfillImage(MagickWand *,const PixelWand *,const double,
    const PixelWand *,const long,const long) magick_attribute((deprecated)),
  MagickGetImageChannelExtrema(MagickWand *,const ChannelType,unsigned long *,
    unsigned long *) magick_attribute((deprecated)),
  MagickGetImageExtrema(MagickWand *,unsigned long *,unsigned long *)
    magick_attribute((deprecated)),
  MagickGetImageMatte(MagickWand *) magick_attribute((deprecated)),
  MagickGetImagePixels(MagickWand *,const long,const long,const unsigned long,
    const unsigned long,const char *,const StorageType,void *)
    magick_attribute((deprecated)),
  MagickMapImage(MagickWand *,const MagickWand *,const MagickBooleanType)
    magick_attribute((deprecated)),
  MagickMatteFloodfillImage(MagickWand *,const double,const double,
    const PixelWand *,const long,const long) magick_attribute((deprecated)),
  MagickOpaqueImage(MagickWand *,const PixelWand *,const PixelWand *,
    const double) magick_attribute((deprecated)),
  MagickPaintFloodfillImage(MagickWand *,const ChannelType,const PixelWand *,
    const double,const PixelWand *,const long,const long)
    magick_attribute((deprecated)),
  MagickPaintOpaqueImage(MagickWand *,const PixelWand *,const PixelWand *,
    const double) magick_attribute((deprecated)),
  MagickPaintOpaqueImageChannel(MagickWand *,const ChannelType,
    const PixelWand *,const PixelWand *,const double)
    magick_attribute((deprecated)),
  MagickPaintTransparentImage(MagickWand *,const PixelWand *,const double,
    const double) magick_attribute((deprecated)),
  MagickSetImageAttribute(MagickWand *,const char *,const char *)
    magick_attribute((deprecated)),
  MagickSetImageIndex(MagickWand *,const long) magick_attribute((deprecated)),
  MagickSetImageOption(MagickWand *,const char *,const char *,const char *)
    magick_attribute((deprecated)),
  MagickSetImagePixels(MagickWand *,const long,const long,const unsigned long,
    const unsigned long,const char *,const StorageType,const void *)
    magick_attribute((deprecated)),
  MagickTransparentImage(MagickWand *,const PixelWand *,const double,
    const double) magick_attribute((deprecated));

extern WandExport MagickWand
  *MagickFlattenImages(MagickWand *) magick_attribute((deprecated)),
  *MagickMosaicImages(MagickWand *) magick_attribute((deprecated)),
  *MagickRegionOfInterestImage(MagickWand *,const unsigned long,
    const unsigned long,const long,const long) magick_attribute((deprecated));

extern WandExport MagickSizeType
  MagickGetImageSize(MagickWand *) magick_attribute((deprecated));

extern WandExport PixelWand
  **PixelGetNextRow(PixelIterator *) magick_attribute((deprecated));

extern WandExport unsigned char
  *MagickWriteImageBlob(MagickWand *,size_t *) magick_attribute((deprecated));

extern WandExport void
  DrawPopGraphicContext(DrawingWand *) magick_attribute((deprecated)),
  DrawPushGraphicContext(DrawingWand *) magick_attribute((deprecated)),
  DrawSetFillAlpha(DrawingWand *,const double) magick_attribute((deprecated)),
  DrawSetStrokeAlpha(DrawingWand *,const double) magick_attribute((deprecated));

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
