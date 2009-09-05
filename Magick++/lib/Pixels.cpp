// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Pixels Implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string> // This is here to compile with Visual C++
#include "Magick++/Thread.h"
#include "Magick++/Exception.h"
#include "Magick++/Pixels.h"

namespace Magick
{


}

// Construct pixel view using specified image.
Magick::Pixels::Pixels( Magick::Image &image_ )
  : _image(image_),
    _view(AcquireCacheView(_image.image())),
    _x(0),
    _y(0),
    _columns(0),
    _rows(0)
{
  GetExceptionInfo( &_exception );

  if (!_view)
    _image.throwImageException();
  (void) DestroyExceptionInfo( &_exception );
}

// Destroy pixel view
Magick::Pixels::~Pixels( void )
{
  if ( _view )
    _view = DestroyCacheView( _view );
  
}

// Transfer pixels from the image to the pixel view as defined by
// the specified region. Modified pixels may be subsequently
// transferred back to the image via sync.
Magick::PixelPacket* Magick::Pixels::get ( const int x_,
					   const int y_,
					   const unsigned int columns_,
					   const unsigned int rows_ )
{
  _x = x_;
  _y = y_;
  _columns = columns_;
  _rows = rows_;

  PixelPacket* pixels = GetCacheViewAuthenticPixels( _view, x_, y_, columns_, rows_,  &_exception);

  if ( !pixels )
    throwException( *GetCacheViewException(_view) );
  
  return pixels;
}

// Transfer read-only pixels from the image to the pixel view as
// defined by the specified region.
const Magick::PixelPacket* Magick::Pixels::getConst ( const int x_, const int y_,
                                                      const unsigned int columns_,
                                                      const unsigned int rows_ )
{
  _x = x_;
  _y = y_;
  _columns = columns_;
  _rows = rows_;

  const PixelPacket* pixels =
    GetCacheViewVirtualPixels(_view, x_, y_, columns_, rows_, &_exception );

  if ( !pixels )
    throwException( _exception );

    return pixels;
}

// Transfers the image view pixels to the image.
void Magick::Pixels::sync ( void )
{
  if( !SyncCacheViewAuthenticPixels( _view, &_exception ) )
    throwException(  _exception );
}
    
// Allocate a pixel view region to store image pixels as defined
// by the region rectangle.  This area is subsequently transferred
// from the pixel view to the image via 'sync'.
Magick::PixelPacket* Magick::Pixels::set ( const int x_,
					   const int y_,
					   const unsigned int columns_,
					   const unsigned int rows_ )
{
  _x = x_;
  _y = y_;
  _columns = columns_;
  _rows = rows_;

  PixelPacket* pixels = QueueCacheViewAuthenticPixels( _view, static_cast<long>(x_), static_cast<long>(y_),
                                      columns_, rows_,  &_exception );
  if ( !pixels )
    throwException( _exception );
  
  return pixels;
}

// Return pixel colormap index array
Magick::IndexPacket* Magick::Pixels::indexes ( void )
{
  IndexPacket* pixel_indexes = GetCacheViewAuthenticIndexQueue( _view );

  if ( !pixel_indexes )
    _image.throwImageException();

  return pixel_indexes;
}
