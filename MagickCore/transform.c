/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%       TTTTT  RRRR    AAA   N   N  SSSSS  FFFFF   OOO   RRRR   M   M         %
%         T    R   R  A   A  NN  N  SS     F      O   O  R   R  MM MM         %
%         T    RRRR   AAAAA  N N N   SSS   FFF    O   O  RRRR   M M M         %
%         T    R R    A   A  N  NN     SS  F      O   O  R R    M   M         %
%         T    R  R   A   A  N   N  SSSSS  F       OOO   R  R   M   M         %
%                                                                             %
%                                                                             %
%                    MagickCore Image Transform Methods                       %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/memory_.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/resize.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/transform-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A u t o O r i e n t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AutoOrientImage() adjusts an image so that its orientation is suitable for
%  viewing (i.e. top-left orientation).
%
%  The format of the AutoOrientImage method is:
%
%      Image *AutoOrientImage(const Image *image,
%        const OrientationType orientation,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o orientation: Current image orientation.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport Image *AutoOrientImage(const Image *image,
  const OrientationType orientation,ExceptionInfo *exception)
{
  Image
    *orient_image;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  orient_image=(Image *) NULL;
  switch(orientation)
  {
    case UndefinedOrientation:
    case TopLeftOrientation:
    default:
    {
      orient_image=CloneImage(image,0,0,MagickTrue,exception);
      break;
    }
    case TopRightOrientation:
    {
      orient_image=FlopImage(image,exception);
      break;
    }
    case BottomRightOrientation:
    {
      orient_image=RotateImage(image,180.0,exception);
      break;
    }
    case BottomLeftOrientation:
    {
      orient_image=FlipImage(image,exception);
      break;
    }
    case LeftTopOrientation:
    {
      orient_image=TransposeImage(image,exception);
      break;
    }
    case RightTopOrientation:
    {
      orient_image=RotateImage(image,90.0,exception);
      break;
    }
    case RightBottomOrientation:
    {
      orient_image=TransverseImage(image,exception);
      break;
    }
    case LeftBottomOrientation:
    {
      orient_image=RotateImage(image,270.0,exception);
      break;
    }
  }
  if (orient_image != (Image *) NULL)
    orient_image->orientation=TopLeftOrientation;
  return(orient_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C h o p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ChopImage() removes a region of an image and collapses the image to occupy
%  the removed portion.
%
%  The format of the ChopImage method is:
%
%      Image *ChopImage(const Image *image,const RectangleInfo *chop_info)
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o chop_info: Define the region of the image to chop.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ChopImage(const Image *image,const RectangleInfo *chop_info,
  ExceptionInfo *exception)
{
#define ChopImageTag  "Chop/Image"

  CacheView
    *chop_view,
    *image_view;

  Image
    *chop_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RectangleInfo
    extent;

  ssize_t
    y;

  /*
    Check chop geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  assert(chop_info != (RectangleInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (((chop_info->x+(ssize_t) chop_info->width) < 0) ||
      ((chop_info->y+(ssize_t) chop_info->height) < 0) ||
      (chop_info->x > (ssize_t) image->columns) ||
      (chop_info->y > (ssize_t) image->rows))
    ThrowImageException(OptionWarning,"GeometryDoesNotContainImage");
  extent=(*chop_info);
  if ((extent.x+(ssize_t) extent.width) > (ssize_t) image->columns)
    extent.width=(size_t) ((ssize_t) image->columns-extent.x);
  if ((extent.y+(ssize_t) extent.height) > (ssize_t) image->rows)
    extent.height=(size_t) ((ssize_t) image->rows-extent.y);
  if (extent.x < 0)
    {
      extent.width-=(size_t) (-extent.x);
      extent.x=0;
    }
  if (extent.y < 0)
    {
      extent.height-=(size_t) (-extent.y);
      extent.y=0;
    }
  if ((extent.width >= image->columns) || (extent.height >= image->rows))
    ThrowImageException(OptionWarning,"GeometryDoesNotContainImage");
  chop_image=CloneImage(image,image->columns-extent.width,image->rows-
    extent.height,MagickTrue,exception);
  if (chop_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Extract chop image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  chop_view=AcquireAuthenticCacheView(chop_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,chop_image,(size_t) extent.y,1)
#endif
  for (y=0; y < (ssize_t) extent.y; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(chop_view,0,y,chop_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x < extent.x) || (x >= (extent.x+(ssize_t) extent.width)))
        {
          ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            PixelTrait chop_traits=GetPixelChannelTraits(chop_image,channel);
            if ((traits == UndefinedPixelTrait) ||
                (chop_traits == UndefinedPixelTrait))
              continue;
            SetPixelChannel(chop_image,channel,p[i],q);
          }
          q+=GetPixelChannels(chop_image);
        }
      p+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(chop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ChopImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  /*
    Extract chop image.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,chop_image,image->rows-((size_t) extent.y+extent.height),1)
#endif
  for (y=0; y < (ssize_t) (image->rows-((size_t) extent.y+extent.height)); y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,extent.y+(ssize_t) extent.height+y,
      image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(chop_view,0,extent.y+y,chop_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x < extent.x) || (x >= (extent.x+(ssize_t) extent.width)))
        {
          ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            PixelTrait chop_traits=GetPixelChannelTraits(chop_image,channel);
            if ((traits == UndefinedPixelTrait) ||
                (chop_traits == UndefinedPixelTrait))
              continue;
            SetPixelChannel(chop_image,channel,p[i],q);
          }
          q+=GetPixelChannels(chop_image);
        }
      p+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(chop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ChopImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  chop_view=DestroyCacheView(chop_view);
  image_view=DestroyCacheView(image_view);
  chop_image->type=image->type;
  if (status == MagickFalse)
    chop_image=DestroyImage(chop_image);
  return(chop_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     C o n s o l i d a t e C M Y K I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConsolidateCMYKImage() consolidates separate C, M, Y, and K planes into a
%  single image.
%
%  The format of the ConsolidateCMYKImage method is:
%
%      Image *ConsolidateCMYKImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ConsolidateCMYKImages(const Image *images,
  ExceptionInfo *exception)
{
  CacheView
    *cmyk_view,
    *image_view;

  Image
    *cmyk_image,
    *cmyk_images;

  ssize_t
    j;

  ssize_t
    y;

  /*
    Consolidate separate C, M, Y, and K planes into a single image.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  cmyk_images=NewImageList();
  for (j=0; j < (ssize_t) GetImageListLength(images); j+=4)
  {
    ssize_t
      i;

    assert(images != (Image *) NULL);
    cmyk_image=CloneImage(images,0,0,MagickTrue,
      exception);
    if (cmyk_image == (Image *) NULL)
      break;
    if (SetImageStorageClass(cmyk_image,DirectClass,exception) == MagickFalse)
      break;
    (void) SetImageColorspace(cmyk_image,CMYKColorspace,exception);
    for (i=0; i < 4; i++)
    {
      image_view=AcquireVirtualCacheView(images,exception);
      cmyk_view=AcquireAuthenticCacheView(cmyk_image,exception);
      for (y=0; y < (ssize_t) images->rows; y++)
      {
        const Quantum
          *magick_restrict p;

        ssize_t
          x;

        Quantum
          *magick_restrict q;

        p=GetCacheViewVirtualPixels(image_view,0,y,images->columns,1,exception);
        q=QueueCacheViewAuthenticPixels(cmyk_view,0,y,cmyk_image->columns,1,
          exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          break;
        for (x=0; x < (ssize_t) images->columns; x++)
        {
          Quantum
            pixel;

          pixel=ClampToQuantum((double) QuantumRange-
            GetPixelIntensity(images,p));
          switch (i)
          {
            case 0: SetPixelCyan(cmyk_image,pixel,q);  break;
            case 1: SetPixelMagenta(cmyk_image,pixel,q);  break;
            case 2: SetPixelYellow(cmyk_image,pixel,q);  break;
            case 3: SetPixelBlack(cmyk_image,pixel,q);  break;
            default: break;
          }
          p+=GetPixelChannels(images);
          q+=GetPixelChannels(cmyk_image);
        }
        if (SyncCacheViewAuthenticPixels(cmyk_view,exception) == MagickFalse)
          break;
      }
      cmyk_view=DestroyCacheView(cmyk_view);
      image_view=DestroyCacheView(image_view);
      images=GetNextImageInList(images);
      if (images == (Image *) NULL)
        break;
    }
    AppendImageToList(&cmyk_images,cmyk_image);
  }
  return(cmyk_images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C r o p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CropImage() extracts a region of the image starting at the offset defined
%  by geometry.  Region must be fully defined, and no special handling of
%  geometry flags is performed.
%
%  The format of the CropImage method is:
%
%      Image *CropImage(const Image *image,const RectangleInfo *geometry,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o geometry: Define the region of the image to crop with members
%      x, y, width, and height.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CropImage(const Image *image,const RectangleInfo *geometry,
  ExceptionInfo *exception)
{
#define CropImageTag  "Crop/Image"

  CacheView
    *crop_view,
    *image_view;

  Image
    *crop_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  OffsetInfo
    offset;

  RectangleInfo
    bounding_box,
    page;

  ssize_t
    y;

  /*
    Check crop geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  bounding_box=image->page;
  if ((bounding_box.width == 0) || (bounding_box.height == 0))
    {
      bounding_box.width=image->columns;
      bounding_box.height=image->rows;
    }
  page=(*geometry);
  if (page.width == 0)
    page.width=bounding_box.width;
  if (page.height == 0)
    page.height=bounding_box.height;
  if ((((double) bounding_box.x-page.x) >= (double) page.width) ||
      (((double) bounding_box.y-page.y) >= (double) page.height) ||
      (((double) page.x-bounding_box.x) > (double) image->columns) ||
      (((double) page.y-bounding_box.y) > (double) image->rows))
    {
      /*
        Crop is not within virtual canvas, return 1 pixel transparent image.
      */
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "GeometryDoesNotContainImage","(\"%.20gx%.20g%+.20g%+.20g\") `%s'",
        (double) geometry->width,(double) geometry->height,
        (double) geometry->x,(double) geometry->y,image->filename);
      crop_image=CloneImage(image,1,1,MagickTrue,exception);
      if (crop_image == (Image *) NULL)
        return((Image *) NULL);
      crop_image->background_color.alpha_trait=BlendPixelTrait;
      crop_image->background_color.alpha=(MagickRealType) TransparentAlpha;
      (void) SetImageBackgroundColor(crop_image,exception);
      crop_image->page=bounding_box;
      crop_image->page.x=(-1);
      crop_image->page.y=(-1);
      if (crop_image->dispose == BackgroundDispose)
        crop_image->dispose=NoneDispose;
      return(crop_image);
    }
  if ((page.x < 0) && (bounding_box.x >= 0))
    {
      page.width=(size_t) ((ssize_t) page.width+page.x-bounding_box.x);
      page.x=0;
    }
  else
    {
      page.width=(size_t) ((ssize_t) page.width-(bounding_box.x-page.x));
      page.x-=bounding_box.x;
      if (page.x < 0)
        page.x=0;
    }
  if ((page.y < 0) && (bounding_box.y >= 0))
    {
      page.height=(size_t) ((ssize_t) page.height+page.y-bounding_box.y);
      page.y=0;
    }
  else
    {
      page.height=(size_t) ((ssize_t) page.height-(bounding_box.y-page.y));
      page.y-=bounding_box.y;
      if (page.y < 0)
        page.y=0;
    }
  if ((page.x+(ssize_t) page.width) > (ssize_t) image->columns)
    page.width=(size_t) ((ssize_t) image->columns-page.x);
  if ((geometry->width != 0) && (page.width > geometry->width))
    page.width=geometry->width;
  if ((page.y+(ssize_t) page.height) > (ssize_t) image->rows)
    page.height=(size_t) ((ssize_t) image->rows-page.y);
  if ((geometry->height != 0) && (page.height > geometry->height))
    page.height=geometry->height;
  bounding_box.x+=page.x;
  bounding_box.y+=page.y;
  if ((page.width == 0) || (page.height == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "GeometryDoesNotContainImage","`%s'",image->filename);
      return((Image *) NULL);
    }
  /*
    Initialize crop image attributes.
  */
  crop_image=CloneImage(image,page.width,page.height,MagickTrue,exception);
  if (crop_image == (Image *) NULL)
    return((Image *) NULL);
  crop_image->page.width=image->page.width;
  crop_image->page.height=image->page.height;
  offset.x=bounding_box.x+(ssize_t) bounding_box.width;
  offset.y=bounding_box.y+(ssize_t) bounding_box.height;
  if ((offset.x > (ssize_t) image->page.width) ||
      (offset.y > (ssize_t) image->page.height))
    {
      crop_image->page.width=bounding_box.width;
      crop_image->page.height=bounding_box.height;
    }
  crop_image->page.x=bounding_box.x;
  crop_image->page.y=bounding_box.y;
  /*
    Crop image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  crop_view=AcquireAuthenticCacheView(crop_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,crop_image,crop_image->rows,1)
#endif
  for (y=0; y < (ssize_t) crop_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,page.x,page.y+y,crop_image->columns,
      1,exception);
    q=QueueCacheViewAuthenticPixels(crop_view,0,y,crop_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) crop_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait crop_traits=GetPixelChannelTraits(crop_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (crop_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(crop_image,channel,p[i],q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(crop_image);
    }
    if (SyncCacheViewAuthenticPixels(crop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,CropImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  crop_view=DestroyCacheView(crop_view);
  image_view=DestroyCacheView(image_view);
  crop_image->type=image->type;
  if (status == MagickFalse)
    crop_image=DestroyImage(crop_image);
  return(crop_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C r o p I m a g e T o T i l e s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CropImageToTiles() crops a single image, into a possible list of tiles.
%  This may include a single sub-region of the image.  This basically applies
%  all the normal geometry flags for Crop.
%
%      Image *CropImageToTiles(const Image *image,
%        const RectangleInfo *crop_geometry, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image The transformed image is returned as this parameter.
%
%    o crop_geometry: A crop geometry string.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline ssize_t PixelRoundOffset(double x)
{
  /*
    Round the fraction to nearest integer.
  */
  if ((x-floor(x)) < (ceil(x)-x))
    return(CastDoubleToLong(floor(x)));
  return(CastDoubleToLong(ceil(x)));
}

MagickExport Image *CropImageToTiles(const Image *image,
  const char *crop_geometry,ExceptionInfo *exception)
{
  Image
    *next,
    *crop_image;

  MagickStatusType
    flags;

  RectangleInfo
    geometry;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  flags=ParseGravityGeometry(image,crop_geometry,&geometry,exception);
  if ((flags & AreaValue) != 0)
    {
      PointInfo
        delta,
        offset;

      RectangleInfo
        crop;

      size_t
        height,
        width;

      /*
        Crop into NxM tiles (@ flag).
      */
      crop_image=NewImageList();
      width=image->columns;
      height=image->rows;
      if (geometry.width == 0)
        geometry.width=1;
      if (geometry.height == 0)
        geometry.height=1;
      if ((flags & AspectValue) == 0)
        {
          width=(size_t) ((ssize_t) width-(geometry.x < 0 ? -1 : 1)*geometry.x);
          height=(size_t) ((ssize_t) height-(geometry.y < 0 ? -1 : 1)*
            geometry.y);
        }
      else
        {
          width=(size_t) ((ssize_t) width+(geometry.x < 0 ? -1 : 1)*geometry.x);
          height=(size_t) ((ssize_t) height+(geometry.y < 0 ? -1 : 1)*
            geometry.y);
        }
      delta.x=(double) width/geometry.width;
      delta.y=(double) height/geometry.height;
      if (delta.x < 1.0)
        delta.x=1.0;
      if (delta.y < 1.0)
        delta.y=1.0;
      for (offset.y=0; offset.y < (double) height; )
      {
        if ((flags & AspectValue) == 0)
          {
            crop.y=PixelRoundOffset((double) (offset.y-
              (geometry.y > 0 ? 0 : geometry.y)));
            offset.y+=delta.y;   /* increment now to find width */
            crop.height=(size_t) PixelRoundOffset((double) (offset.y+
              (geometry.y < 0 ? 0 : geometry.y)));
          }
        else
          {
            crop.y=PixelRoundOffset((double) (offset.y-
              (geometry.y > 0 ? geometry.y : 0)));
            offset.y+=delta.y;  /* increment now to find width */
            crop.height=(size_t) PixelRoundOffset((double)
              (offset.y+(geometry.y < -1 ? geometry.y : 0)));
          }
        crop.height=(size_t) ((ssize_t) crop.height-crop.y);
        crop.y+=image->page.y;
        for (offset.x=0; offset.x < (double) width; )
        {
          if ((flags & AspectValue) == 0)
            {
              crop.x=PixelRoundOffset((double) (offset.x-
                (geometry.x > 0 ? 0 : geometry.x)));
              offset.x+=delta.x;  /* increment now to find height */
              crop.width=(size_t) PixelRoundOffset((double) (offset.x+
                (geometry.x < 0 ? 0 : geometry.x)));
            }
          else
            {
              crop.x=PixelRoundOffset((double) (offset.x-
                (geometry.x > 0 ? geometry.x : 0)));
              offset.x+=delta.x;  /* increment now to find height */
              crop.width=(size_t) PixelRoundOffset((double) (offset.x+
                (geometry.x < 0 ? geometry.x : 0)));
            }
          crop.width=(size_t) ((ssize_t) crop.width-crop.x);
          crop.x+=image->page.x;
          next=CropImage(image,&crop,exception);
          if (next != (Image *) NULL)
            AppendImageToList(&crop_image,next);
        }
      }
      ClearMagickException(exception);
      return(crop_image);
    }
  if (((geometry.width == 0) && (geometry.height == 0)) ||
      ((flags & XValue) != 0) || ((flags & YValue) != 0))
    {
      /*
        Crop a single region at +X+Y.
      */
      crop_image=CropImage(image,&geometry,exception);
      if ((crop_image != (Image *) NULL) && ((flags & AspectValue) != 0))
        {
          crop_image->page.width=geometry.width;
          crop_image->page.height=geometry.height;
          crop_image->page.x-=geometry.x;
          crop_image->page.y-=geometry.y;
        }
      return(crop_image);
    }
  if ((image->columns > geometry.width) || (image->rows > geometry.height))
    {
      RectangleInfo
        page;

      size_t
        height,
        width;

      ssize_t
        x,
        y;

      /*
        Crop into tiles of fixed size WxH.
      */
      page=image->page;
      if (page.width == 0)
        page.width=image->columns;
      if (page.height == 0)
        page.height=image->rows;
      width=geometry.width;
      if (width == 0)
        width=page.width;
      height=geometry.height;
      if (height == 0)
        height=page.height;
      next=(Image *) NULL;
      crop_image=NewImageList();
      for (y=0; y < (ssize_t) page.height; y+=(ssize_t) height)
      {
        for (x=0; x < (ssize_t) page.width; x+=(ssize_t) width)
        {
          geometry.width=width;
          geometry.height=height;
          geometry.x=x;
          geometry.y=y;
          next=CropImage(image,&geometry,exception);
          if (next == (Image *) NULL)
            break;
          AppendImageToList(&crop_image,next);
        }
        if (next == (Image *) NULL)
          break;
      }
      return(crop_image);
    }
  return(CloneImage(image,0,0,MagickTrue,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x c e r p t I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExcerptImage() returns a excerpt of the image as defined by the geometry.
%
%  The format of the ExcerptImage method is:
%
%      Image *ExcerptImage(const Image *image,const RectangleInfo *geometry,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o geometry: Define the region of the image to extend with members
%      x, y, width, and height.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ExcerptImage(const Image *image,
  const RectangleInfo *geometry,ExceptionInfo *exception)
{
#define ExcerptImageTag  "Excerpt/Image"

  CacheView
    *excerpt_view,
    *image_view;

  Image
    *excerpt_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Allocate excerpt image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  excerpt_image=CloneImage(image,geometry->width,geometry->height,MagickTrue,
    exception);
  if (excerpt_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Excerpt each row.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  excerpt_view=AcquireAuthenticCacheView(excerpt_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,excerpt_image,excerpt_image->rows,1)
#endif
  for (y=0; y < (ssize_t) excerpt_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,geometry->x,geometry->y+y,
      geometry->width,1,exception);
    q=GetCacheViewAuthenticPixels(excerpt_view,0,y,excerpt_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) excerpt_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait excerpt_traits=GetPixelChannelTraits(excerpt_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (excerpt_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(excerpt_image,channel,p[i],q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(excerpt_image);
    }
    if (SyncCacheViewAuthenticPixels(excerpt_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ExcerptImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  excerpt_view=DestroyCacheView(excerpt_view);
  image_view=DestroyCacheView(image_view);
  excerpt_image->type=image->type;
  if (status == MagickFalse)
    excerpt_image=DestroyImage(excerpt_image);
  return(excerpt_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x t e n t I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExtentImage() extends the image as defined by the geometry, gravity, and
%  image background color.  Set the (x,y) offset of the geometry to move the
%  original image relative to the extended image.
%
%  The format of the ExtentImage method is:
%
%      Image *ExtentImage(const Image *image,const RectangleInfo *geometry,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o geometry: Define the region of the image to extend with members
%      x, y, width, and height.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ExtentImage(const Image *image,
  const RectangleInfo *geometry,ExceptionInfo *exception)
{
  Image
    *extent_image;

  MagickBooleanType
    status;

  /*
    Allocate extent image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  extent_image=CloneImage(image,geometry->width,geometry->height,MagickTrue,
    exception);
  if (extent_image == (Image *) NULL)
    return((Image *) NULL);
  status=SetImageBackgroundColor(extent_image,exception);
  if (status == MagickFalse)
    {
      extent_image=DestroyImage(extent_image);
      return((Image *) NULL);
    }
  DisableCompositeClampUnlessSpecified(extent_image);
  status=CompositeImage(extent_image,image,image->compose,MagickTrue,
    -geometry->x,-geometry->y,exception);
  if (status != MagickFalse)
    Update8BIMClipPath(extent_image,image->columns,image->rows,geometry);
  return(extent_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F l i p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FlipImage() creates a vertical mirror image by reflecting the pixels
%  around the central x-axis.
%
%  The format of the FlipImage method is:
%
%      Image *FlipImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *FlipImage(const Image *image,ExceptionInfo *exception)
{
#define FlipImageTag  "Flip/Image"

  CacheView
    *flip_view,
    *image_view;

  Image
    *flip_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RectangleInfo
    page;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  flip_image=CloneImage(image,0,0,MagickTrue,exception);
  if (flip_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Flip image.
  */
  status=MagickTrue;
  progress=0;
  page=image->page;
  image_view=AcquireVirtualCacheView(image,exception);
  flip_view=AcquireAuthenticCacheView(flip_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,flip_image,flip_image->rows,1)
#endif
  for (y=0; y < (ssize_t) flip_image->rows; y++)
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
    q=QueueCacheViewAuthenticPixels(flip_view,0,((ssize_t) flip_image->rows-y-
      1),flip_image->columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) flip_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait flip_traits=GetPixelChannelTraits(flip_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (flip_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(flip_image,channel,p[i],q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(flip_image);
    }
    if (SyncCacheViewAuthenticPixels(flip_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,FlipImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  flip_view=DestroyCacheView(flip_view);
  image_view=DestroyCacheView(image_view);
  flip_image->type=image->type;
  if (page.height != 0)
    page.y=((ssize_t) page.height-(ssize_t) flip_image->rows-page.y);
  flip_image->page=page;
  if (status == MagickFalse)
    flip_image=DestroyImage(flip_image);
  return(flip_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F l o p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FlopImage() creates a horizontal mirror image by reflecting the pixels
%  around the central y-axis.
%
%  The format of the FlopImage method is:
%
%      Image *FlopImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *FlopImage(const Image *image,ExceptionInfo *exception)
{
#define FlopImageTag  "Flop/Image"

  CacheView
    *flop_view,
    *image_view;

  Image
    *flop_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RectangleInfo
    page;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  flop_image=CloneImage(image,0,0,MagickTrue,exception);
  if (flop_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Flop each row.
  */
  status=MagickTrue;
  progress=0;
  page=image->page;
  image_view=AcquireVirtualCacheView(image,exception);
  flop_view=AcquireAuthenticCacheView(flop_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,flop_image,flop_image->rows,1)
#endif
  for (y=0; y < (ssize_t) flop_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(flop_view,0,y,flop_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    q+=GetPixelChannels(flop_image)*flop_image->columns;
    for (x=0; x < (ssize_t) flop_image->columns; x++)
    {
      ssize_t
        i;

      q-=GetPixelChannels(flop_image);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait flop_traits=GetPixelChannelTraits(flop_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (flop_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(flop_image,channel,p[i],q);
      }
      p+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(flop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,FlopImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  flop_view=DestroyCacheView(flop_view);
  image_view=DestroyCacheView(image_view);
  flop_image->type=image->type;
  if (page.width != 0)
    page.x=((ssize_t) page.width-(ssize_t) flop_image->columns-page.x);
  flop_image->page=page;
  if (status == MagickFalse)
    flop_image=DestroyImage(flop_image);
  return(flop_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R o l l I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RollImage() offsets an image as defined by x_offset and y_offset.
%
%  The format of the RollImage method is:
%
%      Image *RollImage(const Image *image,const ssize_t x_offset,
%        const ssize_t y_offset,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x_offset: the number of columns to roll in the horizontal direction.
%
%    o y_offset: the number of rows to roll in the vertical direction.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType CopyImageRegion(Image *destination,const Image *source,  const size_t columns,const size_t rows,const ssize_t sx,const ssize_t sy,
  const ssize_t dx,const ssize_t dy,ExceptionInfo *exception)
{
  CacheView
    *source_view,
    *destination_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  if (columns == 0)
    return(MagickTrue);
  status=MagickTrue;
  source_view=AcquireVirtualCacheView(source,exception);
  destination_view=AcquireAuthenticCacheView(destination,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(source,destination,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    MagickBooleanType
      sync;

    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    /*
      Transfer scanline.
    */
    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(source_view,sx,sy+y,columns,1,exception);
    q=GetCacheViewAuthenticPixels(destination_view,dx,dy+y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(source); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(source,i);
        PixelTrait source_traits=GetPixelChannelTraits(source,channel);
        PixelTrait destination_traits=GetPixelChannelTraits(destination,
          channel);
        if ((source_traits == UndefinedPixelTrait) ||
            (destination_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(destination,channel,p[i],q);
      }
      p+=GetPixelChannels(source);
      q+=GetPixelChannels(destination);
    }
    sync=SyncCacheViewAuthenticPixels(destination_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
  }
  destination_view=DestroyCacheView(destination_view);
  source_view=DestroyCacheView(source_view);
  return(status);
}

MagickExport Image *RollImage(const Image *image,const ssize_t x_offset,
  const ssize_t y_offset,ExceptionInfo *exception)
{
#define RollImageTag  "Roll/Image"

  Image
    *roll_image;

  MagickStatusType
    status;

  RectangleInfo
    offset;

  /*
    Initialize roll image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  roll_image=CloneImage(image,0,0,MagickTrue,exception);
  if (roll_image == (Image *) NULL)
    return((Image *) NULL);
  offset.x=x_offset;
  offset.y=y_offset;
  while (offset.x < 0)
    offset.x+=(ssize_t) image->columns;
  while (offset.x >= (ssize_t) image->columns)
    offset.x-=(ssize_t) image->columns;
  while (offset.y < 0)
    offset.y+=(ssize_t) image->rows;
  while (offset.y >= (ssize_t) image->rows)
    offset.y-=(ssize_t) image->rows;
  /*
    Roll image.
  */
  status=CopyImageRegion(roll_image,image,(size_t) offset.x,
    (size_t) offset.y,(ssize_t) image->columns-offset.x,(ssize_t) image->rows-
    offset.y,0,0,exception);
  (void) SetImageProgress(image,RollImageTag,0,3);
  status&=(MagickStatusType) CopyImageRegion(roll_image,image,(size_t)
    ((ssize_t) image->columns-offset.x),(size_t) offset.y,0,(ssize_t)
    image->rows-offset.y,offset.x,0,exception);
  (void) SetImageProgress(image,RollImageTag,1,3);
  status&=(MagickStatusType) CopyImageRegion(roll_image,image,(size_t)
    offset.x,(size_t) ((ssize_t) image->rows-offset.y),(ssize_t)
    image->columns-offset.x,0,0,offset.y,exception);
  (void) SetImageProgress(image,RollImageTag,2,3);
  status&=(MagickStatusType) CopyImageRegion(roll_image,image,(size_t)
    ((ssize_t) image->columns-offset.x),(size_t) ((ssize_t) image->rows-
    offset.y),0,0,offset.x,offset.y,exception);
  (void) SetImageProgress(image,RollImageTag,3,3);
  roll_image->type=image->type;
  if (status == MagickFalse)
    roll_image=DestroyImage(roll_image);
  return(roll_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S h a v e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShaveImage() shaves pixels from the image edges.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ShaveImage method is:
%
%      Image *ShaveImage(const Image *image,const RectangleInfo *shave_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o shave_image: Method ShaveImage returns a pointer to the shaved
%      image.  A null image is returned if there is a memory shortage or
%      if the image width or height is zero.
%
%    o image: the image.
%
%    o shave_info: Specifies a pointer to a RectangleInfo which defines the
%      region of the image to crop.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ShaveImage(const Image *image,
  const RectangleInfo *shave_info,ExceptionInfo *exception)
{
  Image
    *shave_image;

  RectangleInfo
    geometry;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (((2*shave_info->width) >= image->columns) ||
      ((2*shave_info->height) >= image->rows))
    ThrowImageException(OptionWarning,"GeometryDoesNotContainImage");
  SetGeometry(image,&geometry);
  geometry.width-=2*shave_info->width;
  geometry.height-=2*shave_info->height;
  geometry.x=(ssize_t) shave_info->width+image->page.x;
  geometry.y=(ssize_t) shave_info->height+image->page.y;
  shave_image=CropImage(image,&geometry,exception);
  if (shave_image == (Image *) NULL)
    return((Image *) NULL);
  shave_image->page.width-=2*shave_info->width;
  shave_image->page.height-=2*shave_info->height;
  shave_image->page.x-=(ssize_t) shave_info->width;
  shave_image->page.y-=(ssize_t) shave_info->height;
  return(shave_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p l i c e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SpliceImage() splices a solid color into the image as defined by the
%  geometry.
%
%  The format of the SpliceImage method is:
%
%      Image *SpliceImage(const Image *image,const RectangleInfo *geometry,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o geometry: Define the region of the image to splice with members
%      x, y, width, and height.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SpliceImage(const Image *image,
  const RectangleInfo *geometry,ExceptionInfo *exception)
{
#define SpliceImageTag  "Splice/Image"

  CacheView
    *image_view,
    *splice_view;

  Image
    *splice_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RectangleInfo
    splice_geometry;

  ssize_t
    columns,
    y;

  /*
    Allocate splice image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  splice_geometry=(*geometry);
  splice_image=CloneImage(image,image->columns+splice_geometry.width,
    image->rows+splice_geometry.height,MagickTrue,exception);
  if (splice_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(splice_image,DirectClass,exception) == MagickFalse)
    {
      splice_image=DestroyImage(splice_image);
      return((Image *) NULL);
    }
  if ((IsPixelInfoGray(&splice_image->background_color) == MagickFalse) &&
      (IsGrayColorspace(splice_image->colorspace) != MagickFalse))
    (void) SetImageColorspace(splice_image,sRGBColorspace,exception);
  if ((splice_image->background_color.alpha_trait != UndefinedPixelTrait) &&
      (splice_image->alpha_trait == UndefinedPixelTrait))
    (void) SetImageAlpha(splice_image,OpaqueAlpha,exception);
  (void) SetImageBackgroundColor(splice_image,exception);
  /*
    Respect image geometry.
  */
  switch (image->gravity)
  {
    default:
    case UndefinedGravity:
    case NorthWestGravity:
      break;
    case NorthGravity:
    {
      splice_geometry.x+=(ssize_t) splice_geometry.width/2;
      break;
    }
    case NorthEastGravity:
    {
      splice_geometry.x+=(ssize_t) splice_geometry.width;
      break;
    }
    case WestGravity:
    {
      splice_geometry.y+=(ssize_t) splice_geometry.width/2;
      break;
    }
    case CenterGravity:
    {
      splice_geometry.x+=(ssize_t) splice_geometry.width/2;
      splice_geometry.y+=(ssize_t) splice_geometry.height/2;
      break;
    }
    case EastGravity:
    {
      splice_geometry.x+=(ssize_t) splice_geometry.width;
      splice_geometry.y+=(ssize_t) splice_geometry.height/2;
      break;
    }
    case SouthWestGravity:
    {
      splice_geometry.y+=(ssize_t) splice_geometry.height;
      break;
    }
    case SouthGravity:
    {
      splice_geometry.x+=(ssize_t) splice_geometry.width/2;
      splice_geometry.y+=(ssize_t) splice_geometry.height;
      break;
    }
    case SouthEastGravity:
    {
      splice_geometry.x+=(ssize_t) splice_geometry.width;
      splice_geometry.y+=(ssize_t) splice_geometry.height;
      break;
    }
  }
  /*
    Splice image.
  */
  status=MagickTrue;
  progress=0;
  columns=MagickMin(splice_geometry.x,(ssize_t) splice_image->columns);
  image_view=AcquireVirtualCacheView(image,exception);
  splice_view=AcquireAuthenticCacheView(splice_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,splice_image,(size_t) splice_geometry.y,1)
#endif
  for (y=0; y < (ssize_t) splice_geometry.y; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,splice_image->columns,1,
      exception);
    q=QueueCacheViewAuthenticPixels(splice_view,0,y,splice_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait splice_traits=GetPixelChannelTraits(splice_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (splice_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(splice_image,channel,p[i],q);
      }
      SetPixelRed(splice_image,GetPixelRed(image,p),q);
      SetPixelGreen(splice_image,GetPixelGreen(image,p),q);
      SetPixelBlue(splice_image,GetPixelBlue(image,p),q);
      SetPixelAlpha(splice_image,GetPixelAlpha(image,p),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(splice_image);
    }
    for ( ; x < (splice_geometry.x+(ssize_t) splice_geometry.width); x++)
      q+=GetPixelChannels(splice_image);
    for ( ; x < (ssize_t) splice_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait splice_traits=GetPixelChannelTraits(splice_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (splice_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(splice_image,channel,p[i],q);
      }
      SetPixelRed(splice_image,GetPixelRed(image,p),q);
      SetPixelGreen(splice_image,GetPixelGreen(image,p),q);
      SetPixelBlue(splice_image,GetPixelBlue(image,p),q);
      SetPixelAlpha(splice_image,GetPixelAlpha(image,p),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(splice_image);
    }
    if (SyncCacheViewAuthenticPixels(splice_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SpliceImageTag,progress,
          splice_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,splice_image,splice_image->rows,2)
#endif
  for (y=splice_geometry.y+(ssize_t) splice_geometry.height; y < (ssize_t) splice_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    if ((y < 0) || (y >= (ssize_t) splice_image->rows))
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y-(ssize_t) splice_geometry.height,
      splice_image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(splice_view,0,y,splice_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait splice_traits=GetPixelChannelTraits(splice_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (splice_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(splice_image,channel,p[i],q);
      }
      SetPixelRed(splice_image,GetPixelRed(image,p),q);
      SetPixelGreen(splice_image,GetPixelGreen(image,p),q);
      SetPixelBlue(splice_image,GetPixelBlue(image,p),q);
      SetPixelAlpha(splice_image,GetPixelAlpha(image,p),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(splice_image);
    }
    for ( ; x < (splice_geometry.x+(ssize_t) splice_geometry.width); x++)
      q+=GetPixelChannels(splice_image);
    for ( ; x < (ssize_t) splice_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait splice_traits=GetPixelChannelTraits(splice_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (splice_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(splice_image,channel,p[i],q);
      }
      SetPixelRed(splice_image,GetPixelRed(image,p),q);
      SetPixelGreen(splice_image,GetPixelGreen(image,p),q);
      SetPixelBlue(splice_image,GetPixelBlue(image,p),q);
      SetPixelAlpha(splice_image,GetPixelAlpha(image,p),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(splice_image);
    }
    if (SyncCacheViewAuthenticPixels(splice_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SpliceImageTag,progress,
          splice_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  splice_view=DestroyCacheView(splice_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    splice_image=DestroyImage(splice_image);
  return(splice_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s f o r m I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformImage() is a convenience method that behaves like ResizeImage() or
%  CropImage() but accepts scaling and/or cropping information as a region
%  geometry specification.  If the operation fails, the original image handle
%  is left as is.
%
%  This should only be used for single images.
%
%  This function destroys what it assumes to be a single image list.
%  If the input image is part of a larger list, all other images in that list
%  will be simply 'lost', not destroyed.
%
%  Also if the crop generates a list of images only the first image is resized.
%  And finally if the crop succeeds and the resize failed, you will get a
%  cropped image, as well as a 'false' or 'failed' report.
%
%  This function and should probably be deprecated in favor of direct calls
%  to CropImageToTiles() or ResizeImage(), as appropriate.
%
%  The format of the TransformImage method is:
%
%      MagickBooleanType TransformImage(Image **image,const char *crop_geometry,
%        const char *image_geometry,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image The transformed image is returned as this parameter.
%
%    o crop_geometry: A crop geometry string.  This geometry defines a
%      subregion of the image to crop.
%
%    o image_geometry: An image geometry string.  This geometry defines the
%      final size of the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate MagickBooleanType TransformImage(Image **image,
  const char *crop_geometry,const char *image_geometry,ExceptionInfo *exception)
{
  Image
    *resize_image,
    *transform_image;

  RectangleInfo
    geometry;

  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  transform_image=(*image);
  if (crop_geometry != (const char *) NULL)
    {
      Image
        *crop_image;

      /*
        Crop image to a user specified size.
      */
      crop_image=CropImageToTiles(*image,crop_geometry,exception);
      if (crop_image == (Image *) NULL)
        transform_image=CloneImage(*image,0,0,MagickTrue,exception);
      else
        {
          transform_image=DestroyImage(transform_image);
          transform_image=GetFirstImageInList(crop_image);
        }
      *image=transform_image;
    }
  if (image_geometry == (const char *) NULL)
    return(MagickTrue);
  /*
    Scale image to a user specified size.
  */
  (void) ParseRegionGeometry(transform_image,image_geometry,&geometry,
    exception);
  if ((transform_image->columns == geometry.width) &&
      (transform_image->rows == geometry.height))
    return(MagickTrue);
  resize_image=ResizeImage(transform_image,geometry.width,geometry.height,
    transform_image->filter,exception);
  if (resize_image == (Image *) NULL)
    return(MagickFalse);
  transform_image=DestroyImage(transform_image);
  transform_image=resize_image;
  *image=transform_image;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s p o s e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransposeImage() creates a horizontal mirror image by reflecting the pixels
%  around the central y-axis while rotating them by 90 degrees.
%
%  The format of the TransposeImage method is:
%
%      Image *TransposeImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *TransposeImage(const Image *image,ExceptionInfo *exception)
{
#define TransposeImageTag  "Transpose/Image"

  CacheView
    *image_view,
    *transpose_view;

  Image
    *transpose_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RectangleInfo
    page;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  transpose_image=CloneImage(image,image->rows,image->columns,MagickTrue,
    exception);
  if (transpose_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Transpose image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  transpose_view=AcquireAuthenticCacheView(transpose_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,transpose_image,image->rows,1)
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
    p=GetCacheViewVirtualPixels(image_view,0,(ssize_t) image->rows-y-1,
      image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(transpose_view,(ssize_t) image->rows-y-1,
      0,1,transpose_image->rows,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
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
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait transpose_traits=GetPixelChannelTraits(transpose_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (transpose_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(transpose_image,channel,p[i],q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(transpose_image);
    }
    if (SyncCacheViewAuthenticPixels(transpose_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,TransposeImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  transpose_view=DestroyCacheView(transpose_view);
  image_view=DestroyCacheView(image_view);
  transpose_image->type=image->type;
  page=transpose_image->page;
  Swap(page.width,page.height);
  Swap(page.x,page.y);
  transpose_image->page=page;
  if (status == MagickFalse)
    transpose_image=DestroyImage(transpose_image);
  return(transpose_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s v e r s e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransverseImage() creates a vertical mirror image by reflecting the pixels
%  around the central x-axis while rotating them by 270 degrees.
%
%  The format of the TransverseImage method is:
%
%      Image *TransverseImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *TransverseImage(const Image *image,ExceptionInfo *exception)
{
#define TransverseImageTag  "Transverse/Image"

  CacheView
    *image_view,
    *transverse_view;

  Image
    *transverse_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  RectangleInfo
    page;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  transverse_image=CloneImage(image,image->rows,image->columns,MagickTrue,
    exception);
  if (transverse_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Transverse image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  transverse_view=AcquireAuthenticCacheView(transverse_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,transverse_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(transverse_view,(ssize_t) image->rows-y-1,
      0,1,transverse_image->rows,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    q+=GetPixelChannels(transverse_image)*image->columns;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      q-=GetPixelChannels(transverse_image);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait transverse_traits=GetPixelChannelTraits(transverse_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (transverse_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(transverse_image,channel,p[i],q);
      }
      p+=GetPixelChannels(image);
    }
    sync=SyncCacheViewAuthenticPixels(transverse_view,exception);
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
        proceed=SetImageProgress(image,TransverseImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  transverse_view=DestroyCacheView(transverse_view);
  image_view=DestroyCacheView(image_view);
  transverse_image->type=image->type;
  page=transverse_image->page;
  Swap(page.width,page.height);
  Swap(page.x,page.y);
  if (page.width != 0)
    page.x=(ssize_t) page.width-(ssize_t) transverse_image->columns-page.x;
  if (page.height != 0)
    page.y=(ssize_t) page.height-(ssize_t) transverse_image->rows-page.y;
  transverse_image->page=page;
  if (status == MagickFalse)
    transverse_image=DestroyImage(transverse_image);
  return(transverse_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r i m I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TrimImage() trims pixels from the image edges.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the TrimImage method is:
%
%      Image *TrimImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *TrimImage(const Image *image,ExceptionInfo *exception)
{
  const char
    *artifact;

  Image
    *trim_image;

  RectangleInfo
    geometry,
    page;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  geometry=GetImageBoundingBox(image,exception);
  if ((geometry.width == 0) || (geometry.height == 0))
    {
      Image
        *crop_image;

      crop_image=CloneImage(image,1,1,MagickTrue,exception);
      if (crop_image == (Image *) NULL)
        return((Image *) NULL);
      crop_image->background_color.alpha_trait=BlendPixelTrait;
      crop_image->background_color.alpha=(MagickRealType) TransparentAlpha;
      (void) SetImageBackgroundColor(crop_image,exception);
      crop_image->page=image->page;
      crop_image->page.x=(-1);
      crop_image->page.y=(-1);
      return(crop_image);
    }
  page=geometry;
  artifact=GetImageArtifact(image,"trim:minSize");
  if (artifact != (const char *) NULL)
    (void) ParseAbsoluteGeometry(artifact,&page);
  if ((geometry.width < page.width) && (geometry.height < page.height))
    {
      /*
        Limit trim to a minimum size.
      */
      switch (image->gravity)
      {
        case CenterGravity:
        {
          geometry.x-=((ssize_t) page.width-(ssize_t) geometry.width)/2;
          geometry.y-=((ssize_t) page.height-(ssize_t) geometry.height)/2;
          break;
        }
        case NorthWestGravity:
        {
          geometry.x-=((ssize_t) page.width-(ssize_t) geometry.width);
          geometry.y-=((ssize_t) page.height-(ssize_t) geometry.height);
          break;
        }
        case NorthGravity:
        {
          geometry.x-=((ssize_t) page.width-(ssize_t) geometry.width)/2;
          geometry.y-=((ssize_t) page.height-(ssize_t) geometry.height);
          break;
        }
        case NorthEastGravity:
        {
          geometry.y-=((ssize_t) page.height-(ssize_t) geometry.height);
          break;
        }
        case EastGravity:
        {
          geometry.y-=((ssize_t) page.height-(ssize_t) geometry.height)/2;
          break;
        }
        case SouthEastGravity:
          break;
        case SouthGravity:
        {
          geometry.x-=((ssize_t) page.width-(ssize_t) geometry.width)/2;
          break;
        }
        case SouthWestGravity:
        {
          geometry.x-=((ssize_t) page.width-(ssize_t) geometry.width);
          break;
        }
        case WestGravity:
        {
          geometry.x-=((ssize_t) page.width-(ssize_t) geometry.width);
          geometry.y-=((ssize_t) page.height-(ssize_t) geometry.height)/2;
          break;
        }
        default:
          break;
      }
      geometry.width=page.width;
      geometry.height=page.height;
    }
  geometry.x+=image->page.x;
  geometry.y+=image->page.y;
  trim_image=CropImage(image,&geometry,exception);
  if (trim_image != (Image *) NULL)
    Update8BIMClipPath(trim_image,image->columns,image->rows,&geometry);
  return(trim_image);
}
