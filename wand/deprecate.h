/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
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
  DrawGetFillAlpha(const DrawingWand *),
  DrawGetStrokeAlpha(const DrawingWand *);

extern WandExport DrawInfo
  *DrawPeekGraphicWand(const DrawingWand *);

extern WandExport char
  *MagickDescribeImage(MagickWand *),
  *MagickGetImageAttribute(MagickWand *,const char *),
  *PixelIteratorGetException(const PixelIterator *,ExceptionType *);

extern WandExport long
  MagickGetImageIndex(MagickWand *);

extern WandExport MagickBooleanType
  MagickClipPathImage(MagickWand *,const char *,const MagickBooleanType),
  MagickColorFloodfillImage(MagickWand *,const PixelWand *,const double,
    const PixelWand *,const long,const long),
  MagickGetImageChannelExtrema(MagickWand *,const ChannelType,unsigned long *,
    unsigned long *),
  MagickGetImageExtrema(MagickWand *,unsigned long *,unsigned long *),
  MagickGetImageMatte(MagickWand *),
  MagickGetImagePixels(MagickWand *,const long,const long,const unsigned long,
    const unsigned long,const char *,const StorageType,void *),
  MagickMapImage(MagickWand *,const MagickWand *,const MagickBooleanType),
  MagickMatteFloodfillImage(MagickWand *,const double,const double,
    const PixelWand *,const long,const long),
  MagickOpaqueImage(MagickWand *,const PixelWand *,const PixelWand *,
    const double),
  MagickPaintFloodfillImage(MagickWand *,const ChannelType,const PixelWand *,
    const double,const PixelWand *,const long,const long),
  MagickPaintOpaqueImage(MagickWand *,const PixelWand *,const PixelWand *,
    const double),
  MagickPaintOpaqueImageChannel(MagickWand *,const ChannelType,
    const PixelWand *,const PixelWand *,const double),
  MagickPaintTransparentImage(MagickWand *,const PixelWand *,const double,
    const double),
  MagickSetImageAttribute(MagickWand *,const char *,const char *),
  MagickSetImageIndex(MagickWand *,const long),
  MagickSetImageOption(MagickWand *,const char *,const char *,const char *),
  MagickSetImagePixels(MagickWand *,const long,const long,const unsigned long,
    const unsigned long,const char *,const StorageType,const void *),
  MagickTransparentImage(MagickWand *,const PixelWand *,const double,
    const double);

extern WandExport MagickWand
  *MagickFlattenImages(MagickWand *),
  *MagickMosaicImages(MagickWand *),
  *MagickRegionOfInterestImage(MagickWand *,const unsigned long,
    const unsigned long,const long,const long);

extern WandExport MagickSizeType
  MagickGetImageSize(MagickWand *);

extern WandExport PixelWand
  **PixelGetNextRow(PixelIterator *);

extern WandExport unsigned char
  *MagickWriteImageBlob(MagickWand *,size_t *);

extern WandExport void
  DrawPopGraphicContext(DrawingWand *),
  DrawPushGraphicContext(DrawingWand *),
  DrawSetFillAlpha(DrawingWand *,const double),
  DrawSetStrokeAlpha(DrawingWand *,const double);

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
