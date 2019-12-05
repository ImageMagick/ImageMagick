/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
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

#include "MagickCore/magick-config.h" // MAGICKCORE_SIZEOF_VOID_P, MAGICKCORE_IS_NOT_POWER_OF_2

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__powerpc__)
#  define CACHE_LINE_SIZE  (16 * MAGICKCORE_SIZEOF_VOID_P)
#else
#  define CACHE_LINE_SIZE  (8 * MAGICKCORE_SIZEOF_VOID_P)
#endif

#define IS_BAD_CACHE_LINE_SIZE \
  CACHE_LINE_SIZE <= 0 \
    || (CACHE_LINE_SIZE % MAGICKCORE_SIZEOF_VOID_P) != 0 \
      || MAGICKCORE_IS_NOT_POWER_OF_2(CACHE_LINE_SIZE/MAGICKCORE_SIZEOF_VOID_P)

#if IS_BAD_CACHE_LINE_SIZE
#  error "CACHE_LINE_SIZE must be greater than zero, and a multiple of `sizeof(void *)', and a power of 2."
#endif

#undef IS_BAD_CACHE_LINE_SIZE

#define CACHE_ALIGNED(n) MAGICKCORE_ALIGN_UP(n,CACHE_LINE_SIZE)

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

extern MagickPrivate void
  ResetMaxMemoryRequest(void),
  ResetVirtualAnonymousMemory(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
