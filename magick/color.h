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

  MagickCore image color methods.
*/
#ifndef _MAGICKCORE_COLOR_H
#define _MAGICKCORE_COLOR_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/pixel.h>
#include <magick/exception.h>

typedef enum
{
  UndefinedCompliance,
  NoCompliance = 0x0000,
  SVGCompliance = 0x0001,
  X11Compliance = 0x0002,
  XPMCompliance = 0x0004,
  AllCompliance = 0x7fffffff
} ComplianceType;

typedef struct _ColorInfo
{
  char
    *path,
    *name;

  ComplianceType
    compliance;

  MagickPixelPacket
    color;

  MagickBooleanType
    stealth;

  struct _ColorInfo
    *previous,
    *next;  /* deprecated, use GetColorInfoList() */

  unsigned long
    signature;
} ColorInfo;

typedef struct _ColorPacket
{
  PixelPacket
    pixel;

  IndexPacket
    index;

  MagickSizeType
    count;
} ColorPacket;

typedef struct _ErrorInfo
{
  double
    mean_error_per_pixel,
    normalized_mean_error,
    normalized_maximum_error;
} ErrorInfo;

extern MagickExport char
  **GetColorList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport const ColorInfo
  *GetColorInfo(const char *,ExceptionInfo *),
  **GetColorInfoList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport ColorPacket
  *GetImageHistogram(const Image *,unsigned long *,ExceptionInfo *);

extern MagickExport Image
  *UniqueImageColors(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  IsColorSimilar(const Image *,const PixelPacket *,const PixelPacket *),
  IsGrayImage(const Image *,ExceptionInfo *),
  IsHistogramImage(const Image *,ExceptionInfo *),
  IsImageSimilar(const Image *,const Image *,long *x,long *y,ExceptionInfo *),
  IsMagickColorSimilar(const MagickPixelPacket *,const MagickPixelPacket *),
  IsMonochromeImage(const Image *,ExceptionInfo *),
  IsOpacitySimilar(const Image *,const PixelPacket *,const PixelPacket *),
  IsOpaqueImage(const Image *,ExceptionInfo *),
  IsPaletteImage(const Image *,ExceptionInfo *),
  ListColorInfo(FILE *,ExceptionInfo *),
  QueryColorDatabase(const char *,PixelPacket *,ExceptionInfo *),
  QueryColorname(const Image *,const PixelPacket *,const ComplianceType,char *,
    ExceptionInfo *),
  QueryMagickColor(const char *,MagickPixelPacket *,ExceptionInfo *),
  QueryMagickColorname(const Image *,const MagickPixelPacket *,
    const ComplianceType,char *,ExceptionInfo *);

extern MagickExport unsigned long
  GetNumberColors(const Image *,FILE *,ExceptionInfo *);

extern MagickExport void
  ConcatenateColorComponent(const MagickPixelPacket *,const ChannelType,
    const ComplianceType,char *),
  DestroyColorList(void),
  GetColorTuple(const MagickPixelPacket *,const MagickBooleanType,char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
