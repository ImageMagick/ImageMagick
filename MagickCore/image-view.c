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
#include "MagickCore/MagickCore.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/thread-private.h"

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
  assert(image_view->signature == MagickCoreSignature);
  clone_view=(ImageView *) AcquireCriticalMemory(sizeof(*clone_view));
  (void) memset(clone_view,0,sizeof(*clone_view));
  clone_view->description=ConstantString(image_view->description);
  clone_view->extent=image_view->extent;
  clone_view->view=CloneCacheView(image_view->view);
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,image_view->exception);
  clone_view->debug=image_view->debug;
  clone_view->signature=MagickCoreSignature;
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
  assert(image_view->signature == MagickCoreSignature);
  if (image_view->description != (char *) NULL)
    image_view->description=DestroyString(image_view->description);
  image_view->view=DestroyCacheView(image_view->view);
  image_view->exception=DestroyExceptionInfo(image_view->exception);
  image_view->signature=(~MagickCoreSignature);
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
  assert(source->signature == MagickCoreSignature);
  if (transfer == (DuplexTransferImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  destination_image=destination->image;
  status=SetImageStorageClass(destination_image,DirectClass,
    destination->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=source->extent.height-(size_t) source->extent.y;
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,destination_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    const Quantum
      *magick_restrict duplex_pixels,
      *magick_restrict pixels;

    Quantum
      *magick_restrict destination_pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->extent.x,y,
      duplex->extent.width,1,duplex->exception);
    if (duplex_pixels == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,
      destination->exception);
    if (destination_pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (transfer(source,duplex,destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    sync=SyncCacheViewAuthenticPixels(destination->view,destination->exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;
 
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(source_image,source->description,progress,
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
%   G e t I m a g e V i e w A u t h e n t i c M e t a c o n t e n t           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewAuthenticMetacontent() returns the image view authentic
%  meta-content.
%
%  The format of the GetImageViewAuthenticPixels method is:
%
%      void *GetImageViewAuthenticMetacontent(
%        const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport void *GetImageViewAuthenticMetacontent(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickCoreSignature);
  return(GetCacheViewAuthenticMetacontent(image_view->view));
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
%      Quantum *GetImageViewAuthenticPixels(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport Quantum *GetImageViewAuthenticPixels(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickCoreSignature);
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
  assert(image_view->signature == MagickCoreSignature);
  assert(severity != (ExceptionType *) NULL);
  *severity=image_view->exception->severity;
  description=(char *) AcquireQuantumMemory(MagickPathExtent,
    2*sizeof(*description));
  if (description == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *description='\0';
  if (image_view->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      image_view->exception->severity,image_view->exception->reason),
        MagickPathExtent);
  if (image_view->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MagickPathExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        image_view->exception->severity,image_view->exception->description),
        MagickPathExtent);
      (void) ConcatenateMagickString(description,")",MagickPathExtent);
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
  assert(image_view->signature == MagickCoreSignature);
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
  assert(image_view->signature == MagickCoreSignature);
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
  assert(source->signature == MagickCoreSignature);
  if (get == (GetImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=source->extent.height-(size_t) source->extent.y;
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,source_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const Quantum *) NULL)
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
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(source_image,source->description,progress,
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
%   G e t I m a g e V i e w V i r t u a l M e t a c o n t e n t     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageViewVirtualMetacontent() returns the image view virtual
%  meta-content.
%
%  The format of the GetImageViewVirtualMetacontent method is:
%
%      const void *GetImageViewVirtualMetacontent(
%        const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport const void *GetImageViewVirtualMetacontent(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickCoreSignature);
  return(GetCacheViewVirtualMetacontent(image_view->view));
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
%      const Quantum *GetImageViewVirtualPixels(const ImageView *image_view)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
*/
MagickExport const Quantum *GetImageViewVirtualPixels(
  const ImageView *image_view)
{
  assert(image_view != (ImageView *) NULL);
  assert(image_view->signature == MagickCoreSignature);
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
%  IsImageView() returns MagickTrue if the parameter is verified as a image
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
  if (image_view->signature != MagickCoreSignature)
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
%      ImageView *NewImageView(MagickCore *wand,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ImageView *NewImageView(Image *image,ExceptionInfo *exception)
{
  ImageView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  image_view=(ImageView *) AcquireCriticalMemory(sizeof(*image_view));
  (void) memset(image_view,0,sizeof(*image_view));
  image_view->description=ConstantString("ImageView");
  image_view->image=image;
  image_view->view=AcquireVirtualCacheView(image_view->image,exception);
  image_view->extent.width=image->columns;
  image_view->extent.height=image->rows;
  image_view->extent.x=0;
  image_view->extent.y=0;
  image_view->exception=AcquireExceptionInfo();
  image_view->debug=(GetLogEventMask() & ImageEvent) != 0 ? MagickTrue : 
    MagickFalse;
  image_view->signature=MagickCoreSignature;
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
%        const ssize_t y,const size_t width,const size_t height,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x,y,columns,rows:  These values define the perimeter of a extent of
%      pixel_wands view.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ImageView *NewImageViewRegion(Image *image,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height,
  ExceptionInfo *exception)
{
  ImageView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  image_view=(ImageView *) AcquireCriticalMemory(sizeof(*image_view));
  (void) memset(image_view,0,sizeof(*image_view));
  image_view->description=ConstantString("ImageView");
  image_view->view=AcquireVirtualCacheView(image_view->image,exception);
  image_view->image=image;
  image_view->extent.width=width;
  image_view->extent.height=height;
  image_view->extent.x=x;
  image_view->extent.y=y;
  image_view->exception=AcquireExceptionInfo();
  image_view->debug=(GetLogEventMask() & ImageEvent) != 0 ? MagickTrue : 
    MagickFalse;
  image_view->signature=MagickCoreSignature;
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
  assert(image_view->signature == MagickCoreSignature);
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
%  heights that exceed the image dimension.  The pixels are initially
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
  assert(destination->signature == MagickCoreSignature);
  if (set == (SetImageViewMethod) NULL)
    return(MagickFalse);
  destination_image=destination->image;
  status=SetImageStorageClass(destination_image,DirectClass,
    destination->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=destination->extent.height-(size_t) destination->extent.y;
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(destination_image,destination_image,height,1)
#endif
  for (y=destination->extent.y; y < (ssize_t) destination->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    Quantum
      *magick_restrict pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(destination->view,destination->extent.x,
      y,destination->extent.width,1,destination->exception);
    if (pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (set(destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    sync=SyncCacheViewAuthenticPixels(destination->view,destination->exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (destination_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(destination_image,destination->description,
          progress,destination->extent.height);
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
  assert(source->signature == MagickCoreSignature);
  if (transfer == (TransferImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  destination_image=destination->image;
  status=SetImageStorageClass(destination_image,DirectClass,
    destination->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=source->extent.height-(size_t) source->extent.y;
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,destination_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    const Quantum
      *magick_restrict pixels;

    Quantum
      *magick_restrict destination_pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,
      destination->exception);
    if (destination_pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (transfer(source,destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    sync=SyncCacheViewAuthenticPixels(destination->view,destination->exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(source_image,source->description,progress,
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
  assert(source->signature == MagickCoreSignature);
  if (update == (UpdateImageViewMethod) NULL)
    return(MagickFalse);
  source_image=source->image;
  status=SetImageStorageClass(source_image,DirectClass,source->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=source->extent.height-(size_t) source->extent.y;
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,source_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    Quantum
      *magick_restrict pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (update(source,y,id,context) == MagickFalse)
      status=MagickFalse;
    status=SyncCacheViewAuthenticPixels(source->view,source->exception);
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(source_image,source->description,progress,
          source->extent.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}
