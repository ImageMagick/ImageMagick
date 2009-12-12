/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      PPPP   IIIII  X   X  EEEEE  L                          %
%                      P   P    I     X X   E      L                          %
%                      PPPP     I      X    EEE    L                          %
%                      P        I     X X   E      L                          %
%                      P      IIIII  X   X  EEEEE  LLLLL                      %
%                                                                             %
%                        V   V  IIIII  EEEEE  W   W                           %
%                        V   V    I    E      W   W                           %
%                        V   V    I    EEE    W W W                           %
%                         V V     I    E      WW WW                           %
%                          V    IIIII  EEEEE  W   W                           %
%                                                                             %
%                                                                             %
%                       MagickWand Pixel View Methods                         %
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
#define PixelViewId  "PixelView"

/*
  Typedef declarations.
*/
struct _PixelView
{
  unsigned long
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

  unsigned long
    number_threads;

  PixelWand
    ***pixel_wands;

  MagickBooleanType
    debug;

  unsigned long
    signature;
};

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

  register long
    i;

  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  if (pixel_view->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",pixel_view->name);
  clone_view=(PixelView *) AcquireAlignedMemory(1,sizeof(*clone_view));
  if (clone_view == (PixelView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      pixel_view->name);
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->id=AcquireWandId();
  (void) FormatMagickString(clone_view->name,MaxTextExtent,"%s-%lu",PixelViewId,
    clone_view->id);
  clone_view->exception=AcquireExceptionInfo();
  InheritException(clone_view->exception,pixel_view->exception);
  clone_view->view=CloneCacheView(pixel_view->view);
  clone_view->region=pixel_view->region;
  clone_view->number_threads=pixel_view->number_threads;
  for (i=0; i < (long) pixel_view->number_threads; i++)
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
%        const unsigned long number_wands,const unsigned long number_threads)
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
  const unsigned long number_wands,const unsigned long number_threads)
{
  register long
    i;

  assert(pixel_wands != (PixelWand ***) NULL);
  for (i=0; i < (long) number_threads; i++)
    if (pixel_wands[i] != (PixelWand **) NULL)
      pixel_wands[i]=DestroyPixelWands(pixel_wands[i],number_wands);
  pixel_wands=(PixelWand ***) RelinquishAlignedMemory(pixel_wands);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=source->region.y; y < (long) source->region.height; y++)
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

    register long
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
    for (x=0; x < (long) source->region.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (long) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],indexes[x]);
    duplex_pixels=GetCacheViewVirtualPixels(duplex->view,duplex->region.x,y,
      duplex->region.width,1,duplex->exception);
    if (duplex_pixels == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    duplex_indexes=GetCacheViewVirtualIndexQueue(duplex->view);
    for (x=0; x < (long) duplex->region.width; x++)
      PixelSetQuantumColor(duplex->pixel_wands[id][x],duplex_pixels+x);
    if (duplex_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) duplex->region.width; x++)
        PixelSetBlackQuantum(duplex->pixel_wands[id][x],duplex_indexes[x]);
    if (duplex_image->storage_class == PseudoClass)
      for (x=0; x < (long) duplex->region.width; x++)
        PixelSetIndex(duplex->pixel_wands[id][x],duplex_indexes[x]);
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (long) destination->region.width; x++)
      PixelSetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) destination->region.width; x++)
        PixelSetBlackQuantum(destination->pixel_wands[id][x],
          destination_indexes[x]);
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (long) destination->region.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],destination_indexes[x]);
    if (transfer(source,duplex,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (long) destination->region.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) destination->region.width; x++)
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
%      unsigned long GetPixelViewHeight(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport unsigned long GetPixelViewHeight(const PixelView *pixel_view)
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

  long
    progress,
    y;

  MagickBooleanType
    status;

  assert(source != (PixelView *) NULL);
  assert(source->signature == WandSignature);
  if (get == (GetPixelViewMethod) NULL)
    return(MagickFalse);
  source_image=source->wand->images;
  status=MagickTrue;
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=source->region.y; y < (long) source->region.height; y++)
  {
    register const IndexPacket
      *indexes;

    register const PixelPacket
      *pixels;

    register long
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
    for (x=0; x < (long) source->region.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (long) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],indexes[x]);
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
  long
    id;

  assert(pixel_view != (PixelView *) NULL);
  assert(pixel_view->signature == WandSignature);
  id=GetOpenMPThreadId();
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
%      unsigned long GetPixelViewWidth(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport unsigned long GetPixelViewWidth(const PixelView *pixel_view)
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
%      long GetPixelViewX(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport long GetPixelViewX(const PixelView *pixel_view)
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
%      long GetPixelViewY(const PixelView *pixel_view)
%
%  A description of each parameter follows:
%
%    o pixel_view: the pixel view.
%
*/
WandExport long GetPixelViewY(const PixelView *pixel_view)
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

static PixelWand ***AcquirePixelsThreadSet(const unsigned long number_wands,
  const unsigned long number_threads)
{
  PixelWand
    ***pixel_wands;

  register long
    i;

  pixel_wands=(PixelWand ***) AcquireAlignedMemory(number_threads,
    sizeof(*pixel_wands));
  if (pixel_wands == (PixelWand ***) NULL)
    return((PixelWand ***) NULL);
  (void) ResetMagickMemory(pixel_wands,0,number_threads*sizeof(*pixel_wands));
  for (i=0; i < (long) number_threads; i++)
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
  pixel_view=(PixelView *) AcquireAlignedMemory(1,sizeof(*pixel_view));
  if (pixel_view == (PixelView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->id=AcquireWandId();
  (void) FormatMagickString(pixel_view->name,MaxTextExtent,"%s-%lu",
    PixelViewId,pixel_view->id);
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
%      PixelView *NewPixelViewRegion(MagickWand *wand,const long x,
%        const long y,const unsigned long width,const unsigned long height)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixel_wands view.
%
*/
WandExport PixelView *NewPixelViewRegion(MagickWand *wand,const long x,
  const long y,const unsigned long width,const unsigned long height)
{
  PixelView
    *pixel_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickSignature);
  pixel_view=(PixelView *) AcquireAlignedMemory(1,sizeof(*pixel_view));
  if (pixel_view == (PixelView *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(pixel_view,0,sizeof(*pixel_view));
  pixel_view->id=AcquireWandId();
  (void) FormatMagickString(pixel_view->name,MaxTextExtent,"%s-%lu",
    PixelViewId,pixel_view->id);
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

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=destination->region.y; y < (long) destination->region.height; y++)
  {
    MagickBooleanType
      sync;

    register IndexPacket
      *restrict indexes;

    register long
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
    for (x=0; x < (long) destination->region.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) destination->region.width; x++)
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

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=source->region.y; y < (long) source->region.height; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict pixels;

    register IndexPacket
      *restrict destination_indexes;

    register long
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
    for (x=0; x < (long) source->region.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (source_image->storage_class == PseudoClass)
      for (x=0; x < (long) source->region.width; x++)
        PixelSetIndex(source->pixel_wands[id][x],indexes[x]);
    destination_pixels=GetCacheViewAuthenticPixels(destination->view,
      destination->region.x,y,destination->region.width,1,exception);
    if (destination_pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    destination_indexes=GetCacheViewAuthenticIndexQueue(destination->view);
    for (x=0; x < (long) destination->region.width; x++)
      PixelSetQuantumColor(destination->pixel_wands[id][x],pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) destination->region.width; x++)
        PixelSetBlackQuantum(destination->pixel_wands[id][x],indexes[x]);
    if (destination_image->storage_class == PseudoClass)
      for (x=0; x < (long) destination->region.width; x++)
        PixelSetIndex(destination->pixel_wands[id][x],indexes[x]);
    if (transfer(source,destination,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (long) destination->region.width; x++)
      PixelGetQuantumColor(destination->pixel_wands[id][x],
        destination_pixels+x);
    if (destination_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) destination->region.width; x++)
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

  long
    progress,
    y;

  MagickBooleanType
    status;

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
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=source->region.y; y < (long) source->region.height; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
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
    for (x=0; x < (long) source->region.width; x++)
      PixelSetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) source->region.width; x++)
        PixelSetBlackQuantum(source->pixel_wands[id][x],indexes[x]);
    if (update(source,context) == MagickFalse)
      status=MagickFalse;
    for (x=0; x < (long) source->region.width; x++)
      PixelGetQuantumColor(source->pixel_wands[id][x],pixels+x);
    if (source_image->colorspace == CMYKColorspace)
      for (x=0; x < (long) source->region.width; x++)
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
