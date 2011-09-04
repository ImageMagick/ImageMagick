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
  AlphaPixelChannel = 3,
  BlackPixelChannel = 4,
  IndexPixelChannel = 4,
  MaskPixelChannel = 5,
  IntensityPixelChannel = MaxPixelChannels,
  SyncPixelChannel = MaxPixelChannels+1
} PixelChannel;

typedef enum
{
  UndefinedPixelTrait = 0x000000,
  CopyPixelTrait = 0x000001,
  UpdatePixelTrait = 0x000002,
  BlendPixelTrait = 0x000004
} PixelTrait;

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

typedef struct _PixelChannelMap
{
  PixelChannel
    channel;

  PixelTrait
    traits;
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

extern MagickExport ChannelType
  SetPixelChannelMask(Image *,const ChannelType);

extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *,ExceptionInfo *),
  InterpolatePixelChannel(const Image *,const CacheView_ *,
    const PixelChannel,const InterpolatePixelMethod,const double,const double,
    double *,ExceptionInfo *),
  InterpolatePixelInfo(const Image *,const CacheView_ *,
    const InterpolatePixelMethod,const double,const double,PixelInfo *,
    ExceptionInfo *),
  IsFuzzyEquivalencePixel(const Image *,const Quantum *,
    const Quantum *),
  IsFuzzyEquivalencePixelInfo(const PixelInfo *,const PixelInfo *),
  IsFuzzyEquivalencePixelPacket(const Image *,const PixelPacket *,
    const PixelPacket *);

extern MagickExport PixelChannelMap
  *AcquirePixelChannelMap(void),
  *ClonePixelChannelMap(PixelChannelMap *),
  *DestroyPixelChannelMap(PixelChannelMap *);

extern MagickExport PixelInfo
  *ClonePixelInfo(const PixelInfo *);

extern MagickExport void
  InitializePixelChannelMap(Image *),
  GetPixelInfo(const Image *,PixelInfo *),
  SetPixelChannelMap(Image *,const ChannelType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
