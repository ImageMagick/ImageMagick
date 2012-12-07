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
%                                John Cristy                                  %
%                                March 2003                                   %
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
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/magick-wand-private.h"
#include "wand/wand.h"
#include "magick/monitor-private.h"
#include "magick/thread-private.h"
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
    name[MaxTextExtent],
    *description;

  RectangleInfo
    extent;

  MagickWand
    *wand;

  CacheView
    *view;

  size_t
    number_threads;

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
  WandView
    *clone_view;

  register ssize_t
    i;

  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == WandSignature);
  if (wand_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand_view->name);
  clone_view=(WandView *) AcquireMagickMemory(sizeof(*clone_view));
  if (clone_view == (WandView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      wand_view->name);
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->id=AcquireWandId();
  (void) FormatLocaleString(clone_view->name,MaxTextExtent,"%s-%.20g",
    WandViewId,(double) clone_view->id);
  clone_view->description=ConstantString(wand_view->description);
  clone_view->view=CloneCacheView(wand_view->view);
  clone_view->extent=wand_view->extent;
  clone_view->number_threads=wand_view->number_threads;
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,wand_view->exception);
  for (i=0; i < (ssize_t) wand_view->number_threads; i++)
    clone_view->pixel_wands[i]=ClonePixelWands((const PixelWand **)
      wand_view->pixel_wands[i],wand_view->extent.width);
  clone_view->debug=wand_view->debug;
  if (clone_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clone_view->name);
  clone_view->signature=WandSignature;
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

static PixelWand ***DestroyPixelsThreadSet(PixelWand ***pixel_wands,
  const size_t number_wands,const size_t number_threads)
{
  register ssize_t
    i;

  assert(pixel_wands != (PixelWand ***) NULL);
  for (i=0; i < (ssize_t) number_threads; i++)
    if (pixel_wands[i] != (PixelWand **) NULL)
      pixel_wands[i]=DestroyPixelWands(pixel_wands[i],number_wands);
  pixel_wands=(PixelWand ***) RelinquishMagickMemory(pixel_wands);
  return(pixel_wands);
}

WandExport WandView *DestroyWandView(WandView *wand_view)
{
  assert(wand_view != (WandView *) NULL);
  assert(wand_view->signature == WandSignature);
  wand_view->pixel_wands=DestroyPixelsThreadSet(wand_view->pixel_wands,
    wand_view->extent.width,wand_view->number_threads);
  wand_view->view=DestroyCacheView(wand_view->view);
  wand_view->exception=DestroyExceptionInfo(wand_view->exception);
  wand_view->signature=(~WandSignature);
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

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  size_t
    height,
    width;
#endif

  ssize_t
    y;

  assert(source != (WandView *) NULL);
  assert(source->signature == WandSignature);
  if (transfer == (DuplexTransferWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  duplex_image=duplex->wand->images;
  destination_image=destination->wand->images;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) (source->extent.height-source->extent.y);
  width=(size_t) (source->extent.width-source->extent.x);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    dynamic_number_threads(source_image,width,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

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
      x;

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
    indexes=GetCacheViewVirtualIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->extent.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelBlack(indexes+x));
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->extent.x,y,
      duplex->extent.width,1,duplex->exception);
    if (duplex_pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    duplex_indexes=GetCacheViewVirtualIndexQueue(duplex->view);
    for (x=0; x < (ssize_t) duplex->extent.width; x++)
      PixelSetQuantumColor(duplex->pixel_wands[id][x],duplex_pixels+x);
    if (duplex_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) duplex->extent.width; x++)
        PixelSetBlackQuantum(duplex->pixel_wands[id][x],
          GetPixelBlack(duplex_indexes+x));
    if (duplex_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) duplex->extent.width; x++)
        PixelSetIndex(duplex->pixel_wands[id][x],
          GetPixelIndex(duplex_indexes+x));
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (ssize_t) destination->extent.width; x++)
      PixelSetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->extent.width; x++)
        PixelSetBlackQuantum(destination->pixel_wands[id][x],
          GetPixelBlack(destination_indexes+x));
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->extent.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],
          GetPixelIndex(destination_indexes+x));
    if (transfer(source,duplex,destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->extent.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->extent.width; x++)
        SetPixelBlack(destination_indexes+x,PixelGetBlackQuantum(
          destination->pixel_wands[id][x]));
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
        #pragma omp critical (MagickWand_DuplexTransferWandViewIterator)
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
  assert(wand_view->signature == WandSignature);
  if (wand_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand_view->name);
  assert(severity != (ExceptionType *) NULL);
  *severity=wand_view->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MaxTextExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      wand_view->name);
  *description='\0';
  if (wand_view->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      wand_view->exception->severity,wand_view->exception->reason),
        MaxTextExtent);
  if (wand_view->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MaxTextExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        wand_view->exception->severity,wand_view->exception->description),
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
  assert(wand_view->signature == WandSignature);
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
    height,
    width;
