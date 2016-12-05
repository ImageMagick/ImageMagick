/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore cache private methods.
*/
#ifndef MAGICKCORE_CACHE_PRIVATE_H
#define MAGICKCORE_CACHE_PRIVATE_H

#include <time.h>
#include "magick/cache.h"
#include "magick/distribute-cache.h"
#include "magick/opencl-private.h"
#include "magick/random_.h"
#include "magick/thread-private.h"
#include "magick/semaphore.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef void
  *Cache;

typedef const IndexPacket
  *(*GetVirtualIndexesFromHandler)(const Image *);

typedef IndexPacket
  *(*GetAuthenticIndexesFromHandler)(const Image *);

typedef MagickBooleanType
  (*GetOneAuthenticPixelFromHandler)(Image *,const ssize_t,const ssize_t,
    PixelPacket *,ExceptionInfo *),
  (*GetOneVirtualPixelFromHandler)(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,PixelPacket *,ExceptionInfo *),
  (*SyncAuthenticPixelsHandler)(Image *,ExceptionInfo *);

typedef const PixelPacket
  *(*GetVirtualPixelHandler)(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,const size_t,const size_t,ExceptionInfo *),
  *(*GetVirtualPixelsHandler)(const Image *);

typedef PixelPacket
  *(*GetAuthenticPixelsHandler)(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,ExceptionInfo *);

typedef PixelPacket
  *(*GetAuthenticPixelsFromHandler)(const Image *);

typedef PixelPacket
  *(*QueueAuthenticPixelsHandler)(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,ExceptionInfo *);

typedef void
  (*DestroyPixelHandler)(Image *);

typedef struct _CacheMethods
{
  GetVirtualPixelHandler
    get_virtual_pixel_handler;

  GetVirtualPixelsHandler
    get_virtual_pixels_handler;

  GetVirtualIndexesFromHandler
    get_virtual_indexes_from_handler;

  GetOneVirtualPixelFromHandler
    get_one_virtual_pixel_from_handler;

  GetAuthenticPixelsHandler
    get_authentic_pixels_handler;

  GetAuthenticIndexesFromHandler
    get_authentic_indexes_from_handler;

  GetOneAuthenticPixelFromHandler
    get_one_authentic_pixel_from_handler;

  GetAuthenticPixelsFromHandler
    get_authentic_pixels_from_handler;

  QueueAuthenticPixelsHandler
    queue_authentic_pixels_handler;

  SyncAuthenticPixelsHandler
    sync_authentic_pixels_handler;

  DestroyPixelHandler
    destroy_pixel_handler;
} CacheMethods;

typedef struct _NexusInfo
{
  MagickBooleanType
    mapped;

  RectangleInfo
    region;

  MagickSizeType
    length;

  PixelPacket
    *cache,
    *pixels;

  MagickBooleanType
    authentic_pixel_cache;

  IndexPacket
    *indexes;

  size_t
    signature;
} NexusInfo;

typedef struct _OpenCLCacheInfo
{
  cl_event
    *events;

  cl_mem
    buffer;

  cl_uint
    event_count;

  MagickSizeType
    length;

  PixelPacket
    *pixels;
} OpenCLCacheInfo;

typedef struct _CacheInfo
{
  ClassType
    storage_class;

  ColorspaceType
    colorspace;

  size_t
    channels;

  CacheType
    type;

  MapMode
    mode;

  MagickBooleanType
    mapped;

  size_t
    columns,
    rows;

  MagickOffsetType
    offset;

  MagickSizeType
    length;

  VirtualPixelMethod
    virtual_pixel_method;

  MagickPixelPacket
    virtual_pixel_color;

  size_t
    number_threads;

  NexusInfo
    **nexus_info;

  PixelPacket
    *pixels;

  IndexPacket
    *indexes;

  MagickBooleanType
    active_index_channel;

  int
    file;

  char
    filename[MaxTextExtent],
    cache_filename[MaxTextExtent];

  CacheMethods
    methods;

  RandomInfo
    *random_info;

  size_t
    number_connections;

  void
    *server_info;

  MagickBooleanType
    synchronize,
    debug;

  MagickThreadType
    id;

  ssize_t
    reference_count;

  SemaphoreInfo
    *semaphore,
    *file_semaphore;

  time_t
    timestamp;

  size_t
    signature;

  OpenCLCacheInfo
    *opencl;
} CacheInfo;

extern MagickExport Cache
  AcquirePixelCache(const size_t),
  ClonePixelCache(const Cache),
  DestroyPixelCache(Cache),
  ReferencePixelCache(Cache);

extern MagickExport CacheType
  GetPixelCacheType(const Image *);

extern MagickExport ClassType
  GetPixelCacheStorageClass(const Cache);

extern MagickExport ColorspaceType
  GetPixelCacheColorspace(const Cache);

extern MagickExport const IndexPacket
  *GetVirtualIndexesFromNexus(const Cache,NexusInfo *);

extern MagickExport const PixelPacket
  *GetVirtualPixelsFromNexus(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,const size_t,const size_t,NexusInfo *,
    ExceptionInfo *) magick_hot_spot,
  *GetVirtualPixelsNexus(const Cache,NexusInfo *);

extern MagickExport MagickBooleanType
  SyncAuthenticPixelCacheNexus(Image *,NexusInfo *magick_restrict,
    ExceptionInfo *) magick_hot_spot;

extern MagickExport MagickSizeType
  GetPixelCacheNexusExtent(const Cache,NexusInfo *);

extern MagickExport NexusInfo
  **AcquirePixelCacheNexus(const size_t),
  **DestroyPixelCacheNexus(NexusInfo **,const size_t);

extern MagickExport PixelPacket
  *GetAuthenticPixelCacheNexus(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,NexusInfo *,ExceptionInfo *) magick_hot_spot,
  *QueueAuthenticPixel(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,const MagickBooleanType,NexusInfo *,ExceptionInfo *),
  *QueueAuthenticPixelCacheNexus(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,const MagickBooleanType,NexusInfo *,
    ExceptionInfo *) magick_hot_spot;

extern MagickExport size_t
  GetPixelCacheChannels(const Cache);

extern MagickExport void
  ClonePixelCacheMethods(Cache,const Cache),
  GetPixelCacheTileSize(const Image *,size_t *,size_t *),
  GetPixelCacheMethods(CacheMethods *),
  SetPixelCacheMethods(Cache,CacheMethods *);

extern MagickPrivate void
  ResetPixelCacheEpoch(void);

extern MagickPrivate MagickBooleanType
  SyncImagePixelCache(Image *,ExceptionInfo *);

#if defined(MAGICKCORE_OPENCL_SUPPORT)
extern MagickPrivate const cl_event
  *GetOpenCLEvents(const Image *,cl_uint *);

extern MagickPrivate cl_mem
  GetAuthenticOpenCLBuffer(const Image *,ExceptionInfo *);

extern MagickPrivate void
  AddOpenCLEvent(const Image *,cl_event),
  SyncAuthenticOpenCLBuffer(const Image *);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
