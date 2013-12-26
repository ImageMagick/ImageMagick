/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore distributed cache private methods.
*/
#ifndef _MAGICKCORE_DISTRIBUTE_CACHE_PRIVATE_H
#define _MAGICKCORE_DISTRIBUTE_CACHE_PRIVATE_H

#include "magick/geometry.h"
#include "magick/exception.h"

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
    hostname[MaxTextExtent];

  int
    port;

  size_t
    signature;

  MagickBooleanType
    debug;
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
  ReadDistributePixelCacheIndexes(DistributeCacheInfo *,
    const RectangleInfo *,const MagickSizeType,unsigned char *),
  ReadDistributePixelCachePixels(DistributeCacheInfo *,const RectangleInfo *,
    const MagickSizeType,unsigned char *),
  WriteDistributePixelCacheIndexes(DistributeCacheInfo *,
    const RectangleInfo *,const MagickSizeType,const unsigned char *),
  WriteDistributePixelCachePixels(DistributeCacheInfo *,const RectangleInfo *,
    const MagickSizeType,const unsigned char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
