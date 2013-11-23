// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2004
//
// Implementation of Blob
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include "Magick++/Blob.h"
#include "Magick++/BlobRef.h"

#include <string.h>

Magick::Blob::Blob(void)
  : _blobRef(new Magick::BlobRef(0,0))
{
}

Magick::Blob::Blob(const void* data_,size_t length_)
  : _blobRef(new Magick::BlobRef(data_,length_))
{
}

Magick::Blob::Blob(const Magick::Blob& blob_)
  : _blobRef(blob_._blobRef)
{
  // Increase reference count
  Lock(&_blobRef->_mutexLock);
  ++_blobRef->_refCount;
}

Magick::Blob::~Blob ()
{
  bool doDelete=false;
  {
    Lock(&_blobRef->_mutexLock);
    if (--_blobRef->_refCount == 0)
      doDelete=true;
  }

  if (doDelete)
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }
  _blobRef=0;
}

Magick::Blob& Magick::Blob::operator=(const Magick::Blob& blob_)
{
  bool
    doDelete;

  if (this != &blob_)
    {
      {
        Lock(&blob_._blobRef->_mutexLock);
        ++blob_._blobRef->_refCount;
      }
      doDelete=false;
      {
        Lock(&_blobRef->_mutexLock);
        if (--_blobRef->_refCount == 0)
          doDelete=true;
      }
      if (doDelete)
        {
          delete _blobRef;
        }
      _blobRef=blob_._blobRef;
    }
  return(*this);
}

void Magick::Blob::base64(const std::string base64_)
{
  size_t
    length;

  unsigned char
    *decoded;

  decoded=Base64Decode(base64_.c_str(),&length);
  if (decoded)
    updateNoCopy(static_cast<void*>(decoded),length,
      Magick::Blob::MallocAllocator);
}

std::string Magick::Blob::base64(void)
{
  char
    *encoded;

  size_t
    encoded_length;

  std::string
    result;

  encoded_length=0;
  encoded=Base64Encode(static_cast<const unsigned char*>(data()),length(),
    &encoded_length);

  if (encoded)
    {
      result=std::string(encoded,encoded_length);
      encoded=(char *) RelinquishMagickMemory(encoded);
      return result;
    }

  return(std::string());
}

const void* Magick::Blob::data(void) const
{
  return(_blobRef->_data);
}

size_t Magick::Blob::length(void) const
{
  return(_blobRef->_length);
}

void Magick::Blob::update(const void* data_,size_t length_)
{
  bool
    doDelete; 

  doDelete=false;
  {
    Lock( &_blobRef->_mutexLock );
    if (--_blobRef->_refCount == 0)
      doDelete=true;
  }
  if (doDelete)
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }

  _blobRef=new Magick::BlobRef(data_,length_);
}

void Magick::Blob::updateNoCopy(void* data_,size_t length_,
  Magick::Blob::Allocator allocator_)
{
  bool
    doDelete;

  doDelete=false;
  {
    Lock(&_blobRef->_mutexLock);
    if (--_blobRef->_refCount == 0)
      doDelete=true;
  }

  if (doDelete)
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }
  _blobRef=new Magick::BlobRef(0,0);
  _blobRef->_data=data_;
  _blobRef->_length=length_;
  _blobRef->_allocator=allocator_;
}
