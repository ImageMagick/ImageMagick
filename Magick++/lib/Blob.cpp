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

//
// Implementation of Magick::Blob
//

// Default constructor
Magick::Blob::Blob ( void )
  : _blobRef(new Magick::BlobRef( 0, 0 ))
{
}

// Construct with data
Magick::Blob::Blob ( const void* data_, size_t length_ )
  : _blobRef(new Magick::BlobRef( data_, length_ ))
{
}

// Copy constructor (reference counted)
Magick::Blob::Blob ( const Magick::Blob& blob_ )
  : _blobRef(blob_._blobRef)
{
  // Increase reference count
  Lock( &_blobRef->_mutexLock );
  ++_blobRef->_refCount;
}

// Destructor (reference counted)
Magick::Blob::~Blob ()
{
  bool doDelete = false;
  {
    Lock( &_blobRef->_mutexLock );
    if ( --_blobRef->_refCount == 0 )
      doDelete = true;
  }

  if ( doDelete )
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }
  _blobRef=0;
}

// Assignment operator (reference counted)
Magick::Blob& Magick::Blob::operator= ( const Magick::Blob& blob_ )
{
  if(this != &blob_)
    {
      {
        Lock( &blob_._blobRef->_mutexLock );
        ++blob_._blobRef->_refCount;
      }
      bool doDelete = false;
      {
        Lock( &_blobRef->_mutexLock );
        if ( --_blobRef->_refCount == 0 )
          doDelete = true;
      }
      if ( doDelete )
        {
          delete _blobRef;
        }
      _blobRef = blob_._blobRef;
    }
  return *this;
}

// Update object contents from Base64-encoded string representation.
void Magick::Blob::base64 ( const std::string base64_ )
{
  size_t length;

  unsigned char *decoded =
    Base64Decode( base64_.c_str(), &length );

  if(decoded)
    updateNoCopy( static_cast<void*>(decoded), length,
                  Magick::Blob::MallocAllocator );
}

// Return Base64-encoded string representation.
std::string Magick::Blob::base64 ( void )
{
  size_t encoded_length = 0;

  char *encoded =
    Base64Encode(static_cast<const unsigned char*>(data()), length(), &encoded_length);

  if(encoded)
    {
      std::string result(encoded,encoded_length);
      encoded=(char *) RelinquishMagickMemory(encoded);
      return result;
    }

  return std::string();
}

// Update object contents, making a copy of the supplied data.
// Any existing data in the object is deallocated.
void Magick::Blob::update ( const void* data_, size_t length_ )
{
  bool doDelete = false;
  {
    Lock( &_blobRef->_mutexLock );
    if ( --_blobRef->_refCount == 0 )
      doDelete = true;
  }
  if ( doDelete )
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }

  _blobRef = new Magick::BlobRef( data_, length_ );
}

// Update object contents, using supplied pointer directly (no copy)
// Any existing data in the object is deallocated.  The user must
// ensure that the pointer supplied is not deleted or otherwise
// modified after it has been supplied to this method.
void Magick::Blob::updateNoCopy ( void* data_, size_t length_,
                                  Magick::Blob::Allocator allocator_  )
{
  bool doDelete = false;
  {
    Lock( &_blobRef->_mutexLock );
    if ( --_blobRef->_refCount == 0 )
      doDelete = true;
  }
  if ( doDelete )
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }
  _blobRef = new Magick::BlobRef( 0, 0 );
  _blobRef->_data   = data_;
  _blobRef->_length = length_;
  _blobRef->_allocator = allocator_;
}

// Obtain pointer to data
const void* Magick::Blob::data( void ) const
{
  return _blobRef->_data;
}

// Obtain data length
size_t Magick::Blob::length( void ) const
{
  return _blobRef->_length;
}

