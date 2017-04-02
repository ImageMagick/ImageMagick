// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2013-2015
//
// Pixels Implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include <cstring>
#include "Magick++/Include.h"
#include <string> // This is here to compile with Visual C++
#include "Magick++/Thread.h"
#include "Magick++/Exception.h"
#include "Magick++/Pixels.h"

Magick::Pixels::Pixels(Magick::Image &image_)
  : _image(image_),
    _x(0),
    _y(0),
    _columns(0),
    _rows(0)
{
  GetPPException;
    _view=AcquireVirtualCacheView(image_.image(),exceptionInfo),
  ThrowPPException(image_.quiet());
}

Magick::Pixels::~Pixels(void)
{
  if (_view)
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

Magick::PixelData::PixelData(Magick::Image &image_,std::string map_,
  const StorageType type_)
{
  init(image_,0,0,image_.columns(),image_.rows(),map_,type_);
}

Magick::PixelData::PixelData(Magick::Image &image_,const ::ssize_t x_,
  const ::ssize_t y_,const size_t width_,const size_t height_,std::string map_,
  const StorageType type_)
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
  std::string map_,const StorageType type_)
{
  size_t
    size;

  _data=(void *) NULL;
  _length=0;
  _size=0;
  if ((x_ < 0) || (width_ == 0) || (y_ < 0) || (height_ == 0) ||
      (x_ > (ssize_t) image_.columns()) || ((width_ + x_) > image_.columns())
      || (y_ > (ssize_t) image_.rows()) || ((height_ + y_) > image_.rows())
      || (map_.length() == 0))
    return;

  switch(type_)
  {
    case CharPixel:
      size=sizeof(unsigned char);
      break;
    case DoublePixel:
      size=sizeof(double);
      break;
    case FloatPixel:
      size=sizeof(float);
      break;
    case LongPixel:
      size=sizeof(unsigned int);
      break;
    case LongLongPixel:
      size=sizeof(MagickSizeType);
      break;
    case QuantumPixel:
      size=sizeof(Quantum);
      break;
    case ShortPixel:
      size=sizeof(unsigned short);
      break;
    default:
      throwExceptionExplicit(MagickCore::OptionError,"Invalid type");
      return;
  }

  _length=width_*height_*map_.length();
  _size=_length*size;
  _data=AcquireMagickMemory(_size);

  GetPPException;
  MagickCore::ExportImagePixels(image_.image(),x_,y_,width_,height_,
    map_.c_str(),type_,_data,exceptionInfo);
  if (exceptionInfo->severity != MagickCore::UndefinedException)
    relinquish();
  ThrowPPException(image_.quiet());
}

void Magick::PixelData::relinquish(void) throw()
{
  if (_data != (void *)NULL)
    _data=RelinquishMagickMemory(_data);
  _length=0;
  _size=0;
}
