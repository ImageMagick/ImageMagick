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

  MagickCore montage methods.
*/
#ifndef _MAGICKCORE_MONTAGE_H
#define _MAGICKCORE_MONTAGE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedMode,
  FrameMode,
  UnframeMode,
  ConcatenateMode
} MontageMode;

typedef struct _MontageInfo
{
  char
    *geometry,
    *tile,
    *title,
    *frame,
    *texture,
    *font;

  double
    pointsize;

  size_t
    border_width;

  MagickBooleanType
    shadow;

  PixelPacket
    fill,
    stroke,
    background_color,
    border_color,
    matte_color;

  GravityType
    gravity;

  char
    filename[MaxTextExtent];

  MagickBooleanType
    debug;

  size_t
    signature;
} MontageInfo;

extern MagickExport Image
  *MontageImages(const Image *,const MontageInfo *,ExceptionInfo *),
  *MontageImageList(const ImageInfo *,const MontageInfo *,const Image *,
    ExceptionInfo *);

extern MagickExport MontageInfo
  *CloneMontageInfo(const ImageInfo *,const MontageInfo *),
  *DestroyMontageInfo(MontageInfo *);

extern MagickExport void
  GetMontageInfo(const ImageInfo *,MontageInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
