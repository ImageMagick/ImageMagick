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

#include "MagickCore/magick-config.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__powerpc__)
#  define CACHE_LINE_SIZE  (16 * MAGICKCORE_SIZEOF_VOID_P)
#else
#  define CACHE_LINE_SIZE  (8 * MAGICKCORE_SIZEOF_VOID_P)
#endif

#define CACHE_ALIGNED(n)  MAGICKCORE_ALIGN_UP(n,CACHE_LINE_SIZE)

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
