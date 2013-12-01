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
%                                   Cristy                                    %
%                                March 2003                                   %
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
  char
    *description;

  RectangleInfo
    extent;

  Image
    *image;

  CacheView
    *view;

  size_t
    number_threads;

  ExceptionInfo
    *exception;

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
  clone_view=(ImageView *) AcquireMagickMemory(sizeof(*clone_view));
  if (clone_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->description=ConstantString(image_view->description);
  clone_view->extent=image_view->extent;
  clone_view->view=CloneCacheView(image_view->view);
  clone_view->number_threads=image_view->number_threads;
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,image_view->exception);
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
  if (image_view->description != (char *) NULL)
    image_view->description=DestroyString(image_view->description);
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
%  source and duplex pixel extent is not confined to the image canvas-- that is
%  you can include negative offsets or widths or heights that exceed the image
%  dimension.  However, the destination image view is confined to the image
%  canvas-- that is no negative offsets or widths or heights that exceed the
%  image dimension are permitted.
%
%  The callback signature is:
%
%      MagickBooleanType DuplexTransferImageViewMethod(const ImageView *source,
%        const ImageView *duplex,ImageView *destination,const ssize_t y,
%        const int thread_id,void *context)
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
  ExceptionInfo
    *exception;

  Image
    *destination_image,
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  size_t
    height;
