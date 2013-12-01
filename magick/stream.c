/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  SSSSS  TTTTT  RRRR   EEEEE   AAA   M   M                   %
%                  SS       T    R   R  E      A   A  MM MM                   %
%                   SSS     T    RRRR   EEE    AAAAA  M M M                   %
%                     SS    T    R R    E      A   A  M   M                   %
%                  SSSSS    T    R  R   EEEEE  A   A  M   M                   %
%                                                                             %
%                                                                             %
%                       MagickCore Pixel Stream Methods                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2000                                  %
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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/color-private.h"
#include "magick/composite-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/memory_.h"
#include "magick/memory-private.h"
#include "magick/pixel.h"
#include "magick/quantum.h"
#include "magick/quantum-private.h"
#include "magick/semaphore.h"
#include "magick/stream.h"
#include "magick/stream-private.h"
#include "magick/string_.h"

/*
  Typedef declaractions.
*/
struct _StreamInfo
{
  const ImageInfo
    *image_info;

  const Image
    *image;

  Image
    *stream;

  QuantumInfo
    *quantum_info;

  char
    *map;

  StorageType
    storage_type;

  unsigned char
    *pixels;

  RectangleInfo
    extract_info;

  ssize_t
    y;

  ExceptionInfo
    *exception;

  const void
    *client_data;

  size_t
    signature;
};

