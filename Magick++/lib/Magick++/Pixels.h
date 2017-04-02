// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
// Copyright Dirk Lemstra 2014
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
    Quantum *get(const ::ssize_t x_,const ::ssize_t y_,
      const size_t columns_,const size_t rows_);

    // Transfer read-only pixels from the image to the pixel view as
    // defined by the specified region.
    const Quantum *getConst(const ::ssize_t x_,const ::ssize_t y_,
      const size_t columns_,const size_t rows_);

    // Return pixel metacontent
    void *metacontent(void);

    // Returns the offset for the specified channel.
    ssize_t offset(PixelChannel channel) const;

    // Allocate a pixel view region to store image pixels as defined
    // by the region rectangle.  This area is subsequently transferred
    // from the pixel view to the image via sync.
    Quantum *set(const ::ssize_t x_,const ::ssize_t y_,const size_t columns_,
      const size_t rows_ );

    // Transfers the image view pixels to the image.
    void sync(void);

    // Left ordinate of view
    ::ssize_t x(void) const;

    // Top ordinate of view
    ::ssize_t y(void) const;

    // Width of view
    size_t columns(void) const;

    // Height of view
    size_t rows(void) const;

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

  }; // class Pixels

  class MagickPPExport PixelData
  {
  public:

    // Construct pixel data using specified image
    PixelData(Magick::Image &image_,std::string map_,const StorageType type_);

    // Construct pixel data using specified image
    PixelData(Magick::Image &image_,const ::ssize_t x_,const ::ssize_t y_,
      const size_t width_,const size_t height_,std::string map_,
      const StorageType type_);

    // Destroy pixel data
    ~PixelData(void);

    // Pixel data buffer
    const void *data(void) const;

    // Length of the buffer
    ::ssize_t length(void) const;

    // Size of the buffer in bytes
    ::ssize_t size(void) const;

  private:

    // Copying and assigning PixelData is not supported
    PixelData(const PixelData& pixels_);
    const PixelData& operator=(const PixelData& pixels_);

    void init(Magick::Image &image_,const ::ssize_t x_,const ::ssize_t y_,
      const size_t width_,const size_t height_,std::string map_,
      const StorageType type_);

    void relinquish(void) throw();

    void      *_data;  // The pixel data
    ::ssize_t _length; // Length of the data
    ::ssize_t _size;   // Size of the data
  }; // class PixelData

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
