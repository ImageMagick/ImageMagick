/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _MAGICKCORE_CACHE_PRIVATE_H
#define _MAGICKCORE_CACHE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <time.h>
#include "MagickCore/random_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/semaphore.h"

typedef enum
{
  UndefinedCache,
  MemoryCache,
  MapCache,
  DiskCache,
  PingCache
} CacheType;

typedef void
  *Cache;

typedef MagickBooleanType
  (*GetOneAuthenticPixelFromHandler)(Image *,const ssize_t,const ssize_t,
    PixelPacket *,ExceptionInfo *),
  (*GetOneVirtualPixelFromHandler)(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,PixelPacket *,ExceptionInfo *),
  (*SyncAuthenticPixelsHandler)(Image *,ExceptionInfo *);

typedef const Quantum
  *(*GetVirtualPixelHandler)(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,const size_t,const size_t,ExceptionInfo *),
  *(*GetVirtualPixelsHandler)(const Image *);

typedef const void
  *(*GetVirtualMetacontentFromHandler)(const Image *);

typedef Quantum
  *(*GetAuthenticPixelsHandler)(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,ExceptionInfo *);

typedef Quantum
  *(*GetAuthenticPixelsFromHandler)(const Image *);

typedef Quantum
  *(*QueueAuthenticPixelsHandler)(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,ExceptionInfo *);

typedef void
  (*DestroyPixelHandler)(Image *);

typedef void
  *(*GetAuthenticMetacontentFromHandler)(const Image *);

typedef struct _CacheMethods
{
  GetVirtualPixelHandler
    get_virtual_pixel_handler;

  GetVirtualPixelsHandler
    get_virtual_pixels_handler;

  GetVirtualMetacontentFromHandler
    get_virtual_metacontent_from_handler;

  GetOneVirtualPixelFromHandler
    get_one_virtual_pixel_from_handler;

  GetAuthenticPixelsHandler
    get_authentic_pixels_handler;

  GetAuthenticMetacontentFromHandler
    get_authentic_metacontent_from_handler;

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
   NexusInfo;

typedef struct _CacheInfo
{
  ClassType
    storage_class;

  ColorspaceType
    colorspace;

  size_t
    metacontent_extent,
    pixel_components;

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

  PixelInfo
    virtual_pixel_color;

  size_t
    number_threads;

  NexusInfo
    **nexus_info;

  Quantum
    *pixels;

  void
    *metacontent;

  int
    file;

  char
    filename[MaxTextExtent],
    cache_filename[MaxTextExtent];

  CacheMethods
    methods;

  RandomInfo
    *random_info;

  MagickBooleanType
    debug;

  MagickThreadType
    id;

  ssize_t
    reference_count;

  SemaphoreInfo
    *semaphore,
    *disk_semaphore;

  time_t
    timestamp;

  size_t
    signature;
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


extern MagickExport const Quantum
  *GetVirtualPixelsFromNexus(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,const size_t,const size_t,NexusInfo *,
    ExceptionInfo *),
  *GetVirtualPixelsNexus(const Cache,NexusInfo *);

extern MagickExport const void
  *GetVirtualMetacontentFromNexus(const Cache,NexusInfo *);

extern MagickExport MagickBooleanType
  SyncAuthenticPixelCacheNexus(Image *,NexusInfo *,ExceptionInfo *);

extern MagickExport MagickSizeType
  GetPixelCacheNexusExtent(const Cache,NexusInfo *);

extern MagickExport NexusInfo
  **AcquirePixelCacheNexus(const size_t),
  **DestroyPixelCacheNexus(NexusInfo **,const size_t);

extern MagickExport Quantum
  *GetAuthenticPixelCacheNexus(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,NexusInfo *,ExceptionInfo *),
  *GetPixelCacheNexusPixels(const Cache,NexusInfo *),
  *QueueAuthenticNexus(Image *,const ssize_t,const ssize_t,const size_t,
    const size_t,NexusInfo *,ExceptionInfo *);

extern MagickExport size_t
  GetPixelCacheChannels(const Cache);

extern MagickExport void
  ClonePixelCacheMethods(Cache,const Cache),
  GetPixelCacheTileSize(const Image *,size_t *,size_t *),
  GetPixelCacheMethods(CacheMethods *),
  SetPixelCacheMethods(Cache,CacheMethods *);

extern MagickExport void
  *GetPixelCacheNexusMetacontent(const Cache,NexusInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
