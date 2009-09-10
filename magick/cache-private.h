/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
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
#include "magick/random_.h"
#include "magick/thread-private.h"
#include "magick/semaphore.h"

typedef enum
{
  UndefinedCache,
  MemoryCache,
  MapCache,
  DiskCache
} CacheType;

typedef void
  *Cache;

typedef const IndexPacket
  *(*GetVirtualIndexesFromHandler)(const Image *);

typedef IndexPacket
  *(*GetAuthenticIndexesFromHandler)(const Image *);

typedef MagickBooleanType
  (*GetOneAuthenticPixelFromHandler)(Image *,const long,const long,
    PixelPacket *,ExceptionInfo *),
  (*GetOneVirtualPixelFromHandler)(const Image *,const VirtualPixelMethod,
    const long,const long,PixelPacket *,ExceptionInfo *),
  (*SyncAuthenticPixelsHandler)(Image *,ExceptionInfo *);

typedef const PixelPacket
  *(*GetVirtualPixelHandler)(const Image *,const VirtualPixelMethod,const long,
    const long,const unsigned long,const unsigned long,ExceptionInfo *),
  *(*GetVirtualPixelsHandler)(const Image *);

typedef PixelPacket
  *(*GetAuthenticPixelsHandler)(Image *,const long,const long,
    const unsigned long,const unsigned long,ExceptionInfo *);

typedef PixelPacket
  *(*GetAuthenticPixelsFromHandler)(const Image *);

typedef PixelPacket
  *(*QueueAuthenticPixelsHandler)(Image *,const long,const long,
    const unsigned long,const unsigned long,ExceptionInfo *);

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
   NexusInfo;

typedef struct _CacheInfo
{
  ClassType
    storage_class;

  ColorspaceType
    colorspace;

  CacheType
    type;

  MapMode
    mode;

  MagickBooleanType
    mapped;

  unsigned long
    columns,
    rows;

  MagickOffsetType
    offset;

  MagickSizeType
    length;

  VirtualPixelMethod
    virtual_pixel_method;

  unsigned long
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

  MagickBooleanType
    debug;

  MagickThreadType
    id;

  long
    reference_count;

  SemaphoreInfo
    *semaphore,
    *disk_semaphore;

  time_t
    timestamp;

  unsigned long
    signature;
} CacheInfo;

extern MagickExport Cache
  AcquirePixelCache(const unsigned long),
  ClonePixelCache(const Cache),
  DestroyPixelCache(Cache),
  GetImagePixelCache(Image *,const MagickBooleanType,ExceptionInfo *),
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
  *GetVirtualPixelsFromNexus(const Image *,const VirtualPixelMethod,const long,
    const long,const unsigned long,const unsigned long,NexusInfo *,
    ExceptionInfo *),
  *GetVirtualPixelsNexus(const Cache,NexusInfo *);

extern MagickExport IndexPacket
  *GetPixelCacheNexusIndexes(const Cache,NexusInfo *);

extern MagickExport MagickBooleanType
  SyncAuthenticPixelCacheNexus(Image *,NexusInfo *,ExceptionInfo *);

extern MagickExport MagickSizeType
  GetPixelCacheNexusExtent(const Cache,NexusInfo *);

extern MagickExport NexusInfo
  **AcquirePixelCacheNexus(const unsigned long),
  **DestroyPixelCacheNexus(NexusInfo **,const unsigned long);

extern MagickExport PixelPacket
  *GetAuthenticPixelCacheNexus(Image *,const long,const long,
    const unsigned long,const unsigned long,NexusInfo *,ExceptionInfo *),
  *GetPixelCacheNexusPixels(const Cache,NexusInfo *),
  *QueueAuthenticNexus(Image *,const long,const long,const unsigned long,
    const unsigned long,NexusInfo *,ExceptionInfo *);

extern MagickExport void
  ClonePixelCacheMethods(Cache,const Cache),
  GetPixelCacheTileSize(const Image *,unsigned long *,unsigned long *),
  GetPixelCacheMethods(CacheMethods *),
  SetPixelCacheMethods(Cache,CacheMethods *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
