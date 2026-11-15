/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        SSSSS  EEEEE  M   M   AAA   PPPP   H   H   OOO   RRRR   EEEEE        %
%        SS     E      MM MM  A   A  P   P  H   H  O   O  R   R  E            %
%         SSS   EEE    M M M  AAAAA  PPPP   HHHHH  O   O  RRRR   EEE          %
%           SS  E      M   M  A   A  P      H   H  O   O  R R    E            %
%        SSSSS  EEEEE  M   M  A   A  P      H   H   OOO   R  R   EEEEE        %
%                                                                             %
%                                                                             %
%                        MagickCore Semaphore Methods                         %
%                                                                             %
%                              Software Design                                %
%                             William Radcliffe                               %
%                                   Cristy                                    %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/mutex.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/semaphore-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/utility-private.h"
#if defined(MAGICKCORE_HAVE_STDATOMIC_H)
#include <stdatomic.h>
#endif

/*
  Define declarations.
*/
#if !defined(CACHE_LINE_SIZE)
#define CACHE_LINE_SIZE  64
#endif

/*
  Struct declarations.
*/
struct SemaphoreInfo
{
  MagickMutexType
    mutex;

  MagickThreadType
    id;

  ssize_t
    reference_count;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c t i v a t e S e m a p h o r e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ActivateSemaphoreInfo() activates a semaphore under protection of a mutex
%  to ensure only one thread allocates the semaphore.
%
%  The format of the ActivateSemaphoreInfo method is:
%
%      void ActivateSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void ActivateSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
#if defined(MAGICKCORE_HAVE_STDATOMIC_H)
  if (atomic_load_explicit((_Atomic(SemaphoreInfo *) *) semaphore_info,memory_order_acquire) == (SemaphoreInfo *) NULL)
    {
      LockMagickMutex();
      if (atomic_load_explicit((_Atomic(SemaphoreInfo *) *) semaphore_info,memory_order_relaxed) == (SemaphoreInfo *) NULL)
        {
          SemaphoreInfo
            *allocated_semaphore;

          allocated_semaphore=AcquireSemaphoreInfo();
          atomic_store_explicit((_Atomic(SemaphoreInfo *) *) semaphore_info,
            allocated_semaphore,memory_order_release);
        }
      UnlockMagickMutex();
    }
#else
  if (*semaphore_info == (SemaphoreInfo *) NULL)
    {
      LockMagickMutex();
      if (*semaphore_info == (SemaphoreInfo *) NULL)
        {
          SemaphoreInfo
            *allocated_semaphore;

          allocated_semaphore=AcquireSemaphoreInfo();
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
          MemoryBarrier();
#elif defined(__GNUC__) || defined(__clang__)
          __atomic_thread_fence(__ATOMIC_RELEASE);
#endif
          *semaphore_info=allocated_semaphore;
        }
      UnlockMagickMutex();
    }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  MemoryBarrier();
#elif defined(__GNUC__) || defined(__clang__)
  __atomic_thread_fence(__ATOMIC_ACQUIRE);
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S e m a p h o r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSemaphoreInfo() initializes the SemaphoreInfo structure.
%
%  The format of the AcquireSemaphoreInfo method is:
%
%      SemaphoreInfo *AcquireSemaphoreInfo(void)
%
*/

static inline size_t AlignSemaphoreExtent(const size_t size,
  const size_t alignment)
{
  if ((alignment == 0) || ((alignment & (alignment-1)) != 0))
    return(0);
  if (size > (SIZE_MAX-(alignment-1)))
    return(0);
  return((size+(alignment-1)) & ~(alignment-1));
}

static void *AcquireSemaphoreMemory(const size_t count,const size_t quantum)
{
  size_t
    alignment,
    extent,
    size;

  void
    *memory;

  if ((count == 0) || (quantum == 0) || (count > (SIZE_MAX/quantum)))
    {
      errno=ENOMEM;
      return((void *) NULL);
    }
  size=count*quantum;
  alignment=CACHE_LINE_SIZE;
  if (alignment < sizeof(void *))
    alignment=sizeof(void *);
  extent=AlignSemaphoreExtent(size,alignment);
  if (extent == 0)
    {
      errno=ENOMEM;
      return((void *) NULL);
    }
  memory=(void *) NULL;
#if defined(MAGICKCORE_HAVE_POSIX_MEMALIGN)
  if (posix_memalign(&memory,alignment,extent) != 0)
    memory=(void *) NULL;
#elif defined(MAGICKCORE_HAVE__ALIGNED_MALLOC)
  memory=_aligned_malloc(extent,alignment);
#else
  {
    void
      *raw_memory;

    if (size > (SIZE_MAX-(alignment-1)-sizeof(void *)))
      {
        errno=ENOMEM;
        return((void *) NULL);
      }
    extent=size+(alignment-1)+sizeof(void *);
    raw_memory=malloc(extent);
    if (raw_memory != (void *) NULL)
      {
        uintptr_t
          aligned_addr;

        aligned_addr=(uintptr_t) raw_memory+sizeof(void *);
        aligned_addr=(aligned_addr+(alignment-1)) & ~(uintptr_t) (alignment-1);
        memory=(void *) aligned_addr;
        *((void **) memory-1)=raw_memory;
      }
  }
#endif
  return(memory);
}

static void *RelinquishSemaphoreMemory(void *memory)
{
  if (memory == (void *) NULL)
    return((void *) NULL);
#if defined(MAGICKCORE_HAVE_POSIX_MEMALIGN)
  free(memory);
#elif defined(MAGICKCORE_HAVE__ALIGNED_MALLOC)
  _aligned_free(memory);
#else
  free(*((void **) memory-1));
#endif
  return((void *) NULL);
}

static MagickBooleanType InitializeSemaphoreMutex(SemaphoreInfo *semaphore_info)
{
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  omp_init_lock((omp_lock_t *) &semaphore_info->mutex);
  return(MagickTrue);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    pthread_mutexattr_t
      mutex_info;

    status=pthread_mutexattr_init(&mutex_info);
    if (status != 0)
      {
        errno=status;
        return(MagickFalse);
      }
#if defined(MAGICKCORE_DEBUG) && defined(PTHREAD_MUTEX_ERRORCHECK)
    status=pthread_mutexattr_settype(&mutex_info,PTHREAD_MUTEX_ERRORCHECK);
    if (status != 0)
      {
        errno=status;
        (void) pthread_mutexattr_destroy(&mutex_info);
        return(MagickFalse);
      }
#endif
    status=pthread_mutex_init(&semaphore_info->mutex,&mutex_info);
    (void) pthread_mutexattr_destroy(&mutex_info);
    if (status != 0)
      {
        errno=status;
        return(MagickFalse);
      }
  }
  return(MagickTrue);
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  if (InitializeCriticalSectionAndSpinCount(&semaphore_info->mutex,0x0400) == 0)
    {
      errno=ENOMEM;
      return(MagickFalse);
    }
  return(MagickTrue);
#else
  magick_unreferenced(semaphore_info);
  return(MagickTrue);
#endif
}

