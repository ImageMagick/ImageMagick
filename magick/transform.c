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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/transform.h"

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

  long
    j,
    y;

  MagickBooleanType
    proceed,
    status;

  RectangleInfo
    extent;

  register long
    i;

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
  if (((chop_info->x+(long) chop_info->width) < 0) ||
      ((chop_info->y+(long) chop_info->height) < 0) ||
      (chop_info->x > (long) image->columns) ||
      (chop_info->y > (long) image->rows))
    ThrowImageException(OptionWarning,"GeometryDoesNotContainImage");
  extent=(*chop_info);
  if ((extent.x+(long) extent.width) > (long) image->columns)
    extent.width=(unsigned long) ((long) image->columns-extent.x);
  if ((extent.y+(long) extent.height) > (long) image->rows)
    extent.height=(unsigned long) ((long) image->rows-extent.y);
  if (extent.x < 0)
    {
      extent.width-=(unsigned long) (-extent.x);
      extent.x=0;
    }
  if (extent.y < 0)
    {
      extent.height-=(unsigned long) (-extent.y);
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
  i=0;
  j=0;
  image_view=AcquireCacheView(image);
  chop_view=AcquireCacheView(chop_image);
  for (y=0; y < (long) extent.y; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict chop_indexes,
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,i++,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(chop_view,0,j++,chop_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    chop_indexes=GetCacheViewAuthenticIndexQueue(chop_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((x < extent.x) || (x >= (long) (extent.x+extent.width)))
        {
          *q=(*p);
          if (indexes != (IndexPacket *) NULL)
            {
              if (chop_indexes != (IndexPacket *) NULL)
                *chop_indexes++=indexes[x];
            }
          q++;
        }
      p++;
    }
    if (SyncCacheViewAuthenticPixels(chop_view,exception) == MagickFalse)
      status=MagickFalse;
    proceed=SetImageProgress(image,ChopImageTag,y,chop_image->rows);
    if (proceed == MagickFalse)
      status=MagickFalse;
  }
  /*
    Extract chop image.
  */
  i+=extent.height;
  for (y=0; y < (long) (image->rows-(extent.y+extent.height)); y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict chop_indexes,
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,i++,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(chop_view,0,j++,chop_image->columns,1,
      exception);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    chop_indexes=GetCacheViewAuthenticIndexQueue(chop_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((x < extent.x) || (x >= (long) (extent.x+extent.width)))
        {
          *q=(*p);
          if (indexes != (IndexPacket *) NULL)
            {
              if (chop_indexes != (IndexPacket *) NULL)
                *chop_indexes++=indexes[x];
            }
          q++;
        }
      p++;
    }
    if (SyncCacheViewAuthenticPixels(chop_view,exception) == MagickFalse)
      status=MagickFalse;
    proceed=SetImageProgress(image,ChopImageTag,y,chop_image->rows);
    if (proceed == MagickFalse)
      status=MagickFalse;
  }
  chop_view=DestroyCacheView(chop_view);
  image_view=DestroyCacheView(image_view);
  chop_image->type=image->type;
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
  Image
    *cmyk_image,
    *cmyk_images;

  long
    y;

  register long
    i;

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
  for (i=0; i < (long) GetImageListLength(images); i+=4)
  {
    cmyk_image=CloneImage(images,images->columns,images->rows,MagickTrue,
      exception);
    if (cmyk_image == (Image *) NULL)
      break;
    if (SetImageStorageClass(cmyk_image,DirectClass) == MagickFalse)
      break;
    (void) SetImageColorspace(cmyk_image,CMYKColorspace);
    for (y=0; y < (long) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register long
        x;

      register PixelPacket
        *restrict q;

      p=GetVirtualPixels(images,0,y,images->columns,1,exception);
      q=QueueAuthenticPixels(cmyk_image,0,y,cmyk_image->columns,1,exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      for (x=0; x < (long) images->columns; x++)
      {
        q->red=(Quantum) (QuantumRange-PixelIntensityToQuantum(p));
        p++;
        q++;
      }
      if (SyncAuthenticPixels(cmyk_image,exception) == MagickFalse)
        break;
    }
    images=GetNextImageInList(images);
    if (images == (Image *) NULL)
      break;
    for (y=0; y < (long) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register long
        x;

      register PixelPacket
        *restrict q;

      p=GetVirtualPixels(images,0,y,images->columns,1,exception);
      q=GetAuthenticPixels(cmyk_image,0,y,cmyk_image->columns,1,exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      for (x=0; x < (long) images->columns; x++)
      {
        q->green=(Quantum) (QuantumRange-PixelIntensityToQuantum(p));
        p++;
        q++;
      }
      if (SyncAuthenticPixels(cmyk_image,exception) == MagickFalse)
        break;
    }
    images=GetNextImageInList(images);
    if (images == (Image *) NULL)
      break;
    for (y=0; y < (long) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register long
        x;

      register PixelPacket
        *restrict q;

      p=GetVirtualPixels(images,0,y,images->columns,1,exception);
      q=GetAuthenticPixels(cmyk_image,0,y,cmyk_image->columns,1,exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      for (x=0; x < (long) images->columns; x++)
      {
        q->blue=(Quantum) (QuantumRange-PixelIntensityToQuantum(p));
        p++;
        q++;
      }
      if (SyncAuthenticPixels(cmyk_image,exception) == MagickFalse)
        break;
    }
    images=GetNextImageInList(images);
    if (images == (Image *) NULL)
      break;
    for (y=0; y < (long) images->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register IndexPacket
        *restrict indexes;

      register long
        x;

      register PixelPacket
        *restrict q;

      p=GetVirtualPixels(images,0,y,images->columns,1,exception);
      q=GetAuthenticPixels(cmyk_image,0,y,cmyk_image->columns,1,exception);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        break;
      indexes=GetAuthenticIndexQueue(cmyk_image);
      for (x=0; x < (long) images->columns; x++)
      {
        indexes[x]=(IndexPacket) (QuantumRange-PixelIntensityToQuantum(p));
        p++;
      }
      if (SyncAuthenticPixels(cmyk_image,exception) == MagickFalse)
        break;
    }
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
%  by geometry.
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

  long
    progress,
    y;

  MagickBooleanType
    status;

  RectangleInfo
    bounding_box,
    page;

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
  if (((bounding_box.x-page.x) >= (long) page.width) ||
      ((bounding_box.y-page.y) >= (long) page.height) ||
      ((page.x-bounding_box.x) > (long) image->columns) ||
      ((page.y-bounding_box.y) > (long) image->rows))
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
  if ((unsigned long) (page.x+page.width) > image->columns)
    page.width=image->columns-page.x;
  if ((geometry->width != 0) && (page.width > geometry->width))
    page.width=geometry->width;
  if ((unsigned long) (page.y+page.height) > image->rows)
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
  if (((long) (bounding_box.x+bounding_box.width) > (long) image->page.width) ||
      ((long) (bounding_box.y+bounding_box.height) > (long) image->page.height))
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
  image_view=AcquireCacheView(image);
  crop_view=AcquireCacheView(crop_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=0; y < (long) crop_image->rows; y++)
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
    (void) CopyMagickMemory(q,p,(size_t) crop_image->columns*sizeof(*q));
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

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  image_view=AcquireCacheView(image);
  excerpt_view=AcquireCacheView(excerpt_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) excerpt_image->rows; y++)
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
  (void) CompositeImage(extent_image,image->compose,image,geometry->x,
    geometry->y);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  image_view=AcquireCacheView(image);
  flip_view=AcquireCacheView(flip_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=0; y < (long) flip_image->rows; y++)
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
    q=QueueCacheViewAuthenticPixels(flip_view,0,(long) (flip_image->rows-y-1),
      flip_image->columns,1,exception);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  image_view=AcquireCacheView(image);
  flop_view=AcquireCacheView(flop_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=0; y < (long) flop_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict flop_indexes;

    register long
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
    for (x=0; x < (long) flop_image->columns; x++)
    {
      (*--q)=(*p++);
      if ((indexes != (const IndexPacket *) NULL) &&
          (flop_indexes != (IndexPacket *) NULL))
        flop_indexes[flop_image->columns-x-1]=indexes[x];
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
%      Image *RollImage(const Image *image,const long x_offset,
%        const long y_offset,ExceptionInfo *exception)
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
  const Image *source,const unsigned long columns,const unsigned long rows,
  const long sx,const long sy,const long dx,const long dy,
  ExceptionInfo *exception)
{
  CacheView
    *source_view,
    *destination_view;

  long
    y;

  MagickBooleanType
    status;

  status=MagickTrue;
  source_view=AcquireCacheView(source);
  destination_view=AcquireCacheView(destination);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (y=0; y < (long) rows; y++)
  {
    MagickBooleanType
      sync;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict indexes,
      *restrict destination_indexes;

    register long
      x;

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
    indexes=GetCacheViewAuthenticIndexQueue(source_view);
    for (x=0; x < (long) columns; x++)
      *q++=(*p++);
    if (indexes != (IndexPacket *) NULL)
      {
        destination_indexes=GetCacheViewAuthenticIndexQueue(destination_view);
        for (x=0; x < (long) columns; x++)
          destination_indexes[x]=indexes[x];
      }
    sync=SyncCacheViewAuthenticPixels(destination_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
  }
  destination_view=DestroyCacheView(destination_view);
  source_view=DestroyCacheView(source_view);
  return(status);
}

MagickExport Image *RollImage(const Image *image,const long x_offset,
  const long y_offset,ExceptionInfo *exception)
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
    offset.x+=image->columns;
  while (offset.x >= (long) image->columns)
    offset.x-=image->columns;
  while (offset.y < 0)
    offset.y+=image->rows;
  while (offset.y >= (long) image->rows)
    offset.y-=image->rows;
  /*
    Roll image.
  */
  status=CopyImageRegion(roll_image,image,(unsigned long) offset.x,
    (unsigned long) offset.y,(long) image->columns-offset.x,(long) image->rows-
    offset.y,0,0,exception);
  (void) SetImageProgress(image,RollImageTag,0,3);
  status|=CopyImageRegion(roll_image,image,image->columns-offset.x,
    (unsigned long) offset.y,0,(long) image->rows-offset.y,offset.x,0,
    exception);
  (void) SetImageProgress(image,RollImageTag,1,3);
  status|=CopyImageRegion(roll_image,image,(unsigned long) offset.x,image->rows-
    offset.y,(long) image->columns-offset.x,0,0,offset.y,exception);
  (void) SetImageProgress(image,RollImageTag,2,3);
  status|=CopyImageRegion(roll_image,image,image->columns-offset.x,image->rows-
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
  geometry.x=(long) shave_info->width+image->page.x;
  geometry.y=(long) shave_info->height+image->page.y;
  shave_image=CropImage(image,&geometry,exception);
  if (shave_image == (Image *) NULL)
    return((Image *) NULL);
  shave_image->page.width-=2*shave_info->width;
  shave_image->page.height-=2*shave_info->height;
  shave_image->page.x-=shave_info->width;
  shave_image->page.y-=shave_info->height;
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

  long
    progress,
    y;

  MagickBooleanType
    proceed,
    status;

  RectangleInfo
    splice_geometry;

  register long
    i;

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
      splice_geometry.x+=splice_geometry.width/2;
      break;
    }
    case NorthEastGravity:
    {
      splice_geometry.x+=splice_geometry.width;
      break;
    }
    case WestGravity:
    {
      splice_geometry.y+=splice_geometry.width/2;
      break;
    }
    case StaticGravity:
    case CenterGravity:
    {
      splice_geometry.x+=splice_geometry.width/2;
      splice_geometry.y+=splice_geometry.height/2;
      break;
    }
    case EastGravity:
    {
      splice_geometry.x+=splice_geometry.width;
      splice_geometry.y+=splice_geometry.height/2;
      break;
    }
    case SouthWestGravity:
    {
      splice_geometry.y+=splice_geometry.height;
      break;
    }
    case SouthGravity:
    {
      splice_geometry.x+=splice_geometry.width/2;
      splice_geometry.y+=splice_geometry.height;
      break;
    }
    case SouthEastGravity:
    {
      splice_geometry.x+=splice_geometry.width;
      splice_geometry.y+=splice_geometry.height;
      break;
    }
  }
  /*
    Splice image.
  */
  status=MagickTrue;
  i=0;
  progress=0;
  image_view=AcquireCacheView(image);
  splice_view=AcquireCacheView(splice_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) splice_geometry.y; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict indexes,
      *restrict splice_indexes;

    register long
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
      SetRedPixelComponent(q,GetRedPixelComponent(p));
      SetGreenPixelComponent(q,GetGreenPixelComponent(p));
      SetBluePixelComponent(q,GetBluePixelComponent(p));
      SetOpacityPixelComponent(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetOpacityPixelComponent(q,GetOpacityPixelComponent(p));
      if (image->colorspace == CMYKColorspace)
        splice_indexes[x]=(*indexes++);
      p++;
      q++;
    }
    for ( ; x < (long) (splice_geometry.x+splice_geometry.width); x++)
      q++;
    for ( ; x < (long) splice_image->columns; x++)
    {
      SetRedPixelComponent(q,GetRedPixelComponent(p));
      SetGreenPixelComponent(q,GetGreenPixelComponent(p));
      SetBluePixelComponent(q,GetBluePixelComponent(p));
      SetOpacityPixelComponent(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetOpacityPixelComponent(q,GetOpacityPixelComponent(p));
      if (image->colorspace == CMYKColorspace)
        splice_indexes[x]=(*indexes++);
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(splice_view,exception) == MagickFalse)
      status=MagickFalse;
    proceed=SetImageProgress(image,SpliceImageTag,y,splice_image->rows);
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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=(long) (splice_geometry.y+splice_geometry.height);
       y < (long) splice_image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict indexes,
      *restrict splice_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y-splice_geometry.height,
      image->columns,1,exception);
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
      SetRedPixelComponent(q,GetRedPixelComponent(p));
      SetGreenPixelComponent(q,GetGreenPixelComponent(p));
      SetBluePixelComponent(q,GetBluePixelComponent(p));
      SetOpacityPixelComponent(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetOpacityPixelComponent(q,GetOpacityPixelComponent(p));
      if (image->colorspace == CMYKColorspace)
        splice_indexes[x]=(*indexes++);
      p++;
      q++;
    }
    for ( ; x < (long) (splice_geometry.x+splice_geometry.width); x++)
      q++;
    for ( ; x < (long) splice_image->columns; x++)
    {
      SetRedPixelComponent(q,GetRedPixelComponent(p));
      SetGreenPixelComponent(q,GetGreenPixelComponent(p));
      SetBluePixelComponent(q,GetBluePixelComponent(p));
      SetOpacityPixelComponent(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetOpacityPixelComponent(q,GetOpacityPixelComponent(p));
      if (image->colorspace == CMYKColorspace)
        splice_indexes[x]=(*indexes++);
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
%  is returned.
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
static inline long MagickRound(MagickRealType x)
{
  /*
    Round the fraction to nearest integer.
  */
  if (x >= 0.0)
    return((long) (x+0.5));
  return((long) (x-0.5));
}

MagickExport MagickBooleanType TransformImage(Image **image,
  const char *crop_geometry,const char *image_geometry)
{
  Image
    *next,
    *resize_image,
    *transform_image;

  long
    x,
    y;

  MagickStatusType
    flags;

  RectangleInfo
    geometry;

  unsigned long
    height,
    width;

  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  transform_image=(*image);
  if (crop_geometry != (const char *) NULL)
    {
      Image
        *crop_image;

      RectangleInfo
        geometry;

      /*
        Crop image to a user specified size.
      */
      crop_image=NewImageList();
      flags=ParseGravityGeometry(transform_image,crop_geometry,&geometry,
        &(*image)->exception);
      if ((flags & AreaValue) != 0)
        {
          PointInfo
            delta,
            offset;

          RectangleInfo
            crop;

          /*
            Crop into NxM tiles (@ flag) - AT.
          */
          if (geometry.width == 0)
            geometry.width=1;
          if (geometry.height == 0)
            geometry.height=1;
          width=transform_image->columns;
          height=transform_image->rows;
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
          next=NewImageList();
          for (offset.y=0; offset.y < (double) height; )
          {
            if ((flags & AspectValue) == 0)
              {
                crop.y=(long) MagickRound((MagickRealType) (offset.y-
                  (geometry.y > 0 ? 0 : geometry.y)));
                offset.y+=delta.y;
                crop.height=(unsigned long) MagickRound((MagickRealType)
                  (offset.y+(geometry.y < 0 ? 0 : geometry.y)));
              }
            else
              {
                crop.y=(long) MagickRound((MagickRealType) (offset.y-
                  (geometry.y > 0 ? geometry.y : 0)));
                offset.y+=delta.y;
                crop.height=(unsigned long) MagickRound((MagickRealType)
                  (offset.y+(geometry.y < 0 ? geometry.y : 0)));
              }
            crop.height-=crop.y;
            for (offset.x=0; offset.x < (double) width; )
            {
              if ((flags & AspectValue) == 0)
                {
                  crop.x=(long) MagickRound((MagickRealType) (offset.x-
                    (geometry.x > 0 ? 0 : geometry.x)));
                  offset.x+=+delta.x;
                  crop.width=(unsigned long) MagickRound((MagickRealType)
                    (offset.x+(geometry.x < 0 ? 0 : geometry.x)));
                }
              else
                {
                  crop.x=(long) MagickRound((MagickRealType) (offset.x-
                    (geometry.x > 0 ? geometry.x : 0)));
                  offset.x+=+delta.x;
                  crop.width=(unsigned long) MagickRound((MagickRealType)
                    (offset.x+(geometry.x < 0 ? geometry.x : 0)));
                }
              crop.width-=crop.x;
              next=CropImage(transform_image,&crop,&(*image)->exception);
              if (next == (Image *) NULL)
                break;
              AppendImageToList(&crop_image,next);
            }
            if (next == (Image *) NULL)
              break;
          }
        }
      else
        if (((geometry.width == 0) && (geometry.height == 0)) ||
            ((flags & XValue) != 0) || ((flags & YValue) != 0))
          {
            /*
              Crop a single region at +X+Y.
            */
            crop_image=CropImage(transform_image,&geometry,
              &(*image)->exception);
            if ((crop_image != (Image *) NULL) && ((flags & AspectValue) != 0))
              {
                crop_image->page.width=geometry.width;
                crop_image->page.height=geometry.height;
                crop_image->page.x-=geometry.x;
                crop_image->page.y-=geometry.y;
              }
         }
       else
         if ((transform_image->columns > geometry.width) ||
             (transform_image->rows > geometry.height))
           {
             MagickBooleanType
               proceed;

             MagickProgressMonitor
               progress_monitor;

             MagickOffsetType
               i;

             MagickSizeType
               number_images;

             /*
               Crop into tiles of fixed size WxH.
             */
             if (transform_image->page.width == 0)
               transform_image->page.width=transform_image->columns;
             if (transform_image->page.height == 0)
               transform_image->page.height=transform_image->rows;
             width=geometry.width;
             if (width == 0)
               width=transform_image->page.width;
             height=geometry.height;
             if (height == 0)
               height=transform_image->page.height;
             next=NewImageList();
             proceed=MagickTrue;
             i=0;
             number_images=0;
             for (y=0; y < (long) transform_image->page.height; y+=height)
               for (x=0; x < (long) transform_image->page.width; x+=width)
                 number_images++;
             for (y=0; y < (long) transform_image->page.height; y+=height)
             {
               for (x=0; x < (long) transform_image->page.width; x+=width)
               {
                 progress_monitor=SetImageProgressMonitor(transform_image,
                   (MagickProgressMonitor) NULL,transform_image->client_data);
                 geometry.width=width;
                 geometry.height=height;
                 geometry.x=x;
                 geometry.y=y;
                 next=CropImage(transform_image,&geometry,&(*image)->exception);
                 (void) SetImageProgressMonitor(transform_image,
                   progress_monitor,transform_image->client_data);
                 proceed=SetImageProgress(transform_image,CropImageTag,i++,
                   number_images);
                 if (proceed == MagickFalse)
                   break;
                 if (next == (Image *) NULL)
                   break;
                 (void) SetImageProgressMonitor(next,progress_monitor,
                   next->client_data);
                 if (crop_image == (Image *) NULL)
                   crop_image=next;
                 else
                   {
                     next->previous=crop_image;
                     crop_image->next=next;
                     crop_image=crop_image->next;
                   }
               }
               if (next == (Image *) NULL)
                 break;
               if (proceed == MagickFalse)
                 break;
             }
           }
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
  if ((transform_image->columns == geometry.width) &&
      (transform_image->rows == geometry.height))
    return(MagickTrue);
  resize_image=ZoomImage(transform_image,geometry.width,geometry.height,
    &(*image)->exception);
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
%                                                                             % %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

  register long
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
    status|=TransformImage(&image,crop_geometry,image_geometry);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

  RectangleInfo
    page;

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
  image_view=AcquireCacheView(image);
  transpose_view=AcquireCacheView(transpose_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
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
    p=GetCacheViewVirtualPixels(image_view,0,(long) image->rows-y-1,
      image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(transpose_view,(long) (image->rows-y-1),0,
      1,transpose_image->rows,exception);
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
  if (page.width != 0)
    page.x=(long) (page.width-transpose_image->columns-page.x);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

  RectangleInfo
    page;

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
  image_view=AcquireCacheView(image);
  transverse_view=AcquireCacheView(transverse_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict transverse_indexes,
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(transverse_view,(long) (image->rows-y-
      1),0,1,transverse_image->rows,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    q+=image->columns;
    for (x=0; x < (long) image->columns; x++)
      *--q=(*p++);
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    if (indexes != (IndexPacket *) NULL)
      {
        transverse_indexes=GetCacheViewAuthenticIndexQueue(transverse_view);
        if (transverse_indexes != (IndexPacket *) NULL)
          for (x=0; x < (long) image->columns; x++)
            transverse_indexes[image->columns-x-1]=indexes[x];
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
  if (page.height != 0)
    page.y=(long) (page.height-transverse_image->rows-page.y);
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
