// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Pixels Implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include <limits>

#include "Magick++/Include.h"
#include "Magick++/Thread.h"
#include "Magick++/Exception.h"
#include "Magick++/Pixels.h"

Magick::Pixels::Pixels(Magick::Image &image_)
  : _image(image_),
    _view((MagickCore::CacheView *) NULL),
    _x(0),
    _y(0),
    _columns(0),
    _rows(0)
{
  GetPPException;
  _view=AcquireVirtualCacheView(image_.image(),exceptionInfo);
  try
    {
      ThrowPPException(image_.quiet());
    }
  catch (...)
    {
      // The destructor does not run when a constructor throws, so release
      // the view here to avoid leaking it.
      if (_view != (MagickCore::CacheView *) NULL)
        _view=DestroyCacheView(_view);
      throw;
    }
}

Magick::Pixels::~Pixels(void)
{
  if (_view != (MagickCore::CacheView *) NULL)
    _view=DestroyCacheView(_view);
}

Magick::Quantum* Magick::Pixels::get(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  _x=x_;
  _y=y_;
  _columns=columns_;
  _rows=rows_;

  GetPPException;
  Quantum* pixels=GetCacheViewAuthenticPixels(_view,x_,y_,columns_,rows_,
    exceptionInfo);
  ThrowPPException(_image.quiet());

  return pixels;
}

const Magick::Quantum* Magick::Pixels::getConst(const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_)
{
  _x=x_;
  _y=y_;
  _columns=columns_;
  _rows=rows_;

  GetPPException;
  const Quantum* pixels=GetCacheViewVirtualPixels(_view,x_,y_,columns_,rows_,
    exceptionInfo);
  ThrowPPException(_image.quiet());

  return pixels;
}

ssize_t Magick::Pixels::offset(PixelChannel channel) const
{
  // Guard against an out-of-range channel indexing past channel_map.
  if ((ssize_t) channel >= MaxPixelChannels)
    return -1;
  if (_image.constImage()->channel_map[channel].traits == UndefinedPixelTrait)
    return -1;
  return _image.constImage()->channel_map[channel].offset;
}

Magick::Quantum* Magick::Pixels::set(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  _x=x_;
  _y=y_;
  _columns=columns_;
  _rows=rows_;

  GetPPException;
  Quantum* pixels=QueueCacheViewAuthenticPixels(_view,x_,y_,columns_,rows_,
    exceptionInfo);
  ThrowPPException(_image.quiet());

  return pixels;
}

void Magick::Pixels::sync(void)
{
  GetPPException;
  (void) SyncCacheViewAuthenticPixels(_view,exceptionInfo);
  ThrowPPException(_image.quiet());
}

// Return pixel meta content
void* Magick::Pixels::metacontent(void)
{
  void* pixel_metacontent=GetCacheViewAuthenticMetacontent(_view);

  return pixel_metacontent;
}

Magick::PixelData::PixelData(Magick::Image &image_,const std::string &map_,
  const StorageType type_)
  : _data((void *) NULL),
    _length(0),
    _size(0)
{
  init(image_,0,0,image_.columns(),image_.rows(),map_,type_);
}

Magick::PixelData::PixelData(Magick::Image &image_,const ::ssize_t x_,
  const ::ssize_t y_,const size_t width_,const size_t height_,
  const std::string &map_,const StorageType type_)
  : _data((void *) NULL),
    _length(0),
    _size(0)
{
  init(image_,x_,y_,width_,height_,map_,type_);
}

Magick::PixelData::~PixelData(void)
{
  relinquish();
}

const void *Magick::PixelData::data(void) const
{
  return(_data);
}

::ssize_t Magick::PixelData::length(void) const
{
  return(_length);
}

::ssize_t Magick::PixelData::size(void) const
{
  return(_size);
}

void Magick::PixelData::init(Magick::Image &image_,const ::ssize_t x_,
  const ::ssize_t y_,const size_t width_,const size_t height_,
  const std::string &map_,const StorageType type_)
{
  size_t
    columns,
    element_size,
    length,
    rows;

  const size_t
    max_size=(size_t) (std::numeric_limits< ::ssize_t >::max)();

  _data=(void *) NULL;
  _length=0;
  _size=0;

  // Validate the region using unsigned arithmetic so that large width/height
  // values cannot overflow a signed sum and slip past the bounds check.
  columns=image_.columns();
  rows=image_.rows();
  if ((x_ < 0) || (y_ < 0) || (width_ == 0) || (height_ == 0) ||
      (map_.length() == 0) ||
      ((size_t) x_ >= columns) || (width_ > (columns-(size_t) x_)) ||
      ((size_t) y_ >= rows) || (height_ > (rows-(size_t) y_)))
    return;

  switch(type_)
  {
    case CharPixel:
      element_size=sizeof(unsigned char);
      break;
    case DoublePixel:
      element_size=sizeof(double);
      break;
    case FloatPixel:
      element_size=sizeof(float);
      break;
    case LongPixel:
      element_size=sizeof(unsigned int);
      break;
    case LongLongPixel:
      element_size=sizeof(MagickSizeType);
      break;
    case QuantumPixel:
      element_size=sizeof(Quantum);
      break;
    case ShortPixel:
      element_size=sizeof(unsigned short);
      break;
    default:
      throwExceptionExplicit(MagickCore::OptionError,"Invalid type");
      return;
  }

  // Compute width*height*channels and the byte size with overflow checks.
  if (width_ > (max_size/height_))
    {
      throwExceptionExplicit(MagickCore::ResourceLimitError,
        "Memory allocation failed");
      return;
    }
  length=width_*height_;
  if (map_.length() > (max_size/length))
    {
      throwExceptionExplicit(MagickCore::ResourceLimitError,
        "Memory allocation failed");
      return;
    }
  length*=map_.length();
  if (length > (max_size/element_size))
    {
      throwExceptionExplicit(MagickCore::ResourceLimitError,
        "Memory allocation failed");
      return;
    }

  _data=AcquireMagickMemory(length*element_size);
  if (_data == (void *) NULL)
    {
      throwExceptionExplicit(MagickCore::ResourceLimitError,
        "Memory allocation failed");
      return;
    }
  _length=(::ssize_t) length;
  _size=(::ssize_t) (length*element_size);

  GetPPException;
  MagickCore::ExportImagePixels(image_.image(),x_,y_,width_,height_,
    map_.c_str(),type_,_data,exceptionInfo);
  if (exceptionInfo->severity != MagickCore::UndefinedException)
    relinquish();
  ThrowPPException(image_.quiet());
}

void Magick::PixelData::relinquish(void) MAGICK_PIXELS_NOEXCEPT
{
  if (_data != (void *)NULL)
    _data=RelinquishMagickMemory(_data);
  _length=0;
  _size=0;
}
