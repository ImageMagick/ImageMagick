// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
// Copyright Dirk Lemstra 2014-2017
//
// Implementation of thread support
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Thread.h"
#include "Magick++/Exception.h"

#include <string.h>

// Default constructor
Magick::MutexLock::MutexLock(void)
#if defined(MAGICKCORE_HAVE_PTHREAD)
  // POSIX threads
  : _mutex()
{
  ::pthread_mutexattr_t
    attr;

  int
    sysError;

  if ((sysError=::pthread_mutexattr_init(&attr)) == 0)
    if ((sysError=::pthread_mutex_init(&_mutex,&attr)) == 0)
      {
        ::pthread_mutexattr_destroy(&attr);
        return;
      }
  throwExceptionExplicit(MagickCore::OptionError,"mutex initialization failed",
    strerror(sysError));
}
#else
#if defined(_VISUALC_) && defined(_MT)
// Win32 threads
{
  SECURITY_ATTRIBUTES
    security;

  /* Allow the semaphore to be inherited */
  security.nLength=sizeof(security);
  security.lpSecurityDescriptor=(LPVOID) NULL;
  security.bInheritHandle=TRUE;

  /* Create the semaphore, with initial value signaled */
  _mutex=::CreateSemaphore(&security,1,1,(LPCSTR) NULL);
  if (_mutex != (HANDLE) NULL)
    return;
  throwExceptionExplicit(MagickCore::OptionError,
    "mutex initialization failed");
}
#else
// Threads not supported
{
}
#endif
#endif

// Destructor
Magick::MutexLock::~MutexLock(void)
{
#if defined(MAGICKCORE_HAVE_PTHREAD)
  (void) ::pthread_mutex_destroy(&_mutex);
#endif
#if defined(_MT) && defined(_VISUALC_)
  (void) ::CloseHandle(_mutex);
#endif
}

// Lock mutex
void Magick::MutexLock::lock(void)
{
#if defined(MAGICKCORE_HAVE_PTHREAD)
  int
    sysError;

  if ((sysError=::pthread_mutex_lock(&_mutex)) == 0)
    return;
  throwExceptionExplicit(MagickCore::OptionError,"mutex lock failed",
    strerror(sysError));
#endif
#if defined(_MT) && defined(_VISUALC_)
  if (WaitForSingleObject(_mutex,INFINITE) != WAIT_FAILED)
    return;
  throwExceptionExplicit(MagickCore::OptionError,"mutex lock failed");
#endif
}

// Unlock mutex
void Magick::MutexLock::unlock(void)
{
#if defined(MAGICKCORE_HAVE_PTHREAD)
  int
    sysError;

  if ((sysError=::pthread_mutex_unlock(&_mutex)) == 0)
    return;
  throwExceptionExplicit(MagickCore::OptionError,"mutex unlock failed",
    strerror(sysError));
#endif
#if defined(_MT) && defined(_VISUALC_)
  if (ReleaseSemaphore(_mutex,1,(LPLONG) NULL) == TRUE)
    return;
  throwExceptionExplicit(MagickCore::OptionError,"mutex unlock failed");
#endif
}
