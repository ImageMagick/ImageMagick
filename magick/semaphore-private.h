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

  MagickCore private methods to lock and unlock semaphores.
*/
#ifndef _MAGICKCORE_SEMAPHORE_PRIVATE_H
#define _MAGICKCORE_SEMAPHORE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_THREAD_SUPPORT)
static pthread_mutex_t
  semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
static LONG
  semaphore_mutex = 0;
#else
static ssize_t
  semaphore_mutex = 0;
#endif

static inline void LockMagickMutex(void)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
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
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  while (InterlockedCompareExchange(&semaphore_mutex,1L,0L) != 0)
    Sleep(10);
#endif
}

static inline void UnlockMagickMutex(void)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
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
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  InterlockedExchange(&semaphore_mutex,0L);
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
