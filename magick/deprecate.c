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
%                        MagickCore Deprecated Methods                        %
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
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-view.h"
#include "magick/channel.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colormap-private.h"
#include "magick/colorspace.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/draw.h"
#include "magick/draw-private.h"
#include "magick/effect.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fx.h"
#include "magick/geometry.h"
#include "magick/identify.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/morphology.h"
#include "magick/paint.h"
#include "magick/pixel.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/segment.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/threshold.h"
#include "magick/transform.h"
#include "magick/utility.h"

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)
/*
  Global declarations.
*/
static MonitorHandler
  monitor_handler = (MonitorHandler) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C a c h e V i e w I n d e x e s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheViewIndexes() returns the indexes associated with the specified
%  view.
%
%  Deprecated, replace with:
%
%    GetCacheViewVirtualIndexQueue(cache_view);
%
%  The format of the AcquireCacheViewIndexes method is:
%
%      const IndexPacket *AcquireCacheViewIndexes(const CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport const IndexPacket *AcquireCacheViewIndexes(
  const CacheView *cache_view)
{
  return(GetCacheViewVirtualIndexQueue(cache_view));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C a c h e V i e w P i x e l s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheViewPixels() gets pixels from the in-memory or disk pixel cache
%  as defined by the geometry parameters.   A pointer to the pixels is returned
%  if the pixels are transferred, otherwise a NULL is returned.
%
%  Deprecated, replace with:
%
%    GetCacheViewVirtualPixels(cache_view,x,y,columns,rows,exception);
%
%  The format of the AcquireCacheViewPixels method is:
%
%      const PixelPacket *AcquireCacheViewPixels(const CacheView *cache_view,
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
MagickExport const PixelPacket *AcquireCacheViewPixels(
  const CacheView *cache_view,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows,ExceptionInfo *exception)
{
  return(GetCacheViewVirtualPixels(cache_view,x,y,columns,rows,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I m a g e P i x e l s                                       %
%                                                                             % %                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireImagePixels() returns an immutable pixel region. If the
%  region is successfully accessed, a pointer to it is returned, otherwise
%  NULL is returned. The returned pointer may point to a temporary working
%  copy of the pixels or it may point to the original pixels in memory.
%  Performance is maximized if the selected region is part of one row, or one
%  or more full rows, since there is opportunity to access the pixels in-place
%  (without a copy) if the image is in RAM, or in a memory-mapped file.  The
%  returned pointer should *never* be deallocated by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  PixelPacket.  If the image type is CMYK or the storage class is PseudoClass,
%  call GetAuthenticIndexQueue() after invoking GetAuthenticPixels() to access
%  the black color component or to obtain the colormap indexes (of type
%  IndexPacket) corresponding to the region.
%
%  If you plan to modify the pixels, use GetAuthenticPixels() instead.
%
%  Note, the AcquireImagePixels() and GetAuthenticPixels() methods are not
%  thread-safe.  In a threaded environment, use GetCacheViewVirtualPixels() or
%  GetCacheViewAuthenticPixels() instead.
%
%  Deprecated, replace with:
%
%    GetVirtualPixels(image,x,y,columns,rows,exception);
%
%  The format of the AcquireImagePixels() method is:
%
%      const PixelPacket *AcquireImagePixels(const Image *image,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport const PixelPacket *AcquireImagePixels(const Image *image,
  const ssize_t x,const ssize_t y,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
{
  return(GetVirtualPixels(image,x,y,columns,rows,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I n d e x e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireIndexes() returns the black channel or the colormap indexes
%  associated with the last call to QueueAuthenticPixels() or
%  GetVirtualPixels().  NULL is returned if the black channel or colormap
%  indexes are not available.
%
%  Deprecated, replace with:
%
%    GetVirtualIndexQueue(image);
%
%  The format of the AcquireIndexes() method is:
%
%      const IndexPacket *AcquireIndexes(const Image *image)
%
%  A description of each parameter follows:
%
%    o indexes: AcquireIndexes() returns the indexes associated with the last
%      call to QueueAuthenticPixels() or GetVirtualPixels().
%
%    o image: the image.
%
*/
MagickExport const IndexPacket *AcquireIndexes(const Image *image)
{
  return(GetVirtualIndexQueue(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M e m o r y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMemory() returns a pointer to a block of memory at least size bytes
%  suitably aligned for any use.
%
%  The format of the AcquireMemory method is:
%
%      void *AcquireMemory(const size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size of the memory in bytes to allocate.
%
*/
MagickExport void *AcquireMemory(const size_t size)
{
  void
    *allocation;

  assert(size != 0);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  allocation=malloc(size);
  return(allocation);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e O n e C a c h e V i e w P i x e l                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireOneCacheViewPixel() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.  If
%  you plan to modify the pixel, use GetOneCacheViewAuthenticPixel() instead.
%
%  Deprecated, replace with:
%
%    GetOneCacheViewVirtualPixel(cache_view,x,y,pixel,exception);
%
%  The format of the AcquireOneCacheViewPixel method is:
%
%      MagickBooleanType AcquireOneCacheViewPixel(const CacheView *cache_view,
%        const ssize_t x,const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
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
MagickExport MagickBooleanType AcquireOneCacheViewPixel(
  const CacheView *cache_view,const ssize_t x,const ssize_t y,PixelPacket *pixel,
  ExceptionInfo *exception)
{
  return(GetOneCacheViewVirtualPixel(cache_view,x,y,pixel,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e O n e C a c h e V i e w V i r t u a l P i x e l             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireOneCacheViewVirtualPixel() returns a single pixel at the specified
%  (x,y) location.  The image background color is returned if an error occurs.
%  If you plan to modify the pixel, use GetOneCacheViewAuthenticPixel() instead.
%
%  Deprecated, replace with:
%
%    GetOneCacheViewVirtualMethodPixel(cache_view,virtual_pixel_method,
%      x,y,pixel,exception);
%
%  The format of the AcquireOneCacheViewPixel method is:
%
%      MagickBooleanType AcquireOneCacheViewVirtualPixel(
%        const CacheView *cache_view,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
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
MagickExport MagickBooleanType AcquireOneCacheViewVirtualPixel(
  const CacheView *cache_view,const VirtualPixelMethod virtual_pixel_method,
  const ssize_t x,const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetOneCacheViewVirtualMethodPixel(cache_view,virtual_pixel_method,
    x,y,pixel,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e O n e M a g i c k P i x e l                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireOneMagickPixel() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.  If
%  you plan to modify the pixel, use GetOnePixel() instead.
%
%  Deprecated, replace with:
%
%    MagickPixelPacket pixel;
%    GetOneVirtualMagickPixel(image,x,y,&pixel,exception);
%
%  The format of the AcquireOneMagickPixel() method is:
%
%      MagickPixelPacket AcquireOneMagickPixel(const Image image,const ssize_t x,
%        const ssize_t y,ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickPixelPacket AcquireOneMagickPixel(const Image *image,
  const ssize_t x,const ssize_t y,ExceptionInfo *exception)
{
  MagickPixelPacket
    pixel;

  (void) GetOneVirtualMagickPixel(image,x,y,&pixel,exception);
  return(pixel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e O n e P i x e l                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireOnePixel() returns a single pixel at the specified (x,y) location.
%  The image background color is returned if an error occurs.  If you plan to
%  modify the pixel, use GetOnePixel() instead.
%
%  Deprecated, replace with:
%
%    PixelPacket pixel;
%    GetOneVirtualPixel(image,x,y,&pixel,exception);
%
%  The format of the AcquireOnePixel() method is:
%
%      PixelPacket AcquireOnePixel(const Image image,const ssize_t x,
%        const ssize_t y,ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport PixelPacket AcquireOnePixel(const Image *image,const ssize_t x,
  const ssize_t y,ExceptionInfo *exception)
{
  PixelPacket
    pixel;

  (void) GetOneVirtualPixel(image,x,y,&pixel,exception);
  return(pixel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e O n e V i r t u a l P i x e l                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireOneVirtualPixel() returns a single pixel at the specified (x,y)
%  location as defined by specified pixel method.  The image background color
%  is returned if an error occurs.  If you plan to modify the pixel, use
%  GetOnePixel() instead.
%
%  Deprecated, replace with:
%
%    PixelPacket pixel;
%    GetOneVirtualMethodPixel(image,virtual_pixel_method,x,y,&pixel,exception);
%
%  The format of the AcquireOneVirtualPixel() method is:
%
%      PixelPacket AcquireOneVirtualPixel(const Image image,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport PixelPacket AcquireOneVirtualPixel(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  ExceptionInfo *exception)
{
  PixelPacket
    pixel;

  (void) GetOneVirtualMethodPixel(image,virtual_pixel_method,x,y,&pixel,
    exception);
  return(pixel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e P i x e l s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquirePixels() returns the pixels associated with the last call to
%  QueueAuthenticPixels() or GetVirtualPixels().
%
%  Deprecated, replace with:
%
%    GetVirtualPixelQueue(image);
%
%  The format of the AcquirePixels() method is:
%
%      const PixelPacket *AcquirePixels(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const PixelPacket *AcquirePixels(const Image *image)
{
  return(GetVirtualPixelQueue(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A f f i n i t y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AffinityImage() replaces the colors of an image with the closest color from
%  a reference image.
%
%  Deprecated, replace with:
%
%    RemapImage(quantize_info,image,affinity_image);
%
%  The format of the AffinityImage method is:
%
%      MagickBooleanType AffinityImage(const QuantizeInfo *quantize_info,
%        Image *image,const Image *affinity_image)
%
%  A description of each parameter follows:
%
%    o quantize_info: Specifies a pointer to an QuantizeInfo structure.
%
%    o image: the image.
%
%    o affinity_image: the reference image.
%
*/
MagickExport MagickBooleanType AffinityImage(const QuantizeInfo *quantize_info,
  Image *image,const Image *affinity_image)
{
  return(RemapImage(quantize_info,image,affinity_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A f f i n i t y I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AffinityImages() replaces the colors of a sequence of images with the
%  closest color from a reference image.
%
%  Deprecated, replace with:
%
%    RemapImages(quantize_info,images,affinity_image);
%
%  The format of the AffinityImage method is:
%
%      MagickBooleanType AffinityImages(const QuantizeInfo *quantize_info,
%        Image *images,Image *affinity_image)
%
%  A description of each parameter follows:
%
%    o quantize_info: Specifies a pointer to an QuantizeInfo structure.
%
%    o images: the image sequence.
%
%    o affinity_image: the reference image.
%
*/
MagickExport MagickBooleanType AffinityImages(const QuantizeInfo *quantize_info,
  Image *images,const Image *affinity_image)
{
  return(RemapImages(quantize_info,images,affinity_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AllocateImage() returns a pointer to an image structure initialized to
%  default values.
%
%  Deprecated, replace with:
%
%    AcquireImage(image_info);
%
%  The format of the AllocateImage method is:
%
%      Image *AllocateImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
*/
MagickExport Image *AllocateImage(const ImageInfo *image_info)
{
  return(AcquireImage(image_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e I m a g e C o l o r m a p                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AllocateImageColormap() allocates an image colormap and initializes
%  it to a linear gray colorspace.  If the image already has a colormap,
%  it is replaced.  AllocateImageColormap() returns MagickTrue if successful,
%  otherwise MagickFalse if there is not enough memory.
%
%  Deprecated, replace with:
%
%    AcquireImageColormap(image,colors);
%
%  The format of the AllocateImageColormap method is:
%
%      MagickBooleanType AllocateImageColormap(Image *image,
%        const size_t colors)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colors: the number of colors in the image colormap.
%
*/
MagickExport MagickBooleanType AllocateImageColormap(Image *image,
  const size_t colors)
{
  return(AcquireImageColormap(image,colors));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e N e x t I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AllocateNextImage() initializes the next image in a sequence to
%  default values.  The next member of image points to the newly allocated
%  image.  If there is a memory shortage, next is assigned NULL.
%
%  Deprecated, replace with:
%
%    AcquireNextImage(image_info,image);
%
%  The format of the AllocateNextImage method is:
%
%      void AllocateNextImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
%    o image: the image.
%
*/
MagickExport void AllocateNextImage(const ImageInfo *image_info,Image *image)
{
  AcquireNextImage(image_info,image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e S t r i n g                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AllocateString() allocates memory for a string and copies the source string
%  to that memory location (and returns it).
%
%  The format of the AllocateString method is:
%
%      char *AllocateString(const char *source)
%
%  A description of each parameter follows:
%
%    o source: A character string.
%
*/
MagickExport char *AllocateString(const char *source)
{
  char
    *destination;

  size_t
    length;

  assert(source != (const char *) NULL);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  length=strlen(source)+MaxTextExtent+1;
  destination=(char *) AcquireQuantumMemory(length,sizeof(*destination));
  if (destination == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *destination='\0';
  if (source != (char *) NULL)
    (void) CopyMagickString(destination,source,length);
  return(destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A v e r a g e I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AverageImages() takes a set of images and averages them together.  Each
%  image in the set must have the same width and height.  AverageImages()
%  returns a single image with each corresponding pixel component of each
%  image averaged.   On failure, a NULL image is returned and exception
%  describes the reason for the failure.
%
%  Deprecated, replace with:
%
%    EvaluateImages(images,MeanEvaluateOperator,exception);
%
%  The format of the AverageImages method is:
%
%      Image *AverageImages(Image *images,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AverageImages(const Image *images,ExceptionInfo *exception)
{
  return(EvaluateImages(images,MeanEvaluateOperator,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C h a n n e l I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Extract a channel from the image.  A channel is a particular color component
%  of each pixel in the image.
%
%  Deprecated, replace with:
%
%    SeparateImageChannel(image,channel);
%
%  The format of the ChannelImage method is:
%
%      unsigned int ChannelImage(Image *image,const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: Identify which channel to extract: RedChannel, GreenChannel,
%      BlueChannel, OpacityChannel, CyanChannel, MagentaChannel, YellowChannel,
%      or BlackChannel.
%
*/
MagickExport unsigned int ChannelImage(Image *image,const ChannelType channel)
{
  return(SeparateImageChannel(image,channel));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C h a n n e l T h r e s h o l d I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ChannelThresholdImage() changes the value of individual pixels based on
%  the intensity of each pixel channel.  The result is a high-contrast image.
%
%  The format of the ChannelThresholdImage method is:
%
%      unsigned int ChannelThresholdImage(Image *image,const char *level)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o level: define the threshold values.
%
*/
MagickExport unsigned int ChannelThresholdImage(Image *image,const char *level)
{
  MagickPixelPacket
    threshold;

  GeometryInfo
    geometry_info;

  unsigned int
    flags,
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  if (level == (char *) NULL)
    return(MagickFalse);
  flags=ParseGeometry(level,&geometry_info);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    threshold.green=threshold.red;
  threshold.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    threshold.blue=threshold.red;
  status=BilevelImageChannel(image,RedChannel,threshold.red);
  status|=BilevelImageChannel(image,GreenChannel,threshold.green);
  status|=BilevelImageChannel(image,BlueChannel,threshold.blue);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l i p I m a g e P a t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClipPathImage() sets the image clip mask based any clipping path information
%  if it exists.
%
%  Deprecated, replace with:
%
%    ClipImagePath(image,pathname,inside);
%
%  The format of the ClipImage method is:
%
%      MagickBooleanType ClipPathImage(Image *image,const char *pathname,
%        const MagickBooleanType inside)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pathname: name of clipping path resource. If name is preceded by #, use
%      clipping path numbered by name.
%
%    o inside: if non-zero, later operations take effect inside clipping path.
%      Otherwise later operations take effect outside clipping path.
%
*/
MagickExport MagickBooleanType ClipPathImage(Image *image,const char *pathname,
  const MagickBooleanType inside)
{
  return(ClipImagePath(image,pathname,inside));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e A t t r i b u t e s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageAttributes() clones one or more image attributes.
%
%  Deprecated, replace with:
%
%    CloneImageProperties(image,clone_image);
%
%  The format of the CloneImageAttributes method is:
%
%      MagickBooleanType CloneImageAttributes(Image *image,
%        const Image *clone_image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o clone_image: the clone image.
%
*/
MagickExport MagickBooleanType CloneImageAttributes(Image *image,
  const Image *clone_image)
{
  return(CloneImageProperties(image,clone_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e M e m o r y                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneMemory() copies size bytes from memory area source to the destination.
%  Copying between objects that overlap will take place correctly.  It returns
%  destination.
%
%  The format of the CloneMemory method is:
%
%      void *CloneMemory(void *destination,const void *source,
%        const size_t size)
%
%  A description of each parameter follows:
%
%    o destination: the destination.
%
%    o source: the source.
%
%    o size: the size of the memory in bytes to allocate.
%
*/
MagickExport void *CloneMemory(void *destination,const void *source,
  const size_t size)
{
  register const unsigned char
    *p;

  register unsigned char
    *q;

  register ssize_t
    i;

  assert(destination != (void *) NULL);
  assert(source != (const void *) NULL);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  p=(const unsigned char *) source;
  q=(unsigned char *) destination;
  if ((p <= q) || ((p+size) >= q))
    return(CopyMagickMemory(destination,source,size));
  /*
    Overlap, copy backwards.
  */
  p+=size;
  q+=size;
  for (i=(ssize_t) (size-1); i >= 0; i--)
    *--q=(*--p);
  return(destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o s e C a c h e V i e w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloseCacheView() closes the specified view returned by a previous call to
%  OpenCacheView().
%
%  Deprecated, replace with:
%
%    DestroyCacheView(view_info);
%
%  The format of the CloseCacheView method is:
%
%      CacheView *CloseCacheView(CacheView *view_info)
%
%  A description of each parameter follows:
%
%    o view_info: the address of a structure of type CacheView.
%
*/
MagickExport CacheView *CloseCacheView(CacheView *view_info)
{
  return(DestroyCacheView(view_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o l o r F l o o d f i l l I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorFloodfill() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  By default target must match a particular pixel color exactly.
%  However, in many cases two colors may differ by a small amount.  The
%  fuzz member of image defines how much tolerance is acceptable to
%  consider two colors as the same.  For example, set fuzz to 10 and the
%  color red at intensities of 100 and 102 respectively are now
%  interpreted as the same color for the purposes of the floodfill.
%
%  The format of the ColorFloodfillImage method is:
%
%      MagickBooleanType ColorFloodfillImage(Image *image,
%        const DrawInfo *draw_info,const PixelPacket target,
%        const ssize_t x_offset,const ssize_t y_offset,const PaintMethod method)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o draw_info: the draw info.
%
%    o target: the RGB value of the target color.
%
%    o x,y: the starting location of the operation.
%
%    o method: Choose either FloodfillMethod or FillToBorderMethod.
%
*/

#define MaxStacksize  (1UL << 15)
#define PushSegmentStack(up,left,right,delta) \
{ \
  if (s >= (segment_stack+MaxStacksize)) \
    ThrowBinaryException(DrawError,"SegmentStackOverflow",image->filename) \
  else \
    { \
      if ((((up)+(delta)) >= 0) && (((up)+(delta)) < (ssize_t) image->rows)) \
        { \
          s->x1=(double) (left); \
          s->y1=(double) (up); \
          s->x2=(double) (right); \
          s->y2=(double) (delta); \
          s++; \
        } \
    } \
}

MagickExport MagickBooleanType ColorFloodfillImage(Image *image,
  const DrawInfo *draw_info,const PixelPacket target,const ssize_t x_offset,
  const ssize_t y_offset,const PaintMethod method)
{
  Image
    *floodplane_image;

  MagickBooleanType
    skip;

  PixelPacket
    fill_color;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  ssize_t
    offset,
    start,
    x,
    x1,
    x2,
    y;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(draw_info != (DrawInfo *) NULL);
  assert(draw_info->signature == MagickSignature);
  if ((x_offset < 0) || (x_offset >= (ssize_t) image->columns))
    return(MagickFalse);
  if ((y_offset < 0) || (y_offset >= (ssize_t) image->rows))
    return(MagickFalse);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  floodplane_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    &image->exception);
  if (floodplane_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageAlphaChannel(floodplane_image,OpaqueAlphaChannel);
  /*
    Set floodfill color.
  */
  segment_stack=(SegmentInfo *) AcquireQuantumMemory(MaxStacksize,
    sizeof(*segment_stack));
  if (segment_stack == (SegmentInfo *) NULL)
    {
      floodplane_image=DestroyImage(floodplane_image);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Push initial segment on stack.
  */
  x=x_offset;
  y=y_offset;
  start=0;
  s=segment_stack;
  PushSegmentStack(y,x,x,1);
  PushSegmentStack(y+1,x,x,-1);
  while (s > segment_stack)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    /*
      Pop segment off stack.
    */
    s--;
    x1=(ssize_t) s->x1;
    x2=(ssize_t) s->x2;
    offset=(ssize_t) s->y2;
    y=(ssize_t) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    p=GetVirtualPixels(image,0,y,(size_t) (x1+1),1,&image->exception);
    q=GetAuthenticPixels(floodplane_image,0,y,(size_t) (x1+1),1,
      &image->exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    p+=x1;
    q+=x1;
    for (x=x1; x >= 0; x--)
    {
      if (q->opacity == (Quantum) TransparentOpacity)
        break;
      if (method == FloodfillMethod)
        {
          if (IsColorSimilar(image,p,&target) == MagickFalse)
            break;
        }
      else
        if (IsColorSimilar(image,p,&target) != MagickFalse)
          break;
      q->opacity=(Quantum) TransparentOpacity;
      p--;
      q--;
    }
    if (SyncAuthenticPixels(floodplane_image,&image->exception) == MagickFalse)
      break;
    skip=x >= x1 ? MagickTrue : MagickFalse;
    if (skip == MagickFalse)
      {
        start=x+1;
        if (start < x1)
          PushSegmentStack(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (skip == MagickFalse)
        {
          if (x < (ssize_t) image->columns)
            {
              p=GetVirtualPixels(image,x,y,image->columns-x,1,
                &image->exception);
              q=GetAuthenticPixels(floodplane_image,x,y,image->columns-x,1,
                &image->exception);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for ( ; x < (ssize_t) image->columns; x++)
              {
                if (q->opacity == (Quantum) TransparentOpacity)
                  break;
                if (method == FloodfillMethod)
                  {
                    if (IsColorSimilar(image,p,&target) == MagickFalse)
                      break;
                  }
                else
                  if (IsColorSimilar(image,p,&target) != MagickFalse)
                    break;
                q->opacity=(Quantum) TransparentOpacity;
                p++;
                q++;
              }
              if (SyncAuthenticPixels(floodplane_image,&image->exception) == MagickFalse)
                break;
            }
          PushSegmentStack(y,start,x-1,offset);
          if (x > (x2+1))
            PushSegmentStack(y,x2+1,x-1,-offset);
        }
      skip=MagickFalse;
      x++;
      if (x <= x2)
        {
          p=GetVirtualPixels(image,x,y,(size_t) (x2-x+1),1,
            &image->exception);
          q=GetAuthenticPixels(floodplane_image,x,y,(size_t) (x2-x+1),1,
            &image->exception);
          if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
            break;
          for ( ; x <= x2; x++)
          {
            if (q->opacity == (Quantum) TransparentOpacity)
              break;
            if (method == FloodfillMethod)
              {
                if (IsColorSimilar(image,p,&target) != MagickFalse)
                  break;
              }
            else
              if (IsColorSimilar(image,p,&target) == MagickFalse)
                break;
            p++;
            q++;
          }
        }
      start=x;
    } while (x <= x2);
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    /*
      Tile fill color onto floodplane.
    */
    p=GetVirtualPixels(floodplane_image,0,y,image->columns,1,
      &image->exception);
    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelOpacity(p) != OpaqueOpacity)
        {
          (void) GetFillColor(draw_info,x,y,&fill_color);
          MagickCompositeOver(&fill_color,(MagickRealType) fill_color.opacity,q,
            (MagickRealType) q->opacity,q);
        }
      p++;
      q++;
    }
    if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
      break;
  }
  segment_stack=(SegmentInfo *) RelinquishMagickMemory(segment_stack);
  floodplane_image=DestroyImage(floodplane_image);
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e A t t r i b u t e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageAttribute() deletes an attribute from the image.
%
%  Deprecated, replace with:
%
%    DeleteImageProperty(image,key);
%
%  The format of the DeleteImageAttribute method is:
%
%      MagickBooleanType DeleteImageAttribute(Image *image,const char *key)
%
%  A description of each parameter follows:
%
%    o image: the image info.
%
%    o key: the image key.
%
*/
MagickExport MagickBooleanType DeleteImageAttribute(Image *image,
  const char *key)
{
  return(DeleteImageProperty(image,key));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageList() deletes an image at the specified position in the list.
%
%  The format of the DeleteImageList method is:
%
%      unsigned int DeleteImageList(Image *images,const ssize_t offset)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o offset: the position within the list.
%
*/
MagickExport unsigned int DeleteImageList(Image *images,const ssize_t offset)
{
  register ssize_t
    i;

  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  while (GetPreviousImageInList(images) != (Image *) NULL)
    images=GetPreviousImageInList(images);
  for (i=0; i < offset; i++)
  {
    if (GetNextImageInList(images) == (Image *) NULL)
      return(MagickFalse);
    images=GetNextImageInList(images);
  }
  DeleteImageFromList(&images);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e M a g i c k R e g i s t r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteMagickRegistry() deletes an entry in the registry as defined by the id.
%  It returns MagickTrue if the entry is deleted otherwise MagickFalse if no
%  entry is found in the registry that matches the id.
%
%  Deprecated, replace with:
%
%    char key[MaxTextExtent];
%    FormatLocaleString(key,MaxTextExtent,"%ld\n",id);
%    DeleteImageRegistry(key);
%
%  The format of the DeleteMagickRegistry method is:
%
%      MagickBooleanType DeleteMagickRegistry(const ssize_t id)
%
%  A description of each parameter follows:
%
%    o id: the registry id.
%
*/
MagickExport MagickBooleanType DeleteMagickRegistry(const ssize_t id)
{
  char
    key[MaxTextExtent];

  (void) FormatLocaleString(key,MaxTextExtent,"%.20g\n",(double) id);
  return(DeleteImageRegistry(key));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C o n s t i t u t e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyConstitute() destroys the constitute component.
%
%  The format of the DestroyConstitute method is:
%
%      DestroyConstitute(void)
%
*/
MagickExport void DestroyConstitute(void)
{
  ConstituteComponentTerminus();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y M a g i c k R e g i s t r y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMagickRegistry() deallocates memory associated the magick registry.
%
%  Deprecated, replace with:
%
%    RegistryComponentTerminus();
%
%  The format of the DestroyMagickRegistry method is:
%
%       void DestroyMagickRegistry(void)
%
*/
MagickExport void DestroyMagickRegistry(void)
{
  RegistryComponentTerminus();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s c r i b e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DescribeImage() describes an image by printing its attributes to the file.
%  Attributes include the image width, height, size, and others.
%
%  Deprecated, replace with:
%
%    IdentifyImage(image,file,verbose);
%
%  The format of the DescribeImage method is:
%
%      MagickBooleanType DescribeImage(Image *image,FILE *file,
%        const MagickBooleanType verbose)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o file: the file, typically stdout.
%
%    o verbose: A value other than zero prints more detailed information
%      about the image.
%
*/
MagickExport MagickBooleanType DescribeImage(Image *image,FILE *file,
  const MagickBooleanType verbose)
{
  return(IdentifyImage(image,file,verbose));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e A t t r i b u t e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageAttributes() deallocates memory associated with the image
%  attribute list.
%
%  The format of the DestroyImageAttributes method is:
%
%      DestroyImageAttributes(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DestroyImageAttributes(Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->attributes != (void *) NULL)
    image->attributes=(void *) DestroySplayTree((SplayTreeInfo *)
      image->attributes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImages() destroys an image list.
%
%  Deprecated, replace with:
%
%    DestroyImageList(image);
%
%  The format of the DestroyImages method is:
%
%      void DestroyImages(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
*/
MagickExport void DestroyImages(Image *image)
{
  if (image == (Image *) NULL)
    return;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.4.3");
  image=DestroyImageList(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y M a g i c k                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMagick() destroys the ImageMagick environment.
%
%  Deprecated, replace with:
%
%    MagickCoreTerminus();
%
%  The format of the DestroyMagick function is:
%
%      DestroyMagick(void)
%
*/
MagickExport void DestroyMagick(void)
{
  MagickCoreTerminus();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D i s p a t c h I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DispatchImage() extracts pixel data from an image and returns it to you.
%  The method returns MagickFalse on success otherwise MagickTrue if an error is
%  encountered.  The data is returned as char, short int, int, ssize_t, float,
%  or double in the order specified by map.
%
%  Suppose you want to extract the first scanline of a 640x480 image as
%  character data in red-green-blue order:
%
%      DispatchImage(image,0,0,640,1,"RGB",CharPixel,pixels,exception);
%
%  Deprecated, replace with:
%
%    ExportImagePixels(image,x_offset,y_offset,columns,rows,map,type,pixels,
%      exception);
%
%  The format of the DispatchImage method is:
%
%      unsigned int DispatchImage(const Image *image,const ssize_t x_offset,
%        const ssize_t y_offset,const size_t columns,
%        const size_t rows,const char *map,const StorageType type,
%        void *pixels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x_offset, y_offset, columns, rows:  These values define the perimeter
%      of a region of pixels you want to extract.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha, C = cyan, Y = yellow, M = magenta, K = black, or
%      I = intensity (for grayscale).
%
%    o type: Define the data type of the pixels.  Float and double types are
%      normalized to [0..1] otherwise [0..QuantumRange].  Choose from these
%      types: CharPixel, ShortPixel, IntegerPixel, LongPixel, FloatPixel, or
%      DoublePixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport unsigned int DispatchImage(const Image *image,const ssize_t x_offset,
  const ssize_t y_offset,const size_t columns,const size_t rows,
  const char *map,const StorageType type,void *pixels,ExceptionInfo *exception)
{
  unsigned int
    status;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.6");
  status=ExportImagePixels(image,x_offset,y_offset,columns,rows,map,type,pixels,
    exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x t r a c t S u b i m a g e F r o m I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExtractSubimageFromImageImage() extracts a region of the image that most
%  closely resembles the reference.
%
%  The format of the ExtractSubimageFromImageImage method is:
%
%      Image *ExtractSubimageFromImage(const Image *image,
%        const Image *reference,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reference: find an area of the image that closely resembles this image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static double GetSimilarityMetric(const Image *image,const Image *reference,
  const ssize_t x_offset,const ssize_t y_offset,
  const double similarity_threshold,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reference_view;

  double
    channels,
    normalized_similarity,
    similarity;

  ssize_t
    y;

  /*
    Compute the similarity in pixels between two images.
  */
  normalized_similarity=1.0;
  similarity=0.0;
  channels=3;
  if ((image->matte != MagickFalse) && (reference->matte != MagickFalse))
    channels++;
  if ((image->colorspace == CMYKColorspace) &&
      (reference->colorspace == CMYKColorspace))
    channels++;
  image_view=AcquireVirtualCacheView(image,exception);
  reference_view=AcquireVirtualCacheView(reference,exception);
  for (y=0; y < (ssize_t) reference->rows; y++)
  {
    register const IndexPacket
      *indexes,
      *reference_indexes;

    register const PixelPacket
      *p,
      *q;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset+y,
      reference->columns,1,exception);
    q=GetCacheViewVirtualPixels(reference_view,0,y,reference->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      continue;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reference_indexes=GetCacheViewVirtualIndexQueue(reference_view);
    for (x=0; x < (ssize_t) reference->columns; x++)
    {
      MagickRealType
        pixel;

      pixel=QuantumScale*(GetPixelRed(p)-(double)
        GetPixelRed(q));
      similarity+=pixel*pixel;
      pixel=QuantumScale*(GetPixelGreen(p)-(double)
        GetPixelGreen(q));
      similarity+=pixel*pixel;
      pixel=QuantumScale*(GetPixelBlue(p)-(double)
        GetPixelBlue(q));
      similarity+=pixel*pixel;
      if ((image->matte != MagickFalse) && (reference->matte != MagickFalse))
        {
          pixel=QuantumScale*(GetPixelOpacity(p)-(double)
            GetPixelOpacity(q));
          similarity+=pixel*pixel;
        }
      if ((image->colorspace == CMYKColorspace) &&
          (reference->colorspace == CMYKColorspace))
        {
          pixel=QuantumScale*(GetPixelIndex(indexes+x)-(double)
            GetPixelIndex(reference_indexes+x));
          similarity+=pixel*pixel;
        }
      p++;
      q++;
    }
    normalized_similarity=sqrt(similarity)/reference->columns/reference->rows/
      channels;
    if (normalized_similarity > similarity_threshold)
      break;
  }
  reference_view=DestroyCacheView(reference_view);
  image_view=DestroyCacheView(image_view);
  return(normalized_similarity);
}

MagickExport Image *ExtractSubimageFromImage(Image *image,
  const Image *reference,ExceptionInfo *exception)
{
  double
    similarity_threshold;

  RectangleInfo
    offset;

  ssize_t
    y;

  /*
    Extract reference from image.
  */
  if ((reference->columns > image->columns) || (reference->rows > image->rows))
    return((Image *) NULL);
  similarity_threshold=(double) image->columns*image->rows;
  SetGeometry(reference,&offset);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4)
#endif
  for (y=0; y < (ssize_t) (image->rows-reference->rows); y++)
  {
    double
      similarity;

    register ssize_t
      x;

    for (x=0; x < (ssize_t) (image->columns-reference->columns); x++)
    {
      similarity=GetSimilarityMetric(image,reference,x,y,similarity_threshold,
        exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ExtractSubimageFromImage)
#endif
      if (similarity < similarity_threshold)
        {
          similarity_threshold=similarity;
          offset.x=x;
          offset.y=y;
        }
    }
  }
  if (similarity_threshold > (QuantumScale*reference->fuzz/100.0))
    return((Image *) NULL);
  return(CropImage(image,&offset,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     F l a t t e n I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FlattenImages() Obsolete Function: Use MergeImageLayers() instead.
%
%  Deprecated, replace with:
%
%    MergeImageLayers(image,FlattenLayer,exception);
%
%  The format of the FlattenImage method is:
%
%      Image *FlattenImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *FlattenImages(Image *image,ExceptionInfo *exception)
{
  return(MergeImageLayers(image,FlattenLayer,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t I m a g e A t t r i b u t e                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatImageAttribute() permits formatted key/value pairs to be saved as an
%  image attribute.
%
%  The format of the FormatImageAttribute method is:
%
%      MagickBooleanType FormatImageAttribute(Image *image,const char *key,
%        const char *format,...)
%
%  A description of each parameter follows.
%
%   o  image:  The image.
%
%   o  key:  The attribute key.
%
%   o  format:  A string describing the format to use to write the remaining
%      arguments.
%
*/

MagickExport MagickBooleanType FormatImageAttributeList(Image *image,
  const char *key,const char *format,va_list operands)
{
  char
    value[MaxTextExtent];

  int
    n;

#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  n=vsnprintf(value,MaxTextExtent,format,operands);
#else
  n=vsprintf(value,format,operands);
#endif
  if (n < 0)
    value[MaxTextExtent-1]='\0';
  return(SetImageProperty(image,key,value));
}

MagickExport MagickBooleanType FormatImagePropertyList(Image *image,
  const char *property,const char *format,va_list operands)
{
  char
    value[MaxTextExtent];

  int
    n;

#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  n=vsnprintf(value,MaxTextExtent,format,operands);
#else
  n=vsprintf(value,format,operands);
#endif
  if (n < 0)
    value[MaxTextExtent-1]='\0';
  return(SetImageProperty(image,property,value));
}

MagickExport MagickBooleanType FormatImageAttribute(Image *image,
  const char *key,const char *format,...)
{
  char
    value[MaxTextExtent];

  int
    n;

  va_list
    operands;

  va_start(operands,format);
  n=FormatLocaleStringList(value,MaxTextExtent,format,operands);
  (void) n;
  va_end(operands);
  return(SetImageProperty(image,key,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t M a g i c k S t r i n g                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatMagickString() prints formatted output of a variable argument list.
%
%  The format of the FormatMagickString method is:
%
%      ssize_t FormatMagickString(char *string,const size_t length,
%        const char *format,...)
%
%  A description of each parameter follows.
%
%   o string:  FormatMagickString() returns the formatted string in this
%     character buffer.
%
%   o length: the maximum length of the string.
%
%   o format:  A string describing the format to use to write the remaining
%     arguments.
%
*/

MagickExport ssize_t FormatMagickStringList(char *string,const size_t length,
  const char *format,va_list operands)
{
  int
    n;

#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  n=vsnprintf(string,length,format,operands);
#else
  n=vsprintf(string,format,operands);
#endif
  if (n < 0)
    string[length-1]='\0';
  return((ssize_t) n);
}

MagickExport ssize_t FormatMagickString(char *string,const size_t length,
  const char *format,...)
{
  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=(ssize_t) FormatMagickStringList(string,length,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t S t r i n g                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatString() prints formatted output of a variable argument list.
%
%  The format of the FormatString method is:
%
%      void FormatString(char *string,const char *format,...)
%
%  A description of each parameter follows.
%
%   o  string:  Method FormatString returns the formatted string in this
%      character buffer.
%
%   o  format:  A string describing the format to use to write the remaining
%      arguments.
%
*/

MagickExport void FormatStringList(char *string,const char *format,
  va_list operands)
{
  int
    n;

 (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  n=vsnprintf(string,MaxTextExtent,format,operands);
#else
  n=vsprintf(string,format,operands);
#endif
  if (n < 0)
    string[MaxTextExtent-1]='\0';
}

MagickExport void FormatString(char *string,const char *format,...)
{
  va_list
    operands;

  va_start(operands,format);
  (void) FormatLocaleStringList(string,MaxTextExtent,format,operands);
  va_end(operands);
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   F u z z y C o l o r M a t c h                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FuzzyColorMatch() returns true if two pixels are identical in color.
%
%  The format of the ColorMatch method is:
%
%      void FuzzyColorMatch(const PixelPacket *p,const PixelPacket *q,
%        const double fuzz)
%
%  A description of each parameter follows:
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
%    o distance:  Define how much tolerance is acceptable to consider
%      two colors as the same.
%
*/
MagickExport unsigned int FuzzyColorMatch(const PixelPacket *p,
  const PixelPacket *q,const double fuzz)
{
  MagickPixelPacket
    pixel;

  register MagickRealType
    distance;

  if ((fuzz == 0.0) && (GetPixelRed(p) == GetPixelRed(q)) &&
      (GetPixelGreen(p) == GetPixelGreen(q)) &&
      (GetPixelBlue(p) == GetPixelBlue(q)))
    return(MagickTrue);
  pixel.red=GetPixelRed(p)-(MagickRealType) GetPixelRed(q);
  distance=pixel.red*pixel.red;
  if (distance > (fuzz*fuzz))
    return(MagickFalse);
  pixel.green=GetPixelGreen(p)-(MagickRealType)
    GetPixelGreen(q);
  distance+=pixel.green*pixel.green;
  if (distance > (fuzz*fuzz))
    return(MagickFalse);
  pixel.blue=GetPixelBlue(p)-(MagickRealType) GetPixelBlue(q);
  distance+=pixel.blue*pixel.blue;
  if (distance > (fuzz*fuzz))
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   F u z z y C o l o r C o m p a r e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FuzzyColorCompare() returns MagickTrue if the distance between two colors is
%  less than the specified distance in a linear three dimensional color space.
%  This method is used by ColorFloodFill() and other algorithms which
%  compare two colors.
%
%  The format of the FuzzyColorCompare method is:
%
%      void FuzzyColorCompare(const Image *image,const PixelPacket *p,
%        const PixelPacket *q)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType FuzzyColorCompare(const Image *image,
  const PixelPacket *p,const PixelPacket *q)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.2.5");
  return(IsColorSimilar(image,p,q));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   F u z z y O p a c i t y C o m p a r e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FuzzyOpacityCompare() returns true if the distance between two opacity
%  values is less than the specified distance in a linear color space.  This
%  method is used by MatteFloodFill() and other algorithms which compare
%  two opacity values.
%
%  Deprecated, replace with:
%
%    IsOpacitySimilar(image,p,q);
%
%  The format of the FuzzyOpacityCompare method is:
%
%      void FuzzyOpacityCompare(const Image *image,const PixelPacket *p,
%        const PixelPacket *q)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o p: Pixel p.
%
%    o q: Pixel q.
%
*/
MagickExport MagickBooleanType FuzzyOpacityCompare(const Image *image,
  const PixelPacket *p,const PixelPacket *q)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.2.5");
  return(IsOpacitySimilar(image,p,q));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t C o n f i g u r e B l o b                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureBlob() returns the specified configure file as a blob.
%
%  The format of the GetConfigureBlob method is:
%
%      void *GetConfigureBlob(const char *filename,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the configure file name.
%
%    o path: return the full path information of the configure file.
%
%    o length: This pointer to a size_t integer sets the initial length of the
%      blob.  On return, it reflects the actual length of the blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void *GetConfigureBlob(const char *filename,char *path,
  size_t *length,ExceptionInfo *exception)
{
  void
    *blob;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  assert(path != (char *) NULL);
  assert(length != (size_t *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  blob=(void *) NULL;
  (void) CopyMagickString(path,filename,MaxTextExtent);
#if defined(MAGICKCORE_INSTALLED_SUPPORT)
#if defined(MAGICKCORE_LIBRARY_PATH)
  if (blob == (void *) NULL)
    {
      /*
        Search hard coded paths.
      */
      (void) FormatLocaleString(path,MaxTextExtent,"%s%s",
        MAGICKCORE_LIBRARY_PATH,filename);
      if (IsPathAccessible(path) != MagickFalse)
        blob=FileToBlob(path,~0,length,exception);
    }
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !(defined(MAGICKCORE_CONFIGURE_PATH) || defined(MAGICKCORE_SHARE_PATH))
  if (blob == (void *) NULL)
    {
      char
        *key_value;

      /*
        Locate file via registry key.
      */
      key_value=NTRegistryKeyLookup("ConfigurePath");
      if (key_value != (char *) NULL)
        {
          (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",key_value,
            DirectorySeparator,filename);
          if (IsPathAccessible(path) != MagickFalse)
            blob=FileToBlob(path,~0,length,exception);
        }
    }
#endif
#else
  if (blob == (void *) NULL)
    {
      char
        *home;

      home=GetEnvironmentValue("MAGICK_HOME");
      if (home != (char *) NULL)
        {
          /*
            Search MAGICK_HOME.
          */
#if !defined(MAGICKCORE_POSIX_SUPPORT)
          (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",home,
            DirectorySeparator,filename);
#else
          (void) FormatLocaleString(path,MaxTextExtent,"%s/lib/%s/%s",home,
            MAGICKCORE_LIBRARY_RELATIVE_PATH,filename);
#endif
          if (IsPathAccessible(path) != MagickFalse)
            blob=FileToBlob(path,~0,length,exception);
          home=DestroyString(home);
        }
      home=GetEnvironmentValue("HOME");
      if (home == (char *) NULL)
        home=GetEnvironmentValue("USERPROFILE");
      if (home != (char *) NULL)
        {
          /*
            Search $HOME/.magick.
          */
          (void) FormatLocaleString(path,MaxTextExtent,"%s%s.magick%s%s",home,
            DirectorySeparator,DirectorySeparator,filename);
          if ((IsPathAccessible(path) != MagickFalse) && (blob == (void *) NULL))
            blob=FileToBlob(path,~0,length,exception);
          home=DestroyString(home);
        }
    }
  if ((blob == (void *) NULL) && (*GetClientPath() != '\0'))
    {
#if !defined(MAGICKCORE_POSIX_SUPPORT)
      (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",GetClientPath(),
        DirectorySeparator,filename);
#else
      char
        prefix[MaxTextExtent];

      /*
        Search based on executable directory if directory is known.
      */
      (void) CopyMagickString(prefix,GetClientPath(),
        MaxTextExtent);
      ChopPathComponents(prefix,1);
      (void) FormatLocaleString(path,MaxTextExtent,"%s/lib/%s/%s",prefix,
        MAGICKCORE_LIBRARY_RELATIVE_PATH,filename);
#endif
      if (IsPathAccessible(path) != MagickFalse)
        blob=FileToBlob(path,~0,length,exception);
    }
  /*
    Search current directory.
  */
  if ((blob == (void *) NULL) && (IsPathAccessible(path) != MagickFalse))
    blob=FileToBlob(path,~0,length,exception);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  /*
    Search Windows registry.
  */
  if (blob == (void *) NULL)
    blob=NTResourceToBlob(filename);
#endif
#endif
  if (blob == (void *) NULL)
    (void) ThrowMagickException(exception,GetMagickModule(),ConfigureWarning,
      "UnableToOpenConfigureFile","`%s'",path);
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheView() gets pixels from the in-memory or disk pixel cache as
%  defined by the geometry parameters.   A pointer to the pixels is returned if
%  the pixels are transferred, otherwise a NULL is returned.
%
%  Deprecated, replace with:
%
%    GetCacheViewAuthenticPixels(cache_view,x,y,columns,rows,
%      GetCacheViewException(cache_view));
%
%  The format of the GetCacheView method is:
%
%      PixelPacket *GetCacheView(CacheView *cache_view,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the address of a structure of type CacheView.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *GetCacheView(CacheView *cache_view,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows)
{
  PixelPacket
    *pixels;

  pixels=GetCacheViewAuthenticPixels(cache_view,x,y,columns,rows,
    GetCacheViewException(cache_view));
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w I n d e x e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewIndexes() returns the indexes associated with the specified
%  view.
%
%  Deprecated, replace with:
%
%    GetCacheViewAuthenticIndexQueue(cache_view);
%
%  The format of the GetCacheViewIndexes method is:
%
%      IndexPacket *GetCacheViewIndexes(CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport IndexPacket *GetCacheViewIndexes(CacheView *cache_view)
{
  return(GetCacheViewAuthenticIndexQueue(cache_view));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewPixels() gets pixels from the in-memory or disk pixel cache as
%  defined by the geometry parameters.   A pointer to the pixels is returned if
%  the pixels are transferred, otherwise a NULL is returned.
%
%  Deprecated, replace with:
%
%    GetCacheViewAuthenticPixels(cache_view,x,y,columns,rows,
%      GetCacheViewException(cache_view));
%
%  The format of the GetCacheViewPixels method is:
%
%      PixelPacket *GetCacheViewPixels(CacheView *cache_view,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *GetCacheViewPixels(CacheView *cache_view,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows)
{
  PixelPacket
    *pixels;

  pixels=GetCacheViewAuthenticPixels(cache_view,x,y,columns,rows,
    GetCacheViewException(cache_view));
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e A t t r i b u t e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageAttribute() searches the list of image attributes and returns
%  a pointer to the attribute if it exists otherwise NULL.
%
%  The format of the GetImageAttribute method is:
%
%      const ImageAttribute *GetImageAttribute(const Image *image,
%        const char *key)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o key:  These character strings are the name of an image attribute to
%      return.
%
*/

static void *DestroyAttribute(void *attribute)
{
  register ImageAttribute
    *p;

  p=(ImageAttribute *) attribute;
  if (p->value != (char *) NULL)
    p->value=DestroyString(p->value);
  return(RelinquishMagickMemory(p));
}

MagickExport const ImageAttribute *GetImageAttribute(const Image *image,
  const char *key)
{
  const char
    *value;

  ImageAttribute
    *attribute;

  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.3.1");
  value=GetImageProperty(image,key);
  if (value == (const char *) NULL)
    return((const ImageAttribute *) NULL);
  if (image->attributes == (void *) NULL)
    ((Image *) image)->attributes=NewSplayTree(CompareSplayTreeString,
      RelinquishMagickMemory,DestroyAttribute);
  else
    {
      const ImageAttribute
        *attribute;

      attribute=(const ImageAttribute *) GetValueFromSplayTree((SplayTreeInfo *)
        image->attributes,key);
      if (attribute != (const ImageAttribute *) NULL)
        return(attribute);
    }
  attribute=(ImageAttribute *) AcquireMagickMemory(sizeof(*attribute));
  if (attribute == (ImageAttribute *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(attribute,0,sizeof(*attribute));
  attribute->key=ConstantString(key);
  attribute->value=ConstantString(value);
  (void) AddValueToSplayTree((SplayTreeInfo *) ((Image *) image)->attributes,
    attribute->key,attribute);
  return((const ImageAttribute *) attribute);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C l i p p i n g P a t h A t t r i b u t e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageClippingPathAttribute() searches the list of image attributes and
%  returns a pointer to a clipping path if it exists otherwise NULL.
%
%  Deprecated, replace with:
%
%    GetImageAttribute(image,"8BIM:1999,2998");
%
%  The format of the GetImageClippingPathAttribute method is:
%
%      const ImageAttribute *GetImageClippingPathAttribute(Image *image)
%
%  A description of each parameter follows:
%
%    o attribute:  Method GetImageClippingPathAttribute returns the clipping
%      path if it exists otherwise NULL.
%
%    o image: the image.
%
*/
MagickExport const ImageAttribute *GetImageClippingPathAttribute(Image *image)
{
  return(GetImageAttribute(image,"8BIM:1999,2998"));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e F r o m M a g i c k R e g i s t r y                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageFromMagickRegistry() gets an image from the registry as defined by
%  its name.  If the image is not found, a NULL image is returned.
%
%  Deprecated, replace with:
%
%    GetImageRegistry(ImageRegistryType,name,exception);
%
%  The format of the GetImageFromMagickRegistry method is:
%
%      Image *GetImageFromMagickRegistry(const char *name,ssize_t *id,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: the name of the image to retrieve from the registry.
%
%    o id: the registry id.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *GetImageFromMagickRegistry(const char *name,ssize_t *id,
  ExceptionInfo *exception)
{
  *id=0L;
  return((Image *) GetImageRegistry(ImageRegistryType,name,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k R e g i s t r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickRegistry() gets a blob from the registry as defined by the id.  If
%  the blob that matches the id is not found, NULL is returned.
%
%  The format of the GetMagickRegistry method is:
%
%      const void *GetMagickRegistry(const ssize_t id,RegistryType *type,
%        size_t *length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o id: the registry id.
%
%    o type: the registry type.
%
%    o length: the blob length in number of bytes.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void *GetMagickRegistry(const ssize_t id,RegistryType *type,
  size_t *length,ExceptionInfo *exception)
{
  char
    key[MaxTextExtent];

  void
    *blob;

  *type=UndefinedRegistryType;
  *length=0;
  (void) FormatLocaleString(key,MaxTextExtent,"%.20g\n",(double) id);
  blob=(void *) GetImageRegistry(ImageRegistryType,key,exception);
  if (blob != (void *) NULL)
    return(blob);
  blob=(void *) GetImageRegistry(ImageInfoRegistryType,key,exception);
  if (blob != (void *) NULL)
    return(blob);
  return((void *) GetImageRegistry(UndefinedRegistryType,key,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e G e o m e t r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageGeometry() returns a region as defined by the geometry string with
%  respect to the image and its gravity.
%
%  Deprecated, replace with:
%
%    if (size_to_fit != MagickFalse)
%      ParseRegionGeometry(image,geometry,region_info,&image->exception); else
%      ParsePageGeometry(image,geometry,region_info,&image->exception);
%
%  The format of the GetImageGeometry method is:
%
%      int GetImageGeometry(Image *image,const char *geometry,
%        const unsigned int size_to_fit,RectangeInfo *region_info)
%
%  A description of each parameter follows:
%
%    o flags:  Method GetImageGeometry returns a bitmask that indicates
%      which of the four values were located in the geometry string.
%
%    o geometry:  The geometry (e.g. 100x100+10+10).
%
%    o size_to_fit:  A value other than 0 means to scale the region so it
%      fits within the specified width and height.
%
%    o region_info: the region as defined by the geometry string with
%      respect to the image and its gravity.
%
*/
MagickExport int GetImageGeometry(Image *image,const char *geometry,
  const unsigned int size_to_fit,RectangleInfo *region_info)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.4");
  if (size_to_fit != MagickFalse)
    return((int) ParseRegionGeometry(image,geometry,region_info,&image->exception));
  return((int) ParsePageGeometry(image,geometry,region_info,&image->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageList() returns an image at the specified position in the list.
%
%  Deprecated, replace with:
%
%    CloneImage(GetImageFromList(images,(ssize_t) offset),0,0,MagickTrue,
%      exception);
%
%  The format of the GetImageList method is:
%
%      Image *GetImageList(const Image *images,const ssize_t offset,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o offset: the position within the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *GetImageList(const Image *images,const ssize_t offset,
  ExceptionInfo *exception)
{
  Image
    *image;

  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  image=CloneImage(GetImageFromList(images,(ssize_t) offset),0,0,MagickTrue,
    exception);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e L i s t I n d e x                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageListIndex() returns the position in the list of the specified
%  image.
%
%  Deprecated, replace with:
%
%    GetImageIndexInList(images);
%
%  The format of the GetImageListIndex method is:
%
%      ssize_t GetImageListIndex(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport ssize_t GetImageListIndex(const Image *images)
{
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  return(GetImageIndexInList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e L i s t S i z e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageListSize() returns the number of images in the list.
%
%  Deprecated, replace with:
%
%    GetImageListLength(images);
%
%  The format of the GetImageListSize method is:
%
%      size_t GetImageListSize(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport size_t GetImageListSize(const Image *images)
{
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  return(GetImageListLength(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e P i x e l s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImagePixels() obtains a pixel region for read/write access. If the
%  region is successfully accessed, a pointer to a PixelPacket array
%  representing the region is returned, otherwise NULL is returned.
%
%  The returned pointer may point to a temporary working copy of the pixels
%  or it may point to the original pixels in memory. Performance is maximized
%  if the selected region is part of one row, or one or more full rows, since
%  then there is opportunity to access the pixels in-place (without a copy)
%  if the image is in RAM, or in a memory-mapped file. The returned pointer
%  should *never* be deallocated by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  PixelPacket. If the image type is CMYK or if the storage class is
%  PseduoClass, call GetAuthenticIndexQueue() after invoking GetImagePixels()
%  to obtain the black color component or colormap indexes (of type IndexPacket)
%  corresponding to the region.  Once the PixelPacket (and/or IndexPacket)
%  array has been updated, the changes must be saved back to the underlying
%  image using SyncAuthenticPixels() or they may be lost.
%
%  Deprecated, replace with:
%
%    GetAuthenticPixels(image,x,y,columns,rows,&image->exception);
%
%  The format of the GetImagePixels() method is:
%
%      PixelPacket *GetImagePixels(Image *image,const ssize_t x,const ssize_t y,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *GetImagePixels(Image *image,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows)
{
  return(GetAuthenticPixels(image,x,y,columns,rows,&image->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I n d e x e s                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetIndexes() returns the black channel or the colormap indexes associated
%  with the last call to QueueAuthenticPixels() or GetVirtualPixels().  NULL is
%  returned if the black channel or colormap indexes are not available.
%
%  Deprecated, replace with:
%
%    GetAuthenticIndexQueue(image);
%
%  The format of the GetIndexes() method is:
%
%      IndexPacket *GetIndexes(const Image *image)
%
%  A description of each parameter follows:
%
%    o indexes: GetIndexes() returns the indexes associated with the last
%      call to QueueAuthenticPixels() or GetAuthenticPixels().
%
%    o image: the image.
%
*/
MagickExport IndexPacket *GetIndexes(const Image *image)
{
  return(GetAuthenticIndexQueue(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k G e o m e t r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickGeometry() is similar to GetGeometry() except the returned
%  geometry is modified as determined by the meta characters:  %, !, <, >,
%  and ~.
%
%  Deprecated, replace with:
%
%    ParseMetaGeometry(geometry,x,y,width,height);
%
%  The format of the GetMagickGeometry method is:
%
%      unsigned int GetMagickGeometry(const char *geometry,ssize_t *x,ssize_t *y,
%        size_t *width,size_t *height)
%
%  A description of each parameter follows:
%
%    o geometry:  Specifies a character string representing the geometry
%      specification.
%
%    o x,y:  A pointer to an integer.  The x and y offset as determined by
%      the geometry specification is returned here.
%
%    o width,height:  A pointer to an unsigned integer.  The width and height
%      as determined by the geometry specification is returned here.
%
*/
MagickExport unsigned int GetMagickGeometry(const char *geometry,ssize_t *x,
  ssize_t *y,size_t *width,size_t *height)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.3");
  return(ParseMetaGeometry(geometry,x,y,width,height));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImage() returns the next image in a list.
%
%  Deprecated, replace with:
%
%    GetNextImageInList(images);
%
%  The format of the GetNextImage method is:
%
%      Image *GetNextImage(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *GetNextImage(const Image *images)
{
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  return(GetNextImageInList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e A t t r i b u t e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImageAttribute() gets the next image attribute.
%
%  Deprecated, replace with:
%
%    const char *property;
%    property=GetNextImageProperty(image);
%    if (property != (const char *) NULL)
%      GetImageAttribute(image,property);
%
%  The format of the GetNextImageAttribute method is:
%
%      const ImageAttribute *GetNextImageAttribute(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const ImageAttribute *GetNextImageAttribute(const Image *image)
{
  const char
    *property;

  property=GetNextImageProperty(image);
  if (property == (const char *) NULL)
    return((const ImageAttribute *) NULL);
  return(GetImageAttribute(image,property));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N u m b e r S c e n e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNumberScenes() returns the number of images in the list.
%
%  Deprecated, replace with:
%
%    GetImageListLength(image);
%
%  The format of the GetNumberScenes method is:
%
%      unsigned int GetNumberScenes(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport unsigned int GetNumberScenes(const Image *image)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  return((unsigned int) GetImageListLength(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e P i x e l                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOnePixel() returns a single pixel at the specified (x,y) location.
%  The image background color is returned if an error occurs.
%
%  Deprecated, replace with:
%
%    GetOneAuthenticPixel(image,x,y,&pixel,&image->exception);
%
%  The format of the GetOnePixel() method is:
%
%      PixelPacket GetOnePixel(const Image image,const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
*/
MagickExport PixelPacket GetOnePixel(Image *image,const ssize_t x,const ssize_t y)
{
  PixelPacket
    pixel;

  (void) GetOneAuthenticPixel(image,x,y,&pixel,&image->exception);
  return(pixel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P i x e l s                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPixels() returns the pixels associated with the last call to
%  QueueAuthenticPixels() or GetAuthenticPixels().
%
%  Deprecated, replace with:
%
%    GetAuthenticPixelQueue(image);
%
%  The format of the GetPixels() method is:
%
%      PixelPacket *GetPixels(const Image image)
%
%  A description of each parameter follows:
%
%    o pixels: GetPixels() returns the pixels associated with the last call
%      to QueueAuthenticPixels() or GetAuthenticPixels().
%
%    o image: the image.
%
*/
MagickExport PixelPacket *GetPixels(const Image *image)
{
  return(GetAuthenticPixelQueue(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P r e v i o u s I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPreviousImage() returns the previous image in a list.
%
%  Deprecated, replace with:
%
%    GetPreviousImageInList(images));
%
%  The format of the GetPreviousImage method is:
%
%      Image *GetPreviousImage(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *GetPreviousImage(const Image *images)
{
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  return(GetPreviousImageInList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H S L T r a n s f o r m                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HSLTransform() converts a (hue, saturation, lightness) to a (red, green,
%  blue) triple.
%
%  The format of the HSLTransformImage method is:
%
%      void HSLTransform(const double hue,const double saturation,
%        const double lightness,Quantum *red,Quantum *green,Quantum *blue)
%
%  A description of each parameter follows:
%
%    o hue, saturation, lightness: A double value representing a
%      component of the HSL color space.
%
%    o red, green, blue: A pointer to a pixel component of type Quantum.
%
*/

static inline MagickRealType HueToRGB(MagickRealType m1,MagickRealType m2,
  MagickRealType hue)
{
  if (hue < 0.0)
    hue+=1.0;
  if (hue > 1.0)
    hue-=1.0;
  if ((6.0*hue) < 1.0)
    return(m1+6.0*(m2-m1)*hue);
  if ((2.0*hue) < 1.0)
    return(m2);
  if ((3.0*hue) < 2.0)
    return(m1+6.0*(m2-m1)*(2.0/3.0-hue));
  return(m1);
}

MagickExport void HSLTransform(const double hue,const double saturation,
  const double lightness,Quantum *red,Quantum *green,Quantum *blue)
{
  MagickRealType
    b,
    g,
    r,
    m1,
    m2;

  /*
    Convert HSL to RGB colorspace.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  if (lightness <= 0.5)
    m2=lightness*(saturation+1.0);
  else
    m2=lightness+saturation-lightness*saturation;
  m1=2.0*lightness-m2;
  r=HueToRGB(m1,m2,hue+1.0/3.0);
  g=HueToRGB(m1,m2,hue);
  b=HueToRGB(m1,m2,hue-1.0/3.0);
  *red=ClampToQuantum((MagickRealType) QuantumRange*r);
  *green=ClampToQuantum((MagickRealType) QuantumRange*g);
  *blue=ClampToQuantum((MagickRealType) QuantumRange*b);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I d e n t i t y A f f i n e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentityAffine() initializes the affine transform to the identity matrix.
%
%  The format of the IdentityAffine method is:
%
%      IdentityAffine(AffineMatrix *affine)
%
%  A description of each parameter follows:
%
%    o affine: A pointer the affine transform of type AffineMatrix.
%
*/
MagickExport void IdentityAffine(AffineMatrix *affine)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  assert(affine != (AffineMatrix *) NULL);
  (void) ResetMagickMemory(affine,0,sizeof(AffineMatrix));
  affine->sx=1.0;
  affine->sy=1.0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e M a g i c k                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeMagick() initializes the ImageMagick environment.
%
%  Deprecated, replace with:
%
%    MagickCoreGenesis(path,MagickFalse);
%
%  The format of the InitializeMagick function is:
%
%      InitializeMagick(const char *path)
%
%  A description of each parameter follows:
%
%    o path: the execution path of the current ImageMagick client.
%
*/
MagickExport void InitializeMagick(const char *path)
{
  MagickCoreGenesis(path,MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p o l a t e P i x e l C o l o r                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpolatePixelColor() applies bi-linear or tri-linear interpolation
%  between a pixel and it's neighbors.
%
%  The format of the InterpolatePixelColor method is:
%
%      MagickPixelPacket InterpolatePixelColor(const Image *image,
%        CacheView *view_info,InterpolatePixelMethod method,const double x,
%        const double y,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o image_view: the image cache view.
%
%    o type:  the type of pixel color interpolation.
%
%    o x,y: A double representing the current (x,y) position of the pixel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static void BicubicInterpolate(const MagickPixelPacket *pixels,const double dx,
  MagickPixelPacket *pixel)
{
  MagickRealType
    dx2,
    p,
    q,
    r,
    s;

  dx2=dx*dx;
  p=(pixels[3].red-pixels[2].red)-(pixels[0].red-pixels[1].red);
  q=(pixels[0].red-pixels[1].red)-p;
  r=pixels[2].red-pixels[0].red;
  s=pixels[1].red;
  pixel->red=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].green-pixels[2].green)-(pixels[0].green-pixels[1].green);
  q=(pixels[0].green-pixels[1].green)-p;
  r=pixels[2].green-pixels[0].green;
  s=pixels[1].green;
  pixel->green=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].blue-pixels[2].blue)-(pixels[0].blue-pixels[1].blue);
  q=(pixels[0].blue-pixels[1].blue)-p;
  r=pixels[2].blue-pixels[0].blue;
  s=pixels[1].blue;
  pixel->blue=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  p=(pixels[3].opacity-pixels[2].opacity)-(pixels[0].opacity-pixels[1].opacity);
  q=(pixels[0].opacity-pixels[1].opacity)-p;
  r=pixels[2].opacity-pixels[0].opacity;
  s=pixels[1].opacity;
  pixel->opacity=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
  if (pixel->colorspace == CMYKColorspace)
    {
      p=(pixels[3].index-pixels[2].index)-(pixels[0].index-pixels[1].index);
      q=(pixels[0].index-pixels[1].index)-p;
      r=pixels[2].index-pixels[0].index;
      s=pixels[1].index;
      pixel->index=(dx*dx2*p)+(dx2*q)+(dx*r)+s;
    }
}

static inline MagickRealType CubicWeightingFunction(const MagickRealType x)
{
  MagickRealType
    alpha,
    gamma;

  alpha=MagickMax(x+2.0,0.0);
  gamma=1.0*alpha*alpha*alpha;
  alpha=MagickMax(x+1.0,0.0);
  gamma-=4.0*alpha*alpha*alpha;
  alpha=MagickMax(x+0.0,0.0);
  gamma+=6.0*alpha*alpha*alpha;
  alpha=MagickMax(x-1.0,0.0);
  gamma-=4.0*alpha*alpha*alpha;
  return(gamma/6.0);
}

static inline double MeshInterpolate(const PointInfo *delta,const double p,
  const double x,const double y)
{
  return(delta->x*x+delta->y*y+(1.0-delta->x-delta->y)*p);
}

static inline ssize_t NearestNeighbor(MagickRealType x)
{
  if (x >= 0.0)
    return((ssize_t) (x+0.5));
  return((ssize_t) (x-0.5));
}

MagickExport MagickPixelPacket InterpolatePixelColor(const Image *image,
  CacheView *image_view,const InterpolatePixelMethod method,const double x,
  const double y,ExceptionInfo *exception)
{
  MagickPixelPacket
    pixel;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(image_view != (CacheView *) NULL);
  GetMagickPixelPacket(image,&pixel);
  switch (method)
  {
    case AverageInterpolatePixel:
    {
      MagickPixelPacket
        pixels[16];

      MagickRealType
        alpha[16],
        gamma;

      p=GetCacheViewVirtualPixels(image_view,(ssize_t) floor(x)-1,(ssize_t)
        floor(y)-1,4,4,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 16L; i++)
      {
        GetMagickPixelPacket(image,pixels+i);
        SetMagickPixelPacket(image,p,indexes+i,pixels+i);
        alpha[i]=1.0;
        if (image->matte != MagickFalse)
          {
            alpha[i]=QuantumScale*((MagickRealType) GetPixelAlpha(p));
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        gamma=alpha[i];
        gamma=PerceptibleReciprocal(gamma);
        pixel.red+=gamma*0.0625*pixels[i].red;
        pixel.green+=gamma*0.0625*pixels[i].green;
        pixel.blue+=gamma*0.0625*pixels[i].blue;
        pixel.opacity+=0.0625*pixels[i].opacity;
        if (image->colorspace == CMYKColorspace)
          pixel.index+=gamma*0.0625*pixels[i].index;
        p++;
      }
      break;
    }
    case BicubicInterpolatePixel:
    {
      MagickPixelPacket
        pixels[16],
        u[4];

      MagickRealType
        alpha[16];

      PointInfo
        delta;

      p=GetCacheViewVirtualPixels(image_view,(ssize_t) floor(x)-1,(ssize_t)
        floor(y)-1,4,4,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 16L; i++)
      {
        GetMagickPixelPacket(image,pixels+i);
        SetMagickPixelPacket(image,p,indexes+i,pixels+i);
        alpha[i]=1.0;
        if (image->matte != MagickFalse)
          {
            alpha[i]=QuantumScale*((MagickRealType) GetPixelAlpha(p));
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        p++;
      }
      delta.x=x-floor(x);
      for (i=0; i < 4L; i++)
        BicubicInterpolate(pixels+4*i,delta.x,u+i);
      delta.y=y-floor(y);
      BicubicInterpolate(u,delta.y,&pixel);
      break;
    }
    case BilinearInterpolatePixel:
    default:
    {
      MagickPixelPacket
        pixels[16];

      MagickRealType
        alpha[16],
        gamma;

      PointInfo
        delta;

      p=GetCacheViewVirtualPixels(image_view,(ssize_t) floor(x),(ssize_t)
        floor(y),2,2,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 4L; i++)
      {
        GetMagickPixelPacket(image,pixels+i);
        SetMagickPixelPacket(image,p,indexes+i,pixels+i);
        alpha[i]=1.0;
        if (image->matte != MagickFalse)
          {
            alpha[i]=QuantumScale*((MagickRealType) GetPixelAlpha(p));
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        p++;
      }
      delta.x=x-floor(x);
      delta.y=y-floor(y);
      gamma=(((1.0-delta.y)*((1.0-delta.x)*alpha[0]+delta.x*alpha[1])+delta.y*
        ((1.0-delta.x)*alpha[2]+delta.x*alpha[3])));
      gamma=PerceptibleReciprocal(gamma);
      pixel.red=gamma*((1.0-delta.y)*((1.0-delta.x)*pixels[0].red+delta.x*
        pixels[1].red)+delta.y*((1.0-delta.x)*pixels[2].red+delta.x*
        pixels[3].red));
      pixel.green=gamma*((1.0-delta.y)*((1.0-delta.x)*pixels[0].green+delta.x*
        pixels[1].green)+delta.y*((1.0-delta.x)*pixels[2].green+
        delta.x*pixels[3].green));
      pixel.blue=gamma*((1.0-delta.y)*((1.0-delta.x)*pixels[0].blue+delta.x*
        pixels[1].blue)+delta.y*((1.0-delta.x)*pixels[2].blue+delta.x*
        pixels[3].blue));
      pixel.opacity=((1.0-delta.y)*((1.0-delta.x)*pixels[0].opacity+delta.x*
        pixels[1].opacity)+delta.y*((1.0-delta.x)*pixels[2].opacity+delta.x*
        pixels[3].opacity));
      if (image->colorspace == CMYKColorspace)
        pixel.index=gamma*((1.0-delta.y)*((1.0-delta.x)*pixels[0].index+delta.x*
          pixels[1].index)+delta.y*((1.0-delta.x)*pixels[2].index+delta.x*
          pixels[3].index));
      break;
    }
    case FilterInterpolatePixel:
    {
      Image
        *excerpt_image,
        *filter_image;

      MagickPixelPacket
        pixels[1];

      RectangleInfo
        geometry;

      geometry.width=4L;
      geometry.height=4L;
      geometry.x=(ssize_t) floor(x)-1L;
      geometry.y=(ssize_t) floor(y)-1L;
      excerpt_image=ExcerptImage(image,&geometry,exception);
      if (excerpt_image == (Image *) NULL)
        break;
      filter_image=ResizeImage(excerpt_image,1,1,image->filter,image->blur,
        exception);
      excerpt_image=DestroyImage(excerpt_image);
      if (filter_image == (Image *) NULL)
        break;
      p=GetVirtualPixels(filter_image,0,0,1,1,exception);
      if (p == (const PixelPacket *) NULL)
        {
          filter_image=DestroyImage(filter_image);
          break;
        }
      indexes=GetVirtualIndexQueue(filter_image);
      GetMagickPixelPacket(image,pixels);
      SetMagickPixelPacket(image,p,indexes,&pixel);
      filter_image=DestroyImage(filter_image);
      break;
    }
    case IntegerInterpolatePixel:
    {
      MagickPixelPacket
        pixels[1];

      p=GetCacheViewVirtualPixels(image_view,(ssize_t) floor(x),(ssize_t)
        floor(y),1,1,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      GetMagickPixelPacket(image,pixels);
      SetMagickPixelPacket(image,p,indexes,&pixel);
      break;
    }
    case MeshInterpolatePixel:
    {
      MagickPixelPacket
        pixels[4];

      MagickRealType
        alpha[4],
        gamma;

      PointInfo
        delta,
        luminance;

      p=GetCacheViewVirtualPixels(image_view,(ssize_t) floor(x),(ssize_t)
        floor(y),2,2,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      for (i=0; i < 4L; i++)
      {
        GetMagickPixelPacket(image,pixels+i);
        SetMagickPixelPacket(image,p,indexes+i,pixels+i);
        alpha[i]=1.0;
        if (image->matte != MagickFalse)
          {
            alpha[i]=QuantumScale*((MagickRealType) GetPixelAlpha(p));
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        p++;
      }
      delta.x=x-floor(x);
      delta.y=y-floor(y);
      luminance.x=MagickPixelLuminance(pixels+0)-MagickPixelLuminance(pixels+3);
      luminance.y=MagickPixelLuminance(pixels+1)-MagickPixelLuminance(pixels+2);
      if (fabs(luminance.x) < fabs(luminance.y))
        {
          /*
            Diagonal 0-3 NW-SE.
          */
          if (delta.x <= delta.y)
            {
              /*
                Bottom-left triangle  (pixel:2, diagonal: 0-3).
              */
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[2],alpha[3],alpha[0]);
              gamma=PerceptibleReciprocal(gamma);
              pixel.red=gamma*MeshInterpolate(&delta,pixels[2].red,
                pixels[3].red,pixels[0].red);
              pixel.green=gamma*MeshInterpolate(&delta,pixels[2].green,
                pixels[3].green,pixels[0].green);
              pixel.blue=gamma*MeshInterpolate(&delta,pixels[2].blue,
                pixels[3].blue,pixels[0].blue);
              pixel.opacity=gamma*MeshInterpolate(&delta,pixels[2].opacity,
                pixels[3].opacity,pixels[0].opacity);
              if (image->colorspace == CMYKColorspace)
                pixel.index=gamma*MeshInterpolate(&delta,pixels[2].index,
                  pixels[3].index,pixels[0].index);
            }
          else
            {
              /*
                Top-right triangle (pixel:1, diagonal: 0-3).
              */
              delta.x=1.0-delta.x;
              gamma=MeshInterpolate(&delta,alpha[1],alpha[0],alpha[3]);
              gamma=PerceptibleReciprocal(gamma);
              pixel.red=gamma*MeshInterpolate(&delta,pixels[1].red,
                pixels[0].red,pixels[3].red);
              pixel.green=gamma*MeshInterpolate(&delta,pixels[1].green,
                pixels[0].green,pixels[3].green);
              pixel.blue=gamma*MeshInterpolate(&delta,pixels[1].blue,
                pixels[0].blue,pixels[3].blue);
              pixel.opacity=gamma*MeshInterpolate(&delta,pixels[1].opacity,
                pixels[0].opacity,pixels[3].opacity);
              if (image->colorspace == CMYKColorspace)
                pixel.index=gamma*MeshInterpolate(&delta,pixels[1].index,
                  pixels[0].index,pixels[3].index);
            }
        }
      else
        {
          /*
            Diagonal 1-2 NE-SW.
          */
          if (delta.x <= (1.0-delta.y))
            {
              /*
                Top-left triangle (pixel 0, diagonal: 1-2).
              */
              gamma=MeshInterpolate(&delta,alpha[0],alpha[1],alpha[2]);
              gamma=PerceptibleReciprocal(gamma);
              pixel.red=gamma*MeshInterpolate(&delta,pixels[0].red,
                pixels[1].red,pixels[2].red);
              pixel.green=gamma*MeshInterpolate(&delta,pixels[0].green,
                pixels[1].green,pixels[2].green);
              pixel.blue=gamma*MeshInterpolate(&delta,pixels[0].blue,
                pixels[1].blue,pixels[2].blue);
              pixel.opacity=gamma*MeshInterpolate(&delta,pixels[0].opacity,
                pixels[1].opacity,pixels[2].opacity);
              if (image->colorspace == CMYKColorspace)
                pixel.index=gamma*MeshInterpolate(&delta,pixels[0].index,
                  pixels[1].index,pixels[2].index);
            }
          else
            {
              /*
                Bottom-right triangle (pixel: 3, diagonal: 1-2).
              */
              delta.x=1.0-delta.x;
              delta.y=1.0-delta.y;
              gamma=MeshInterpolate(&delta,alpha[3],alpha[2],alpha[1]);
              gamma=PerceptibleReciprocal(gamma);
              pixel.red=gamma*MeshInterpolate(&delta,pixels[3].red,
                pixels[2].red,pixels[1].red);
              pixel.green=gamma*MeshInterpolate(&delta,pixels[3].green,
                pixels[2].green,pixels[1].green);
              pixel.blue=gamma*MeshInterpolate(&delta,pixels[3].blue,
                pixels[2].blue,pixels[1].blue);
              pixel.opacity=gamma*MeshInterpolate(&delta,pixels[3].opacity,
                pixels[2].opacity,pixels[1].opacity);
              if (image->colorspace == CMYKColorspace)
                pixel.index=gamma*MeshInterpolate(&delta,pixels[3].index,
                  pixels[2].index,pixels[1].index);
            }
        }
      break;
    }
    case NearestNeighborInterpolatePixel:
    {
      MagickPixelPacket
        pixels[1];

      p=GetCacheViewVirtualPixels(image_view,NearestNeighbor(x),
        NearestNeighbor(y),1,1,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      GetMagickPixelPacket(image,pixels);
      SetMagickPixelPacket(image,p,indexes,&pixel);
      break;
    }
    case SplineInterpolatePixel:
    {
      MagickPixelPacket
        pixels[16];

      MagickRealType
        alpha[16],
        dx,
        dy,
        gamma;

      PointInfo
        delta;

      ssize_t
        j,
        n;

      p=GetCacheViewVirtualPixels(image_view,(ssize_t) floor(x)-1,(ssize_t)
        floor(y)-1,4,4,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      n=0;
      delta.x=x-floor(x);
      delta.y=y-floor(y);
      for (i=(-1); i < 3L; i++)
      {
        dy=CubicWeightingFunction((MagickRealType) i-delta.y);
        for (j=(-1); j < 3L; j++)
        {
          GetMagickPixelPacket(image,pixels+n);
          SetMagickPixelPacket(image,p,indexes+n,pixels+n);
          alpha[n]=1.0;
          if (image->matte != MagickFalse)
            {
              alpha[n]=QuantumScale*((MagickRealType)
                GetPixelAlpha(p));
              pixels[n].red*=alpha[n];
              pixels[n].green*=alpha[n];
              pixels[n].blue*=alpha[n];
              if (image->colorspace == CMYKColorspace)
                pixels[n].index*=alpha[n];
            }
          dx=CubicWeightingFunction(delta.x-(MagickRealType) j);
          gamma=alpha[n];
          gamma=PerceptibleReciprocal(gamma);
          pixel.red+=gamma*dx*dy*pixels[n].red;
          pixel.green+=gamma*dx*dy*pixels[n].green;
          pixel.blue+=gamma*dx*dy*pixels[n].blue;
          if (image->matte != MagickFalse)
            pixel.opacity+=dx*dy*pixels[n].opacity;
          if (image->colorspace == CMYKColorspace)
            pixel.index+=gamma*dx*dy*pixels[n].index;
          n++;
          p++;
        }
      }
      break;
    }
  }
  return(pixel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p r e t I m a g e A t t r i b u t e s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretImageAttributes() replaces any embedded formatting characters with
%  the appropriate image attribute and returns the translated text.
%
%  Deprecated, replace with:
%
%    InterpretImageProperties(image_info,image,embed_text);
%
%  The format of the InterpretImageAttributes method is:
%
%      char *InterpretImageAttributes(const ImageInfo *image_info,Image *image,
%        const char *embed_text)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o embed_text: the address of a character string containing the embedded
%      formatting characters.
%
*/
MagickExport char *InterpretImageAttributes(const ImageInfo *image_info,
  Image *image,const char *embed_text)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.3.1");
  return(InterpretImageProperties(image_info,image,embed_text));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n v e r s e s R G B C o m p a n d o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InversesRGBCompandor() removes the gamma function from a sRGB pixel.
%
%  The format of the InversesRGBCompandor method is:
%
%      MagickRealType InversesRGBCompandor(const MagickRealType pixel)
%
%  A description of each parameter follows:
%
%    o pixel: the pixel.
%
*/
MagickExport MagickRealType InversesRGBCompandor(const MagickRealType pixel)
{
  if (pixel <= (0.0404482362771076*QuantumRange))
    return(pixel/12.92);
  return(QuantumRange*pow((QuantumScale*pixel+0.055)/1.055,2.4));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     I s S u b i m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSubimage() returns MagickTrue if the geometry is a valid subimage
%  specification (e.g. [1], [1-9], [1,7,4]).
%
%  The format of the IsSubimage method is:
%
%      unsigned int IsSubimage(const char *geometry,const unsigned int pedantic)
%
%  A description of each parameter follows:
%
%    o geometry: This string is the geometry specification.
%
%    o pedantic: A value other than 0 invokes a more restrictive set of
%      conditions for a valid specification (e.g. [1], [1-4], [4-1]).
%
*/
MagickExport unsigned int IsSubimage(const char *geometry,
  const unsigned int pedantic)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  if (geometry == (const char *) NULL)
    return(MagickFalse);
  if ((strchr(geometry,'x') != (char *) NULL) ||
      (strchr(geometry,'X') != (char *) NULL))
    return(MagickFalse);
  if ((pedantic != MagickFalse) && (strchr(geometry,',') != (char *) NULL))
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l I m a g e C o l o r s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelImageColor() will map the given color to "black" and "white"
%  values, limearly spreading out the colors, and level values on a channel by
%  channel bases, as per LevelImage().  The given colors allows you to specify
%  different level ranges for each of the color channels separately.
%
%  If the boolean 'invert' is set true the image values will modifyed in the
%  reverse direction. That is any existing "black" and "white" colors in the
%  image will become the color values given, with all other values compressed
%  appropriatally.  This effectivally maps a greyscale gradient into the given
%  color gradient.
%
%  Deprecated, replace with:
%
%    LevelColorsImageChannel(image,channel,black_color,white_color,invert);
%
%  The format of the LevelImageColors method is:
%
%  MagickBooleanType LevelImageColors(Image *image,const ChannelType channel,
%    const MagickPixelPacket *black_color,const MagickPixelPacket *white_color,
%    const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o black_color: The color to map black to/from
%
%    o white_point: The color to map white to/from
%
%    o invert: if true map the colors (levelize), rather than from (level)
%
*/
MagickBooleanType LevelImageColors(Image *image,const ChannelType channel,
  const MagickPixelPacket *black_color,const MagickPixelPacket *white_color,
  const MagickBooleanType invert)
{
  return(LevelColorsImageChannel(image,channel,black_color,white_color,invert));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i b e r a t e M e m o r y                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LiberateMemory() frees memory that has already been allocated, and NULL's
%  the pointer to it.
%
%  The format of the LiberateMemory method is:
%
%      void LiberateMemory(void **memory)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a block of memory to free for reuse.
%
*/
MagickExport void LiberateMemory(void **memory)
{
  assert(memory != (void **) NULL);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  if (*memory == (void *) NULL)
    return;
  free(*memory);
  *memory=(void *) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i b e r a t e S e m a p h o r e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LiberateSemaphoreInfo() relinquishes a semaphore.
%
%  Deprecated, replace with:
%
%    UnlockSemaphoreInfo(*semaphore_info);
%
%  The format of the LiberateSemaphoreInfo method is:
%
%      LiberateSemaphoreInfo(void **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void LiberateSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  UnlockSemaphoreInfo(*semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I n c a r n a t e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickIncarnate() initializes the ImageMagick environment.
%
%  Deprecated, replace with:
%
%    MagickCoreGenesis(path,MagickFalse);
%
%  The format of the MagickIncarnate function is:
%
%      MagickIncarnate(const char *path)
%
%  A description of each parameter follows:
%
%    o path: the execution path of the current ImageMagick client.
%
*/

MagickExport void MagickIncarnate(const char *path)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.1");
  MagickCoreGenesis(path,MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o n i t o r                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMonitor() calls the monitor handler method with a text string that
%  describes the task and a measure of completion.  The method returns
%  MagickTrue on success otherwise MagickFalse if an error is encountered, e.g.
%  if there was a user interrupt.
%
%  The format of the MagickMonitor method is:
%
%      MagickBooleanType MagickMonitor(const char *text,
%        const MagickOffsetType offset,const MagickSizeType span,
%        void *client_data)
%
%  A description of each parameter follows:
%
%    o offset: the position relative to the span parameter which represents
%      how much progress has been made toward completing a task.
%
%    o span: the span relative to completing a task.
%
%    o client_data: the client data.
%
*/
MagickExport MagickBooleanType MagickMonitor(const char *text,
  const MagickOffsetType offset,const MagickSizeType span,
  void *magick_unused(client_data))
{
  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  assert(text != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",text);
  ProcessPendingEvents(text);
  status=MagickTrue;
  exception=AcquireExceptionInfo();
  if (monitor_handler != (MonitorHandler) NULL)
    status=(*monitor_handler)(text,offset,span,exception);
  exception=DestroyExceptionInfo(exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a p I m a g e                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MapImage() replaces the colors of an image with the closest color from a
%  reference image.
%
%  Deprecated, replace with:
%
%     QuantizeInfo quantize_info;
%     GetQuantizeInfo(&quantize_info);
%     quantize_info.dither=dither;
%     RemapImage(&quantize_info,image,map_image);
%
%  The format of the MapImage method is:
%
%      MagickBooleanType MapImage(Image *image,const Image *map_image,
%        const MagickBooleanType dither)
%
%  A description of each parameter follows:
%
%    o image: Specifies a pointer to an Image structure.
%
%    o map_image: the image.  Reduce image to a set of colors represented by
%      this image.
%
%    o dither: Set this integer value to something other than zero to
%      dither the mapped image.
%
*/
MagickExport MagickBooleanType MapImage(Image *image,const Image *map_image,
  const MagickBooleanType dither)
{
  QuantizeInfo
    quantize_info;

  /*
    Initialize color cube.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(map_image != (Image *) NULL);
  assert(map_image->signature == MagickSignature);
  GetQuantizeInfo(&quantize_info);
  quantize_info.dither=dither;
  return(RemapImage(&quantize_info,image,map_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a p I m a g e s                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MapImages() replaces the colors of a sequence of images with the closest
%  color from a reference image.
%
%  Deprecated, replace with:
%
%     QuantizeInfo quantize_info;
%     GetQuantizeInfo(&quantize_info);
%     quantize_info.dither=dither;
%     RemapImages(&quantize_info,images,map_image);
%
%  The format of the MapImage method is:
%
%      MagickBooleanType MapImages(Image *images,Image *map_image,
%        const MagickBooleanType dither)
%
%  A description of each parameter follows:
%
%    o image: Specifies a pointer to a set of Image structures.
%
%    o map_image: the image.  Reduce image to a set of colors represented by
%      this image.
%
%    o dither: Set this integer value to something other than zero to
%      dither the quantized image.
%
*/
MagickExport MagickBooleanType MapImages(Image *images,const Image *map_image,
  const MagickBooleanType dither)
{
  QuantizeInfo
    quantize_info;

  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  GetQuantizeInfo(&quantize_info);
  quantize_info.dither=dither;
  return(RemapImages(&quantize_info,images,map_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a t t e F l o o d f i l l I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MatteFloodfill() changes the transparency value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod
%  is specified, the transparency value is changed for any neighbor pixel
%  that does not match the bordercolor member of image.
%
%  By default target must match a particular pixel transparency exactly.
%  However, in many cases two transparency values may differ by a
%  small amount.  The fuzz member of image defines how much tolerance is
%  acceptable to consider two transparency values as the same.  For example,
%  set fuzz to 10 and the opacity values of 100 and 102 respectively are
%  now interpreted as the same value for the purposes of the floodfill.
%
%  The format of the MatteFloodfillImage method is:
%
%      MagickBooleanType MatteFloodfillImage(Image *image,
%        const PixelPacket target,const Quantum opacity,const ssize_t x_offset,
%        const ssize_t y_offset,const PaintMethod method)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target: the RGB value of the target color.
%
%    o opacity: the level of transparency: 0 is fully opaque and QuantumRange is
%      fully transparent.
%
%    o x,y: the starting location of the operation.
%
%    o method:  Choose either FloodfillMethod or FillToBorderMethod.
%
*/
MagickExport MagickBooleanType MatteFloodfillImage(Image *image,
  const PixelPacket target,const Quantum opacity,const ssize_t x_offset,
  const ssize_t y_offset,const PaintMethod method)
{
  Image
    *floodplane_image;

  MagickBooleanType
    skip;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  ssize_t
    offset,
    start,
    x,
    x1,
    x2,
    y;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((x_offset < 0) || (x_offset >= (ssize_t) image->columns))
    return(MagickFalse);
  if ((y_offset < 0) || (y_offset >= (ssize_t) image->rows))
    return(MagickFalse);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  floodplane_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    &image->exception);
  if (floodplane_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageAlphaChannel(floodplane_image,OpaqueAlphaChannel);
  /*
    Set floodfill color.
  */
  segment_stack=(SegmentInfo *) AcquireQuantumMemory(MaxStacksize,
    sizeof(*segment_stack));
  if (segment_stack == (SegmentInfo *) NULL)
    {
      floodplane_image=DestroyImage(floodplane_image);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Push initial segment on stack.
  */
  x=x_offset;
  y=y_offset;
  start=0;
  s=segment_stack;
  PushSegmentStack(y,x,x,1);
  PushSegmentStack(y+1,x,x,-1);
  while (s > segment_stack)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    /*
      Pop segment off stack.
    */
    s--;
    x1=(ssize_t) s->x1;
    x2=(ssize_t) s->x2;
    offset=(ssize_t) s->y2;
    y=(ssize_t) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    p=GetVirtualPixels(image,0,y,(size_t) (x1+1),1,&image->exception);
    q=GetAuthenticPixels(floodplane_image,0,y,(size_t) (x1+1),1,
      &image->exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    p+=x1;
    q+=x1;
    for (x=x1; x >= 0; x--)
    {
      if (q->opacity == (Quantum) TransparentOpacity)
        break;
      if (method == FloodfillMethod)
        {
          if (IsColorSimilar(image,p,&target) == MagickFalse)
            break;
        }
      else
        if (IsColorSimilar(image,p,&target) != MagickFalse)
          break;
      q->opacity=(Quantum) TransparentOpacity;
      q--;
      p--;
    }
    if (SyncAuthenticPixels(floodplane_image,&image->exception) == MagickFalse)
      break;
    skip=x >= x1 ? MagickTrue : MagickFalse;
    if (skip == MagickFalse)
      {
        start=x+1;
        if (start < x1)
          PushSegmentStack(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (skip == MagickFalse)
        {
          if (x < (ssize_t) image->columns)
            {
              p=GetVirtualPixels(image,x,y,image->columns-x,1,
                &image->exception);
              q=GetAuthenticPixels(floodplane_image,x,y,image->columns-x,1,
                &image->exception);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for ( ; x < (ssize_t) image->columns; x++)
              {
                if (q->opacity == (Quantum) TransparentOpacity)
                  break;
                if (method == FloodfillMethod)
                  {
                    if (IsColorSimilar(image,p,&target) == MagickFalse)
                      break;
                  }
                else
                  if (IsColorSimilar(image,p,&target) != MagickFalse)
                    break;
                q->opacity=(Quantum) TransparentOpacity;
                q++;
                p++;
              }
              if (SyncAuthenticPixels(floodplane_image,&image->exception) == MagickFalse)
                break;
            }
          PushSegmentStack(y,start,x-1,offset);
          if (x > (x2+1))
            PushSegmentStack(y,x2+1,x-1,-offset);
        }
      skip=MagickFalse;
      x++;
      if (x <= x2)
        {
          p=GetVirtualPixels(image,x,y,(size_t) (x2-x+1),1,
            &image->exception);
          q=GetAuthenticPixels(floodplane_image,x,y,(size_t) (x2-x+1),1,
            &image->exception);
          if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
            break;
          for ( ; x <= x2; x++)
          {
            if (q->opacity == (Quantum) TransparentOpacity)
              break;
            if (method == FloodfillMethod)
              {
                if (IsColorSimilar(image,p,&target) != MagickFalse)
                  break;
              }
            else
              if (IsColorSimilar(image,p,&target) == MagickFalse)
                break;
            p++;
            q++;
          }
        }
      start=x;
    } while (x <= x2);
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    /*
      Tile fill color onto floodplane.
    */
    p=GetVirtualPixels(floodplane_image,0,y,image->columns,1,
      &image->exception);
    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelOpacity(p) != OpaqueOpacity)
        q->opacity=opacity;
      p++;
      q++;
    }
    if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
      break;
  }
  segment_stack=(SegmentInfo *) RelinquishMagickMemory(segment_stack);
  floodplane_image=DestroyImage(floodplane_image);
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a x i m u m I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MaximumImages() returns the maximum intensity of an image sequence.
%
%  Deprecated, replace with:
%
%    EvaluateImages(images,MinEvaluateOperator,exception);
%
%  The format of the MaxImages method is:
%
%      Image *MaximumImages(Image *images,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MaximumImages(const Image *images,ExceptionInfo *exception)
{
  return(EvaluateImages(images,MinEvaluateOperator,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M i n i m u m I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MinimumImages() returns the minimum intensity of an image sequence.
%
%  Deprecated, replace with:
%
%    EvaluateImages(images,MinEvaluateOperator,exception);
%
%  The format of the MinimumImages method is:
%
%      Image *MinimumImages(Image *images,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MinimumImages(const Image *images,ExceptionInfo *exception)
{
  return(EvaluateImages(images,MinEvaluateOperator,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M e d i a n F i l t e r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MedianFilterImage() applies a digital filter that improves the quality
%  of a noisy image.  Each pixel is replaced by the median in a set of
%  neighboring pixels as defined by radius.
%
%  The algorithm was contributed by Mike Edmonds and implements an insertion
%  sort for selecting median color-channel values.  For more on this algorithm
%  see "Skip Lists: A probabilistic Alternative to Balanced Trees" by William
%  Pugh in the June 1990 of Communications of the ACM.
%
%  The format of the MedianFilterImage method is:
%
%      Image *MedianFilterImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MedianFilterImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
  Image
    *median_image;

  median_image=StatisticImage(image,MedianStatistic,(size_t) radius,(size_t)
    radius,exception);
  return(median_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o d e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModeImage() makes each pixel the 'predominant color' of the neighborhood
%  of the specified radius.
%
%  The format of the ModeImage method is:
%
%      Image *ModeImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ModeImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
  Image
    *mode_image;

  mode_image=StatisticImage(image,ModeStatistic,(size_t) radius,(size_t) radius,
    exception);
  return(mode_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o s a i c I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MosaicImages() Obsolete Function: Use MergeImageLayers() instead.
%
%  Deprecated, replace with:
%
%    MergeImageLayers(image,MosaicLayer,exception);
%
%  The format of the MosaicImage method is:
%
%      Image *MosaicImages(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image list to be composited together
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MosaicImages(Image *image,ExceptionInfo *exception)
{
  return(MergeImageLayers(image,MosaicLayer,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O p a q u e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpaqueImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  The format of the OpaqueImage method is:
%
%      MagickBooleanType OpaqueImage(Image *image,
%        const PixelPacket *target,const PixelPacket fill)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target: the RGB value of the target color.
%
%    o fill: the replacement color.
%
*/
MagickExport MagickBooleanType OpaqueImage(Image *image,
  const PixelPacket target,const PixelPacket fill)
{
#define OpaqueImageTag  "Opaque/Image"

  MagickBooleanType
    proceed;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Make image color opaque.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.1.0");
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  switch (image->storage_class)
  {
    case DirectClass:
    default:
    {
      /*
        Make DirectClass image opaque.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        register ssize_t
          x;

        register PixelPacket
          *restrict q;

        q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          if (IsColorSimilar(image,q,&target) != MagickFalse)
            *q=fill;
          q++;
        }
        if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
          break;
        proceed=SetImageProgress(image,OpaqueImageTag,(MagickOffsetType) y,
          image->rows);
        if (proceed == MagickFalse)
          break;
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Make PseudoClass image opaque.
      */
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if (IsColorSimilar(image,&image->colormap[i],&target) != MagickFalse)
          image->colormap[i]=fill;
      }
      if (fill.opacity != OpaqueOpacity)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            register ssize_t
              x;

            register PixelPacket
              *restrict q;

            q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (IsColorSimilar(image,q,&target) != MagickFalse)
                q->opacity=fill.opacity;
              q++;
            }
            if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
              break;
          }
        }
      (void) SyncImage(image);
      break;
    }
  }
  if (fill.opacity != OpaqueOpacity)
    image->matte=MagickTrue;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O p e n C a c h e V i e w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenCacheView() opens a view into the pixel cache, using the
%  VirtualPixelMethod that is defined within the given image itself.
%
%  Deprecated, replace with:
%
%    AcquireVirtualCacheView(image,&image->exception);
%
%  The format of the OpenCacheView method is:
%
%      CacheView *OpenCacheView(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport CacheView *OpenCacheView(const Image *image)
{
  return(AcquireVirtualCacheView(image,&((Image *) image)->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O p e n M a g i c k S t r e a m                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenMagickStream() opens the file at the specified path and return the
%  associated stream.
%
%  The path of the OpenMagickStream method is:
%
%      FILE *OpenMagickStream(const char *path,const char *mode)
%
%  A description of each parameter follows.
%
%   o  path: the file path.
%
%   o  mode: the file mode.
%
*/

#if defined(MAGICKCORE_HAVE__WFOPEN)
static size_t UTF8ToUTF16(const unsigned char *utf8,wchar_t *utf16)
{
  register const unsigned char
    *p;

  if (utf16 != (wchar_t *) NULL)
    {
      register wchar_t
        *q;

      wchar_t
        c;

      /*
        Convert UTF-8 to UTF-16.
      */
      q=utf16;
      for (p=utf8; *p != '\0'; p++)
      {
        if ((*p & 0x80) == 0)
          *q=(*p);
        else
          if ((*p & 0xE0) == 0xC0)
            {
              c=(*p);
              *q=(c & 0x1F) << 6;
              p++;
              if ((*p & 0xC0) != 0x80)
                return(0);
              *q|=(*p & 0x3F);
            }
          else
            if ((*p & 0xF0) == 0xE0)
              {
                c=(*p);
                *q=c << 12;
                p++;
                if ((*p & 0xC0) != 0x80)
                  return(0);
                c=(*p);
                *q|=(c & 0x3F) << 6;
                p++;
                if ((*p & 0xC0) != 0x80)
                  return(0);
                *q|=(*p & 0x3F);
              }
            else
              return(0);
        q++;
      }
      *q++='\0';
      return(q-utf16);
    }
  /*
    Compute UTF-16 string length.
  */
  for (p=utf8; *p != '\0'; p++)
  {
    if ((*p & 0x80) == 0)
      ;
    else
      if ((*p & 0xE0) == 0xC0)
        {
          p++;
          if ((*p & 0xC0) != 0x80)
            return(0);
        }
      else
        if ((*p & 0xF0) == 0xE0)
          {
            p++;
            if ((*p & 0xC0) != 0x80)
              return(0);
            p++;
            if ((*p & 0xC0) != 0x80)
              return(0);
         }
       else
         return(0);
  }
  return(p-utf8);
}

static wchar_t *ConvertUTF8ToUTF16(const unsigned char *source)
{
  size_t
    length;

  wchar_t
    *utf16;

  length=UTF8ToUTF16(source,(wchar_t *) NULL);
  if (length == 0)
    {
      register ssize_t
        i;

      /*
        Not UTF-8, just copy.
      */
      length=strlen((const char *) source);
      utf16=(wchar_t *) AcquireQuantumMemory(length+1,sizeof(*utf16));
      if (utf16 == (wchar_t *) NULL)
        return((wchar_t *) NULL);
      for (i=0; i <= (ssize_t) length; i++)
        utf16[i]=source[i];
      return(utf16);
    }
  utf16=(wchar_t *) AcquireQuantumMemory(length+1,sizeof(*utf16));
  if (utf16 == (wchar_t *) NULL)
    return((wchar_t *) NULL);
  length=UTF8ToUTF16(source,utf16);
  return(utf16);
}
#endif

MagickExport FILE *OpenMagickStream(const char *path,const char *mode)
{
  FILE
    *file;

  if ((path == (const char *) NULL) || (mode == (const char *) NULL))
    {
      errno=EINVAL;
      return((FILE *) NULL);
    }
  file=(FILE *) NULL;
#if defined(MAGICKCORE_HAVE__WFOPEN)
  {
    wchar_t
      *unicode_mode,
      *unicode_path;

    unicode_path=ConvertUTF8ToUTF16((const unsigned char *) path);
    if (unicode_path == (wchar_t *) NULL)
      return((FILE *) NULL);
    unicode_mode=ConvertUTF8ToUTF16((const unsigned char *) mode);
    if (unicode_mode == (wchar_t *) NULL)
      {
        unicode_path=(wchar_t *) RelinquishMagickMemory(unicode_path);
        return((FILE *) NULL);
      }
    file=_wfopen(unicode_path,unicode_mode);
    unicode_mode=(wchar_t *) RelinquishMagickMemory(unicode_mode);
    unicode_path=(wchar_t *) RelinquishMagickMemory(unicode_path);
  }
#endif
  if (file == (FILE *) NULL)
    file=fopen(path,mode);
  return(file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a i n t F l o o d f i l l I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PaintFloodfill() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  By default target must match a particular pixel color exactly.
%  However, in many cases two colors may differ by a small amount.  The
%  fuzz member of image defines how much tolerance is acceptable to
%  consider two colors as the same.  For example, set fuzz to 10 and the
%  color red at intensities of 100 and 102 respectively are now
%  interpreted as the same color for the purposes of the floodfill.
%
%  Deprecated, replace with:
%
%    FloodfillPaintImage(image,channel,draw_info,target,x,y,
%      method == FloodfillMethod ? MagickFalse : MagickTrue);
%
%  The format of the PaintFloodfillImage method is:
%
%      MagickBooleanType PaintFloodfillImage(Image *image,
%        const ChannelType channel,const MagickPixelPacket target,
%        const ssize_t x,const ssize_t y,const DrawInfo *draw_info,
%        const PaintMethod method)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel(s).
%
%    o target: the RGB value of the target color.
%
%    o x,y: the starting location of the operation.
%
%    o draw_info: the draw info.
%
%    o method: Choose either FloodfillMethod or FillToBorderMethod.
%
*/
MagickExport MagickBooleanType PaintFloodfillImage(Image *image,
  const ChannelType channel,const MagickPixelPacket *target,const ssize_t x,
  const ssize_t y,const DrawInfo *draw_info,const PaintMethod method)
{
  MagickBooleanType
    status;

  status=FloodfillPaintImage(image,channel,draw_info,target,x,y,
    method == FloodfillMethod ? MagickFalse : MagickTrue);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     P a i n t O p a q u e I m a g e                                         %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PaintOpaqueImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  Deprecated, replace with:
%
%    OpaquePaintImageChannel(image,DefaultChannels,target,fill,MagickFalse);
%    OpaquePaintImageChannel(image,channel,target,fill,MagickFalse);
%
%  The format of the PaintOpaqueImage method is:
%
%      MagickBooleanType PaintOpaqueImage(Image *image,
%        const PixelPacket *target,const PixelPacket *fill)
%      MagickBooleanType PaintOpaqueImageChannel(Image *image,
%        const ChannelType channel,const PixelPacket *target,
%        const PixelPacket *fill)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel(s).
%
%    o target: the RGB value of the target color.
%
%    o fill: the replacement color.
%
*/

MagickExport MagickBooleanType PaintOpaqueImage(Image *image,
  const MagickPixelPacket *target,const MagickPixelPacket *fill)
{
  MagickBooleanType
    status;

  status=OpaquePaintImageChannel(image,DefaultChannels,target,fill,MagickFalse);
  return(status);
}

MagickExport MagickBooleanType PaintOpaqueImageChannel(Image *image,
  const ChannelType channel,const MagickPixelPacket *target,
  const MagickPixelPacket *fill)
{
  return(OpaquePaintImageChannel(image,channel,target,fill,MagickFalse));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P a i n t T r a n s p a r e n t I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PaintTransparentImage() changes the opacity value associated with any pixel
%  that matches color to the value defined by opacity.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  Deprecated, replace with:
%
%    TransparentPaintImage(image,target,opacity,MagickFalse);
%
%  The format of the PaintTransparentImage method is:
%
%      MagickBooleanType PaintTransparentImage(Image *image,
%        const MagickPixelPacket *target,const Quantum opacity)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target: the RGB value of the target color.
%
%    o opacity: the replacement opacity value.
%
*/
MagickExport MagickBooleanType PaintTransparentImage(Image *image,
  const MagickPixelPacket *target,const Quantum opacity)
{
  return(TransparentPaintImage(image,target,opacity,MagickFalse));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P a r s e I m a g e G e o m e t r y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseImageGeometry() is similar to GetGeometry() except the returned
%  geometry is modified as determined by the meta characters:  %, !, <,
%  and >.
%
%  Deprecated, replace with:
%
%    ParseMetaGeometry(geometry,x,y,width,height);
%
%  The format of the ParseImageGeometry method is:
%
%      int ParseImageGeometry(char *geometry,ssize_t *x,ssize_t *y,
%        size_t *width,size_t *height)
%
%  A description of each parameter follows:
%
%    o flags:  Method ParseImageGeometry returns a bitmask that indicates
%      which of the four values were located in the geometry string.
%
%    o image_geometry:  Specifies a character string representing the geometry
%      specification.
%
%    o x,y:  A pointer to an integer.  The x and y offset as determined by
%      the geometry specification is returned here.
%
%    o width,height:  A pointer to an unsigned integer.  The width and height
%      as determined by the geometry specification is returned here.
%
*/
MagickExport int ParseImageGeometry(const char *geometry,ssize_t *x,ssize_t *y,
  size_t *width,size_t *height)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.1");
  return((int) ParseMetaGeometry(geometry,x,y,width,height));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e S i z e G e o m e t r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseSizeGeometry() returns a region as defined by the geometry string with
%  respect to the image dimensions and aspect ratio.
%
%  Deprecated, replace with:
%
%    ParseMetaGeometry(geometry,&region_info->x,&region_info->y,
%      &region_info->width,&region_info->height);
%
%  The format of the ParseSizeGeometry method is:
%
%      MagickStatusType ParseSizeGeometry(const Image *image,
%        const char *geometry,RectangeInfo *region_info)
%
%  A description of each parameter follows:
%
%    o geometry:  The geometry (e.g. 100x100+10+10).
%
%    o region_info: the region as defined by the geometry string.
%
*/
MagickExport MagickStatusType ParseSizeGeometry(const Image *image,
  const char *geometry,RectangleInfo *region_info)
{
  MagickStatusType
    flags;

  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.4.7");
  SetGeometry(image,region_info);
  flags=ParseMetaGeometry(geometry,&region_info->x,&region_info->y,
    &region_info->width,&region_info->height);
  return(flags);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P o p I m a g e L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PopImageList() removes the last image in the list.
%
%  Deprecated, replace with:
%
%    RemoveLastImageFromList(images);
%
%  The format of the PopImageList method is:
%
%      Image *PopImageList(Image **images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *PopImageList(Image **images)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  return(RemoveLastImageFromList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P o p I m a g e P i x e l s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PopImagePixels() transfers one or more pixel components from the image pixel
%  cache to a user supplied buffer.  The pixels are returned in network byte
%  order.  MagickTrue is returned if the pixels are successfully transferred,
%  otherwise MagickFalse.
%
%  The format of the PopImagePixels method is:
%
%      size_t PopImagePixels(Image *,const QuantumType quantum,
%        unsigned char *destination)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum: Declare which pixel components to transfer (RGB, RGBA, etc).
%
%    o destination:  The components are transferred to this buffer.
%
*/
MagickExport size_t PopImagePixels(Image *image,const QuantumType quantum,
  unsigned char *destination)
{
  QuantumInfo
    *quantum_info;

  size_t
    length;

  quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
  if (quantum_info == (QuantumInfo *) NULL)
    return(0);
  length=ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
    quantum,destination,&image->exception);
  quantum_info=DestroyQuantumInfo(quantum_info);
  return(length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  P o s t s c r i p t G e o m e t r y                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PostscriptGeometry() replaces any page mneumonic with the equivalent size in
%  picas.
%
%  Deprecated, replace with:
%
%    GetPageGeometry(page);
%
%  The format of the PostscriptGeometry method is:
%
%      char *PostscriptGeometry(const char *page)
%
%  A description of each parameter follows.
%
%   o  page:  Specifies a pointer to an array of characters.
%      The string is either a Postscript page name (e.g. A4) or a postscript
%      page geometry (e.g. 612x792+36+36).
%
*/
MagickExport char *PostscriptGeometry(const char *page)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.1");
  return(GetPageGeometry(page));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P u s h I m a g e L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PushImageList() adds an image to the end of the list.
%
%  Deprecated, replace with:
%
%    AppendImageToList(images,CloneImageList(image,exception));
%
%  The format of the PushImageList method is:
%
%      unsigned int PushImageList(Image *images,const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport unsigned int PushImageList(Image **images,const Image *image,
  ExceptionInfo *exception)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  AppendImageToList(images,CloneImageList(image,exception));
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P u s h I m a g e P i x e l s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PushImagePixels() transfers one or more pixel components from a user
%  supplied buffer into the image pixel cache of an image.  The pixels are
%  expected in network byte order.  It returns MagickTrue if the pixels are
%  successfully transferred, otherwise MagickFalse.
%
%  The format of the PushImagePixels method is:
%
%      size_t PushImagePixels(Image *image,const QuantumType quantum,
%        const unsigned char *source)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum: Declare which pixel components to transfer (red, green, blue,
%      opacity, RGB, or RGBA).
%
%    o source:  The pixel components are transferred from this buffer.
%
*/
MagickExport size_t PushImagePixels(Image *image,const QuantumType quantum,
  const unsigned char *source)
{
  QuantumInfo
    *quantum_info;

  size_t
    length;

  quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
  if (quantum_info == (QuantumInfo *) NULL)
    return(0);
  length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,quantum,
    source,&image->exception);
  quantum_info=DestroyQuantumInfo(quantum_info);
  return(length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  Q u a n t i z a t i o n E r r o r                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QuantizationError() measures the difference between the original and
%  quantized images.  This difference is the total quantization error.  The
%  error is computed by summing over all pixels in an image the distance
%  squared in RGB space between each reference pixel value and its quantized
%  value.  These values are computed:
%
%    o mean_error_per_pixel:  This value is the mean error for any single
%      pixel in the image.
%
%    o normalized_mean_square_error:  This value is the normalized mean
%      quantization error for any single pixel in the image.  This distance
%      measure is normalized to a range between 0 and 1.  It is independent
%      of the range of red, green, and blue values in the image.
%
%    o normalized_maximum_square_error:  Thsi value is the normalized
%      maximum quantization error for any single pixel in the image.  This
%      distance measure is normalized to a range between 0 and 1.  It is
%      independent of the range of red, green, and blue values in your image.
%
%  Deprecated, replace with:
%
%    GetImageQuantizeError(image);
%
%  The format of the QuantizationError method is:
%
%      unsigned int QuantizationError(Image *image)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
*/
MagickExport unsigned int QuantizationError(Image *image)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.3");
  return(GetImageQuantizeError(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     R a n d o m C h a n n e l T h r e s h o l d I m a g e                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RandomChannelThresholdImage() changes the value of individual pixels based
%  on the intensity of each pixel compared to a random threshold.  The result
%  is a low-contrast, two color image.
%
%  The format of the RandomChannelThresholdImage method is:
%
%      unsigned int RandomChannelThresholdImage(Image *image,
%         const char *channel, const char *thresholds,
%         ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o thresholds: a geometry string containing LOWxHIGH thresholds.
%      If the string contains 2x2, 3x3, or 4x4, then an ordered
%      dither of order 2, 3, or 4 will be performed instead.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport unsigned int RandomChannelThresholdImage(Image *image,const char
    *channel,const char *thresholds,ExceptionInfo *exception)
{
#define RandomChannelThresholdImageText  "  RandomChannelThreshold image...  "

  double
    lower_threshold,
    upper_threshold;

  RandomInfo
    *random_info;

  ssize_t
    count,
    y;

  static MagickRealType
    o2[4]={0.2f, 0.6f, 0.8f, 0.4f},
    o3[9]={0.1f, 0.6f, 0.3f, 0.7f, 0.5f, 0.8f, 0.4f, 0.9f, 0.2f},
    o4[16]={0.1f, 0.7f, 1.1f, 0.3f, 1.0f, 0.5f, 1.5f, 0.8f, 1.4f, 1.6f, 0.6f,
      1.2f, 0.4f, 0.9f, 1.3f, 0.2f},
    threshold=128;

  size_t
    order;

  /*
    Threshold image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (LocaleCompare(thresholds,"2x2") == 0)
    order=2;
  else
    if (LocaleCompare(thresholds,"3x3") == 0)
      order=3;
    else
      if (LocaleCompare(thresholds,"4x4") == 0)
        order=4;
      else
        {
          order=1;
          lower_threshold=0;
          upper_threshold=0;
          count=(ssize_t) sscanf(thresholds,"%lf[/x%%]%lf",&lower_threshold,
            &upper_threshold);
          if (strchr(thresholds,'%') != (char *) NULL)
            {
              upper_threshold*=(.01*QuantumRange);
              lower_threshold*=(.01*QuantumRange);
            }
          if (count == 1)
            upper_threshold=(MagickRealType) QuantumRange-lower_threshold;
        }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TransformEvent,GetMagickModule(),
      "  RandomChannelThresholdImage: channel type=%s",channel);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TransformEvent,GetMagickModule(),
      "    Thresholds: %s (%fx%f)",thresholds,lower_threshold,upper_threshold);
  if (LocaleCompare(channel,"all") == 0 ||
      LocaleCompare(channel,"intensity") == 0)
    if (AcquireImageColormap(image,2) == MagickFalse)
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
  random_info=AcquireRandomInfo();
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register IndexPacket
      index,
      *restrict indexes;

    register PixelPacket
      *restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    if (LocaleCompare(channel,"all") == 0 ||
        LocaleCompare(channel,"intensity") == 0)
      {
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          MagickRealType
            intensity;

          intensity=(MagickRealType) PixelIntensityToQuantum(image,q);
          if (order == 1)
            {
              if (intensity < lower_threshold)
                threshold=lower_threshold;
              else if (intensity > upper_threshold)
                threshold=upper_threshold;
              else
                threshold=(MagickRealType) (QuantumRange*
                  GetPseudoRandomValue(random_info));
            }
          else if (order == 2)
            threshold=(MagickRealType) QuantumRange*o2[(x%2)+2*(y%2)];
          else if (order == 3)
            threshold=(MagickRealType) QuantumRange*o3[(x%3)+3*(y%3)];
          else if (order == 4)
            threshold=(MagickRealType) QuantumRange*o4[(x%4)+4*(y%4)];
          index=(IndexPacket) (intensity <= threshold ? 0 : 1);
          SetPixelIndex(indexes+x,index);
          SetPixelRGBO(q,image->colormap+(ssize_t) index);
          q++;
        }
      }
    if (LocaleCompare(channel,"opacity") == 0 ||
        LocaleCompare(channel,"all") == 0 ||
        LocaleCompare(channel,"matte") == 0)
      {
        if (image->matte != MagickFalse)
          for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (order == 1)
                {
                  if ((MagickRealType) q->opacity < lower_threshold)
                    threshold=lower_threshold;
                  else if ((MagickRealType) q->opacity > upper_threshold)
                    threshold=upper_threshold;
                  else
                    threshold=(MagickRealType) (QuantumRange*
                      GetPseudoRandomValue(random_info));
                }
              else if (order == 2)
                threshold=(MagickRealType) QuantumRange*o2[(x%2)+2*(y%2)];
              else if (order == 3)
                threshold=(MagickRealType) QuantumRange*o3[(x%3)+3*(y%3)];
              else if (order == 4)
                threshold=(MagickRealType) QuantumRange*o4[(x%4)+4*(y%4)]/1.7;
              SetPixelOpacity(q,(MagickRealType) q->opacity <=
                threshold ? 0 : QuantumRange);
              q++;
            }
      }
    else
      {
        /* To Do: red, green, blue, cyan, magenta, yellow, black */
        if (LocaleCompare(channel,"intensity") != 0)
          ThrowBinaryException(OptionError,"UnrecognizedChannelType",
            image->filename);
      }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  random_info=DestroyRandomInfo(random_info);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a c q u i r e M e m o r y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReacquireMemory() changes the size of the memory and returns a pointer to
%  the (possibly moved) block.  The contents will be unchanged up to the
%  lesser of the new and old sizes.
%
%  The format of the ReacquireMemory method is:
%
%      void ReacquireMemory(void **memory,const size_t size)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a memory allocation.  On return the pointer
%      may change but the contents of the original allocation will not.
%
%    o size: the new size of the allocated memory.
%
*/
MagickExport void ReacquireMemory(void **memory,const size_t size)
{
  void
    *allocation;

  assert(memory != (void **) NULL);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  if (*memory == (void *) NULL)
    {
      *memory=AcquireMagickMemory(size);
      return;
    }
  allocation=realloc(*memory,size);
  if (allocation == (void *) NULL)
    *memory=RelinquishMagickMemory(*memory);
  *memory=allocation;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R e c o l o r I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RecolorImage() apply color transformation to an image. The method permits
%  saturation changes, hue rotation, luminance to alpha, and various other
%  effects.  Although variable-sized transformation matrices can be used,
%  typically one uses a 5x5 matrix for an RGBA image and a 6x6 for CMYKA
%  (or RGBA with offsets).  The matrix is similar to those used by Adobe Flash
%  except offsets are in column 6 rather than 5 (in support of CMYKA images)
%  and offsets are normalized (divide Flash offset by 255).
%
%  The format of the RecolorImage method is:
%
%      Image *RecolorImage(const Image *image,const size_t order,
%        const double *color_matrix,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o order: the number of columns and rows in the recolor matrix.
%
%    o color_matrix: An array of double representing the recolor matrix.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *RecolorImage(const Image *image,const size_t order,
  const double *color_matrix,ExceptionInfo *exception)
{
  KernelInfo
    *kernel_info;

  Image
    *recolor_image;

  kernel_info=AcquireKernelInfo("1");
  if (kernel_info == (KernelInfo *) NULL)
    return((Image *) NULL);
  kernel_info->width=order;
  kernel_info->height=order;
  kernel_info->values=(double *) color_matrix;
  recolor_image=ColorMatrixImage(image,kernel_info,exception);
  kernel_info->values=(double *) NULL;
  kernel_info=DestroyKernelInfo(kernel_info);
  return(recolor_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R e d u c e N o i s e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReduceNoiseImage() smooths the contours of an image while still preserving
%  edge information.  The algorithm works by replacing each pixel with its
%  neighbor closest in value.  A neighbor is defined by radius.  Use a radius
%  of 0 and ReduceNoise() selects a suitable radius for you.
%
%  The format of the ReduceNoiseImage method is:
%
%      Image *ReduceNoiseImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the pixel neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ReduceNoiseImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
  Image
    *reduce_image;

  reduce_image=StatisticImage(image,NonpeakStatistic,(size_t) radius,(size_t)
    radius,exception);
  return(reduce_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e A t t r i b u t e I t e r a t o r                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImageAttributeIterator() resets the image attributes iterator.  Use it
%  in conjunction with GetNextImageAttribute() to iterate over all the values
%  associated with an image.
%
%  Deprecated, replace with:
%
%    ResetImagePropertyIterator(image);
%
%  The format of the ResetImageAttributeIterator method is:
%
%      ResetImageAttributeIterator(const ImageInfo *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void ResetImageAttributeIterator(const Image *image)
{
  ResetImagePropertyIterator(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e V i e w P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCacheViewPixels() gets pixels from the in-memory or disk pixel cache as
%  defined by the geometry parameters.   A pointer to the pixels is returned
%  if the pixels are transferred, otherwise a NULL is returned.
%
%  Deprecated, replace with:
%
%    QueueCacheViewAuthenticPixels(cache_view,x,y,columns,rows,
%      GetCacheViewException(cache_view));
%
%  The format of the SetCacheViewPixels method is:
%
%      PixelPacket *SetCacheViewPixels(CacheView *cache_view,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *SetCacheViewPixels(CacheView *cache_view,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows)
{
  PixelPacket
    *pixels;

  pixels=QueueCacheViewAuthenticPixels(cache_view,x,y,columns,rows,
    GetCacheViewException(cache_view));
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t C a c h e T h e s h o l d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCacheThreshold() sets the amount of free memory allocated for the pixel
%  cache.  Once this threshold is exceeded, all subsequent pixels cache
%  operations are to/from disk.
%
%  The format of the SetCacheThreshold() method is:
%
%      void SetCacheThreshold(const size_t threshold)
%
%  A description of each parameter follows:
%
%    o threshold: the number of megabytes of memory available to the pixel
%      cache.
%
*/
MagickExport void SetCacheThreshold(const size_t size)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.1");
  (void) SetMagickResourceLimit(MemoryResource,size*1024*1024);
  (void) SetMagickResourceLimit(MapResource,2*size*1024*1024);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t E x c e p t i o n I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetExceptionInfo() sets the exception severity.
%
%  The format of the SetExceptionInfo method is:
%
%      MagickBooleanType SetExceptionInfo(ExceptionInfo *exception,
%        ExceptionType severity)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
%    o severity: the exception severity.
%
*/
MagickExport MagickBooleanType SetExceptionInfo(ExceptionInfo *exception,
  ExceptionType severity)
{
  assert(exception != (ExceptionInfo *) NULL);
  ClearMagickException(exception);
  exception->severity=severity;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImage() sets the red, green, and blue components of each pixel to
%  the image background color and the opacity component to the specified
%  level of transparency.  The background color is defined by the
%  background_color member of the image.
%
%  The format of the SetImage method is:
%
%      void SetImage(Image *image,const Quantum opacity)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o opacity: Set each pixel to this level of transparency.
%
*/
MagickExport void SetImage(Image *image,const Quantum opacity)
{
  PixelPacket
    background_color;

  ssize_t
    y;

  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.2.0");
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  background_color=image->background_color;
  if (opacity != OpaqueOpacity)
    background_color.opacity=opacity;
  if (background_color.opacity != OpaqueOpacity)
    {
      (void) SetImageStorageClass(image,DirectClass);
      image->matte=MagickTrue;
    }
  if ((image->storage_class == PseudoClass) ||
      (image->colorspace == CMYKColorspace))
    {
      /*
        Set colormapped or CMYK image.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        register IndexPacket
          *restrict indexes;

        register ssize_t
          x;

        register PixelPacket
          *restrict q;

        q=QueueAuthenticPixels(image,0,y,image->columns,1,&image->exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRGBO(q,&background_color);
          q++;
        }
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < (ssize_t) image->columns; x++)
          SetPixelIndex(indexes+x,0);
        if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
          break;
      }
      return;
    }
  /*
    Set DirectClass image.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRGBO(q,&background_color);
      q++;
    }
    if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
      break;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e A t t r i b u t e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageAttribute() searches the list of image attributes and replaces the
%  attribute value.  If it is not found in the list, the attribute name
%  and value is added to the list.
%
%  Deprecated, replace with:
%
%    SetImageProperty(image,key,value);
%
%  The format of the SetImageAttribute method is:
%
%       MagickBooleanType SetImageAttribute(Image *image,const char *key,
%         const char *value)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o key: the key.
%
%    o value: the value.
%
*/
MagickExport MagickBooleanType SetImageAttribute(Image *image,const char *key,
  const char *value)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.3.1");
  return(SetImageProperty(image,key,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageList() inserts an image into the list at the specified position.
%
%  The format of the SetImageList method is:
%
%      unsigned int SetImageList(Image *images,const Image *image,
%        const ssize_t offset,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o image: the image.
%
%    o offset: the position within the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport unsigned int SetImageList(Image **images,const Image *image,
  const ssize_t offset,ExceptionInfo *exception)
{
  Image
    *clone;

  register ssize_t
    i;

  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  clone=CloneImageList(image,exception);
  while (GetPreviousImageInList(*images) != (Image *) NULL)
    (*images)=GetPreviousImageInList(*images);
  for (i=0; i < offset; i++)
  {
    if (GetNextImageInList(*images) == (Image *) NULL)
      return(MagickFalse);
    (*images)=GetNextImageInList(*images);
  }
  InsertImageInList(images,clone);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e P i x e l s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImagePixels() queues a mutable pixel region.
%  If the region is successfully initialized a pointer to a PixelPacket
%  array representing the region is returned, otherwise NULL is returned.
%  The returned pointer may point to a temporary working buffer for the
%  pixels or it may point to the final location of the pixels in memory.
%
%  Write-only access means that any existing pixel values corresponding to
%  the region are ignored.  This useful while the initial image is being
%  created from scratch, or if the existing pixel values are to be
%  completely replaced without need to refer to their pre-existing values.
%  The application is free to read and write the pixel buffer returned by
%  SetImagePixels() any way it pleases. SetImagePixels() does not initialize
%  the pixel array values. Initializing pixel array values is the
%  application's responsibility.
%
%  Performance is maximized if the selected region is part of one row, or
%  one or more full rows, since then there is opportunity to access the
%  pixels in-place (without a copy) if the image is in RAM, or in a
%  memory-mapped file. The returned pointer should *never* be deallocated
%  by the user.
%
%  Pixels accessed via the returned pointer represent a simple array of type
%  PixelPacket. If the image type is CMYK or the storage class is PseudoClass,
%  call GetAuthenticIndexQueue() after invoking GetAuthenticPixels() to obtain
%  the black color component or the colormap indexes (of type IndexPacket)
%  corresponding to the region.  Once the PixelPacket (and/or IndexPacket)
%  array has been updated, the changes must be saved back to the underlying
%  image using SyncAuthenticPixels() or they may be lost.
%
%  Deprecated, replace with:
%
%    QueueAuthenticPixels(image,x,y,columns,rows,&image->exception);
%
%  The format of the SetImagePixels() method is:
%
%      PixelPacket *SetImagePixels(Image *image,const ssize_t x,const ssize_t y,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o pixels: SetImagePixels returns a pointer to the pixels if they are
%      transferred, otherwise a NULL is returned.
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *SetImagePixels(Image *image,const ssize_t x,const ssize_t y,
  const size_t columns,const size_t rows)
{
  return(QueueAuthenticPixels(image,x,y,columns,rows,&image->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a g i c k R e g i s t r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMagickRegistry() sets a blob into the registry and returns a unique ID.
%  If an error occurs, -1 is returned.
%
%  The format of the SetMagickRegistry method is:
%
%      ssize_t SetMagickRegistry(const RegistryType type,const void *blob,
%        const size_t length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o type: the registry type.
%
%    o blob: the address of a Binary Large OBject.
%
%    o length: For a registry type of ImageRegistryType use sizeof(Image)
%      otherise the blob length in number of bytes.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ssize_t SetMagickRegistry(const RegistryType type,const void *blob,
  const size_t magick_unused(length),ExceptionInfo *exception)
{
  char
    key[MaxTextExtent];

  MagickBooleanType
    status;

  static ssize_t
    id = 0;

  (void) FormatLocaleString(key,MaxTextExtent,"%.20g\n",(double) id);
  status=SetImageRegistry(type,key,blob,exception);
  if (status == MagickFalse)
    return(-1);
  return(id++);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M o n i t o r H a n d l e r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMonitorHandler() sets the monitor handler to the specified method
%  and returns the previous monitor handler.
%
%  The format of the SetMonitorHandler method is:
%
%      MonitorHandler SetMonitorHandler(MonitorHandler handler)
%
%  A description of each parameter follows:
%
%    o handler: Specifies a pointer to a method to handle monitors.
%
*/

MagickExport MonitorHandler GetMonitorHandler(void)
{
  return(monitor_handler);
}

MagickExport MonitorHandler SetMonitorHandler(MonitorHandler handler)
{
  MonitorHandler
    previous_handler;

  previous_handler=monitor_handler;
  monitor_handler=handler;
  return(previous_handler);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S h i f t I m a g e L i s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShiftImageList() removes an image from the beginning of the list.
%
%  Deprecated, replace with:
%
%    RemoveFirstImageFromList(images);
%
%  The format of the ShiftImageList method is:
%
%      Image *ShiftImageList(Image **images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport Image *ShiftImageList(Image **images)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  return(RemoveFirstImageFromList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S i z e B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SizeBlob() returns the current length of the image file or blob.
%
%  Deprecated, replace with:
%
%    GetBlobSize(image);
%
%  The format of the SizeBlob method is:
%
%      off_t SizeBlob(Image *image)
%
%  A description of each parameter follows:
%
%    o size:  Method SizeBlob returns the current length of the image file
%      or blob.
%
%    o image: the image.
%
*/
MagickExport MagickOffsetType SizeBlob(Image *image)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.4.3");
  return((MagickOffsetType) GetBlobSize(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p l i c e I m a g e L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SpliceImageList() removes the images designated by offset and length from
%  the list and replaces them with the specified list.
%
%  The format of the SpliceImageList method is:
%
%      Image *SpliceImageList(Image *images,const ssize_t offset,
%        const size_t length,const Image *splices,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o offset: the position within the list.
%
%    o length: the length of the image list to remove.
%
%    o splice: Replace the removed image list with this list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SpliceImageList(Image *images,const ssize_t offset,
  const size_t length,const Image *splices,ExceptionInfo *exception)
{
  Image
    *clone;

  register ssize_t
    i;

  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  clone=CloneImageList(splices,exception);
  while (GetPreviousImageInList(images) != (Image *) NULL)
    images=GetPreviousImageInList(images);
  for (i=0; i < offset; i++)
  {
    if (GetNextImageInList(images) == (Image *) NULL)
      return((Image *) NULL);
    images=GetNextImageInList(images);
  }
  (void) SpliceImageIntoList(&images,length,clone);
  return(images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   s R G B C o m p a n d o r                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  sRGBCompandor() adds the gamma function to a sRGB pixel.
%
%  The format of the sRGBCompandor method is:
%
%      MagickRealType sRGBCompandor(const MagickRealType pixel)
%
%  A description of each parameter follows:
%
%    o pixel: the pixel.
%
*/
MagickExport MagickRealType sRGBCompandor(const MagickRealType pixel)
{
  if (pixel <= (0.0031306684425005883*QuantumRange))
    return(12.92*pixel);
  return(QuantumRange*(1.055*pow(QuantumScale*pixel,1.0/2.4)-0.055));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i p                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Strip() strips any whitespace or quotes from the beginning and end of a
%  string of characters.
%
%  The format of the Strip method is:
%
%      void Strip(char *message)
%
%  A description of each parameter follows:
%
%    o message: Specifies an array of characters.
%
*/
MagickExport void Strip(char *message)
{
  register char
    *p,
    *q;

  assert(message != (char *) NULL);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  if (*message == '\0')
    return;
  if (strlen(message) == 1)
    return;
  p=message;
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  if ((*p == '\'') || (*p == '"'))
    p++;
  q=message+strlen(message)-1;
  while ((isspace((int) ((unsigned char) *q)) != 0) && (q > p))
    q--;
  if (q > p)
    if ((*q == '\'') || (*q == '"'))
      q--;
  (void) CopyMagickMemory(message,p,(size_t) (q-p+1));
  message[q-p+1]='\0';
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c C a c h e V i e w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncCacheView() saves the cache view pixels to the in-memory or disk
%  cache.  It returns MagickTrue if the pixel region is synced, otherwise
%  MagickFalse.
%
%  Deprecated, replace with:
%
%    SyncCacheViewAuthenticPixels(cache_view,GetCacheViewException(cache_view));
%
%  The format of the SyncCacheView method is:
%
%      MagickBooleanType SyncCacheView(CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport MagickBooleanType SyncCacheView(CacheView *cache_view)
{
  MagickBooleanType
    status;

  status=SyncCacheViewAuthenticPixels(cache_view,
    GetCacheViewException(cache_view));
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c C a c h e V i e w P i x e l s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncCacheViewPixels() saves the cache view pixels to the in-memory
%  or disk cache.  It returns MagickTrue if the pixel region is flushed,
%  otherwise MagickFalse.
%
%  Deprecated, replace with:
%
%    SyncCacheViewAuthenticPixels(cache_view,GetCacheViewException(cache_view));
%
%  The format of the SyncCacheViewPixels method is:
%
%      MagickBooleanType SyncCacheViewPixels(CacheView *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SyncCacheViewPixels(CacheView *cache_view)
{
  MagickBooleanType
    status;

  status=SyncCacheViewAuthenticPixels(cache_view,
    GetCacheViewException(cache_view));
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c I m a g e P i x e l s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncImagePixels() saves the image pixels to the in-memory or disk cache.
%  The method returns MagickTrue if the pixel region is synced, otherwise
%  MagickFalse.
%
%  Deprecated, replace with:
%
%    SyncAuthenticPixels(image,&image->exception);
%
%  The format of the SyncImagePixels() method is:
%
%      MagickBooleanType SyncImagePixels(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType SyncImagePixels(Image *image)
{
  return(SyncAuthenticPixels(image,&image->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  T e m p o r a r y F i l e n a m e                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TemporaryFilename() replaces the contents of path by a unique path name.
%
%  The format of the TemporaryFilename method is:
%
%      void TemporaryFilename(char *path)
%
%  A description of each parameter follows.
%
%   o  path:  Specifies a pointer to an array of characters.  The unique path
%      name is returned in this array.
%
*/
MagickExport void TemporaryFilename(char *path)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.6");
  (void) AcquireUniqueFilename(path);
  (void) RelinquishUniqueFileResource(path);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T h r e s h o l d I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ThresholdImage() changes the value of individual pixels based on
%  the intensity of each pixel compared to threshold.  The result is a
%  high-contrast, two color image.
%
%  The format of the ThresholdImage method is:
%
%      unsigned int ThresholdImage(Image *image,const double threshold)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: Define the threshold value
%
*/
MagickExport unsigned int ThresholdImage(Image *image,const double threshold)
{
#define ThresholdImageTag  "Threshold/Image"

  IndexPacket
    index;

  ssize_t
    y;

  /*
    Threshold image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.7");
  if (!AcquireImageColormap(image,2))
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      "UnableToThresholdImage");
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      index=(IndexPacket) ((MagickRealType) PixelIntensityToQuantum(image,q) <=
        threshold ? 0 : 1);
      SetPixelIndex(indexes+x,index);
      SetPixelRGBO(q,image->colormap+(ssize_t) index);
      q++;
    }
    if (!SyncAuthenticPixels(image,&image->exception))
      break;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T h r e s h o l d I m a g e C h a n n e l                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ThresholdImageChannel() changes the value of individual pixels based on
%  the intensity of each pixel channel.  The result is a high-contrast image.
%
%  The format of the ThresholdImageChannel method is:
%
%      unsigned int ThresholdImageChannel(Image *image,const char *threshold)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: define the threshold values.
%
*/
MagickExport unsigned int ThresholdImageChannel(Image *image,
  const char *threshold)
{
#define ThresholdImageTag  "Threshold/Image"

  MagickPixelPacket
    pixel;

  GeometryInfo
    geometry_info;

  IndexPacket
    index;

  ssize_t
    y;

  unsigned int
    flags;

  /*
    Threshold image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (threshold == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  flags=ParseGeometry(threshold,&geometry_info);
  pixel.red=geometry_info.rho;
  if (flags & SigmaValue)
    pixel.green=geometry_info.sigma;
  else
    pixel.green=pixel.red;
  if (flags & XiValue)
    pixel.blue=geometry_info.xi;
  else
    pixel.blue=pixel.red;
  if (flags & PsiValue)
    pixel.opacity=geometry_info.psi;
  else
    pixel.opacity=(MagickRealType) OpaqueOpacity;
  if (flags & PercentValue)
    {
      pixel.red*=QuantumRange/100.0f;
      pixel.green*=QuantumRange/100.0f;
      pixel.blue*=QuantumRange/100.0f;
      pixel.opacity*=QuantumRange/100.0f;
    }
  if (!(flags & SigmaValue))
    {
      if (!AcquireImageColormap(image,2))
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          "UnableToThresholdImage");
      if (pixel.red == 0)
        (void) GetImageDynamicThreshold(image,2.0,2.0,&pixel,&image->exception);
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    if (IsMagickGray(&pixel) != MagickFalse)
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        index=(IndexPacket) ((MagickRealType)
          PixelIntensityToQuantum(image,q) <= pixel.red ? 0 : 1);
        SetPixelIndex(indexes+x,index);
        SetPixelRed(q,image->colormap[(ssize_t) index].red);
        SetPixelGreen(q,image->colormap[(ssize_t) index].green);
        SetPixelBlue(q,image->colormap[(ssize_t) index].blue);
        q++;
      }
    else
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelRed(q,(MagickRealType) q->red <= pixel.red
          ? 0 : QuantumRange);
        SetPixelGreen(q,(MagickRealType) q->green <= pixel.green
          ? 0 : QuantumRange);
        SetPixelBlue(q,(MagickRealType) q->blue <= pixel.blue
          ?  0 : QuantumRange);
        SetPixelOpacity(q,(MagickRealType) q->opacity <= pixel.opacity
          ? 0 : QuantumRange);
        q++;
      }
    if (!SyncAuthenticPixels(image,&image->exception))
      break;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                              %
%                                                                              %
%                                                                              %
+     T r a n s f o r m C o l o r s p a c e                                    %
%                                                                              %
%                                                                              %
%                                                                              %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformColorspace() converts the image to a specified colorspace.
%  If the image is already in the requested colorspace, no work is performed.
%  Note that the current colorspace is stored in the image colorspace member.
%  The transformation matrices are not necessarily the standard ones: the
%  weights are rescaled to normalize the range of the transformed values to
%  be [0..QuantumRange].
%
%  Deprecated, replace with:
%
%    TransformImageColorspace(image,colorspace);
%
%  The format of the TransformColorspace method is:
%
%      unsigned int (void) TransformColorspace(Image *image,
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o image: the image to transform
%
%    o colorspace: the desired colorspace.
%
*/
MagickExport unsigned int TransformColorspace(Image *image,
  const ColorspaceType colorspace)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.6");
  return(TransformImageColorspace(image,colorspace));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s f o r m H S L                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformHSL() converts a (red, green, blue) to a (hue, saturation,
%  lightness) triple.
%
%  The format of the TransformHSL method is:
%
%      void TransformHSL(const Quantum red,const Quantum green,
%        const Quantum blue,double *hue,double *saturation,double *lightness)
%
%  A description of each parameter follows:
%
%    o red, green, blue: A Quantum value representing the red, green, and
%      blue component of a pixel..
%
%    o hue, saturation, lightness: A pointer to a double value representing a
%      component of the HSL color space.
%
*/

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport void TransformHSL(const Quantum red,const Quantum green,
  const Quantum blue,double *hue,double *saturation,double *lightness)
{
  MagickRealType
    b,
    delta,
    g,
    max,
    min,
    r;

  /*
    Convert RGB to HSL colorspace.
  */
  assert(hue != (double *) NULL);
  assert(saturation != (double *) NULL);
  assert(lightness != (double *) NULL);
  r=QuantumScale*red;
  g=QuantumScale*green;
  b=QuantumScale*blue;
  max=MagickMax(r,MagickMax(g,b));
  min=MagickMin(r,MagickMin(g,b));
  *hue=0.0;
  *saturation=0.0;
  *lightness=(double) ((min+max)/2.0);
  delta=max-min;
  if (delta == 0.0)
    return;
  *saturation=(double) (delta/((*lightness < 0.5) ? (min+max) :
    (2.0-max-min)));
  if (r == max)
    *hue=(double) (g == min ? 5.0+(max-b)/delta : 1.0-(max-g)/delta);
  else
    if (g == max)
      *hue=(double) (b == min ? 1.0+(max-r)/delta : 3.0-(max-b)/delta);
    else
      *hue=(double) (r == min ? 3.0+(max-g)/delta : 5.0-(max-r)/delta);
  *hue/=6.0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s l a t e T e x t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TranslateText() replaces any embedded formatting characters with the
%  appropriate image attribute and returns the translated text.
%
%  Deprecated, replace with:
%
%    InterpretImageProperties(image_info,image,embed_text);
%
%  The format of the TranslateText method is:
%
%      char *TranslateText(const ImageInfo *image_info,Image *image,
%        const char *embed_text)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o embed_text: the address of a character string containing the embedded
%      formatting characters.
%
*/
MagickExport char *TranslateText(const ImageInfo *image_info,Image *image,
  const char *embed_text)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.2.6");
  return(InterpretImageProperties(image_info,image,embed_text));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T r a n s p a r e n t I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransparentImage() changes the opacity value associated with any pixel
%  that matches color to the value defined by opacity.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  The format of the TransparentImage method is:
%
%      MagickBooleanType TransparentImage(Image *image,
%        const PixelPacket target,const Quantum opacity)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target: the RGB value of the target color.
%
%    o opacity: the replacement opacity value.
%
*/
MagickExport MagickBooleanType TransparentImage(Image *image,
  const PixelPacket target,const Quantum opacity)
{
#define TransparentImageTag  "Transparent/Image"

  MagickBooleanType
    proceed;

  ssize_t
    y;

  /*
    Make image color transparent.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v6.1.0");
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (IsColorSimilar(image,q,&target) != MagickFalse)
        q->opacity=opacity;
      q++;
    }
    if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,TransparentImageTag,(MagickOffsetType) y,
      image->rows);
    if (proceed == MagickFalse)
      break;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n s h i f t I m a g e L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnshiftImageList() adds the image to the beginning of the list.
%
%  Deprecated, replace with:
%
%    PrependImageToList(images,CloneImageList(image,exception));
%
%  The format of the UnshiftImageList method is:
%
%      unsigned int UnshiftImageList(Image *images,const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport unsigned int UnshiftImageList(Image **images,const Image *image,
  ExceptionInfo *exception)
{
  (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  PrependImageToList(images,CloneImageList(image,exception));
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   V a l i d a t e C o l o r m a p I n d e x                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateColormapIndex() validates the colormap index.  If the index does
%  not range from 0 to the number of colors in the colormap an exception
%  issued and 0 is returned.
%
%  Deprecated, replace with:
%
%    ConstrainColormapIndex(image,index);
%
%  The format of the ValidateColormapIndex method is:
%
%      IndexPacket ValidateColormapIndex(Image *image,const unsigned int index)
%
%  A description of each parameter follows:
%
%    o index: Method ValidateColormapIndex returns colormap index if it is
%      valid other an exception issued and 0 is returned.
%
%    o image: the image.
%
%    o index: This integer is the colormap index.
%
*/
MagickExport IndexPacket ValidateColormapIndex(Image *image,
  const size_t index)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.4.4");
  return(ConstrainColormapIndex(image,index));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Z o o m I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ZoomImage() creates a new image that is a scaled size of an existing one.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.  The Point filter gives fast pixel replication,
%  Triangle is equivalent to bi-linear interpolation, and Mitchel giver slower,
%  very high-quality results.  See Graphic Gems III for details on this
%  algorithm.
%
%  The filter member of the Image structure specifies which image filter to
%  use. Blur specifies the blur factor where > 1 is blurry, < 1 is sharp.
%
%  The format of the ZoomImage method is:
%
%      Image *ZoomImage(const Image *image,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: An integer that specifies the number of columns in the zoom
%      image.
%
%    o rows: An integer that specifies the number of rows in the scaled
%      image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ZoomImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
{
  Image
    *zoom_image;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  zoom_image=ResizeImage(image,columns,rows,image->filter,image->blur,
    exception);
  return(zoom_image);
}
#endif
