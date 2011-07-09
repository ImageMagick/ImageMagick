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

#include <MagickCore/colorspace.h>
#include <MagickCore/constitute.h>

#define MaxPixelComponents  32
#define MaxPixelComponentMaps  8

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

typedef enum
{
  UndefinedPixelTrait = 0x000000,
  ActivePixelTrait = 0x000001,
  BlendPixelTrait = 0x000002
} PixelTrait;

typedef struct _PixelComponentMap
{
  PixelComponent
    component;

  PixelTrait
    traits;
} PixelComponentMap;

typedef struct _DoublePixelPacket
{
  double
    red,
    green,
    blue,
    alpha,
    black;
} DoublePixelPacket;

typedef struct _LongPixelPacket
{
  unsigned int
    red,
    green,
    blue,
    alpha,
    black;
} LongPixelPacket;

typedef struct _PixelInfo
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
    alpha,
    black,
    index;
} PixelInfo;

typedef struct _PixelPacket
{
  Quantum
    red,
    green,
    blue,
    alpha,
    black,
    index;

 MagickSizeType
    count;
} PixelPacket;

typedef struct _CacheView
  CacheView_;

extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *),
  InterpolatePixelInfo(const Image *,const CacheView_ *,
    const InterpolatePixelMethod,const double,const double,PixelInfo *,
    ExceptionInfo *),
  IsFuzzyEquivalencePixel(const Image *,const Quantum *,
    const Quantum *),
  IsFuzzyEquivalencePixelInfo(const PixelInfo *,const PixelInfo *),
  IsFuzzyEquivalencePixelPacket(const Image *,const PixelPacket *,
    const PixelPacket *);

extern MagickExport PixelComponentMap
  **AcquirePixelComponentMap(void),
  **ClonePixelComponentMap(PixelComponentMap **),
  **DestroyPixelComponentMap(PixelComponentMap **);

extern MagickExport PixelInfo
  *ClonePixelInfo(const PixelInfo *);

extern MagickExport void
  StandardPixelComponentMap(Image *),
  GetPixelInfo(const Image *,PixelInfo *),
  PopPixelComponentMap(Image *),
  PushPixelComponentMap(Image *,const ChannelType),
  SetPixelComponentMap(Image *,const ChannelType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
