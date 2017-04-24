/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore cache private methods.
*/
#ifndef MAGICKCORE_CACHE_PRIVATE_H
#define MAGICKCORE_CACHE_PRIVATE_H

#include "MagickCore/cache.h"
#include "MagickCore/distribute-cache.h"
#include "MagickCore/opencl-private.h"
#include "MagickCore/pixel.h"
#include "MagickCore/random_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/semaphore.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef void
  *Cache;

typedef MagickBooleanType
  (*GetOneAuthenticPixelFromHandler)(Image *,const ssize_t,const ssize_t,
    Quantum *,ExceptionInfo *),
  (*GetOneVirtualPixelFromHandler)(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,Quantum *,ExceptionInfo *),
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
{
  MagickBooleanType
    mapped;

  RectangleInfo
    region;

  MagickSizeType
    length;

  Quantum
    *cache,
    *pixels;

  MagickBooleanType
    authentic_pixel_cache;

  void
    *metacontent;

  size_t
    signature;
} NexusInfo;

typedef struct _CacheInfo
{
  ClassType
    storage_class;

  ColorspaceType
    colorspace;

  PixelTrait
    alpha_trait;

  MagickBooleanType
    read_mask,
    write_mask;

  size_t
    columns,
    rows;

  size_t
    metacontent_extent,
    number_channels;

  PixelChannelMap
    channel_map[MaxPixelChannels];

  CacheType
    type;

  MapMode
    mode;

  MagickBooleanType
    mapped;

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
    filename[MagickPathExtent],
    cache_filename[MagickPathExtent];

  CacheMethods
    methods;

  RandomInfo
    *random_info;

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

  MagickCLCacheInfo
    opencl;
} CacheInfo;

extern MagickPrivate Cache
  AcquirePixelCache(const size_t),
  ClonePixelCache(const Cache),
  DestroyPixelCache(Cache),
  ReferencePixelCache(Cache);

extern MagickPrivate ClassType
  GetPixelCacheStorageClass(const Cache);

extern MagickPrivate ColorspaceType
  GetPixelCacheColorspace(const Cache);

extern MagickPrivate const Quantum
  *GetVirtualPixelsFromNexus(const Image *,const VirtualPixelMethod,
    const ssize_t,const ssize_t,const size_t,const size_t,NexusInfo *,
    ExceptionInfo *) magick_hot_spot,
  *GetVirtualPixelsNexus(const Cache,NexusInfo *magick_restrict);

extern MagickPrivate const void
  *AcquirePixelCachePixels(const Image *,MagickSizeType *,ExceptionInfo *),
  *GetVirtualMetacontentFromNexus(const Cache,NexusInfo *magick_restrict);

extern MagickPrivate MagickBooleanType
  CacheComponentGenesis(void),
  SyncAuthenticPixelCacheNexus(Image *,NexusInfo *magick_restrict,
    ExceptionInfo *) magick_hot_spot,
  SyncImagePixelCache(Image *,ExceptionInfo *);

extern MagickPrivate MagickSizeType
  GetPixelCacheNexusExtent(const Cache,NexusInfo *magick_restrict);

extern MagickPrivate NexusInfo
  **AcquirePixelCacheNexus(const size_t),
  **DestroyPixelCacheNexus(NexusInfo **,const size_t);

extern MagickPrivate Quantum
  *GetAuthenticPixelCacheNexus(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,NexusInfo *,ExceptionInfo *) magick_hot_spot,
  *QueueAuthenticPixelCacheNexus(Image *,const ssize_t,const ssize_t,
    const size_t,const size_t,const MagickBooleanType,NexusInfo *,
    ExceptionInfo *) magick_hot_spot;

extern MagickPrivate size_t
  GetPixelCacheChannels(const Cache);

extern MagickPrivate VirtualPixelMethod
  GetPixelCacheVirtualMethod(const Image *),
  SetPixelCacheVirtualMethod(Image *,const VirtualPixelMethod,ExceptionInfo *);

extern MagickPrivate void
  CacheComponentTerminus(void),
  ClonePixelCacheMethods(Cache,const Cache),
  *GetPixelCachePixels(Image *,MagickSizeType *,ExceptionInfo *),
  GetPixelCacheTileSize(const Image *,size_t *,size_t *),
  GetPixelCacheMethods(CacheMethods *),
  ResetPixelCacheEpoch(void),
  ResetPixelCacheChannels(Image *),
  SetPixelCacheMethods(Cache,CacheMethods *);

#if defined(MAGICKCORE_OPENCL_SUPPORT)
extern MagickPrivate cl_mem
  GetAuthenticOpenCLBuffer(const Image *,MagickCLDevice,ExceptionInfo *);

extern MagickPrivate void
  SyncAuthenticOpenCLBuffer(const Image *);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
