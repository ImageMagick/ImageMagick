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

#define MaxPixelChannels  32
#undef index

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
} PixelInterpolateMethod;

typedef enum
{
  UndefinedPixelChannel = 0,
  RedPixelChannel = 0,
  CyanPixelChannel = 0,
  GrayPixelChannel = 0,
  YPixelChannel = 0,
  GreenPixelChannel = 1,
  MagentaPixelChannel = 1,
  CbPixelChannel = 1,
  BluePixelChannel = 2,
  YellowPixelChannel = 2,
  CrPixelChannel = 2,
  BlackPixelChannel = 3,
  AlphaPixelChannel = 4,
  IndexPixelChannel = 5,
  MaskPixelChannel = 6,
  MetaPixelChannel = 7,
  IntensityPixelChannel = MaxPixelChannels,
  CompositePixelChannel = MaxPixelChannels,
  SyncPixelChannel = MaxPixelChannels+1
} PixelChannel;

typedef enum
{
  UndefinedPixelTrait = 0x000000,
  CopyPixelTrait = 0x000001,
  UpdatePixelTrait = 0x000002,
  BlendPixelTrait = 0x000004
} PixelTrait;

typedef struct _PixelChannelMap
{
  PixelChannel
    channel;

  PixelTrait
    traits;

  ssize_t
    offset;
} PixelChannelMap;

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

  MagickSizeType
    count;

  double
    red,
    green,
    blue,
    black,
    alpha,
    index;
} PixelInfo;

typedef struct _PixelPacket
{
  unsigned int
    red,
    green,
    blue,
    alpha,
    black;
} PixelPacket;

typedef struct _CacheView
  CacheView_;

extern MagickExport ChannelType
  SetPixelChannelMask(Image *,const ChannelType);

extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *,ExceptionInfo *),
  InterpolatePixelChannel(const Image *,const CacheView_ *,
    const PixelChannel,const PixelInterpolateMethod,const double,const double,
    double *,ExceptionInfo *),
  InterpolatePixelChannels(const Image *,const CacheView_ *,const Image *,
    const PixelInterpolateMethod,const double,const double,Quantum *,
    ExceptionInfo *),
  InterpolatePixelInfo(const Image *,const CacheView_ *,
    const PixelInterpolateMethod,const double,const double,PixelInfo *,
    ExceptionInfo *),
  IsFuzzyEquivalencePixel(const Image *,const Quantum *,
    const Quantum *),
  IsFuzzyEquivalencePixelInfo(const PixelInfo *,const PixelInfo *);

extern MagickExport PixelChannelMap
  *AcquirePixelChannelMap(void),
  *ClonePixelChannelMap(PixelChannelMap *),
  *DestroyPixelChannelMap(PixelChannelMap *);

extern MagickExport PixelInfo
  *ClonePixelInfo(const PixelInfo *);

extern MagickExport void
  InitializePixelChannelMap(Image *),
  GetPixelInfo(const Image *,PixelInfo *),
  SetPixelChannelMapMask(Image *,const ChannelType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
