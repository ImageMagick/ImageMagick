// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2003
// Copyright Dirk Lemstra 2014
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

namespace Magick
{
  // Mutex lock wrapper
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

  private:

    // Don't support copy constructor
    MutexLock(const MutexLock& original_);
    
    // Don't support assignment
    MutexLock& operator=(const MutexLock& original );

#if defined(MAGICKCORE_HAVE_PTHREAD)
    pthread_mutex_t _mutex;
#endif
#if defined(_MT) && defined(_VISUALC_)
    HANDLE _mutex;
#endif
  };
}

#endif // Magick_Thread_header
