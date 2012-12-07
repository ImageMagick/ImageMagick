/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     IIIII  M   M   AAA    GGGG  EEEEE                       %
%                       I    MM MM  A   A  G      E                           %
%                       I    M M M  AAAAA  G  GG  EEE                         %
%                       I    M   M  A   A  G   G  E                           %
%                     IIIII  M   M  A   A   GGGG  EEEEE                       %
%                                                                             %
%                                                                             %
%                           MagickCore Image Methods                          %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/animate.h"
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/channel.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/compress.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/display.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/histogram.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/resource_.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/threshold.h"
#include "magick/timer.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/version.h"
#include "magick/xwindow-private.h"

/*
  Constant declaration.
*/
const char
  BackgroundColor[] = "#ffffff",  /* white */
  BorderColor[] = "#dfdfdf",  /* gray */
  DefaultTileFrame[] = "15x15+3+3",
  DefaultTileGeometry[] = "120x120+4+3>",
  DefaultTileLabel[] = "%f\n%G\n%b",
  ForegroundColor[] = "#000",  /* black */
  LoadImageTag[] = "Load/Image",
  LoadImagesTag[] = "Load/Images",
  MatteColor[] = "#bdbdbd",  /* gray */
  PSDensityGeometry[] = "72.0x72.0",
  PSPageGeometry[] = "612x792",
  SaveImageTag[] = "Save/Image",
  SaveImagesTag[] = "Save/Images",
  TransparentColor[] = "#00000000";  /* transparent black */

const double
  DefaultResolution = 72.0;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireImage() returns a pointer to an image structure initialized to
