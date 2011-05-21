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

#define ClampRedPixelComponent(pixel) ClampToQuantum((pixel)->red)
#define ClampGreenPixelComponent(pixel) ClampToQuantum((pixel)->green)
#define ClampBluePixelComponent(pixel) ClampToQuantum((pixel)->blue)
#define ClampIndexPixelComponent(indexes) ClampToQuantum(*(indexes))
#define ClampOpacityPixelComponent(pixel) ClampToQuantum((pixel)->opacity)
#define GetAlphaPixelComponent(pixel) (QuantumRange-(pixel)->opacity)
#define GetBlackPixelComponent(indexes) (*(indexes))
#define GetBluePixelComponent(pixel) ((pixel)->blue)
#define GetCbPixelComponent(pixel) ((pixel)->green)
#define GetCrPixelComponent(pixel) ((pixel)->blue)
#define GetCyanPixelComponent(pixel) ((pixel)->red)
#define GetGrayPixelComponent(pixel) ((pixel)->red)
#define GetGreenPixelComponent(pixel) ((pixel)->green)
#define GetIndexPixelComponent(indexes) (*(indexes))
#define GetMagentaPixelComponent(pixel) ((pixel)->green)
#define GetNextPixel(pixel)  ((pixel)+1)
#define GetOpacityPixelComponent(pixel) ((pixel)->opacity)
#define GetRedPixelComponent(pixel) ((pixel)->red)
#define GetYPixelComponent(pixel) ((pixel)->red)
#define GetYellowPixelComponent(pixel) ((pixel)->blue)
#define SetAlphaPixelComponent(pixel,value) \
  ((pixel)->opacity=(Quantum) (QuantumRange-(value)))
#define SetBlackPixelComponent(indexes,value) (*(indexes)=(Quantum) (value))
#define SetBluePixelComponent(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetCbPixelComponent(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetCrPixelComponent(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetCyanPixelComponent(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetGrayPixelComponent(pixel,value) \
  ((pixel)->red=(pixel)->green=(pixel)->blue=(Quantum) (value))
#define SetGreenPixelComponent(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetIndexPixelComponent(indexes,value) (*(indexes)=(IndexPacket) (value))
#define SetMagentaPixelComponent(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetOpacityPixelComponent(pixel,value) \
  ((pixel)->opacity=(Quantum) (value))
#define SetRedPixelComponent(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetRGBPixelComponent(destination,source) \
{ \
  SetRedPixelComponent(destination,GetRedPixelComponent(source)); \
  SetGreenPixelComponent(destination,GetGreenPixelComponent(source)); \
  SetBluePixelComponent(destination,GetBluePixelComponent(source)); \
}
#define SetRGBAPixelComponent(destination,source) \
{ \
  SetRedPixelComponent(destination,GetRedPixelComponent(source)); \
  SetGreenPixelComponent(destination,GetGreenPixelComponent(source)); \
  SetBluePixelComponent(destination,GetBluePixelComponent(source)); \
  SetAlphaPixelComponent(destination,GetAlphaPixelComponent(source)); \
}
#define SetRGBOPixelComponent(destination,source) \
{ \
  SetRedPixelComponent(destination,GetRedPixelComponent(source)); \
  SetGreenPixelComponent(destination,GetGreenPixelComponent(source)); \
  SetBluePixelComponent(destination,GetBluePixelComponent(source)); \
  SetOpacityPixelComponent(destination,GetOpacityPixelComponent(source)); \
}
#define SetYellowPixelComponent(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetYPixelComponent(pixel,value) ((pixel)->red=(Quantum) (value))

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
  RedPixelComponent = 0,
  CyanPixelComponent = 0,
  GrayPixelComponent = 0,
  YPixelComponent = 0,
  GreenPixelComponent = 1,
  MagentaPixelComponent = 1,
  CbPixelComponent = 1,
  BluePixelComponent = 2,
  YellowPixelComponent = 2,
  CrPixelComponent = 2,
  AlphaPixelComponent = 3,
  BlackPixelComponent = 4,
  IndexPixelComponent = 4,
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