#endif

  ssize_t
    y;

  assert(source != (WandView *) NULL);
  assert(source->signature == WandSignature);
  if (get == (GetWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) (source->extent.height-source->extent.y);
  width=(size_t) (source->extent.width-source->extent.x);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    dynamic_number_threads(source_image,width,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register const IndexPacket
      *indexes;

    register const PixelPacket
      *pixels;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(source->view,source->extent.x,y,
      source->extent.width,1,source->exception);
    if (pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->extent.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelBlack(indexes+x));
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (get(source,y,id,context) == MagickFalse)
      status=MagickFalse;
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickWand_GetWandViewIterator)
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
  assert(wand_view->signature == WandSignature);
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
  assert(wand_view->signature == WandSignature);
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
%  IsWandView() returns MagickTrue if the the parameter is verified as a wand
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
  if (wand_view->signature != WandSignature)
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

static PixelWand ***AcquirePixelsThreadSet(const size_t number_wands,
  const size_t number_threads)
{
  PixelWand
    ***pixel_wands;

  register ssize_t
    i;

  pixel_wands=(PixelWand ***) AcquireQuantumMemory(number_threads,
    sizeof(*pixel_wands));
  if (pixel_wands == (PixelWand ***) NULL)
    return((PixelWand ***) NULL);
  (void) ResetMagickMemory(pixel_wands,0,number_threads*sizeof(*pixel_wands));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    pixel_wands[i]=NewPixelWands(number_wands);
    if (pixel_wands[i] == (PixelWand **) NULL)
      return(DestroyPixelsThreadSet(pixel_wands,number_wands,number_threads));
  }
  return(pixel_wands);
}

