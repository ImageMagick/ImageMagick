// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2003
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Definition of types and classes to support threads
//
// This class is a Magick++ implementation class and is not intended
// for use by end-users.
//
#if !defined (Magick_Thread_header)
#define Magick_Thread_header

#include "Magick++/Include.h"

#if defined(_VISUALC_)
#include <windows.h>
#endif // defined(_VISUALC_)

#if defined(MAGICKCORE_HAVE_PTHREAD)
# include <pthread.h>
#endif // defined(MAGICKCORE_HAVE_PTHREAD)

#include <mutex>

namespace Magick
{
  // Mutex lock wrapper
  //
  // Provides lock()/unlock() so it satisfies the BasicLockable requirements
  // and can be used with std::lock_guard<Magick::MutexLock>.
  class MagickPPExport MutexLock
  {
  public:

    // Default constructor
    MutexLock(void);

    // Destructor
    ~MutexLock(void);

    // Lock mutex
    void lock(void);

    // Unlock mutex
    void unlock(void);

    // Don't support copy constructor
    MutexLock(const MutexLock& original_) = delete;

    // Don't support assignment
    MutexLock& operator=(const MutexLock& original_) = delete;

  private:

#if defined(MAGICKCORE_HAVE_PTHREAD)
    pthread_mutex_t _mutex;
#endif
#if defined(_MT) && defined(_VISUALC_)
    HANDLE _mutex;
#endif
  };
}

#endif // Magick_Thread_header
