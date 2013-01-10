/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
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

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/exception.h"

#define MaxNumberDistributedCacheClients  32

typedef struct _DistributeCacheInfo
{
  size_t
    number_connections,
    session[MaxNumberDistributedCacheClients];

#if defined(MAGICKCORE_HAVE_SOCKET)
  int
    server[MaxNumberDistributedCacheClients];
#endif

  size_t
    signature;
} DistributeCacheInfo;

extern MagickPrivate DistributeCacheInfo
  *AcquireDistributeCacheInfo(ExceptionInfo *),
  *DestroyDistributeCacheInfo(DistributeCacheInfo *);

#endif