MagickExport SemaphoreInfo *AcquireSemaphoreInfo(void)
{
  SemaphoreInfo
    *semaphore_info;

  /*
    Acquire semaphore.
  */
  semaphore_info=(SemaphoreInfo *) AcquireSemaphoreMemory(1,
    sizeof(*semaphore_info));
  if (semaphore_info == (SemaphoreInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(semaphore_info,0,sizeof(*semaphore_info));
  /*
    Initialize the semaphore.
  */
  if (InitializeSemaphoreMutex(semaphore_info) == MagickFalse)
    {
      semaphore_info=(SemaphoreInfo *) RelinquishSemaphoreMemory(
        semaphore_info);
      ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
    }
  semaphore_info->id=GetMagickThreadId();
  semaphore_info->reference_count=0;
  semaphore_info->signature=MagickCoreSignature;
  return(semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c k S e m a p h o r e I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LockSemaphoreInfo() locks a semaphore.
%
%  The format of the LockSemaphoreInfo method is:
%
%      void LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == MagickCoreSignature);
#if defined(MAGICKCORE_DEBUG)
  if ((semaphore_info->reference_count > 0) &&
      (IsMagickThreadEqual(semaphore_info->id) != MagickFalse))
    {
      (void) FormatLocaleFile(stderr,"Warning: unexpected recursive lock!\n");
      (void) fflush(stderr);
    }
#endif
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  omp_set_lock((omp_lock_t *) &semaphore_info->mutex);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_lock(&semaphore_info->mutex);
    if (status != 0)
      {
        errno=status;
        perror("unable to lock mutex");
        _exit(1);
      }
  }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  EnterCriticalSection(&semaphore_info->mutex);
#endif
#if defined(MAGICKCORE_DEBUG)
  semaphore_info->id=GetMagickThreadId();
  semaphore_info->reference_count++;
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h S e m a p h o r e I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishSemaphoreInfo() destroys a semaphore.
%
%  The format of the RelinquishSemaphoreInfo method is:
%
%      void RelinquishSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void RelinquishSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  SemaphoreInfo
    *target_semaphore;

  assert(semaphore_info != (SemaphoreInfo **) NULL);
  LockMagickMutex();
#if defined(MAGICKCORE_HAVE_STDATOMIC_H)
  target_semaphore=atomic_exchange_explicit(
    (_Atomic(SemaphoreInfo *) *) semaphore_info,(SemaphoreInfo *) NULL,
    memory_order_acq_rel);
#else
  target_semaphore=(*semaphore_info);
  *semaphore_info=(SemaphoreInfo *) NULL;
#endif
  if (target_semaphore != (SemaphoreInfo *) NULL)
    {
      assert(target_semaphore->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      omp_destroy_lock((omp_lock_t *) &target_semaphore->mutex);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
      {
        int
          status;

        status=pthread_mutex_destroy(&target_semaphore->mutex);
        if (status != 0)
          {
            errno=status;
            perror("unable to destroy mutex");
            _exit(1);
          }
      }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
      DeleteCriticalSection(&target_semaphore->mutex);
#endif
      target_semaphore->signature=(~MagickCoreSignature);
      (void) RelinquishSemaphoreMemory(target_semaphore);
    }
  UnlockMagickMutex();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e m a p h o r e C o m p o n e n t G e n e s i s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SemaphoreComponentGenesis() instantiates the semaphore environment.
%
%  The format of the SemaphoreComponentGenesis method is:
%
%      MagickBooleanType SemaphoreComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType SemaphoreComponentGenesis(void)
{
  InitializeMagickMutex();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e m a p h o r e C o m p o n e n t T e r m i n u s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SemaphoreComponentTerminus() destroys the semaphore component.
%
%  The format of the SemaphoreComponentTerminus method is:
%
%      SemaphoreComponentTerminus(void)
%
*/
MagickPrivate void SemaphoreComponentTerminus(void)
{
  DestroyMagickMutex();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n l o c k S e m a p h o r e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnlockSemaphoreInfo() unlocks a semaphore.
%
%  The format of the UnlockSemaphoreInfo method is:
%
%      void UnlockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void UnlockSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == MagickCoreSignature);
#if defined(MAGICKCORE_DEBUG)
  assert(IsMagickThreadEqual(semaphore_info->id) != MagickFalse);
  if (semaphore_info->reference_count == 0)
    {
      (void) FormatLocaleFile(stderr,
        "Warning: semaphore lock already unlocked!\n");
      (void) fflush(stderr);
      return;
    }
  semaphore_info->reference_count--;
#endif
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  omp_unset_lock((omp_lock_t *) &semaphore_info->mutex);
#elif defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_unlock(&semaphore_info->mutex);
    if (status != 0)
      {
        errno=status;
        perror("unable to unlock mutex");
        _exit(1);
      }
  }
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  LeaveCriticalSection(&semaphore_info->mutex);
#endif
}
