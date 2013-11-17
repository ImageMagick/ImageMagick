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
  class MagickPPExport Pixels
  {
  public:

    // Construct pixel view using specified image.
    Pixels(Magick::Image &image_);

    // Destroy pixel view
    ~Pixels(void);

    // Transfer pixels from the image to the pixel view as defined by
    // the specified region. Modified pixels may be subsequently
    // transferred back to the image via sync.
    PixelPacket *get(const ::ssize_t x_, const ::ssize_t y_,
      const size_t columns_,const  size_t rows_ );

    // Transfer read-only pixels from the image to the pixel view as
    // defined by the specified region.
    const PixelPacket *getConst(const ::ssize_t x_,const ::ssize_t y_,
      const size_t columns_,const size_t rows_);

    // Allocate a pixel view region to store image pixels as defined
    // by the region rectangle.  This area is subsequently transferred
    // from the pixel view to the image via sync.
    PixelPacket *set(const ::ssize_t x_,const ::ssize_t y_,
      const size_t columns_,const size_t rows_);

    // Transfers the image view pixels to the image.
    void sync(void);

    // Width of view
    size_t columns(void) const;

    // Return pixel colormap index array
    IndexPacket *indexes(void);

    // Height of view
    size_t rows (void) const;

    // Left ordinate of view
    ::ssize_t x(void) const;

    // Top ordinate of view
    ::ssize_t y(void) const;

  private:

    // Copying and assigning Pixels is not supported.
    Pixels(const Pixels& pixels_);
    const Pixels& operator=(const Pixels& pixels_);

    Magick::Image             _image;     // Image reference
    MagickCore::CacheView     *_view;     // Image view handle
    ::ssize_t                 _x;         // Left ordinate of view
    ::ssize_t                 _y;         // Top ordinate of view
    size_t                    _columns;   // Width of view
    size_t                    _rows;      // Height of view
    MagickCore::ExceptionInfo _exception; // Any thrown exception

  }; // class Pixels

} // Magick namespace

//
// Inline methods
//

// Left ordinate of view
inline ::ssize_t Magick::Pixels::x(void) const
{
  return _x;
}

// Top ordinate of view
inline ::ssize_t Magick::Pixels::y(void) const
{
  return _y;
}

// Width of view
inline size_t Magick::Pixels::columns(void) const
{
  return _columns;
}

// Height of view
inline size_t Magick::Pixels::rows(void) const
{
  return _rows;
}

#endif // Magick_Pixels_header
