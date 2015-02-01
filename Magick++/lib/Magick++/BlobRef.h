// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
// Copyright Dirk Lemstra 2014-2015
//
// Blob reference class
//
// This is an internal implementation class that should not be
// accessed by users.
//

#if !defined(Magick_Blob_header)
#define Magick_Blob_header

#include "Magick++/Include.h"
#include "Magick++/Thread.h"
#include "Magick++/Blob.h"

namespace Magick
{
  class BlobRef
  {
  public:

    // Construct with data, making private copy of data
    BlobRef(const void* data_,const size_t length_);

    // Destructor (actually destroys data)
    ~BlobRef(void);

    // Decreases reference count and return the new count
    size_t decrease();

    // Increases reference count
    void increase();

    Blob::Allocator allocator; // Memory allocation system in use
    size_t          length;    // Blob length
    void*           data;      // Blob data

  private:
    // Copy constructor and assignment are not supported
    BlobRef(const BlobRef&);
    BlobRef& operator=(const BlobRef&);

    MutexLock _mutexLock; // Mutex lock
    size_t    _refCount;  // Reference count
  };

} // namespace Magick

#endif // Magick_Blob_header
