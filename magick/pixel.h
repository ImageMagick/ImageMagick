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

  MagickCore image constitute methods.
*/
#ifndef _MAGICKCORE_PIXEL_H
#define _MAGICKCORE_PIXEL_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/colorspace.h>
#include <magick/constitute.h>

#define ClampPixelRed(pixel) ClampToQuantum((pixel)->red)
#define ClampPixelGreen(pixel) ClampToQuantum((pixel)->green)
#define ClampPixelBlue(pixel) ClampToQuantum((pixel)->blue)
#define ClampPixelIndex(indexes) ClampToQuantum(*(indexes))
#define ClampPixelOpacity(pixel) ClampToQuantum((pixel)->opacity)
#define GetPixelAlpha(pixel) (QuantumRange-(pixel)->opacity)
#define GetPixelBlack(indexes) (*(indexes))
#define GetPixelBlue(pixel) ((pixel)->blue)
#define GetPixelCb(pixel) ((pixel)->green)
#define GetPixelCr(pixel) ((pixel)->blue)
#define GetPixelCyan(pixel) ((pixel)->red)
#define GetPixelGray(pixel) ((pixel)->red)
#define GetPixelGreen(pixel) ((pixel)->green)
#define GetPixelIndex(indexes) (*(indexes))
#define GetPixelMagenta(pixel) ((pixel)->green)
#define GetPixelNext(pixel)  ((pixel)+1)
#define GetPixelOpacity(pixel) ((pixel)->opacity)
#define GetPixelRed(pixel) ((pixel)->red)
#define GetPixelRGB(pixel,packet) \
{ \
  (packet)->red=GetPixelRed((pixel)); \
  (packet)->green=GetPixelGreen((pixel)); \
  (packet)->blue=GetPixelBlue((pixel)); \
}
#define GetPixelRGBO(pixel,packet) \
{ \
  (packet)->red=GetPixelRed((pixel)); \
  (packet)->green=GetPixelGreen((pixel)); \
  (packet)->blue=GetPixelBlue((pixel)); \
  (packet)->opacity=GetPixelOpacity((pixel)); \
}
#define GetPixelY(pixel) ((pixel)->red)
#define GetPixelYellow(pixel) ((pixel)->blue)
#define SetPixelAlpha(pixel,value) \
  ((pixel)->opacity=(Quantum) (QuantumRange-(value)))
#define SetPixelBlack(indexes,value) (*(indexes)=(Quantum) (value))
#define SetPixelBlue(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelCb(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelCr(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelCyan(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetPixelGray(pixel,value) \
  ((pixel)->red=(pixel)->green=(pixel)->blue=(Quantum) (value))
#define SetPixelGreen(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelIndex(indexes,value) (*(indexes)=(IndexPacket) (value))
#define SetPixelMagenta(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelOpacity(pixel,value) \
  ((pixel)->opacity=(Quantum) (value))
#define SetPixelRed(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetPixelRgb(pixel,packet) \
{ \
  SetPixelRed(pixel,(packet)->red); \
  SetPixelGreen(pixel,(packet)->green); \
  SetPixelBlue(pixel,(packet)->blue); \
}
#define SetPixelRGBA(pixel,packet) \
{ \
  SetPixelRed(pixel,(packet)->red); \
  SetPixelGreen(pixel,(packet)->green); \
  SetPixelBlue(pixel,(packet)->blue); \
  SetPixelAlpha(pixel,(QuantumRange-(packet)->opacity)); \
}
#define SetPixelRGBO(pixel,packet) \
{ \
  SetPixelRed(pixel,(packet)->red); \
  SetPixelGreen(pixel,(packet)->green); \
  SetPixelBlue(pixel,(packet)->blue); \
  SetPixelOpacity(pixel,(packet)->opacity); \
}
#define SetPixelYellow(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelY(pixel,value) ((pixel)->red=(Quantum) (value))

typedef enum
{
  UndefinedInterpolatePixel,
  AverageInterpolatePixel,
  BicubicInterpolatePixel,
  BilinearInterpolatePixel,
  FilterInterpolatePixel,
  IntegerInterpolatePixel,
  MeshInterpolatePixel,
  NearestNeighborInterpolatePixel,
  SplineInterpolatePixel
} InterpolatePixelMethod;

typedef enum
{
  PixelRed = 0,
  PixelCyan = 0,
  PixelGray = 0,
  PixelY = 0,
  PixelGreen = 1,
  PixelMagenta = 1,
  PixelCb = 1,
  PixelBlue = 2,
  PixelYellow = 2,
  PixelCr = 2,
  PixelAlpha = 3,
  PixelBlack = 4,
  PixelIndex = 4,
  MaskPixelComponent = 5
} PixelComponent;

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

typedef struct _CacheView
  CacheView_;

extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *),
  InterpolateMagickPixelPacket(const Image *,const CacheView_ *,
    const InterpolatePixelMethod,const double,const double,MagickPixelPacket *,
    ExceptionInfo *);

extern MagickExport void
  GetMagickPixelPacket(const Image *,MagickPixelPacket *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
