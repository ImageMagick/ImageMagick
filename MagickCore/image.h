/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image methods.
*/
#ifndef MAGICKCORE_IMAGE_H
#define MAGICKCORE_IMAGE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define OpaqueAlpha  ((Quantum) QuantumRange)
#define TransparentAlpha  ((Quantum) 0)

typedef enum
{
  UndefinedType,
  BilevelType,
  GrayscaleType,
  GrayscaleAlphaType,
  PaletteType,
  PaletteAlphaType,
  TrueColorType,
  TrueColorAlphaType,
  ColorSeparationType,
  ColorSeparationAlphaType,
  OptimizeType,
  PaletteBilevelAlphaType
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
    colorspace;         /* colorspace of image data */

  CompressionType
    compression;        /* compression of image when read/write */

  size_t
    quality;            /* compression quality setting, meaning varies */

  OrientationType
    orientation;        /* photo orientation of image */

  MagickBooleanType
    taint;              /* has image been modified since reading */

  size_t
    columns,            /* physical size of image */
    rows,
    depth,              /* depth of image on read/write */
    colors;             /* Size of color table, or actual color count */
                        /* Only valid if image is not DirectClass */

  PixelInfo
    *colormap,
    alpha_color,        /* deprecated */
    background_color,   /* current background color attribute */
    border_color,       /* current bordercolor attribute */
    transparent_color;  /* color for 'transparent' color index in GIF */

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
    offset;         /* ??? */

  PointInfo
    resolution;     /* image resolution/density */

  RectangleInfo
    page,           /* virtual canvas size and offset of image */
    extract_info;

  double
    fuzz;           /* current color fuzz attribute - move to image_info */

  FilterType
    filter;         /* resize/distort filter to apply */

  PixelIntensityMethod
    intensity;      /* method to generate an intensity value from a pixel */

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

  size_t
    scene,          /* index of image in multi-image file */
    delay,          /* Animation delay time */
    duration;       /* Total animation duration sum(delay*iterations) */

  ssize_t
    ticks_per_second;  /* units for delay time, default 100 for GIF */

  size_t
    iterations,        /* number of interactions for GIF animations */
    total_colors;

  ssize_t
    start_loop;        /* ??? */

  PixelInterpolateMethod
    interpolate;       /* Interpolation of color for between pixel lookups */

  MagickBooleanType
    black_point_compensation;

  RectangleInfo
    tile_offset;

  ImageType
    type;

  MagickBooleanType
    dither;            /* dithering on/off */

  MagickSizeType
    extent;            /* Size of image read from disk */

  MagickBooleanType
    ping;              /* no image data read, just attributes */

  MagickBooleanType
    read_mask,
    write_mask;

  PixelTrait
    alpha_trait;       /* is transparency channel defined and active */

  size_t
    number_channels,
    number_meta_channels,
    metacontent_extent;

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
    *generic_profile;

  void
    *properties,       /* general settings, to save with image */
    *artifacts;        /* general operational/coder settings, not saved */

  char
    filename[MagickPathExtent],        /* images input filename */
    magick_filename[MagickPathExtent], /* given image filename (with read mods) */
    magick[MagickPathExtent];          /* images file format (file magic) */

  size_t
    magick_columns,     /* size of image when read/created */
    magick_rows;

  BlobInfo
    *blob;             /* image file as in-memory string of 'extent' */

  time_t
    timestamp;

  MagickBooleanType
    debug;             /* debug output attribute */

  ssize_t
    reference_count;   /* image data sharing memory management */

  SemaphoreInfo
    *semaphore;

  struct _ImageInfo
    *image_info;       /* (Optional) Image belongs to this ImageInfo 'list'
                        * For access to 'global options' when no per-image
                        * attribute, prosperity, or artifact has been set.
                        */

  struct _Image
    *list,             /* Undo/Redo image processing list (for display) */
    *previous,         /* Image list links */
    *next;

  size_t
    signature;

  PixelInfo
    matte_color;        /* current mattecolor attribute */

  MagickBooleanType
    composite_mask;

  PixelTrait
    mask_trait;       /* apply the clip or composite mask */

  ChannelType
    channels;

  time_t
    ttl;
};

/*
  ImageInfo structure:
    Stores an image list, as well as all global settings used by all images
    held, -- unless overridden for that specific image.  See SyncImagesettings()
    which maps any global setting that always overrides specific image settings.
*/
struct _ImageInfo
{
  CompressionType
    compression;        /* compression method when reading/saving image */

  OrientationType
    orientation;        /* orientation setting */

  MagickBooleanType
    temporary,          /* image file to be deleted after read "ephemeral:" */
    adjoin,             /* save images to separate scene files */
    affirm,
    antialias;

  char
    *size,              /* image generation size */
    *extract,           /* crop/resize string on image read */
    *page,
    *scenes;            /* scene numbers that is to be read in */

  size_t
    scene,              /* starting value for image save numbering */
    number_scenes,      /* total number of images in list - for escapes */
    depth;              /* current read/save depth of images */

  InterlaceType
    interlace;          /* interlace for image write */

  EndianType
    endian;             /* integer endian order for raw image data */

  ResolutionType
    units;              /* density pixels/inch or pixel/cm */

