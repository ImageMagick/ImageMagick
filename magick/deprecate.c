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
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/draw.h"
#include "magick/draw-private.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
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
#include "magick/paint.h"
#include "magick/pixel.h"
#include "magick/pixel-private.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/segment.h"
#include "magick/splay-tree.h"
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
%  The format of the AcquireCacheViewPixels method is:
%
%      const PixelPacket *AcquireCacheViewPixels(const CacheView *cache_view,
%        const long x,const long y,const unsigned long columns,
%        const unsigned long rows,ExceptionInfo *exception)
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
  const CacheView *cache_view,const long x,const long y,
  const unsigned long columns,const unsigned long rows,ExceptionInfo *exception)
{
  return(GetCacheViewVirtualPixels(cache_view,x,y,columns,rows,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I m a g e P i x e l s                                        %
%                                                                             %
%                                                                             %
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
%  call GetAuthenticIndexQueue() after invoking GetAuthenticPixels() to access the
%  black color component or to obtain the colormap indexes (of type IndexPacket)
%  corresponding to the region.
%
%  If you plan to modify the pixels, use GetAuthenticPixels() instead.
%
%  Note, the AcquireImagePixels() and GetAuthenticPixels() methods are not
%  thread-safe.  In a threaded environment, use GetCacheViewVirtualPixels() or
%  GetCacheViewAuthenticPixels() instead.
%
%  The format of the AcquireImagePixels() method is:
%
%      const PixelPacket *AcquireImagePixels(const Image *image,const long x,
%        const long y,const unsigned long columns,const unsigned long rows,
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
  const long x,const long y,const unsigned long columns,
  const unsigned long rows,ExceptionInfo *exception)
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
%  associated with the last call to QueueAuthenticPixels() or GetVirtualPixels().
%  NULL is returned if the black channel or colormap indexes are not available.
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
%  The format of the AcquireOneCacheViewPixel method is:
%
%      MagickBooleanType AcquireOneCacheViewPixel(const CacheView *cache_view,
%        const long x,const long y,PixelPacket *pixel,ExceptionInfo *exception)
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
  const CacheView *cache_view,const long x,const long y,PixelPacket *pixel,
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
%  The format of the AcquireOneCacheViewPixel method is:
%
%      MagickBooleanType AcquireOneCacheViewVirtualPixel(
%        const CacheView *cache_view,
%        const VirtualPixelMethod virtual_pixel_method,const long x,
%        const long y,PixelPacket *pixel,ExceptionInfo *exception)
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
  const long x,const long y,PixelPacket *pixel,ExceptionInfo *exception)
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
%  The format of the AcquireOneMagickPixel() method is:
%
%      MagickPixelPacket AcquireOneMagickPixel(const Image image,const long x,
%        const long y,ExceptionInfo exception)
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
  const long x,const long y,ExceptionInfo *exception)
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
%  The format of the AcquireOnePixel() method is:
%
%      PixelPacket AcquireOnePixel(const Image image,const long x,
%        const long y,ExceptionInfo exception)
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
MagickExport PixelPacket AcquireOnePixel(const Image *image,const long x,
  const long y,ExceptionInfo *exception)
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
%  The format of the AcquireOneVirtualPixel() method is:
%
%      PixelPacket AcquireOneVirtualPixel(const Image image,
%        const VirtualPixelMethod virtual_pixel_method,const long x,
%        const long y,ExceptionInfo exception)
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
  const VirtualPixelMethod virtual_pixel_method,const long x,const long y,
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
%  The format of the AllocateImageColormap method is:
%
%      MagickBooleanType AllocateImageColormap(Image *image,
%        const unsigned long colors)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colors: the number of colors in the image colormap.
%
*/
MagickExport MagickBooleanType AllocateImageColormap(Image *image,
  const unsigned long colors)
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
%   A c q u i r e S t r i n g                                                 %
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
%     C h a n n e l I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Extract a channel from the image.  A channel is a particular color component
%  of each pixel in the image.
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

  register long
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
  for (i=(long) (size-1); i >= 0; i--)
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
%        const long x_offset,const long y_offset,const PaintMethod method)
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
      if ((((up)+(delta)) >= 0) && (((up)+(delta)) < (long) image->rows)) \
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
  const DrawInfo *draw_info,const PixelPacket target,const long x_offset,
  const long y_offset,const PaintMethod method)
{
  Image
    *floodplane_image;

  long
    offset,
    start,
    x,
    x1,
    x2,
    y;

  MagickBooleanType
    skip;

  PixelPacket
    fill_color;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(draw_info != (DrawInfo *) NULL);
  assert(draw_info->signature == MagickSignature);
  if ((x_offset < 0) || (x_offset >= (long) image->columns))
    return(MagickFalse);
  if ((y_offset < 0) || (y_offset >= (long) image->rows))
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
      *__restrict p;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Pop segment off stack.
    */
    s--;
    x1=(long) s->x1;
    x2=(long) s->x2;
    offset=(long) s->y2;
    y=(long) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    p=GetVirtualPixels(image,0,y,(unsigned long) (x1+1),1,&image->exception);
    q=GetAuthenticPixels(floodplane_image,0,y,(unsigned long) (x1+1),1,
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
          if (x < (long) image->columns)
            {
              p=GetVirtualPixels(image,x,y,image->columns-x,1,
                &image->exception);
              q=GetAuthenticPixels(floodplane_image,x,y,image->columns-x,1,
                &image->exception);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for ( ; x < (long) image->columns; x++)
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
          p=GetVirtualPixels(image,x,y,(unsigned long) (x2-x+1),1,
            &image->exception);
          q=GetAuthenticPixels(floodplane_image,x,y,(unsigned long) (x2-x+1),1,
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
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *__restrict p;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Tile fill color onto floodplane.
    */
    p=GetVirtualPixels(floodplane_image,0,y,image->columns,1,
      &image->exception);
    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (long) image->columns; x++)
    {
      if (p->opacity != OpaqueOpacity)
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
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
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
%      unsigned int DeleteImageList(Image *images,const long offset)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
%    o offset: the position within the list.
%
*/
MagickExport unsigned int DeleteImageList(Image *images,const long offset)
{
  register long
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
%  The format of the DeleteMagickRegistry method is:
%
%      MagickBooleanType DeleteMagickRegistry(const long id)
%
%  A description of each parameter follows:
%
%    o id: the registry id.
%
*/
MagickExport MagickBooleanType DeleteMagickRegistry(const long id)
{
  char
    key[MaxTextExtent];

  (void) FormatMagickString(key,MaxTextExtent,"%ld\n",id);
  return(DeleteImageRegistry(key));
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
%  The format of the DestroyMagickRegistry method is:
%
%       void DestroyMagickRegistry(void)
%
*/
MagickExport void DestroyMagickRegistry(void)
{
  DestroyImageRegistry();
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
%  encountered.  The data is returned as char, short int, int, long, float,
%  or double in the order specified by map.
%
%  Suppose you want to extract the first scanline of a 640x480 image as
%  character data in red-green-blue order:
%
%      DispatchImage(image,0,0,640,1,"RGB",CharPixel,pixels,exception);
%
%  The format of the DispatchImage method is:
%
%      unsigned int DispatchImage(const Image *image,const long x_offset,
%        const long y_offset,const unsigned long columns,
%        const unsigned long rows,const char *map,const StorageType type,
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
MagickExport unsigned int DispatchImage(const Image *image,const long x_offset,
  const long y_offset,const unsigned long columns,const unsigned long rows,
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
%      Image *ExtractSubimageFromImage(const Image *image,const Image *reference,
%        ExceptionInfo *exception)
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
  const long x_offset,const long y_offset,const double similarity_threshold,
  ExceptionInfo *exception)
{
  long
    y;

  double
    channels,
    normalized_similarity,
    similarity;

  CacheView
    *image_view,
    *reference_view;

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
  image_view=AcquireCacheView(image);
  reference_view=AcquireCacheView(reference);
  for (y=0; y < (long) reference->rows; y++)
  {
    register const IndexPacket
      *indexes,
      *reference_indexes;

    register const PixelPacket
      *p,
      *q;

    register long
      x;

    p=GetCacheViewVirtualPixels(image_view,x_offset,y_offset+y,
      reference->columns,1,exception);
    q=GetCacheViewVirtualPixels(reference_view,0,y,reference->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      continue;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reference_indexes=GetCacheViewVirtualIndexQueue(reference_view);
    for (x=0; x < (long) reference->columns; x++)
    {
      MagickRealType
        pixel;

      pixel=QuantumScale*(p->red-(double) q->red);
      similarity+=pixel*pixel;
      pixel=QuantumScale*(p->green-(double) q->green);
      similarity+=pixel*pixel;
      pixel=QuantumScale*(p->blue-(double) q->blue);
      similarity+=pixel*pixel;
      if ((image->matte != MagickFalse) && (reference->matte != MagickFalse))
        {
          pixel=QuantumScale*(p->opacity-(double) q->opacity);
          similarity+=pixel*pixel;
        }
      if ((image->colorspace == CMYKColorspace) &&
          (reference->colorspace == CMYKColorspace))
        {
          pixel=QuantumScale*(indexes[x]-(double) reference_indexes[x]);
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
  long
    y;

  double
    similarity_threshold;

  RectangleInfo
    offset;

  /*
    Extract reference from image.
  */
  if ((reference->columns > image->columns) || (reference->rows > image->rows))
    return((Image *) NULL);
  similarity_threshold=image->columns*image->rows;
  SetGeometry(reference,&offset);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1)
#endif
  for (y=0; y < (long) (image->rows-reference->rows); y++)
  {
    double
      similarity;

    register long
      x;

    for (x=0; x < (long) (image->columns-reference->columns); x++)
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
  return(SetImageAttribute(image,key,value));
}

MagickExport MagickBooleanType FormatImageAttribute(Image *image,
  const char *key,const char *format,...)
{
  MagickBooleanType
    status;

  va_list
    operands;

  va_start(operands,format);
  status=FormatImageAttributeList(image,key,format,operands);
  va_end(operands);
  return(status);
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
  FormatStringList(string,format,operands);
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

  if ((fuzz == 0.0) && (p->red == q->red) && (p->green == q->green) &&
      (p->blue == q->blue))
    return(MagickTrue);
  pixel.red=p->red-(MagickRealType) q->red;
  distance=pixel.red*pixel.red;
  if (distance > (fuzz*fuzz))
    return(MagickFalse);
  pixel.green=p->green-(MagickRealType) q->green;
  distance+=pixel.green*pixel.green;
  if (distance > (fuzz*fuzz))
    return(MagickFalse);
  pixel.blue=p->blue-(MagickRealType) q->blue;
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
      (void) FormatMagickString(path,MaxTextExtent,"%s%s",
        MAGICKCORE_LIBRARY_PATH,filename);
      if (IsPathAccessible(path) != MagickFalse)
        blob=FileToBlob(path,~0,length,exception);
    }
#endif
#if defined(__WINDOWS__) && !(defined(MAGICKCORE_CONFIGURE_PATH) || defined(MAGICKCORE_SHARE_CONFIGURE_PATH))
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
          (void) FormatMagickString(path,MaxTextExtent,"%s%s%s",key_value,
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
          (void) FormatMagickString(path,MaxTextExtent,"%s%s%s",home,
            DirectorySeparator,filename);
#else
          (void) FormatMagickString(path,MaxTextExtent,"%s/lib/%s/%s",home,
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
          (void) FormatMagickString(path,MaxTextExtent,"%s%s.magick%s%s",home,
            DirectorySeparator,DirectorySeparator,filename);
          if ((IsPathAccessible(path) != MagickFalse) && (blob == (void *) NULL))
            blob=FileToBlob(path,~0,length,exception);
          home=DestroyString(home);
        }
    }
  if ((blob == (void *) NULL) && (*GetClientPath() != '\0'))
    {
#if !defined(MAGICKCORE_POSIX_SUPPORT)
      (void) FormatMagickString(path,MaxTextExtent,"%s%s%s",GetClientPath(),
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
      (void) FormatMagickString(path,MaxTextExtent,"%s/lib/%s/%s",prefix,
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
#if defined(__WINDOWS__)
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
%  The format of the GetCacheView method is:
%
%      PixelPacket *GetCacheView(CacheView *cache_view,const long x,
%        const long y,const unsigned long columns,const unsigned long rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the address of a structure of type CacheView.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *GetCacheView(CacheView *cache_view,const long x,
  const long y,const unsigned long columns,const unsigned long rows)
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
%  The format of the GetCacheViewPixels method is:
%
%      PixelPacket *GetCacheViewPixels(CacheView *cache_view,const long x,
%        const long y,const unsigned long columns,const unsigned long rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *GetCacheViewPixels(CacheView *cache_view,const long x,
  const long y,const unsigned long columns,const unsigned long rows)
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
%  The format of the GetImageFromMagickRegistry method is:
%
%      Image *GetImageFromMagickRegistry(const char *name,long *id,
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
MagickExport Image *GetImageFromMagickRegistry(const char *name,long *id,
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
%      const void *GetMagickRegistry(const long id,RegistryType *type,
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
MagickExport void *GetMagickRegistry(const long id,RegistryType *type,
  size_t *length,ExceptionInfo *exception)
{
  char
    key[MaxTextExtent];

  void
    *blob;

  *type=UndefinedRegistryType;
  *length=0;
  (void) FormatMagickString(key,MaxTextExtent,"%ld\n",id);
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
%  The format of the GetImageList method is:
%
%      Image *GetImageList(const Image *images,const long offset,
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
MagickExport Image *GetImageList(const Image *images,const long offset,
  ExceptionInfo *exception)
{
  Image
    *image;

  if (images->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.5.2");
  image=CloneImage(GetImageFromList(images,(long) offset),0,0,MagickTrue,
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
%  The format of the GetImageListIndex method is:
%
%      long GetImageListIndex(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport long GetImageListIndex(const Image *images)
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
%  The format of the GetImageListSize method is:
%
%      unsigned long GetImageListSize(const Image *images)
%
%  A description of each parameter follows:
%
%    o images: the image list.
%
*/
MagickExport unsigned long GetImageListSize(const Image *images)
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
%  The format of the GetImagePixels() method is:
%
%      PixelPacket *GetImagePixels(Image *image,const long x,const long y,
%        const unsigned long columns,const unsigned long rows)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *GetImagePixels(Image *image,const long x,const long y,
  const unsigned long columns,const unsigned long rows)
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
%  The format of the GetMagickGeometry method is:
%
%      unsigned int GetMagickGeometry(const char *geometry,long *x,long *y,
%        unsigned long *width,unsigned long *height)
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
MagickExport unsigned int GetMagickGeometry(const char *geometry,long *x,
  long *y,unsigned long *width,unsigned long *height)
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
%  The format of the GetOnePixel() method is:
%
%      PixelPacket GetOnePixel(const Image image,const long x,const long y)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o x,y:  These values define the location of the pixel to return.
%
*/
MagickExport PixelPacket GetOnePixel(Image *image,const long x,const long y)
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
  *red=RoundToQuantum((MagickRealType) QuantumRange*r);
  *green=RoundToQuantum((MagickRealType) QuantumRange*g);
  *blue=RoundToQuantum((MagickRealType) QuantumRange*b);
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

static inline long NearestNeighbor(MagickRealType x)
{
  if (x >= 0.0)
    return((long) (x+0.5));
  return((long) (x-0.5));
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

  register long
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

      p=GetCacheViewVirtualPixels(image_view,(long) floor(x)-1,(long) floor(y)-1,
        4,4,exception);
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
            alpha[i]=QuantumScale*((MagickRealType) QuantumRange-p->opacity);
            pixels[i].red*=alpha[i];
            pixels[i].green*=alpha[i];
            pixels[i].blue*=alpha[i];
            if (image->colorspace == CMYKColorspace)
              pixels[i].index*=alpha[i];
          }
        gamma=alpha[i];
        gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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

      p=GetCacheViewVirtualPixels(image_view,(long) floor(x)-1,(long) floor(y)-1,
        4,4,exception);
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
            alpha[i]=QuantumScale*((MagickRealType) QuantumRange-p->opacity);
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

      p=GetCacheViewVirtualPixels(image_view,(long) floor(x),(long) floor(y),2,2,
        exception);
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
            alpha[i]=QuantumScale*((MagickRealType) QuantumRange-p->opacity);
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
      gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
      geometry.x=(long) floor(x)-1L;
      geometry.y=(long) floor(y)-1L;
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

      p=GetCacheViewVirtualPixels(image_view,(long) floor(x),(long) floor(y),1,1,
        exception);
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

      p=GetCacheViewVirtualPixels(image_view,(long) floor(x),(long) floor(y),
        2,2,exception);
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
            alpha[i]=QuantumScale*((MagickRealType) QuantumRange-p->opacity);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
              gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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

      p=GetCacheViewVirtualPixels(image_view,NearestNeighbor(x),NearestNeighbor(y),
        1,1,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewVirtualIndexQueue(image_view);
      GetMagickPixelPacket(image,pixels);
      SetMagickPixelPacket(image,p,indexes,&pixel);
      break;
    }
    case SplineInterpolatePixel:
    {
      long
        j,
        n;

      MagickPixelPacket
        pixels[16];

      MagickRealType
        alpha[16],
        dx,
        dy,
        gamma;

      PointInfo
        delta;

      p=GetCacheViewVirtualPixels(image_view,(long) floor(x)-1,(long) floor(y)-1,
        4,4,exception);
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
              alpha[n]=QuantumScale*((MagickRealType) QuantumRange-p->opacity);
              pixels[n].red*=alpha[n];
              pixels[n].green*=alpha[n];
              pixels[n].blue*=alpha[n];
              if (image->colorspace == CMYKColorspace)
                pixels[n].index*=alpha[n];
            }
          dx=CubicWeightingFunction(delta.x-(MagickRealType) j);
          gamma=alpha[n];
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
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
  RelinquishSemaphoreInfo(*semaphore_info);
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
  InitializeMagick(path);
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
%        const PixelPacket target,const Quantum opacity,const long x_offset,
%        const long y_offset,const PaintMethod method)
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
  const PixelPacket target,const Quantum opacity,const long x_offset,
  const long y_offset,const PaintMethod method)
{
  Image
    *floodplane_image;

  long
    offset,
    start,
    x,
    x1,
    x2,
    y;

  MagickBooleanType
    skip;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((x_offset < 0) || (x_offset >= (long) image->columns))
    return(MagickFalse);
  if ((y_offset < 0) || (y_offset >= (long) image->rows))
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
      *__restrict p;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Pop segment off stack.
    */
    s--;
    x1=(long) s->x1;
    x2=(long) s->x2;
    offset=(long) s->y2;
    y=(long) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    p=GetVirtualPixels(image,0,y,(unsigned long) (x1+1),1,&image->exception);
    q=GetAuthenticPixels(floodplane_image,0,y,(unsigned long) (x1+1),1,
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
          if (x < (long) image->columns)
            {
              p=GetVirtualPixels(image,x,y,image->columns-x,1,
                &image->exception);
              q=GetAuthenticPixels(floodplane_image,x,y,image->columns-x,1,
                &image->exception);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for ( ; x < (long) image->columns; x++)
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
          p=GetVirtualPixels(image,x,y,(unsigned long) (x2-x+1),1,
            &image->exception);
          q=GetAuthenticPixels(floodplane_image,x,y,(unsigned long) (x2-x+1),1,
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
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *__restrict p;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Tile fill color onto floodplane.
    */
    p=GetVirtualPixels(floodplane_image,0,y,image->columns,1,
      &image->exception);
    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (long) image->columns; x++)
    {
      if (p->opacity != OpaqueOpacity)
        q->opacity=opacity;
      p++;
      q++;
    }
    if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
      break;
  }
  segment_stack=(SegmentInfo *) RelinquishMagickMemory(segment_stack);
  floodplane_image=DestroyImage(floodplane_image);
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
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

  long
    y;

  MagickBooleanType
    proceed;

  register long
    i;

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
      for (y=0; y < (long) image->rows; y++)
      {
        register long
          x;

        register PixelPacket
          *__restrict q;

        q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (long) image->columns; x++)
        {
          if (IsColorSimilar(image,q,&target) != MagickFalse)
            *q=fill;
          q++;
        }
        if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
          break;
        proceed=SetImageProgress(image,OpaqueImageTag,y,image->rows);
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
      for (i=0; i < (long) image->colors; i++)
      {
        if (IsColorSimilar(image,&image->colormap[i],&target) != MagickFalse)
          image->colormap[i]=fill;
      }
      if (fill.opacity != OpaqueOpacity)
        {
          for (y=0; y < (long) image->rows; y++)
          {
            register long
              x;

            register PixelPacket
              *__restrict q;

            q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (long) image->columns; x++)
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
  return(AcquireCacheView(image));
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
%  The format of the PaintFloodfillImage method is:
%
%      MagickBooleanType PaintFloodfillImage(Image *image,
%        const ChannelType channel,const MagickPixelPacket target,const long x,
%        const long y,const DrawInfo *draw_info,const PaintMethod method)
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
  const ChannelType channel,const MagickPixelPacket *target,const long x,
  const long y,const DrawInfo *draw_info,const PaintMethod method)
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
  return(PaintOpaqueImageChannel(image,DefaultChannels,target,fill));
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
%  The format of the ParseImageGeometry method is:
%
%      int ParseImageGeometry(char *geometry,long *x,long *y,
%        unsigned long *width,unsigned long *height)
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
MagickExport int ParseImageGeometry(const char *geometry,long *x,long *y,
  unsigned long *width,unsigned long *height)
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

  long
    count,
    y;

  RandomInfo
    *random_info;

  static MagickRealType
    o2[4]={0.2f, 0.6f, 0.8f, 0.4f}, 
    o3[9]={0.1f, 0.6f, 0.3f, 0.7f, 0.5f, 0.8f, 0.4f, 0.9f, 0.2f}, 
    o4[16]={0.1f, 0.7f, 1.1f, 0.3f, 1.0f, 0.5f, 1.5f, 0.8f, 1.4f, 1.6f, 0.6f,
      1.2f, 0.4f, 0.9f, 1.3f, 0.2f}, 
    threshold=128;

  unsigned int
    status;

  unsigned long
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
          count=sscanf(thresholds,"%lf[/x%%]%lf",&lower_threshold,
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
  for (y=0; y < (long) image->rows; y++)
  {
    register long
      x;

    register IndexPacket
      index,
      *__restrict indexes;

    register PixelPacket
      *__restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    if (LocaleCompare(channel,"all") == 0 ||
        LocaleCompare(channel,"intensity") == 0)
      {
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < (long) image->columns; x++)
          {
            MagickRealType
              intensity;

            intensity=(MagickRealType) PixelIntensityToQuantum(q);
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
            q->red=q->green=q->blue=(Quantum) (intensity <=
               threshold ? 0 : QuantumRange);
            index=(IndexPacket) (intensity <= threshold ? 0 : 1);
            *indexes++=index;
            q->red=q->green=q->blue=image->colormap[(long) index].red;
            q++;
          }
      }
    if (LocaleCompare(channel,"opacity") == 0 ||
        LocaleCompare(channel,"all") == 0 ||
        LocaleCompare(channel,"matte") == 0)
      {
        if (image->matte != MagickFalse)
          for (x=0; x < (long) image->columns; x++)
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
              q->opacity=(Quantum) ((MagickRealType) q->opacity <= threshold ?
                 0 : QuantumRange);
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
    if (QuantumTick(y,image->rows) != MagickFalse)
      {
        status=MagickMonitor(RandomChannelThresholdImageText,y,image->rows,
          exception);
        if (status == MagickFalse)
          break;
      }
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
%  The format of the SetCacheViewPixels method is:
%
%      PixelPacket *SetCacheViewPixels(CacheView *cache_view,const long x,
%        const long y,const unsigned long columns,const unsigned long rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *SetCacheViewPixels(CacheView *cache_view,const long x,
  const long y,const unsigned long columns,const unsigned long rows)
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
MagickExport void SetCacheThreshold(const unsigned long size)
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
  long
    y;

  PixelPacket
    background_color;

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
      for (y=0; y < (long) image->rows; y++)
      {
        register IndexPacket
          *__restrict indexes;

        register long
          x;

        register PixelPacket
          *__restrict q;

        q=QueueAuthenticPixels(image,0,y,image->columns,1,&image->exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (long) image->columns; x++)
          *q++=background_color;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < (long) image->columns; x++)
          indexes[x]=(IndexPacket) 0;
        if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
          break;
      }
      return;
    }
  /*
    Set DirectClass image.
  */
  for (y=0; y < (long) image->rows; y++)
  {
    register long
      x;

    register PixelPacket
      *__restrict q;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (long) image->columns; x++)
      *q++=background_color;
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
%        const long offset,ExceptionInfo *exception)
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
  const long offset,ExceptionInfo *exception)
{
  Image
    *clone;

  register long
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
%  If the region is successfully intialized a pointer to a PixelPacket
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
%  The format of the SetImagePixels() method is:
%
%      PixelPacket *SetImagePixels(Image *image,const long x,const long y,
%        const unsigned long columns,const unsigned long rows)
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
MagickExport PixelPacket *SetImagePixels(Image *image,const long x,const long y,
  const unsigned long columns,const unsigned long rows)
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
%      long SetMagickRegistry(const RegistryType type,const void *blob,
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
MagickExport long SetMagickRegistry(const RegistryType type,const void *blob,
  const size_t magick_unused(length),ExceptionInfo *exception)
{
  char
    key[MaxTextExtent];

  MagickBooleanType
    status;

  static long
    id = 0;

  (void) FormatMagickString(key,MaxTextExtent,"%ld\n",id);
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
%      Image *SpliceImageList(Image *images,const long offset,
%        const unsigned long length,const Image *splices,
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
MagickExport Image *SpliceImageList(Image *images,const long offset,
  const unsigned long length,const Image *splices,ExceptionInfo *exception)
{
  Image
    *clone;

  register long
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

  long
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
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      index=(IndexPacket) ((MagickRealType)
        PixelIntensityToQuantum(q) <= threshold ? 0 : 1);
      indexes[x]=index;
      q->red=image->colormap[(long) index].red;
      q->green=image->colormap[(long) index].green;
      q->blue=image->colormap[(long) index].blue;
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

  long
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
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    if (IsMagickGray(&pixel) != MagickFalse)
      for (x=0; x < (long) image->columns; x++)
      {
        index=(IndexPacket) ((MagickRealType)
          PixelIntensityToQuantum(q) <= pixel.red ? 0 : 1);
        indexes[x]=index;
        q->red=image->colormap[(long) index].red;
        q->green=image->colormap[(long) index].green;
        q->blue=image->colormap[(long) index].blue;
        q++;
      }
    else
      for (x=0; x < (long) image->columns; x++)
      {
        q->red=(Quantum) ((MagickRealType)
          q->red <= pixel.red ? 0 : QuantumRange);
        q->green=(Quantum) ((MagickRealType)
          q->green <= pixel.green ? 0 : QuantumRange);
        q->blue=(Quantum) ((MagickRealType)
          q->blue <= pixel.blue ? 0 : QuantumRange);
        q->opacity=(Quantum) ((MagickRealType)
          q->opacity <= pixel.opacity ? 0 : QuantumRange);
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

  long
    y;

  MagickBooleanType
    proceed;

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
  for (y=0; y < (long) image->rows; y++)
  {
    register long
      x;

    register PixelPacket
      *__restrict q;

    q=GetAuthenticPixels(image,0,y,image->columns,1,&image->exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (long) image->columns; x++)
    {
      if (IsColorSimilar(image,q,&target) != MagickFalse)
        q->opacity=opacity;
      q++;
    }
    if (SyncAuthenticPixels(image,&image->exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,TransparentImageTag,y,image->rows);
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
  const unsigned long index)
{
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DeprecateEvent,GetMagickModule(),"last use: v5.4.4");
  return(ConstrainColormapIndex(image,index));
}
#endif
