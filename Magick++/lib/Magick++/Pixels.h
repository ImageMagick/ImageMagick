// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Representation of a pixel view.
//

#if !defined(Magick_Pixels_header)
#define Magick_Pixels_header

#include "Magick++/Include.h"
#include "Magick++/Color.h"
#include "Magick++/Image.h"

namespace Magick
{
  class MagickDLLDecl Pixels
  {
  public:

    // Construct pixel view using specified image.
    Pixels( Magick::Image &image_ );

    // Destroy pixel view
    ~Pixels( void );
    
    // Transfer pixels from the image to the pixel view as defined by
    // the specified region. Modified pixels may be subsequently
    // transferred back to the image via sync.
    PixelPacket* get ( const int x_, const int y_,
		       const unsigned int columns_,const  unsigned int rows_ );

    // Transfer read-only pixels from the image to the pixel view as
    // defined by the specified region.
    const PixelPacket* getConst ( const int x_, const int y_,
                                  const unsigned int columns_,
                                  const unsigned int rows_ );
    
    // Transfers the image view pixels to the image.
    void sync ( void );
    
    // Allocate a pixel view region to store image pixels as defined
    // by the region rectangle.  This area is subsequently transferred
    // from the pixel view to the image via sync.
    PixelPacket* set ( const int x_, const int y_,
		       const unsigned int columns_, const unsigned int rows_ );

    // Return pixel colormap index array
    IndexPacket* indexes ( void );

    // Left ordinate of view
    int x ( void ) const;

    // Top ordinate of view
    int y ( void ) const;

    // Width of view
    unsigned int columns ( void ) const;

    // Height of view
    unsigned int rows ( void ) const;

#if 0
    // Transfer one or more pixel components from a buffer or file
    // into the image pixel view of an image.  Used to support image
    // decoders.
    void decode ( const QuantumType quantum_,
		  const unsigned char *source_ )
      {
	MagickCore::ReadPixelCache( _image.image(), quantum_, source_ );
      }
    
    // Transfer one or more pixel components from the image pixel
    // view to a buffer or file.  Used to support image encoders.
    void encode ( const QuantumType quantum_,
		  const unsigned char *destination_ )
      {
	MagickCore::WritePixelCache( _image.image(), quantum_, destination_ );
      }
#endif
  private:

    // Copying and assigning Pixels is not supported.
    Pixels( const Pixels& pixels_ );
    const Pixels& operator=( const Pixels& pixels_ );

    Magick::Image          _image;   // Image reference
    MagickCore::CacheView*   _view;    // Image view handle
    int                    _x;       // Left ordinate of view
    int                    _y;       // Top ordinate of view
    unsigned int           _columns; // Width of view
    unsigned int           _rows;    // Height of view
    MagickCore:: ExceptionInfo _exception; // Any thrown exception

  }; // class Pixels

} // Magick namespace

//
// Inline methods
//

// Left ordinate of view
inline int Magick::Pixels::x ( void ) const
{
  return _x;
}

// Top ordinate of view
inline int Magick::Pixels::y ( void ) const
{
  return _y;
}

// Width of view
inline unsigned int Magick::Pixels::columns ( void ) const
{
  return _columns;
}

// Height of view
inline unsigned int Magick::Pixels::rows ( void ) const
{
  return _rows;
}

#endif // Magick_Pixels_header
