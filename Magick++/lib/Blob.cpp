// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2004
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Implementation of Blob
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include "Magick++/Blob.h"
#include "Magick++/BlobRef.h"
#include "Magick++/Exception.h"

Magick::Blob::Blob(void)
  : _blobRef(new Magick::BlobRef(0,0))
{
}

Magick::Blob::Blob(const void* data_,const size_t length_)
  : _blobRef(new Magick::BlobRef(data_, length_))
{
}

Magick::Blob::Blob(const Magick::Blob& blob_)
  : _blobRef(blob_._blobRef)
{
  // Increase reference count
  _blobRef->increase();
}

Magick::Blob::~Blob()
{
  try
  {
    if (_blobRef->decrease() == 0)
      delete _blobRef;
  }
  catch(Magick::Exception&)
  {
  }

  _blobRef=(Magick::BlobRef *) NULL;
}

Magick::Blob& Magick::Blob::operator=(const Magick::Blob& blob_)
{
  if (this != &blob_)
    {
      blob_._blobRef->increase();
      if (_blobRef->decrease() == 0)
        delete _blobRef;
      
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

  if(decoded)
    updateNoCopy(static_cast<void*>(decoded),length,
      Magick::Blob::MallocAllocator);
}

std::string Magick::Blob::base64(void) const
{
  size_t
    encoded_length;

  char
    *encoded;

  std::string
    result;

  encoded_length=0;
  encoded=Base64Encode(static_cast<const unsigned char*>(data()),length(),
    &encoded_length);

  if(encoded)
    {
      result=std::string(encoded,encoded_length);
      encoded=(char *) RelinquishMagickMemory(encoded);
      return result;
    }

  return(std::string());
}

const void* Magick::Blob::data(void) const
{
  return(_blobRef->data);
}

size_t Magick::Blob::length(void) const
{
  return(_blobRef->length);
}

void Magick::Blob::update(const void* data_,size_t length_)
{
  BlobRef
    *newRef,
    *oldRef;

  // Build the replacement first. data_ may alias the buffer owned by the
  // current BlobRef (e.g. blob.update(blob.data(),blob.length())), so the
  // old buffer must stay alive until the copy is complete. If the
  // allocation throws, _blobRef is untouched and the Blob remains valid.
  newRef=new Magick::BlobRef(data_,length_);
  oldRef=_blobRef;
  _blobRef=newRef;

  if (oldRef->decrease() == 0)
    delete oldRef;
}

void Magick::Blob::updateNoCopy(void* data_,size_t length_,
  Magick::Blob::Allocator allocator_)
{
  BlobRef
    *newRef,
    *oldRef;

  // Allocate the replacement before releasing the current reference so
  // that a failed allocation leaves the Blob in its previous valid state.
  newRef=new Magick::BlobRef((const void*) NULL,0);
  newRef->data=data_;
  newRef->length=length_;
  newRef->allocator=allocator_;

  oldRef=_blobRef;
  _blobRef=newRef;

  if (oldRef->decrease() == 0)
    delete oldRef;
}
