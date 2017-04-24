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

  MagickCore distributed cache private methods.
*/
#ifndef MAGICKCORE_DISTRIBUTE_CACHE_PRIVATE_H
#define MAGICKCORE_DISTRIBUTE_CACHE_PRIVATE_H

#include "MagickCore/geometry.h"
#include "MagickCore/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _DistributeCacheInfo
{
  int
    file;

  size_t
    session_key;

  char
    hostname[MagickPathExtent];

  int
    port;

  MagickBooleanType
    debug;

  size_t
    signature;
} DistributeCacheInfo;

extern MagickPrivate const char
  *GetDistributeCacheHostname(const DistributeCacheInfo *);

extern MagickPrivate DistributeCacheInfo
  *AcquireDistributeCacheInfo(ExceptionInfo *),
  *DestroyDistributeCacheInfo(DistributeCacheInfo *);

extern MagickPrivate int
  GetDistributeCacheFile(const DistributeCacheInfo *),
  GetDistributeCachePort(const DistributeCacheInfo *);

extern MagickPrivate MagickBooleanType
  OpenDistributePixelCache(DistributeCacheInfo *,Image *),
  RelinquishDistributePixelCache(DistributeCacheInfo *);

extern MagickPrivate MagickOffsetType
  ReadDistributePixelCacheMetacontent(DistributeCacheInfo *,
    const RectangleInfo *,const MagickSizeType,unsigned char *),
  ReadDistributePixelCachePixels(DistributeCacheInfo *,const RectangleInfo *,
    const MagickSizeType,unsigned char *magick_restrict),
  WriteDistributePixelCacheMetacontent(DistributeCacheInfo *,
    const RectangleInfo *,const MagickSizeType,const unsigned char *),
  WriteDistributePixelCachePixels(DistributeCacheInfo *,const RectangleInfo *,
    const MagickSizeType,const unsigned char *magick_restrict);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
