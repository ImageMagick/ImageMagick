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
%                                John Cristy                                  %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
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
#include "magick/studio.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/memory_.h"
#include "magick/semaphore.h"
#include "magick/semaphore-private.h"
#include "magick/string_.h"
#include "magick/thread_.h"
#include "magick/thread-private.h"

/*
  Struct declaractions.
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
%   A c q u i r e S e m a p h o r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSemaphoreInfo() acquires a semaphore.
%
%  The format of the AcquireSemaphoreInfo method is:
%
%      void AcquireSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void AcquireSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
  if (*semaphore_info == (SemaphoreInfo *) NULL)
    {
      LockMagickMutex();
      if (*semaphore_info == (SemaphoreInfo *) NULL)
        *semaphore_info=AllocateSemaphoreInfo();
      UnlockMagickMutex();
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e S e m a p h o r e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AllocateSemaphoreInfo() initializes the SemaphoreInfo structure.
%
%  The format of the AllocateSemaphoreInfo method is:
%
%      SemaphoreInfo *AllocateSemaphoreInfo(void)
%
*/
MagickExport SemaphoreInfo *AllocateSemaphoreInfo(void)
{
  SemaphoreInfo
    *semaphore_info;

  /*
    Allocate semaphore.
  */
  semaphore_info=(SemaphoreInfo *) malloc(sizeof(SemaphoreInfo));
  if (semaphore_info == (SemaphoreInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(semaphore_info,0,sizeof(SemaphoreInfo));
  /*
    Initialize the semaphore.
  */
#if defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    pthread_mutexattr_t
      mutex_info;

    status=pthread_mutexattr_init(&mutex_info);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToInitializeSemaphore");
      }
#if defined(MAGICKCORE_DEBUG)
#if defined(PTHREAD_MUTEX_ERRORCHECK)
    status=pthread_mutex_settype(&mutex_info,PTHREAD_MUTEX_ERRORCHECK);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToInitializeSemaphore");
      }
#endif
#endif
    status=pthread_mutex_init(&semaphore_info->mutex,&mutex_info);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToInitializeSemaphore");
      }
    status=pthread_mutexattr_destroy(&mutex_info);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToInitializeSemaphore");
      }
  }
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  {
    int
      status;

    status=InitializeCriticalSectionAndSpinCount(&semaphore_info->mutex,0x0400);
    if (status == 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToInitializeSemaphore");
       }
  }
#endif
  semaphore_info->id=GetMagickThreadId();
  semaphore_info->reference_count=0;
  semaphore_info->signature=MagickSignature;
  return(semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S e m a p h o r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySemaphoreInfo() destroys a semaphore.
%
%  The format of the DestroySemaphoreInfo method is:
%
%      void DestroySemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void DestroySemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
  assert((*semaphore_info) != (SemaphoreInfo *) NULL);
  assert((*semaphore_info)->signature == MagickSignature);
  LockMagickMutex();
#if defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_destroy(&(*semaphore_info)->mutex);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,"UnableToDestroySemaphore");
      }
  }
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  DeleteCriticalSection(&(*semaphore_info)->mutex);
#endif
  (*semaphore_info)->signature=(~MagickSignature);
  free(*semaphore_info);
  *semaphore_info=(SemaphoreInfo *) NULL;
  UnlockMagickMutex();
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
  assert(semaphore_info->signature == MagickSignature);
#if defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_lock(&semaphore_info->mutex);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,"UnableToLockSemaphore");
      }
  }
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  EnterCriticalSection(&semaphore_info->mutex);
#endif
#if defined(MAGICKCORE_DEBUG)
  if ((semaphore_info->reference_count > 0) &&
      (IsMagickThreadEqual(semaphore_info->id) != MagickFalse))
    {
      (void) FormatLocaleFile(stderr,"Warning: unexpected recursive lock!\n");
      (void) fflush(stderr);
    }
#endif
  semaphore_info->id=GetMagickThreadId();
  semaphore_info->reference_count++;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n g u i s h S e m a p h o r e I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishSemaphoreInfo() relinquishes a semaphore.
%
%  The format of the RelinquishSemaphoreInfo method is:
%
%      RelinquishSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
MagickExport void RelinquishSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == MagickSignature);
  UnlockSemaphoreInfo(semaphore_info);
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
MagickExport MagickBooleanType SemaphoreComponentGenesis(void)
{
  LockMagickMutex();
  UnlockMagickMutex();
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
MagickExport void SemaphoreComponentTerminus(void)
{
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
  assert(semaphore_info->signature == MagickSignature);
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
#if defined(MAGICKCORE_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_unlock(&semaphore_info->mutex);
    if (status != 0)
      {
        errno=status;
        ThrowFatalException(ResourceLimitFatalError,"UnableToUnlockSemaphore");
      }
  }
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  LeaveCriticalSection(&semaphore_info->mutex);
#endif
}
