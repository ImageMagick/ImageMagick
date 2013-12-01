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
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/attribute.h"
#include "magick/cache.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/distort.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/memory_.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/resource_.h"
#include "magick/resize.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/transform.h"

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
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  assert(chop_info != (RectangleInfo *) NULL);
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
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,chop_image,1,1)
#endif
  for (y=0; y < (ssize_t) extent.y; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict chop_indexes,
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(chop_view,0,y,chop_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    chop_indexes=GetCacheViewAuthenticIndexQueue(chop_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x < extent.x) || (x >= (ssize_t) (extent.x+extent.width)))
        {
          *q=(*p);
          if (indexes != (IndexPacket *) NULL)
            {
              if (chop_indexes != (IndexPacket *) NULL)
                *chop_indexes++=GetPixelIndex(indexes+x);
            }
          q++;
        }
      p++;
    }
    if (SyncCacheViewAuthenticPixels(chop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_ChopImage)
#endif
        proceed=SetImageProgress(image,ChopImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  /*
    Extract chop image.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,1,1)
#endif
  for (y=0; y < (ssize_t) (image->rows-(extent.y+extent.height)); y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict chop_indexes,
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,extent.y+extent.height+y,
      image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(chop_view,0,extent.y+y,chop_image->columns,
      1,exception);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    chop_indexes=GetCacheViewAuthenticIndexQueue(chop_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x < extent.x) || (x >= (ssize_t) (extent.x+extent.width)))
        {
          *q=(*p);
          if (indexes != (IndexPacket *) NULL)
            {
              if (chop_indexes != (IndexPacket *) NULL)
                *chop_indexes++=GetPixelIndex(indexes+x);
            }
          q++;
        }
      p++;
    }
    if (SyncCacheViewAuthenticPixels(chop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_ChopImage)
#endif
        proceed=SetImageProgress(image,ChopImageTag,progress++,image->rows);
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

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Consolidate separate C, M, Y, and K planes into a single image.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  cmyk_images=NewImageList();
  for (i=0; i < (ssize_t) GetImageListLength(images); i+=4)
  {
    cmyk_image=CloneImage(images,images->columns,images->rows,MagickTrue,
      exception);
    if (cmyk_image == (Image *) NULL)
      break;
    if (SetImageStorageClass(cmyk_image,DirectClass) == MagickFalse)
      break;
    (void) SetImageColorspace(cmyk_image,CMYKColorspace);
    image_view=AcquireVirtualCacheView(images,exception);
    cmyk_view=AcquireAuthenticCacheView(cmyk_image,exception);
    for (y=0; y < (ssize_t) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register ssize_t
        x;

      register PixelPacket
        *restrict q;

      p=GetCacheViewVirtualPixels(image_view,0,y,images->columns,1,exception);
      q=QueueCacheViewAuthenticPixels(cmyk_view,0,y,cmyk_image->columns,1,
        exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      for (x=0; x < (ssize_t) images->columns; x++)
      {
        SetPixelRed(q,ClampToQuantum(QuantumRange-GetPixelIntensity(images,p)));
        p++;
        q++;
      }
      if (SyncCacheViewAuthenticPixels(cmyk_view,exception) == MagickFalse)
        break;
    }
    cmyk_view=DestroyCacheView(cmyk_view);
    image_view=DestroyCacheView(image_view);
    images=GetNextImageInList(images);
    if (images == (Image *) NULL)
      break;
    image_view=AcquireVirtualCacheView(images,exception);
    cmyk_view=AcquireAuthenticCacheView(cmyk_image,exception);
    for (y=0; y < (ssize_t) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register ssize_t
        x;

      register PixelPacket
        *restrict q;

      p=GetCacheViewVirtualPixels(image_view,0,y,images->columns,1,exception);
      q=GetCacheViewAuthenticPixels(cmyk_view,0,y,cmyk_image->columns,1,
        exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      for (x=0; x < (ssize_t) images->columns; x++)
      {
        q->green=ClampToQuantum(QuantumRange-GetPixelIntensity(images,p));
        p++;
        q++;
      }
      if (SyncCacheViewAuthenticPixels(cmyk_view,exception) == MagickFalse)
        break;
    }
    cmyk_view=DestroyCacheView(cmyk_view);
    image_view=DestroyCacheView(image_view);
    images=GetNextImageInList(images);
    if (images == (Image *) NULL)
      break;
    image_view=AcquireVirtualCacheView(images,exception);
    cmyk_view=AcquireAuthenticCacheView(cmyk_image,exception);
    for (y=0; y < (ssize_t) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register ssize_t
        x;

      register PixelPacket
        *restrict q;

      p=GetCacheViewVirtualPixels(image_view,0,y,images->columns,1,exception);
      q=GetCacheViewAuthenticPixels(cmyk_view,0,y,cmyk_image->columns,1,
        exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      for (x=0; x < (ssize_t) images->columns; x++)
      {
        q->blue=ClampToQuantum(QuantumRange-GetPixelIntensity(images,p));
        p++;
        q++;
      }
      if (SyncCacheViewAuthenticPixels(cmyk_view,exception) == MagickFalse)
        break;
    }
    cmyk_view=DestroyCacheView(cmyk_view);
    image_view=DestroyCacheView(image_view);
    images=GetNextImageInList(images);
    if (images == (Image *) NULL)
      break;
    image_view=AcquireVirtualCacheView(images,exception);
    cmyk_view=AcquireAuthenticCacheView(cmyk_image,exception);
    for (y=0; y < (ssize_t) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register IndexPacket
        *restrict indexes;

      register ssize_t
        x;

      register PixelPacket
        *restrict q;

      p=GetCacheViewVirtualPixels(image_view,0,y,images->columns,1,exception);
      q=GetCacheViewAuthenticPixels(cmyk_view,0,y,cmyk_image->columns,1,
        exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      indexes=GetCacheViewAuthenticIndexQueue(cmyk_view);
      for (x=0; x < (ssize_t) images->columns; x++)
      {
        SetPixelIndex(indexes+x,ClampToQuantum(QuantumRange-
          GetPixelIntensity(images,p)));
        p++;
      }
      if (SyncCacheViewAuthenticPixels(cmyk_view,exception) == MagickFalse)
        break;
    }
    cmyk_view=DestroyCacheView(cmyk_view);
    image_view=DestroyCacheView(image_view);
    AppendImageToList(&cmyk_images,cmyk_image);
    images=GetNextImageInList(images);
    if (images == (Image *) NULL)
      break;
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

  RectangleInfo
    bounding_box,
    page;

  ssize_t
    y;

  /*
    Check crop geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
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
  if (((bounding_box.x-page.x) >= (ssize_t) page.width) ||
      ((bounding_box.y-page.y) >= (ssize_t) page.height) ||
      ((page.x-bounding_box.x) > (ssize_t) image->columns) ||
      ((page.y-bounding_box.y) > (ssize_t) image->rows))
    {
      /*
        Crop is not within virtual canvas, return 1 pixel transparent image.
      */
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "GeometryDoesNotContainImage","`%s'",image->filename);
      crop_image=CloneImage(image,1,1,MagickTrue,exception);
      if (crop_image == (Image *) NULL)
        return((Image *) NULL);
      crop_image->background_color.opacity=(Quantum) TransparentOpacity;
      (void) SetImageBackgroundColor(crop_image);
      crop_image->page=bounding_box;
      crop_image->page.x=(-1);
      crop_image->page.y=(-1);
      if (crop_image->dispose == BackgroundDispose)
        crop_image->dispose=NoneDispose;
      return(crop_image);
    }
  if ((page.x < 0) && (bounding_box.x >= 0))
    {
      page.width+=page.x-bounding_box.x;
      page.x=0;
    }
  else
    {
      page.width-=bounding_box.x-page.x;
      page.x-=bounding_box.x;
      if (page.x < 0)
        page.x=0;
    }
  if ((page.y < 0) && (bounding_box.y >= 0))
    {
      page.height+=page.y-bounding_box.y;
      page.y=0;
    }
  else
    {
      page.height-=bounding_box.y-page.y;
      page.y-=bounding_box.y;
      if (page.y < 0)
        page.y=0;
    }
  if ((size_t) (page.x+page.width) > image->columns)
    page.width=image->columns-page.x;
  if ((geometry->width != 0) && (page.width > geometry->width))
    page.width=geometry->width;
  if ((size_t) (page.y+page.height) > image->rows)
    page.height=image->rows-page.y;
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
  if (((ssize_t) (bounding_box.x+bounding_box.width) > (ssize_t) image->page.width) ||
      ((ssize_t) (bounding_box.y+bounding_box.height) > (ssize_t) image->page.height))
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
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,crop_image,1,1)
#endif
  for (y=0; y < (ssize_t) crop_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict crop_indexes;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,page.x,page.y+y,crop_image->columns,
      1,exception);
    q=QueueCacheViewAuthenticPixels(crop_view,0,y,crop_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    crop_indexes=GetCacheViewAuthenticIndexQueue(crop_view);
    (void) CopyMagickMemory(q,p,(size_t) crop_image->columns*sizeof(*p));
    if ((indexes != (IndexPacket *) NULL) &&
        (crop_indexes != (IndexPacket *) NULL))
      (void) CopyMagickMemory(crop_indexes,indexes,(size_t) crop_image->columns*
        sizeof(*crop_indexes));
    if (SyncCacheViewAuthenticPixels(crop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_CropImage)
#endif
        proceed=SetImageProgress(image,CropImageTag,progress++,image->rows);
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

static inline double MagickRound(double x)
{
  /*
    Round the fraction to nearest integer.
  */
  if ((x-floor(x)) < (ceil(x)-x))
    return(floor(x));
  return(ceil(x));
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  crop_image=NewImageList();
  next=NewImageList();
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
      width=image->columns;
      height=image->rows;
      if (geometry.width == 0)
        geometry.width=1;
      if (geometry.height == 0)
        geometry.height=1;
      if ((flags & AspectValue) == 0)
        {
          width-=(geometry.x < 0 ? -1 : 1)*geometry.x;
          height-=(geometry.y < 0 ? -1 : 1)*geometry.y;
        }
      else
        {
          width+=(geometry.x < 0 ? -1 : 1)*geometry.x;
          height+=(geometry.y < 0 ? -1 : 1)*geometry.y;
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
            crop.y=(ssize_t) MagickRound((MagickRealType) (offset.y-
              (geometry.y > 0 ? 0 : geometry.y)));
            offset.y+=delta.y;   /* increment now to find width */
            crop.height=(size_t) MagickRound((MagickRealType) (offset.y+
              (geometry.y < 0 ? 0 : geometry.y)));
          }
        else
          {
            crop.y=(ssize_t) MagickRound((MagickRealType) (offset.y-
              (geometry.y > 0 ? geometry.y : 0)));
            offset.y+=delta.y;  /* increment now to find width */
            crop.height=(size_t) MagickRound((MagickRealType) (offset.y+
              (geometry.y < 0 ? geometry.y : 0)));
          }
        crop.height-=crop.y;
        crop.y+=image->page.y;
        for (offset.x=0; offset.x < (double) width; )
        {
          if ((flags & AspectValue) == 0)
            {
              crop.x=(ssize_t) MagickRound((MagickRealType) (offset.x-
                (geometry.x > 0 ? 0 : geometry.x)));
              offset.x+=delta.x;  /* increment now to find height */
              crop.width=(size_t) MagickRound((MagickRealType) (offset.x+
                (geometry.x < 0 ? 0 : geometry.x)));
            }
          else
            {
              crop.x=(ssize_t) MagickRound((MagickRealType) (offset.x-
                (geometry.x > 0 ? geometry.x : 0)));
              offset.x+=delta.x;  /* increment now to find height */
              crop.width=(size_t) MagickRound((MagickRealType) (offset.x+
                (geometry.x < 0 ? geometry.x : 0)));
            }
          crop.width-=crop.x;
          crop.x+=image->page.x;
          next=CropImage(image,&crop,exception);
          if (next == (Image *) NULL)
            break;
          AppendImageToList(&crop_image,next);
        }
        if (next == (Image *) NULL)
          break;
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
      next=NewImageList();
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
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
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,excerpt_image,excerpt_image->rows,1)
#endif
  for (y=0; y < (ssize_t) excerpt_image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict excerpt_indexes,
      *restrict indexes;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,geometry->x,geometry->y+y,
      geometry->width,1,exception);
    q=GetCacheViewAuthenticPixels(excerpt_view,0,y,excerpt_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) CopyMagickMemory(q,p,(size_t) excerpt_image->columns*sizeof(*q));
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    if (indexes != (IndexPacket *) NULL)
      {
        excerpt_indexes=GetCacheViewAuthenticIndexQueue(excerpt_view);
        if (excerpt_indexes != (IndexPacket *) NULL)
          (void) CopyMagickMemory(excerpt_indexes,indexes,(size_t)
            excerpt_image->columns*sizeof(*excerpt_indexes));
      }
    if (SyncCacheViewAuthenticPixels(excerpt_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_ExcerptImage)
#endif
        proceed=SetImageProgress(image,ExcerptImageTag,progress++,image->rows);
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

  /*
    Allocate extent image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  extent_image=CloneImage(image,geometry->width,geometry->height,MagickTrue,
    exception);
  if (extent_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(extent_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&extent_image->exception);
      extent_image=DestroyImage(extent_image);
      return((Image *) NULL);
    }
  if (extent_image->background_color.opacity != OpaqueOpacity)
    extent_image->matte=MagickTrue;
  (void) SetImageBackgroundColor(extent_image);
  (void) CompositeImage(extent_image,image->compose,image,-geometry->x,
    -geometry->y);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  flip_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
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
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,flip_image,1,1)
#endif
  for (y=0; y < (ssize_t) flip_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict flip_indexes;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(flip_view,0,(ssize_t) (flip_image->rows-y-
      1),flip_image->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) CopyMagickMemory(q,p,(size_t) image->columns*sizeof(*q));
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    if (indexes != (const IndexPacket *) NULL)
      {
        flip_indexes=GetCacheViewAuthenticIndexQueue(flip_view);
        if (flip_indexes != (IndexPacket *) NULL)
          (void) CopyMagickMemory(flip_indexes,indexes,(size_t) image->columns*
            sizeof(*flip_indexes));
      }
    if (SyncCacheViewAuthenticPixels(flip_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_FlipImage)
#endif
        proceed=SetImageProgress(image,FlipImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  flip_view=DestroyCacheView(flip_view);
  image_view=DestroyCacheView(image_view);
  flip_image->type=image->type;
  if (page.height != 0)
    page.y=(ssize_t) (page.height-flip_image->rows-page.y);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  flop_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
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
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,flop_image,1,1)
#endif
  for (y=0; y < (ssize_t) flop_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict flop_indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(flop_view,0,y,flop_image->columns,1,
      exception);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    q+=flop_image->columns;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    flop_indexes=GetCacheViewAuthenticIndexQueue(flop_view);
    for (x=0; x < (ssize_t) flop_image->columns; x++)
    {
      (*--q)=(*p++);
      if ((indexes != (const IndexPacket *) NULL) &&
          (flop_indexes != (IndexPacket *) NULL))
        SetPixelIndex(flop_indexes+flop_image->columns-x-1,
          GetPixelIndex(indexes+x));
    }
    if (SyncCacheViewAuthenticPixels(flop_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_FlopImage)
#endif
        proceed=SetImageProgress(image,FlopImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  flop_view=DestroyCacheView(flop_view);
  image_view=DestroyCacheView(image_view);
  flop_image->type=image->type;
  if (page.width != 0)
    page.x=(ssize_t) (page.width-flop_image->columns-page.x);
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

static inline MagickBooleanType CopyImageRegion(Image *destination,
  const Image *source,const size_t columns,const size_t rows,
  const ssize_t sx,const ssize_t sy,const ssize_t dx,const ssize_t dy,
  ExceptionInfo *exception)
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
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(source,destination,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict destination_indexes;

    register PixelPacket
      *restrict q;

    /*
      Transfer scanline.
    */
    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(source_view,sx,sy+y,columns,1,exception);
    q=GetCacheViewAuthenticPixels(destination_view,dx,dy+y,columns,1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(source_view);
    (void) CopyMagickMemory(q,p,(size_t) columns*sizeof(*p));
    if (indexes != (IndexPacket *) NULL)
      {
        destination_indexes=GetCacheViewAuthenticIndexQueue(destination_view);
        if (destination_indexes != (IndexPacket *) NULL)
          (void) CopyMagickMemory(destination_indexes,indexes,(size_t)
            columns*sizeof(*indexes));
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  roll_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
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
  status&=CopyImageRegion(roll_image,image,image->columns-offset.x,
    (size_t) offset.y,0,(ssize_t) image->rows-offset.y,offset.x,0,
    exception);
  (void) SetImageProgress(image,RollImageTag,1,3);
  status&=CopyImageRegion(roll_image,image,(size_t) offset.x,image->rows-
    offset.y,(ssize_t) image->columns-offset.x,0,0,offset.y,exception);
  (void) SetImageProgress(image,RollImageTag,2,3);
  status&=CopyImageRegion(roll_image,image,image->columns-offset.x,image->rows-
    offset.y,0,0,offset.x,offset.y,exception);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
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
    y;

  /*
    Allocate splice image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  splice_geometry=(*geometry);
  splice_image=CloneImage(image,image->columns+splice_geometry.width,
    image->rows+splice_geometry.height,MagickTrue,exception);
  if (splice_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(splice_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&splice_image->exception);
      splice_image=DestroyImage(splice_image);
      return((Image *) NULL);
    }
  (void) SetImageBackgroundColor(splice_image);
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
    case StaticGravity:
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
  image_view=AcquireVirtualCacheView(image,exception);
  splice_view=AcquireAuthenticCacheView(splice_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,splice_image,1,1)
#endif
  for (y=0; y < (ssize_t) splice_geometry.y; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict indexes,
      *restrict splice_indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(splice_view,0,y,splice_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    splice_indexes=GetCacheViewAuthenticIndexQueue(splice_view);
    for (x=0; x < splice_geometry.x; x++)
    {
      SetPixelRed(q,GetPixelRed(p));
      SetPixelGreen(q,GetPixelGreen(p));
      SetPixelBlue(q,GetPixelBlue(p));
      SetPixelOpacity(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetPixelOpacity(q,GetPixelOpacity(p));
      if (image->colorspace == CMYKColorspace)
        SetPixelIndex(splice_indexes+x,GetPixelIndex(indexes));
      indexes++;
      p++;
      q++;
    }
    for ( ; x < (ssize_t) (splice_geometry.x+splice_geometry.width); x++)
      q++;
    for ( ; x < (ssize_t) splice_image->columns; x++)
    {
      SetPixelRed(q,GetPixelRed(p));
      SetPixelGreen(q,GetPixelGreen(p));
      SetPixelBlue(q,GetPixelBlue(p));
      SetPixelOpacity(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetPixelOpacity(q,GetPixelOpacity(p));
      if (image->colorspace == CMYKColorspace)
        SetPixelIndex(splice_indexes+x,GetPixelIndex(indexes));
      indexes++;
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(splice_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_TransposeImage)
#endif
        proceed=SetImageProgress(image,SpliceImageTag,progress++,
          splice_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,splice_image,1,1)
#endif
  for (y=(ssize_t) (splice_geometry.y+splice_geometry.height);
       y < (ssize_t) splice_image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict indexes,
      *restrict splice_indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y-(ssize_t) splice_geometry.height,
      image->columns,1,exception);
    if ((y < 0) || (y >= (ssize_t) splice_image->rows))
      continue;
    q=QueueCacheViewAuthenticPixels(splice_view,0,y,splice_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    splice_indexes=GetCacheViewAuthenticIndexQueue(splice_view);
    for (x=0; x < splice_geometry.x; x++)
    {
      SetPixelRed(q,GetPixelRed(p));
      SetPixelGreen(q,GetPixelGreen(p));
      SetPixelBlue(q,GetPixelBlue(p));
      SetPixelOpacity(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetPixelOpacity(q,GetPixelOpacity(p));
      if (image->colorspace == CMYKColorspace)
        SetPixelIndex(splice_indexes+x,GetPixelIndex(indexes));
      indexes++;
      p++;
      q++;
    }
    for ( ; x < (ssize_t) (splice_geometry.x+splice_geometry.width); x++)
      q++;
    for ( ; x < (ssize_t) splice_image->columns; x++)
    {
      SetPixelRed(q,GetPixelRed(p));
      SetPixelGreen(q,GetPixelGreen(p));
      SetPixelBlue(q,GetPixelBlue(p));
      SetPixelOpacity(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetPixelOpacity(q,GetPixelOpacity(p));
      if (image->colorspace == CMYKColorspace)
        SetPixelIndex(splice_indexes+x,GetPixelIndex(indexes));
      indexes++;
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(splice_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_TransposeImage)
#endif
        proceed=SetImageProgress(image,SpliceImageTag,progress++,
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
%  The format of the TransformImage method is:
%
%      MagickBooleanType TransformImage(Image **image,const char *crop_geometry,
%        const char *image_geometry)
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
*/
/*
  DANGER: This function destroys what it assumes to be a single image list.
  If the input image is part of a larger list, all other images in that list
  will be simply 'lost', not destroyed.

  Also if the crop generates a list of images only the first image is resized.
  And finally if the crop succeeds and the resize failed, you will get a
  cropped image, as well as a 'false' or 'failed' report.

  This function and should probably be depreciated in favor of direct calls
  to CropImageToTiles() or ResizeImage(), as appropriate.

*/
MagickExport MagickBooleanType TransformImage(Image **image,
  const char *crop_geometry,const char *image_geometry)
{
  Image
    *resize_image,
    *transform_image;

  MagickStatusType
    flags;

  RectangleInfo
    geometry;

  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  transform_image=(*image);
  if (crop_geometry != (const char *) NULL)
    {
      Image
        *crop_image;

      /*
        Crop image to a user specified size.
      */
      crop_image=CropImageToTiles(*image,crop_geometry,&(*image)->exception);
      if (crop_image == (Image *) NULL)
        transform_image=CloneImage(*image,0,0,MagickTrue,&(*image)->exception);
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
  flags=ParseRegionGeometry(transform_image,image_geometry,&geometry,
    &(*image)->exception);
  (void) flags;
  if ((transform_image->columns == geometry.width) &&
      (transform_image->rows == geometry.height))
    return(MagickTrue);
  resize_image=ResizeImage(transform_image,geometry.width,geometry.height,
    transform_image->filter,transform_image->blur,&(*image)->exception);
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
%   T r a n s f o r m I m a g e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformImages() calls TransformImage() on each image of a sequence.
%
%  The format of the TransformImage method is:
%
%      MagickBooleanType TransformImages(Image **image,
%        const char *crop_geometry,const char *image_geometry)
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
*/
MagickExport MagickBooleanType TransformImages(Image **images,
  const char *crop_geometry,const char *image_geometry)
{
  Image
    *image,
    **image_list,
    *transform_images;

  MagickStatusType
    status;

  register ssize_t
    i;

  assert(images != (Image **) NULL);
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  image_list=ImageListToArray(*images,&(*images)->exception);
  if (image_list == (Image **) NULL)
    return(MagickFalse);
  status=MagickTrue;
  transform_images=NewImageList();
  for (i=0; image_list[i] != (Image *) NULL; i++)
  {
    image=image_list[i];
    status&=TransformImage(&image,crop_geometry,image_geometry);
    AppendImageToList(&transform_images,image);
  }
  *images=transform_images;
  image_list=(Image **) RelinquishMagickMemory(image_list);
  return(status != 0 ? MagickTrue : MagickFalse);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
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
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,transpose_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict transpose_indexes,
      *restrict indexes;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,(ssize_t) image->rows-y-1,
      image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(transpose_view,(ssize_t) (image->rows-y-1),
      0,1,transpose_image->rows,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) CopyMagickMemory(q,p,(size_t) image->columns*sizeof(*q));
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    if (indexes != (IndexPacket *) NULL)
      {
        transpose_indexes=GetCacheViewAuthenticIndexQueue(transpose_view);
        if (transpose_indexes != (IndexPacket *) NULL)
          (void) CopyMagickMemory(transpose_indexes,indexes,(size_t)
            image->columns*sizeof(*transpose_indexes));
      }
    if (SyncCacheViewAuthenticPixels(transpose_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_TransposeImage)
#endif
        proceed=SetImageProgress(image,TransposeImageTag,progress++,
          image->rows);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
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
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,transverse_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict transverse_indexes,
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(transverse_view,(ssize_t) (image->rows-y-
      1),0,1,transverse_image->rows,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    q+=image->columns;
    for (x=0; x < (ssize_t) image->columns; x++)
      *--q=(*p++);
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    if (indexes != (IndexPacket *) NULL)
      {
        transverse_indexes=GetCacheViewAuthenticIndexQueue(transverse_view);
        if (transverse_indexes != (IndexPacket *) NULL)
          for (x=0; x < (ssize_t) image->columns; x++)
            SetPixelIndex(transverse_indexes+image->columns-x-1,
              GetPixelIndex(indexes+x));
      }
    sync=SyncCacheViewAuthenticPixels(transverse_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_TransverseImage)
#endif
        proceed=SetImageProgress(image,TransverseImageTag,progress++,
          image->rows);
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
    page.x=(ssize_t) (page.width-transverse_image->columns-page.x);
  if (page.height != 0)
    page.y=(ssize_t) (page.height-transverse_image->rows-page.y);
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
  RectangleInfo
    geometry;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  geometry=GetImageBoundingBox(image,exception);
  if ((geometry.width == 0) || (geometry.height == 0))
    {
      Image
        *crop_image;

      crop_image=CloneImage(image,1,1,MagickTrue,exception);
      if (crop_image == (Image *) NULL)
        return((Image *) NULL);
      crop_image->background_color.opacity=(Quantum) TransparentOpacity;
      (void) SetImageBackgroundColor(crop_image);
      crop_image->page=image->page;
      crop_image->page.x=(-1);
      crop_image->page.y=(-1);
      return(crop_image);
    }
  geometry.x+=image->page.x;
  geometry.y+=image->page.y;
  return(CropImage(image,&geometry,exception));
}