/*
  Declare pixel cache interfaces.
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static const PixelPacket
  *GetVirtualPixelStream(const Image *,const VirtualPixelMethod,const ssize_t,
    const ssize_t,const size_t,const size_t,ExceptionInfo *);

static MagickBooleanType
  StreamImagePixels(const StreamInfo *,const Image *,ExceptionInfo *),
  SyncAuthenticPixelsStream(Image *,ExceptionInfo *);

static PixelPacket
  *QueueAuthenticPixelsStream(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e S t r e a m I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireStreamInfo() allocates the StreamInfo structure.
%
%  The format of the AcquireStreamInfo method is:
%
%      StreamInfo *AcquireStreamInfo(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport StreamInfo *AcquireStreamInfo(const ImageInfo *image_info)
{
  StreamInfo
    *stream_info;

  stream_info=(StreamInfo *) AcquireMagickMemory(sizeof(*stream_info));
  if (stream_info == (StreamInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(stream_info,0,sizeof(*stream_info));
  stream_info->pixels=(unsigned char *) MagickAssumeAligned(
    AcquireAlignedMemory(1,sizeof(*stream_info->pixels)));
  if (stream_info->pixels == (unsigned char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  stream_info->map=ConstantString("RGB");
  stream_info->storage_type=CharPixel;
  stream_info->stream=AcquireImage(image_info);
  stream_info->signature=MagickSignature;
  return(stream_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y P i x e l S t r e a m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelStream() deallocates memory associated with the pixel stream.
%
%  The format of the DestroyPixelStream() method is:
%
%      void DestroyPixelStream(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/

static inline void RelinquishStreamPixels(CacheInfo *cache_info)
{
  assert(cache_info != (CacheInfo *) NULL);
  if (cache_info->mapped == MagickFalse)
    (void) RelinquishAlignedMemory(cache_info->pixels);
  else
    (void) UnmapBlob(cache_info->pixels,(size_t) cache_info->length);
  cache_info->pixels=(PixelPacket *) NULL;
  cache_info->indexes=(IndexPacket *) NULL;
  cache_info->length=0;
  cache_info->mapped=MagickFalse;
}

static void DestroyPixelStream(Image *image)
{
  CacheInfo
    *cache_info;

  MagickBooleanType
    destroy;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  destroy=MagickFalse;
  LockSemaphoreInfo(cache_info->semaphore);
  cache_info->reference_count--;
  if (cache_info->reference_count == 0)
    destroy=MagickTrue;
  UnlockSemaphoreInfo(cache_info->semaphore);
  if (destroy == MagickFalse)
    return;
  RelinquishStreamPixels(cache_info);
  if (cache_info->nexus_info != (NexusInfo **) NULL)
    cache_info->nexus_info=DestroyPixelCacheNexus(cache_info->nexus_info,
      cache_info->number_threads);
  if (cache_info->file_semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&cache_info->file_semaphore);
  if (cache_info->semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&cache_info->semaphore);
  cache_info=(CacheInfo *) RelinquishMagickMemory(cache_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y S t r e a m I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyStreamInfo() destroys memory associated with the StreamInfo
%  structure.
%
%  The format of the DestroyStreamInfo method is:
%
%      StreamInfo *DestroyStreamInfo(StreamInfo *stream_info)
%
%  A description of each parameter follows:
%
%    o stream_info: the stream info.
%
*/
MagickExport StreamInfo *DestroyStreamInfo(StreamInfo *stream_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(stream_info != (StreamInfo *) NULL);
  assert(stream_info->signature == MagickSignature);
  if (stream_info->map != (char *) NULL)
    stream_info->map=DestroyString(stream_info->map);
  if (stream_info->pixels != (unsigned char *) NULL)
    stream_info->pixels=(unsigned char *) RelinquishAlignedMemory(
      stream_info->pixels);
  if (stream_info->stream != (Image *) NULL)
    {
      (void) CloseBlob(stream_info->stream);
      stream_info->stream=DestroyImage(stream_info->stream);
    }
  if (stream_info->quantum_info != (QuantumInfo *) NULL)
    stream_info->quantum_info=DestroyQuantumInfo(stream_info->quantum_info);
  stream_info->signature=(~MagickSignature);
  stream_info=(StreamInfo *) RelinquishMagickMemory(stream_info);
  return(stream_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c I n d e x e s F r o m S t r e a m                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticIndexesFromStream() returns the indexes associated with the
%  last call to QueueAuthenticPixelsStream() or GetAuthenticPixelsStream().
%
%  The format of the GetAuthenticIndexesFromStream() method is:
%
%      IndexPacket *GetAuthenticIndexesFromStream(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static IndexPacket *GetAuthenticIndexesFromStream(const Image *image)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  return(cache_info->indexes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c P i x e l S t r e a m                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticPixelsStream() gets pixels from the in-memory or disk pixel
%  cache as defined by the geometry parameters.   A pointer to the pixels is
%  returned if the pixels are transferred, otherwise a NULL is returned.  For
%  streams this method is a no-op.
%
%  The format of the GetAuthenticPixelsStream() method is:
%
%      PixelPacket *GetAuthenticPixelsStream(Image *image,const ssize_t x,
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
static PixelPacket *GetAuthenticPixelsStream(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  PixelPacket
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  pixels=QueueAuthenticPixelsStream(image,x,y,columns,rows,exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t A u t h e n t i c P i x e l F r o m S t e a m                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticPixelsFromStream() returns the pixels associated with the last
%  call to QueueAuthenticPixelsStream() or GetAuthenticPixelsStream().
%
%  The format of the GetAuthenticPixelsFromStream() method is:
%
%      PixelPacket *GetAuthenticPixelsFromStream(const Image image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static PixelPacket *GetAuthenticPixelsFromStream(const Image *image)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  return(cache_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t O n e A u t h e n t i c P i x e l F r o m S t r e a m               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneAuthenticPixelFromStream() returns a single pixel at the specified
%  (x,y) location.  The image background color is returned if an error occurs.
%
%  The format of the GetOneAuthenticPixelFromStream() method is:
%
%      MagickBooleanType GetOneAuthenticPixelFromStream(const Image image,
%        const ssize_t x,const ssize_t y,PixelPacket *pixel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType GetOneAuthenticPixelFromStream(Image *image,
  const ssize_t x,const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
{
  register PixelPacket
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  *pixel=image->background_color;
  pixels=GetAuthenticPixelsStream(image,x,y,1,1,exception);
  if (pixels != (PixelPacket *) NULL)
    return(MagickFalse);
  *pixel=(*pixels);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t O n e V i r t u a l P i x e l F r o m S t r e a m                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneVirtualPixelFromStream() returns a single pixel at the specified
%  (x.y) location.  The image background color is returned if an error occurs.
%
%  The format of the GetOneVirtualPixelFromStream() method is:
%
%      MagickBooleanType GetOneVirtualPixelFromStream(const Image image,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,PixelPacket *pixel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y:  These values define the location of the pixel to return.
%
%    o pixel: return a pixel at the specified (x,y) location.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType GetOneVirtualPixelFromStream(const Image *image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,const ssize_t y,
  PixelPacket *pixel,ExceptionInfo *exception)
{
  const PixelPacket
    *pixels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  *pixel=image->background_color;
  pixels=GetVirtualPixelStream(image,virtual_pixel_method,x,y,1,1,exception);
  if (pixels != (const PixelPacket *) NULL)
    return(MagickFalse);
  *pixel=(*pixels);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t S t r e a m I n f o C l i e n t D a t a                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStreamInfoClientData() gets the stream info client data.
%
%  The format of the SetStreamInfoClientData method is:
%
%      const void *GetStreamInfoClientData(StreamInfo *stream_info)
%
%  A description of each parameter follows:
%
%    o stream_info: the stream info.
%
*/
MagickExport const void *GetStreamInfoClientData(StreamInfo *stream_info)
{
  assert(stream_info != (StreamInfo *) NULL);
  assert(stream_info->signature == MagickSignature);
  return(stream_info->client_data);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t  V i r t u a l P i x e l s F r o m S t r e a m                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixelsStream() returns the pixels associated with the last
%  call to QueueAuthenticPixelsStream() or GetVirtualPixelStream().
%
%  The format of the GetVirtualPixelsStream() method is:
%
%      const IndexPacket *GetVirtualPixelsStream(const Image *image)
%
%  A description of each parameter follows:
%
%    o pixels: return the pixels associated with the last call to
%      QueueAuthenticPixelsStream() or GetVirtualPixelStream().
%
%    o image: the image.
%
*/
static const PixelPacket *GetVirtualPixelsStream(const Image *image)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  return(cache_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l I n d e x e s F r o m S t r e a m                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualIndexesFromStream() returns the indexes associated with the last
%  call to QueueAuthenticPixelsStream() or GetVirtualPixelStream().
%
%  The format of the GetVirtualIndexesFromStream() method is:
%
%      const IndexPacket *GetVirtualIndexesFromStream(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static const IndexPacket *GetVirtualIndexesFromStream(const Image *image)
{
  CacheInfo
    *cache_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  return(cache_info->indexes);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t V i r t u a l P i x e l S t r e a m                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualPixelStream() gets pixels from the in-memory or disk pixel cache as
%  defined by the geometry parameters.   A pointer to the pixels is returned if
%  the pixels are transferred, otherwise a NULL is returned.  For streams this
%  method is a no-op.
%
%  The format of the GetVirtualPixelStream() method is:
%
%      const PixelPacket *GetVirtualPixelStream(const Image *image,
%        const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
%        const ssize_t y,const size_t columns,const size_t rows,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType AcquireStreamPixels(CacheInfo *cache_info,
  ExceptionInfo *exception)
{
  if (cache_info->length != (MagickSizeType) ((size_t) cache_info->length))
    return(MagickFalse);
  cache_info->mapped=MagickFalse;
  cache_info->pixels=(PixelPacket *) MagickAssumeAligned(AcquireAlignedMemory(1,
    (size_t) cache_info->length));
  if (cache_info->pixels == (PixelPacket *) NULL)
    {
      cache_info->mapped=MagickTrue;
      cache_info->pixels=(PixelPacket *) MapBlob(-1,IOMode,0,(size_t)
        cache_info->length);
    }
  if (cache_info->pixels == (PixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",
        cache_info->filename);
      return(MagickFalse);
    }
  return(MagickTrue);
}

static const PixelPacket *GetVirtualPixelStream(const Image *image,
  const VirtualPixelMethod magick_unused(virtual_pixel_method),const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  size_t
    length;

  magick_unreferenced(virtual_pixel_method);

  /*
    Validate pixel cache geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((x < 0) || (y < 0) ||
      ((x+(ssize_t) columns) > (ssize_t) image->columns) ||
      ((y+(ssize_t) rows) > (ssize_t) image->rows) ||
      (columns == 0) || (rows == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),StreamError,
        "ImageDoesNotContainTheStreamGeometry","`%s'",image->filename);
      return((PixelPacket *) NULL);
    }
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  /*
    Pixels are stored in a temporary buffer until they are synced to the cache.
  */
  cache_info->active_index_channel=((image->storage_class == PseudoClass) ||
    (image->colorspace == CMYKColorspace)) ? MagickTrue : MagickFalse;
  number_pixels=(MagickSizeType) columns*rows;
  length=(size_t) number_pixels*sizeof(PixelPacket);
  if (cache_info->active_index_channel != MagickFalse)
    length+=number_pixels*sizeof(IndexPacket);
  if (cache_info->pixels == (PixelPacket *) NULL)
    {
      cache_info->length=length;
      status=AcquireStreamPixels(cache_info,exception);
      if (status == MagickFalse)
        {
          cache_info->length=0;
          return((PixelPacket *) NULL);
        }
    }
  else
    if (cache_info->length < length)
      {
        RelinquishStreamPixels(cache_info);
        cache_info->length=length;
        status=AcquireStreamPixels(cache_info,exception);
        if (status == MagickFalse)
          {
            cache_info->length=0;
            return((PixelPacket *) NULL);
          }
      }
  cache_info->indexes=(IndexPacket *) NULL;
  if (cache_info->active_index_channel != MagickFalse)
    cache_info->indexes=(IndexPacket *) (cache_info->pixels+number_pixels);
  return(cache_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n S t r e a m                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenStream() opens a stream for writing by the StreamImage() method.
%
%  The format of the OpenStream method is:
%
%       MagickBooleanType OpenStream(const ImageInfo *image_info,
%        StreamInfo *stream_info,const char *filename,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o stream_info: the stream info.
%
%    o filename: the stream filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType OpenStream(const ImageInfo *image_info,
  StreamInfo *stream_info,const char *filename,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  (void) CopyMagickString(stream_info->stream->filename,filename,MaxTextExtent);
  status=OpenBlob(image_info,stream_info->stream,WriteBinaryBlobMode,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   Q u e u e A u t h e n t i c P i x e l s S t r e a m                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QueueAuthenticPixelsStream() allocates an area to store image pixels as
%  defined by the region rectangle and returns a pointer to the area.  This
%  area is subsequently transferred from the pixel cache with method
%  SyncAuthenticPixelsStream().  A pointer to the pixels is returned if the
%  pixels are transferred, otherwise a NULL is returned.
%
%  The format of the QueueAuthenticPixelsStream() method is:
%
%      PixelPacket *QueueAuthenticPixelsStream(Image *image,const ssize_t x,
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
*/
static PixelPacket *QueueAuthenticPixelsStream(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  size_t
    length;

  StreamHandler
    stream_handler;

  /*
    Validate pixel cache geometry.
  */
  assert(image != (Image *) NULL);
  if ((x < 0) || (y < 0) ||
      ((x+(ssize_t) columns) > (ssize_t) image->columns) ||
      ((y+(ssize_t) rows) > (ssize_t) image->rows) ||
      (columns == 0) || (rows == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),StreamError,
        "ImageDoesNotContainTheStreamGeometry","`%s'",image->filename);
      return((PixelPacket *) NULL);
    }
  stream_handler=GetBlobStreamHandler(image);
  if (stream_handler == (StreamHandler) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),StreamError,
        "NoStreamHandlerIsDefined","`%s'",image->filename);
      return((PixelPacket *) NULL);
    }
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  if ((image->storage_class != GetPixelCacheStorageClass(image->cache)) ||
      (image->colorspace != GetPixelCacheColorspace(image->cache)))
    {
      if (GetPixelCacheStorageClass(image->cache) == UndefinedClass)
        (void) stream_handler(image,(const void *) NULL,(size_t)
          cache_info->columns);
      cache_info->storage_class=image->storage_class;
      cache_info->colorspace=image->colorspace;
      cache_info->columns=image->columns;
      cache_info->rows=image->rows;
      image->cache=cache_info;
    }
  /*
    Pixels are stored in a temporary buffer until they are synced to the cache.
  */
  cache_info->active_index_channel=((image->storage_class == PseudoClass) ||
    (image->colorspace == CMYKColorspace)) ? MagickTrue : MagickFalse;
  cache_info->columns=columns;
  cache_info->rows=rows;
  number_pixels=(MagickSizeType) columns*rows;
  length=(size_t) number_pixels*sizeof(PixelPacket);
  if (cache_info->active_index_channel != MagickFalse)
    length+=number_pixels*sizeof(IndexPacket);
  if (cache_info->pixels == (PixelPacket *) NULL)
    {
      cache_info->length=length;
      status=AcquireStreamPixels(cache_info,exception);
      if (status == MagickFalse)
        {
          cache_info->length=0;
          return((PixelPacket *) NULL);
        }
    }
  else
    if (cache_info->length < length)
      {
        RelinquishStreamPixels(cache_info);
        cache_info->length=length;
        status=AcquireStreamPixels(cache_info,exception);
        if (status == MagickFalse)
          {
            cache_info->length=0;
            return((PixelPacket *) NULL);
          }
      }
  cache_info->indexes=(IndexPacket *) NULL;
  if (cache_info->active_index_channel != MagickFalse)
    cache_info->indexes=(IndexPacket *) (cache_info->pixels+number_pixels);
  return(cache_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S t r e a m                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadStream() makes the image pixels available to a user supplied callback
%  method immediately upon reading a scanline with the ReadImage() method.
%
%  The format of the ReadStream() method is:
%
%      Image *ReadStream(const ImageInfo *image_info,StreamHandler stream,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o stream: a callback method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ReadStream(const ImageInfo *image_info,StreamHandler stream,
  ExceptionInfo *exception)
{
  CacheMethods
    cache_methods;

  Image
    *image;

  ImageInfo
    *read_info;

  /*
    Stream image pixels.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  read_info=CloneImageInfo(image_info);
  read_info->cache=AcquirePixelCache(0);
  GetPixelCacheMethods(&cache_methods);
  cache_methods.get_virtual_pixel_handler=GetVirtualPixelStream;
  cache_methods.get_virtual_indexes_from_handler=GetVirtualIndexesFromStream;
  cache_methods.get_virtual_pixels_handler=GetVirtualPixelsStream;
  cache_methods.get_authentic_pixels_handler=GetAuthenticPixelsStream;
  cache_methods.queue_authentic_pixels_handler=QueueAuthenticPixelsStream;
  cache_methods.sync_authentic_pixels_handler=SyncAuthenticPixelsStream;
  cache_methods.get_authentic_pixels_from_handler=GetAuthenticPixelsFromStream;
  cache_methods.get_authentic_indexes_from_handler=
    GetAuthenticIndexesFromStream;
  cache_methods.get_one_virtual_pixel_from_handler=GetOneVirtualPixelFromStream;
  cache_methods.get_one_authentic_pixel_from_handler=
    GetOneAuthenticPixelFromStream;
  cache_methods.destroy_pixel_handler=DestroyPixelStream;
  SetPixelCacheMethods(read_info->cache,&cache_methods);
  read_info->stream=stream;
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t S t r e a m I n f o C l i e n t D a t a                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStreamInfoClientData() sets the stream info client data.
%
%  The format of the SetStreamInfoClientData method is:
%
%      void SetStreamInfoClientData(StreamInfo *stream_info,
%        const void *client_data)
%
%  A description of each parameter follows:
%
%    o stream_info: the stream info.
%
%    o client_data: the client data.
%
*/
MagickExport void SetStreamInfoClientData(StreamInfo *stream_info,
  const void *client_data)
{
  assert(stream_info != (StreamInfo *) NULL);
  assert(stream_info->signature == MagickSignature);
  stream_info->client_data=client_data;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t S t r e a m I n f o M a p                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStreamInfoMap() sets the stream info map member.
%
%  The format of the SetStreamInfoMap method is:
%
%      void SetStreamInfoMap(StreamInfo *stream_info,const char *map)
%
%  A description of each parameter follows:
%
%    o stream_info: the stream info.
%
%    o map: the map.
%
*/
MagickExport void SetStreamInfoMap(StreamInfo *stream_info,const char *map)
{
  assert(stream_info != (StreamInfo *) NULL);
  assert(stream_info->signature == MagickSignature);
  (void) CloneString(&stream_info->map,map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t S t r e a m I n f o S t o r a g e T y p e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStreamInfoStorageType() sets the stream info storage type member.
%
%  The format of the SetStreamInfoStorageType method is:
%
%      void SetStreamInfoStorageType(StreamInfo *stream_info,
%        const StoreageType *storage_type)
%
%  A description of each parameter follows:
%
%    o stream_info: the stream info.
%
%    o storage_type: the storage type.
%
*/
MagickExport void SetStreamInfoStorageType(StreamInfo *stream_info,
  const StorageType storage_type)
{
  assert(stream_info != (StreamInfo *) NULL);
  assert(stream_info->signature == MagickSignature);
  stream_info->storage_type=storage_type;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t r e a m I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StreamImage() streams pixels from an image and writes them in a user
%  defined format and storage type (e.g. RGBA as 8-bit unsigned char).
%
%  The format of the StreamImage() method is:
%
%      Image *StreamImage(const ImageInfo *image_info,
%        StreamInfo *stream_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o stream_info: the stream info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static size_t WriteStreamImage(const Image *image,const void *pixels,
  const size_t columns)
{
  CacheInfo
    *cache_info;

  RectangleInfo
    extract_info;

  size_t
    length,
    packet_size;

  ssize_t
    count;

  StreamInfo
    *stream_info;

  (void) pixels;
  stream_info=(StreamInfo *) image->client_data;
  switch (stream_info->storage_type)
  {
    default: packet_size=sizeof(char); break;
    case CharPixel: packet_size=sizeof(char); break;
    case DoublePixel: packet_size=sizeof(double); break;
    case FloatPixel: packet_size=sizeof(float); break;
    case IntegerPixel: packet_size=sizeof(int); break;
    case LongPixel: packet_size=sizeof(ssize_t); break;
    case QuantumPixel: packet_size=sizeof(Quantum); break;
    case ShortPixel: packet_size=sizeof(unsigned short); break;
  }
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  packet_size*=strlen(stream_info->map);
  length=packet_size*cache_info->columns*cache_info->rows;
  if (image != stream_info->image)
    {
      ImageInfo
        *write_info;

      /*
        Prepare stream for writing.
      */
      (void) RelinquishAlignedMemory(stream_info->pixels);
      stream_info->pixels=(unsigned char *) MagickAssumeAligned(
        AcquireAlignedMemory(1,length));
      if (stream_info->pixels == (unsigned char *) NULL)
        return(0);
      (void) ResetMagickMemory(stream_info->pixels,0,length);
      stream_info->image=image;
      write_info=CloneImageInfo(stream_info->image_info);
      (void) SetImageInfo(write_info,1,stream_info->exception);
      if (write_info->extract != (char *) NULL)
        (void) ParseAbsoluteGeometry(write_info->extract,
          &stream_info->extract_info);
      stream_info->y=0;
      write_info=DestroyImageInfo(write_info);
    }
  extract_info=stream_info->extract_info;
  if ((extract_info.width == 0) || (extract_info.height == 0))
    {
      /*
        Write all pixels to stream.
      */
      (void) StreamImagePixels(stream_info,image,stream_info->exception);
      count=WriteBlob(stream_info->stream,length,stream_info->pixels);
      stream_info->y++;
      return(count == 0 ? 0 : columns);
    }
  if ((stream_info->y < extract_info.y) ||
      (stream_info->y >= (ssize_t) (extract_info.y+extract_info.height)))
    {
      stream_info->y++;
      return(columns);
    }
  /*
    Write a portion of the pixel row to the stream.
  */
  (void) StreamImagePixels(stream_info,image,stream_info->exception);
  length=packet_size*extract_info.width;
  count=WriteBlob(stream_info->stream,length,stream_info->pixels+packet_size*
    extract_info.x);
  stream_info->y++;
  return(count == 0 ? 0 : columns);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport Image *StreamImage(const ImageInfo *image_info,
  StreamInfo *stream_info,ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *read_info;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(stream_info != (StreamInfo *) NULL);
  assert(stream_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  read_info=CloneImageInfo(image_info);
  stream_info->image_info=image_info;
  stream_info->quantum_info=AcquireQuantumInfo(image_info,(Image *) NULL);
  stream_info->exception=exception;
  read_info->client_data=(void *) stream_info;
  image=ReadStream(read_info,&WriteStreamImage,exception);
  read_info=DestroyImageInfo(read_info);
  stream_info->quantum_info=DestroyQuantumInfo(stream_info->quantum_info);
  stream_info->quantum_info=AcquireQuantumInfo(image_info,image);
  if (stream_info->quantum_info == (QuantumInfo *) NULL)
    image=DestroyImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t r e a m I m a g e P i x e l s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StreamImagePixels() extracts pixel data from an image and returns it in the
%  stream_info->pixels structure in the format as defined by
%  stream_info->quantum_info->map and stream_info->quantum_info->storage_type.
%
%  The format of the StreamImagePixels method is:
%
%      MagickBooleanType StreamImagePixels(const StreamInfo *stream_info,
%        const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o stream_info: the stream info.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType StreamImagePixels(const StreamInfo *stream_info,
  const Image *image,ExceptionInfo *exception)
{
  QuantumInfo
    *quantum_info;

  QuantumType
    *quantum_map;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  size_t
    length;

  assert(stream_info != (StreamInfo *) NULL);
  assert(stream_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=strlen(stream_info->map);
  quantum_map=(QuantumType *) AcquireQuantumMemory(length,sizeof(*quantum_map));
  if (quantum_map == (QuantumType *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  for (i=0; i < (ssize_t) length; i++)
  {
    switch (stream_info->map[i])
    {
      case 'A':
      case 'a':
      {
        quantum_map[i]=AlphaQuantum;
        break;
      }
      case 'B':
      case 'b':
      {
        quantum_map[i]=BlueQuantum;
        break;
      }
      case 'C':
      case 'c':
      {
        quantum_map[i]=CyanQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",stream_info->map);
        return(MagickFalse);
      }
      case 'g':
      case 'G':
      {
        quantum_map[i]=GreenQuantum;
        break;
      }
      case 'I':
      case 'i':
      {
        quantum_map[i]=IndexQuantum;
        break;
      }
      case 'K':
      case 'k':
      {
        quantum_map[i]=BlackQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",stream_info->map);
        return(MagickFalse);
      }
      case 'M':
      case 'm':
      {
        quantum_map[i]=MagentaQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",stream_info->map);
        return(MagickFalse);
      }
      case 'o':
      case 'O':
      {
        quantum_map[i]=OpacityQuantum;
        break;
      }
      case 'P':
      case 'p':
      {
        quantum_map[i]=UndefinedQuantum;
        break;
      }
      case 'R':
      case 'r':
      {
        quantum_map[i]=RedQuantum;
        break;
      }
      case 'Y':
      case 'y':
      {
        quantum_map[i]=YellowQuantum;
        if (image->colorspace == CMYKColorspace)
          break;
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
          "ColorSeparatedImageRequired","`%s'",stream_info->map);
        return(MagickFalse);
      }
      default:
      {
        quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "UnrecognizedPixelMap","`%s'",stream_info->map);
        return(MagickFalse);
      }
    }
  }
  quantum_info=stream_info->quantum_info;
  switch (stream_info->storage_type)
  {
    case CharPixel:
    {
      register unsigned char
        *q;

      q=(unsigned char *) stream_info->pixels;
      if (LocaleCompare(stream_info->map,"BGR") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            *q++=ScaleQuantumToChar((Quantum) (GetPixelAlpha(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            *q++=ScaleQuantumToChar((Quantum) 0);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"I") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToChar(ClampToQuantum(GetPixelIntensity(image,p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGB") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            *q++=ScaleQuantumToChar((Quantum) (GetPixelAlpha(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            *q++=ScaleQuantumToChar((Quantum) 0);
            p++;
          }
          break;
        }
      p=GetAuthenticPixelQueue(image);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) GetImageExtent(image); x++)
      {
        for (i=0; i < (ssize_t) length; i++)
        {
          *q=0;
          switch (quantum_map[i])
          {
            case RedQuantum:
            case CyanQuantum:
            {
              *q=ScaleQuantumToChar(GetPixelRed(p));
              break;
            }
            case GreenQuantum:
            case MagentaQuantum:
            {
              *q=ScaleQuantumToChar(GetPixelGreen(p));
              break;
            }
            case BlueQuantum:
            case YellowQuantum:
            {
              *q=ScaleQuantumToChar(GetPixelBlue(p));
              break;
            }
            case AlphaQuantum:
            {
              *q=ScaleQuantumToChar((Quantum) (GetPixelAlpha(p)));
              break;
            }
            case OpacityQuantum:
            {
              *q=ScaleQuantumToChar(GetPixelOpacity(p));
              break;
            }
            case BlackQuantum:
            {
              if (image->colorspace == CMYKColorspace)
                *q=ScaleQuantumToChar(GetPixelIndex(indexes+x));
              break;
            }
            case IndexQuantum:
            {
              *q=ScaleQuantumToChar(ClampToQuantum(GetPixelIntensity(image,p)));
              break;
            }
            default:
              break;
          }
          q++;
        }
        p++;
      }
      break;
    }
    case DoublePixel:
    {
      register double
        *q;

      q=(double *) stream_info->pixels;
      if (LocaleCompare(stream_info->map,"BGR") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(double) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(double) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelAlpha(p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(double) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=0.0;
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"I") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(double) ((QuantumScale*GetPixelIntensity(image,p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGB") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(double) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(double) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelAlpha(p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(double) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(double) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=0.0;
            p++;
          }
          break;
        }
      p=GetAuthenticPixelQueue(image);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) GetImageExtent(image); x++)
      {
        for (i=0; i < (ssize_t) length; i++)
        {
          *q=0;
          switch (quantum_map[i])
          {
            case RedQuantum:
            case CyanQuantum:
            {
              *q=(double) ((QuantumScale*GetPixelRed(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case GreenQuantum:
            case MagentaQuantum:
            {
              *q=(double) ((QuantumScale*GetPixelGreen(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case BlueQuantum:
            case YellowQuantum:
            {
              *q=(double) ((QuantumScale*GetPixelBlue(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case AlphaQuantum:
            {
              *q=(double) ((QuantumScale*GetPixelAlpha(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case OpacityQuantum:
            {
              *q=(double) ((QuantumScale*GetPixelOpacity(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case BlackQuantum:
            {
              if (image->colorspace == CMYKColorspace)
                *q=(double) ((QuantumScale*GetPixelIndex(indexes+x))*
                  quantum_info->scale+quantum_info->minimum);
              break;
            }
            case IndexQuantum:
            {
              *q=(double) ((QuantumScale*GetPixelIntensity(image,p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            default:
              *q=0;
          }
          q++;
        }
        p++;
      }
      break;
    }
    case FloatPixel:
    {
      register float
        *q;

      q=(float *) stream_info->pixels;
      if (LocaleCompare(stream_info->map,"BGR") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(float) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(float) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*(Quantum) (GetPixelAlpha(p)))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(float) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=0.0;
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"I") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(float) ((QuantumScale*GetPixelIntensity(image,p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGB") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(float) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(float) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelAlpha(p))*
              quantum_info->scale+quantum_info->minimum);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(float) ((QuantumScale*GetPixelRed(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelGreen(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=(float) ((QuantumScale*GetPixelBlue(p))*
              quantum_info->scale+quantum_info->minimum);
            *q++=0.0;
            p++;
          }
          break;
        }
      p=GetAuthenticPixelQueue(image);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) GetImageExtent(image); x++)
      {
        for (i=0; i < (ssize_t) length; i++)
        {
          *q=0;
          switch (quantum_map[i])
          {
            case RedQuantum:
            case CyanQuantum:
            {
              *q=(float) ((QuantumScale*GetPixelRed(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case GreenQuantum:
            case MagentaQuantum:
            {
              *q=(float) ((QuantumScale*GetPixelGreen(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case BlueQuantum:
            case YellowQuantum:
            {
              *q=(float) ((QuantumScale*GetPixelBlue(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case AlphaQuantum:
            {
              *q=(float) ((QuantumScale*GetPixelAlpha(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case OpacityQuantum:
            {
              *q=(float) ((QuantumScale*GetPixelOpacity(p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            case BlackQuantum:
            {
              if (image->colorspace == CMYKColorspace)
                *q=(float) ((QuantumScale*GetPixelIndex(indexes+x))*
                  quantum_info->scale+quantum_info->minimum);
              break;
            }
            case IndexQuantum:
            {
              *q=(float) ((QuantumScale*GetPixelIntensity(image,p))*
                quantum_info->scale+quantum_info->minimum);
              break;
            }
            default:
              *q=0;
          }
          q++;
        }
        p++;
      }
      break;
    }
    case IntegerPixel:
    {
      register unsigned int
        *q;

      q=(unsigned int *) stream_info->pixels;
      if (LocaleCompare(stream_info->map,"BGR") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
            *q++=(unsigned int) ScaleQuantumToLong((Quantum) (QuantumRange-
              GetPixelOpacity(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
            *q++=0U;
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"I") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(unsigned int) ScaleQuantumToLong(ClampToQuantum(GetPixelIntensity(image,p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGB") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
            *q++=(unsigned int) ScaleQuantumToLong((Quantum)
              (GetPixelAlpha(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
            *q++=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
            *q++=0U;
            p++;
          }
          break;
        }
      p=GetAuthenticPixelQueue(image);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) GetImageExtent(image); x++)
      {
        for (i=0; i < (ssize_t) length; i++)
        {
          *q=0;
          switch (quantum_map[i])
          {
            case RedQuantum:
            case CyanQuantum:
            {
              *q=(unsigned int) ScaleQuantumToLong(GetPixelRed(p));
              break;
            }
            case GreenQuantum:
            case MagentaQuantum:
            {
              *q=(unsigned int) ScaleQuantumToLong(GetPixelGreen(p));
              break;
            }
            case BlueQuantum:
            case YellowQuantum:
            {
              *q=(unsigned int) ScaleQuantumToLong(GetPixelBlue(p));
              break;
            }
            case AlphaQuantum:
            {
              *q=(unsigned int) ScaleQuantumToLong((Quantum) (QuantumRange-
                GetPixelOpacity(p)));
              break;
            }
            case OpacityQuantum:
            {
              *q=(unsigned int) ScaleQuantumToLong(GetPixelOpacity(p));
              break;
            }
            case BlackQuantum:
            {
              if (image->colorspace == CMYKColorspace)
                *q=(unsigned int) ScaleQuantumToLong(GetPixelIndex(
                  indexes+x));
              break;
            }
            case IndexQuantum:
            {
              *q=(unsigned int) ScaleQuantumToLong(ClampToQuantum(GetPixelIntensity(image,p)));
              break;
            }
            default:
              *q=0;
          }
          q++;
        }
        p++;
      }
      break;
    }
    case LongPixel:
    {
      register size_t
        *q;

      q=(size_t *) stream_info->pixels;
      if (LocaleCompare(stream_info->map,"BGR") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToLong(GetPixelBlue(p));
            *q++=ScaleQuantumToLong(GetPixelGreen(p));
            *q++=ScaleQuantumToLong(GetPixelRed(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToLong(GetPixelBlue(p));
            *q++=ScaleQuantumToLong(GetPixelGreen(p));
            *q++=ScaleQuantumToLong(GetPixelRed(p));
            *q++=ScaleQuantumToLong((Quantum) (GetPixelAlpha(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToLong(GetPixelBlue(p));
            *q++=ScaleQuantumToLong(GetPixelGreen(p));
            *q++=ScaleQuantumToLong(GetPixelRed(p));
            *q++=0;
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"I") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToLong(ClampToQuantum(GetPixelIntensity(image,p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGB") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToLong(GetPixelRed(p));
            *q++=ScaleQuantumToLong(GetPixelGreen(p));
            *q++=ScaleQuantumToLong(GetPixelBlue(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToLong(GetPixelRed(p));
            *q++=ScaleQuantumToLong(GetPixelGreen(p));
            *q++=ScaleQuantumToLong(GetPixelBlue(p));
            *q++=ScaleQuantumToLong((Quantum) (GetPixelAlpha(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToLong(GetPixelRed(p));
            *q++=ScaleQuantumToLong(GetPixelGreen(p));
            *q++=ScaleQuantumToLong(GetPixelBlue(p));
            *q++=0;
            p++;
          }
          break;
        }
      p=GetAuthenticPixelQueue(image);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) GetImageExtent(image); x++)
      {
        for (i=0; i < (ssize_t) length; i++)
        {
          *q=0;
          switch (quantum_map[i])
          {
            case RedQuantum:
            case CyanQuantum:
            {
              *q=ScaleQuantumToLong(GetPixelRed(p));
              break;
            }
            case GreenQuantum:
            case MagentaQuantum:
            {
              *q=ScaleQuantumToLong(GetPixelGreen(p));
              break;
            }
            case BlueQuantum:
            case YellowQuantum:
            {
              *q=ScaleQuantumToLong(GetPixelBlue(p));
              break;
            }
            case AlphaQuantum:
            {
              *q=ScaleQuantumToLong((Quantum) (GetPixelAlpha(p)));
              break;
            }
            case OpacityQuantum:
            {
              *q=ScaleQuantumToLong(GetPixelOpacity(p));
              break;
            }
            case BlackQuantum:
            {
              if (image->colorspace == CMYKColorspace)
                *q=ScaleQuantumToLong(GetPixelIndex(indexes+x));
              break;
            }
            case IndexQuantum:
            {
              *q=ScaleQuantumToLong(ClampToQuantum(GetPixelIntensity(image,p)));
              break;
            }
            default:
              break;
          }
          q++;
        }
        p++;
      }
      break;
    }
    case QuantumPixel:
    {
      register Quantum
        *q;

      q=(Quantum *) stream_info->pixels;
      if (LocaleCompare(stream_info->map,"BGR") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=GetPixelBlue(p);
            *q++=GetPixelGreen(p);
            *q++=GetPixelRed(p);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=GetPixelBlue(p);
            *q++=GetPixelGreen(p);
            *q++=GetPixelRed(p);
            *q++=(Quantum) (GetPixelAlpha(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=GetPixelBlue(p);
            *q++=GetPixelGreen(p);
            *q++=GetPixelRed(p);
            *q++=0;
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"I") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ClampToQuantum(GetPixelIntensity(image,p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGB") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=GetPixelRed(p);
            *q++=GetPixelGreen(p);
            *q++=GetPixelBlue(p);
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=GetPixelRed(p);
            *q++=GetPixelGreen(p);
            *q++=GetPixelBlue(p);
            *q++=(Quantum) (GetPixelAlpha(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=GetPixelRed(p);
            *q++=GetPixelGreen(p);
            *q++=GetPixelBlue(p);
            *q++=0U;
            p++;
          }
          break;
        }
      p=GetAuthenticPixelQueue(image);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) GetImageExtent(image); x++)
      {
        for (i=0; i < (ssize_t) length; i++)
        {
          *q=(Quantum) 0;
          switch (quantum_map[i])
          {
            case RedQuantum:
            case CyanQuantum:
            {
              *q=GetPixelRed(p);
              break;
            }
            case GreenQuantum:
            case MagentaQuantum:
            {
              *q=GetPixelGreen(p);
              break;
            }
            case BlueQuantum:
            case YellowQuantum:
            {
              *q=GetPixelBlue(p);
              break;
            }
            case AlphaQuantum:
            {
              *q=(Quantum) (GetPixelAlpha(p));
              break;
            }
            case OpacityQuantum:
            {
              *q=GetPixelOpacity(p);
              break;
            }
            case BlackQuantum:
            {
              if (image->colorspace == CMYKColorspace)
                *q=GetPixelIndex(indexes+x);
              break;
            }
            case IndexQuantum:
            {
              *q=ClampToQuantum(GetPixelIntensity(image,p));
              break;
            }
            default:
              *q=0;
          }
          q++;
        }
        p++;
      }
      break;
    }
    case ShortPixel:
    {
      register unsigned short
        *q;

      q=(unsigned short *) stream_info->pixels;
      if (LocaleCompare(stream_info->map,"BGR") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToShort(GetPixelBlue(p));
            *q++=ScaleQuantumToShort(GetPixelGreen(p));
            *q++=ScaleQuantumToShort(GetPixelRed(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToShort(GetPixelBlue(p));
            *q++=ScaleQuantumToShort(GetPixelGreen(p));
            *q++=ScaleQuantumToShort(GetPixelRed(p));
            *q++=ScaleQuantumToShort((Quantum) (GetPixelAlpha(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"BGRP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
            if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToShort(GetPixelBlue(p));
            *q++=ScaleQuantumToShort(GetPixelGreen(p));
            *q++=ScaleQuantumToShort(GetPixelRed(p));
            *q++=0;
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"I") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToShort(ClampToQuantum(GetPixelIntensity(image,p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGB") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToShort(GetPixelRed(p));
            *q++=ScaleQuantumToShort(GetPixelGreen(p));
            *q++=ScaleQuantumToShort(GetPixelBlue(p));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBA") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToShort(GetPixelRed(p));
            *q++=ScaleQuantumToShort(GetPixelGreen(p));
            *q++=ScaleQuantumToShort(GetPixelBlue(p));
            *q++=ScaleQuantumToShort((Quantum) (GetPixelAlpha(p)));
            p++;
          }
          break;
        }
      if (LocaleCompare(stream_info->map,"RGBP") == 0)
        {
          p=GetAuthenticPixelQueue(image);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) GetImageExtent(image); x++)
          {
            *q++=ScaleQuantumToShort(GetPixelRed(p));
            *q++=ScaleQuantumToShort(GetPixelGreen(p));
            *q++=ScaleQuantumToShort(GetPixelBlue(p));
            *q++=0;
            p++;
          }
          break;
        }
      p=GetAuthenticPixelQueue(image);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) GetImageExtent(image); x++)
      {
        for (i=0; i < (ssize_t) length; i++)
        {
          *q=0;
          switch (quantum_map[i])
          {
            case RedQuantum:
            case CyanQuantum:
            {
              *q=ScaleQuantumToShort(GetPixelRed(p));
              break;
            }
            case GreenQuantum:
            case MagentaQuantum:
            {
              *q=ScaleQuantumToShort(GetPixelGreen(p));
              break;
            }
            case BlueQuantum:
            case YellowQuantum:
            {
              *q=ScaleQuantumToShort(GetPixelBlue(p));
              break;
            }
            case AlphaQuantum:
            {
              *q=ScaleQuantumToShort((Quantum) (GetPixelAlpha(p)));
              break;
            }
            case OpacityQuantum:
            {
              *q=ScaleQuantumToShort(GetPixelOpacity(p));
              break;
            }
            case BlackQuantum:
            {
              if (image->colorspace == CMYKColorspace)
                *q=ScaleQuantumToShort(GetPixelIndex(indexes+x));
              break;
            }
            case IndexQuantum:
            {
              *q=ScaleQuantumToShort(ClampToQuantum(GetPixelIntensity(image,p)));
              break;
            }
            default:
              break;
          }
          q++;
        }
        p++;
      }
      break;
    }
    default:
    {
      quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "UnrecognizedPixelMap","`%s'",stream_info->map);
      break;
    }
  }
  quantum_map=(QuantumType *) RelinquishMagickMemory(quantum_map);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c A u t h e n t i c P i x e l s S t r e a m                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncAuthenticPixelsStream() calls the user supplied callback method with
%  the latest stream of pixels.
%
%  The format of the SyncAuthenticPixelsStream method is:
%
%      MagickBooleanType SyncAuthenticPixelsStream(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType SyncAuthenticPixelsStream(Image *image,
  ExceptionInfo *exception)
{
  CacheInfo
    *cache_info;

  size_t
    length;

  StreamHandler
    stream_handler;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_info=(CacheInfo *) image->cache;
  assert(cache_info->signature == MagickSignature);
  stream_handler=GetBlobStreamHandler(image);
  if (stream_handler == (StreamHandler) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),StreamError,
        "NoStreamHandlerIsDefined","`%s'",image->filename);
      return(MagickFalse);
    }
  length=stream_handler(image,cache_info->pixels,(size_t) cache_info->columns);
  return(length == cache_info->columns ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S t r e a m                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteStream() makes the image pixels available to a user supplied callback
%  method immediately upon writing pixel data with the WriteImage() method.
%
%  The format of the WriteStream() method is:
%
%      MagickBooleanType WriteStream(const ImageInfo *image_info,Image *,
%        StreamHandler stream)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o stream: A callback method.
%
*/
MagickExport MagickBooleanType WriteStream(const ImageInfo *image_info,
  Image *image,StreamHandler stream)
{
  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  write_info=CloneImageInfo(image_info);
  write_info->stream=stream;
  status=WriteImage(write_info,image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}
