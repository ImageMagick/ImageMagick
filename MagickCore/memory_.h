/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore memory methods.
*/
#ifndef MAGICKCORE_MEMORY_H
#define MAGICKCORE_MEMORY_H

#include <errno.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _MemoryInfo
  MemoryInfo;

typedef void
  *(*AcquireMemoryHandler)(size_t) magick_alloc_size(1),
  (*DestroyMemoryHandler)(void *),
  *(*ResizeMemoryHandler)(void *,size_t) magick_alloc_size(2),
  *(*AcquireAlignedMemoryHandler)(const size_t,const size_t),
  (*RelinquishAlignedMemoryHandler)(void *);

extern MagickExport MemoryInfo
  *AcquireVirtualMemory(const size_t,const size_t) magick_alloc_sizes(1,2),
  *RelinquishVirtualMemory(MemoryInfo *);

extern MagickExport size_t
  GetMaxMemoryRequest(void);

extern MagickExport void
  *AcquireAlignedMemory(const size_t,const size_t)
    magick_attribute((__malloc__)) magick_alloc_sizes(1,2),
  *AcquireMagickMemory(const size_t) magick_attribute((__malloc__))
    magick_alloc_size(1),
  *AcquireCriticalMemory(const size_t),
  *AcquireQuantumMemory(const size_t,const size_t)
    magick_attribute((__malloc__)) magick_alloc_sizes(1,2),
  *CopyMagickMemory(void *magick_restrict,const void *magick_restrict,
    const size_t) magick_attribute((__nonnull__)),
  DestroyMagickMemory(void),
  GetMagickMemoryMethods(AcquireMemoryHandler *,ResizeMemoryHandler *,
    DestroyMemoryHandler *),
  *GetVirtualMemoryBlob(const MemoryInfo *),
  *RelinquishAlignedMemory(void *),
  *RelinquishMagickMemory(void *),
  *ResetMagickMemory(void *,int,const size_t),
  *ResizeMagickMemory(void *,const size_t)
    magick_attribute((__malloc__)) magick_alloc_size(2),
  *ResizeQuantumMemory(void *,const size_t,const size_t)
    magick_attribute((__malloc__)) magick_alloc_sizes(2,3),
  SetMagickAlignedMemoryMethods(AcquireAlignedMemoryHandler,
    RelinquishAlignedMemoryHandler),
  SetMagickMemoryMethods(AcquireMemoryHandler,ResizeMemoryHandler,
    DestroyMemoryHandler);

static inline MagickBooleanType HeapOverflowSanityCheck(
  const size_t count,const size_t quantum)
{
  if ((count == 0) || (quantum == 0))
    return(MagickTrue);
  if (quantum != ((count*quantum)/count))
    {
      errno=ENOMEM;
      return(MagickTrue);
    }
  return(MagickFalse);
}

static inline MagickBooleanType HeapOverflowSanityCheckGetSize(
  const size_t count,const size_t quantum,size_t *const extent)
{
  size_t
    length;

  if ((count == 0) || (quantum == 0))
    return(MagickTrue);
  length=count*quantum;
  if (quantum != (length/count))
    {
      errno=ENOMEM;
      return(MagickTrue);
    }
  if (extent != NULL)
    *extent=length;
  return(MagickFalse);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
