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

  MagickCore image constitute methods.
*/
#ifndef _MAGICKCORE_PIXEL_H
#define _MAGICKCORE_PIXEL_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/colorspace.h>
#include <magick/constitute.h>

#define ClampRedPixelComponent(p) ClampToQuantum((p)->red)
#define ClampGreenPixelComponent(p) ClampToQuantum((p)->green)
#define ClampBluePixelComponent(p) ClampToQuantum((p)->blue)
#define ClampOpacityPixelComponent(p) ClampToQuantum((p)->opacity)
#define ClampIndexPixelComponent(p) ClampToQuantum((p)->index)

#define GetRedPixelComponent(p) ((p)->red)
#define GetGreenPixelComponent(p) ((p)->green)
#define GetBluePixelComponent(p) ((p)->blue)
#define GetOpacityPixelComponent(p) ((p)->opacity)
#define GetAlphaPixelComponent(p) (QuantumRange-(p)->opacity)
#define GetIndexPixelComponent(p) ((p)->index)

#define SetRedPixelComponent(q,value) ((q)->red=(value))
#define SetGreenPixelComponent(q,value) ((q)->green=(value))
#define SetBluePixelComponent(q,value) ((q)->blue=(value))
#define SetOpacityPixelComponent(q,value) ((q)->opacity=(value))
#define SetAlphaPixelComponent(q,value) \
  ((q)->opacity=(Quantum) (QuantumRange-(value)))
#define SetIndexPixelComponent(q,value) ((q)->index=(value))

#define GetGrayPixelComponent(p) ((p)->red)
#define SetGrayPixelComponent(q,value) ((q)->red=(q)->green=(q)->blue=(value))

#define GetYPixelComponent(p) ((p)->red)
#define GetCbPixelComponent(p) ((p)->green)
#define GetCrPixelComponent(p) ((p)->blue)

#define SetYPixelComponent(q,value) ((q)->red=(value))
#define SetCbPixelComponent(q,value) ((q)->green=(value))
#define SetCrPixelComponent(q,value) ((q)->blue=(value))

#define GetCyanPixelComponent(p) ((p)->red)
#define GetMagentaPixelComponent(p) ((p)->green)
#define GetYellowPixelComponent(p) ((p)->blue)
#define GetBlackPixelComponent(p,x) (p[x])

#define SetCyanPixelComponent(q,value) ((q)->red=(value))
#define SetMagentaPixelComponent(q,value) ((q)->green=(value))
#define SetYellowPixelComponent(q,value) ((q)->blue=(value))
#define SetBlackPixelComponent(p,x,value) (p[x]=(value))

typedef struct _DoublePixelPacket
{
  double
    red,
    green,
    blue,
    opacity,
    index;
} DoublePixelPacket;

typedef struct _LongPixelPacket
{
  unsigned int
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

  size_t
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
#define MAGICK_PIXEL_RGBA  1
  Quantum
    red,
    green,
    blue,
    opacity;
#else
#define MAGICK_PIXEL_BGRA  1
  Quantum
    blue,
    green,
    red,
    opacity;
#endif
} PixelPacket;

extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *);

extern MagickExport void
  GetMagickPixelPacket(const Image *,MagickPixelPacket *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
