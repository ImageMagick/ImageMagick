/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private memory methods.
*/
#ifndef MAGICKCORE_MEMORY_PRIVATE_H
#define MAGICKCORE_MEMORY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/exception-private.h"

#if defined(__powerpc__)
#  define CACHE_LINE_SIZE  (16*sizeof(void *))
#else
#  define CACHE_LINE_SIZE  (8*sizeof(void *))
#endif

#define CacheAlign(size)  ((size) < CACHE_LINE_SIZE ? CACHE_LINE_SIZE : (size))

#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 6))
#if !defined(__ICC)
#define MagickAssumeAligned(address) \
  __builtin_assume_aligned((address),CACHE_LINE_SIZE)
#else
#define MagickAssumeAligned(address)  (address)
#endif
#else
#define MagickAssumeAligned(address)  (address)
#endif

MagickExport MagickBooleanType 
  HeapOverflowSanityCheck(const size_t,const size_t);

MagickExport size_t
  GetMaxMemoryRequest(void);

extern MagickPrivate void
  ResetMaxMemoryRequest(void),
  ResetVirtualAnonymousMemory(void);

static inline void *AcquireCriticalMemory(const size_t size)
{
  register void
    *memory;
 
  /*
    Fail if memory request cannot be fulfilled.
  */
  memory=AcquireMagickMemory(size);
  if (memory == (void *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  return(memory);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
