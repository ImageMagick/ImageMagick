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

  MagickCore private methods for internal threading.
*/
#ifndef MAGICKCORE_THREAD_PRIVATE_H
#define MAGICKCORE_THREAD_PRIVATE_H

#include "MagickCore/cache.h"
#include "MagickCore/image-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/thread_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define magick_number_threads(source,destination,chunk,multithreaded) \
  num_threads(GetMagickNumberThreads(source,destination,chunk,multithreaded))
#if defined(__clang__) || (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ > 10))
#define MagickCachePrefetch(address,mode,locality) \
  __builtin_prefetch(address,mode,locality)
#else
#define MagickCachePrefetch(address,mode,locality) \
  magick_unreferenced(address); \
  magick_unreferenced(mode); \
  magick_unreferenced(locality);
#endif

#if defined(MAGICKCORE_THREAD_SUPPORT)
  typedef pthread_mutex_t MagickMutexType;
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  typedef CRITICAL_SECTION MagickMutexType;
#else
  typedef size_t MagickMutexType;
#endif

static inline int GetMagickNumberThreads(const Image *source,
  const Image *destination,const size_t chunk,int multithreaded)
{
  const CacheType
    destination_type = (const CacheType) GetImagePixelCacheType(destination),
    source_type = (const CacheType) GetImagePixelCacheType(source);

  int
    number_threads;

  /*
    Return number of threads dependent on cache type and work load.
  */
  if (multithreaded == 0)
    return(1);
  if (((source_type != MemoryCache) && (source_type != MapCache)) ||
      ((destination_type != MemoryCache) && (destination_type != MapCache)))
    number_threads=(int) MagickMin(GetMagickResourceLimit(ThreadResource),2);
  else
    number_threads=(int) MagickMin((ssize_t)
      GetMagickResourceLimit(ThreadResource),(ssize_t) (chunk)/64);
  return(MagickMax(number_threads,1));
}

static inline MagickThreadType GetMagickThreadId(void)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_self());
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  return(GetCurrentThreadId());
#else
  return(getpid());
#endif
}

static inline size_t GetMagickThreadSignature(void)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  {
    union
    {
      pthread_t
        id;

      size_t
        signature;
    } magick_thread;

    magick_thread.signature=0UL;
    magick_thread.id=pthread_self();
    return(magick_thread.signature);
  }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  return((size_t) GetCurrentThreadId());
#else
  return((size_t) getpid());
#endif
}

static inline MagickBooleanType IsMagickThreadEqual(const MagickThreadType id)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  if (pthread_equal(id,pthread_self()) != 0)
    return(MagickTrue);
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  if (id == GetCurrentThreadId())
    return(MagickTrue);
#else
  if (id == getpid())
    return(MagickTrue);
#endif
  return(MagickFalse);
}

/*
  Lightweight OpenMP methods.
*/
static inline size_t GetOpenMPMaximumThreads(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  return(omp_get_max_threads());
#else
  return(1);
#endif
}

static inline int GetOpenMPThreadId(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  return(omp_get_thread_num());
#else
  return(0);
#endif
}

static inline void SetOpenMPMaximumThreads(const int threads)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  omp_set_num_threads(threads);
#else
  magick_unreferenced(threads);
#endif
}

static inline void SetOpenMPNested(const int value)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  omp_set_nested(value);
#else
  magick_unreferenced(value);
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
