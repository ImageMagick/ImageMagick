// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2004
// Copyright Dirk Lemstra 2014-2015
//
// Implementation of Blob
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include "Magick++/BlobRef.h"
#include "Magick++/Exception.h"
#include "Magick++/Thread.h"

#include <string.h>

Magick::BlobRef::BlobRef(const void* data_,const size_t length_)
  : allocator(Magick::Blob::NewAllocator),
    length(length_),
    data((void*) NULL),
    _mutexLock(),
    _refCount(1)
{
  if (data_ != (const void*) NULL)
    {
      data=new unsigned char[length_];
      memcpy(data,data_,length_);
    }
}

Magick::BlobRef::~BlobRef(void)
{
  if (allocator == Magick::Blob::NewAllocator)
    {
      delete[] static_cast<unsigned char*>(data);
      data=(void *) NULL;
    }
  else if (allocator == Magick::Blob::MallocAllocator)
    data=(void *) RelinquishMagickMemory(data);
}

size_t Magick::BlobRef::decrease()
{
  size_t
    count;

  _mutexLock.lock();
  if (_refCount == 0)
    {
      _mutexLock.unlock();
      throwExceptionExplicit(MagickCore::OptionError,
        "Invalid call to decrease");
      return(0);
    }
  count=--_refCount;
  _mutexLock.unlock();
  return(count);
}

void Magick::BlobRef::increase()
{
  _mutexLock.lock();
  _refCount++;
  _mutexLock.unlock();
}
