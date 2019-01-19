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

  MagickCore private methods to lock and unlock semaphores.
*/
#ifndef MAGICKCORE_SEMAPHORE_PRIVATE_H
#define MAGICKCORE_SEMAPHORE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate MagickBooleanType
  SemaphoreComponentGenesis(void);

extern MagickPrivate void
  SemaphoreComponentTerminus(void);

#if defined(MAGICKCORE_OPENMP_SUPPORT)
static omp_lock_t
  semaphore_mutex;
#elif defined(MAGICKCORE_THREAD_SUPPORT)
static pthread_mutex_t
  semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
static LONG
  semaphore_mutex = 0;
#else
static ssize_t
  semaphore_mutex = 0;
#endif

static MagickBooleanType
  active_mutex = MagickFalse;

static inline void DestroyMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  if (active_mutex != MagickFalse)
    omp_destroy_lock(&semaphore_mutex);
#endif
  active_mutex=MagickFalse;
}

static inline void InitializeMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  if (active_mutex == MagickFalse)
    omp_init_lock(&semaphore_mutex);
#endif
  active_mutex=MagickTrue;
}

static inline void LockMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  omp_set_lock(&semaphore_mutex);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_lock(&semaphore_mutex);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,"UnableToLockSemaphore");
      }
  }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  while (InterlockedCompareExchange(&semaphore_mutex,1L,0L) != 0)
    Sleep(10);
#endif
}

static inline void UnlockMagickMutex(void)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  omp_unset_lock(&semaphore_mutex);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_unlock(&semaphore_mutex);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,"UnableToUnlockSemaphore");
      }
  }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  InterlockedExchange(&semaphore_mutex,0L);
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
