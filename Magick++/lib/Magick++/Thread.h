// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2003
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
#if defined(_MT)
struct win32_mutex {
	HANDLE id;
};

// This is a binary semphore -- increase for a counting semaphore
#define MAXSEMLEN	1
#endif // defined(_MT)
#endif // defined(_VISUALC_)

#if defined(MAGICKCORE_HAVE_PTHREAD)
# include <pthread.h>
#endif // defined(MAGICKCORE_HAVE_PTHREAD)

namespace Magick
{
  // Mutex lock wrapper
  class MagickDLLDecl MutexLock
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
    MutexLock ( const MutexLock& original_ );
    
    // Don't support assignment
    MutexLock& operator = ( const MutexLock& original_ );

#if defined(MAGICKCORE_HAVE_PTHREAD)
    pthread_mutex_t  _mutex;
#endif
#if defined(_MT) && defined(_VISUALC_)
    win32_mutex  _mutex;
#endif
  };

  // Lock mutex while object is in scope
  class MagickDLLDecl Lock
  {
  public:
    // Construct with mutex lock (locks mutex)
    Lock( MutexLock *mutexLock_ );

    // Destrutor (unlocks mutex)
    ~Lock( void );
  private:

    // Don't support copy constructor
    Lock ( const Lock& original_ );
    
    // Don't support assignment
    Lock& operator = ( const Lock& original_ );

    MutexLock* _mutexLock;
  };
}

// Construct with mutex lock (locks mutex)
inline Magick::Lock::Lock( MutexLock *mutexLock_ )
  : _mutexLock(mutexLock_)
{
  _mutexLock->lock();
}

// Destrutor (unlocks mutex)
inline Magick::Lock::~Lock( void )
{
  _mutexLock->unlock();
  _mutexLock=0;
}

#endif // Magick_Thread_header
