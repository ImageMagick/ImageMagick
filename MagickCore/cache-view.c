/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      CCCC   AAA    CCCC  H   H  EEEEE                       %
%                     C      A   A  C      H   H  E                           %
%                     C      AAAAA  C      HHHHH  EEE                         %
%                     C      A   A  C      H   H  E                           %
%                      CCCC  A   A   CCCC  H   H  EEEEE                       %
%                                                                             %
%                        V   V  IIIII  EEEEE  W   W                           %
%                        V   V    I    E      W   W                           %
%                        V   V    I    EEE    W W W                           %
%                         V V     I    E      WW WW                           %
%                          V    IIIII  EEEEE  W   W                           %
%                                                                             %
%                                                                             %
%                        MagickCore Cache View Methods                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               February 2000                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"

/*
  Typedef declarations.
*/
struct _CacheView
{
  Image
    *image;

  VirtualPixelMethod
    virtual_pixel_method;

  size_t
    number_threads;

  NexusInfo
    **nexus_info;

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
%   A c q u i r e A u t h e n t i c C a c h e V i e w                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireAuthenticCacheView() acquires an authentic view into the pixel cache.
%  It always succeeds but may return a warning or informational exception.
%
%  The format of the AcquireAuthenticCacheView method is:
%
%      CacheView *AcquireAuthenticCacheView(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport CacheView *AcquireAuthenticCacheView(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *magick_restrict cache_view;

  cache_view=AcquireVirtualCacheView(image,exception);
  return(cache_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e V i r t u a l C a c h e V i e w                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireVirtualCacheView() acquires a virtual view into the pixel cache,
%  using the VirtualPixelMethod that is defined within the given image itself.
%  It always succeeds but may return a warning or informational exception.
%
%  The format of the AcquireVirtualCacheView method is:
%
%      CacheView *AcquireVirtualCacheView(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport CacheView *AcquireVirtualCacheView(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *magick_restrict cache_view;

  magick_unreferenced(exception);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  SyncAuthenticOpenCLBuffer(image);
#endif
  cache_view=(CacheView *) MagickAssumeAligned(AcquireAlignedMemory(1,
    sizeof(*cache_view)));
  if (cache_view == (CacheView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(cache_view,0,sizeof(*cache_view));
  cache_view->image=ReferenceImage((Image *) image);
  cache_view->number_threads=GetOpenMPMaximumThreads();
  if (GetMagickResourceLimit(ThreadResource) > cache_view->number_threads)
    cache_view->number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  if (cache_view->number_threads == 0)
    cache_view->number_threads=1;
  cache_view->nexus_info=AcquirePixelCacheNexus(cache_view->number_threads);
  cache_view->virtual_pixel_method=GetImageVirtualPixelMethod(image);
  cache_view->debug=IsEventLogging();
  cache_view->signature=MagickCoreSignature;
  if (cache_view->nexus_info == (NexusInfo **) NULL)
    ThrowFatalException(CacheFatalError,"UnableToAcquireCacheView");
  return(cache_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e C a c h e V i e w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneCacheView()  makes an exact copy of the specified cache view.
%
%  The format of the CloneCacheView method is:
%
%      CacheView *CloneCacheView(const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport CacheView *CloneCacheView(const CacheView *cache_view)
{
  CacheView
    *magick_restrict clone_view;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  clone_view=(CacheView *) MagickAssumeAligned(AcquireAlignedMemory(1,
    sizeof(*clone_view)));
  if (clone_view == (CacheView *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(clone_view,0,sizeof(*clone_view));
  clone_view->image=ReferenceImage(cache_view->image);
  clone_view->number_threads=cache_view->number_threads;
  clone_view->nexus_info=AcquirePixelCacheNexus(cache_view->number_threads);
  clone_view->virtual_pixel_method=cache_view->virtual_pixel_method;
  clone_view->debug=cache_view->debug;
  clone_view->signature=MagickCoreSignature;
  return(clone_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C a c h e V i e w                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCacheView() destroys the specified view returned by a previous call
%  to AcquireCacheView().
%
%  The format of the DestroyCacheView method is:
%
%      CacheView *DestroyCacheView(CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport CacheView *DestroyCacheView(CacheView *cache_view)
{
  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  if (cache_view->nexus_info != (NexusInfo **) NULL)
    cache_view->nexus_info=DestroyPixelCacheNexus(cache_view->nexus_info,
      cache_view->number_threads);
  cache_view->image=DestroyImage(cache_view->image);
  cache_view->signature=(~MagickCoreSignature);
  cache_view=(CacheView *) RelinquishAlignedMemory(cache_view);
  return(cache_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w A u t h e n t i c P i x e l s                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewAuthenticPixels() gets pixels from the in-memory or disk pixel
%  cache as defined by the geometry parameters.   A pointer to the pixels is
%  returned if the pixels are transferred, otherwise a NULL is returned.
%
%  The format of the GetCacheViewAuthenticPixels method is:
%
%      Quantum *GetCacheViewAuthenticPixels(CacheView *cache_view,
%        const ssize_t x,const ssize_t y,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Quantum *GetCacheViewAuthenticPixels(CacheView *cache_view,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  Quantum
    *magick_restrict pixels;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  pixels=GetAuthenticPixelCacheNexus(cache_view->image,x,y,columns,rows,
    cache_view->nexus_info[id],exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w A u t h e n t i c M e t a c o n t e n t           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewAuthenticMetacontent() returns the meta-content corresponding
%  with the last call to SetCacheViewIndexes() or
%  GetCacheViewAuthenticMetacontent().  The meta-content are authentic and can
%  be updated.
%
%  The format of the GetCacheViewAuthenticMetacontent() method is:
%
%      void *GetCacheViewAuthenticMetacontent(CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport void *GetCacheViewAuthenticMetacontent(CacheView *cache_view)
{
  const int
    id = GetOpenMPThreadId();

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(cache_view->image->cache != (Cache) NULL);
  assert(id < (int) cache_view->number_threads);
  return(cache_view->nexus_info[id]->metacontent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w A u t h e n t i c P i x e l Q u e u e             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewAuthenticPixelQueue() returns the pixels associated with the
%  last call to QueueCacheViewAuthenticPixels() or
%  GetCacheViewAuthenticPixels().  The pixels are authentic and therefore can be
%  updated.
%
%  The format of the GetCacheViewAuthenticPixelQueue() method is:
%
%      Quantum *GetCacheViewAuthenticPixelQueue(CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport Quantum *GetCacheViewAuthenticPixelQueue(CacheView *cache_view)
{
  const int
    id = GetOpenMPThreadId();

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(cache_view->image->cache != (Cache) NULL);
  assert(id < (int) cache_view->number_threads);
  return(cache_view->nexus_info[id]->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w C o l o r s p a c e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewColorspace() returns the image colorspace associated with the
%  specified view.
%
%  The format of the GetCacheViewColorspace method is:
%
%      ColorspaceType GetCacheViewColorspace(const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport ColorspaceType GetCacheViewColorspace(const CacheView *cache_view)
{
  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(GetPixelCacheColorspace(cache_view->image->cache));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C a c h e V i e w E x t e n t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewExtent() returns the extent of the pixels associated with the
%  last call to QueueCacheViewAuthenticPixels() or
%  GetCacheViewAuthenticPixels().
%
%  The format of the GetCacheViewExtent() method is:
%
%      MagickSizeType GetCacheViewExtent(const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport MagickSizeType GetCacheViewExtent(const CacheView *cache_view)
{
  const int
    id = GetOpenMPThreadId();

  MagickSizeType
    extent;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  assert(cache_view->image->cache != (Cache) NULL);
  assert(id < (int) cache_view->number_threads);
  extent=GetPixelCacheNexusExtent(cache_view->image->cache,
    cache_view->nexus_info[id]);
  return(extent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewImage() returns the image associated with the specified view.
%
%  The format of the GetCacheViewImage method is:
%
%      const Image *GetCacheViewImage(const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport const Image *GetCacheViewImage(const CacheView *cache_view)
{
  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(cache_view->image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w S t o r a g e C l a s s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewStorageClass() returns the image storage class associated with
%  the specified view.
%
%  The format of the GetCacheViewStorageClass method is:
%
%      ClassType GetCacheViewStorageClass(const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport ClassType GetCacheViewStorageClass(const CacheView *cache_view)
{
  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(GetPixelCacheStorageClass(cache_view->image->cache));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w V i r t u a l M e t a c o n t e n t               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewVirtualMetacontent() returns the meta-content corresponding
%  with the last call to GetCacheViewVirtualMetacontent().  The meta-content
%  is virtual and therefore cannot be updated.
%
%  The format of the GetCacheViewVirtualMetacontent() method is:
%
%      const void *GetCacheViewVirtualMetacontent(
%        const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport const void *GetCacheViewVirtualMetacontent(
  const CacheView *cache_view)
{
  const int
    id = GetOpenMPThreadId();

  const void
    *magick_restrict metacontent;

  assert(cache_view != (const CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(cache_view->image->cache != (Cache) NULL);
  assert(id < (int) cache_view->number_threads);
  metacontent=GetVirtualMetacontentFromNexus(cache_view->image->cache,
    cache_view->nexus_info[id]);
  return(metacontent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w V i r t u a l P i x e l Q u e u e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewVirtualPixelQueue() returns the pixels associated with
%  the last call to GetCacheViewVirtualPixels().  The pixels are virtual
%  and therefore cannot be updated.
%
%  The format of the GetCacheViewVirtualPixelQueue() method is:
%
%      const Quantum *GetCacheViewVirtualPixelQueue(
%        const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport const Quantum *GetCacheViewVirtualPixelQueue(
  const CacheView *cache_view)
{
  const int
    id = GetOpenMPThreadId();

  const Quantum
    *magick_restrict pixels;

  assert(cache_view != (const CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(cache_view->image->cache != (Cache) NULL);
  assert(id < (int) cache_view->number_threads);
  pixels=GetVirtualPixelsNexus(cache_view->image->cache,
    cache_view->nexus_info[id]);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w V i r t u a l P i x e l s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewVirtualPixels() gets virtual pixels from the in-memory or
%  disk pixel cache as defined by the geometry parameters.   A pointer to the
%  pixels is returned if the pixels are transferred, otherwise a NULL is
%  returned.
%
%  The format of the GetCacheViewVirtualPixels method is:
%
%      const Quantum *GetCacheViewVirtualPixels(
%        const CacheView *cache_view,const ssize_t x,const ssize_t y,
%        const size_t columns,const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const Quantum *GetCacheViewVirtualPixels(
  const CacheView *cache_view,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows,ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  const Quantum
    *magick_restrict pixels;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  pixels=GetVirtualPixelCacheNexus(cache_view->image,
    cache_view->virtual_pixel_method,x,y,columns,rows,
    cache_view->nexus_info[id],exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e C a c h e V i e w A u t h e n t i c P i x e l                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneCacheViewAuthenticPixel() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.
%
%  The format of the GetOneCacheViewAuthenticPixel method is:
%
%      MagickBooleaNType GetOneCacheViewAuthenticPixel(
%        const CacheView *cache_view,const ssize_t x,const ssize_t y,
%        Quantum *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y:  These values define the offset of the pixel.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetOneCacheViewAuthenticPixel(
  const CacheView *cache_view,const ssize_t x,const ssize_t y,Quantum *pixel,
  ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  Quantum
    *magick_restrict q;

  register ssize_t
    i;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  (void) memset(pixel,0,MaxPixelChannels*sizeof(*pixel));
  q=GetAuthenticPixelCacheNexus(cache_view->image,x,y,1,1,
    cache_view->nexus_info[id],exception);
  if (q == (const Quantum *) NULL)
    {
      PixelInfo
        background_color;

      background_color=cache_view->image->background_color;
      pixel[RedPixelChannel]=ClampToQuantum(background_color.red);
      pixel[GreenPixelChannel]=ClampToQuantum(background_color.green);
      pixel[BluePixelChannel]=ClampToQuantum(background_color.blue);
      pixel[BlackPixelChannel]=ClampToQuantum(background_color.black);
      pixel[AlphaPixelChannel]=ClampToQuantum(background_color.alpha);
      return(MagickFalse);
    }
  for (i=0; i < (ssize_t) GetPixelChannels(cache_view->image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(cache_view->image,i);
    pixel[channel]=q[i];
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e C a c h e V i e w V i r t u a l P i x e l                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneCacheViewVirtualPixel() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.  If
%  you plan to modify the pixel, use GetOneCacheViewAuthenticPixel() instead.
%
%  The format of the GetOneCacheViewVirtualPixel method is:
%
%      MagickBooleanType GetOneCacheViewVirtualPixel(
%        const CacheView *cache_view,const ssize_t x,const ssize_t y,
%        Quantum *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y:  These values define the offset of the pixel.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetOneCacheViewVirtualPixel(
  const CacheView *cache_view,const ssize_t x,const ssize_t y,Quantum *pixel,
  ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  register const Quantum
    *magick_restrict p;

  register ssize_t
    i;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  (void) memset(pixel,0,MaxPixelChannels*sizeof(*pixel));
  p=GetVirtualPixelCacheNexus(cache_view->image,
    cache_view->virtual_pixel_method,x,y,1,1,cache_view->nexus_info[id],
    exception);
  if (p == (const Quantum *) NULL)
    {
      PixelInfo
        background_color;

      background_color=cache_view->image->background_color;
      pixel[RedPixelChannel]=ClampToQuantum(background_color.red);
      pixel[GreenPixelChannel]=ClampToQuantum(background_color.green);
      pixel[BluePixelChannel]=ClampToQuantum(background_color.blue);
      pixel[BlackPixelChannel]=ClampToQuantum(background_color.black);
      pixel[AlphaPixelChannel]=ClampToQuantum(background_color.alpha);
      return(MagickFalse);
    }
  for (i=0; i < (ssize_t) GetPixelChannels(cache_view->image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(cache_view->image,i);
    pixel[channel]=p[i];
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e C a c h e V i e w V i r t u a l P i x e l I n f o             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneCacheViewVirtualPixelInfo() returns a single pixel at the specified
%  (x,y) location.  The image background color is returned if an error occurs.
%  If you plan to modify the pixel, use GetOneCacheViewAuthenticPixel() instead.
%
%  The format of the GetOneCacheViewVirtualPixelInfo method is:
%
%      MagickBooleanType GetOneCacheViewVirtualPixelInfo(
%        const CacheView *cache_view,const ssize_t x,const ssize_t y,
%        PixelInfo *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y:  These values define the offset of the pixel.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetOneCacheViewVirtualPixelInfo(
  const CacheView *cache_view,const ssize_t x,const ssize_t y,PixelInfo *pixel,
  ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  register const Quantum
    *magick_restrict p;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  GetPixelInfo(cache_view->image,pixel);
  p=GetVirtualPixelCacheNexus(cache_view->image,
    cache_view->virtual_pixel_method,x,y,1,1,cache_view->nexus_info[id],
    exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  GetPixelInfoPixel(cache_view->image,p,pixel);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e C a c h e V i e w V i r t u a l P i x e l                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneCacheViewVirtualMethodPixel() returns a single virtual pixel at
%  the specified (x,y) location.  The image background color is returned if an
%  error occurs.  If you plan to modify the pixel, use
%  GetOneCacheViewAuthenticPixel() instead.
%
%  The format of the GetOneCacheViewVirtualPixel method is:
%
%      MagickBooleanType GetOneCacheViewVirtualMethodPixel(
%        const CacheView *cache_view,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,Quantum *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y:  These values define the offset of the pixel.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GetOneCacheViewVirtualMethodPixel(
  const CacheView *cache_view,const VirtualPixelMethod virtual_pixel_method,
  const ssize_t x,const ssize_t y,Quantum *pixel,ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  const Quantum
    *magick_restrict p;

  register ssize_t
    i;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  (void) memset(pixel,0,MaxPixelChannels*sizeof(*pixel));
  p=GetVirtualPixelCacheNexus(cache_view->image,virtual_pixel_method,x,y,1,1,
    cache_view->nexus_info[id],exception);
  if (p == (const Quantum *) NULL)
    {
      PixelInfo
        background_color;

      background_color=cache_view->image->background_color;
      pixel[RedPixelChannel]=ClampToQuantum(background_color.red);
      pixel[GreenPixelChannel]=ClampToQuantum(background_color.green);
      pixel[BluePixelChannel]=ClampToQuantum(background_color.blue);
      pixel[BlackPixelChannel]=ClampToQuantum(background_color.black);
      pixel[AlphaPixelChannel]=ClampToQuantum(background_color.alpha);
      return(MagickFalse);
    }
  for (i=0; i < (ssize_t) GetPixelChannels(cache_view->image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(cache_view->image,i);
    pixel[channel]=p[i];
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Q u e u e C a c h e V i e w A u t h e n t i c P i x e l s                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueueCacheViewAuthenticPixels() queues authentic pixels from the in-memory or
%  disk pixel cache as defined by the geometry parameters.   A pointer to the
%  pixels is returned if the pixels are transferred, otherwise a NULL is
%  returned.
%
%  The format of the QueueCacheViewAuthenticPixels method is:
%
%      Quantum *QueueCacheViewAuthenticPixels(CacheView *cache_view,
%        const ssize_t x,const ssize_t y,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Quantum *QueueCacheViewAuthenticPixels(CacheView *cache_view,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  Quantum
    *magick_restrict pixels;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  pixels=QueueAuthenticPixelCacheNexus(cache_view->image,x,y,columns,rows,
    MagickFalse,cache_view->nexus_info[id],exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e V i e w S t o r a g e C l a s s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCacheViewStorageClass() sets the image storage class associated with
%  the specified view.
%
%  The format of the SetCacheViewStorageClass method is:
%
%      MagickBooleanType SetCacheViewStorageClass(CacheView *cache_view,
%        const ClassType storage_class,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o storage_class: the image storage class: PseudoClass or DirectClass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetCacheViewStorageClass(CacheView *cache_view,
  const ClassType storage_class,ExceptionInfo *exception)
{
  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(SetImageStorageClass(cache_view->image,storage_class,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e V i e w V i r t u a l P i x e l M e t h o d               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCacheViewVirtualPixelMethod() sets the virtual pixel method associated
%  with the specified cache view.
%
%  The format of the SetCacheViewVirtualPixelMethod method is:
%
%      MagickBooleanType SetCacheViewVirtualPixelMethod(CacheView *cache_view,
%        const VirtualPixelMethod virtual_pixel_method)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o virtual_pixel_method: the virtual pixel method.
%
*/
MagickExport MagickBooleanType SetCacheViewVirtualPixelMethod(
  CacheView *magick_restrict cache_view,
  const VirtualPixelMethod virtual_pixel_method)
{
  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  cache_view->virtual_pixel_method=virtual_pixel_method;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c C a c h e V i e w A u t h e n t i c P i x e l s                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncCacheViewAuthenticPixels() saves the cache view pixels to the in-memory
%  or disk cache.  It returns MagickTrue if the pixel region is flushed,
%  otherwise MagickFalse.
%
%  The format of the SyncCacheViewAuthenticPixels method is:
%
%      MagickBooleanType SyncCacheViewAuthenticPixels(CacheView *cache_view,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SyncCacheViewAuthenticPixels(
  CacheView *magick_restrict cache_view,ExceptionInfo *exception)
{
  const int
    id = GetOpenMPThreadId();

  MagickBooleanType
    status;

  assert(cache_view != (CacheView *) NULL);
  assert(cache_view->signature == MagickCoreSignature);
  assert(id < (int) cache_view->number_threads);
  status=SyncAuthenticPixelCacheNexus(cache_view->image,
    cache_view->nexus_info[id],exception);
  return(status);
}
