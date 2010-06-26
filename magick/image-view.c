/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                     IIIII  M   M   AAA    GGGG  EEEEE                       %
%                       I    MM MM  A   A  G      E                           %
%                       I    M M M  AAAAA  G  GG  EEE                         %
%                       I    M   M  A   A  G   G  E                           %
%                     IIIII  M   M  A   A   GGGG  EEEEE                       %
%                                                                             %
%                        V   V  IIIII  EEEEE  W   W                           %
%                        V   V    I    E      W   W                           %
%                        V   V    I    EEE    W W W                           %
%                         V V     I    E      WW WW                           %
%                          V    IIIII  EEEEE  W   W                           %
%                                                                             %
%                                                                             %
%                       MagickCore Image View Methods                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                March 2003                                   %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/MagickCore.h"
#include "magick/exception-private.h"
#include "magick/monitor-private.h"
#include "magick/thread-private.h"

/*
  Typedef declarations.
*/
struct _ImageView
{
  ExceptionInfo
    *exception;

  Image
    *image;

  CacheView
    *view;

  RectangleInfo
    region;

  size_t
    number_threads;

  MagickBooleanType
    debug;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e V i e w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageView() makes a copy of the specified pixel view.
%
%  The format of the CloneImageView method is:
%
%      ImageView *CloneImageView(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport ImageView *CloneImageView(const ImageView *pixel_view)
{
  ImageView
    *clone_view;

  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  clone_view=(ImageView *) AcquireAlignedMemory(1,sizeof(*clone_view));
  if (clone_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,pixel_view->exception);
  clone_view->view=CloneCacheView(pixel_view->view);
  clone_view->region=pixel_view->region;
  clone_view->number_threads=pixel_view->number_threads;
  clone_view->debug=pixel_view->debug;
  clone_view->signature=MagickSignature;
  return(clone_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e V i e w                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageView() deallocates memory associated with a pixel view.
%
%  The format of the DestroyImageView method is:
%
%      ImageView *DestroyImageView(ImageView *pixel_view,
%        const size_t number_wands,const size_t number_threads)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
%    o number_wand: the number of pixel wands.
%
%    o number_threads: number of threads.
%
*/
MagickExport ImageView *DestroyImageView(ImageView *pixel_view)
{
  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  pixel_view->view=DestroyCacheView(pixel_view->view);
  pixel_view->exception=DestroyExceptionInfo(pixel_view->exception);
  pixel_view->signature=(~MagickSignature);
  pixel_view=(ImageView *) RelinquishMagickMemory(pixel_view);
  return(pixel_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D u p l e x T r a n s f e r I m a g e V i e w I t e r a t o r             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DuplexTransferImageViewIterator() iterates over three pixel views in
%  parallel and calls your transfer method for each scanline of the view.  The
%  source and duplex pixel region is not confined to the image canvas-- that is
%  you can include negative offsets or widths or heights that exceed the image
%  dimension.  However, the destination pixel view is confined to the image
%  canvas-- that is no negative offsets or widths or heights that exceed the
%  image dimension are permitted.
%
%  Use this pragma:
%
%    #pragma omp critical
%
%  to define a section of code in your callback transfer method that must be
%  executed by a single thread at a time.
%
%  The format of the DuplexTransferImageViewIterator method is:
%
%      MagickBooleanType DuplexTransferImageViewIterator(ImageView *source,
%        ImageView *duplex,ImageView *destination,
%        DuplexTransferImageViewMethod transfer,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source pixel view.
%
%    o duplex: the duplex pixel view.
%
%    o destination: the destination pixel view.
%
%    o transfer: the transfer callback method.
%
%    o context: the user defined context.
%
*/
MagickExport MagickBooleanType DuplexTransferImageViewIterator(
  ImageView *source,ImageView *duplex,ImageView *destination,
  DuplexTransferImageViewMethod transfer,void *context)
{
#define DuplexTransferImageViewTag  "ImageView/DuplexTransfer"

  ExceptionInfo
    *exception;

  Image
    *destination_image,
    *duplex_image,
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(source != (ImageView *) NULL);
  assert(source->signature == MagickSignature);
  if (transfer == (DuplexTransferImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  duplex_image=duplex->image;
  destination_image=destination->image;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict duplex_indexes,
      *restrict indexes;

    register const PixelPacket
      *restrict duplex_pixels,
      *restrict pixels;

    register IndexPacket
      *restrict destination_indexes;

    register ssize_t
      id,
      x;

    register PixelPacket
      *restrict destination_pixels;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    pixels=GetCacheViewVirtualPixels(source->view,source->region.x,y,
      source->region.width,1,source->exception);
    if (pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->region.width; x++)
      ;
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->region.x,y,
      duplex->region.width,1,duplex->exception);
    if (duplex_pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    duplex_indexes=GetCacheViewVirtualIndexQueue(duplex->view);
    for (x=0; x < (ssize_t) duplex->region.width; x++)
      ;
    if (duplex_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) duplex->region.width; x++)
        ;
    if (duplex_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) duplex->region.width; x++)
        ;
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (ssize_t) destination->region.width; x++)
      ;
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        ;
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        ;
    if (transfer(source,duplex,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->region.width; x++)
      ;
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        ;
    sync=SyncCacheViewAuthenticPixels(destination->view,exception);
    if (sync == MagickFalse)
      {
        InheritException(destination->exception,GetCacheViewException(
          source->view));
        status=MagickFalse;
      }
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_DuplexTransferImageViewIterator)
#endif
        proceed=SetImageProgress(source_image,DuplexTransferImageViewTag,
          progress++,source->region.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w E x c e p t i o n                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewException() returns the severity, reason, and description of any
%  error that occurs when utilizing a pixel view.
%
%  The format of the GetImageViewException method is:
%
%      char *GetImageViewException(const PixelImage *pixel_view,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel pixel_view.
%
%    o severity: the severity of the error is returned here.
%
*/
MagickExport char *GetImageViewException(const ImageView *pixel_view,
  ExceptionType *severity)
{
  char
    *description;

  assert(pixel_view != (const ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  assert(severity != (ExceptionType *) NULL);
  *severity=pixel_view->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MaxTextExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *description='\0';
  if (pixel_view->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      pixel_view->exception->severity,pixel_view->exception->reason),
        MaxTextExtent);
  if (pixel_view->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MaxTextExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        pixel_view->exception->severity,pixel_view->exception->description),
        MaxTextExtent);
      (void) ConcatenateMagickString(description,")",MaxTextExtent);
    }
  return(description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w H e i g h t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewHeight() returns the pixel view height.
%
%  The format of the GetImageViewHeight method is:
%
%      size_t GetImageViewHeight(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport size_t GetImageViewHeight(const ImageView *pixel_view)
{
  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  return(pixel_view->region.height);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w I t e r a t o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewIterator() iterates over the pixel view in parallel and calls
%  your get method for each scanline of the view.  The pixel region is
%  not confined to the image canvas-- that is you can include negative offsets
%  or widths or heights that exceed the image dimension.  Any updates to
%  the pixels in your callback are ignored.
%
%  Use this pragma:
%
%    #pragma omp critical
%
%  to define a section of code in your callback get method that must be
%  executed by a single thread at a time.
%
%  The format of the GetImageViewIterator method is:
%
%      MagickBooleanType GetImageViewIterator(ImageView *source,
%        GetImageViewMethod get,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source pixel view.
%
%    o get: the get callback method.
%
%    o context: the user defined context.
%
*/
MagickExport MagickBooleanType GetImageViewIterator(ImageView *source,
  GetImageViewMethod get,void *context)
{
#define GetImageViewTag  "ImageView/Get"

  Image
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(source != (ImageView *) NULL);
  assert(source->signature == MagickSignature);
  if (get == (GetImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
  {
    register const IndexPacket
      *indexes;

    register const PixelPacket
      *pixels;

    register ssize_t
      id,
      x;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    pixels=GetCacheViewVirtualPixels(source->view,source->region.x,y,
      source->region.width,1,source->exception);
    if (pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->region.width; x++)
      ;
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    if (get(source,context) == MagickFalse)
      status=MagickFalse;
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_GetImageViewIterator)
#endif
        proceed=SetImageProgress(source_image,GetImageViewTag,progress++,
          source->region.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewPixels() returns the pixel view pixel_wands.
%
%  The format of the GetImageViewPixels method is:
%
%      PixelImage *GetImageViewPixels(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport PixelPacket **GetImageViewPixels(const ImageView *pixel_view)
{
  ssize_t
    id;

  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  id=GetOpenMPThreadId();
  return((PixelPacket **) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w W a n d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewImage() returns the magick wand associated with the pixel view.
%
%  The format of the GetImageViewImage method is:
%
%      MagickCore *GetImageViewImage(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport Image *GetImageViewImage(const ImageView *pixel_view)
{
  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  return(pixel_view->image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w W i d t h                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewWidth() returns the pixel view width.
%
%  The format of the GetImageViewWidth method is:
%
%      size_t GetImageViewWidth(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport size_t GetImageViewWidth(const ImageView *pixel_view)
{
  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  return(pixel_view->region.width);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w X                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewX() returns the pixel view x offset.
%
%  The format of the GetImageViewX method is:
%
%      ssize_t GetImageViewX(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport ssize_t GetImageViewX(const ImageView *pixel_view)
{
  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  return(pixel_view->region.x);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w Y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewY() returns the pixel view y offset.
%
%  The format of the GetImageViewY method is:
%
%      ssize_t GetImageViewY(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport ssize_t GetImageViewY(const ImageView *pixel_view)
{
  assert(pixel_view != (ImageView *) NULL);
  assert(pixel_view->signature == MagickSignature);
  return(pixel_view->region.y);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s I m a g e V i e w                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageView() returns MagickTrue if the the parameter is verified as a pixel
%  view container.
%
%  The format of the IsImageView method is:
%
%      MagickBooleanType IsImageView(const ImageView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
MagickExport MagickBooleanType IsImageView(const ImageView *pixel_view)
{
  if (pixel_view == (const ImageView *) NULL)
    return(MagickFalse);
  if (pixel_view->signature != MagickSignature)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w I m a g e V i e w                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewImageView() returns a pixel view required for all other methods in the
%  Pixel View API.
%
%  The format of the NewImageView method is:
%
%      ImageView *NewImageView(MagickCore *wand)
%
%  A description of each parameter follows:
%
%    o wand: the wand.
%
*/
MagickExport ImageView *NewImageView(Image *image)
{
  ImageView
    *pixel_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  pixel_view=(ImageView *) AcquireAlignedMemory(1,sizeof(*pixel_view));
  if (pixel_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->exception=AcquireExceptionInfo();
  pixel_view->image=image;
  pixel_view->view=AcquireCacheView(pixel_view->image);
  pixel_view->region.width=image->columns;
  pixel_view->region.height=image->rows;
  pixel_view->number_threads=GetOpenMPMaximumThreads();
  pixel_view->debug=IsEventLogging();
  pixel_view->signature=MagickSignature;
  return(pixel_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w I m a g e V i e w R e g i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewImageViewRegion() returns a pixel view required for all other methods
%  in the Pixel View API.
%
%  The format of the NewImageViewRegion method is:
%
%      ImageView *NewImageViewRegion(MagickCore *wand,const ssize_t x,
%        const ssize_t y,const size_t width,const size_t height)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixel_wands view.
%
*/
MagickExport ImageView *NewImageViewRegion(Image *image,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height)
{
  ImageView
    *pixel_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  pixel_view=(ImageView *) AcquireAlignedMemory(1,sizeof(*pixel_view));
  if (pixel_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->exception=AcquireExceptionInfo();
  pixel_view->view=AcquireCacheView(pixel_view->image);
  pixel_view->image=image;
  pixel_view->region.width=width;
  pixel_view->region.height=height;
  pixel_view->region.x=x;
  pixel_view->region.y=y;
  pixel_view->number_threads=GetOpenMPMaximumThreads();
  pixel_view->debug=IsEventLogging();
  pixel_view->signature=MagickSignature;
  return(pixel_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e V i e w I t e r a t o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageViewIterator() iterates over the pixel view in parallel and calls
%  your set method for each scanline of the view.  The pixel region is
%  confined to the image canvas-- that is no negative offsets or widths or
%  heights that exceed the image dimension.  The pixels are initiallly
%  undefined and any settings you make in the callback method are automagically
%  synced back to your image.
%
%  Use this pragma:
%
%    #pragma omp critical
%
%  to define a section of code in your callback set method that must be
%  executed by a single thread at a time.
%
%  The format of the SetImageViewIterator method is:
%
%      MagickBooleanType SetImageViewIterator(ImageView *destination,
%        SetImageViewMethod set,void *context)
%
%  A description of each parameter follows:
%
%    o destination: the pixel view.
%
%    o set: the set callback method.
%
%    o context: the user defined context.
%
*/
MagickExport MagickBooleanType SetImageViewIterator(ImageView *destination,
  SetImageViewMethod set,void *context)
{
#define SetImageViewTag  "ImageView/Set"

  ExceptionInfo
    *exception;

  Image
    *destination_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(destination != (ImageView *) NULL);
  assert(destination->signature == MagickSignature);
  if (set == (SetImageViewMethod) NULL)
    return(MagickFalse);
  destination_image=destination->image;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=destination->region.y; y < (ssize_t) destination->region.height; y++)
  {
    MagickBooleanType
      sync;

    register IndexPacket
      *restrict indexes;

    register ssize_t
      id,
      x;

    register PixelPacket
      *restrict pixels;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    pixels=GetCacheViewAuthenticPixels(destination->view,destination->region.x,
      y,destination->region.width,1,exception);
    if (pixels == (PixelPacket *) NULL)
      {
        InheritException(destination->exception,GetCacheViewException(
          destination->view));
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    if (set(destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->region.width; x++)
      ;
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        ;
    sync=SyncCacheViewAuthenticPixels(destination->view,exception);
    if (sync == MagickFalse)
      {
        InheritException(destination->exception,GetCacheViewException(
          destination->view));
        status=MagickFalse;
      }
    if (destination_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_SetImageViewIterator)
#endif
        proceed=SetImageProgress(destination_image,SetImageViewTag,progress++,
          destination->region.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s f e r I m a g e V i e w I t e r a t o r                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransferImageViewIterator() iterates over two pixel views in parallel and
%  calls your transfer method for each scanline of the view.  The source pixel
%  region is not confined to the image canvas-- that is you can include
%  negative offsets or widths or heights that exceed the image dimension.
%  However, the destination pixel view is confined to the image canvas-- that
%  is no negative offsets or widths or heights that exceed the image dimension
%  are permitted.
%
%  Use this pragma:
%
%    #pragma omp critical
%
%  to define a section of code in your callback transfer method that must be
%  executed by a single thread at a time.
%
%  The format of the TransferImageViewIterator method is:
%
%      MagickBooleanType TransferImageViewIterator(ImageView *source,
%        ImageView *destination,TransferImageViewMethod transfer,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source pixel view.
%
%    o destination: the destination pixel view.
%
%    o transfer: the transfer callback method.
%
%    o context: the user defined context.
%
*/
MagickExport MagickBooleanType TransferImageViewIterator(ImageView *source,
  ImageView *destination,TransferImageViewMethod transfer,void *context)
{
#define TransferImageViewTag  "ImageView/Transfer"

  ExceptionInfo
    *exception;

  Image
    *destination_image,
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(source != (ImageView *) NULL);
  assert(source->signature == MagickSignature);
  if (transfer == (TransferImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  destination_image=destination->image;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict pixels;

    register IndexPacket
      *restrict destination_indexes;

    register ssize_t
      id,
      x;

    register PixelPacket
      *restrict destination_pixels;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    pixels=GetCacheViewVirtualPixels(source->view,source->region.x,y,
      source->region.width,1,source->exception);
    if (pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->region.width; x++)
      ;
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (ssize_t) destination->region.width; x++)
      ;
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        ;
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        ;
    if (transfer(source,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->region.width; x++)
      ;
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        ;
    sync=SyncCacheViewAuthenticPixels(destination->view,exception);
    if (sync == MagickFalse)
      {
        InheritException(destination->exception,GetCacheViewException(
          source->view));
        status=MagickFalse;
      }
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_TransferImageViewIterator)
#endif
        proceed=SetImageProgress(source_image,TransferImageViewTag,progress++,
          source->region.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U p d a t e I m a g e V i e w I t e r a t o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateImageViewIterator() iterates over the pixel view in parallel and calls
%  your update method for each scanline of the view.  The pixel region is
%  confined to the image canvas-- that is no negative offsets or widths or
%  heights that exceed the image dimension are permitted.  Updates to pixels
%  in your callback are automagically synced back to the image.
%
%  Use this pragma:
%
%    #pragma omp critical
%
%  to define a section of code in your callback update method that must be
%  executed by a single thread at a time.
%
%  The format of the UpdateImageViewIterator method is:
%
%      MagickBooleanType UpdateImageViewIterator(ImageView *source,
%        UpdateImageViewMethod update,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source pixel view.
%
%    o update: the update callback method.
%
%    o context: the user defined context.
%
*/
MagickExport MagickBooleanType UpdateImageViewIterator(ImageView *source,
  UpdateImageViewMethod update,void *context)
{
#define UpdateImageViewTag  "ImageView/Update"

  ExceptionInfo
    *exception;

  Image
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(source != (ImageView *) NULL);
  assert(source->signature == MagickSignature);
  if (update == (UpdateImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  if (SetImageStorageClass(source_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=source->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      id,
      x;

    register PixelPacket
      *restrict pixels;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    pixels=GetCacheViewAuthenticPixels(source->view,source->region.x,y,
      source->region.width,1,exception);
    if (pixels == (PixelPacket *) NULL)
      {
        InheritException(source->exception,GetCacheViewException(
          source->view));
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->region.width; x++)
      ;
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    if (update(source,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) source->region.width; x++)
      ;
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        ;
    if (SyncCacheViewAuthenticPixels(source->view,exception) == MagickFalse)
      {
        InheritException(source->exception,GetCacheViewException(source->view));
        status=MagickFalse;
      }
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_UpdateImageViewIterator)
#endif
        proceed=SetImageProgress(source_image,UpdateImageViewTag,progress++,
          source->region.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}
