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

  MagickCore private memory methods.
*/
#ifndef MAGICKCORE_MEMORY_PRIVATE_H
#define MAGICKCORE_MEMORY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


#if defined(__powerpc__)
#  define CACHE_LINE_SIZE  128
#else
#  define CACHE_LINE_SIZE  64
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
  HeapOverflowSanityCheck(const size_t,const size_t) magick_alloc_sizes(1,2);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
