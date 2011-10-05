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

  MagickCore image methods.
*/
#ifndef _MAGICKCORE_IMAGE_H
#define _MAGICKCORE_IMAGE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <MagickCore/color.h>

#define OpaqueAlpha  ((Quantum) QuantumRange)
#define TransparentAlpha  ((Quantum) 0UL)

typedef enum
{
  UndefinedAlphaChannel,
  ActivateAlphaChannel,
  BackgroundAlphaChannel,
  CopyAlphaChannel,
  DeactivateAlphaChannel,
  ExtractAlphaChannel,
  OpaqueAlphaChannel,
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

#include "MagickCore/blob.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/composite.h"
#include "MagickCore/compress.h"
#include "MagickCore/effect.h"
#include "MagickCore/geometry.h"
#include "MagickCore/layer.h"
#include "MagickCore/locale_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/pixel.h"
#include "MagickCore/profile.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resample.h"
#include "MagickCore/resize.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/stream.h"
#include "MagickCore/timer.h"

struct _Image
{
  ClassType
    storage_class;

  ColorspaceType
    colorspace;      /* colorspace of image data */

  CompressionType
    compression;     /* compression of image when read/write */

  size_t
    quality;         /* compression quality setting, meaning varies */

  OrientationType
    orientation;     /* photo orientation of image */

  MagickBooleanType
    taint,           /* has image been modified since reading */
    matte;           /* is transparency channel defined and active */

  size_t
    columns,         /* physical size of image */
    rows,
    depth,           /* depth of image on read/write */
    colors;          /* size of color table on read */

  PixelPacket
    *colormap,
    background_color, /* current background color attribute */
    border_color,     /* current bordercolor attribute */
    matte_color;      /* current mattecolor attribute */

  double
    gamma;

  ChromaticityInfo
    chromaticity;

  RenderingIntent
    rendering_intent;

  void
    *profiles;

  ResolutionType
    units;          /* resolution/density  ppi or ppc */

  char
    *montage,
    *directory,
    *geometry;

  ssize_t
    offset;

  double
    x_resolution,   /* image resolution/density */
    y_resolution;

  RectangleInfo
    page,           /* virtual canvas size and offset of image */
    extract_info;

  double
    bias,           /* depreciated ? */
    blur,
    fuzz;           /* current color fuzz attribute */

  FilterTypes
    filter;         /* resize/distort filter to apply */

  InterlaceType
    interlace;

  EndianType
    endian;         /* raw data integer ordering on read/write */

  GravityType
    gravity;        /* Gravity attribute for positioning in image */

  CompositeOperator
    compose;        /* alpha composition method for layered images */

  DisposeType
    dispose;        /* GIF animation disposal method */

  struct _Image
    *clip_mask;

  size_t
    scene,          /* index of image in multi-image file */
    delay;          /* Animation delay time */

  ssize_t
    ticks_per_second;  /* units for delay time, default 100 for GIF */

  size_t
    iterations,
    total_colors;

  ssize_t
    start_loop;

  PixelInterpolateMethod
    interpolate;       /* Interpolation of color for between pixel lookups */

  MagickBooleanType
    black_point_compensation;

  PixelPacket
    transparent_color; /* color for 'transparent' color index in GIF */

  struct _Image
    *mask;

  RectangleInfo
    tile_offset;

  void
    *properties,       /* per image properities */
    *artifacts;        /* per image sequence image artifacts */

  ImageType
    type;

  MagickBooleanType
    dither;            /* dithering on/off */

  MagickSizeType
    extent;            /* Size of image read from disk */

  MagickBooleanType
    ping;

  size_t
    number_channels,
    number_meta_channels,
    metacontent_extent;

  MagickBooleanType
    sync;

  ChannelType
    channel_mask;

  PixelChannelMap
    *channel_map;

  void
    *cache;

  ErrorInfo
    error;

  TimerInfo
    timer;

  MagickProgressMonitor
    progress_monitor;

  void
    *client_data;

  Ascii85Info
    *ascii85;

  ProfileInfo
    color_profile,
    iptc_profile,
    *generic_profile;

  char
    filename[MaxTextExtent],        /* images input filename */
    magick_filename[MaxTextExtent], /* given image filename (with read mods) */
    magick[MaxTextExtent];          /* images file format (file magic) */

  size_t
    magick_columns,
    magick_rows;

  BlobInfo
    *blob;             /* image file as in-memory string of 'extent' */

  ExceptionInfo
    exception;         /* Error handling report */

  MagickBooleanType
    debug;             /* debug output attribute */

  volatile ssize_t
    reference_count;   /* image data sharing memory management */

  SemaphoreInfo
    *semaphore;

  struct _Image
    *previous,         /* Image sequence list links */
    *list,
    *next;

  size_t
    signature;
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

  size_t
    scene,
    number_scenes,
    depth;

  InterlaceType
    interlace;          /* interlace for image write */

  EndianType
    endian;             /* integer endian order for raw image data */

  ResolutionType
    units;              /* denisty pixels/inch or pixel/cm */

  size_t
    quality;            /* compression quality */

  char
    *sampling_factor,   /* JPEG write sampling factor */
    *server_name,       /* X windows server name - display/animate */
    *font,              /* draw_info */
    *texture,           /* montage/display background tile */
    *density;

  double
    pointsize,
    fuzz;

  PixelPacket
    background_color,
    border_color,
    matte_color;

  MagickBooleanType
    dither,            /* dither enable-disable */
    monochrome;        /* read/write pcl,pdf,ps,xps as monocrome image */

  size_t
    colors;

  ColorspaceType
    colorspace;

  ImageType
    type;

  PreviewType
    preview_type;

  ssize_t
    group;

  MagickBooleanType
    ping,                    /* fast read image attributes, not image data */
    verbose;                 /* verbose output enable/disable */

  char
    *view;
    /* authenticate -- moved to ImageOptions()

  ChannelType
    channel;

  void
    *options;                /* splay tree of use options */

  VirtualPixelMethod
    virtual_pixel_method;

  PixelPacket
    transparent_color;

  void
    *profile;

  MagickBooleanType
    synchronize;

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
    magick[MaxTextExtent],    /* image file format (file magick) */
    unique[MaxTextExtent],    /* unique tempory filename - delegates */
    zero[MaxTextExtent],      /* unique filename ? - delegates */
    filename[MaxTextExtent];  /* filename when reading/writing image */

  MagickBooleanType
    debug;

  size_t
    signature;
};

extern MagickExport const char
  DefaultTileGeometry[],
  DefaultTileLabel[],
  LoadImageTag[],
  LoadImagesTag[],
  PSDensityGeometry[],
  PSPageGeometry[],
  SaveImageTag[],
  SaveImagesTag[];

extern MagickExport const double
  DefaultResolution;

extern MagickExport ExceptionType
  CatchImageException(Image *);

extern MagickExport FILE
  *GetImageInfoFile(const ImageInfo *);

extern MagickExport Image
  *AcquireImage(const ImageInfo *,ExceptionInfo *),
  *AppendImages(const Image *,const MagickBooleanType,ExceptionInfo *),
  *CloneImage(const Image *,const size_t,const size_t,const MagickBooleanType,
    ExceptionInfo *),
  *CombineImages(const Image *,ExceptionInfo *),
  *DestroyImage(Image *),
  *GetImageClipMask(const Image *,ExceptionInfo *),
  *GetImageMask(const Image *,ExceptionInfo *),
  *NewMagickImage(const ImageInfo *,const size_t,const size_t,
    const PixelInfo *),
  *ReferenceImage(Image *),
  *SeparateImages(const Image *,ExceptionInfo *),
  *SmushImages(const Image *,const MagickBooleanType,const ssize_t,
    ExceptionInfo *);

extern MagickExport ImageInfo
  *AcquireImageInfo(void),
  *CloneImageInfo(const ImageInfo *),
  *DestroyImageInfo(ImageInfo *);

extern MagickExport MagickBooleanType
  ClipImage(Image *,ExceptionInfo *),
  ClipImagePath(Image *,const char *,const MagickBooleanType,ExceptionInfo *),
  GetImageAlphaChannel(const Image *),
  IsTaintImage(const Image *),
  IsHighDynamicRangeImage(const Image *,ExceptionInfo *),
  IsImageObject(const Image *),
  ListMagickInfo(FILE *,ExceptionInfo *),
  ModifyImage(Image **,ExceptionInfo *),
  ResetImagePage(Image *,const char *),
  SeparateImage(Image *),
  SetImageAlpha(Image *,const Quantum),
  SetImageAlphaChannel(Image *,const AlphaChannelType,ExceptionInfo *),
  SetImageBackgroundColor(Image *),
  SetImageClipMask(Image *,const Image *,ExceptionInfo *),
  SetImageColor(Image *,const PixelInfo *),
  SetImageExtent(Image *,const size_t,const size_t,ExceptionInfo *),
  SetImageInfo(ImageInfo *,const unsigned int,ExceptionInfo *),
  SetImageMask(Image *,const Image *,ExceptionInfo *),
  SetImageStorageClass(Image *,const ClassType,ExceptionInfo *),
  SetImageType(Image *,const ImageType,ExceptionInfo *),
  StripImage(Image *),
  SyncImage(Image *),
  SyncImageSettings(const ImageInfo *,Image *),
  SyncImagesSettings(ImageInfo *,Image *);

extern MagickExport size_t
  InterpretImageFilename(const ImageInfo *,Image *,const char *,int,char *);

extern MagickExport ssize_t
  GetImageReferenceCount(Image *);

extern MagickExport VirtualPixelMethod
  GetImageVirtualPixelMethod(const Image *),
  SetImageVirtualPixelMethod(const Image *,const VirtualPixelMethod);

extern MagickExport void
  AcquireNextImage(const ImageInfo *,Image *,ExceptionInfo *),
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
