/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                        W   W   AAA   N   N  DDDD                            %
%                        W   W  A   A  NN  N  D   D                           %
%                        W W W  AAAAA  N N N  D   D                           %
%                        WW WW  A   A  N  NN  D   D                           %
%                        W   W  A   A  N   N  DDDD                            %
%                                                                             %
%                        V   V  IIIII  EEEEE  W   W                           %
%                        V   V    I    E      W   W                           %
%                        V   V    I    EEE    W W W                           %
%                         V V     I    E      WW WW                           %
%                          V    IIIII  EEEEE  W   W                           %
%                                                                             %
%                                                                             %
%                        MagickWand Wand View Methods                         %
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
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/magick-wand-private.h"
#include "MagickWand/wand.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/thread-private.h"
/*
 Define declarations.
*/
#define WandViewId  "WandView"

/*
  Typedef declarations.
*/
struct _WandView
{
  size_t
    id;

  char
    name[MagickPathExtent],
    *description;

  RectangleInfo
    extent;

  MagickWand
    *wand;

  CacheView
    *view;

  Image
    *image;

  PixelWand
    ***pixel_wands;

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
%   C l o n e W a n d V i e w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneWandView() makes a copy of the specified wand view.
%
%  The format of the CloneWandView method is:
%
%      WandView *CloneWandView(const WandView *wand_view)
%
%  A description of each parameter follows:
%
%    o wand_view: the wand view.
%
*/
WandExport WandView *CloneWandView(const WandView *wand_view)
{
  ssize_t
    i;

  WandView
    *clone_view;

  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == MagickWandSignature);
  if (wand_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand_view->name);
  clone_view=(WandView *) AcquireCriticalMemory(sizeof(*clone_view));
  (void) memset(clone_view,0,sizeof(*clone_view));
  clone_view->id=AcquireWandId();
  (void) FormatLocaleString(clone_view->name,MagickPathExtent,"%s-%.20g",
    WandViewId,(double) clone_view->id);
  clone_view->description=ConstantString(wand_view->description);
  clone_view->image=CloneImage(wand_view->image,0,0,MagickTrue,
    wand_view->exception);
  clone_view->view=CloneCacheView(wand_view->view);
  clone_view->extent=wand_view->extent;
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,wand_view->exception);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    clone_view->pixel_wands[i]=ClonePixelWands((const PixelWand **)
      wand_view->pixel_wands[i],wand_view->extent.width);
  clone_view->debug=wand_view->debug;
  if (clone_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clone_view->name);
  clone_view->signature=MagickWandSignature;
  return(clone_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y W a n d V i e w                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyWandView() deallocates memory associated with a wand view.
%
%  The format of the DestroyWandView method is:
%
%      WandView *DestroyWandView(WandView *wand_view)
%
%  A description of each parameter follows:
%
%    o wand_view: the wand view.
%
*/

static PixelWand ***DestroyPixelsTLS(PixelWand ***pixel_wands,
  const size_t number_wands)
{
  ssize_t
    i;

  assert(pixel_wands != (PixelWand ***) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (pixel_wands[i] != (PixelWand **) NULL)
      pixel_wands[i]=DestroyPixelWands(pixel_wands[i],number_wands);
  pixel_wands=(PixelWand ***) RelinquishMagickMemory(pixel_wands);
  return(pixel_wands);
}

WandExport WandView *DestroyWandView(WandView *wand_view)
{
  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == MagickWandSignature);
  wand_view->pixel_wands=DestroyPixelsTLS(wand_view->pixel_wands,
    wand_view->extent.width);
  wand_view->view=DestroyCacheView(wand_view->view);
  wand_view->exception=DestroyExceptionInfo(wand_view->exception);
  wand_view->signature=(~MagickWandSignature);
  RelinquishWandId(wand_view->id);
  wand_view=(WandView *) RelinquishMagickMemory(wand_view);
  return(wand_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D u p l e x T r a n s f e r W a n d V i e w I t e r a t o r               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DuplexTransferWandViewIterator() iterates over three wand views in
%  parallel and calls your transfer method for each scanline of the view.  The
%  source and duplex pixel extent is not confined to the image canvas-- that is
%  you can include negative offsets or widths or heights that exceed the image
%  dimension.  However, the destination wand view is confined to the image
%  canvas-- that is no negative offsets or widths or heights that exceed the
%  image dimension are permitted.
%
%  The callback signature is:
%
%      MagickBooleanType DuplexTransferImageViewMethod(const WandView *source,
%        const WandView *duplex,WandView *destination,const ssize_t y,
%        const int thread_id,void *context)
%
%  Use this pragma if the view is not single threaded:
%
%    #pragma omp critical
%
%  to define a section of code in your callback transfer method that must be
%  executed by a single thread at a time.
%
%  The format of the DuplexTransferWandViewIterator method is:
%
%      MagickBooleanType DuplexTransferWandViewIterator(WandView *source,
%        WandView *duplex,WandView *destination,
%        DuplexTransferWandViewMethod transfer,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source wand view.
%
%    o duplex: the duplex wand view.
%
%    o destination: the destination wand view.
%
%    o transfer: the transfer callback method.
%
%    o context: the user defined context.
%
*/
WandExport MagickBooleanType DuplexTransferWandViewIterator(WandView *source,
  WandView *duplex,WandView *destination,DuplexTransferWandViewMethod transfer,
  void *context)
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

  assert(source != (WandView *) NULL);
  assert(source->signature == MagickWandSignature);
  if (transfer == (DuplexTransferWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  destination_image=destination->wand->images;
  status=SetImageStorageClass(destination_image,DirectClass,
    destination->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) ((ssize_t) source->extent.height-source->extent.y);
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

    ssize_t
      x;

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
    for (x=0; x < (ssize_t) source->extent.width; x++)
    {
      PixelSetQuantumPixel(source->image,pixels,source->pixel_wands[id][x]);
      pixels+=GetPixelChannels(source->image);
    }
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->extent.x,y,
      duplex->extent.width,1,duplex->exception);
    if (duplex_pixels == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) duplex->extent.width; x++)
    {
      PixelSetQuantumPixel(duplex->image,duplex_pixels,
        duplex->pixel_wands[id][x]);
      duplex_pixels+=GetPixelChannels(duplex->image);
    }
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,
      destination->exception);
    if (destination_pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) destination->extent.width; x++)
    {
      PixelSetQuantumPixel(destination->image,destination_pixels,
        destination->pixel_wands[id][x]);
      destination_pixels+=GetPixelChannels(destination->image);
    }
    if (transfer(source,duplex,destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,
      destination->exception);
    for (x=0; x < (ssize_t) destination->extent.width; x++)
    {
      PixelGetQuantumPixel(destination->image,destination->pixel_wands[id][x],
        destination_pixels);
      destination_pixels+=GetPixelChannels(destination->image);
    }
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
%   G e t W a n d V i e w E x c e p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewException() returns the severity, reason, and description of any
%  error that occurs when utilizing a wand view.
%
%  The format of the GetWandViewException method is:
%
%      char *GetWandViewException(const WandView *wand_view,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o wand_view: the pixel wand_view.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *GetWandViewException(const WandView *wand_view,
  ExceptionType *severity)
{
  char
    *description;

  assert(wand_view != (const WandView *) NULL);
  assert(wand_view->signature == MagickWandSignature);
  if (wand_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand_view->name);
  assert(severity != (ExceptionType *) NULL);
  *severity=wand_view->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MagickPathExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      wand_view->name);
  *description='\0';
  if (wand_view->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      wand_view->exception->severity,wand_view->exception->reason),
        MagickPathExtent);
  if (wand_view->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MagickPathExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        wand_view->exception->severity,wand_view->exception->description),
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
%   G e t W a n d V i e w E x t e n t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewExtent() returns the wand view extent.
%
%  The format of the GetWandViewExtent method is:
%
%      RectangleInfo GetWandViewExtent(const WandView *wand_view)
%
%  A description of each parameter follows:
%
%    o wand_view: the wand view.
%
*/
WandExport RectangleInfo GetWandViewExtent(const WandView *wand_view)
{
  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == MagickWandSignature);
  return(wand_view->extent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W a n d V i e w I t e r a t o r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewIterator() iterates over the wand view in parallel and calls
%  your get method for each scanline of the view.  The pixel extent is
%  not confined to the image canvas-- that is you can include negative offsets
%  or widths or heights that exceed the image dimension.  Any updates to
%  the pixels in your callback are ignored.
%
%  The callback signature is:
%
%      MagickBooleanType GetImageViewMethod(const WandView *source,
%        const ssize_t y,const int thread_id,void *context)
%
%  Use this pragma if the view is not single threaded:
%
%    #pragma omp critical
%
%  to define a section of code in your callback get method that must be
%  executed by a single thread at a time.
%
%  The format of the GetWandViewIterator method is:
%
%      MagickBooleanType GetWandViewIterator(WandView *source,
%        GetWandViewMethod get,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source wand view.
%
%    o get: the get callback method.
%
%    o context: the user defined context.
%
*/
WandExport MagickBooleanType GetWandViewIterator(WandView *source,
  GetWandViewMethod get,void *context)
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

  assert(source != (WandView *) NULL);
  assert(source->signature == MagickWandSignature);
  if (get == (GetWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) ((ssize_t) source->extent.height-source->extent.y);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,source_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *pixels;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) source->extent.width; x++)
    {
      PixelSetQuantumPixel(source->image,pixels,source->pixel_wands[id][x]);
      pixels+=GetPixelChannels(source->image);
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
%   G e t W a n d V i e w P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewPixels() returns the wand view pixel_wands.
%
%  The format of the GetWandViewPixels method is:
%
%      PixelWand *GetWandViewPixels(const WandView *wand_view)
%
%  A description of each parameter follows:
%
%    o wand_view: the wand view.
%
*/
WandExport PixelWand **GetWandViewPixels(const WandView *wand_view)
{
  const int
    id = GetOpenMPThreadId();

  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == MagickWandSignature);
  return(wand_view->pixel_wands[id]);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W a n d V i e w W a n d                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewWand() returns the magick wand associated with the wand view.
%
%  The format of the GetWandViewWand method is:
%
%      MagickWand *GetWandViewWand(const WandView *wand_view)
%
%  A description of each parameter follows:
%
%    o wand_view: the wand view.
%
*/
WandExport MagickWand *GetWandViewWand(const WandView *wand_view)
{
  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == MagickWandSignature);
  return(wand_view->wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s W a n d V i e w                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWandView() returns MagickTrue if the parameter is verified as a wand
%  view object.
%
%  The format of the IsWandView method is:
%
%      MagickBooleanType IsWandView(const WandView *wand_view)
%
%  A description of each parameter follows:
%
%    o wand_view: the wand view.
%
*/
WandExport MagickBooleanType IsWandView(const WandView *wand_view)
{
  size_t
    length;

  if (wand_view == (const WandView *) NULL)
    return(MagickFalse);
  if (wand_view->signature != MagickWandSignature)
    return(MagickFalse);
  length=strlen(WandViewId);
  if (LocaleNCompare(wand_view->name,WandViewId,length) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w W a n d V i e w                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewWandView() returns a wand view required for all other methods in the
%  Wand View API.
%
%  The format of the NewWandView method is:
%
%      WandView *NewWandView(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the wand.
%
*/

static PixelWand ***AcquirePixelsTLS(const size_t number_wands)
{
  PixelWand
    ***pixel_wands;

  size_t
    number_threads;

  ssize_t
    i;

  number_threads=GetOpenMPMaximumThreads();
  pixel_wands=(PixelWand ***) AcquireQuantumMemory(number_threads,
    sizeof(*pixel_wands));
  if (pixel_wands == (PixelWand ***) NULL)
    return((PixelWand ***) NULL);
  (void) memset(pixel_wands,0,number_threads*sizeof(*pixel_wands));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    pixel_wands[i]=NewPixelWands(number_wands);
    if (pixel_wands[i] == (PixelWand **) NULL)
      return(DestroyPixelsTLS(pixel_wands,number_wands));
  }
  return(pixel_wands);
}

WandExport WandView *NewWandView(MagickWand *wand)
{
  ExceptionInfo
    *exception;

  WandView
    *wand_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  wand_view=(WandView *) AcquireCriticalMemory(sizeof(*wand_view));
  (void) memset(wand_view,0,sizeof(*wand_view));
  wand_view->id=AcquireWandId();
  (void) FormatLocaleString(wand_view->name,MagickPathExtent,"%s-%.20g",
    WandViewId,(double) wand_view->id);
  wand_view->description=ConstantString("WandView");
  wand_view->wand=wand;
  exception=AcquireExceptionInfo();
  wand_view->view=AcquireVirtualCacheView(wand_view->wand->images,exception);
  wand_view->image=(Image *) GetCacheViewImage(wand_view->view);
  wand_view->extent.width=wand_view->image->columns;
  wand_view->extent.height=wand_view->image->rows;
  wand_view->pixel_wands=AcquirePixelsTLS(wand_view->extent.width);
  wand_view->exception=exception;
  if (wand_view->pixel_wands == (PixelWand ***) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  wand_view->debug=IsEventLogging();
  wand_view->signature=MagickWandSignature;
  return(wand_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w W a n d V i e w E x t e n t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewWandViewExtent() returns a wand view required for all other methods
%  in the Wand View API.
%
%  The format of the NewWandViewExtent method is:
%
%      WandView *NewWandViewExtent(MagickWand *wand,const ssize_t x,
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
WandExport WandView *NewWandViewExtent(MagickWand *wand,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height)
{
  ExceptionInfo
    *exception;

  WandView
    *wand_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  wand_view=(WandView *) AcquireCriticalMemory(sizeof(*wand_view));
  (void) memset(wand_view,0,sizeof(*wand_view));
  wand_view->id=AcquireWandId();
  (void) FormatLocaleString(wand_view->name,MagickPathExtent,"%s-%.20g",
    WandViewId,(double) wand_view->id);
  wand_view->description=ConstantString("WandView");
  exception=AcquireExceptionInfo();
  wand_view->view=AcquireVirtualCacheView(wand_view->wand->images,exception);
  wand_view->wand=wand;
  wand_view->extent.width=width;
  wand_view->extent.height=height;
  wand_view->extent.x=x;
  wand_view->extent.y=y;
  wand_view->exception=exception;
  wand_view->pixel_wands=AcquirePixelsTLS(wand_view->extent.width);
  if (wand_view->pixel_wands == (PixelWand ***) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  wand_view->debug=IsEventLogging();
  wand_view->signature=MagickWandSignature;
  return(wand_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t W a n d V i e w D e s c r i p t i o n                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetWandViewDescription() associates a description with an image view.
%
%  The format of the SetWandViewDescription method is:
%
%      void SetWandViewDescription(WandView *image_view,const char *description)
%
%  A description of each parameter follows:
%
%    o wand_view: the wand view.
%
%    o description: the wand view description.
%
*/
MagickExport void SetWandViewDescription(WandView *wand_view,
  const char *description)
{
  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == MagickWandSignature);
  wand_view->description=ConstantString(description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t W a n d V i e w I t e r a t o r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetWandViewIterator() iterates over the wand view in parallel and calls
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
%  The format of the SetWandViewIterator method is:
%
%      MagickBooleanType SetWandViewIterator(WandView *destination,
%        SetWandViewMethod set,void *context)
%
%  A description of each parameter follows:
%
%    o destination: the wand view.
%
%    o set: the set callback method.
%
%    o context: the user defined context.
%
*/
WandExport MagickBooleanType SetWandViewIterator(WandView *destination,
  SetWandViewMethod set,void *context)
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

  assert(destination != (WandView *) NULL);
  assert(destination->signature == MagickWandSignature);
  if (set == (SetWandViewMethod) NULL)
    return(MagickFalse);
  destination_image=destination->wand->images;
  status=SetImageStorageClass(destination_image,DirectClass,
    destination->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) ((ssize_t) destination->extent.height-destination->extent.y);
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

    ssize_t
      x;

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
    for (x=0; x < (ssize_t) destination->extent.width; x++)
    {
      PixelGetQuantumPixel(destination->image,destination->pixel_wands[id][x],
        pixels);
      pixels+=GetPixelChannels(destination->image);
    }
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
%   T r a n s f e r W a n d V i e w I t e r a t o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransferWandViewIterator() iterates over two wand views in parallel and
%  calls your transfer method for each scanline of the view.  The source pixel
%  extent is not confined to the image canvas-- that is you can include
%  negative offsets or widths or heights that exceed the image dimension.
%  However, the destination wand view is confined to the image canvas-- that
%  is no negative offsets or widths or heights that exceed the image dimension
%  are permitted.
%
%  The callback signature is:
%
%      MagickBooleanType TransferImageViewMethod(const WandView *source,
%        WandView *destination,const ssize_t y,const int thread_id,
%        void *context)
%
%  Use this pragma if the view is not single threaded:
%
%    #pragma omp critical
%
%  to define a section of code in your callback transfer method that must be
%  executed by a single thread at a time.
%
%  The format of the TransferWandViewIterator method is:
%
%      MagickBooleanType TransferWandViewIterator(WandView *source,
%        WandView *destination,TransferWandViewMethod transfer,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source wand view.
%
%    o destination: the destination wand view.
%
%    o transfer: the transfer callback method.
%
%    o context: the user defined context.
%
*/
WandExport MagickBooleanType TransferWandViewIterator(WandView *source,
  WandView *destination,TransferWandViewMethod transfer,void *context)
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

  assert(source != (WandView *) NULL);
  assert(source->signature == MagickWandSignature);
  if (transfer == (TransferWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  destination_image=destination->wand->images;
  status=SetImageStorageClass(destination_image,DirectClass,
    destination->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) ((ssize_t) source->extent.height-source->extent.y);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,destination_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *magick_restrict pixels;

    MagickBooleanType
      sync;

    Quantum
      *magick_restrict destination_pixels;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) source->extent.width; x++)
    {
      PixelSetQuantumPixel(source->image,pixels,source->pixel_wands[id][x]);
      pixels+=GetPixelChannels(source->image);
    }
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,
      destination->exception);
    if (destination_pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) destination->extent.width; x++)
    {
      PixelSetQuantumPixel(destination->image,destination_pixels,
        destination->pixel_wands[id][x]);
      destination_pixels+=GetPixelChannels(destination->image);
    }
    if (transfer(source,destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,
      destination->exception);
    for (x=0; x < (ssize_t) destination->extent.width; x++)
    {
      PixelGetQuantumPixel(destination->image,destination->pixel_wands[id][x],
        destination_pixels);
      destination_pixels+=GetPixelChannels(destination->image);
    }
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
%   U p d a t e W a n d V i e w I t e r a t o r                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateWandViewIterator() iterates over the wand view in parallel and calls
%  your update method for each scanline of the view.  The pixel extent is
%  confined to the image canvas-- that is no negative offsets or widths or
%  heights that exceed the image dimension are permitted.  Updates to pixels
%  in your callback are automagically synced back to the image.
%
%  The callback signature is:
%
%      MagickBooleanType UpdateImageViewMethod(WandView *source,const ssize_t y,
%        const int thread_id,void *context)
%
%  Use this pragma if the view is not single threaded:
%
%    #pragma omp critical
%
%  to define a section of code in your callback update method that must be
%  executed by a single thread at a time.
%
%  The format of the UpdateWandViewIterator method is:
%
%      MagickBooleanType UpdateWandViewIterator(WandView *source,
%        UpdateWandViewMethod update,void *context)
%
%  A description of each parameter follows:
%
%    o source: the source wand view.
%
%    o update: the update callback method.
%
%    o context: the user defined context.
%
*/
WandExport MagickBooleanType UpdateWandViewIterator(WandView *source,
  UpdateWandViewMethod update,void *context)
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

  assert(source != (WandView *) NULL);
  assert(source->signature == MagickWandSignature);
  if (update == (UpdateWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  status=SetImageStorageClass(source_image,DirectClass,source->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) ((ssize_t) source->extent.height-source->extent.y);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,source_image,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *magick_restrict p;

    MagickBooleanType
      sync;

    ssize_t
      x;

    Quantum
      *magick_restrict pixels,
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    p=(const Quantum *) pixels;
    for (x=0; x < (ssize_t) source->extent.width; x++)
    {
      PixelSetQuantumPixel(source->image,p,source->pixel_wands[id][x]);
      p+=GetPixelChannels(source->image);
    }
    if (update(source,y,id,context) == MagickFalse)
      status=MagickFalse;
    q=pixels;
    for (x=0; x < (ssize_t) source->extent.width; x++)
    {
      PixelGetQuantumPixel(source->image,source->pixel_wands[id][x],q);
      q+=GetPixelChannels(source->image);
    }
    sync=SyncCacheViewAuthenticPixels(source->view,source->exception);
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
