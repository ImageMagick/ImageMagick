/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image pixel methods.
*/
#ifndef MAGICKCORE_PIXEL_H
#define MAGICKCORE_PIXEL_H

#include "MagickCore/colorspace.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MaxPixelChannels  64
#undef index

/*
  Pixel enum declarations.
*/
typedef enum
{
  UndefinedChannel = 0x0000,
  RedChannel = 0x0001,
  GrayChannel = 0x0001,
  CyanChannel = 0x0001,
  LChannel = 0x0001,
  GreenChannel = 0x0002,
  MagentaChannel = 0x0002,
  aChannel = 0x0002,
  BlueChannel = 0x0004,
  bChannel = 0x0002,
  YellowChannel = 0x0004,
  BlackChannel = 0x0008,
  AlphaChannel = 0x0010,
  OpacityChannel = 0x0010,
  IndexChannel = 0x0020,             /* Color Index Table? */
  ReadMaskChannel = 0x0040,          /* Pixel is Not Readable? */
  WriteMaskChannel = 0x0080,         /* Pixel is Write Protected? */
  MetaChannel = 0x0100,              /* not used */
  CompositeMaskChannel = 0x0200,     /* SVG mask */
  CompositeChannels = 0x001F,
  AllChannels = 0x7ffffff,
  /*
    Special purpose channel types.
    FUTURE: are these needed any more - they are more like hacks
    SyncChannels for example is NOT a real channel but a 'flag'
    It really says -- "User has not defined channels"
    Though it does have extra meaning in the "-auto-level" operator
  */
  TrueAlphaChannel = 0x0100, /* extract actual alpha channel from opacity */
  RGBChannels = 0x0200,      /* set alpha from grayscale mask in RGB */
  GrayChannels = 0x0400,
  SyncChannels = 0x20000,    /* channels modified as a single unit */
  DefaultChannels = AllChannels
} ChannelType;  /* must correspond to PixelChannel */

typedef enum
{
  UndefinedPixelChannel = 0,
  RedPixelChannel = 0,
  CyanPixelChannel = 0,
  GrayPixelChannel = 0,
  LPixelChannel = 0,
  LabelPixelChannel = 0,
  YPixelChannel = 0,
  aPixelChannel = 1,
  GreenPixelChannel = 1,
  MagentaPixelChannel = 1,
  CbPixelChannel = 1,
  bPixelChannel = 2,
  BluePixelChannel = 2,
  YellowPixelChannel = 2,
  CrPixelChannel = 2,
  BlackPixelChannel = 3,
  AlphaPixelChannel = 4,
  IndexPixelChannel = 5,
  ReadMaskPixelChannel = 6,
  WriteMaskPixelChannel = 7,
  MetaPixelChannel = 8,
  CompositeMaskPixelChannel = 9,
  IntensityPixelChannel = MaxPixelChannels,  /* ???? */
  CompositePixelChannel = MaxPixelChannels,  /* ???? */
  SyncPixelChannel = MaxPixelChannels+1      /* not a real channel */
} PixelChannel;  /* must correspond to ChannelType */

typedef enum
{
  UndefinedPixelIntensityMethod = 0,
  AveragePixelIntensityMethod,
  BrightnessPixelIntensityMethod,
  LightnessPixelIntensityMethod,
  MSPixelIntensityMethod,
  Rec601LumaPixelIntensityMethod,
  Rec601LuminancePixelIntensityMethod,
  Rec709LumaPixelIntensityMethod,
  Rec709LuminancePixelIntensityMethod,
  RMSPixelIntensityMethod
} PixelIntensityMethod;

typedef enum
{
  UndefinedInterpolatePixel,
  AverageInterpolatePixel,    /* Average 4 nearest neighbours */
  Average9InterpolatePixel,   /* Average 9 nearest neighbours */
  Average16InterpolatePixel,  /* Average 16 nearest neighbours */
  BackgroundInterpolatePixel, /* Just return background color */
  BilinearInterpolatePixel,   /* Triangular filter interpolation */
  BlendInterpolatePixel,      /* blend of nearest 1, 2 or 4 pixels */
  CatromInterpolatePixel,     /* Catmull-Rom interpolation */
  IntegerInterpolatePixel,    /* Integer (floor) interpolation */
  MeshInterpolatePixel,       /* Triangular Mesh interpolation */
  NearestInterpolatePixel,    /* Nearest Neighbour Only */
  SplineInterpolatePixel      /* Cubic Spline (blurred) interpolation */
} PixelInterpolateMethod;

typedef enum
{
  UndefinedPixelMask = 0x000000,
  ReadPixelMask = 0x000001,
  WritePixelMask = 0x000002,
  CompositePixelMask = 0x000004
} PixelMask;

typedef enum
{
  UndefinedPixelTrait = 0x000000,
  CopyPixelTrait = 0x000001,
  UpdatePixelTrait = 0x000002,
  BlendPixelTrait = 0x000004
} PixelTrait;

typedef enum
{
  UndefinedPixel,
  CharPixel,
  DoublePixel,
  FloatPixel,
  LongPixel,
  LongLongPixel,
  QuantumPixel,
  ShortPixel
} StorageType;

/*
  Pixel typedef declarations.
*/
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

  PixelTrait
    alpha_trait;

  double
    fuzz;

  size_t
    depth;

  MagickSizeType
    count;

  MagickRealType
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

/*
  Pixel method declarations.
*/
extern MagickExport ChannelType
  SetPixelChannelMask(Image *,const ChannelType);

extern MagickExport MagickBooleanType
  ExportImagePixels(const Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,void *,ExceptionInfo *),
  ImportImagePixels(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const char *,const StorageType,const void *,ExceptionInfo *),
  InterpolatePixelChannel(const Image *magick_restrict,const CacheView_ *,
    const PixelChannel,const PixelInterpolateMethod,const double,const double,
    double *,ExceptionInfo *),
  InterpolatePixelChannels(const Image *magick_restrict,const CacheView_ *,
    const Image * magick_restrict,const PixelInterpolateMethod,const double,
    const double,Quantum *,ExceptionInfo *),
  InterpolatePixelInfo(const Image *,const CacheView_ *,
    const PixelInterpolateMethod,const double,const double,PixelInfo *,
    ExceptionInfo *),
  IsFuzzyEquivalencePixel(const Image *,const Quantum *,const Image *,
    const Quantum *) magick_attribute((__pure__)),
  IsFuzzyEquivalencePixelInfo(const PixelInfo *,const PixelInfo *)
    magick_attribute((__pure__)),
  SetPixelMetaChannels(Image *,const size_t,ExceptionInfo *),
  SortImagePixels(Image *,ExceptionInfo *);

extern MagickExport MagickRealType
  GetPixelInfoIntensity(const Image *magick_restrict,
    const PixelInfo *magick_restrict) magick_hot_spot,
  GetPixelIntensity(const Image *magick_restrict,
    const Quantum *magick_restrict) magick_hot_spot;

extern MagickExport PixelChannelMap
  *AcquirePixelChannelMap(void),
  *ClonePixelChannelMap(PixelChannelMap *),
  *DestroyPixelChannelMap(PixelChannelMap *);

extern MagickExport PixelInfo
  *ClonePixelInfo(const PixelInfo *);

extern MagickExport MagickRealType
  DecodePixelGamma(const MagickRealType) magick_hot_spot,
  EncodePixelGamma(const MagickRealType) magick_hot_spot;

extern MagickExport void
  ConformPixelInfo(Image *,const PixelInfo *,PixelInfo *,ExceptionInfo *),
  GetPixelInfo(const Image *,PixelInfo *),
  InitializePixelChannelMap(Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