%  default values.
%
%  The format of the AcquireImage method is:
%
%      Image *AcquireImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
*/
MagickExport Image *AcquireImage(const ImageInfo *image_info)
{
  const char
    *option;

  Image
    *image;

  MagickStatusType
    flags;

  /*
    Allocate image structure.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  image=(Image *) AcquireMagickMemory(sizeof(*image));
  if (image == (Image *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(image,0,sizeof(*image));
  /*
    Initialize Image structure.
  */
  (void) CopyMagickString(image->magick,"MIFF",MaxTextExtent);
  image->storage_class=DirectClass;
  image->depth=MAGICKCORE_QUANTUM_DEPTH;
  image->colorspace=sRGBColorspace;
  image->rendering_intent=PerceptualIntent;
  image->gamma=1.000f/2.200f;
  image->chromaticity.red_primary.x=0.6400f;
  image->chromaticity.red_primary.y=0.3300f;
  image->chromaticity.red_primary.z=0.0300f;
  image->chromaticity.green_primary.x=0.3000f;
  image->chromaticity.green_primary.y=0.6000f;
  image->chromaticity.green_primary.z=0.1000f;
  image->chromaticity.blue_primary.x=0.1500f;
  image->chromaticity.blue_primary.y=0.0600f;
  image->chromaticity.blue_primary.z=0.7900f;
  image->chromaticity.white_point.x=0.3127f;
  image->chromaticity.white_point.y=0.3290f;
  image->chromaticity.white_point.z=0.3583f;
  image->interlace=NoInterlace;
  image->ticks_per_second=UndefinedTicksPerSecond;
  image->compose=OverCompositeOp;
  image->blur=1.0;
  GetExceptionInfo(&image->exception);
  (void) QueryColorDatabase(BackgroundColor,&image->background_color,
    &image->exception);
  (void) QueryColorDatabase(BorderColor,&image->border_color,&image->exception);
  (void) QueryColorDatabase(MatteColor,&image->matte_color,&image->exception);
  (void) QueryColorDatabase(TransparentColor,&image->transparent_color,
    &image->exception);
  GetTimerInfo(&image->timer);
  image->ping=MagickFalse;
  image->cache=AcquirePixelCache(0);
  image->blob=CloneBlobInfo((BlobInfo *) NULL);
  image->debug=IsEventLogging();
  image->reference_count=1;
  image->semaphore=AllocateSemaphoreInfo();
  image->signature=MagickSignature;
  if (image_info == (ImageInfo *) NULL)
    return(image);
  /*
    Transfer image info.
  */
  SetBlobExempt(image,image_info->file != (FILE *) NULL ? MagickTrue :
    MagickFalse);
  (void) CopyMagickString(image->filename,image_info->filename,MaxTextExtent);
  (void) CopyMagickString(image->magick_filename,image_info->filename,
    MaxTextExtent);
  (void) CopyMagickString(image->magick,image_info->magick,MaxTextExtent);
  if (image_info->size != (char *) NULL)
    {
      (void) ParseAbsoluteGeometry(image_info->size,&image->extract_info);
      image->columns=image->extract_info.width;
      image->rows=image->extract_info.height;
      image->offset=image->extract_info.x;
      image->extract_info.x=0;
      image->extract_info.y=0;
    }
  if (image_info->extract != (char *) NULL)
    {
      RectangleInfo
        geometry;

      flags=ParseAbsoluteGeometry(image_info->extract,&geometry);
      if (((flags & XValue) != 0) || ((flags & YValue) != 0))
        {
          image->extract_info=geometry;
          Swap(image->columns,image->extract_info.width);
          Swap(image->rows,image->extract_info.height);
        }
    }
  image->compression=image_info->compression;
  image->quality=image_info->quality;
  image->endian=image_info->endian;
  image->interlace=image_info->interlace;
  image->units=image_info->units;
  if (image_info->density != (char *) NULL)
    {
      GeometryInfo
        geometry_info;

      flags=ParseGeometry(image_info->density,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
    }
  if (image_info->page != (char *) NULL)
    {
      char
        *geometry;

      image->page=image->extract_info;
      geometry=GetPageGeometry(image_info->page);
      (void) ParseAbsoluteGeometry(geometry,&image->page);
      geometry=DestroyString(geometry);
    }
  if (image_info->depth != 0)
    image->depth=image_info->depth;
  image->dither=image_info->dither;
  image->background_color=image_info->background_color;
  image->border_color=image_info->border_color;
  image->matte_color=image_info->matte_color;
  image->transparent_color=image_info->transparent_color;
  image->ping=image_info->ping;
  image->progress_monitor=image_info->progress_monitor;
  image->client_data=image_info->client_data;
  if (image_info->cache != (void *) NULL)
    ClonePixelCacheMethods(image->cache,image_info->cache);
  (void) SyncImageSettings(image_info,image);
  option=GetImageOption(image_info,"delay");
  if (option != (const char *) NULL)
    {
      GeometryInfo
        geometry_info;

      flags=ParseGeometry(option,&geometry_info);
      if ((flags & GreaterValue) != 0)
        {
          if (image->delay > (size_t) floor(geometry_info.rho+0.5))
            image->delay=(size_t) floor(geometry_info.rho+0.5);
        }
      else
        if ((flags & LessValue) != 0)
          {
            if (image->delay < (size_t) floor(geometry_info.rho+0.5))
              image->ticks_per_second=(ssize_t) floor(geometry_info.sigma+0.5);
          }
        else
          image->delay=(size_t) floor(geometry_info.rho+0.5);
      if ((flags & SigmaValue) != 0)
        image->ticks_per_second=(ssize_t) floor(geometry_info.sigma+0.5);
    }
  option=GetImageOption(image_info,"dispose");
  if (option != (const char *) NULL)
    image->dispose=(DisposeType) ParseCommandOption(MagickDisposeOptions,
      MagickFalse,option);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I m a g e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireImageInfo() allocates the ImageInfo structure.
%
%  The format of the AcquireImageInfo method is:
%
%      ImageInfo *AcquireImageInfo(void)
%
*/
MagickExport ImageInfo *AcquireImageInfo(void)
{
  ImageInfo
    *image_info;

  image_info=(ImageInfo *) AcquireMagickMemory(sizeof(*image_info));
  if (image_info == (ImageInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  GetImageInfo(image_info);
  return(image_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e N e x t I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireNextImage() initializes the next image in a sequence to
%  default values.  The next member of image points to the newly allocated
%  image.  If there is a memory shortage, next is assigned NULL.
%
%  The format of the AcquireNextImage method is:
%
%      void AcquireNextImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
%    o image: the image.
%
*/
MagickExport void AcquireNextImage(const ImageInfo *image_info,Image *image)
{
  /*
    Allocate image structure.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  image->next=AcquireImage(image_info);
  if (GetNextImageInList(image) == (Image *) NULL)
    return;
  (void) CopyMagickString(GetNextImageInList(image)->filename,image->filename,
    MaxTextExtent);
  if (image_info != (ImageInfo *) NULL)
    (void) CopyMagickString(GetNextImageInList(image)->filename,
      image_info->filename,MaxTextExtent);
  DestroyBlob(GetNextImageInList(image));
  image->next->blob=ReferenceBlob(image->blob);
  image->next->endian=image->endian;
  image->next->scene=image->scene+1;
  image->next->previous=image;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A p p e n d I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AppendImages() takes all images from the current image pointer to the end
%  of the image list and appends them to each other top-to-bottom if the
%  stack parameter is true, otherwise left-to-right.
%
%  The current gravity setting now effects how the image is justified in the
%  final image.
%
%  The format of the AppendImages method is:
%
%      Image *AppendImages(const Image *images,const MagickBooleanType stack,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o stack: A value other than 0 stacks the images top-to-bottom.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AppendImages(const Image *images,
  const MagickBooleanType stack,ExceptionInfo *exception)
{
#define AppendImageTag  "Append/Image"

  CacheView
    *append_view;

  Image
    *append_image;

  MagickBooleanType
    matte,
    proceed,
    status;

  MagickOffsetType
    n;

  RectangleInfo
    geometry;

  register const Image
    *next;

  size_t
    height,
    number_images,
    width;

  ssize_t
    x_offset,
    y,
    y_offset;

  /*
    Compute maximum area of appended area.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  matte=images->matte;
  number_images=1;
  width=images->columns;
  height=images->rows;
  next=GetNextImageInList(images);
  for ( ; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->matte != MagickFalse)
      matte=MagickTrue;
    number_images++;
    if (stack != MagickFalse)
      {
        if (next->columns > width)
          width=next->columns;
        height+=next->rows;
        continue;
      }
    width+=next->columns;
    if (next->rows > height)
      height=next->rows;
  }
  /*
    Append images.
  */
  append_image=CloneImage(images,width,height,MagickTrue,exception);
  if (append_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(append_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&append_image->exception);
      append_image=DestroyImage(append_image);
      return((Image *) NULL);
    }
  append_image->matte=matte;
  (void) SetImageBackgroundColor(append_image);
  status=MagickTrue;
  x_offset=0;
  y_offset=0;
  next=images;
  append_view=AcquireAuthenticCacheView(append_image,exception);
  for (n=0; n < (MagickOffsetType) number_images; n++)
  {
    CacheView
      *image_view;

    Image
      *image;

    image=CloneImage(next,0,0,MagickTrue,exception);
    if (image == (Image *) NULL)
      break;
    status=TransformImageColorspace(image,append_image->colorspace);
    if (status == MagickFalse)
      break;
    SetGeometry(append_image,&geometry);
    GravityAdjustGeometry(image->columns,image->rows,image->gravity,&geometry);
    if (stack != MagickFalse)
      x_offset-=geometry.x;
    else
      y_offset-=geometry.y;
    image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp parallel for schedule(static,4) shared(status) \
      dynamic_number_threads(image,image->columns,image->rows,1)
#endif
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      MagickBooleanType
        sync;

      register const IndexPacket
        *restrict indexes;

      register const PixelPacket
        *restrict p;

      register IndexPacket
        *restrict append_indexes;

      register PixelPacket
        *restrict q;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
      q=QueueCacheViewAuthenticPixels(append_view,x_offset,y+y_offset,
        image->columns,1,exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        {
          status=MagickFalse;
          continue;
        }
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      append_indexes=GetCacheViewAuthenticIndexQueue(append_view);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelRed(q,GetPixelRed(p));
        SetPixelGreen(q,GetPixelGreen(p));
        SetPixelBlue(q,GetPixelBlue(p));
        SetPixelOpacity(q,OpaqueOpacity);
        if (image->matte != MagickFalse)
          SetPixelOpacity(q,GetPixelOpacity(p));
        if ((image->colorspace == CMYKColorspace) &&
            (append_image->colorspace == CMYKColorspace))
          SetPixelIndex(append_indexes+x,GetPixelIndex(indexes+x));
        p++;
        q++;
      }
      sync=SyncCacheViewAuthenticPixels(append_view,exception);
      if (sync == MagickFalse)
        status=MagickFalse;
    }
    image_view=DestroyCacheView(image_view);
    if (stack == MagickFalse)
      {
        x_offset+=(ssize_t) image->columns;
        y_offset=0;
      }
    else
      {
        x_offset=0;
        y_offset+=(ssize_t) image->rows;
      }
    image=DestroyImage(image);
    proceed=SetImageProgress(append_image,AppendImageTag,n,number_images);
    if (proceed == MagickFalse)
      break;
    next=GetNextImageInList(next);
  }
  append_view=DestroyCacheView(append_view);
  if (status == MagickFalse)
    append_image=DestroyImage(append_image);
  return(append_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C a t c h I m a g e E x c e p t i o n                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CatchImageException() returns if no exceptions are found in the image
%  sequence, otherwise it determines the most severe exception and reports
%  it as a warning or error depending on the severity.
%
%  The format of the CatchImageException method is:
%
%      ExceptionType CatchImageException(Image *image)
%
%  A description of each parameter follows:
%
%    o image: An image sequence.
%
*/
MagickExport ExceptionType CatchImageException(Image *image)
{
  ExceptionInfo
    *exception;

  ExceptionType
    severity;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  exception=AcquireExceptionInfo();
  GetImageException(image,exception);
  CatchException(exception);
  severity=exception->severity;
  exception=DestroyExceptionInfo(exception);
  return(severity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l i p I m a g e P a t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClipImagePath() sets the image clip mask based any clipping path information
%  if it exists.
%
%  The format of the ClipImagePath method is:
%
%      MagickBooleanType ClipImagePath(Image *image,const char *pathname,
%        const MagickBooleanType inside)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pathname: name of clipping path resource. If name is preceded by #, use
%      clipping path numbered by name.
%
%    o inside: if non-zero, later operations take effect inside clipping path.
%      Otherwise later operations take effect outside clipping path.
%
*/

MagickExport MagickBooleanType ClipImage(Image *image)
{
  return(ClipImagePath(image,"#1",MagickTrue));
}

MagickExport MagickBooleanType ClipImagePath(Image *image,const char *pathname,
  const MagickBooleanType inside)
{
#define ClipImagePathTag  "ClipPath/Image"

  char
    *property;

  const char
    *value;

  Image
    *clip_mask;

  ImageInfo
    *image_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pathname != NULL);
  property=AcquireString(pathname);
  (void) FormatLocaleString(property,MaxTextExtent,"8BIM:1999,2998:%s",
    pathname);
  value=GetImageProperty(image,property);
  property=DestroyString(property);
  if (value == (const char *) NULL)
    {
      ThrowFileException(&image->exception,OptionError,"NoClipPathDefined",
        image->filename);
      return(MagickFalse);
    }
  image_info=AcquireImageInfo();
  (void) CopyMagickString(image_info->filename,image->filename,MaxTextExtent);
  (void) ConcatenateMagickString(image_info->filename,pathname,MaxTextExtent);
  clip_mask=BlobToImage(image_info,value,strlen(value),&image->exception);
  image_info=DestroyImageInfo(image_info);
  if (clip_mask == (Image *) NULL)
    return(MagickFalse);
  if (clip_mask->storage_class == PseudoClass)
    {
      (void) SyncImage(clip_mask);
      if (SetImageStorageClass(clip_mask,DirectClass) == MagickFalse)
        return(MagickFalse);
    }
  if (inside == MagickFalse)
    (void) NegateImage(clip_mask,MagickFalse);
  (void) FormatLocaleString(clip_mask->magick_filename,MaxTextExtent,
    "8BIM:1999,2998:%s\nPS",pathname);
  (void) SetImageClipMask(image,clip_mask);
  clip_mask=DestroyImage(clip_mask);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImage() copies an image and returns the copy as a new image object.
%
%  If the specified columns and rows is 0, an exact copy of the image is
%  returned, otherwise the pixel data is undefined and must be initialized
%  with the QueueAuthenticPixels() and SyncAuthenticPixels() methods.  On
%  failure, a NULL image is returned and exception describes the reason for the
%  failure.
%
%  The format of the CloneImage method is:
%
%      Image *CloneImage(const Image *image,const size_t columns,
%        const size_t rows,const MagickBooleanType orphan,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the cloned image.
%
%    o rows: the number of rows in the cloned image.
%
%    o detach:  With a value other than 0, the cloned image is detached from
%      its parent I/O stream.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CloneImage(const Image *image,const size_t columns,
  const size_t rows,const MagickBooleanType detach,ExceptionInfo *exception)
{
  double
    scale;

  Image
    *clone_image;

  size_t
    length;

  /*
    Clone the image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  clone_image=(Image *) AcquireMagickMemory(sizeof(*clone_image));
  if (clone_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(clone_image,0,sizeof(*clone_image));
  clone_image->signature=MagickSignature;
  clone_image->storage_class=image->storage_class;
  clone_image->channels=image->channels;
  clone_image->colorspace=image->colorspace;
  clone_image->matte=image->matte;
  clone_image->columns=image->columns;
  clone_image->rows=image->rows;
  clone_image->dither=image->dither;
  if (image->colormap != (PixelPacket *) NULL)
    {
      /*
        Allocate and copy the image colormap.
      */
      clone_image->colors=image->colors;
      length=(size_t) image->colors;
      clone_image->colormap=(PixelPacket *) AcquireQuantumMemory(length,
        sizeof(*clone_image->colormap));
      if (clone_image->colormap == (PixelPacket *) NULL)
        ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
      (void) CopyMagickMemory(clone_image->colormap,image->colormap,length*
        sizeof(*clone_image->colormap));
    }
  (void) CloneImageProfiles(clone_image,image);
  (void) CloneImageProperties(clone_image,image);
  (void) CloneImageArtifacts(clone_image,image);
  GetTimerInfo(&clone_image->timer);
  GetExceptionInfo(&clone_image->exception);
  InheritException(&clone_image->exception,&image->exception);
  if (image->ascii85 != (void *) NULL)
    Ascii85Initialize(clone_image);
  clone_image->magick_columns=image->magick_columns;
  clone_image->magick_rows=image->magick_rows;
  clone_image->type=image->type;
  (void) CopyMagickString(clone_image->magick_filename,image->magick_filename,
    MaxTextExtent);
  (void) CopyMagickString(clone_image->magick,image->magick,MaxTextExtent);
  (void) CopyMagickString(clone_image->filename,image->filename,MaxTextExtent);
  clone_image->progress_monitor=image->progress_monitor;
  clone_image->client_data=image->client_data;
  clone_image->reference_count=1;
  clone_image->next=image->next;
  clone_image->previous=image->previous;
  clone_image->list=NewImageList();
  clone_image->clip_mask=NewImageList();
  clone_image->mask=NewImageList();
  if (detach == MagickFalse)
    clone_image->blob=ReferenceBlob(image->blob);
  else
    {
      clone_image->next=NewImageList();
      clone_image->previous=NewImageList();
      clone_image->blob=CloneBlobInfo((BlobInfo *) NULL);
    }
  clone_image->ping=image->ping;
  clone_image->debug=IsEventLogging();
  clone_image->semaphore=AllocateSemaphoreInfo();
  if ((columns == 0) && (rows == 0))
    {
      if (image->montage != (char *) NULL)
        (void) CloneString(&clone_image->montage,image->montage);
      if (image->directory != (char *) NULL)
        (void) CloneString(&clone_image->directory,image->directory);
      if (image->clip_mask != (Image *) NULL)
        clone_image->clip_mask=CloneImage(image->clip_mask,0,0,MagickTrue,
          exception);
      if (image->mask != (Image *) NULL)
        clone_image->mask=CloneImage(image->mask,0,0,MagickTrue,exception);
      clone_image->cache=ReferencePixelCache(image->cache);
      return(clone_image);
    }
  if ((columns == image->columns) && (rows == image->rows))
    {
      if (image->clip_mask != (Image *) NULL)
        clone_image->clip_mask=CloneImage(image->clip_mask,0,0,MagickTrue,
          exception);
      if (image->mask != (Image *) NULL)
        clone_image->mask=CloneImage(image->mask,0,0,MagickTrue,exception);
    }
  scale=(double) columns/(double) image->columns;
  clone_image->page.width=(size_t) floor(scale*image->page.width+0.5);
  clone_image->page.x=(ssize_t) ceil(scale*image->page.x-0.5);
  clone_image->tile_offset.x=(ssize_t) ceil(scale*image->tile_offset.x-0.5);
  scale=(double) rows/(double) image->rows;
  clone_image->page.height=(size_t) floor(scale*image->page.height+0.5);
  clone_image->page.y=(ssize_t) ceil(scale*image->page.y-0.5);
  clone_image->tile_offset.y=(ssize_t) ceil(scale*image->tile_offset.y-0.5);
  clone_image->columns=columns;
  clone_image->rows=rows;
  clone_image->cache=ClonePixelCache(image->cache);
  return(clone_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageInfo() makes a copy of the given image info structure.  If
%  NULL is specified, a new image info structure is created initialized to
%  default values.
%
%  The format of the CloneImageInfo method is:
%
%      ImageInfo *CloneImageInfo(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport ImageInfo *CloneImageInfo(const ImageInfo *image_info)
{
  ImageInfo
    *clone_info;

  clone_info=AcquireImageInfo();
  if (image_info == (ImageInfo *) NULL)
    return(clone_info);
  clone_info->compression=image_info->compression;
  clone_info->temporary=image_info->temporary;
  clone_info->adjoin=image_info->adjoin;
  clone_info->antialias=image_info->antialias;
  clone_info->scene=image_info->scene;
  clone_info->number_scenes=image_info->number_scenes;
  clone_info->depth=image_info->depth;
  (void) CloneString(&clone_info->size,image_info->size);
  (void) CloneString(&clone_info->extract,image_info->extract);
  (void) CloneString(&clone_info->scenes,image_info->scenes);
  (void) CloneString(&clone_info->page,image_info->page);
  clone_info->interlace=image_info->interlace;
  clone_info->endian=image_info->endian;
  clone_info->units=image_info->units;
  clone_info->quality=image_info->quality;
  (void) CloneString(&clone_info->sampling_factor,image_info->sampling_factor);
  (void) CloneString(&clone_info->server_name,image_info->server_name);
  (void) CloneString(&clone_info->font,image_info->font);
  (void) CloneString(&clone_info->texture,image_info->texture);
  (void) CloneString(&clone_info->density,image_info->density);
  clone_info->pointsize=image_info->pointsize;
  clone_info->fuzz=image_info->fuzz;
  clone_info->pen=image_info->pen;
  clone_info->background_color=image_info->background_color;
  clone_info->border_color=image_info->border_color;
  clone_info->matte_color=image_info->matte_color;
  clone_info->transparent_color=image_info->transparent_color;
  clone_info->dither=image_info->dither;
  clone_info->monochrome=image_info->monochrome;
  clone_info->colors=image_info->colors;
  clone_info->colorspace=image_info->colorspace;
  clone_info->type=image_info->type;
  clone_info->orientation=image_info->orientation;
  clone_info->preview_type=image_info->preview_type;
  clone_info->group=image_info->group;
  clone_info->ping=image_info->ping;
  clone_info->verbose=image_info->verbose;
  (void) CloneString(&clone_info->view,image_info->view);
  (void) CloneString(&clone_info->authenticate,image_info->authenticate);
  (void) CloneImageOptions(clone_info,image_info);
  clone_info->progress_monitor=image_info->progress_monitor;
  clone_info->client_data=image_info->client_data;
  clone_info->cache=image_info->cache;
  if (image_info->cache != (void *) NULL)
    clone_info->cache=ReferencePixelCache(image_info->cache);
  if (image_info->profile != (void *) NULL)
    clone_info->profile=(void *) CloneStringInfo((StringInfo *)
      image_info->profile);
  SetImageInfoFile(clone_info,image_info->file);
  SetImageInfoBlob(clone_info,image_info->blob,image_info->length);
  clone_info->stream=image_info->stream;
  clone_info->virtual_pixel_method=image_info->virtual_pixel_method;
  (void) CopyMagickString(clone_info->magick,image_info->magick,MaxTextExtent);
  (void) CopyMagickString(clone_info->unique,image_info->unique,MaxTextExtent);
  (void) CopyMagickString(clone_info->zero,image_info->zero,MaxTextExtent);
  (void) CopyMagickString(clone_info->filename,image_info->filename,
    MaxTextExtent);
  clone_info->subimage=image_info->scene;  /* deprecated */
  clone_info->subrange=image_info->number_scenes;  /* deprecated */
  clone_info->channel=image_info->channel;
  clone_info->debug=IsEventLogging();
  clone_info->signature=image_info->signature;
  return(clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImage() dereferences an image, deallocating memory associated with
%  the image if the reference count becomes zero.
%
%  The format of the DestroyImage method is:
%
%      Image *DestroyImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *DestroyImage(Image *image)
{
  MagickBooleanType
    destroy;

  /*
    Dereference image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  destroy=MagickFalse;
  LockSemaphoreInfo(image->semaphore);
  image->reference_count--;
  if (image->reference_count == 0)
    destroy=MagickTrue;
  UnlockSemaphoreInfo(image->semaphore);
  if (destroy == MagickFalse)
    return((Image *) NULL);
  /*
    Destroy image.
  */
  DestroyImagePixels(image);
  if (image->clip_mask != (Image *) NULL)
    image->clip_mask=DestroyImage(image->clip_mask);
  if (image->mask != (Image *) NULL)
    image->mask=DestroyImage(image->mask);
  if (image->montage != (char *) NULL)
    image->montage=DestroyString(image->montage);
  if (image->directory != (char *) NULL)
    image->directory=DestroyString(image->directory);
  if (image->colormap != (PixelPacket *) NULL)
    image->colormap=(PixelPacket *) RelinquishMagickMemory(image->colormap);
  if (image->geometry != (char *) NULL)
    image->geometry=DestroyString(image->geometry);
  DestroyImageProfiles(image);
  DestroyImageProperties(image);
  DestroyImageArtifacts(image);
  if (image->ascii85 != (Ascii85Info*) NULL)
    image->ascii85=(Ascii85Info *) RelinquishMagickMemory(image->ascii85);
  DestroyBlob(image);
  (void) DestroyExceptionInfo(&image->exception);
  if (image->semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&image->semaphore);
  image->signature=(~MagickSignature);
  image=(Image *) RelinquishMagickMemory(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageInfo() deallocates memory associated with an ImageInfo
%  structure.
%
%  The format of the DestroyImageInfo method is:
%
%      ImageInfo *DestroyImageInfo(ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport ImageInfo *DestroyImageInfo(ImageInfo *image_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->size != (char *) NULL)
    image_info->size=DestroyString(image_info->size);
  if (image_info->extract != (char *) NULL)
    image_info->extract=DestroyString(image_info->extract);
  if (image_info->scenes != (char *) NULL)
    image_info->scenes=DestroyString(image_info->scenes);
  if (image_info->page != (char *) NULL)
    image_info->page=DestroyString(image_info->page);
  if (image_info->sampling_factor != (char *) NULL)
    image_info->sampling_factor=DestroyString(
      image_info->sampling_factor);
  if (image_info->server_name != (char *) NULL)
    image_info->server_name=DestroyString(
      image_info->server_name);
  if (image_info->font != (char *) NULL)
    image_info->font=DestroyString(image_info->font);
  if (image_info->texture != (char *) NULL)
    image_info->texture=DestroyString(image_info->texture);
  if (image_info->density != (char *) NULL)
    image_info->density=DestroyString(image_info->density);
  if (image_info->view != (char *) NULL)
    image_info->view=DestroyString(image_info->view);
  if (image_info->authenticate != (char *) NULL)
    image_info->authenticate=DestroyString(
      image_info->authenticate);
  DestroyImageOptions(image_info);
  if (image_info->cache != (void *) NULL)
    image_info->cache=DestroyPixelCache(image_info->cache);
  if (image_info->profile != (StringInfo *) NULL)
    image_info->profile=(void *) DestroyStringInfo((StringInfo *)
      image_info->profile);
  image_info->signature=(~MagickSignature);
  image_info=(ImageInfo *) RelinquishMagickMemory(image_info);
  return(image_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s a s s o c i a t e I m a g e S t r e a m                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DisassociateImageStream() disassociates the image stream.
%
%  The format of the DisassociateImageStream method is:
%
%      MagickBooleanType DisassociateImageStream(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DisassociateImageStream(Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) DetachBlob(image->blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C l i p M a s k                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageClipMask() returns the clip path associated with the image.
%
%  The format of the GetImageClipMask method is:
%
%      Image *GetImageClipMask(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *GetImageClipMask(const Image *image,
  ExceptionInfo *exception)
{
  assert(image != (const Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->clip_mask == (Image *) NULL)
    return((Image *) NULL);
  return(CloneImage(image->clip_mask,0,0,MagickTrue,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e E x c e p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageException() traverses an image sequence and returns any
%  error more severe than noted by the exception parameter.
%
%  The format of the GetImageException method is:
%
%      void GetImageException(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: Specifies a pointer to a list of one or more images.
%
%    o exception: return the highest severity exception.
%
*/
MagickExport void GetImageException(Image *image,ExceptionInfo *exception)
{
  register Image
    *next;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->exception.severity == UndefinedException)
      continue;
    if (next->exception.severity > exception->severity)
      InheritException(exception,&next->exception);
    next->exception.severity=UndefinedException;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageInfo() initializes image_info to default values.
%
%  The format of the GetImageInfo method is:
%
%      void GetImageInfo(ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport void GetImageInfo(ImageInfo *image_info)
{
  const char
    *synchronize;

  ExceptionInfo
    *exception;

  /*
    File and image dimension members.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image_info != (ImageInfo *) NULL);
  (void) ResetMagickMemory(image_info,0,sizeof(*image_info));
  image_info->adjoin=MagickTrue;
  image_info->interlace=NoInterlace;
  image_info->channel=DefaultChannels;
  image_info->quality=UndefinedCompressionQuality;
  image_info->antialias=MagickTrue;
  image_info->dither=MagickTrue;
  synchronize=GetEnvironmentValue("MAGICK_SYNCHRONIZE");
  if (synchronize != (const char *) NULL)
    image_info->synchronize=IsMagickTrue(synchronize);
  exception=AcquireExceptionInfo();
  (void) QueryColorDatabase(BackgroundColor,&image_info->background_color,
    exception);
  (void) QueryColorDatabase(BorderColor,&image_info->border_color,exception);
  (void) QueryColorDatabase(MatteColor,&image_info->matte_color,exception);
  (void) QueryColorDatabase(TransparentColor,&image_info->transparent_color,
    exception);
  exception=DestroyExceptionInfo(exception);
  image_info->debug=IsEventLogging();
  image_info->signature=MagickSignature;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e I n f o F i l e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageInfoFile() returns the image info file member.
%
%  The format of the GetImageInfoFile method is:
%
%      FILE *GetImageInfoFile(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport FILE *GetImageInfoFile(const ImageInfo *image_info)
{
  return(image_info->file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e M a s k                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageMask() returns the mask associated with the image.
%
%  The format of the GetImageMask method is:
%
%      Image *GetImageMask(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *GetImageMask(const Image *image,ExceptionInfo *exception)
{
  assert(image != (const Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->mask == (Image *) NULL)
    return((Image *) NULL);
  return(CloneImage(image->mask,0,0,MagickTrue,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannels() returns the number of pixel channels associated with the
%  specified image.
%
%  The format of the GetChannels method is:
%
%      size_t GetImageChannels(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport size_t GetImageChannels(Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(image->channels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e R e f e r e n c e C o u n t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageReferenceCount() returns the image reference count.
%
%  The format of the GetReferenceCount method is:
%
%      ssize_t GetImageReferenceCount(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport ssize_t GetImageReferenceCount(Image *image)
{
  ssize_t
    reference_count;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  LockSemaphoreInfo(image->semaphore);
  reference_count=image->reference_count;
  UnlockSemaphoreInfo(image->semaphore);
  return(reference_count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i r t u a l P i x e l M e t h o d                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageVirtualPixelMethod() gets the "virtual pixels" method for the
%  image.  A virtual pixel is any pixel access that is outside the boundaries
%  of the image cache.
%
%  The format of the GetImageVirtualPixelMethod() method is:
%
%      VirtualPixelMethod GetImageVirtualPixelMethod(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport VirtualPixelMethod GetImageVirtualPixelMethod(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(GetPixelCacheVirtualMethod(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I n t e r p r e t I m a g e F i l e n a m e                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretImageFilename() interprets embedded characters in an image filename.
%  The filename length is returned.
%
%  The format of the InterpretImageFilename method is:
%
%      size_t InterpretImageFilename(const ImageInfo *image_info,Image *image,
%        const char *format,int value,char *filename)
%
%  A description of each parameter follows.
%
%    o image_info: the image info..
%
%    o image: the image.
%
%    o format:  A filename describing the format to use to write the numeric
%      argument. Only the first numeric format identifier is replaced.
%
%    o value:  Numeric value to substitute into format filename.
%
%    o filename:  return the formatted filename in this character buffer.
%
*/
MagickExport size_t InterpretImageFilename(const ImageInfo *image_info,
  Image *image,const char *format,int value,char *filename)
{
  char
    *q;

  int
    c;

  MagickBooleanType
    canonical;

  register const char
    *p;

  size_t
    length;

  canonical=MagickFalse;
  length=0;
  (void) CopyMagickString(filename,format,MaxTextExtent);
  for (p=strchr(format,'%'); p != (char *) NULL; p=strchr(p+1,'%'))
  {
    q=(char *) p+1;
    if (*q == '%')
      {
        p=q+1;
        continue;
      }
    if (*q == '0')
      {
        ssize_t
          value;

        value=(ssize_t) strtol(q,&q,10);
        (void) value;
      }
    switch (*q)
    {
      case 'd':
      case 'o':
      case 'x':
      {
        q++;
        c=(*q);
        *q='\0';
        (void) FormatLocaleString(filename+(p-format),(size_t) (MaxTextExtent-
          (p-format)),p,value);
        *q=c;
        (void) ConcatenateMagickString(filename,q,MaxTextExtent);
        canonical=MagickTrue;
        if (*(q-1) != '%')
          break;
        p++;
        break;
      }
      case '[':
      {
        char
          pattern[MaxTextExtent];

        const char
          *value;

        register char
          *r;

        register ssize_t
          i;

        ssize_t
          depth;

        /*
          Image option.
        */
        if (strchr(p,']') == (char *) NULL)
          break;
        depth=1;
        r=q+1;
        for (i=0; (i < (MaxTextExtent-1L)) && (*r != '\0'); i++)
        {
          if (*r == '[')
            depth++;
          if (*r == ']')
            depth--;
          if (depth <= 0)
            break;
          pattern[i]=(*r++);
        }
        pattern[i]='\0';
        if (LocaleNCompare(pattern,"filename:",9) != 0)
          break;
        value=(const char *) NULL;
#if 0
        // FUTURE: remove this code. -- Anthony  29 Arpil 2012
        // Removed as GetMagickProperty() will will never match a "filename:"
        // string as this is not a 'known' image property.
        //
        if ((image_info != (const ImageInfo *) NULL) &&
            (image != (const Image *) NULL))
          value=GetMagickProperty(image_info,image,pattern);
        else
#endif
        if (image != (Image *) NULL)
          value=GetImageProperty(image,pattern);
        if ((value == (const char *) NULL) &&
            (image != (Image *) NULL))
          value=GetImageArtifact(image,pattern);
        if ((value == (const char *) NULL) &&
            (image_info != (ImageInfo *) NULL))
          value=GetImageOption(image_info,pattern);
        if (value == (const char *) NULL)
          break;
        q--;
        c=(*q);
        *q='\0';
        (void) CopyMagickString(filename+(p-format-length),value,(size_t)
          (MaxTextExtent-(p-format-length)));
        length+=strlen(pattern)-1;
        *q=c;
        (void) ConcatenateMagickString(filename,r+1,MaxTextExtent);
        canonical=MagickTrue;
        if (*(q-1) != '%')
          break;
        p++;
        break;
      }
      default:
        break;
    }
  }
  for (q=filename; *q != '\0'; q++)
    if ((*q == '%') && (*(q+1) == '%'))
      {
        (void) CopyMagickString(q,q+1,(size_t) (MaxTextExtent-(q-filename)));
        canonical=MagickTrue;
      }
  if (canonical == MagickFalse)
    (void) CopyMagickString(filename,format,MaxTextExtent);
  return(strlen(filename));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H i g h D y n a m i c R a n g e I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHighDynamicRangeImage() returns MagickTrue if any pixel component is
%  non-integer or exceeds the bounds of the quantum depth (e.g. for Q16
%  0..65535.
%
%  The format of the IsHighDynamicRangeImage method is:
%
%      MagickBooleanType IsHighDynamicRangeImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsHighDynamicRangeImage(const Image *image,
  ExceptionInfo *exception)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  (void) image;
  (void) exception;
  return(MagickFalse);
#else
  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  GetMagickPixelPacket(image,&zero);
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register const IndexPacket
      *indexes;

    register const PixelPacket
      *p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    pixel=zero;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((pixel.red < 0.0) || (pixel.red > QuantumRange) ||
          (pixel.red != (QuantumAny) pixel.red))
        break;
      if ((pixel.green < 0.0) || (pixel.green > QuantumRange) ||
          (pixel.green != (QuantumAny) pixel.green))
        break;
      if ((pixel.blue < 0.0) || (pixel.blue > QuantumRange) ||
          (pixel.blue != (QuantumAny) pixel.blue))
        break;
      if (pixel.matte != MagickFalse)
        {
          if ((pixel.opacity < 0.0) || (pixel.opacity > QuantumRange) ||
              (pixel.opacity != (QuantumAny) pixel.opacity))
            break;
        }
      if (pixel.colorspace == CMYKColorspace)
        {
          if ((pixel.index < 0.0) || (pixel.index > QuantumRange) ||
              (pixel.index != (QuantumAny) pixel.index))
            break;
        }
      p++;
    }
    if (x < (ssize_t) image->columns)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status != MagickFalse ? MagickFalse : MagickTrue);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s I m a g e O b j e c t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageObject() returns MagickTrue if the image sequence contains a valid
%  set of image objects.
%
%  The format of the IsImageObject method is:
%
%      MagickBooleanType IsImageObject(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsImageObject(const Image *image)
{
  register const Image
    *p;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
    if (p->signature != MagickSignature)
      return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s T a i n t I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsTaintImage() returns MagickTrue any pixel in the image has been altered
%  since it was first constituted.
%
%  The format of the IsTaintImage method is:
%
%      MagickBooleanType IsTaintImage(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsTaintImage(const Image *image)
{
  char
    magick[MaxTextExtent],
    filename[MaxTextExtent];

  register const Image
    *p;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  (void) CopyMagickString(magick,image->magick,MaxTextExtent);
  (void) CopyMagickString(filename,image->filename,MaxTextExtent);
  for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    if (p->taint != MagickFalse)
      return(MagickTrue);
    if (LocaleCompare(p->magick,magick) != 0)
      return(MagickTrue);
    if (LocaleCompare(p->filename,filename) != 0)
      return(MagickTrue);
  }
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M o d i f y I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModifyImage() ensures that there is only a single reference to the image
%  to be modified, updating the provided image pointer to point to a clone of
%  the original image if necessary.
%
%  The format of the ModifyImage method is:
%
%      MagickBooleanType ModifyImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ModifyImage(Image **image,
  ExceptionInfo *exception)
{
  Image
    *clone_image;

  assert(image != (Image **) NULL);
  assert(*image != (Image *) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  if (GetImageReferenceCount(*image) <= 1)
    return(MagickTrue);
  clone_image=CloneImage(*image,0,0,MagickTrue,exception);
  LockSemaphoreInfo((*image)->semaphore);
  (*image)->reference_count--;
  UnlockSemaphoreInfo((*image)->semaphore);
  *image=clone_image;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w M a g i c k I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewMagickImage() creates a blank image canvas of the specified size and
%  background color.
%
%  The format of the NewMagickImage method is:
%
%      Image *NewMagickImage(const ImageInfo *image_info,
%        const size_t width,const size_t height,
%        const MagickPixelPacket *background)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the image width.
%
%    o height: the image height.
%
%    o background: the image color.
%
*/
MagickExport Image *NewMagickImage(const ImageInfo *image_info,
  const size_t width,const size_t height,
  const MagickPixelPacket *background)
{
  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  Image
    *image;

  ssize_t
    y;

  MagickBooleanType
    status;

  assert(image_info != (const ImageInfo *) NULL);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image_info->signature == MagickSignature);
  assert(background != (const MagickPixelPacket *) NULL);
  image=AcquireImage(image_info);
  image->columns=width;
  image->rows=height;
  image->colorspace=background->colorspace;
  image->matte=background->matte;
  image->fuzz=background->fuzz;
  image->depth=background->depth;
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelPacket(image,background,q,indexes+x);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e f e r e n c e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReferenceImage() increments the reference count associated with an image
%  returning a pointer to the image.
%
%  The format of the ReferenceImage method is:
%
%      Image *ReferenceImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *ReferenceImage(Image *image)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  LockSemaphoreInfo(image->semaphore);
  image->reference_count++;
  UnlockSemaphoreInfo(image->semaphore);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e P a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImagePage() resets the image page canvas and position.
%
%  The format of the ResetImagePage method is:
%
%      MagickBooleanType ResetImagePage(Image *image,const char *page)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o page: the relative page specification.
%
*/
MagickExport MagickBooleanType ResetImagePage(Image *image,const char *page)
{
  MagickStatusType
    flags;

  RectangleInfo
    geometry;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  flags=ParseAbsoluteGeometry(page,&geometry);
  if ((flags & WidthValue) != 0)
    {
      if ((flags & HeightValue) == 0)
        geometry.height=geometry.width;
      image->page.width=geometry.width;
      image->page.height=geometry.height;
    }
  if ((flags & AspectValue) != 0)
    {
      if ((flags & XValue) != 0)
        image->page.x+=geometry.x;
      if ((flags & YValue) != 0)
        image->page.y+=geometry.y;
    }
  else
    {
      if ((flags & XValue) != 0)
        {
          image->page.x=geometry.x;
          if ((image->page.width == 0) && (geometry.x > 0))
            image->page.width=image->columns+geometry.x;
        }
      if ((flags & YValue) != 0)
        {
          image->page.y=geometry.y;
          if ((image->page.height == 0) && (geometry.y > 0))
            image->page.height=image->rows+geometry.y;
        }
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e B a c k g r o u n d C o l o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageBackgroundColor() initializes the image pixels to the image
%  background color.  The background color is defined by the background_color
%  member of the image structure.
%
%  The format of the SetImage method is:
%
%      MagickBooleanType SetImageBackgroundColor(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType SetImageBackgroundColor(Image *image)
{
  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  IndexPacket
    index;

  MagickBooleanType
    status;

  MagickPixelPacket
    background;

  PixelPacket
    pixel;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if ((IsPixelGray(&image->background_color) == MagickFalse) &&
      (IsGrayColorspace(image->colorspace) != MagickFalse))
    (void) TransformImageColorspace(image,RGBColorspace);
  if ((image->background_color.opacity != OpaqueOpacity) &&
      (image->matte == MagickFalse))
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  GetMagickPixelPacket(image,&background);
  SetMagickPixelPacket(image,&image->background_color,(const IndexPacket *)
    NULL,&background);
  if (image->colorspace == CMYKColorspace)
    ConvertRGBToCMYK(&background);
  index=0;
  SetPixelPacket(image,&background,&pixel,&index);
  /*
    Set image background color.
  */
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
      *q++=pixel;
    if (image->colorspace == CMYKColorspace)
      {
        register IndexPacket
          *restrict indexes;

        indexes=GetCacheViewAuthenticIndexQueue(image_view);
        for (x=0; x < (ssize_t) image->columns; x++)
          SetPixelIndex(indexes+x,index);
      }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e C h a n n e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageChannels() sets the number of pixels channels associated with the
%  image.
%
%  The format of the SetImageChannels method is:
%
%      MagickBooleanType SetImageChannels(Image *image,const size_t channels)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channels:  The number of pixel channels.
%
*/
MagickExport MagickBooleanType SetImageChannels(Image *image,
  const size_t channels)
{
  image->channels=channels;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e C o l o r                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageColor() set the entire image canvas to the specified color.
%
%  The format of the SetImageColor method is:
%
%      MagickBooleanType SetImageColor(Image *image,
%        const MagickPixelPacket *color)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o background: the image color.
%
*/
MagickExport MagickBooleanType SetImageColor(Image *image,
  const MagickPixelPacket *color)
{
  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  assert(color != (const MagickPixelPacket *) NULL);
  image->colorspace=color->colorspace;
  image->matte=color->matte;
  image->fuzz=color->fuzz;
  image->depth=color->depth;
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelPacket(image,color,q,indexes+x);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e S t o r a g e C l a s s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageStorageClass() sets the image class: DirectClass for true color
%  images or PseudoClass for colormapped images.
%
%  The format of the SetImageStorageClass method is:
%
%      MagickBooleanType SetImageStorageClass(Image *image,
%        const ClassType storage_class)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o storage_class:  The image class.
%
*/
MagickExport MagickBooleanType SetImageStorageClass(Image *image,
  const ClassType storage_class)
{
  image->storage_class=storage_class;
  return(SyncImagePixelCache(image,&image->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e C l i p M a s k                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageClipMask() associates a clip path with the image.  The clip path
%  must be the same dimensions as the image.  Set any pixel component of
%  the clip path to TransparentOpacity to prevent that corresponding image
%  pixel component from being updated when SyncAuthenticPixels() is applied.
%
%  The format of the SetImageClipMask method is:
%
%      MagickBooleanType SetImageClipMask(Image *image,const Image *clip_mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o clip_mask: the image clip path.
%
*/
MagickExport MagickBooleanType SetImageClipMask(Image *image,
  const Image *clip_mask)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (clip_mask != (const Image *) NULL)
    if ((clip_mask->columns != image->columns) ||
        (clip_mask->rows != image->rows))
      ThrowBinaryException(ImageError,"ImageSizeDiffers",image->filename);
  if (image->clip_mask != (Image *) NULL)
    image->clip_mask=DestroyImage(image->clip_mask);
  image->clip_mask=NewImageList();
  if (clip_mask == (Image *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  image->clip_mask=CloneImage(clip_mask,0,0,MagickTrue,&image->exception);
  if (image->clip_mask == (Image *) NULL)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e E x t e n t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageExtent() sets the image size (i.e. columns & rows).
%
%  The format of the SetImageExtent method is:
%
%      MagickBooleanType SetImageExtent(Image *image,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns:  The image width in pixels.
%
%    o rows:  The image height in pixels.
%
*/
MagickExport MagickBooleanType SetImageExtent(Image *image,
  const size_t columns,const size_t rows)
{
  if ((columns == 0) || (rows == 0))
    return(MagickFalse);
  image->columns=columns;
  image->rows=rows;
  return(SyncImagePixelCache(image,&image->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t I m a g e I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfo() initializes the `magick' field of the ImageInfo structure.
%  It is set to a type of image format based on the prefix or suffix of the
%  filename.  For example, `ps:image' returns PS indicating a Postscript image.
%  JPEG is returned for this filename: `image.jpg'.  The filename prefix has
%  precendence over the suffix.  Use an optional index enclosed in brackets
%  after a file name to specify a desired scene of a multi-resolution image
%  format like Photo CD (e.g. img0001.pcd[4]).  A True (non-zero) return value
%  indicates success.
%
%  The format of the SetImageInfo method is:
%
%      MagickBooleanType SetImageInfo(ImageInfo *image_info,
%        const unsigned int frames,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o frames: the number of images you intend to write.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageInfo(ImageInfo *image_info,
  const unsigned int frames,ExceptionInfo *exception)
{
  char
    extension[MaxTextExtent],
    filename[MaxTextExtent],
    magic[MaxTextExtent],
    *q,
    subimage[MaxTextExtent];

  const MagicInfo
    *magic_info;

  const MagickInfo
    *magick_info;

  ExceptionInfo
    *sans_exception;

  Image
    *image;

  MagickBooleanType
    status;

  register const char
    *p;

  ssize_t
    count;

  unsigned char
    magick[2*MaxTextExtent];

  /*
    Look for 'image.format' in filename.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  *subimage='\0';
  if (frames == 0)
    {
      GetPathComponent(image_info->filename,SubimagePath,subimage);
      if (*subimage != '\0')
        {
          /*
            Look for scene specification (e.g. img0001.pcd[4]).
          */
          if (IsSceneGeometry(subimage,MagickFalse) == MagickFalse)
            {
              if (IsGeometry(subimage) != MagickFalse)
                (void) CloneString(&image_info->extract,subimage);
            }
          else
            {
              size_t
                first,
                last;

              (void) CloneString(&image_info->scenes,subimage);
              image_info->scene=StringToUnsignedLong(image_info->scenes);
              image_info->number_scenes=image_info->scene;
              p=image_info->scenes;
              for (q=(char *) image_info->scenes; *q != '\0'; p++)
              {
                while ((isspace((int) ((unsigned char) *p)) != 0) ||
                       (*p == ','))
                  p++;
                first=(size_t) strtol(p,&q,10);
                last=first;
                while (isspace((int) ((unsigned char) *q)) != 0)
                  q++;
                if (*q == '-')
                  last=(size_t) strtol(q+1,&q,10);
                if (first > last)
                  Swap(first,last);
                if (first < image_info->scene)
                  image_info->scene=first;
                if (last > image_info->number_scenes)
                  image_info->number_scenes=last;
                p=q;
              }
              image_info->number_scenes-=image_info->scene-1;
              image_info->subimage=image_info->scene;
              image_info->subrange=image_info->number_scenes;
            }
        }
    }
  *extension='\0';
  GetPathComponent(image_info->filename,ExtensionPath,extension);
#if defined(MAGICKCORE_ZLIB_DELEGATE)
  if (*extension != '\0')
    if ((LocaleCompare(extension,"gz") == 0) ||
        (LocaleCompare(extension,"Z") == 0) ||
        (LocaleCompare(extension,"svgz") == 0) ||
        (LocaleCompare(extension,"wmz") == 0))
      {
        char
          path[MaxTextExtent];

        (void) CopyMagickString(path,image_info->filename,MaxTextExtent);
        path[strlen(path)-strlen(extension)-1]='\0';
        GetPathComponent(path,ExtensionPath,extension);
      }
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  if (*extension != '\0')
    if (LocaleCompare(extension,"bz2") == 0)
      {
        char
          path[MaxTextExtent];

        (void) CopyMagickString(path,image_info->filename,MaxTextExtent);
        path[strlen(path)-strlen(extension)-1]='\0';
        GetPathComponent(path,ExtensionPath,extension);
      }
#endif
  image_info->affirm=MagickFalse;
  sans_exception=AcquireExceptionInfo();
  if (*extension != '\0')
    {
      MagickFormatType
        format_type;

      register ssize_t
        i;

      static const char
        *format_type_formats[] =
        {
          "AUTOTRACE",
          "BROWSE",
          "DCRAW",
          "EDIT",
          "EPHEMERAL",
          "LAUNCH",
          "MPEG:DECODE",
          "MPEG:ENCODE",
          "PRINT",
          "PS:ALPHA",
          "PS:CMYK",
          "PS:COLOR",
          "PS:GRAY",
          "PS:MONO",
          "SCAN",
          "SHOW",
          "WIN",
          (char *) NULL
        };

      /*
        User specified image format.
      */
      (void) CopyMagickString(magic,extension,MaxTextExtent);
      LocaleUpper(magic);
      /*
        Look for explicit image formats.
      */
      format_type=UndefinedFormatType;
      i=0;
      while ((format_type == UndefinedFormatType) &&
             (format_type_formats[i] != (char *) NULL))
      {
        if ((*magic == *format_type_formats[i]) &&
            (LocaleCompare(magic,format_type_formats[i]) == 0))
          format_type=ExplicitFormatType;
        i++;
      }
      magick_info=GetMagickInfo(magic,sans_exception);
      if ((magick_info != (const MagickInfo *) NULL) &&
          (magick_info->format_type != UndefinedFormatType))
        format_type=magick_info->format_type;
      if (format_type == UndefinedFormatType)
        (void) CopyMagickString(image_info->magick,magic,MaxTextExtent);
      else
        if (format_type == ExplicitFormatType)
          {
            image_info->affirm=MagickTrue;
            (void) CopyMagickString(image_info->magick,magic,MaxTextExtent);
          }
      if (LocaleCompare(magic,"RGB") == 0)
        image_info->affirm=MagickFalse;  /* maybe SGI disguised as RGB */
    }
  /*
    Look for explicit 'format:image' in filename.
  */
  *magic='\0';
  GetPathComponent(image_info->filename,MagickPath,magic);
  if (*magic == '\0')
    (void) CopyMagickString(magic,image_info->magick,MaxTextExtent);
  else
    {
      /*
        User specified image format.
      */
      LocaleUpper(magic);
      if (IsMagickConflict(magic) == MagickFalse)
        {
          (void) CopyMagickString(image_info->magick,magic,MaxTextExtent);
          if (LocaleCompare(magic,"EPHEMERAL") != 0)
            image_info->affirm=MagickTrue;
          else
            image_info->temporary=MagickTrue;
        }
    }
  magick_info=GetMagickInfo(magic,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetMagickEndianSupport(magick_info) == MagickFalse))
    image_info->endian=UndefinedEndian;
  GetPathComponent(image_info->filename,CanonicalPath,filename);
  (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
  if ((image_info->adjoin != MagickFalse) && (frames > 1))
    {
      /*
        Test for multiple image support (e.g. image%02d.png).
      */
      (void) InterpretImageFilename(image_info,(Image *) NULL,
        image_info->filename,(int) image_info->scene,filename);
      if ((LocaleCompare(filename,image_info->filename) != 0) &&
          (strchr(filename,'%') == (char *) NULL))
        image_info->adjoin=MagickFalse;
    }
  if ((image_info->adjoin != MagickFalse) && (frames > 0))
    {
      /*
        Some image formats do not support multiple frames per file.
      */
      magick_info=GetMagickInfo(magic,exception);
      if (magick_info != (const MagickInfo *) NULL)
        if (GetMagickAdjoin(magick_info) == MagickFalse)
          image_info->adjoin=MagickFalse;
    }
  if (image_info->affirm != MagickFalse)
    return(MagickTrue);
  if (frames == 0)
    {
      /*
        Determine the image format from the first few bytes of the file.
      */
      image=AcquireImage(image_info);
      (void) CopyMagickString(image->filename,image_info->filename,
        MaxTextExtent);
      status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
      if (status == MagickFalse)
        {
          image=DestroyImage(image);
          return(MagickFalse);
        }
      if ((IsBlobSeekable(image) == MagickFalse) ||
          (IsBlobExempt(image) != MagickFalse))
        {
          /*
            Copy standard input or pipe to temporary file.
          */
          *filename='\0';
          status=ImageToFile(image,filename,exception);
          (void) CloseBlob(image);
          if (status == MagickFalse)
            {
              image=DestroyImage(image);
              return(MagickFalse);
            }
          SetImageInfoFile(image_info,(FILE *) NULL);
          (void) CopyMagickString(image->filename,filename,MaxTextExtent);
          status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
          if (status == MagickFalse)
            {
              image=DestroyImage(image);
              return(MagickFalse);
            }
          (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
          image_info->temporary=MagickTrue;
        }
      (void) ResetMagickMemory(magick,0,sizeof(magick));
      count=ReadBlob(image,2*MaxTextExtent,magick);
      (void) CloseBlob(image);
      image=DestroyImage(image);
      /*
        Check magic.xml configuration file.
      */
      sans_exception=AcquireExceptionInfo();
      magic_info=GetMagicInfo(magick,(size_t) count,sans_exception);
      if ((magic_info != (const MagicInfo *) NULL) &&
          (GetMagicName(magic_info) != (char *) NULL))
        {
          (void) CopyMagickString(image_info->magick,GetMagicName(magic_info),
            MaxTextExtent);
          magick_info=GetMagickInfo(image_info->magick,sans_exception);
          if ((magick_info == (const MagickInfo *) NULL) ||
              (GetMagickEndianSupport(magick_info) == MagickFalse))
            image_info->endian=UndefinedEndian;
          sans_exception=DestroyExceptionInfo(sans_exception);
          return(MagickTrue);
        }
      magick_info=GetMagickInfo(image_info->magick,sans_exception);
      if ((magick_info == (const MagickInfo *) NULL) ||
          (GetMagickEndianSupport(magick_info) == MagickFalse))
        image_info->endian=UndefinedEndian;
      sans_exception=DestroyExceptionInfo(sans_exception);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e I n f o B l o b                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfoBlob() sets the image info blob member.
%
%  The format of the SetImageInfoBlob method is:
%
%      void SetImageInfoBlob(ImageInfo *image_info,const void *blob,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o blob: the blob.
%
%    o length: the blob length.
%
*/
MagickExport void SetImageInfoBlob(ImageInfo *image_info,const void *blob,
  const size_t length)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image_info->blob=(void *) blob;
  image_info->length=length;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e I n f o F i l e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfoFile() sets the image info file member.
%
%  The format of the SetImageInfoFile method is:
%
%      void SetImageInfoFile(ImageInfo *image_info,FILE *file)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o file: the file.
%
*/
MagickExport void SetImageInfoFile(ImageInfo *image_info,FILE *file)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image_info->file=file;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e M a s k                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageMask() associates a mask with the image.  The mask must be the same
%  dimensions as the image.
%
%  The format of the SetImageMask method is:
%
%      MagickBooleanType SetImageMask(Image *image,const Image *mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o mask: the image mask.
%
*/
MagickExport MagickBooleanType SetImageMask(Image *image,const Image *mask)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (mask != (const Image *) NULL)
    if ((mask->columns != image->columns) || (mask->rows != image->rows))
      ThrowBinaryException(ImageError,"ImageSizeDiffers",image->filename);
  if (image->mask != (Image *) NULL)
    image->mask=DestroyImage(image->mask);
  image->mask=NewImageList();
  if (mask == (Image *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  image->mask=CloneImage(mask,0,0,MagickTrue,&image->exception);
  if (image->mask == (Image *) NULL)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e t I m a g e O p a c i t y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageOpacity() sets the opacity levels of the image.
%
%  The format of the SetImageOpacity method is:
%
%      MagickBooleanType SetImageOpacity(Image *image,const Quantum opacity)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o opacity: the level of transparency: 0 is fully opaque and QuantumRange is
%      fully transparent.
%
*/
MagickExport MagickBooleanType SetImageOpacity(Image *image,
  const Quantum opacity)
{
  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  image->matte=MagickTrue;
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelOpacity(q,opacity);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e V i r t u a l P i x e l M e t h o d                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageVirtualPixelMethod() sets the "virtual pixels" method for the
%  image and returns the previous setting.  A virtual pixel is any pixel access
%  that is outside the boundaries of the image cache.
%
%  The format of the SetImageVirtualPixelMethod() method is:
%
%      VirtualPixelMethod SetImageVirtualPixelMethod(const Image *image,
%        const VirtualPixelMethod virtual_pixel_method)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: choose the type of virtual pixel.
%
*/
MagickExport VirtualPixelMethod SetImageVirtualPixelMethod(const Image *image,
  const VirtualPixelMethod virtual_pixel_method)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(SetPixelCacheVirtualMethod(image,virtual_pixel_method));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S m u s h I m a g e s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SmushImages() takes all images from the current image pointer to the end
%  of the image list and smushes them to each other top-to-bottom if the
%  stack parameter is true, otherwise left-to-right.
%
%  The current gravity setting now effects how the image is justified in the
%  final image.
%
%  The format of the SmushImages method is:
%
%      Image *SmushImages(const Image *images,const MagickBooleanType stack,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o stack: A value other than 0 stacks the images top-to-bottom.
%
%    o offset: minimum distance in pixels between images.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static ssize_t SmushXGap(const Image *smush_image,const Image *images,
  const ssize_t offset,ExceptionInfo *exception)
{
  CacheView
    *left_view,
    *right_view;

  const Image
    *left_image,
    *right_image;

  RectangleInfo
    left_geometry,
    right_geometry;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    y;

  size_t
    gap;

  ssize_t
    x;

  if (images->previous == (Image *) NULL)
    return(0);
  right_image=images;
  SetGeometry(smush_image,&right_geometry);
  GravityAdjustGeometry(right_image->columns,right_image->rows,
    right_image->gravity,&right_geometry);
  left_image=images->previous;
  SetGeometry(smush_image,&left_geometry);
  GravityAdjustGeometry(left_image->columns,left_image->rows,
    left_image->gravity,&left_geometry);
  gap=right_image->columns;
  left_view=AcquireVirtualCacheView(left_image,exception);
  right_view=AcquireVirtualCacheView(right_image,exception);
  for (y=0; y < (ssize_t) smush_image->rows; y++)
  {
    for (x=(ssize_t) left_image->columns-1; x > 0; x--)
    {
      p=GetCacheViewVirtualPixels(left_view,x,left_geometry.y+y,1,1,exception);
      if ((p == (const PixelPacket *) NULL) ||
          (GetPixelOpacity(p) != TransparentOpacity) ||
          ((left_image->columns-x-1) >= gap))
        break;
    }
    i=(ssize_t) left_image->columns-x-1;
    for (x=0; x < (ssize_t) right_image->columns; x++)
    {
      p=GetCacheViewVirtualPixels(right_view,x,right_geometry.y+y,1,1,
        exception);
      if ((p == (const PixelPacket *) NULL) ||
          (GetPixelOpacity(p) != TransparentOpacity) ||
          ((x+i) >= (ssize_t) gap))
        break;
    }
    if ((x+i) < (ssize_t) gap)
      gap=(size_t) (x+i);
  }
  right_view=DestroyCacheView(right_view);
  left_view=DestroyCacheView(left_view);
  if (y < (ssize_t) smush_image->rows)
    return(offset);
  return((ssize_t) gap-offset);
}

static ssize_t SmushYGap(const Image *smush_image,const Image *images,
  const ssize_t offset,ExceptionInfo *exception)
{
  CacheView
    *bottom_view,
    *top_view;

  const Image
    *bottom_image,
    *top_image;

  RectangleInfo
    bottom_geometry,
    top_geometry;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  size_t
    gap;

  ssize_t
    y;

  if (images->previous == (Image *) NULL)
    return(0);
  bottom_image=images;
  SetGeometry(smush_image,&bottom_geometry);
  GravityAdjustGeometry(bottom_image->columns,bottom_image->rows,
    bottom_image->gravity,&bottom_geometry);
  top_image=images->previous;
  SetGeometry(smush_image,&top_geometry);
  GravityAdjustGeometry(top_image->columns,top_image->rows,top_image->gravity,
    &top_geometry);
  gap=bottom_image->rows;
  top_view=AcquireVirtualCacheView(top_image,exception);
  bottom_view=AcquireVirtualCacheView(bottom_image,exception);
  for (x=0; x < (ssize_t) smush_image->columns; x++)
  {
    for (y=(ssize_t) top_image->rows-1; y > 0; y--)
    {
      p=GetCacheViewVirtualPixels(top_view,top_geometry.x+x,y,1,1,exception);
      if ((p == (const PixelPacket *) NULL) ||
          (GetPixelOpacity(p) != TransparentOpacity) || ((top_image->rows-y-1) >= gap))
        break;
    }
    i=(ssize_t) top_image->rows-y-1;
    for (y=0; y < (ssize_t) bottom_image->rows; y++)
    {
      p=GetCacheViewVirtualPixels(bottom_view,bottom_geometry.x+x,y,1,1,
        exception);
      if ((p == (const PixelPacket *) NULL) ||
          (GetPixelOpacity(p) != TransparentOpacity) || ((y+i) >= (ssize_t) gap))
        break;
    }
    if ((y+i) < (ssize_t) gap)
      gap=(size_t) (y+i);
  }
  bottom_view=DestroyCacheView(bottom_view);
  top_view=DestroyCacheView(top_view);
  if (x < (ssize_t) smush_image->columns)
    return(offset);
  return((ssize_t) gap-offset);
}

MagickExport Image *SmushImages(const Image *images,
  const MagickBooleanType stack,const ssize_t offset,ExceptionInfo *exception)
{
#define SmushImageTag  "Smush/Image"

  CacheView
    *smush_view;

  const Image
    *image;

  Image
    *smush_image;

  MagickBooleanType
    matte,
    proceed,
    status;

  MagickOffsetType
    n;

  RectangleInfo
    geometry;

  register const Image
    *next;

  size_t
    height,
    number_images,
    width;

  ssize_t
    x_offset,
    y_offset;

  /*
    Compute maximum area of smushed area.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=images;
  matte=image->matte;
  number_images=1;
  width=image->columns;
  height=image->rows;
  next=GetNextImageInList(image);
  for ( ; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->matte != MagickFalse)
      matte=MagickTrue;
    number_images++;
    if (stack != MagickFalse)
      {
        if (next->columns > width)
          width=next->columns;
        height+=next->rows;
        if (next->previous != (Image *) NULL)
          height+=offset;
        continue;
      }
    width+=next->columns;
    if (next->previous != (Image *) NULL)
      width+=offset;
    if (next->rows > height)
      height=next->rows;
  }
  /*
    Smush images.
  */
  smush_image=CloneImage(image,width,height,MagickTrue,exception);
  if (smush_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(smush_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&smush_image->exception);
      smush_image=DestroyImage(smush_image);
      return((Image *) NULL);
    }
  smush_image->matte=matte;
  (void) SetImageBackgroundColor(smush_image);
  status=MagickTrue;
  x_offset=0;
  y_offset=0;
  smush_view=AcquireVirtualCacheView(smush_image,exception);
  for (n=0; n < (MagickOffsetType) number_images; n++)
  {
    SetGeometry(smush_image,&geometry);
    GravityAdjustGeometry(image->columns,image->rows,image->gravity,&geometry);
    if (stack != MagickFalse)
      {
        x_offset-=geometry.x;
        y_offset-=SmushYGap(smush_image,image,offset,exception);
      }
    else
      {
        x_offset-=SmushXGap(smush_image,image,offset,exception);
        y_offset-=geometry.y;
      }
    status=CompositeImage(smush_image,OverCompositeOp,image,x_offset,y_offset);
    proceed=SetImageProgress(image,SmushImageTag,n,number_images);
    if (proceed == MagickFalse)
      break;
    if (stack == MagickFalse)
      {
        x_offset+=(ssize_t) image->columns;
        y_offset=0;
      }
    else
      {
        x_offset=0;
        y_offset+=(ssize_t) image->rows;
      }
    image=GetNextImageInList(image);
  }
  if (stack == MagickFalse)
    smush_image->columns=(size_t) x_offset;
  else
    smush_image->rows=(size_t) y_offset;
  smush_view=DestroyCacheView(smush_view);
  if (status == MagickFalse)
    smush_image=DestroyImage(smush_image);
  return(smush_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i p I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StripImage() strips an image of all profiles and comments.
%
%  The format of the StripImage method is:
%
%      MagickBooleanType StripImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType StripImage(Image *image)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  DestroyImageProfiles(image);
  (void) DeleteImageProperty(image,"comment");
  (void) DeleteImageProperty(image,"date:create");
  (void) DeleteImageProperty(image,"date:modify");
  (void) SetImageArtifact(image,"png:include-chunk","none,trns,gama");
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncImage() initializes the red, green, and blue intensities of each pixel
%  as defined by the colormap index.
%
%  The format of the SyncImage method is:
%
%      MagickBooleanType SyncImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/

static inline IndexPacket PushColormapIndex(Image *image,
  const size_t index,MagickBooleanType *range_exception)
{
  if (index < image->colors)
    return((IndexPacket) index);
  *range_exception=MagickTrue;
  return((IndexPacket) 0);
}

MagickExport MagickBooleanType SyncImage(Image *image)
{
  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    range_exception,
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->storage_class == DirectClass)
    return(MagickFalse);
  range_exception=MagickFalse;
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(range_exception,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    IndexPacket
      index;

    register IndexPacket
      *restrict indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      index=PushColormapIndex(image,(size_t) GetPixelIndex(indexes+x),
        &range_exception);
      if (image->matte == MagickFalse)
        SetPixelRgb(q,image->colormap+(ssize_t) index)
      else
        SetPixelRGBO(q,image->colormap+(ssize_t) index);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if ((image->ping == MagickFalse) && (range_exception != MagickFalse))
    (void) ThrowMagickException(&image->exception,GetMagickModule(),
      CorruptImageError,"InvalidColormapIndex","`%s'",image->filename);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c I m a g e S e t t i n g s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncImageSettings() syncs image_info options into per-image attributes.
%
%  The format of the SyncImageSettings method is:
%
%      MagickBooleanType SyncImageSettings(const ImageInfo *image_info,
%        Image *image)
%      MagickBooleanType SyncImagesSettings(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
*/

MagickExport MagickBooleanType SyncImagesSettings(ImageInfo *image_info,
  Image *images)
{
  Image
    *image;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  image=images;
  for ( ; image != (Image *) NULL; image=GetNextImageInList(image))
    (void) SyncImageSettings(image_info,image);
  (void) DeleteImageOption(image_info,"page");
  return(MagickTrue);
}

MagickExport MagickBooleanType SyncImageSettings(const ImageInfo *image_info,
  Image *image)
{
  char
    property[MaxTextExtent];

  const char
    *option,
    *value;

  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  ResolutionType
    units;

  /*
    Sync image options.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  option=GetImageOption(image_info,"background");
  if (option != (const char *) NULL)
    (void) QueryColorDatabase(option,&image->background_color,
      &image->exception);
  option=GetImageOption(image_info,"bias");
  if (option != (const char *) NULL)
    image->bias=StringToDoubleInterval(option,(double) QuantumRange+1.0);
  option=GetImageOption(image_info,"black-point-compensation");
  if (option != (const char *) NULL)
    image->black_point_compensation=(MagickBooleanType) ParseCommandOption(
      MagickBooleanOptions,MagickFalse,option);
  option=GetImageOption(image_info,"blue-primary");
  if (option != (const char *) NULL)
    {
      flags=ParseGeometry(option,&geometry_info);
      image->chromaticity.blue_primary.x=geometry_info.rho;
      image->chromaticity.blue_primary.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->chromaticity.blue_primary.y=image->chromaticity.blue_primary.x;
    }
  option=GetImageOption(image_info,"bordercolor");
  if (option != (const char *) NULL)
    (void) QueryColorDatabase(option,&image->border_color,&image->exception);
  option=GetImageOption(image_info,"colors");
  if (option != (const char *) NULL)
    image->colors=StringToUnsignedLong(option);
  option=GetImageOption(image_info,"compose");
  if (option != (const char *) NULL)
    image->compose=(CompositeOperator) ParseCommandOption(MagickComposeOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"compress");
  if (option != (const char *) NULL)
    image->compression=(CompressionType) ParseCommandOption(
      MagickCompressOptions,MagickFalse,option);
  option=GetImageOption(image_info,"debug");
  if (option != (const char *) NULL)
    image->debug=(MagickBooleanType) ParseCommandOption(MagickBooleanOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"density");
  if (option != (const char *) NULL)
    {
      GeometryInfo
        geometry_info;

      /*
        Set image density.
      */
      flags=ParseGeometry(option,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
    }
  option=GetImageOption(image_info,"depth");
  if (option != (const char *) NULL)
    image->depth=StringToUnsignedLong(option);
  option=GetImageOption(image_info,"endian");
  if (option != (const char *) NULL)
    image->endian=(EndianType) ParseCommandOption(MagickEndianOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"filter");
  if (option != (const char *) NULL)
    image->filter=(FilterTypes) ParseCommandOption(MagickFilterOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"fuzz");
  if (option != (const char *) NULL)
    image->fuzz=StringToDoubleInterval(option,(double) QuantumRange+1.0);
  option=GetImageOption(image_info,"gravity");
  if (option != (const char *) NULL)
    image->gravity=(GravityType) ParseCommandOption(MagickGravityOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"green-primary");
  if (option != (const char *) NULL)
    {
      flags=ParseGeometry(option,&geometry_info);
      image->chromaticity.green_primary.x=geometry_info.rho;
      image->chromaticity.green_primary.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->chromaticity.green_primary.y=image->chromaticity.green_primary.x;
    }
  option=GetImageOption(image_info,"intent");
  if (option != (const char *) NULL)
    image->rendering_intent=(RenderingIntent) ParseCommandOption(
      MagickIntentOptions,MagickFalse,option);
  option=GetImageOption(image_info,"interlace");
  if (option != (const char *) NULL)
    image->interlace=(InterlaceType) ParseCommandOption(MagickInterlaceOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"interpolate");
  if (option != (const char *) NULL)
    image->interpolate=(InterpolatePixelMethod) ParseCommandOption(
      MagickInterpolateOptions,MagickFalse,option);
  option=GetImageOption(image_info,"loop");
  if (option != (const char *) NULL)
    image->iterations=StringToUnsignedLong(option);
  option=GetImageOption(image_info,"mattecolor");
  if (option != (const char *) NULL)
    (void) QueryColorDatabase(option,&image->matte_color,&image->exception);
  option=GetImageOption(image_info,"orient");
  if (option != (const char *) NULL)
    image->orientation=(OrientationType) ParseCommandOption(
      MagickOrientationOptions,MagickFalse,option);
  option=GetImageOption(image_info,"page");
  if (option != (const char *) NULL)
    {
      char
        *geometry;

      geometry=GetPageGeometry(option);
      flags=ParseAbsoluteGeometry(geometry,&image->page);
      geometry=DestroyString(geometry);
    }
  option=GetImageOption(image_info,"quality");
  if (option != (const char *) NULL)
    image->quality=StringToUnsignedLong(option);
  option=GetImageOption(image_info,"red-primary");
  if (option != (const char *) NULL)
    {
      flags=ParseGeometry(option,&geometry_info);
      image->chromaticity.red_primary.x=geometry_info.rho;
      image->chromaticity.red_primary.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->chromaticity.red_primary.y=image->chromaticity.red_primary.x;
    }
  if (image_info->quality != UndefinedCompressionQuality)
    image->quality=image_info->quality;
  option=GetImageOption(image_info,"scene");
  if (option != (const char *) NULL)
    image->scene=StringToUnsignedLong(option);
  option=GetImageOption(image_info,"taint");
  if (option != (const char *) NULL)
    image->taint=(MagickBooleanType) ParseCommandOption(MagickBooleanOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"tile-offset");
  if (option != (const char *) NULL)
    {
      char
        *geometry;

      geometry=GetPageGeometry(option);
      flags=ParseAbsoluteGeometry(geometry,&image->tile_offset);
      geometry=DestroyString(geometry);
    }
  option=GetImageOption(image_info,"transparent-color");
  if (option != (const char *) NULL)
    (void) QueryColorDatabase(option,&image->transparent_color,
      &image->exception);
  option=GetImageOption(image_info,"type");
  if (option != (const char *) NULL)
    image->type=(ImageType) ParseCommandOption(MagickTypeOptions,MagickFalse,
      option);
  option=GetImageOption(image_info,"units");
  if (option != (const char *) NULL)
    units=(ResolutionType) ParseCommandOption(MagickResolutionOptions,
      MagickFalse,option);
  else
    units = image_info->units;
  if (units != UndefinedResolution)
    {
      if (image->units != units)
        switch (image->units)
        {
          case PixelsPerInchResolution:
          {
            if (units == PixelsPerCentimeterResolution)
              {
                image->x_resolution/=2.54;
                image->y_resolution/=2.54;
              }
            break;
          }
          case PixelsPerCentimeterResolution:
          {
            if (units == PixelsPerInchResolution)
              {
                image->x_resolution=(double) ((size_t) (100.0*2.54*
                  image->x_resolution+0.5))/100.0;
                image->y_resolution=(double) ((size_t) (100.0*2.54*
                  image->y_resolution+0.5))/100.0;
              }
            break;
          }
          default:
            break;
        }
      image->units=units;
    }
  option=GetImageOption(image_info,"white-point");
  if (option != (const char *) NULL)
    {
      flags=ParseGeometry(option,&geometry_info);
      image->chromaticity.white_point.x=geometry_info.rho;
      image->chromaticity.white_point.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->chromaticity.white_point.y=image->chromaticity.white_point.x;
    }
  ResetImageOptionIterator(image_info);
  for (option=GetNextImageOption(image_info); option != (const char *) NULL; )
  {
    value=GetImageOption(image_info,option);
    if (value != (const char *) NULL)
      {
        (void) FormatLocaleString(property,MaxTextExtent,"%s",option);
        (void) SetImageArtifact(image,property,value);
      }
    option=GetNextImageOption(image_info);
  }
  return(MagickTrue);
}
