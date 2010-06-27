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
%                         V   V  IIIII  EEEEE  W   W                          %
%                         V   V    I    E      W   W                          %
%                         V   V    I    EEE    W W W                          %
%                          V V     I    E      WW WW                          %
%                           V    IIIII  EEEEE  W   W                          %
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
%  CloneImageView() makes a copy of the specified image view.
%
%  The format of the CloneImageView method is:
%
%      ImageView *CloneImageView(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport ImageView *CloneImageView(const ImageView *image_view)
{
  ImageView
    *clone_view;

  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  clone_view=(ImageView *) AcquireAlignedMemory(1,sizeof(*clone_view));
  if (clone_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,image_view->exception);
  clone_view->view=CloneCacheView(image_view->view);
  clone_view->region=image_view->region;
  clone_view->number_threads=image_view->number_threads;
  clone_view->debug=image_view->debug;
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
%  DestroyImageView() deallocates memory associated with a image view.
%
%  The format of the DestroyImageView method is:
%
%      ImageView *DestroyImageView(ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport ImageView *DestroyImageView(ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  image_view->view=DestroyCacheView(image_view->view);
  image_view->exception=DestroyExceptionInfo(image_view->exception);
  image_view->signature=(~MagickSignature);
  image_view=(ImageView *) RelinquishMagickMemory(image_view);
  return(image_view);
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
%  DuplexTransferImageViewIterator() iterates over three image views in
%  parallel and calls your transfer method for each scanline of the view.  The
%  source and duplex pixel region is not confined to the image canvas-- that is
%  you can include negative offsets or widths or heights that exceed the image
%  dimension.  However, the destination image view is confined to the image
%  canvas-- that is no negative offsets or widths or heights that exceed the
%  image dimension are permitted.
%
%  Use this pragma if the view is not single threaded:
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
%    o source: the source image view.
%
%    o duplex: the duplex image view.
%
%    o destination: the destination image view.
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
      id;

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
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->region.x,y,
      duplex->region.width,1,duplex->exception);
    if (duplex_pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    duplex_indexes=GetCacheViewVirtualIndexQueue(duplex->view);
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    if (transfer(source,duplex,destination,context) == MagickFalse)
      status=MagickFalse;
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
%   G e t I m a g e V i e w A u t h e n t i c I n d e x e s                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewAuthenticIndexes() returns the image view authentic indexes.
%
%  The format of the GetImageViewAuthenticPixels method is:
%
%      IndexPacket *GetImageViewAuthenticIndexes(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport IndexPacket *GetImageViewAuthenticIndexes(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(GetCacheViewAuthenticIndexQueue(image_view->view));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w A u t h e n t i c P i x e l s                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewAuthenticPixels() returns the image view authentic pixels.
%
%  The format of the GetImageViewAuthenticPixels method is:
%
%      PixelPacket *GetImageViewAuthenticPixels(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport PixelPacket *GetImageViewAuthenticPixels(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(GetCacheViewAuthenticPixelQueue(image_view->view));
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
%  error that occurs when utilizing a image view.
%
%  The format of the GetImageViewException method is:
%
%      char *GetImageViewException(const PixelImage *image_view,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o image_view: the pixel image_view.
%
%    o severity: the severity of the error is returned here.
%
*/
MagickExport char *GetImageViewException(const ImageView *image_view,
  ExceptionType *severity)
{
  char
    *description;

  assert(image_view != (const ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  assert(severity != (ExceptionType *) NULL);
  *severity=image_view->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MaxTextExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *description='\0';
  if (image_view->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      image_view->exception->severity,image_view->exception->reason),
        MaxTextExtent);
  if (image_view->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MaxTextExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        image_view->exception->severity,image_view->exception->description),
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
%  GetImageViewHeight() returns the image view height.
%
%  The format of the GetImageViewHeight method is:
%
%      size_t GetImageViewHeight(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport size_t GetImageViewHeight(const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(image_view->region.height);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewImage() returns the image associated with the image view.
%
%  The format of the GetImageViewImage method is:
%
%      MagickCore *GetImageViewImage(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport Image *GetImageViewImage(const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(image_view->image);
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
%  GetImageViewIterator() iterates over the image view in parallel and calls
%  your get method for each scanline of the view.  The pixel region is
%  not confined to the image canvas-- that is you can include negative offsets
%  or widths or heights that exceed the image dimension.  Any updates to
%  the pixels in your callback are ignored.
%
%  Use this pragma if the view is not single threaded:
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
%    o source: the source image view.
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
      id;

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
%   G e t I m a g e V i e w V i r t u a l I n d e x e s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewVirtualIndexes() returns the image view virtual indexes.
%
%  The format of the GetImageViewVirtualIndexes method is:
%
%      const IndexPacket *GetImageViewVirtualIndexes(
%        const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport const IndexPacket *GetImageViewVirtualIndexes(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(GetCacheViewVirtualIndexQueue(image_view->view));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i e w V i r t u a l P i x e l s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewVirtualPixels() returns the image view virtual pixels.
%
%  The format of the GetImageViewVirtualPixels method is:
%
%      const PixelPacket *GetImageViewVirtualPixels(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport const PixelPacket *GetImageViewVirtualPixels(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(GetCacheViewVirtualPixelQueue(image_view->view));
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
%  GetImageViewWidth() returns the image view width.
%
%  The format of the GetImageViewWidth method is:
%
%      size_t GetImageViewWidth(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport size_t GetImageViewWidth(const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(image_view->region.width);
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
%  GetImageViewX() returns the image view x offset.
%
%  The format of the GetImageViewX method is:
%
%      ssize_t GetImageViewX(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport ssize_t GetImageViewX(const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(image_view->region.x);
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
%  GetImageViewY() returns the image view y offset.
%
%  The format of the GetImageViewY method is:
%
%      ssize_t GetImageViewY(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport ssize_t GetImageViewY(const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(image_view->region.y);
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
%  IsImageView() returns MagickTrue if the the parameter is verified as a image
%  view object.
%
%  The format of the IsImageView method is:
%
%      MagickBooleanType IsImageView(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport MagickBooleanType IsImageView(const ImageView *image_view)
{
  if (image_view == (const ImageView *) NULL)
    return(MagickFalse);
  if (image_view->signature != MagickSignature)
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
%  NewImageView() returns a image view required for all other methods in the
%  Image View API.
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
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  image_view=(ImageView *) AcquireAlignedMemory(1,sizeof(*image_view));
  if (image_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(image_view,0,sizeof(*image_view));
  image_view->exception=AcquireExceptionInfo();
  image_view->image=image;
  image_view->view=AcquireCacheView(image_view->image);
  image_view->region.width=image->columns;
  image_view->region.height=image->rows;
  image_view->number_threads=GetOpenMPMaximumThreads();
  image_view->debug=IsEventLogging();
  image_view->signature=MagickSignature;
  return(image_view);
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
%  NewImageViewRegion() returns a image view required for all other methods
%  in the Image View API.
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
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  image_view=(ImageView *) AcquireAlignedMemory(1,sizeof(*image_view));
  if (image_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(image_view,0,sizeof(*image_view));
  image_view->exception=AcquireExceptionInfo();
  image_view->view=AcquireCacheView(image_view->image);
  image_view->image=image;
  image_view->region.width=width;
  image_view->region.height=height;
  image_view->region.x=x;
  image_view->region.y=y;
  image_view->number_threads=GetOpenMPMaximumThreads();
  image_view->debug=IsEventLogging();
  image_view->signature=MagickSignature;
  return(image_view);
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
%  SetImageViewIterator() iterates over the image view in parallel and calls
%  your set method for each scanline of the view.  The pixel region is
%  confined to the image canvas-- that is no negative offsets or widths or
%  heights that exceed the image dimension.  The pixels are initiallly
%  undefined and any settings you make in the callback method are automagically
%  synced back to your image.
%
%  Use this pragma if the view is not single threaded:
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
%    o destination: the image view.
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
      id;

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
%  TransferImageViewIterator() iterates over two image views in parallel and
%  calls your transfer method for each scanline of the view.  The source pixel
%  region is not confined to the image canvas-- that is you can include
%  negative offsets or widths or heights that exceed the image dimension.
%  However, the destination image view is confined to the image canvas-- that
%  is no negative offsets or widths or heights that exceed the image dimension
%  are permitted.
%
%  Use this pragma if the view is not single threaded:
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
%    o source: the source image view.
%
%    o destination: the destination image view.
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
      id;

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
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    if (transfer(source,destination,context) == MagickFalse)
      status=MagickFalse;
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
%  UpdateImageViewIterator() iterates over the image view in parallel and calls
%  your update method for each scanline of the view.  The pixel region is
%  confined to the image canvas-- that is no negative offsets or widths or
%  heights that exceed the image dimension are permitted.  Updates to pixels
%  in your callback are automagically synced back to the image.
%
%  Use this pragma if the view is not single threaded:
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
%    o source: the source image view.
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

    register PixelPacket
      *restrict pixels;

    register ssize_t
      id;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    pixels=GetCacheViewAuthenticPixels(source->view,source->region.x,y,
      source->region.width,1,exception);
    if (pixels == (PixelPacket *) NULL)
      {
        InheritException(source->exception,GetCacheViewException(source->view));
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(source->view);
    if (update(source,context) == MagickFalse)
      status=MagickFalse;
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