  size_t
    quality;            /* compression quality */

  char
    *sampling_factor,   /* Chroma subsampling ratio string */
    *server_name,       /* X windows server name - display/animate */
    *font,              /* DUP for draw_info */
    *texture,           /* montage/display background tile */
    *density;           /* DUP for image and draw_info */

  double
    pointsize,
    fuzz;               /* current color fuzz attribute */

  PixelInfo
    alpha_color,        /* deprecated */
    background_color,   /* user set background color */
    border_color,       /* user set border color */
    transparent_color;  /* color for transparent index in color tables */
                        /* NB: fill color is only needed in draw_info! */
                        /* the same for undercolor (for font drawing) */

  MagickBooleanType
    dither,             /* dither enable-disable */
    monochrome;         /* read/write pcl,pdf,ps,xps as monochrome image */

  ColorspaceType
    colorspace;

  CompositeOperator
    compose;

  ImageType
    type;

  MagickBooleanType
    ping,                    /* fast read image attributes, not image data */
    verbose;                 /* verbose output enable/disable */

  ChannelType
    channel;

  void
    *options;                /* splay tree of global options */

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
    magick[MagickPathExtent],    /* image file format (file magick) */
    unique[MagickPathExtent],    /* unique temporary filename - delegates */
    filename[MagickPathExtent];  /* filename when reading/writing image */

  MagickBooleanType
    debug;

  size_t
    signature;

  CustomStreamInfo
    *custom_stream;

  PixelInfo
    matte_color;        /* matte (frame) color */
};

extern MagickExport ChannelType
  SetImageChannelMask(Image *,const ChannelType);

extern MagickExport ExceptionType
  CatchImageException(Image *);

extern MagickExport FILE
  *GetImageInfoFile(const ImageInfo *);

extern MagickExport Image
  *AcquireImage(const ImageInfo *,ExceptionInfo *),
  *AppendImages(const Image *,const MagickBooleanType,ExceptionInfo *),
  *CloneImage(const Image *,const size_t,const size_t,const MagickBooleanType,
    ExceptionInfo *),
  *DestroyImage(Image *),
  *GetImageMask(const Image *,const PixelMask,ExceptionInfo *),
  *NewMagickImage(const ImageInfo *,const size_t,const size_t,const PixelInfo *,
    ExceptionInfo *),
  *ReferenceImage(Image *),
  *SmushImages(const Image *,const MagickBooleanType,const ssize_t,
    ExceptionInfo *);

extern MagickExport ImageInfo
  *AcquireImageInfo(void),
  *CloneImageInfo(const ImageInfo *),
  *DestroyImageInfo(ImageInfo *);

extern MagickExport MagickBooleanType
  ClipImage(Image *,ExceptionInfo *),
  ClipImagePath(Image *,const char *,const MagickBooleanType,ExceptionInfo *),
  CopyImagePixels(Image *,const Image *,const RectangleInfo *,
    const OffsetInfo *,ExceptionInfo *),
  IsTaintImage(const Image *),
  IsHighDynamicRangeImage(const Image *,ExceptionInfo *),
  IsImageObject(const Image *),
  ListMagickInfo(FILE *,ExceptionInfo *),
  ModifyImage(Image **,ExceptionInfo *),
  ResetImagePage(Image *,const char *),
  ResetImagePixels(Image *,ExceptionInfo *),
  SetImageAlpha(Image *,const Quantum,ExceptionInfo *),
  SetImageBackgroundColor(Image *,ExceptionInfo *),
  SetImageColor(Image *,const PixelInfo *,ExceptionInfo *),
  SetImageExtent(Image *,const size_t,const size_t,ExceptionInfo *),
  SetImageInfo(ImageInfo *,const unsigned int,ExceptionInfo *),
  SetImageMask(Image *,const PixelMask type,const Image *,ExceptionInfo *),
  SetImageRegionMask(Image *,const PixelMask type,const RectangleInfo *,
    ExceptionInfo *),
  SetImageStorageClass(Image *,const ClassType,ExceptionInfo *),
  StripImage(Image *,ExceptionInfo *),
  SyncImage(Image *,ExceptionInfo *),
  SyncImageSettings(const ImageInfo *,Image *,ExceptionInfo *),
  SyncImagesSettings(ImageInfo *,Image *,ExceptionInfo *);

extern MagickExport size_t
  InterpretImageFilename(const ImageInfo *,Image *,const char *,int,char *,
    ExceptionInfo *);

extern MagickExport ssize_t
  GetImageReferenceCount(Image *);

extern MagickExport VirtualPixelMethod
  GetImageVirtualPixelMethod(const Image *),
  SetImageVirtualPixelMethod(Image *,const VirtualPixelMethod,ExceptionInfo *);

extern MagickExport void
  AcquireNextImage(const ImageInfo *,Image *,ExceptionInfo *),
  DestroyImagePixels(Image *),
  DisassociateImageStream(Image *),
  GetImageInfo(ImageInfo *),
  SetImageInfoBlob(ImageInfo *,const void *,const size_t),
  SetImageInfoFile(ImageInfo *,FILE *),
  SetImageInfoCustomStream(ImageInfo *,CustomStreamInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
