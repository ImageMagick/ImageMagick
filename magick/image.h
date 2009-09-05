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

  MagickCore image methods.
*/
#ifndef _MAGICKCORE_IMAGE_H
#define _MAGICKCORE_IMAGE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/color.h>

#define OpaqueOpacity  ((Quantum) 0UL)
#define TransparentOpacity  ((Quantum) QuantumRange)

typedef enum
{
  UndefinedAlphaChannel,
  ActivateAlphaChannel,
  BackgroundAlphaChannel,
  CopyAlphaChannel,
  DeactivateAlphaChannel,
  ExtractAlphaChannel,
  OpaqueAlphaChannel,
  ResetAlphaChannel,  /* deprecated */
  SetAlphaChannel,
  ShapeAlphaChannel,
  TransparentAlphaChannel
} AlphaChannelType;

typedef enum
{
  UndefinedType,
  BilevelType,
  GrayscaleType,
  GrayscaleMatteType,
  PaletteType,
  PaletteMatteType,
  TrueColorType,
  TrueColorMatteType,
  ColorSeparationType,
  ColorSeparationMatteType,
  OptimizeType,
  PaletteBilevelMatteType
} ImageType;

typedef enum
{
  UndefinedInterlace,
  NoInterlace,
  LineInterlace,
  PlaneInterlace,
  PartitionInterlace,
  GIFInterlace,
  JPEGInterlace,
  PNGInterlace
} InterlaceType;

typedef enum
{
  UndefinedOrientation,
  TopLeftOrientation,
  TopRightOrientation,
  BottomRightOrientation,
  BottomLeftOrientation,
  LeftTopOrientation,
  RightTopOrientation,
  RightBottomOrientation,
  LeftBottomOrientation
} OrientationType;

typedef enum
{
  UndefinedResolution,
  PixelsPerInchResolution,
  PixelsPerCentimeterResolution
} ResolutionType;

typedef struct _PrimaryInfo
{
  double
    x,
    y,
    z;
} PrimaryInfo;

typedef struct _SegmentInfo
{
  double
    x1,
    y1,
    x2,
    y2;
} SegmentInfo;

typedef enum
{
  UndefinedTransmitType,
  FileTransmitType,
  BlobTransmitType,
  StreamTransmitType,
  ImageTransmitType
} TransmitType;

typedef struct _ChromaticityInfo
{
  PrimaryInfo
    red_primary,
    green_primary,
    blue_primary,
    white_point;
} ChromaticityInfo;

#include "magick/blob.h"
#include "magick/colorspace.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/composite.h"
#include "magick/compress.h"
#include "magick/effect.h"
#include "magick/geometry.h"
#include "magick/layer.h"
#include "magick/monitor.h"
#include "magick/pixel.h"
#include "magick/profile.h"
#include "magick/quantum.h"
#include "magick/resample.h"
#include "magick/resize.h"
#include "magick/semaphore.h"
#include "magick/stream.h"
#include "magick/timer.h"

struct _Image
{
  ClassType
    storage_class;

  ColorspaceType
    colorspace;

  CompressionType
    compression;

  unsigned long
    quality;

  OrientationType
    orientation;

  MagickBooleanType
    taint,
    matte;

  unsigned long
    columns,
    rows,
    depth,
    colors;

  PixelPacket
    *colormap,
    background_color,
    border_color,
    matte_color;

  double
    gamma;

  ChromaticityInfo
    chromaticity;

  RenderingIntent
    rendering_intent;

  void
    *profiles;

  ResolutionType
    units;

  char
    *montage,
    *directory,
    *geometry;

  long
    offset;

  double
    x_resolution,
    y_resolution;

  RectangleInfo
    page,
    extract_info,
    tile_info;  /* deprecated */

  double
    bias,
    blur,  /* deprecated */
    fuzz;

  FilterTypes
    filter;

  InterlaceType
    interlace;

  EndianType
    endian;

  GravityType
    gravity;

  CompositeOperator
    compose;

  DisposeType
    dispose;

  struct _Image
    *clip_mask;

  unsigned long
    scene,
    delay;

  long
    ticks_per_second;

  unsigned long
    iterations,
    total_colors;

  long
    start_loop;

  ErrorInfo
    error;

  TimerInfo
    timer;

  MagickProgressMonitor
    progress_monitor;

  void
    *client_data,
    *cache,
    *attributes;  /* deprecated */

  Ascii85Info
    *ascii85;

  BlobInfo
    *blob;

  char
    filename[MaxTextExtent],
    magick_filename[MaxTextExtent],
    magick[MaxTextExtent];

  unsigned long
    magick_columns,
    magick_rows;

  ExceptionInfo
    exception;

  MagickBooleanType
    debug;

  volatile long
    reference_count;

  SemaphoreInfo
    *semaphore;

  ProfileInfo
    color_profile,
    iptc_profile,
    *generic_profile;

  unsigned long
    generic_profiles;  /* this & ProfileInfo is deprecated */

  unsigned long
    signature;

  struct _Image
    *previous,
    *list,
    *next;

  InterpolatePixelMethod
    interpolate;

  MagickBooleanType
    black_point_compensation;

  PixelPacket
    transparent_color;

  struct _Image
    *mask;

