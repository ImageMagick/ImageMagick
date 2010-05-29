// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Definition of an Image reference
//
// This is a private implementation class which should never be
// referenced by any user code.
//

#if !defined(Magick_ImageRef_header)
#define Magick_ImageRef_header

#include "Magick++/Include.h"
#include "Magick++/Thread.h"

namespace Magick
{
  class Options;

  //
  // Reference counted access to Image *
  //
  class MagickDLLDecl ImageRef {
    friend class Image; 
  private:
    // Construct with an image pointer and default options
    ImageRef ( MagickCore::Image * image_ );
    // Construct with an image pointer and options
    ImageRef ( MagickCore::Image * image_, const Options * options_ );
    // Construct with null image and default options
    ImageRef ( void );
    // Destroy image and options
    ~ImageRef ( void );

    // Copy constructor and assignment are not supported
    ImageRef(const ImageRef&);
    ImageRef& operator=(const ImageRef&);
    
    void                 image ( MagickCore::Image * image_ );
    MagickCore::Image *&  image ( void );
    
    void                 options ( Options * options_ );
    Options *            options ( void );

    void                 id ( const ssize_t id_ );
    ssize_t                 id ( void ) const;
    
    MagickCore::Image *   _image;    // ImageMagick Image
    Options *            _options;  // User-specified options
    ssize_t                 _id;       // Registry ID (-1 if not registered)
    ssize_t                  _refCount; // Reference count
    MutexLock            _mutexLock;// Mutex lock
  };

} // end of namespace Magick

//
// Inlines
//

// Retrieve image from reference
inline MagickCore::Image *& Magick::ImageRef::image ( void )
{
  return _image;
}

// Retrieve Options from reference
inline Magick::Options * Magick::ImageRef::options ( void )
{
  return _options;
}

// Retrieve registration id from reference
inline ssize_t Magick::ImageRef::id ( void ) const
{
  return _id;
}

#endif // Magick_ImageRef_header
