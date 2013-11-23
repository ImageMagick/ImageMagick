// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2004
//
// Implementation of Blob
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include "Magick++/Thread.h"
#include "Magick++/BlobRef.h"

#include <string.h>

Magick::BlobRef::BlobRef(const void* data_,size_t length_)
  : data(0),
    length(length_),
    allocator(Magick::Blob::NewAllocator),
    refCount(1),
    mutexLock()
{
  if (data_)
    {
      data=new unsigned char[length_];
      memcpy(data,data_, length_);
    }
}

Magick::BlobRef::~BlobRef(void)
{
  if (allocator == Magick::Blob::NewAllocator)
    {
      delete[] static_cast<unsigned char*>(data);
      data=0;
    }
  else if (allocator == Magick::Blob::MallocAllocator)
    {
      data=(void *) RelinquishMagickMemory(data);
    }
}
