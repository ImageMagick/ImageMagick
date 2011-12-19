/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore histogram methods.
*/
#ifndef _MAGICKCORE_HISTOGRAM_H
#define _MAGICKCORE_HISTOGRAM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _ColorPacket
{
  PixelPacket
    pixel;

  IndexPacket
    index;

  MagickSizeType
    count;
} ColorPacket;

extern MagickExport ColorPacket
  *GetImageHistogram(const Image *,size_t *,ExceptionInfo *);

extern MagickExport Image
  *UniqueImageColors(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  IsHistogramImage(const Image *,ExceptionInfo *),
  IsPaletteImage(const Image *,ExceptionInfo *),
  MinMaxStretchImage(Image *,const ChannelType,const double,const double);

extern MagickExport size_t
  GetNumberColors(const Image *,FILE *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
