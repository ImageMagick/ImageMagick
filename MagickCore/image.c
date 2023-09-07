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
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
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
#include "MagickCore/studio.h"
#include "MagickCore/animate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/compress.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/display.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magic.h"
#include "MagickCore/magick.h"
#include "MagickCore/magick-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/random_.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/timer.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/token.h"
#include "MagickCore/token-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#include "MagickCore/xwindow-private.h"

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
%      Image *AcquireImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AcquireImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  image=(Image *) AcquireCriticalMemory(sizeof(*image));
  (void) memset(image,0,sizeof(*image));
  /*
    Initialize Image structure.
  */
  (void) CopyMagickString(image->magick,"MIFF",MagickPathExtent);
  image->storage_class=DirectClass;
  image->depth=MAGICKCORE_QUANTUM_DEPTH;
  image->colorspace=sRGBColorspace;
  image->rendering_intent=PerceptualIntent;
  image->gamma=1.000/2.200;
  image->chromaticity.red_primary.x=0.6400;
  image->chromaticity.red_primary.y=0.3300;
  image->chromaticity.red_primary.z=0.0300;
  image->chromaticity.green_primary.x=0.3000;
  image->chromaticity.green_primary.y=0.6000;
  image->chromaticity.green_primary.z=0.1000;
  image->chromaticity.blue_primary.x=0.1500;
  image->chromaticity.blue_primary.y=0.0600;
  image->chromaticity.blue_primary.z=0.7900;
  image->chromaticity.white_point.x=0.3127;
  image->chromaticity.white_point.y=0.3290;
  image->chromaticity.white_point.z=0.3583;
  image->interlace=NoInterlace;
  image->ticks_per_second=UndefinedTicksPerSecond;
  image->compose=OverCompositeOp;
  GetPixelInfoRGBA(BackgroundColorRGBA,&image->background_color);
  GetPixelInfoRGBA(BorderColorRGBA,&image->border_color);
  GetPixelInfoRGBA(MatteColorRGBA,&image->matte_color);
  GetPixelInfoRGBA(TransparentColorRGBA,&image->transparent_color);
  GetTimerInfo(&image->timer);
  image->cache=AcquirePixelCache(0);
  image->channel_mask=AllChannels;
  image->channel_map=AcquirePixelChannelMap();
  image->blob=CloneBlobInfo((BlobInfo *) NULL);
  image->timestamp=GetMagickTime();
  image->debug=(GetLogEventMask() & (ImageEvent | TransformEvent | CoderEvent))
    != 0 ? MagickTrue : MagickFalse;
  image->reference_count=1;
  image->semaphore=AcquireSemaphoreInfo();
  image->signature=MagickCoreSignature;
  if (image_info == (ImageInfo *) NULL)
    return(image);
  /*
    Transfer image info.
  */
  SetBlobExempt(image,image_info->file != (FILE *) NULL ? MagickTrue :
    MagickFalse);
  (void) CopyMagickString(image->filename,image_info->filename,
    MagickPathExtent);
  (void) CopyMagickString(image->magick_filename,image_info->filename,
    MagickPathExtent);
  (void) CopyMagickString(image->magick,image_info->magick,MagickPathExtent);
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

      (void) memset(&geometry,0,sizeof(geometry));
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
      if ((flags & RhoValue) != 0)
        image->resolution.x=geometry_info.rho;
      image->resolution.y=image->resolution.x;
      if ((flags & SigmaValue) != 0)
        image->resolution.y=geometry_info.sigma;
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
  image->matte_color=image_info->matte_color;
  image->background_color=image_info->background_color;
  image->border_color=image_info->border_color;
  image->transparent_color=image_info->transparent_color;
  image->ping=image_info->ping;
  image->progress_monitor=image_info->progress_monitor;
  image->client_data=image_info->client_data;
  if (image_info->cache != (void *) NULL)
    ClonePixelCacheMethods(image->cache,image_info->cache);
  /*
    Set all global options that map to per-image settings.
  */
  (void) SyncImageSettings(image_info,image,exception);
  /*
    Global options that are only set for new images.
  */
  option=GetImageOption(image_info,"delay");
  if (option != (const char *) NULL)
    {
      GeometryInfo
        geometry_info;

      flags=ParseGeometry(option,&geometry_info);
      if ((flags & GreaterValue) != 0)
        {
          if ((double) image->delay > floor(geometry_info.rho+0.5))
            image->delay=(size_t) CastDoubleToLong(floor(
              geometry_info.rho+0.5));
        }
      else
        if ((flags & LessValue) != 0)
          {
            if ((double) image->delay < floor(geometry_info.rho+0.5))
              image->ticks_per_second=CastDoubleToLong(floor(
                geometry_info.sigma+0.5));
          }
        else
          image->delay=(size_t) CastDoubleToLong(floor(geometry_info.rho+0.5));
      if ((flags & SigmaValue) != 0)
        image->ticks_per_second=CastDoubleToLong(floor(
          geometry_info.sigma+0.5));
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

  image_info=(ImageInfo *) AcquireCriticalMemory(sizeof(*image_info));
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
%      void AcquireNextImage(const ImageInfo *image_info,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void AcquireNextImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  /*
    Allocate image structure.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  image->next=AcquireImage(image_info,exception);
  if (GetNextImageInList(image) == (Image *) NULL)
    return;
  (void) CopyMagickString(GetNextImageInList(image)->filename,image->filename,
    MagickPathExtent);
  if (image_info != (ImageInfo *) NULL)
    (void) CopyMagickString(GetNextImageInList(image)->filename,
      image_info->filename,MagickPathExtent);
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
%  The current gravity setting effects how the image is justified in the
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

  ImageType
    image_type;

  MagickBooleanType
    homogeneous_colorspace,
    status;

  MagickOffsetType
    n;

  PixelTrait
    alpha_trait;

  RectangleInfo
    geometry;

  const Image
    *next;

  size_t
    depth,
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
  assert(images->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  alpha_trait=images->alpha_trait;
  number_images=1;
  width=images->columns;
  height=images->rows;
  depth=images->depth;
  image_type=images->type;
  homogeneous_colorspace=MagickTrue;
  next=GetNextImageInList(images);
  for ( ; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->depth > depth)
      depth=next->depth;
    if (next->type != images->type)
      image_type=UndefinedType;
    if (next->colorspace != images->colorspace)
      homogeneous_colorspace=MagickFalse;
    if (next->alpha_trait != UndefinedPixelTrait)
      alpha_trait=BlendPixelTrait;
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
  if (image_type != BilevelType)
    {
      if (SetImageStorageClass(append_image,DirectClass,exception) == MagickFalse)
        {
          append_image=DestroyImage(append_image);
          return((Image *) NULL);
        }
      if (homogeneous_colorspace == MagickFalse)
        (void) SetImageColorspace(append_image,sRGBColorspace,exception);
    }
  append_image->depth=depth;
  append_image->alpha_trait=alpha_trait;
  append_image->page=images->page;
  (void) SetImageBackgroundColor(append_image,exception);
  status=MagickTrue;
  x_offset=0;
  y_offset=0;
  next=images;
  append_view=AcquireAuthenticCacheView(append_image,exception);
  for (n=0; n < (MagickOffsetType) number_images; n++)
  {
    CacheView
      *image_view;

    MagickBooleanType
      proceed;

    SetGeometry(append_image,&geometry);
    GravityAdjustGeometry(next->columns,next->rows,next->gravity,&geometry);
    if (stack != MagickFalse)
      x_offset-=geometry.x;
    else
      y_offset-=geometry.y;
    image_view=AcquireVirtualCacheView(next,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp parallel for schedule(static) shared(status) \
      magick_number_threads(next,next,next->rows,1)
#endif
    for (y=0; y < (ssize_t) next->rows; y++)
    {
      MagickBooleanType
        sync;

      PixelInfo
        pixel;

      const Quantum
        *magick_restrict p;

      Quantum
        *magick_restrict q;

      ssize_t
        x;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
      q=QueueCacheViewAuthenticPixels(append_view,x_offset,y+y_offset,
        next->columns,1,exception);
      if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
        {
          status=MagickFalse;
          continue;
        }
      GetPixelInfo(next,&pixel);
      for (x=0; x < (ssize_t) next->columns; x++)
      {
        GetPixelInfoPixel(next,p,&pixel);
        SetPixelViaPixelInfo(append_image,&pixel,q);
        p+=GetPixelChannels(next);
        q+=GetPixelChannels(append_image);
      }
      sync=SyncCacheViewAuthenticPixels(append_view,exception);
      if (sync == MagickFalse)
        status=MagickFalse;
    }
    image_view=DestroyCacheView(image_view);
    if (stack == MagickFalse)
      {
        x_offset+=(ssize_t) next->columns;
        y_offset=0;
      }
    else
      {
        x_offset=0;
        y_offset+=(ssize_t) next->rows;
      }
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
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  exception=AcquireExceptionInfo();
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
%        const MagickBooleanType inside,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType ClipImage(Image *image,ExceptionInfo *exception)
{
  return(ClipImagePath(image,"#1",MagickTrue,exception));
}

MagickExport MagickBooleanType ClipImagePath(Image *image,const char *pathname,
  const MagickBooleanType inside,ExceptionInfo *exception)
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
  assert(image->signature == MagickCoreSignature);
  assert(pathname != NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  property=AcquireString(pathname);
  (void) FormatLocaleString(property,MagickPathExtent,"8BIM:1999,2998:%s",
    pathname);
  value=GetImageProperty(image,property,exception);
  property=DestroyString(property);
  if (value == (const char *) NULL)
    {
      ThrowFileException(exception,OptionError,"NoClipPathDefined",
        image->filename);
      return(MagickFalse);
    }
  image_info=AcquireImageInfo();
  (void) CopyMagickString(image_info->filename,image->filename,
     MagickPathExtent);
  (void) ConcatenateMagickString(image_info->filename,pathname,
    MagickPathExtent);
  clip_mask=BlobToImage(image_info,value,strlen(value),exception);
  image_info=DestroyImageInfo(image_info);
  if (clip_mask == (Image *) NULL)
    return(MagickFalse);
  if (clip_mask->storage_class == PseudoClass)
    {
      (void) SyncImage(clip_mask,exception);
      if (SetImageStorageClass(clip_mask,DirectClass,exception) == MagickFalse)
        return(MagickFalse);
    }
  if (inside != MagickFalse)
    (void) NegateImage(clip_mask,MagickFalse,exception);
  (void) FormatLocaleString(clip_mask->magick_filename,MagickPathExtent,
    "8BIM:1999,2998:%s\nPS",pathname);
  (void) SetImageMask(image,WritePixelMask,clip_mask,exception);
  image->mask_trait=UpdatePixelTrait;
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
    scale_x,
    scale_y;

  Image
    *clone_image;

  size_t
    length;

  /*
    Clone the image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns == 0) || (image->rows == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        "NegativeOrZeroImageSize","`%s'",image->filename);
      return((Image *) NULL);
    }
  clone_image=(Image *) AcquireCriticalMemory(sizeof(*clone_image));
  (void) memset(clone_image,0,sizeof(*clone_image));
  clone_image->signature=MagickCoreSignature;
  clone_image->storage_class=image->storage_class;
  clone_image->number_channels=image->number_channels;
  clone_image->number_meta_channels=image->number_meta_channels;
  clone_image->metacontent_extent=image->metacontent_extent;
  clone_image->colorspace=image->colorspace;
  clone_image->alpha_trait=image->alpha_trait;
  clone_image->channels=image->channels;
  clone_image->mask_trait=image->mask_trait;
  clone_image->columns=image->columns;
  clone_image->rows=image->rows;
  clone_image->dither=image->dither;
  clone_image->image_info=CloneImageInfo(image->image_info);
  (void) CloneImageProfiles(clone_image,image);
  (void) CloneImageProperties(clone_image,image);
  (void) CloneImageArtifacts(clone_image,image);
  GetTimerInfo(&clone_image->timer);
  if (image->ascii85 != (void *) NULL)
    Ascii85Initialize(clone_image);
  clone_image->extent=image->extent;
  clone_image->magick_columns=image->magick_columns;
  clone_image->magick_rows=image->magick_rows;
  clone_image->type=image->type;
  clone_image->channel_mask=image->channel_mask;
  clone_image->channel_map=ClonePixelChannelMap(image->channel_map);
  (void) CopyMagickString(clone_image->magick_filename,image->magick_filename,
    MagickPathExtent);
  (void) CopyMagickString(clone_image->magick,image->magick,MagickPathExtent);
  (void) CopyMagickString(clone_image->filename,image->filename,
    MagickPathExtent);
  clone_image->progress_monitor=image->progress_monitor;
  clone_image->client_data=image->client_data;
  clone_image->reference_count=1;
  clone_image->next=image->next;
  clone_image->previous=image->previous;
  clone_image->list=NewImageList();
  if (detach == MagickFalse)
    clone_image->blob=ReferenceBlob(image->blob);
  else
    {
      clone_image->next=NewImageList();
      clone_image->previous=NewImageList();
      clone_image->blob=CloneBlobInfo((BlobInfo *) NULL);
    }
  clone_image->ping=image->ping;
  clone_image->debug=image->debug;
  clone_image->semaphore=AcquireSemaphoreInfo();
  if (image->colormap != (PixelInfo *) NULL)
    {
      /*
        Allocate and copy the image colormap.
      */
      clone_image->colors=image->colors;
      length=(size_t) image->colors;
      clone_image->colormap=(PixelInfo *) AcquireQuantumMemory(length+1,
        sizeof(*clone_image->colormap));
      if (clone_image->colormap == (PixelInfo *) NULL)
        {
          clone_image=DestroyImage(clone_image);
          ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
        }
      (void) memcpy(clone_image->colormap,image->colormap,length*
        sizeof(*clone_image->colormap));
    }
  if ((columns == 0) || (rows == 0))
    {
      if (image->montage != (char *) NULL)
        (void) CloneString(&clone_image->montage,image->montage);
      if (image->directory != (char *) NULL)
        (void) CloneString(&clone_image->directory,image->directory);
      clone_image->cache=ReferencePixelCache(image->cache);
      return(clone_image);
    }
  scale_x=1.0;
  scale_y=1.0;
  if (image->columns != 0)
    scale_x=(double) columns/(double) image->columns;
  if (image->rows != 0)
    scale_y=(double) rows/(double) image->rows;
  clone_image->page.width=(size_t) CastDoubleToLong(floor(scale_x*
    image->page.width+0.5));
  clone_image->page.height=(size_t) CastDoubleToLong(floor(scale_y*
    image->page.height+0.5));
  if (MagickAbsoluteValue(scale_x-scale_y) < 2.0)
    scale_x=scale_y=MagickMin(scale_x,scale_y);
  clone_image->page.x=CastDoubleToLong(ceil(scale_x*image->page.x-0.5));
  clone_image->tile_offset.x=CastDoubleToLong(ceil(scale_x*
    image->tile_offset.x-0.5));
  clone_image->page.y=CastDoubleToLong(ceil(scale_y*image->page.y-0.5));
  clone_image->tile_offset.y=CastDoubleToLong(ceil(scale_y*
    image->tile_offset.y-0.5));
  clone_image->cache=ClonePixelCache(image->cache);
  if (SetImageExtent(clone_image,columns,rows,exception) == MagickFalse)
    clone_image=DestroyImage(clone_image);
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
  if (image_info->size != (char *) NULL)
    (void) CloneString(&clone_info->size,image_info->size);
  if (image_info->extract != (char *) NULL)
    (void) CloneString(&clone_info->extract,image_info->extract);
  if (image_info->scenes != (char *) NULL)
    (void) CloneString(&clone_info->scenes,image_info->scenes);
  if (image_info->page != (char *) NULL)
    (void) CloneString(&clone_info->page,image_info->page);
  clone_info->interlace=image_info->interlace;
  clone_info->endian=image_info->endian;
  clone_info->units=image_info->units;
  clone_info->quality=image_info->quality;
  if (image_info->sampling_factor != (char *) NULL)
    (void) CloneString(&clone_info->sampling_factor,
      image_info->sampling_factor);
  if (image_info->server_name != (char *) NULL)
    (void) CloneString(&clone_info->server_name,image_info->server_name);
  if (image_info->font != (char *) NULL)
    (void) CloneString(&clone_info->font,image_info->font);
  if (image_info->texture != (char *) NULL)
    (void) CloneString(&clone_info->texture,image_info->texture);
  if (image_info->density != (char *) NULL)
    (void) CloneString(&clone_info->density,image_info->density);
  clone_info->pointsize=image_info->pointsize;
  clone_info->fuzz=image_info->fuzz;
  clone_info->matte_color=image_info->matte_color;
  clone_info->background_color=image_info->background_color;
  clone_info->border_color=image_info->border_color;
  clone_info->transparent_color=image_info->transparent_color;
  clone_info->dither=image_info->dither;
  clone_info->monochrome=image_info->monochrome;
  clone_info->colorspace=image_info->colorspace;
  clone_info->type=image_info->type;
  clone_info->orientation=image_info->orientation;
  clone_info->ping=image_info->ping;
  clone_info->verbose=image_info->verbose;
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
  clone_info->custom_stream=image_info->custom_stream;
  (void) CopyMagickString(clone_info->magick,image_info->magick,
    MagickPathExtent);
  (void) CopyMagickString(clone_info->unique,image_info->unique,
    MagickPathExtent);
  (void) CopyMagickString(clone_info->filename,image_info->filename,
    MagickPathExtent);
  clone_info->channel=image_info->channel;
  (void) CloneImageOptions(clone_info,image_info);
  clone_info->debug=image_info->debug;
  clone_info->signature=image_info->signature;
  return(clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o p y I m a g e P i x e l s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CopyImagePixels() copies pixels from the source image as defined by the
%  geometry the destination image at the specified offset.
%
%  The format of the CopyImagePixels method is:
%
%      MagickBooleanType CopyImagePixels(Image *image,const Image *source_image,
%        const RectangleInfo *geometry,const OffsetInfo *offset,
%        ExceptionInfo *exception);
%
%  A description of each parameter follows:
%
%    o image: the destination image.
%
%    o source_image: the source image.
%
%    o geometry: define the dimensions of the source pixel rectangle.
%
%    o offset: define the offset in the destination image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType CopyImagePixels(Image *image,
  const Image *source_image,const RectangleInfo *geometry,
  const OffsetInfo *offset,ExceptionInfo *exception)
{
#define CopyImageTag  "Copy/Image"

  CacheView
    *image_view,
    *source_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(source_image != (Image *) NULL);
  assert(geometry != (RectangleInfo *) NULL);
  assert(offset != (OffsetInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if ((offset->x < 0) || (offset->y < 0) ||
      ((offset->x+(ssize_t) geometry->width) > (ssize_t) image->columns) ||
      ((offset->y+(ssize_t) geometry->height) > (ssize_t) image->rows))
    ThrowBinaryException(OptionError,"GeometryDoesNotContainImage",
      image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  /*
    Copy image pixels.
  */
  status=MagickTrue;
  progress=0;
  source_view=AcquireVirtualCacheView(source_image,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,source_image,geometry->height,1)
#endif
  for (y=0; y < (ssize_t) geometry->height; y++)
  {
    MagickBooleanType
      sync;

    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(source_view,geometry->x,y+geometry->y,
      geometry->width,1,exception);
    q=QueueCacheViewAuthenticPixels(image_view,offset->x,y+offset->y,
      geometry->width,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) geometry->width; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait source_traits=GetPixelChannelTraits(source_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            ((traits & UpdatePixelTrait) == 0) ||
            (source_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(image,channel,p[i],q);
      }
      p+=GetPixelChannels(source_image);
      q+=GetPixelChannels(image);
    }
    sync=SyncCacheViewAuthenticPixels(image_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,CopyImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  source_view=DestroyCacheView(source_view);
  image_view=DestroyCacheView(image_view);
  return(status);
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
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
  image->channel_map=DestroyPixelChannelMap(image->channel_map);
  if (image->montage != (char *) NULL)
    image->montage=DestroyString(image->montage);
  if (image->directory != (char *) NULL)
    image->directory=DestroyString(image->directory);
  if (image->colormap != (PixelInfo *) NULL)
    image->colormap=(PixelInfo *) RelinquishMagickMemory(image->colormap);
  if (image->geometry != (char *) NULL)
    image->geometry=DestroyString(image->geometry);
  DestroyImageProfiles(image);
  DestroyImageProperties(image);
  DestroyImageArtifacts(image);
  if (image->ascii85 != (Ascii85Info *) NULL)
    image->ascii85=(Ascii85Info *) RelinquishMagickMemory(image->ascii85);
  if (image->image_info != (ImageInfo *) NULL)
    image->image_info=DestroyImageInfo(image->image_info);
  DestroyBlob(image);
  if (image->semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&image->semaphore);
  image->signature=(~MagickCoreSignature);
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
  assert(image_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
  if (image_info->cache != (void *) NULL)
    image_info->cache=DestroyPixelCache(image_info->cache);
  if (image_info->profile != (StringInfo *) NULL)
    image_info->profile=(void *) DestroyStringInfo((StringInfo *)
      image_info->profile);
  DestroyImageOptions(image_info);
  image_info->signature=(~MagickCoreSignature);
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
%  DisassociateImageStream() disassociates the image stream.  It checks if the
%  blob of the specified image is referenced by other images. If the reference
%  count is higher then 1 a new blob is assigned to the specified image.
%
%  The format of the DisassociateImageStream method is:
%
%      void DisassociateImageStream(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DisassociateImageStream(Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  DisassociateBlob(image);
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
  char
    *synchronize;

  /*
    File and image dimension members.
  */
  assert(image_info != (ImageInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  (void) memset(image_info,0,sizeof(*image_info));
  image_info->adjoin=MagickTrue;
  image_info->interlace=NoInterlace;
  image_info->channel=AllChannels;
  image_info->quality=UndefinedCompressionQuality;
  image_info->antialias=MagickTrue;
  image_info->dither=MagickTrue;
  image_info->depth=0;
  synchronize=GetEnvironmentValue("MAGICK_SYNCHRONIZE");
  if (synchronize != (const char *) NULL)
    {
      image_info->synchronize=IsStringTrue(synchronize);
      synchronize=DestroyString(synchronize);
    }
  GetPixelInfoRGBA(BackgroundColorRGBA,&image_info->background_color);
  GetPixelInfoRGBA(BorderColorRGBA,&image_info->border_color);
  GetPixelInfoRGBA(MatteColorRGBA,&image_info->matte_color);
  GetPixelInfoRGBA(TransparentColorRGBA,&image_info->transparent_color);
  image_info->debug=(GetLogEventMask() & ImageEvent) != 0 ? MagickTrue :
    MagickFalse;
  image_info->signature=MagickCoreSignature;
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
%      Image *GetImageMask(const Image *image,const PixelMask type,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: the mask type, ReadPixelMask or WritePixelMask.
%
*/
MagickExport Image *GetImageMask(const Image *image,const PixelMask type,
  ExceptionInfo *exception)
{
  CacheView
    *mask_view,
    *image_view;

  Image
    *mask_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Get image mask.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  switch (type)
  {
    case ReadPixelMask:
    {
      if ((image->channels & ReadMaskChannel) == 0)
        return((Image *) NULL);
      break;
    }
    case WritePixelMask:
    {
      if ((image->channels & WriteMaskChannel) == 0)
        return((Image *) NULL);
      break;
    }
    default:
    {
      if ((image->channels & CompositeMaskChannel) == 0)
        return((Image *) NULL);
      break;
    }
  }
  mask_image=AcquireImage((ImageInfo *) NULL,exception);
  status=SetImageExtent(mask_image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImage(mask_image));
  status=MagickTrue;
  mask_image->alpha_trait=UndefinedPixelTrait;
  (void) SetImageColorspace(mask_image,GRAYColorspace,exception);
  image_view=AcquireVirtualCacheView(image,exception);
  mask_view=AcquireAuthenticCacheView(mask_image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(mask_view,0,y,mask_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      switch (type)
      {
        case ReadPixelMask:
        {
          SetPixelGray(mask_image,GetPixelReadMask(image,p),q);
          break;
        }
        case WritePixelMask:
        {
          SetPixelGray(mask_image,GetPixelWriteMask(image,p),q);
          break;
        }
        default:
        {
          SetPixelGray(mask_image,GetPixelCompositeMask(image,p),q);
          break;
        }
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(mask_image);
    }
    if (SyncCacheViewAuthenticPixels(mask_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  mask_view=DestroyCacheView(mask_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    mask_image=DestroyImage(mask_image);
  return(mask_image);
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
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
%        const char *format,int value,char *filename,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport size_t InterpretImageFilename(const ImageInfo *image_info,
  Image *image,const char *format,int value,char *filename,
  ExceptionInfo *exception)
{
  char
    *q;

  const char
    *p;

  int
    c;

  MagickBooleanType
    canonical;

  ssize_t
    field_width,
    offset;

  canonical=MagickFalse;
  offset=0;
  (void) CopyMagickString(filename,format,MagickPathExtent);
  if (IsStringTrue(GetImageOption(image_info,"filename:literal")) != MagickFalse)
    return(strlen(filename));
  for (p=strchr(format,'%'); p != (char *) NULL; p=strchr(p+1,'%'))
  {
    q=(char *) p+1;
    if (*q == '%')
      {
        p=q+1;
        continue;
      }
    field_width=0;
    if (*q == '0')
      field_width=(ssize_t) strtol(q,&q,10);
    switch (*q)
    {
      case 'd':
      case 'o':
      case 'x':
      {
        q++;
        c=(*q);
        *q='\0';
        (void) FormatLocaleString(filename+(p-format-offset),(size_t)
          (MagickPathExtent-(p-format-offset)),p,value);
        offset+=(4-field_width);
        *q=c;
        (void) ConcatenateMagickString(filename,q,MagickPathExtent);
        canonical=MagickTrue;
        if (*(q-1) != '%')
          break;
        p++;
        break;
      }
      case '[':
      {
        char
          pattern[MagickPathExtent];

        const char
          *option;

        char
          *r;

        ssize_t
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
        for (i=0; (i < (MagickPathExtent-1L)) && (*r != '\0'); i++)
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
        option=(const char *) NULL;
        if (image != (Image *) NULL)
          option=GetImageProperty(image,pattern,exception);
        if ((option == (const char *) NULL) && (image != (Image *) NULL))
          option=GetImageArtifact(image,pattern);
        if ((option == (const char *) NULL) &&
            (image_info != (ImageInfo *) NULL))
          option=GetImageOption(image_info,pattern);
        if (option == (const char *) NULL)
          break;
        q--;
        c=(*q);
        *q='\0';
        (void) CopyMagickString(filename+(p-format-offset),option,(size_t)
          (MagickPathExtent-(p-format-offset)));
        offset+=(ssize_t) strlen(pattern)-(ssize_t) strlen(option)+3;
        *q=c;
        (void) ConcatenateMagickString(filename,r+1,MagickPathExtent);
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
  if (canonical == MagickFalse)
    (void) CopyMagickString(filename,format,MagickPathExtent);
  else
    for (q=filename; *q != '\0'; q++)
      if ((*q == '%') && (*(q+1) == '%'))
        (void) CopyMagickString(q,q+1,(size_t) (MagickPathExtent-(q-filename)));
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

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *p;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          pixel;

        PixelTrait
          traits;

        traits=GetPixelChannelTraits(image,(PixelChannel) i);
        if (traits == UndefinedPixelTrait)
          continue;
        pixel=(double) p[i];
        if ((pixel < 0.0) || (pixel > (double) QuantumRange) ||
            (pixel != (double) ((QuantumAny) pixel)))
          break;
      }
      p+=GetPixelChannels(image);
      if (i < (ssize_t) GetPixelChannels(image))
        status=MagickFalse;
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
  const Image
    *p;

  assert(image != (Image *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
    if (p->signature != MagickCoreSignature)
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
    magick[MagickPathExtent],
    filename[MagickPathExtent];

  const Image
    *p;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  (void) CopyMagickString(magick,image->magick,MagickPathExtent);
  (void) CopyMagickString(filename,image->filename,MagickPathExtent);
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
  assert((*image)->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
%      Image *NewMagickImage(const ImageInfo *image_info,const size_t width,
%        const size_t height,const PixelInfo *background,
%        ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *NewMagickImage(const ImageInfo *image_info,
  const size_t width,const size_t height,const PixelInfo *background,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  Image
    *image;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(background != (const PixelInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  image=AcquireImage(image_info,exception);
  image->columns=width;
  image->rows=height;
  image->colorspace=background->colorspace;
  image->alpha_trait=background->alpha_trait;
  image->fuzz=background->fuzz;
  image->depth=background->depth;
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelViaPixelInfo(image,background,q);
      q+=GetPixelChannels(image);
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
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
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
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
            image->page.width=(size_t) ((ssize_t) image->columns+geometry.x);
        }
      if ((flags & YValue) != 0)
        {
          image->page.y=geometry.y;
          if ((image->page.height == 0) && (geometry.y > 0))
            image->page.height=(size_t) ((ssize_t) image->rows+geometry.y);
        }
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e P i x e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImagePixels() reset the image pixels, that is, all the pixel components
%  are zeroed.
%
%  The format of the SetImage method is:
%
%      MagickBooleanType ResetImagePixels(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ResetImagePixels(Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  size_t
    length;

  ssize_t
    y;

  void
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  pixels=AcquirePixelCachePixels(image,&length,exception);
  if (pixels != (void *) NULL)
    {
      /*
        Reset in-core image pixels.
      */
      (void) memset(pixels,0,length);
      return(MagickTrue);
    }
  /*
    Reset image pixels.
  */
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      (void) memset(q,0,GetPixelChannels(image)*sizeof(Quantum));
      q+=GetPixelChannels(image);
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
%     S e t I m a g e A l p h a                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageAlpha() sets the alpha levels of the image.
%
%  The format of the SetImageAlpha method is:
%
%      MagickBooleanType SetImageAlpha(Image *image,const Quantum alpha,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o alpha: the level of transparency: 0 is fully transparent and QuantumRange
%      is fully opaque.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageAlpha(Image *image,const Quantum alpha,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  image->alpha_trait=BlendPixelTrait;
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelWriteMask(image,q) > (QuantumRange/2))
        SetPixelAlpha(image,alpha,q);
      q+=GetPixelChannels(image);
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
%      MagickBooleanType SetImageBackgroundColor(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageBackgroundColor(Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  PixelInfo
    background;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if ((image->background_color.alpha_trait != UndefinedPixelTrait) &&
      ((image->alpha_trait & BlendPixelTrait) == 0))
    (void) SetImageAlphaChannel(image,OnAlphaChannel,exception);
  ConformPixelInfo(image,&image->background_color,&background,exception);
  /*
    Set image background color.
  */
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelViaPixelInfo(image,&background,q);
      q+=GetPixelChannels(image);
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
%   S e t I m a g e C h a n n e l M a s k                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageChannelMask() sets the image channel mask from the specified channel
%  mask.
%
%  The format of the SetImageChannelMask method is:
%
%      ChannelType SetImageChannelMask(Image *image,
%        const ChannelType channel_mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel_mask: the channel mask.
%
*/
MagickExport ChannelType SetImageChannelMask(Image *image,
  const ChannelType channel_mask)
{
  return(SetPixelChannelMask(image,channel_mask));
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
%      MagickBooleanType SetImageColor(Image *image,const PixelInfo *color,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o background: the image color.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageColor(Image *image,
  const PixelInfo *color,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(color != (const PixelInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  image->colorspace=color->colorspace;
  image->alpha_trait=color->alpha_trait;
  image->fuzz=color->fuzz;
  image->depth=color->depth;
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelViaPixelInfo(image,color,q);
      q+=GetPixelChannels(image);
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
%        const ClassType storage_class,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o storage_class:  The image class.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageStorageClass(Image *image,
  const ClassType storage_class,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  image->storage_class=storage_class;
  return(SyncImagePixelCache(image,exception));
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
%      MagickBooleanType SetImageExtent(Image *image,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns:  The image width in pixels.
%
%    o rows:  The image height in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageExtent(Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
{
  if ((columns == 0) || (rows == 0))
    ThrowBinaryException(ImageError,"NegativeOrZeroImageSize",image->filename);
  image->columns=columns;
  image->rows=rows;
  if (image->depth == 0)
    {
      image->depth=8;
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ImageDepthNotSupported","`%s'",image->filename);
    }
  if (image->depth > (8*sizeof(MagickSizeType)))
    {
      image->depth=8*sizeof(MagickSizeType);
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ImageDepthNotSupported","`%s'",image->filename);
    }
  return(SyncImagePixelCache(image,exception));
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
%  SetImageInfo() initializes the 'magick' field of the ImageInfo structure.
%  It is set to a type of image format based on the prefix or suffix of the
%  filename.  For example, 'ps:image' returns PS indicating a Postscript image.
%  JPEG is returned for this filename: 'image.jpg'.  The filename prefix has
%  precedence over the suffix.  Use an optional index enclosed in brackets
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

static const MagickInfo *SetImageInfoFromExtension(ImageInfo *image_info,
  const char *component,char *magic,ExceptionInfo *exception)
{
  const MagickInfo
    *magick_info;

  MagickFormatType
    format_type;

  ssize_t
    i;

  static const char
    *format_type_formats[] =
    {
      "AUTOTRACE",
      "BROWSE",
      "DCRAW",
      "EDIT",
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
  (void) CopyMagickString(magic,component,MagickPathExtent);
  LocaleUpper(magic);
  /*
    Look for explicit image formats.
  */
  format_type=UndefinedFormatType;
  magick_info=GetMagickInfo(magic,exception);
  if ((magick_info != (const MagickInfo *) NULL) &&
      (magick_info->format_type != UndefinedFormatType))
    format_type=magick_info->format_type;
  i=0;
  while ((format_type == UndefinedFormatType) &&
         (format_type_formats[i] != (char *) NULL))
  {
    if ((*magic == *format_type_formats[i]) &&
        (LocaleCompare(magic,format_type_formats[i]) == 0))
      format_type=ExplicitFormatType;
    i++;
  }
  if (format_type == UndefinedFormatType)
    (void) CopyMagickString(image_info->magick,magic,MagickPathExtent);
  else
    if (format_type == ExplicitFormatType)
      {
        image_info->affirm=MagickTrue;
        (void) CopyMagickString(image_info->magick,magic,MagickPathExtent);
      }
  if (LocaleCompare(magic,"RGB") == 0)
    image_info->affirm=MagickFalse;  /* maybe SGI disguised as RGB */
  return(magick_info);
}

MagickExport MagickBooleanType SetImageInfo(ImageInfo *image_info,
  const unsigned int frames,ExceptionInfo *exception)
{
  char
    component[MagickPathExtent],
    magic[MagickPathExtent],
    path[MagickPathExtent],
    *q;

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

  const char
    *p;

  ssize_t
    count;

  /*
    Look for 'image.format' in filename.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  *component='\0';
  GetPathComponent(image_info->filename,SubimagePath,component);
  if (*component != '\0')
    {
      /*
        Look for scene specification (e.g. img0001.pcd[4]).
      */
      if (IsSceneGeometry(component,MagickFalse) == MagickFalse)
        {
          if (IsGeometry(component) != MagickFalse)
            (void) CloneString(&image_info->extract,component);
        }
      else
        {
          size_t
            first,
            last;

          (void) CloneString(&image_info->scenes,component);
          image_info->scene=StringToUnsignedLong(image_info->scenes);
          image_info->number_scenes=image_info->scene;
          p=image_info->scenes;
          for (q=(char *) image_info->scenes; *q != '\0'; p++)
          {
            while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
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
        }
    }
  *component='\0';
  if (*image_info->magick == '\0')
    GetPathComponent(image_info->filename,ExtensionPath,component);
  if (*component != '\0')
    {
      /*
        Base path sans any compression extension.
      */
      GetPathComponent(image_info->filename,BasePathSansCompressExtension,path);
      GetPathComponent(path,ExtensionPath,component);
    }
  image_info->affirm=MagickFalse;
  sans_exception=AcquireExceptionInfo();
  if ((*component != '\0') && (IsGlob(component) == MagickFalse))
    magick_info=SetImageInfoFromExtension(image_info,component,magic,
      sans_exception);
  /*
    Look for explicit 'format:image' in filename.
  */
  *magic='\0';
  GetPathComponent(image_info->filename,MagickPath,magic);
  if (*magic == '\0')
    {
      (void) CopyMagickString(magic,image_info->magick,MagickPathExtent);
      magick_info=GetMagickInfo(magic,sans_exception);
      if ((magick_info != (const MagickInfo *) NULL) &&
          (magick_info->format_type == ExplicitFormatType))
        image_info->affirm=MagickTrue;
      if (frames == 0)
        GetPathComponent(image_info->filename,CanonicalPath,component);
      else
        GetPathComponent(image_info->filename,SubcanonicalPath,component);
      (void) CopyMagickString(image_info->filename,component,MagickPathExtent);
    }
  else
    {
      const DelegateInfo
        *delegate_info;

      /*
        User specified image format.
      */
      LocaleUpper(magic);
      magick_info=GetMagickInfo(magic,sans_exception);
      delegate_info=(const DelegateInfo *) NULL;
      if (magick_info == (const MagickInfo *) NULL)
        {
          delegate_info=GetDelegateInfo(magic,"*",sans_exception);
          if (delegate_info == (const DelegateInfo *) NULL)
            delegate_info=GetDelegateInfo("*",magic,sans_exception);
          if ((delegate_info == (const DelegateInfo *) NULL) &&
              ((*component != '\0') && (IsGlob(component) == MagickFalse)))
            {
              /*
                Retry in case GetMagickInfo loaded a custom module.
              */
              magick_info=SetImageInfoFromExtension(image_info,component,magic,
                sans_exception);
            }
        }
      if (((magick_info != (const MagickInfo *) NULL) ||
           (delegate_info != (const DelegateInfo *) NULL)) &&
          (IsMagickConflict(magic) == MagickFalse))
        {
          image_info->affirm=MagickTrue;
          (void) CopyMagickString(image_info->magick,magic,MagickPathExtent);
          GetPathComponent(image_info->filename,CanonicalPath,component);
          (void) CopyMagickString(image_info->filename,component,
            MagickPathExtent);
        }
    }
  sans_exception=DestroyExceptionInfo(sans_exception);
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetMagickEndianSupport(magick_info) == MagickFalse))
    image_info->endian=UndefinedEndian;
  if ((image_info->adjoin != MagickFalse) && (frames > 1))
    {
      /*
        Test for multiple image support (e.g. image%02d.png).
      */
      (void) InterpretImageFilename(image_info,(Image *) NULL,
        image_info->filename,(int) image_info->scene,component,exception);
      if ((LocaleCompare(component,image_info->filename) != 0) &&
          (strchr(component,'%') == (char *) NULL))
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
      unsigned char
        *magick;

      size_t
        magick_size;

      /*
        Determine the image format from the first few bytes of the file.
      */
      magick_size=GetMagicPatternExtent(exception);
      if (magick_size == 0)
        return(MagickFalse);
      image=AcquireImage(image_info,exception);
      (void) CopyMagickString(image->filename,image_info->filename,
        MagickPathExtent);
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
            Copy image to seekable temporary file.
          */
          *component='\0';
          status=ImageToFile(image,component,exception);
          (void) CloseBlob(image);
          if (status == MagickFalse)
            {
              (void) RelinquishUniqueFileResource(component);
              image=DestroyImage(image);
              return(MagickFalse);
            }
          SetImageInfoFile(image_info,(FILE *) NULL);
          (void) CopyMagickString(image->filename,component,MagickPathExtent);
          status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
          if (status == MagickFalse)
            {
              (void) RelinquishUniqueFileResource(component);
              image=DestroyImage(image);
              return(MagickFalse);
            }
          (void) CopyMagickString(image_info->filename,component,
            MagickPathExtent);
          image_info->temporary=MagickTrue;
        }
      magick=(unsigned char *) AcquireQuantumMemory(1,magick_size);
      if (magick == (unsigned char *) NULL)
        {
          (void) CloseBlob(image);
          image=DestroyImage(image);
          return(MagickFalse);
        }
      (void) memset(magick,0,magick_size);
      count=ReadBlob(image,magick_size,magick);
      (void) SeekBlob(image,-((MagickOffsetType) count),SEEK_CUR);
      (void) CloseBlob(image);
      image=DestroyImage(image);
      /*
        Check magic cache.
      */
      sans_exception=AcquireExceptionInfo();
      magic_info=GetMagicInfo(magick,(size_t) count,sans_exception);
      magick=(unsigned char *) RelinquishMagickMemory(magick);
      if ((magic_info != (const MagicInfo *) NULL) &&
          (GetMagicName(magic_info) != (char *) NULL))
        {
          /*
            Try to use magick_info that was determined earlier by the extension
          */
          if ((magick_info != (const MagickInfo *) NULL) &&
              (GetMagickUseExtension(magick_info) != MagickFalse) &&
              (LocaleCompare(magick_info->magick_module,GetMagicName(
                magic_info)) == 0))
            (void) CopyMagickString(image_info->magick,magick_info->name,
              MagickPathExtent);
          else
            {
              (void) CopyMagickString(image_info->magick,GetMagicName(
                magic_info),MagickPathExtent);
              magick_info=GetMagickInfo(image_info->magick,sans_exception);
            }
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
  assert(image_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
%   S e t I m a g e I n f o C u s t o m S t r e a m                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfoCustomStream() sets the image info custom stream handlers.
%
%  The format of the SetImageInfoCustomStream method is:
%
%      void SetImageInfoCustomStream(ImageInfo *image_info,
%        CustomStreamInfo *custom_stream)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o custom_stream: your custom stream methods.
%
*/
MagickExport void SetImageInfoCustomStream(ImageInfo *image_info,
  CustomStreamInfo *custom_stream)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image_info->custom_stream=(CustomStreamInfo *) custom_stream;
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
  assert(image_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
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
%      MagickBooleanType SetImageMask(Image *image,const PixelMask type,
%        const Image *mask,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: the mask type, ReadPixelMask or WritePixelMask.
%
%    o mask: the image mask.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageMask(Image *image,const PixelMask type,
  const Image *mask,ExceptionInfo *exception)
{
  CacheView
    *mask_view,
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Set image mask.
  */
  assert(image != (Image *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  if (mask == (const Image *) NULL)
    {
      switch (type)
      {
        case ReadPixelMask:
        {
          image->channels=(ChannelType) (image->channels &
            (unsigned int) ~ReadMaskChannel);
          break;
        }
        case WritePixelMask:
        {
          image->channels=(ChannelType) (image->channels &
            (unsigned int) ~WriteMaskChannel);
          magick_fallthrough;
        }
        default:
        {
          image->channels=(ChannelType) (image->channels &
            (unsigned int) ~CompositeMaskChannel);
          break;
        }
      }
      return(SyncImagePixelCache(image,exception));
    }
  switch (type)
  {
    case ReadPixelMask:
    {
      image->channels=(ChannelType) (image->channels | ReadMaskChannel);
      break;
    }
    case WritePixelMask:
    {
      image->channels=(ChannelType) (image->channels | WriteMaskChannel);
      break;
    }
    default:
    {
      image->channels=(ChannelType) (image->channels | CompositeMaskChannel);
      break;
    }
  }
  if (SyncImagePixelCache(image,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  image->mask_trait=UpdatePixelTrait;
  mask_view=AcquireVirtualCacheView(mask,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(mask,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(mask_view,0,y,mask->columns,1,exception);
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        intensity;

      intensity=0.0;
      if ((x < (ssize_t) mask->columns) && (y < (ssize_t) mask->rows))
        intensity=GetPixelIntensity(mask,p);
      switch (type)
      {
        case ReadPixelMask:
        {
          SetPixelReadMask(image,ClampToQuantum(intensity),q);
          break;
        }
        case WritePixelMask:
        {
          SetPixelWriteMask(image,ClampToQuantum(intensity),q);
          break;
        }
        default:
        {
          SetPixelCompositeMask(image,ClampToQuantum(intensity),q);
          break;
        }
      }
      p+=GetPixelChannels(mask);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image->mask_trait=UndefinedPixelTrait;
  mask_view=DestroyCacheView(mask_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e R e g i o n M a s k                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageRegionMask() associates a mask with the image as defined by the
%  specified region.
%
%  The format of the SetImageRegionMask method is:
%
%      MagickBooleanType SetImageRegionMask(Image *image,const PixelMask type,
%        const RectangleInfo *region,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: the mask type, ReadPixelMask or WritePixelMask.
%
%    o geometry: the mask region.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageRegionMask(Image *image,
  const PixelMask type,const RectangleInfo *region,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Set image mask as defined by the region.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (region == (const RectangleInfo *) NULL)
    {
      switch (type)
      {
        case ReadPixelMask:
        {
          image->channels=(ChannelType) (image->channels &
            (unsigned int) ~ReadMaskChannel);
          break;
        }
        case WritePixelMask:
        {
          image->channels=(ChannelType) (image->channels &
            (unsigned int) ~WriteMaskChannel);
          break;
        }
        default:
        {
          image->channels=(ChannelType) (image->channels &
            (unsigned int) ~CompositeMaskChannel);
          break;
        }
      }
      return(SyncImagePixelCache(image,exception));
    }
  switch (type)
  {
    case ReadPixelMask:
    {
      image->channels=(ChannelType) (image->channels | ReadMaskChannel);
      break;
    }
    case WritePixelMask:
    {
      image->channels=(ChannelType) (image->channels | WriteMaskChannel);
      break;
    }
    default:
    {
      image->channels=(ChannelType) (image->channels | CompositeMaskChannel);
      break;
    }
  }
  if (SyncImagePixelCache(image,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  image->mask_trait=UpdatePixelTrait;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      Quantum
        pixel;

      pixel=QuantumRange;
      if (((x >= region->x) && (x < (region->x+(ssize_t) region->width))) &&
          ((y >= region->y) && (y < (region->y+(ssize_t) region->height))))
        pixel=(Quantum) 0;
      switch (type)
      {
        case ReadPixelMask:
        {
          SetPixelReadMask(image,pixel,q);
          break;
        }
        case WritePixelMask:
        {
          SetPixelWriteMask(image,pixel,q);
          break;
        }
        default:
        {
          SetPixelCompositeMask(image,pixel,q);
          break;
        }
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image->mask_trait=UndefinedPixelTrait;
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
%      VirtualPixelMethod SetImageVirtualPixelMethod(Image *image,
%        const VirtualPixelMethod virtual_pixel_method,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: choose the type of virtual pixel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport VirtualPixelMethod SetImageVirtualPixelMethod(Image *image,
  const VirtualPixelMethod virtual_pixel_method,ExceptionInfo *exception)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(SetPixelCacheVirtualMethod(image,virtual_pixel_method,exception));
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

  const Quantum
    *p;

  ssize_t
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
      if ((p == (const Quantum *) NULL) ||
          (GetPixelAlpha(left_image,p) != TransparentAlpha) ||
          (((ssize_t) left_image->columns-x-1) >= (ssize_t) gap))
        break;
    }
    i=(ssize_t) left_image->columns-x-1;
    for (x=0; x < (ssize_t) right_image->columns; x++)
    {
      p=GetCacheViewVirtualPixels(right_view,x,right_geometry.y+y,1,1,
        exception);
      if ((p == (const Quantum *) NULL) ||
          (GetPixelAlpha(right_image,p) != TransparentAlpha) ||
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

  const Quantum
    *p;

  ssize_t
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
      if ((p == (const Quantum *) NULL) ||
          (GetPixelAlpha(top_image,p) != TransparentAlpha) ||
          (((ssize_t) top_image->rows-y-1) >= (ssize_t) gap))
        break;
    }
    i=(ssize_t) top_image->rows-y-1;
    for (y=0; y < (ssize_t) bottom_image->rows; y++)
    {
      p=GetCacheViewVirtualPixels(bottom_view,bottom_geometry.x+x,y,1,1,
        exception);
      if ((p == (const Quantum *) NULL) ||
          (GetPixelAlpha(bottom_image,p) != TransparentAlpha) ||
          ((y+i) >= (ssize_t) gap))
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

  const Image
    *image;

  Image
    *smush_image;

  MagickBooleanType
    proceed,
    status;

  MagickOffsetType
    n;

  PixelTrait
    alpha_trait;

  RectangleInfo
    geometry;

  const Image
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
  assert(images->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  image=images;
  alpha_trait=image->alpha_trait;
  number_images=1;
  width=image->columns;
  height=image->rows;
  next=GetNextImageInList(image);
  for ( ; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->alpha_trait != UndefinedPixelTrait)
      alpha_trait=BlendPixelTrait;
    number_images++;
    if (stack != MagickFalse)
      {
        if (next->columns > width)
          width=next->columns;
        height+=next->rows;
        if (next->previous != (Image *) NULL)
          height=(size_t) MagickMax((ssize_t) height+offset,0U);
        continue;
      }
    width+=next->columns;
    if (next->previous != (Image *) NULL)
      width=(size_t) MagickMax((ssize_t) width+offset,0U);
    if (next->rows > height)
      height=next->rows;
  }
  /*
    Smush images.
  */
  smush_image=CloneImage(image,width,height,MagickTrue,exception);
  if (smush_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(smush_image,DirectClass,exception) == MagickFalse)
    {
      smush_image=DestroyImage(smush_image);
      return((Image *) NULL);
    }
  smush_image->alpha_trait=alpha_trait;
  (void) SetImageBackgroundColor(smush_image,exception);
  status=MagickTrue;
  x_offset=0;
  y_offset=0;
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
    status=CompositeImage(smush_image,image,OverCompositeOp,MagickTrue,x_offset,
      y_offset,exception);
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
%      MagickBooleanType StripImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType StripImage(Image *image,
  ExceptionInfo *magick_unused(exception))
{
  MagickBooleanType
    status;

  magick_unreferenced(exception);
  assert(image != (Image *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  DestroyImageProfiles(image);
  (void) DeleteImageProperty(image,"comment");
  (void) DeleteImageProperty(image,"date:create");
  (void) DeleteImageProperty(image,"date:modify");
  (void) DeleteImageProperty(image,"date:timestamp");
  status=SetImageArtifact(image,"png:exclude-chunk",
    "bKGD,caNv,cHRM,eXIf,gAMA,iCCP,iTXt,pHYs,sRGB,tEXt,zCCP,zTXt,date");
  return(status);
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
%      MagickBooleanType SyncImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline Quantum PushColormapIndex(Image *image,const Quantum index,
  MagickBooleanType *range_exception)
{
  if ((size_t) index < image->colors)
    return(index);
  *range_exception=MagickTrue;
  return((Quantum) 0);
}

MagickExport MagickBooleanType SyncImage(Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    range_exception,
    status,
    taint;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (image->ping != MagickFalse)
    return(MagickTrue);
  if (image->storage_class != PseudoClass)
    return(MagickFalse);
  assert(image->colormap != (PixelInfo *) NULL);
  range_exception=MagickFalse;
  status=MagickTrue;
  taint=image->taint;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(range_exception,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      index;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      index=PushColormapIndex(image,GetPixelIndex(image,q),&range_exception);
      SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  image->taint=taint;
  if ((image->ping == MagickFalse) && (range_exception != MagickFalse))
    (void) ThrowMagickException(exception,GetMagickModule(),
      CorruptImageWarning,"InvalidColormapIndex","`%s'",image->filename);
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
%  SyncImageSettings() syncs any image_info global options into per-image
%  attributes.
%
%  Note: in IMv6 free form 'options' were always mapped into 'artifacts', so
%  that operations and coders can find such settings.  In IMv7 if a desired
%  per-image artifact is not set, then it will directly look for a global
%  option as a fallback, as such this copy is no longer needed, only the
%  link set up.
%
%  The format of the SyncImageSettings method is:
%
%      MagickBooleanType SyncImageSettings(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%      MagickBooleanType SyncImagesSettings(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType SyncImagesSettings(ImageInfo *image_info,
  Image *images,ExceptionInfo *exception)
{
  Image
    *image;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  image=images;
  for ( ; image != (Image *) NULL; image=GetNextImageInList(image))
    (void) SyncImageSettings(image_info,image,exception);
  (void) DeleteImageOption(image_info,"page");
  return(MagickTrue);
}

MagickExport MagickBooleanType SyncImageSettings(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const char
    *option;

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
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  option=GetImageOption(image_info,"background");
  if (option != (const char *) NULL)
    (void) QueryColorCompliance(option,AllCompliance,&image->background_color,
      exception);
  option=GetImageOption(image_info,"black-point-compensation");
  if (option != (const char *) NULL)
    image->black_point_compensation=(MagickBooleanType) ParseCommandOption(
      MagickBooleanOptions,MagickFalse,option);
  option=GetImageOption(image_info,"blue-primary");
  if (option != (const char *) NULL)
    {
      flags=ParseGeometry(option,&geometry_info);
      if ((flags & RhoValue) != 0)
        image->chromaticity.blue_primary.x=geometry_info.rho;
      image->chromaticity.blue_primary.y=image->chromaticity.blue_primary.x;
      if ((flags & SigmaValue) != 0)
        image->chromaticity.blue_primary.y=geometry_info.sigma;
    }
  option=GetImageOption(image_info,"bordercolor");
  if (option != (const char *) NULL)
    (void) QueryColorCompliance(option,AllCompliance,&image->border_color,
      exception);
  /* FUTURE: do not sync compose to per-image compose setting here */
  option=GetImageOption(image_info,"compose");
  if (option != (const char *) NULL)
    image->compose=(CompositeOperator) ParseCommandOption(MagickComposeOptions,
      MagickFalse,option);
  /* -- */
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
      flags=ParseGeometry(option,&geometry_info);
      if ((flags & RhoValue) != 0)
        image->resolution.x=geometry_info.rho;
      image->resolution.y=image->resolution.x;
      if ((flags & SigmaValue) != 0)
        image->resolution.y=geometry_info.sigma;
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
    image->filter=(FilterType) ParseCommandOption(MagickFilterOptions,
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
      if ((flags & RhoValue) != 0)
        image->chromaticity.green_primary.x=geometry_info.rho;
      image->chromaticity.green_primary.y=image->chromaticity.green_primary.x;
      if ((flags & SigmaValue) != 0)
        image->chromaticity.green_primary.y=geometry_info.sigma;
    }
  option=GetImageOption(image_info,"intent");
  if (option != (const char *) NULL)
    image->rendering_intent=(RenderingIntent) ParseCommandOption(
      MagickIntentOptions,MagickFalse,option);
  option=GetImageOption(image_info,"intensity");
  if (option != (const char *) NULL)
    image->intensity=(PixelIntensityMethod) ParseCommandOption(
      MagickPixelIntensityOptions,MagickFalse,option);
  option=GetImageOption(image_info,"interlace");
  if (option != (const char *) NULL)
    image->interlace=(InterlaceType) ParseCommandOption(MagickInterlaceOptions,
      MagickFalse,option);
  option=GetImageOption(image_info,"interpolate");
  if (option != (const char *) NULL)
    image->interpolate=(PixelInterpolateMethod) ParseCommandOption(
      MagickInterpolateOptions,MagickFalse,option);
  option=GetImageOption(image_info,"loop");
  if (option != (const char *) NULL)
    image->iterations=StringToUnsignedLong(option);
  option=GetImageOption(image_info,"mattecolor");
  if (option != (const char *) NULL)
    (void) QueryColorCompliance(option,AllCompliance,&image->matte_color,
      exception);
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
      if ((flags & RhoValue) != 0)
        image->chromaticity.red_primary.x=geometry_info.rho;
      image->chromaticity.red_primary.y=image->chromaticity.red_primary.x;
      if ((flags & SigmaValue) != 0)
        image->chromaticity.red_primary.y=geometry_info.sigma;
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
    (void) QueryColorCompliance(option,AllCompliance,&image->transparent_color,
      exception);
  option=GetImageOption(image_info,"type");
  if (option != (const char *) NULL)
    image->type=(ImageType) ParseCommandOption(MagickTypeOptions,MagickFalse,
      option);
  option=GetImageOption(image_info,"units");
  units=image_info->units;
  if (option != (const char *) NULL)
    units=(ResolutionType) ParseCommandOption(MagickResolutionOptions,
      MagickFalse,option);
  if (units != UndefinedResolution)
    {
      if (image->units != units)
        switch (image->units)
        {
          case PixelsPerInchResolution:
          {
            if (units == PixelsPerCentimeterResolution)
              {
                image->resolution.x/=2.54;
                image->resolution.y/=2.54;
              }
            break;
          }
          case PixelsPerCentimeterResolution:
          {
            if (units == PixelsPerInchResolution)
              {
                image->resolution.x=(double) ((size_t) (100.0*2.54*
                  image->resolution.x+0.5))/100.0;
                image->resolution.y=(double) ((size_t) (100.0*2.54*
                  image->resolution.y+0.5))/100.0;
              }
            break;
          }
          default:
            break;
        }
      image->units=units;
      option=GetImageOption(image_info,"density");
      if (option != (const char *) NULL)
        {
          flags=ParseGeometry(option,&geometry_info);
          if ((flags & RhoValue) != 0)
            image->resolution.x=geometry_info.rho;
          image->resolution.y=image->resolution.x;
          if ((flags & SigmaValue) != 0)
            image->resolution.y=geometry_info.sigma;
        }
    }
  option=GetImageOption(image_info,"virtual-pixel");
  if (option != (const char *) NULL)
    (void) SetImageVirtualPixelMethod(image,(VirtualPixelMethod)
      ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,option),
      exception);
  option=GetImageOption(image_info,"white-point");
  if (option != (const char *) NULL)
    {
      flags=ParseGeometry(option,&geometry_info);
      if ((flags & RhoValue) != 0)
        image->chromaticity.white_point.x=geometry_info.rho;
      image->chromaticity.white_point.y=image->chromaticity.white_point.x;
      if ((flags & SigmaValue) != 0)
        image->chromaticity.white_point.y=geometry_info.sigma;
    }
  /*
    Pointer to allow the lookup of pre-image artifact will fallback to a global
    option setting/define.  This saves a lot of duplication of global options
    into per-image artifacts, while ensuring only specifically set per-image
    artifacts are preserved when parenthesis ends.
  */
  if (image->image_info != (ImageInfo *) NULL)
    image->image_info=DestroyImageInfo(image->image_info);
  image->image_info=CloneImageInfo(image_info);
  return(MagickTrue);
}