  RectangleInfo
    tile_offset;

  void
    *properties,
    *artifacts;

  ImageType
    type;

  MagickBooleanType
    dither;
};

struct _ImageInfo
{
  CompressionType
    compression;

  OrientationType
    orientation;

  MagickBooleanType
    temporary,
    adjoin,
    affirm,
    antialias;

  char
    *size,
    *extract,
    *page,
    *scenes;

  unsigned long
    scene,
    number_scenes,
    depth;

  InterlaceType
    interlace;

  EndianType
    endian;

  ResolutionType
    units;

  unsigned long
    quality;

  char
    *sampling_factor,
    *server_name,
    *font,
    *texture,
    *density;

  double
    pointsize,
    fuzz;

  PixelPacket
    background_color,
    border_color,
    matte_color;

  MagickBooleanType
    dither,
    monochrome;

  unsigned long
    colors;

  ColorspaceType
    colorspace;

  ImageType
    type;

  PreviewType
    preview_type;

  long
    group;

  MagickBooleanType
    ping,
    verbose;

  char
    *view,
    *authenticate;

  ChannelType
    channel;

  Image
    *attributes;  /* deprecated */

  void
    *options;

  MagickProgressMonitor
    progress_monitor;

  void
    *client_data,
    *cache;

  StreamHandler
    stream;

  FILE
    *file;

  void
    *blob;

  size_t
    length;

  char
    magick[MaxTextExtent],
    unique[MaxTextExtent],
    zero[MaxTextExtent],
    filename[MaxTextExtent];

  MagickBooleanType
    debug;

  char
    *tile;  /* deprecated */

  unsigned long
    subimage,  /* deprecated */
    subrange;  /* deprecated */

  PixelPacket
    pen;  /* deprecated */

  unsigned long
    signature;

  VirtualPixelMethod
    virtual_pixel_method;

  PixelPacket
    transparent_color;

  void
    *profile;

  MagickBooleanType
    synchronize;
};

extern MagickExport ExceptionType
  CatchImageException(Image *);

extern MagickExport Image
  *AcquireImage(const ImageInfo *),
  *AppendImages(const Image *,const MagickBooleanType,ExceptionInfo *),
  *AverageImages(const Image *,ExceptionInfo *),
  *CloneImage(const Image *,const unsigned long,const unsigned long,
    const MagickBooleanType,ExceptionInfo *),
  *CombineImages(const Image *,const ChannelType,ExceptionInfo *),
  *DestroyImage(Image *),
  *GetImageClipMask(const Image *,ExceptionInfo *),
  *GetImageMask(const Image *,ExceptionInfo *),
  *NewMagickImage(const ImageInfo *,const unsigned long,const unsigned long,
    const MagickPixelPacket *),
  *ReferenceImage(Image *),
  *SeparateImages(const Image *,const ChannelType,ExceptionInfo *);

extern MagickExport ImageInfo
  *AcquireImageInfo(void),
  *CloneImageInfo(const ImageInfo *),
  *DestroyImageInfo(ImageInfo *);

extern MagickExport ImageType
  GetImageType(const Image *,ExceptionInfo *);

extern MagickExport long
  GetImageReferenceCount(Image *);

extern MagickExport MagickBooleanType
  AcquireImageColormap(Image *,const unsigned long),
  ClipImage(Image *),
  ClipImagePath(Image *,const char *,const MagickBooleanType),
  CycleColormapImage(Image *,const long),
  GetImageAlphaChannel(const Image *),
  IsTaintImage(const Image *),
  IsMagickConflict(const char *),
  IsHighDynamicRangeImage(const Image *,ExceptionInfo *),
  IsImageObject(const Image *),
  ListMagickInfo(FILE *,ExceptionInfo *),
  ModifyImage(Image **,ExceptionInfo *),
  ResetImagePage(Image *,const char *),
  SeparateImageChannel(Image *,const ChannelType),
  SetImageAlphaChannel(Image *,const AlphaChannelType),
  SetImageBackgroundColor(Image *),
  SetImageClipMask(Image *,const Image *),
  SetImageExtent(Image *,const unsigned long,const unsigned long),
  SetImageInfo(ImageInfo *,const MagickBooleanType,ExceptionInfo *),
  SetImageMask(Image *,const Image *),
  SetImageOpacity(Image *,const Quantum),
  SetImageStorageClass(Image *,const ClassType),
  SetImageType(Image *,const ImageType),
  SortColormapByIntensity(Image *),
  StripImage(Image *),
  SyncImage(Image *),
  TextureImage(Image *,const Image *);

extern MagickExport size_t
  InterpretImageFilename(const ImageInfo *,Image *,const char *,int,char *);

extern MagickExport VirtualPixelMethod
  GetImageVirtualPixelMethod(const Image *),
  SetImageVirtualPixelMethod(const Image *,const VirtualPixelMethod);

extern MagickExport void
  AcquireNextImage(const ImageInfo *,Image *),
  DestroyImagePixels(Image *),
  DisassociateImageStream(Image *),
  GetImageException(Image *,ExceptionInfo *),
  GetImageInfo(ImageInfo *),
  SetImageInfoBlob(ImageInfo *,const void *,const size_t),
  SetImageInfoFile(ImageInfo *,FILE *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
