/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
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

#include "magick/colorspace.h"
#include "magick/constitute.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Pixel enum declarations.
*/
typedef enum
{
  UndefinedInterpolatePixel,
  AverageInterpolatePixel,           /* Average 4 nearest neighbours */
  BicubicInterpolatePixel,           /* Catmull-Rom interpolation */
  BilinearInterpolatePixel,          /* Triangular filter interpolation */
  FilterInterpolatePixel,            /* Use resize filter - (very slow) */
  IntegerInterpolatePixel,           /* Integer (floor) interpolation */
  MeshInterpolatePixel,              /* Triangular mesh interpolation */
  NearestNeighborInterpolatePixel,   /* Nearest neighbour only */
  SplineInterpolatePixel,            /* Cubic Spline (blurred) interpolation */
  Average9InterpolatePixel,          /* Average 9 nearest neighbours */
  Average16InterpolatePixel,         /* Average 16 nearest neighbours */
  BlendInterpolatePixel,             /* blend of nearest 1, 2 or 4 pixels */
  BackgroundInterpolatePixel,        /* just return background color */
  CatromInterpolatePixel             /* Catmull-Rom interpolation */
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

typedef enum
{
  UndefinedPixelIntensityMethod = 0,
  AveragePixelIntensityMethod,
  BrightnessPixelIntensityMethod,
  LightnessPixelIntensityMethod,
  Rec601LumaPixelIntensityMethod,
  Rec601LuminancePixelIntensityMethod,
  Rec709LumaPixelIntensityMethod,
  Rec709LuminancePixelIntensityMethod,
  RMSPixelIntensityMethod,
  MSPixelIntensityMethod
} PixelIntensityMethod;

/*
  Pixel typedef declarations.
*/
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

typedef struct _QuantumPixelPacket
{
  Quantum
    red,
    green,
    blue,
    opacity,
    index;
} QuantumPixelPacket;

typedef struct _CacheView
  CacheView_;

/*
  Pixel method declarations.
*/
extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *),
  InterpolateMagickPixelPacket(const Image *,const CacheView_ *,
    const InterpolatePixelMethod,const double,const double,MagickPixelPacket *,
    ExceptionInfo *);

extern MagickExport MagickPixelPacket
  *CloneMagickPixelPacket(const MagickPixelPacket *);

extern MagickExport MagickRealType
  DecodePixelGamma(const MagickRealType) magick_hot_spot,
  EncodePixelGamma(const MagickRealType) magick_hot_spot,
  GetMagickPixelIntensity(const Image *image,
    const MagickPixelPacket *magick_restrict) magick_hot_spot,
  GetPixelIntensity(const Image *image,const PixelPacket *magick_restrict)
    magick_hot_spot;

extern MagickExport void
  ConformMagickPixelPacket(Image *,const MagickPixelPacket *,
    MagickPixelPacket *,ExceptionInfo *),
  GetMagickPixelPacket(const Image *,MagickPixelPacket *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
