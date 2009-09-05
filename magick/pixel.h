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

  MagickCore image constitute methods.
*/
#ifndef _MAGICKCORE_PIXEL_H
#define _MAGICKCORE_PIXEL_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/colorspace.h>
#include <magick/constitute.h>

typedef struct _LongPixelPacket
{
  unsigned long
    red,
    green,
    blue,
    opacity,
    index;
} LongPixelPacket;

typedef struct _MagickPixelPacket
{
  ClassType
    storage_class;

  ColorspaceType
    colorspace;

  MagickBooleanType
    matte;

  double
    fuzz;

  unsigned long
    depth;

  MagickRealType
    red,
    green,
    blue,
    opacity,
    index;
} MagickPixelPacket;

typedef Quantum IndexPacket;

typedef struct _PixelPacket
{
#if defined(MAGICKCORE_WORDS_BIGENDIAN)
  Quantum
    red,
    green,
    blue,
    opacity;
#else
  Quantum
    blue,
    green,
    red,
    opacity;
#endif
} PixelPacket;

extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const long,const long,const unsigned long,
    const unsigned long,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const long,const long,const unsigned long,
    const unsigned long,const char *,const StorageType,const void *);

extern MagickExport void
  GetMagickPixelPacket(const Image *,MagickPixelPacket *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
