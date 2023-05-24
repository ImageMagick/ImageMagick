/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore methods to synchronize code within a translation unit.
*/
#ifndef MAGICKCORE_MUTEX_H
#define MAGICKCORE_MUTEX_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  When included in a translation unit, the following code provides the
  translation unit a means by which to synchronize multiple threads that might
  try to enter the same critical section or to access a shared resource; it can
  be included in multiple translation units, and thereby provide a separate,
  independent means of synchronization to each such translation unit.
*/

#if defined(MAGICKCORE_OPENMP_SUPPORT)
static MagickBooleanType
  translation_unit_initialized = MagickFalse;

static omp_lock_t
  translation_unit_mutex;
#elif defined(MAGICKCORE_THREAD_SUPPORT)
static pthread_mutex_t
  translation_unit_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
static LONG
  translation_unit_mutex = 0;
#endif

static inline void DestroyMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  if (translation_unit_initialized != MagickFalse)
    omp_destroy_lock(&translation_unit_mutex);
  translation_unit_initialized=MagickFalse;
#endif
}

static inline void InitializeMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  if (translation_unit_initialized == MagickFalse)
    omp_init_lock(&translation_unit_mutex);
  translation_unit_initialized=MagickTrue;
#endif
}

static inline void LockMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  if (translation_unit_initialized == MagickFalse)
    InitializeMagickMutex();
  omp_set_lock(&translation_unit_mutex);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_lock(&translation_unit_mutex);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,"UnableToLockSemaphore");
      }
  }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  while (InterlockedCompareExchange(&translation_unit_mutex,1L,0L) != 0)
    Sleep(10);
#endif
}

static inline void UnlockMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  if (translation_unit_initialized == MagickFalse)
    InitializeMagickMutex();
  omp_unset_lock(&translation_unit_mutex);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_unlock(&translation_unit_mutex);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,"UnableToUnlockSemaphore");
      }
  }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  InterlockedExchange(&translation_unit_mutex,0L);
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
