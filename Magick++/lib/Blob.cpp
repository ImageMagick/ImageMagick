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
  // A destructor must never let an exception escape.
  try
  {
    if (_blobRef->decrease() == 0)
      delete _blobRef;
  }
  catch(...)
  {
  }

  _blobRef=(Magick::BlobRef *) NULL;
}

Magick::Blob& Magick::Blob::operator=(const Magick::Blob& blob_)
{
  if (this != &blob_)
    {
      BlobRef
        *oldRef;

      // Take the new reference and publish it before releasing the old
      // one, so this object always points at a live BlobRef even if the
      // release throws.
      blob_._blobRef->increase();
      oldRef=_blobRef;
      _blobRef=blob_._blobRef;

      if (oldRef->decrease() == 0)
        delete oldRef;
    }
  return(*this);
}

void Magick::Blob::base64(const std::string& base64_)
{
  size_t
    length;

  unsigned char
    *decoded;

  length=0;
  decoded=Base64Decode(base64_.c_str(),&length);

  if (decoded == (unsigned char *) NULL)
    return;

  // updateNoCopy() only throws before it takes ownership, so release the
  // decoded buffer here if that happens.
  try
  {
    updateNoCopy(static_cast<void*>(decoded),length,
      Magick::Blob::MallocAllocator);
  }
  catch(...)
  {
    decoded=(unsigned char *) RelinquishMagickMemory(decoded);
    throw;
  }
}

std::string Magick::Blob::base64(void) const
{
  size_t
    encoded_length;

  char
    *encoded;

  std::string
    result;

  // Base64Encode() asserts on an empty or NULL buffer.
  if ((data() == (const void *) NULL) || (length() == 0))
    return(std::string());

  encoded_length=0;
  encoded=Base64Encode(static_cast<const unsigned char*>(data()),length(),
    &encoded_length);

  if (encoded == (char *) NULL)
    return(std::string());

  // Constructing the string can throw (bad_alloc); do not leak the buffer.
  try
  {
    result.assign(encoded,encoded_length);
  }
  catch(...)
  {
    encoded=(char *) RelinquishMagickMemory(encoded);
    throw;
  }

  encoded=(char *) RelinquishMagickMemory(encoded);
  return(result);
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
  // Ownership of data_ transfers only once newRef holds it, so on a throw
  // here the caller still owns data_.
  newRef=new Magick::BlobRef((const void*) NULL,0);
  newRef->data=data_;
  newRef->length=length_;
  newRef->allocator=allocator_;

  oldRef=_blobRef;
  _blobRef=newRef;

  if (oldRef->decrease() == 0)
    delete oldRef;
}