#endif

  ssize_t
    y;

  assert(source != (ImageView *) NULL);
  assert(source->signature == MagickSignature);
  if (transfer == (DuplexTransferImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  destination_image=destination->image;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) (source->extent.height-source->extent.y);
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(source_image,destination_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    register const PixelPacket
      *restrict duplex_pixels,
      *restrict pixels;

    register PixelPacket
      *restrict destination_pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->extent.x,y,
      duplex->extent.width,1,duplex->exception);
    if (duplex_pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (transfer(source,duplex,destination,y,id,context) == MagickFalse)
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
        proceed=SetImageProgress(source_image,source->description,progress++,
          source->extent.height);
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
%   G e t I m a g e V i e w E x t e n t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewExtent() returns the image view extent.
%
%  The format of the GetImageViewExtent method is:
%
%      RectangleInfo GetImageViewExtent(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport RectangleInfo GetImageViewExtent(const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  return(image_view->extent);
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
%  your get method for each scanline of the view.  The pixel extent is
%  not confined to the image canvas-- that is you can include negative offsets
%  or widths or heights that exceed the image dimension.  Any updates to
%  the pixels in your callback are ignored.
%
%  The callback signature is:
%
%      MagickBooleanType GetImageViewMethod(const ImageView *source,
%        const ssize_t y,const int thread_id,void *context)
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
  Image
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  size_t
    height;
#endif

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
  height=(size_t) (source->extent.height-source->extent.y);
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(source_image,source_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register const PixelPacket
      *pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (get(source,y,id,context) == MagickFalse)
      status=MagickFalse;
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_GetImageViewIterator)
#endif
        proceed=SetImageProgress(source_image,source->description,progress++,
          source->extent.height);
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
  image_view=(ImageView *) AcquireMagickMemory(sizeof(*image_view));
  if (image_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(image_view,0,sizeof(*image_view));
  image_view->description=ConstantString("ImageView");
  image_view->image=image;
  image_view->exception=AcquireExceptionInfo();
  image_view->view=AcquireVirtualCacheView(image_view->image,
    image_view->exception);
  image_view->extent.width=image->columns;
  image_view->extent.height=image->rows;
  image_view->extent.x=0;
  image_view->extent.y=0;
  image_view->number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
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
%    o x,y,columns,rows:  These values define the perimeter of a extent of
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
  image_view=(ImageView *) AcquireMagickMemory(sizeof(*image_view));
  if (image_view == (ImageView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(image_view,0,sizeof(*image_view));
  image_view->description=ConstantString("ImageView");
  image_view->exception=AcquireExceptionInfo();
  image_view->view=AcquireVirtualCacheView(image_view->image,
    image_view->exception);
  image_view->image=image;
  image_view->extent.width=width;
  image_view->extent.height=height;
  image_view->extent.x=x;
  image_view->extent.y=y;
  image_view->number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  image_view->debug=IsEventLogging();
  image_view->signature=MagickSignature;
  return(image_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e V i e w D e s c r i p t i o n                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageViewDescription() associates a description with an image view.
%
%  The format of the SetImageViewDescription method is:
%
%      void SetImageViewDescription(ImageView *image_view,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
%    o description: the image view description.
%
*/
MagickExport void SetImageViewDescription(ImageView *image_view,
  const char *description)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  image_view->description=ConstantString(description);
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
%  your set method for each scanline of the view.  The pixel extent is
%  confined to the image canvas-- that is no negative offsets or widths or
%  heights that exceed the image dimension.  The pixels are initiallly
%  undefined and any settings you make in the callback method are automagically
%  synced back to your image.
%
%  The callback signature is:
%
%      MagickBooleanType SetImageViewMethod(ImageView *destination,
%        const ssize_t y,const int thread_id,void *context)
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
  ExceptionInfo
    *exception;

  Image
    *destination_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  size_t
    height;
#endif

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
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) (destination->extent.height-destination->extent.y);
#endif
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(destination_image,destination_image,height,1)
#endif
  for (y=destination->extent.y; y < (ssize_t) destination->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    register PixelPacket
      *restrict pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(destination->view,destination->extent.x,
      y,destination->extent.width,1,exception);
    if (pixels == (PixelPacket *) NULL)
      {
        InheritException(destination->exception,GetCacheViewException(
          destination->view));
        status=MagickFalse;
        continue;
      }
    if (set(destination,y,id,context) == MagickFalse)
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
        proceed=SetImageProgress(destination_image,destination->description,
          progress++,destination->extent.height);
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
%   S e t I m a g e V i e w T h r e a d s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageViewThreads() sets the number of threads in a thread team.
%
%  The format of the SetImageViewDescription method is:
%
%      void SetImageViewThreads(ImageView *image_view,
%        const size_t number_threads)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
%    o number_threads: the number of threads in a thread team.
%
*/
MagickExport void SetImageViewThreads(ImageView *image_view,
  const size_t number_threads)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickSignature);
  image_view->number_threads=number_threads;
  if (number_threads > (size_t) GetMagickResourceLimit(ThreadResource))
    image_view->number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
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
%  extent is not confined to the image canvas-- that is you can include
%  negative offsets or widths or heights that exceed the image dimension.
%  However, the destination image view is confined to the image canvas-- that
%  is no negative offsets or widths or heights that exceed the image dimension
%  are permitted.
%
%  The callback signature is:
%
%      MagickBooleanType TransferImageViewMethod(const ImageView *source,
%        ImageView *destination,const ssize_t y,const int thread_id,
%        void *context)
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
  ExceptionInfo
    *exception;

  Image
    *destination_image,
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  size_t
    height;
#endif

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
  height=(size_t) (source->extent.height-source->extent.y);
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(source_image,destination_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    register const PixelPacket
      *restrict pixels;

    register PixelPacket
      *restrict destination_pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (transfer(source,destination,y,id,context) == MagickFalse)
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
        proceed=SetImageProgress(source_image,source->description,progress++,
          source->extent.height);
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
%  your update method for each scanline of the view.  The pixel extent is
%  confined to the image canvas-- that is no negative offsets or widths or
%  heights that exceed the image dimension are permitted.  Updates to pixels
%  in your callback are automagically synced back to the image.
%
%  The callback signature is:
%
%      MagickBooleanType UpdateImageViewMethod(ImageView *source,
%        const ssize_t y,const int thread_id,void *context)
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
  ExceptionInfo
    *exception;

  Image
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  size_t
    height;
#endif

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
  height=(size_t) (source->extent.height-source->extent.y);
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(source_image,source_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register PixelPacket
      *restrict pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(source->view,source->extent.x,y,
      source->extent.width,1,exception);
    if (pixels == (PixelPacket *) NULL)
      {
        InheritException(source->exception,GetCacheViewException(source->view));
        status=MagickFalse;
        continue;
      }
    if (update(source,y,id,context) == MagickFalse)
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
        proceed=SetImageProgress(source_image,source->description,progress++,
          source->extent.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}
