/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                         W   W   AAA   N   N  DDDD                           %
%                         W   W  A   A  NN  N  D   D                          %
%                         W W W  AAAAA  N N N  D   D                          %
%                         WW WW  A   A  N  NN  D   D                          %
%                         W   W  A   A  N   N  DDDD                           %
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
    name[MaxTextExtent];

  ExceptionInfo
    *exception;

  MagickWand
    *wand;

  CacheView
    *view;

  RectangleInfo
    region;

  size_t
    number_threads;

  PixelWand
    ***pixel_wands;

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
%  CloneWandView() makes a copy of the specified pixel view.
%
%  The format of the CloneWandView method is:
%
%      WandView *CloneWandView(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport WandView *CloneWandView(const WandView *pixel_view)
{
  WandView
    *clone_view;

  register ssize_t
    i;

  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  if (pixel_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",pixel_view->name);
  clone_view=(WandView *) AcquireAlignedMemory(1,sizeof(*clone_view));
  if (clone_view == (WandView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      pixel_view->name);
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->id=AcquireWandId();
  (void) FormatMagickString(clone_view->name,MaxTextExtent,"%s-%.20g",
    WandViewId,(double) clone_view->id);
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,pixel_view->exception);
  clone_view->view=CloneCacheView(pixel_view->view);
  clone_view->region=pixel_view->region;
  clone_view->number_threads=pixel_view->number_threads;
  for (i=0; i < (ssize_t) pixel_view->number_threads; i++)
    clone_view->pixel_wands[i]=ClonePixelWands((const PixelWand **)
      pixel_view->pixel_wands[i],pixel_view->region.width);
  clone_view->debug=pixel_view->debug;
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
%  DestroyWandView() deallocates memory associated with a pixel view.
%
%  The format of the DestroyWandView method is:
%
%      WandView *DestroyWandView(WandView *pixel_view,
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

static PixelWand ***DestroyPixelsThreadSet(PixelWand ***pixel_wands,
  const size_t number_wands,const size_t number_threads)
{
  register ssize_t
    i;

  assert(pixel_wands != (PixelWand ***) NULL);
  for (i=0; i < (ssize_t) number_threads; i++)
    if (pixel_wands[i] != (PixelWand **) NULL)
      pixel_wands[i]=DestroyPixelWands(pixel_wands[i],number_wands);
  pixel_wands=(PixelWand ***) RelinquishAlignedMemory(pixel_wands);
  return(pixel_wands);
}

WandExport WandView *DestroyWandView(WandView *pixel_view)
{
  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  pixel_view->pixel_wands=DestroyPixelsThreadSet(pixel_view->pixel_wands,
    pixel_view->region.width,pixel_view->number_threads);
  pixel_view->view=DestroyCacheView(pixel_view->view);
  pixel_view->exception=DestroyExceptionInfo(pixel_view->exception);
  pixel_view->signature=(~WandSignature);
  RelinquishWandId(pixel_view->id);
  pixel_view=(WandView *) RelinquishMagickMemory(pixel_view);
  return(pixel_view);
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
%  DuplexTransferWandViewIterator() iterates over three pixel views in
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
%  The format of the DuplexTransferWandViewIterator method is:
%
%      MagickBooleanType DuplexTransferWandViewIterator(WandView *source,
%        WandView *duplex,WandView *destination,
%        DuplexTransferWandViewMethod transfer,void *context)
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
WandExport MagickBooleanType DuplexTransferWandViewIterator(
  WandView *source,WandView *duplex,WandView *destination,
  DuplexTransferWandViewMethod transfer,void *context)
{
#define DuplexTransferWandViewTag  "WandView/DuplexTransfer"

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
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],indexes[x]);
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->region.x,y,
      duplex->region.width,1,duplex->exception);
    if (duplex_pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    duplex_indexes=GetCacheViewVirtualIndexQueue(duplex->view);
    for (x=0; x < (ssize_t) duplex->region.width; x++)
      PixelSetQuantumColor(duplex->pixel_wands[id][x],duplex_pixels+x);
    if (duplex_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) duplex->region.width; x++)
        PixelSetBlackQuantum(duplex->pixel_wands[id][x],duplex_indexes[x]);
    if (duplex_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) duplex->region.width; x++)
        PixelSetIndex(duplex->pixel_wands[id][x],duplex_indexes[x]);
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (ssize_t) destination->region.width; x++)
      PixelSetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        PixelSetBlackQuantum(destination->pixel_wands[id][x],
          destination_indexes[x]);
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],destination_indexes[x]);
    if (transfer(source,duplex,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->region.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        destination_indexes[x]=PixelGetBlackQuantum(
          destination->pixel_wands[id][x]);
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
        proceed=SetImageProgress(source_image,DuplexTransferWandViewTag,
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
%   G e t W a n d V i e w E x c e p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewException() returns the severity, reason, and description of any
%  error that occurs when utilizing a pixel view.
%
%  The format of the GetWandViewException method is:
%
%      char *GetWandViewException(const PixelWand *pixel_view,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel pixel_view.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *GetWandViewException(const WandView *pixel_view,
  ExceptionType *severity)
{
  char
    *description;

  assert(pixel_view != (const WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  if (pixel_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",pixel_view->name);
  assert(severity != (ExceptionType *) NULL);
  *severity=pixel_view->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MaxTextExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      pixel_view->name);
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
%   G e t W a n d V i e w H e i g h t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewHeight() returns the pixel view height.
%
%  The format of the GetWandViewHeight method is:
%
%      size_t GetWandViewHeight(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport size_t GetWandViewHeight(const WandView *pixel_view)
{
  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.height);
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
%  GetWandViewIterator() iterates over the pixel view in parallel and calls
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
%  The format of the GetWandViewIterator method is:
%
%      MagickBooleanType GetWandViewIterator(WandView *source,
%        GetWandViewMethod get,void *context)
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
WandExport MagickBooleanType GetWandViewIterator(WandView *source,
  GetWandViewMethod get,void *context)
{
#define GetWandViewTag  "WandView/Get"

  Image
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

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
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],indexes[x]);
    if (get(source,context) == MagickFalse)
      status=MagickFalse;
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickWand_GetWandViewIterator)
#endif
        proceed=SetImageProgress(source_image,GetWandViewTag,progress++,
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
%   G e t W a n d V i e w P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewPixels() returns the pixel view pixel_wands.
%
%  The format of the GetWandViewPixels method is:
%
%      PixelWand *GetWandViewPixels(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport PixelWand **GetWandViewPixels(const WandView *pixel_view)
{
  ssize_t
    id;

  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  id=GetOpenMPThreadId();
  return(pixel_view->pixel_wands[id]);
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
%  GetWandViewWand() returns the magick wand associated with the pixel view.
%
%  The format of the GetWandViewWand method is:
%
%      MagickWand *GetWandViewWand(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport MagickWand *GetWandViewWand(const WandView *pixel_view)
{
  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W a n d V i e w W i d t h                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewWidth() returns the pixel view width.
%
%  The format of the GetWandViewWidth method is:
%
%      size_t GetWandViewWidth(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport size_t GetWandViewWidth(const WandView *pixel_view)
{
  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.width);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W a n d V i e w X                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewX() returns the pixel view x offset.
%
%  The format of the GetWandViewX method is:
%
%      ssize_t GetWandViewX(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport ssize_t GetWandViewX(const WandView *pixel_view)
{
  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.x);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W a n d V i e w Y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWandViewY() returns the pixel view y offset.
%
%  The format of the GetWandViewY method is:
%
%      ssize_t GetWandViewY(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport ssize_t GetWandViewY(const WandView *pixel_view)
{
  assert(pixel_view != (WandView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.y);
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
%  IsWandView() returns MagickTrue if the the parameter is verified as a pixel
%  view container.
%
%  The format of the IsWandView method is:
%
%      MagickBooleanType IsWandView(const WandView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport MagickBooleanType IsWandView(const WandView *pixel_view)
{
  size_t
    length;

  if (pixel_view == (const WandView *) NULL)
    return(MagickFalse);
  if (pixel_view->signature != WandSignature)
    return(MagickFalse);
  length=strlen(WandViewId);
  if (LocaleNCompare(pixel_view->name,WandViewId,length) != 0)
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
%  NewWandView() returns a pixel view required for all other methods in the
%  Pixel View API.
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

  pixel_wands=(PixelWand ***) AcquireAlignedMemory(number_threads,
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
    *pixel_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickSignature);
  pixel_view=(WandView *) AcquireAlignedMemory(1,sizeof(*pixel_view));
  if (pixel_view == (WandView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->id=AcquireWandId();
  (void) FormatMagickString(pixel_view->name,MaxTextExtent,"%s-%.20g",
    WandViewId,(double) pixel_view->id);
  pixel_view->exception=AcquireExceptionInfo();
  pixel_view->wand=wand;
  pixel_view->view=AcquireCacheView(pixel_view->wand->images);
  pixel_view->region.width=wand->images->columns;
  pixel_view->region.height=wand->images->rows;
  pixel_view->number_threads=GetOpenMPMaximumThreads();
  pixel_view->pixel_wands=AcquirePixelsThreadSet(pixel_view->region.width,
    pixel_view->number_threads);
  if (pixel_view->pixel_wands == (PixelWand ***) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  pixel_view->debug=IsEventLogging();
  pixel_view->signature=WandSignature;
  return(pixel_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w W a n d V i e w R e g i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewWandViewRegion() returns a pixel view required for all other methods
%  in the Pixel View API.
%
%  The format of the NewWandViewRegion method is:
%
%      WandView *NewWandViewRegion(MagickWand *wand,const ssize_t x,
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
WandExport WandView *NewWandViewRegion(MagickWand *wand,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height)
{
  WandView
    *pixel_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickSignature);
  pixel_view=(WandView *) AcquireAlignedMemory(1,sizeof(*pixel_view));
  if (pixel_view == (WandView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->id=AcquireWandId();
  (void) FormatMagickString(pixel_view->name,MaxTextExtent,"%s-%.20g",
    WandViewId,(double) pixel_view->id);
  pixel_view->exception=AcquireExceptionInfo();
  pixel_view->view=AcquireCacheView(pixel_view->wand->images);
  pixel_view->wand=wand;
  pixel_view->region.width=width;
  pixel_view->region.height=height;
  pixel_view->region.x=x;
  pixel_view->region.y=y;
  pixel_view->number_threads=GetOpenMPMaximumThreads();
  pixel_view->pixel_wands=AcquirePixelsThreadSet(pixel_view->region.width,
    pixel_view->number_threads);
  if (pixel_view->pixel_wands == (PixelWand ***) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  pixel_view->debug=IsEventLogging();
  pixel_view->signature=WandSignature;
  return(pixel_view);
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
%  SetWandViewIterator() iterates over the pixel view in parallel and calls
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
%  The format of the SetWandViewIterator method is:
%
%      MagickBooleanType SetWandViewIterator(WandView *destination,
%        SetWandViewMethod set,void *context)
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
WandExport MagickBooleanType SetWandViewIterator(WandView *destination,
  SetWandViewMethod set,void *context)
{
#define SetWandViewTag  "WandView/Set"

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
      PixelGetQuantumColor(destination->pixel_wands[id][x],pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        indexes[x]=PixelGetBlackQuantum(destination->pixel_wands[id][x]);
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
        proceed=SetImageProgress(destination_image,SetWandViewTag,progress++,
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
%   T r a n s f e r W a n d V i e w I t e r a t o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransferWandViewIterator() iterates over two pixel views in parallel and
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
%  The format of the TransferWandViewIterator method is:
%
%      MagickBooleanType TransferWandViewIterator(WandView *source,
%        WandView *destination,TransferWandViewMethod transfer,void *context)
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
WandExport MagickBooleanType TransferWandViewIterator(WandView *source,
  WandView *destination,TransferWandViewMethod transfer,void *context)
{
#define TransferWandViewTag  "WandView/Transfer"

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
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],indexes[x]);
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (ssize_t) destination->region.width; x++)
      PixelSetQuantumColor(destination->pixel_wands[id][x],pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        PixelSetBlackQuantum(destination->pixel_wands[id][x],indexes[x]);
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],indexes[x]);
    if (transfer(source,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->region.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        destination_indexes[x]=PixelGetBlackQuantum(
          destination->pixel_wands[id][x]);
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
        proceed=SetImageProgress(source_image,TransferWandViewTag,progress++,
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
%   U p d a t e W a n d V i e w I t e r a t o r                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateWandViewIterator() iterates over the pixel view in parallel and calls
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
%  The format of the UpdateWandViewIterator method is:
%
%      MagickBooleanType UpdateWandViewIterator(WandView *source,
%        UpdateWandViewMethod update,void *context)
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
WandExport MagickBooleanType UpdateWandViewIterator(WandView *source,
  UpdateWandViewMethod update,void *context)
{
#define UpdateWandViewTag  "WandView/Update"

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
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (update(source,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) source->region.width; x++)
      PixelGetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        indexes[x]=PixelGetBlackQuantum(source->pixel_wands[id][x]);
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
        proceed=SetImageProgress(source_image,UpdateWandViewTag,progress++,
          source->region.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}