WandExport WandView *NewWandView(MagickWand *wand)
{
  WandView
    *wand_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  wand_view=(WandView *) AcquireMagickMemory(sizeof(*wand_view));
  if (wand_view == (WandView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(wand_view,0,sizeof(*wand_view));
  wand_view->id=AcquireWandId();
  (void) FormatLocaleString(wand_view->name,MaxTextExtent,"%s-%.20g",
    WandViewId,(double) wand_view->id);
  wand_view->description=ConstantString("WandView");
  wand_view->wand=wand;
  wand_view->exception=AcquireExceptionInfo();
  wand_view->view=AcquireVirtualCacheView(wand_view->wand->images,
    wand_view->exception);
  wand_view->extent.width=wand->images->columns;
  wand_view->extent.height=wand->images->rows;
  wand_view->number_threads=GetOpenMPMaximumThreads();
  wand_view->pixel_wands=AcquirePixelsThreadSet(wand_view->extent.width,
    wand_view->number_threads);
  if (wand_view->pixel_wands == (PixelWand ***) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  wand_view->debug=IsEventLogging();
  wand_view->signature=WandSignature;
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
  WandView
    *wand_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  wand_view=(WandView *) AcquireMagickMemory(sizeof(*wand_view));
  if (wand_view == (WandView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(wand_view,0,sizeof(*wand_view));
  wand_view->id=AcquireWandId();
  (void) FormatLocaleString(wand_view->name,MaxTextExtent,"%s-%.20g",
    WandViewId,(double) wand_view->id);
  wand_view->description=ConstantString("WandView");
  wand_view->exception=AcquireExceptionInfo();
  wand_view->view=AcquireVirtualCacheView(wand_view->wand->images,
    wand_view->exception);
  wand_view->wand=wand;
  wand_view->extent.width=width;
  wand_view->extent.height=height;
  wand_view->extent.x=x;
  wand_view->extent.y=y;
  wand_view->number_threads=GetOpenMPMaximumThreads();
  wand_view->pixel_wands=AcquirePixelsThreadSet(wand_view->extent.width,
    wand_view->number_threads);
  if (wand_view->pixel_wands == (PixelWand ***) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  wand_view->debug=IsEventLogging();
  wand_view->signature=WandSignature;
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
  assert(wand_view->signature == WandSignature);
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
    height,
    width;
#endif

  ssize_t
    y;

  assert(destination != (WandView *) NULL);
  assert(destination->signature == WandSignature);
  if (set == (SetWandViewMethod) NULL)
    return(MagickFalse);
  destination_image=destination->wand->images;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) (destination->extent.height-destination->extent.y);
  width=(size_t) (destination->extent.width-destination->extent.x);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    dynamic_number_threads(destination_image,width,height,1)
#endif
  for (y=destination->extent.y; y < (ssize_t) destination->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

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
    indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    if (set(destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->extent.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->extent.width; x++)
        SetPixelBlack(indexes+x,PixelGetBlackQuantum(
          destination->pixel_wands[id][x]));
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
        #pragma omp critical (MagickWand_SetWandViewIterator)
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
%   S e t W a n d V i e w T h r e a d s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetWandViewThreads() sets the number of threads in a thread team.
%
%  The format of the SetWandViewDescription method is:
%
%      void SetWandViewThreads(WandView *image_view,
%        const size_t number_threads)
%
%  A description of each parameter follows:
%
%    o image_view: the image view.
%
%    o number_threads: the number of threads in a thread team.
%
*/
MagickExport void SetWandViewThreads(WandView *image_view,
  const size_t number_threads)
{
  assert(image_view != (WandView *) NULL);
  assert(image_view->signature == MagickSignature);
  image_view->number_threads=number_threads;
  if (number_threads > (size_t) GetMagickResourceLimit(ThreadResource))
    image_view->number_threads=GetOpenMPMaximumThreads();
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
    height,
    width;
#endif

  ssize_t
    y;

  assert(source != (WandView *) NULL);
  assert(source->signature == WandSignature);
  if (transfer == (TransferWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  destination_image=destination->wand->images;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) (source->extent.height-source->extent.y);
  width=(size_t) (source->extent.width-source->extent.x);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    dynamic_number_threads(source_image,width,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict pixels;

    register IndexPacket
      *restrict destination_indexes;

    register ssize_t
      x;

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
    indexes=GetCacheViewVirtualIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->extent.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelBlack(indexes+x));
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->extent.x,y,destination->extent.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (ssize_t) destination->extent.width; x++)
      PixelSetQuantumColor(destination->pixel_wands[id][x],pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->extent.width; x++)
        PixelSetBlackQuantum(destination->pixel_wands[id][x],
          GetPixelBlack(indexes+x));
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->extent.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (transfer(source,destination,y,id,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->extent.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->extent.width; x++)
        SetPixelBlack(destination_indexes+x,PixelGetBlackQuantum(
          destination->pixel_wands[id][x]));
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
        #pragma omp critical (MagickWand_TransferWandViewIterator)
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
    height,
    width;
#endif

  ssize_t
    y;

  assert(source != (WandView *) NULL);
  assert(source->signature == WandSignature);
  if (update == (UpdateWandViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  if (SetImageStorageClass(source_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=source->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  height=(size_t) (source->extent.height-source->extent.y);
  width=(size_t) (source->extent.width-source->extent.x);
  #pragma omp parallel for schedule(static) shared(progress,status) \
    dynamic_number_threads(source_image,width,height,1)
#endif
  for (y=source->extent.y; y < (ssize_t) source->extent.height; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict pixels;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(source->view,source->extent.x,y,
      source->extent.width,1,exception);
    if (pixels == (PixelPacket *) NULL)
      {
        InheritException(source->exception,GetCacheViewException(
          source->view));
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(source->view);
    for (x=0; x < (ssize_t) source->extent.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelBlack(indexes+x));
    if (update(source,y,id,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) source->extent.width; x++)
      PixelGetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->extent.width; x++)
        SetPixelBlack(indexes+x,PixelGetBlackQuantum(
          source->pixel_wands[id][x]));
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
        #pragma omp critical (MagickWand_UpdateWandViewIterator)
#endif
        proceed=SetImageProgress(source_image,source->description,progress++,
          source->extent.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}
