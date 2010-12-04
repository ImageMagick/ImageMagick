/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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

typedef struct _PixelView
  PixelView;

typedef MagickBooleanType
  (*DuplexTransferPixelViewMethod)(const PixelView *,const PixelView *,
    PixelView *,void *),
  (*GetPixelViewMethod)(const PixelView *,void *),
  (*SetPixelViewMethod)(PixelView *,void *),
  (*TransferPixelViewMethod)(const PixelView *,PixelView *,void *),
  (*UpdatePixelViewMethod)(PixelView *,void *);

extern WandExport char
  *GetPixelViewException(const PixelView *,ExceptionType *)
    magick_attribute((deprecated));

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

extern WandExport ssize_t
  MagickGetImageIndex(MagickWand *) magick_attribute((deprecated));

extern WandExport MagickBooleanType
  DuplexTransferPixelViewIterator(PixelView *,PixelView *,PixelView *,
    DuplexTransferPixelViewMethod,void *) magick_attribute((deprecated)),
  GetPixelViewIterator(PixelView *,GetPixelViewMethod,void *)
    magick_attribute((deprecated)),
  IsPixelView(const PixelView *) magick_attribute((deprecated)),
  MagickClipPathImage(MagickWand *,const char *,const MagickBooleanType)
    magick_attribute((deprecated)),
  MagickColorFloodfillImage(MagickWand *,const PixelWand *,const double,
    const PixelWand *,const ssize_t,const ssize_t)
    magick_attribute((deprecated)),
  MagickGetImageChannelExtrema(MagickWand *,const ChannelType,size_t *,
    size_t *) magick_attribute((deprecated)),
  MagickGetImageExtrema(MagickWand *,size_t *,size_t *)
    magick_attribute((deprecated)),
  MagickGetImageMatte(MagickWand *) magick_attribute((deprecated)),
  MagickGetImagePixels(MagickWand *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *)
    magick_attribute((deprecated)),
  MagickMapImage(MagickWand *,const MagickWand *,const MagickBooleanType)
    magick_attribute((deprecated)),
  MagickMatteFloodfillImage(MagickWand *,const double,const double,
    const PixelWand *,const ssize_t,const ssize_t)
    magick_attribute((deprecated)),
  MagickOpaqueImage(MagickWand *,const PixelWand *,const PixelWand *,
    const double) magick_attribute((deprecated)),
  MagickPaintFloodfillImage(MagickWand *,const ChannelType,const PixelWand *,
    const double,const PixelWand *,const ssize_t,const ssize_t)
    magick_attribute((deprecated)),
  MagickPaintOpaqueImage(MagickWand *,const PixelWand *,const PixelWand *,
    const double) magick_attribute((deprecated)),
  MagickPaintOpaqueImageChannel(MagickWand *,const ChannelType,
    const PixelWand *,const PixelWand *,const double)
    magick_attribute((deprecated)),
  MagickPaintTransparentImage(MagickWand *,const PixelWand *,const double,
    const double) magick_attribute((deprecated)),
  MagickRecolorImage(MagickWand *,const size_t,const double *)
    magick_attribute((deprecated)),
  MagickSetImageAttribute(MagickWand *,const char *,const char *)
    magick_attribute((deprecated)),
  MagickSetImageIndex(MagickWand *,const ssize_t)
    magick_attribute((deprecated)),
  MagickSetImageOption(MagickWand *,const char *,const char *,const char *)
    magick_attribute((deprecated)),
  MagickSetImagePixels(MagickWand *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *)
    magick_attribute((deprecated)),
  MagickTransparentImage(MagickWand *,const PixelWand *,const double,
    const double) magick_attribute((deprecated)),
  SetPixelViewIterator(PixelView *,SetPixelViewMethod,void *)
    magick_attribute((deprecated)),
  TransferPixelViewIterator(PixelView *,PixelView *,TransferPixelViewMethod,
    void *) magick_attribute((deprecated)),
  UpdatePixelViewIterator(PixelView *,UpdatePixelViewMethod,void *)
    magick_attribute((deprecated));

extern WandExport MagickWand
  *GetPixelViewWand(const PixelView *) magick_attribute((deprecated)),
  *MagickAverageImages(MagickWand *) magick_attribute((deprecated)),
  *MagickFlattenImages(MagickWand *) magick_attribute((deprecated)),
  *MagickMaximumImages(MagickWand *) magick_attribute((deprecated)),
  *MagickMinimumImages(MagickWand *) magick_attribute((deprecated)),
  *MagickMosaicImages(MagickWand *) magick_attribute((deprecated)),
  *MagickRegionOfInterestImage(MagickWand *,const size_t,const size_t,
    const ssize_t,const ssize_t) magick_attribute((deprecated));

extern WandExport MagickSizeType
  MagickGetImageSize(MagickWand *) magick_attribute((deprecated));

extern WandExport PixelView
  *ClonePixelView(const PixelView *) magick_attribute((deprecated)),
  *DestroyPixelView(PixelView *) magick_attribute((deprecated)),
  *NewPixelView(MagickWand *) magick_attribute((deprecated)),
  *NewPixelViewRegion(MagickWand *,const ssize_t,const ssize_t,const size_t,
    const size_t) magick_attribute((deprecated));

extern WandExport PixelWand
  **GetPixelViewPixels(const PixelView *) magick_attribute((deprecated)),
  **PixelGetNextRow(PixelIterator *) magick_attribute((deprecated));

extern WandExport size_t
  GetPixelViewHeight(const PixelView *) magick_attribute((deprecated)),
  GetPixelViewWidth(const PixelView *) magick_attribute((deprecated));

extern WandExport ssize_t
  GetPixelViewX(const PixelView *) magick_attribute((deprecated)),
  GetPixelViewY(const PixelView *) magick_attribute((deprecated));

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
