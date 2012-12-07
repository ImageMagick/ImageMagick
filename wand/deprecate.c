/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        DDDD   EEEEE  PPPP   RRRR   EEEEE   CCCC   AAA   TTTTT  EEEEE        %
%        D   D  E      P   P  R   R  E      C      A   A    T    E            %
%        D   D  EEE    PPPPP  RRRR   EEE    C      AAAAA    T    EEE          %
%        D   D  E      P      R R    E      C      A   A    T    E            %
%        DDDD   EEEEE  P      R  R   EEEEE   CCCC  A   A    T    EEEEE        %
%                                                                             %
%                                                                             %
%                       MagickWand Deprecated Methods                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                October 2002                                 %
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
#define PixelViewId  "PixelView"
#define ThrowWandException(severity,tag,context) \
{ \
  (void) ThrowMagickException(wand->exception,GetMagickModule(),severity, \
    tag,"`%s'",context); \
  return(MagickFalse); \
}

/*
  Typedef declarations.
*/
struct _PixelView
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

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A v e r a g e I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAverageImages() average a set of images.
%
%  The format of the MagickAverageImages method is:
%
%      MagickWand *MagickAverageImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/

static MagickWand *CloneMagickWandFromImages(const MagickWand *wand,
  Image *images)
{
  MagickWand
    *clone_wand;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  clone_wand=(MagickWand *) AcquireMagickMemory(sizeof(*clone_wand));
  if (clone_wand == (MagickWand *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      images->filename);
  (void) ResetMagickMemory(clone_wand,0,sizeof(*clone_wand));
  clone_wand->id=AcquireWandId();
  (void) FormatLocaleString(clone_wand->name,MaxTextExtent,"%s-%.20g",
    MagickWandId,(double) clone_wand->id);
  clone_wand->exception=AcquireExceptionInfo();
  InheritException(clone_wand->exception,wand->exception);
  clone_wand->image_info=CloneImageInfo(wand->image_info);
  clone_wand->quantize_info=CloneQuantizeInfo(wand->quantize_info);
  clone_wand->images=images;
  clone_wand->debug=IsEventLogging();
  if (clone_wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clone_wand->name);
  clone_wand->signature=WandSignature;
  return(clone_wand);
}

WandExport MagickWand *MagickAverageImages(MagickWand *wand)
{
  Image
    *average_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  average_image=EvaluateImages(wand->images,MeanEvaluateOperator,
    wand->exception);
  if (average_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,average_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e P i x e l V i e w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelView() makes a copy of the specified pixel view.
%
%  The format of the ClonePixelView method is:
%
%      PixelView *ClonePixelView(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport PixelView *ClonePixelView(const PixelView *pixel_view)
{
  PixelView
    *clone_view;

  register ssize_t
    i;

  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  if (pixel_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",pixel_view->name);
  clone_view=(PixelView *) AcquireMagickMemory(sizeof(*clone_view));
  if (clone_view == (PixelView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      pixel_view->name);
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->id=AcquireWandId();
  (void) FormatLocaleString(clone_view->name,MaxTextExtent,"%s-%.20g",
    PixelViewId,(double) clone_view->id);
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
%   D e s t r o y P i x e l V i e w                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelView() deallocates memory associated with a pixel view.
%
%  The format of the DestroyPixelView method is:
%
%      PixelView *DestroyPixelView(PixelView *pixel_view,
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
  pixel_wands=(PixelWand ***) RelinquishMagickMemory(pixel_wands);
  return(pixel_wands);
}

WandExport PixelView *DestroyPixelView(PixelView *pixel_view)
{
  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  pixel_view->pixel_wands=DestroyPixelsThreadSet(pixel_view->pixel_wands,
    pixel_view->region.width,pixel_view->number_threads);
  pixel_view->view=DestroyCacheView(pixel_view->view);
  pixel_view->exception=DestroyExceptionInfo(pixel_view->exception);
  pixel_view->signature=(~WandSignature);
  RelinquishWandId(pixel_view->id);
  pixel_view=(PixelView *) RelinquishMagickMemory(pixel_view);
  return(pixel_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D u p l e x T r a n s f e r P i x e l V i e w I t e r a t o r             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DuplexTransferPixelViewIterator() iterates over three pixel views in
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
%  The format of the DuplexTransferPixelViewIterator method is:
%
%      MagickBooleanType DuplexTransferPixelViewIterator(PixelView *source,
%        PixelView *duplex,PixelView *destination,
%        DuplexTransferPixelViewMethod transfer,void *context)
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
WandExport MagickBooleanType DuplexTransferPixelViewIterator(
  PixelView *source,PixelView *duplex,PixelView *destination,
  DuplexTransferPixelViewMethod transfer,void *context)
{
#define DuplexTransferPixelViewTag  "PixelView/DuplexTransfer"

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

  assert(source != (PixelView *) NULL);
  assert(source->signature == WandSignature);
  if (transfer == (DuplexTransferPixelViewMethod) NULL)
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
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
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
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],
         GetPixelIndex(indexes+x));
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
        PixelSetBlackQuantum(duplex->pixel_wands[id][x],
          GetPixelIndex(duplex_indexes+x));
    if (duplex_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) duplex->region.width; x++)
        PixelSetIndex(duplex->pixel_wands[id][x],
          GetPixelIndex(duplex_indexes+x));
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
          GetPixelIndex(destination_indexes+x));
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],
          GetPixelIndex(destination_indexes+x));
    if (transfer(source,duplex,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->region.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        SetPixelIndex(destination_indexes+x,PixelGetBlackQuantum(
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
  #pragma omp critical (MagickWand_DuplexTransferPixelViewIterator)
#endif
        proceed=SetImageProgress(source_image,DuplexTransferPixelViewTag,
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
%   G e t P i x e l V i e w E x c e p t i o n                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewException() returns the severity, reason, and description of any
%  error that occurs when utilizing a pixel view.
%
%  The format of the GetPixelViewException method is:
%
%      char *GetPixelViewException(const PixelWand *pixel_view,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel pixel_view.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *GetPixelViewException(const PixelView *pixel_view,
  ExceptionType *severity)
{
  char
    *description;

  assert(pixel_view != (const PixelView *) NULL);
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
%   G e t P i x e l V i e w H e i g h t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewHeight() returns the pixel view height.
%
%  The format of the GetPixelViewHeight method is:
%
%      size_t GetPixelViewHeight(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport size_t GetPixelViewHeight(const PixelView *pixel_view)
{
  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.height);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l V i e w I t e r a t o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewIterator() iterates over the pixel view in parallel and calls
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
%  The format of the GetPixelViewIterator method is:
%
%      MagickBooleanType GetPixelViewIterator(PixelView *source,
%        GetPixelViewMethod get,void *context)
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
WandExport MagickBooleanType GetPixelViewIterator(PixelView *source,
  GetPixelViewMethod get,void *context)
{
#define GetPixelViewTag  "PixelView/Get"

  Image
    *source_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(source != (PixelView *) NULL);
  assert(source->signature == WandSignature);
  if (get == (GetPixelViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
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
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (get(source,context) == MagickFalse)
      status=MagickFalse;
    if (source_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickWand_GetPixelViewIterator)
#endif
        proceed=SetImageProgress(source_image,GetPixelViewTag,progress++,
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
%   G e t P i x e l V i e w P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewPixels() returns the pixel view pixel_wands.
%
%  The format of the GetPixelViewPixels method is:
%
%      PixelWand *GetPixelViewPixels(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport PixelWand **GetPixelViewPixels(const PixelView *pixel_view)
{
  const int
    id = GetOpenMPThreadId();

  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->pixel_wands[id]);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l V i e w W a n d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewWand() returns the magick wand associated with the pixel view.
%
%  The format of the GetPixelViewWand method is:
%
%      MagickWand *GetPixelViewWand(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport MagickWand *GetPixelViewWand(const PixelView *pixel_view)
{
  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l V i e w W i d t h                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewWidth() returns the pixel view width.
%
%  The format of the GetPixelViewWidth method is:
%
%      size_t GetPixelViewWidth(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport size_t GetPixelViewWidth(const PixelView *pixel_view)
{
  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.width);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l V i e w X                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewX() returns the pixel view x offset.
%
%  The format of the GetPixelViewX method is:
%
%      ssize_t GetPixelViewX(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport ssize_t GetPixelViewX(const PixelView *pixel_view)
{
  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.x);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l V i e w Y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixelViewY() returns the pixel view y offset.
%
%  The format of the GetPixelViewY method is:
%
%      ssize_t GetPixelViewY(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport ssize_t GetPixelViewY(const PixelView *pixel_view)
{
  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  return(pixel_view->region.y);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P i x e l V i e w                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPixelView() returns MagickTrue if the the parameter is verified as a pixel
%  view container.
%
%  The format of the IsPixelView method is:
%
%      MagickBooleanType IsPixelView(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport MagickBooleanType IsPixelView(const PixelView *pixel_view)
{
  size_t
    length;

  if (pixel_view == (const PixelView *) NULL)
    return(MagickFalse);
  if (pixel_view->signature != WandSignature)
    return(MagickFalse);
  length=strlen(PixelViewId);
  if (LocaleNCompare(pixel_view->name,PixelViewId,length) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C l i p P a t h I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickClipPathImage() clips along the named paths from the 8BIM profile, if
%  present. Later operations take effect inside the path.  Id may be a number
%  if preceded with #, to work on a numbered path, e.g., "#1" to use the first
%  path.
%
%  The format of the MagickClipPathImage method is:
%
%      MagickBooleanType MagickClipPathImage(MagickWand *wand,
%        const char *pathname,const MagickBooleanType inside)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o pathname: name of clipping path resource. If name is preceded by #, use
%      clipping path numbered by name.
%
%    o inside: if non-zero, later operations take effect inside clipping path.
%      Otherwise later operations take effect outside clipping path.
%
*/
WandExport MagickBooleanType MagickClipPathImage(MagickWand *wand,
  const char *pathname,const MagickBooleanType inside)
{
  return(MagickClipImagePath(wand,pathname,inside));
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w G e t F i l l A l p h a                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawGetFillAlpha() returns the alpha used when drawing using the fill
%  color or fill texture.  Fully opaque is 1.0.
%
%  The format of the DrawGetFillAlpha method is:
%
%      double DrawGetFillAlpha(const DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport double DrawGetFillAlpha(const DrawingWand *wand)
{
  return(DrawGetFillOpacity(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w G e t S t r o k e A l p h a                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawGetStrokeAlpha() returns the alpha of stroked object outlines.
%
%  The format of the DrawGetStrokeAlpha method is:
%
%      double DrawGetStrokeAlpha(const DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
*/
WandExport double DrawGetStrokeAlpha(const DrawingWand *wand)
{
  return(DrawGetStrokeOpacity(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w P e e k G r a p h i c W a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawPeekGraphicWand() returns the current drawing wand.
%
%  The format of the PeekDrawingWand method is:
%
%      DrawInfo *DrawPeekGraphicWand(const DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport DrawInfo *DrawPeekGraphicWand(const DrawingWand *wand)
{
  return(PeekDrawingWand(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w P o p G r a p h i c C o n t e x t                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawPopGraphicContext() destroys the current drawing wand and returns to the
%  previously pushed drawing wand. Multiple drawing wands may exist. It is an
%  error to attempt to pop more drawing wands than have been pushed, and it is
%  proper form to pop all drawing wands which have been pushed.
%
%  The format of the DrawPopGraphicContext method is:
%
%      MagickBooleanType DrawPopGraphicContext(DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport void DrawPopGraphicContext(DrawingWand *wand)
{
  (void) PopDrawingWand(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w P u s h G r a p h i c C o n t e x t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawPushGraphicContext() clones the current drawing wand to create a new
%  drawing wand.  The original drawing wand(s) may be returned to by
%  invoking PopDrawingWand().  The drawing wands are stored on a drawing wand
%  stack.  For every Pop there must have already been an equivalent Push.
%
%  The format of the DrawPushGraphicContext method is:
%
%      MagickBooleanType DrawPushGraphicContext(DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport void DrawPushGraphicContext(DrawingWand *wand)
{
  (void) PushDrawingWand(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w S e t F i l l A l p h a                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawSetFillAlpha() sets the alpha to use when drawing using the fill
%  color or fill texture.  Fully opaque is 1.0.
%
%  The format of the DrawSetFillAlpha method is:
%
%      void DrawSetFillAlpha(DrawingWand *wand,const double fill_alpha)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
%    o fill_alpha: fill alpha
%
*/
WandExport void DrawSetFillAlpha(DrawingWand *wand,const double fill_alpha)
{
  DrawSetFillOpacity(wand,fill_alpha);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w S e t S t r o k e A l p h a                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawSetStrokeAlpha() specifies the alpha of stroked object outlines.
%
%  The format of the DrawSetStrokeAlpha method is:
%
%      void DrawSetStrokeAlpha(DrawingWand *wand,const double stroke_alpha)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
%    o stroke_alpha: stroke alpha.  The value 1.0 is opaque.
%
*/
WandExport void DrawSetStrokeAlpha(DrawingWand *wand,const double stroke_alpha)
{
  DrawSetStrokeOpacity(wand,stroke_alpha);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o l o r F l o o d f i l l I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickColorFloodfillImage() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  The format of the MagickColorFloodfillImage method is:
%
%      MagickBooleanType MagickColorFloodfillImage(MagickWand *wand,
%        const PixelWand *fill,const double fuzz,const PixelWand *bordercolor,
%        const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o fill: the floodfill color pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
%    o bordercolor: the border color pixel wand.
%
%    o x,y: the starting location of the operation.
%
*/
WandExport MagickBooleanType MagickColorFloodfillImage(MagickWand *wand,
  const PixelWand *fill,const double fuzz,const PixelWand *bordercolor,
  const ssize_t x,const ssize_t y)
{
  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  PixelPacket
    target;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=CloneDrawInfo(wand->image_info,(DrawInfo *) NULL);
  PixelGetQuantumColor(fill,&draw_info->fill);
  (void) GetOneVirtualPixel(wand->images,x % wand->images->columns,
    y % wand->images->rows,&target,wand->exception);
  if (bordercolor != (PixelWand *) NULL)
    PixelGetQuantumColor(bordercolor,&target);
  wand->images->fuzz=fuzz;
  status=ColorFloodfillImage(wand->images,draw_info,target,x,y,
    bordercolor != (PixelWand *) NULL ? FillToBorderMethod : FloodfillMethod);
  if (status == MagickFalse)
    InheritException(wand->exception,&wand->images->exception);
  draw_info=DestroyDrawInfo(draw_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e s c r i b e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDescribeImage() identifies an image by printing its attributes to the
%  file.  Attributes include the image width, height, size, and others.
%
%  The format of the MagickDescribeImage method is:
%
%      const char *MagickDescribeImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport char *MagickDescribeImage(MagickWand *wand)
{
  return(MagickIdentifyImage(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F l a t t e n I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFlattenImages() merges a sequence of images.  This useful for
%  combining Photoshop layers into a single image.
%
%  The format of the MagickFlattenImages method is:
%
%      MagickWand *MagickFlattenImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickFlattenImages(MagickWand *wand)
{
  Image
    *flatten_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  flatten_image=FlattenImages(wand->images,wand->exception);
  if (flatten_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,flatten_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e A t t r i b u t e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageAttribute() returns a value associated with the specified
%  property.  Use MagickRelinquishMemory() to free the value when you are
%  finished with it.
%
%  The format of the MagickGetImageAttribute method is:
%
%      char *MagickGetImageAttribute(MagickWand *wand,const char *property)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o property: the property.
%
*/
WandExport char *MagickGetImageAttribute(MagickWand *wand,const char *property)
{
  return(MagickGetImageProperty(wand,property));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k G e t I m a g e I n d e x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageIndex() returns the index of the current image.
%
%  The format of the MagickGetImageIndex method is:
%
%      ssize_t MagickGetImageIndex(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ssize_t MagickGetImageIndex(MagickWand *wand)
{
  return(MagickGetIteratorIndex(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k G e t I m a g e C h a n n e l E x t r e m a                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageChannelExtrema() gets the extrema for one or more image
%  channels.
%
%  The format of the MagickGetImageChannelExtrema method is:
%
%      MagickBooleanType MagickGetImageChannelExtrema(MagickWand *wand,
%        const ChannelType channel,size_t *minima,size_t *maxima)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the image channel(s).
%
%    o minima:  The minimum pixel value for the specified channel(s).
%
%    o maxima:  The maximum pixel value for the specified channel(s).
%
*/
WandExport MagickBooleanType MagickGetImageChannelExtrema(MagickWand *wand,
  const ChannelType channel,size_t *minima,size_t *maxima)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageChannelExtrema(wand->images,channel,minima,maxima,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k G e t I m a g e E x t r e m a                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageExtrema() gets the extrema for the image.
%
%  The format of the MagickGetImageExtrema method is:
%
%      MagickBooleanType MagickGetImageExtrema(MagickWand *wand,
%        size_t *minima,size_t *maxima)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o minima:  The minimum pixel value for the specified channel(s).
%
%    o maxima:  The maximum pixel value for the specified channel(s).
%
*/
WandExport MagickBooleanType MagickGetImageExtrema(MagickWand *wand,
  size_t *minima,size_t *maxima)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageExtrema(wand->images,minima,maxima,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e M a t t e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageMatte() returns MagickTrue if the image has a matte channel
%  otherwise MagickFalse.
%
%  The format of the MagickGetImageMatte method is:
%
%      size_t MagickGetImageMatte(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickGetImageMatte(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->matte);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImagePixels() extracts pixel data from an image and returns it to
%  you.  The method returns MagickTrue on success otherwise MagickFalse if an
%  error is encountered.  The data is returned as char, short int, int, ssize_t,
%  float, or double in the order specified by map.
%
%  Suppose you want to extract the first scanline of a 640x480 image as
%  character data in red-green-blue order:
%
%      MagickGetImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the MagickGetImagePixels method is:
%
%      MagickBooleanType MagickGetImagePixels(MagickWand *wand,
%        const ssize_t x,const ssize_t y,const size_t columns,
%        const size_t rows,const char *map,const StorageType storage,
%        void *pixels)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x, y, columns, rows:  These values define the perimeter
%      of a region of pixels you want to extract.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan,
%      Y = yellow, M = magenta, K = black, I = intensity (for grayscale),
%      P = pad.
%
%    o storage: Define the data type of the pixels.  Float and double types are
%      expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from
%      these types: CharPixel, DoublePixel, FloatPixel, IntegerPixel,
%      LongPixel, QuantumPixel, or ShortPixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
*/
WandExport MagickBooleanType MagickGetImagePixels(MagickWand *wand,
  const ssize_t x,const ssize_t y,const size_t columns,
  const size_t rows,const char *map,const StorageType storage,
  void *pixels)
{
  return(MagickExportImagePixels(wand,x,y,columns,rows,map,storage,pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e S i z e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageSize() returns the image length in bytes.
%
%  The format of the MagickGetImageSize method is:
%
%      MagickBooleanType MagickGetImageSize(MagickWand *wand,
%        MagickSizeType *length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o length: the image length in bytes.
%
*/
WandExport MagickSizeType MagickGetImageSize(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(GetBlobSize(wand->images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M a p I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMapImage() replaces the colors of an image with the closest color
%  from a reference image.
%
%  The format of the MagickMapImage method is:
%
%      MagickBooleanType MagickMapImage(MagickWand *wand,
%        const MagickWand *map_wand,const MagickBooleanType dither)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o map: the map wand.
%
%    o dither: Set this integer value to something other than zero to dither
%      the mapped image.
%
*/
WandExport MagickBooleanType MagickMapImage(MagickWand *wand,
  const MagickWand *map_wand,const MagickBooleanType dither)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) || (map_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=MapImage(wand->images,map_wand->images,dither);
  if (status == MagickFalse)
    InheritException(wand->exception,&wand->images->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M a t t e F l o o d f i l l I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMatteFloodfillImage() changes the transparency value of any pixel that
%  matches target and is an immediate neighbor.  If the method
%  FillToBorderMethod is specified, the transparency value is changed for any
%  neighbor pixel that does not match the bordercolor member of image.
%
%  The format of the MagickMatteFloodfillImage method is:
%
%      MagickBooleanType MagickMatteFloodfillImage(MagickWand *wand,
%        const double alpha,const double fuzz,const PixelWand *bordercolor,
%        const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
%    o bordercolor: the border color pixel wand.
%
%    o x,y: the starting location of the operation.
%
*/
WandExport MagickBooleanType MagickMatteFloodfillImage(MagickWand *wand,
  const double alpha,const double fuzz,const PixelWand *bordercolor,
  const ssize_t x,const ssize_t y)
{
  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  PixelPacket
    target;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=CloneDrawInfo(wand->image_info,(DrawInfo *) NULL);
  (void) GetOneVirtualPixel(wand->images,x % wand->images->columns,
    y % wand->images->rows,&target,wand->exception);
  if (bordercolor != (PixelWand *) NULL)
    PixelGetQuantumColor(bordercolor,&target);
  wand->images->fuzz=fuzz;
  status=MatteFloodfillImage(wand->images,target,ClampToQuantum(
    (MagickRealType) QuantumRange-QuantumRange*alpha),x,y,bordercolor !=
    (PixelWand *) NULL ? FillToBorderMethod : FloodfillMethod);
  if (status == MagickFalse)
    InheritException(wand->exception,&wand->images->exception);
  draw_info=DestroyDrawInfo(draw_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M e d i a n F i l t e r I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMedianFilterImage() applies a digital filter that improves the quality
%  of a noisy image.  Each pixel is replaced by the median in a set of
%  neighboring pixels as defined by radius.
%
%  The format of the MagickMedianFilterImage method is:
%
%      MagickBooleanType MagickMedianFilterImage(MagickWand *wand,
%        const double radius)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the pixel neighborhood.
%
*/
WandExport MagickBooleanType MagickMedianFilterImage(MagickWand *wand,
  const double radius)
{
  Image
    *median_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  median_image=MedianFilterImage(wand->images,radius,wand->exception);
  if (median_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,median_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M i n i m u m I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMinimumImages() returns the minimum intensity of an image sequence.
%
%  The format of the MagickMinimumImages method is:
%
%      MagickWand *MagickMinimumImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickMinimumImages(MagickWand *wand)
{
  Image
    *minimum_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  minimum_image=EvaluateImages(wand->images,MinEvaluateOperator,
    wand->exception);
  if (minimum_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,minimum_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o d e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickModeImage() makes each pixel the 'predominant color' of the
%  neighborhood of the specified radius.
%
%  The format of the MagickModeImage method is:
%
%      MagickBooleanType MagickModeImage(MagickWand *wand,
%        const double radius)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the pixel neighborhood.
%
*/
WandExport MagickBooleanType MagickModeImage(MagickWand *wand,
  const double radius)
{
  Image
    *mode_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  mode_image=ModeImage(wand->images,radius,wand->exception);
  if (mode_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,mode_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o s a i c I m a g e s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMosaicImages() inlays an image sequence to form a single coherent
%  picture.  It returns a wand with each image in the sequence composited at
%  the location defined by the page offset of the image.
%
%  The format of the MagickMosaicImages method is:
%
%      MagickWand *MagickMosaicImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickMosaicImages(MagickWand *wand)
{
  Image
    *mosaic_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  mosaic_image=MosaicImages(wand->images,wand->exception);
  if (mosaic_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,mosaic_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k O p a q u e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOpaqueImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  The format of the MagickOpaqueImage method is:
%
%      MagickBooleanType MagickOpaqueImage(MagickWand *wand,
%        const PixelWand *target,const PixelWand *fill,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel(s).
%
%    o target: Change this target color to the fill color within the image.
%
%    o fill: the fill pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/
WandExport MagickBooleanType MagickOpaqueImage(MagickWand *wand,
  const PixelWand *target,const PixelWand *fill,const double fuzz)
{
  return(MagickPaintOpaqueImage(wand,target,fill,fuzz));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P a i n t F l o o d f i l l I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPaintFloodfillImage() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  The format of the MagickPaintFloodfillImage method is:
%
%      MagickBooleanType MagickPaintFloodfillImage(MagickWand *wand,
%        const ChannelType channel,const PixelWand *fill,const double fuzz,
%        const PixelWand *bordercolor,const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel(s).
%
%    o fill: the floodfill color pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
%    o bordercolor: the border color pixel wand.
%
%    o x,y: the starting location of the operation.
%
*/
WandExport MagickBooleanType MagickPaintFloodfillImage(MagickWand *wand,
  const ChannelType channel,const PixelWand *fill,const double fuzz,
  const PixelWand *bordercolor,const ssize_t x,const ssize_t y)
{
  MagickBooleanType
    status;

  status=MagickFloodfillPaintImage(wand,channel,fill,fuzz,bordercolor,x,y,
    MagickFalse);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P a i n t O p a q u e I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPaintOpaqueImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  The format of the MagickPaintOpaqueImage method is:
%
%      MagickBooleanType MagickPaintOpaqueImage(MagickWand *wand,
%        const PixelWand *target,const PixelWand *fill,const double fuzz)
%      MagickBooleanType MagickPaintOpaqueImageChannel(MagickWand *wand,
%        const ChannelType channel,const PixelWand *target,
%        const PixelWand *fill,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel(s).
%
%    o target: Change this target color to the fill color within the image.
%
%    o fill: the fill pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/

WandExport MagickBooleanType MagickPaintOpaqueImage(MagickWand *wand,
  const PixelWand *target,const PixelWand *fill,const double fuzz)
{
  return(MagickPaintOpaqueImageChannel(wand,DefaultChannels,target,fill,fuzz));
}

WandExport MagickBooleanType MagickPaintOpaqueImageChannel(MagickWand *wand,
  const ChannelType channel,const PixelWand *target,const PixelWand *fill,
  const double fuzz)
{
  MagickBooleanType
    status;

  status=MagickOpaquePaintImageChannel(wand,channel,target,fill,fuzz,
    MagickFalse);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P a i n t T r a n s p a r e n t I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPaintTransparentImage() changes any pixel that matches color with the
%  color defined by fill.
%
%  The format of the MagickPaintTransparentImage method is:
%
%      MagickBooleanType MagickPaintTransparentImage(MagickWand *wand,
%        const PixelWand *target,const double alpha,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o target: Change this target color to specified opacity value within
%      the image.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/
WandExport MagickBooleanType MagickPaintTransparentImage(MagickWand *wand,
  const PixelWand *target,const double alpha,const double fuzz)
{
  return(MagickTransparentPaintImage(wand,target,alpha,fuzz,MagickFalse));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e c o l o r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRecolorImage() apply color transformation to an image. The method
%  permits saturation changes, hue rotation, luminance to alpha, and various
%  other effects.  Although variable-sized transformation matrices can be used,
%  typically one uses a 5x5 matrix for an RGBA image and a 6x6 for CMYKA
%  (or RGBA with offsets).  The matrix is similar to those used by Adobe Flash
%  except offsets are in column 6 rather than 5 (in support of CMYKA images)
%  and offsets are normalized (divide Flash offset by 255).
%
%  The format of the MagickRecolorImage method is:
%
%      MagickBooleanType MagickRecolorImage(MagickWand *wand,
%        const size_t order,const double *color_matrix)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o order: the number of columns and rows in the color matrix.
%
%    o color_matrix: An array of doubles representing the color matrix.
%
*/
WandExport MagickBooleanType MagickRecolorImage(MagickWand *wand,
  const size_t order,const double *color_matrix)
{
  Image
    *transform_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (color_matrix == (const double *) NULL)
    return(MagickFalse);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  transform_image=RecolorImage(wand->images,order,color_matrix,
    wand->exception);
  if (transform_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,transform_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k R e d u c e N o i s e I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickReduceNoiseImage() smooths the contours of an image while still
%  preserving edge information.  The algorithm works by replacing each pixel
%  with its neighbor closest in value.  A neighbor is defined by radius.  Use
%  a radius of 0 and ReduceNoise() selects a suitable radius for you.
%
%  The format of the MagickReduceNoiseImage method is:
%
%      MagickBooleanType MagickReduceNoiseImage(MagickWand *wand,
%        const double radius)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the pixel neighborhood.
%
*/
WandExport MagickBooleanType MagickReduceNoiseImage(MagickWand *wand,
  const double radius)
{
  Image
    *noise_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  noise_image=ReduceNoiseImage(wand->images,radius,wand->exception);
  if (noise_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,noise_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M a x i m u m I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMaximumImages() returns the maximum intensity of an image sequence.
%
%  The format of the MagickMaximumImages method is:
%
%      MagickWand *MagickMaximumImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickMaximumImages(MagickWand *wand)
{
  Image
    *maximum_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  maximum_image=EvaluateImages(wand->images,MaxEvaluateOperator,
    wand->exception);
  if (maximum_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,maximum_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e A t t r i b u t e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageAttribute() associates a property with an image.
%
%  The format of the MagickSetImageAttribute method is:
%
%      MagickBooleanType MagickSetImageAttribute(MagickWand *wand,
%        const char *property,const char *value)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o property: the property.
%
%    o value: the value.
%
*/
WandExport MagickBooleanType MagickSetImageAttribute(MagickWand *wand,
  const char *property,const char *value)
{
  return(SetImageProperty(wand->images,property,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e I n d e x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageIndex() set the current image to the position of the list
%  specified with the index parameter.
%
%  The format of the MagickSetImageIndex method is:
%
%      MagickBooleanType MagickSetImageIndex(MagickWand *wand,
%        const ssize_t index)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o index: the scene number.
%
*/
WandExport MagickBooleanType MagickSetImageIndex(MagickWand *wand,
  const ssize_t index)
{
  return(MagickSetIteratorIndex(wand,index));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k S e t I m a g e O p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageOption() associates one or options with a particular image
%  format (.e.g MagickSetImageOption(wand,"jpeg","perserve","yes").
%
%  The format of the MagickSetImageOption method is:
%
%      MagickBooleanType MagickSetImageOption(MagickWand *wand,
%        const char *format,const char *key,const char *value)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o format: the image format.
%
%    o key:  The key.
%
%    o value:  The value.
%
*/
WandExport MagickBooleanType MagickSetImageOption(MagickWand *wand,
  const char *format,const char *key,const char *value)
{
  char
    option[MaxTextExtent];

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  (void) FormatLocaleString(option,MaxTextExtent,"%s:%s=%s",format,key,value);
  return(DefineImageOption(wand->image_info,option));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T r a n s p a r e n t I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTransparentImage() changes any pixel that matches color with the
%  color defined by fill.
%
%  The format of the MagickTransparentImage method is:
%
%      MagickBooleanType MagickTransparentImage(MagickWand *wand,
%        const PixelWand *target,const double alpha,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o target: Change this target color to specified opacity value within
%      the image.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/
WandExport MagickBooleanType MagickTransparentImage(MagickWand *wand,
  const PixelWand *target,const double alpha,const double fuzz)
{
  return(MagickPaintTransparentImage(wand,target,alpha,fuzz));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e g i o n O f I n t e r e s t I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRegionOfInterestImage() extracts a region of the image and returns it
%  as a new wand.
%
%  The format of the MagickRegionOfInterestImage method is:
%
%      MagickWand *MagickRegionOfInterestImage(MagickWand *wand,
%        const size_t width,const size_t height,const ssize_t x,
%        const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width: the region width.
%
%    o height: the region height.
%
%    o x: the region x offset.
%
%    o y: the region y offset.
%
*/
WandExport MagickWand *MagickRegionOfInterestImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
{
  return(MagickGetImageRegion(wand,width,height,x,y));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImagePixels() accepts pixel datand stores it in the image at the
%  location you specify.  The method returns MagickFalse on success otherwise
%  MagickTrue if an error is encountered.  The pixel data can be either char,
%  short int, int, ssize_t, float, or double in the order specified by map.
%
%  Suppose your want to upload the first scanline of a 640x480 image from
%  character data in red-green-blue order:
%
%      MagickSetImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the MagickSetImagePixels method is:
%
%      MagickBooleanType MagickSetImagePixels(MagickWand *wand,
%        const ssize_t x,const ssize_t y,const size_t columns,
%        const size_t rows,const char *map,const StorageType storage,
%        const void *pixels)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x, y, columns, rows:  These values define the perimeter of a region
%      of pixels you want to define.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan,
%      Y = yellow, M = magenta, K = black, I = intensity (for grayscale),
%      P = pad.
%
%    o storage: Define the data type of the pixels.  Float and double types are
%      expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from
%      these types: CharPixel, ShortPixel, IntegerPixel, LongPixel, FloatPixel,
%      or DoublePixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
*/
WandExport MagickBooleanType MagickSetImagePixels(MagickWand *wand,
  const ssize_t x,const ssize_t y,const size_t columns,
  const size_t rows,const char *map,const StorageType storage,
  const void *pixels)
{
  return(MagickImportImagePixels(wand,x,y,columns,rows,map,storage,pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W r i t e I m a g e B l o b                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWriteImageBlob() implements direct to memory image formats.  It
%  returns the image as a blob and its length.   Use MagickSetFormat() to
%  set the format of the returned blob (GIF, JPEG,  PNG, etc.).
%
%  Use MagickRelinquishMemory() to free the blob when you are done with it.
%
%  The format of the MagickWriteImageBlob method is:
%
%      unsigned char *MagickWriteImageBlob(MagickWand *wand,size_t *length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o length: the length of the blob.
%
*/
WandExport unsigned char *MagickWriteImageBlob(MagickWand *wand,size_t *length)
{
  return(MagickGetImageBlob(wand,length));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w P i x e l V i e w                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewPixelView() returns a pixel view required for all other methods in the
%  Pixel View API.
%
%  The format of the NewPixelView method is:
%
%      PixelView *NewPixelView(MagickWand *wand)
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

WandExport PixelView *NewPixelView(MagickWand *wand)
{
  PixelView
    *pixel_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickSignature);
  pixel_view=(PixelView *) AcquireMagickMemory(sizeof(*pixel_view));
  if (pixel_view == (PixelView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->id=AcquireWandId();
  (void) FormatLocaleString(pixel_view->name,MaxTextExtent,"%s-%.20g",
    PixelViewId,(double) pixel_view->id);
  pixel_view->exception=AcquireExceptionInfo();
  pixel_view->wand=wand;
  pixel_view->view=AcquireVirtualCacheView(pixel_view->wand->images,
    pixel_view->exception);
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
%   N e w P i x e l V i e w R e g i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewPixelViewRegion() returns a pixel view required for all other methods
%  in the Pixel View API.
%
%  The format of the NewPixelViewRegion method is:
%
%      PixelView *NewPixelViewRegion(MagickWand *wand,const ssize_t x,
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
WandExport PixelView *NewPixelViewRegion(MagickWand *wand,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height)
{
  PixelView
    *pixel_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickSignature);
  pixel_view=(PixelView *) AcquireMagickMemory(sizeof(*pixel_view));
  if (pixel_view == (PixelView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->id=AcquireWandId();
  (void) FormatLocaleString(pixel_view->name,MaxTextExtent,"%s-%.20g",
    PixelViewId,(double) pixel_view->id);
  pixel_view->exception=AcquireExceptionInfo();
  pixel_view->view=AcquireVirtualCacheView(pixel_view->wand->images,
    pixel_view->exception);
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
%   P i x e l G e t N e x t R o w                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetNextRow() returns the next row as an array of pixel wands from the
%  pixel iterator.
%
%  The format of the PixelGetNextRow method is:
%
%      PixelWand **PixelGetNextRow(PixelIterator *iterator,
%        size_t *number_wands)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o number_wands: the number of pixel wands.
%
*/
WandExport PixelWand **PixelGetNextRow(PixelIterator *iterator)
{
  size_t
    number_wands;

  return(PixelGetNextIteratorRow(iterator,&number_wands));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l I t e r a t o r G e t E x c e p t i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelIteratorGetException() returns the severity, reason, and description of
%  any error that occurs when using other methods in this API.
%
%  The format of the PixelIteratorGetException method is:
%
%      char *PixelIteratorGetException(const Pixeliterator *iterator,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *PixelIteratorGetException(const PixelIterator *iterator,
  ExceptionType *severity)
{
  return(PixelGetIteratorException(iterator,severity));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t P i x e l V i e w I t e r a t o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetPixelViewIterator() iterates over the pixel view in parallel and calls
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
%  The format of the SetPixelViewIterator method is:
%
%      MagickBooleanType SetPixelViewIterator(PixelView *destination,
%        SetPixelViewMethod set,void *context)
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
WandExport MagickBooleanType SetPixelViewIterator(PixelView *destination,
  SetPixelViewMethod set,void *context)
{
#define SetPixelViewTag  "PixelView/Set"

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

  assert(destination != (PixelView *) NULL);
  assert(destination->signature == WandSignature);
  if (set == (SetPixelViewMethod) NULL)
    return(MagickFalse);
  destination_image=destination->wand->images;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=destination->region.y; y < (ssize_t) destination->region.height; y++)
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
        SetPixelIndex(indexes+x,PixelGetBlackQuantum(
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
  #pragma omp critical (MagickWand_SetPixelViewIterator)
#endif
        proceed=SetImageProgress(destination_image,SetPixelViewTag,progress++,
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
%   T r a n s f e r P i x e l V i e w I t e r a t o r                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransferPixelViewIterator() iterates over two pixel views in parallel and
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
%  The format of the TransferPixelViewIterator method is:
%
%      MagickBooleanType TransferPixelViewIterator(PixelView *source,
%        PixelView *destination,TransferPixelViewMethod transfer,void *context)
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
WandExport MagickBooleanType TransferPixelViewIterator(PixelView *source,
  PixelView *destination,TransferPixelViewMethod transfer,void *context)
{
#define TransferPixelViewTag  "PixelView/Transfer"

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

  assert(source != (PixelView *) NULL);
  assert(source->signature == WandSignature);
  if (transfer == (TransferPixelViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  destination_image=destination->wand->images;
  if (SetImageStorageClass(destination_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=destination->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
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
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
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
        PixelSetBlackQuantum(destination->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (transfer(source,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) destination->region.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) destination->region.width; x++)
        SetPixelIndex(destination_indexes+x,PixelGetBlackQuantum(
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
  #pragma omp critical (MagickWand_TransferPixelViewIterator)
#endif
        proceed=SetImageProgress(source_image,TransferPixelViewTag,progress++,
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
%   U p d a t e P i x e l V i e w I t e r a t o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdatePixelViewIterator() iterates over the pixel view in parallel and calls
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
%  The format of the UpdatePixelViewIterator method is:
%
%      MagickBooleanType UpdatePixelViewIterator(PixelView *source,
%        UpdatePixelViewMethod update,void *context)
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
WandExport MagickBooleanType UpdatePixelViewIterator(PixelView *source,
  UpdatePixelViewMethod update,void *context)
{
#define UpdatePixelViewTag  "PixelView/Update"

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

  assert(source != (PixelView *) NULL);
  assert(source->signature == WandSignature);
  if (update == (UpdatePixelViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  if (SetImageStorageClass(source_image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  exception=source->exception;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status)
#endif
  for (y=source->region.y; y < (ssize_t) source->region.height; y++)
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
        PixelSetBlackQuantum(source->pixel_wands[id][x],
          GetPixelIndex(indexes+x));
    if (update(source,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (ssize_t) source->region.width; x++)
      PixelGetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (ssize_t) source->region.width; x++)
        SetPixelIndex(indexes+x,PixelGetBlackQuantum(
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
  #pragma omp critical (MagickWand_UpdatePixelViewIterator)
#endif
        proceed=SetImageProgress(source_image,UpdatePixelViewTag,progress++,
          source->region.height);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  return(status);
}
#endif
