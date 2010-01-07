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

  MagickCore image constitute methods.
*/
#ifndef _MAGICKCORE_PIXEL_H
#define _MAGICKCORE_PIXEL_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/colorspace.h>
#include <magick/constitute.h>

#define GetRedSample(p) ((p)->red)
#define GetGreenSample(p) ((p)->green)
#define GetBlueSample(p) ((p)->blue)
#define GetOpacitySample(p) ((p)->opacity)

#define SetRedSample(q,sample) ((q)->red=(sample))
#define SetGreenSample(q,sample) ((q)->green=(sample))
#define SetBlueSample(q,sample) ((q)->blue=(sample))
#define SetOpacitySample(q,sample) ((q)->opacity=(sample))

#define GetGraySample(p) ((p)->red)
#define SetGraySample(q,sample) ((q)->red=(q)->green=(q)->blue=(sample))

#define GetYSample(p) ((p)->red)
#define GetCbSample(p) ((p)->green)
#define GetCrSample(p) ((p)->blue)

#define SetYSample(q,sample) ((q)->red=(sample))
#define SetCbSample(q,sample) ((q)->green=(sample))
#define SetCrSample(q,sample) ((q)->blue=(sample))

#define GetCyanSample(p) ((p)->red)
#define GetMagentaSample(p) ((p)->green)
#define GetYellowSample(p) ((p)->blue)
#define GetBlackSample(p) ((p)->opacity)

#define SetCyanSample(q,sample) ((q)->red=(sample))
#define SetMagentaSample(q,sample) ((q)->green=(sample))
#define SetYellowSample(q,sample) ((q)->blue=(sample))
#define SetBlackSample(q,sample) ((q)->opacity=(sample))

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
