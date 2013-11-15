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

Magick::Pixels::Pixels(Magick::Image &image_)
  : _image(image_),
    _view(AcquireVirtualCacheView(_image.image(),&_exception)),
    _x(0),
    _y(0),
    _columns(0),
    _rows(0)
{
  GetExceptionInfo(&_exception);

  if (!_view)
    _image.throwImageException();
}

Magick::Pixels::~Pixels(void)
{
  if (_view)
    _view=DestroyCacheView(_view);

  (void) DestroyExceptionInfo(&_exception);
}

Magick::PixelPacket* Magick::Pixels::get(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  _x=x_;
  _y=y_;
  _columns=columns_;
  _rows=rows_;

  PixelPacket* pixels=GetCacheViewAuthenticPixels(_view,x_,y_,columns_,rows_,
    &_exception);

  if (!pixels)
    throwException(_exception);

  return pixels;
}

const Magick::PixelPacket* Magick::Pixels::getConst(const ssize_t x_,
  const ssize_t y_,const size_t columns_,const size_t rows_)
{
  _x=x_;
  _y=y_;
  _columns=columns_;
  _rows=rows_;

  const PixelPacket* pixels=GetCacheViewVirtualPixels(_view,x_,y_,columns_,
    rows_,&_exception);

  if (!pixels)
    throwException(_exception);

  return pixels;
}

Magick::PixelPacket* Magick::Pixels::set(const ssize_t x_,const ssize_t y_,
  const size_t columns_,const size_t rows_)
{
  _x=x_;
  _y=y_;
  _columns=columns_;
  _rows=rows_;

  PixelPacket* pixels=QueueCacheViewAuthenticPixels(_view,x_,y_,columns_,rows_,
    &_exception);

  if (!pixels)
    throwException(_exception);

  return pixels;
}

void Magick::Pixels::sync(void)
{
  if( !SyncCacheViewAuthenticPixels(_view,&_exception))
    throwException(_exception);
}

Magick::IndexPacket* Magick::Pixels::indexes (void)
{
  IndexPacket* pixel_indexes=GetCacheViewAuthenticIndexQueue(_view);

  if (!pixel_indexes)
    _image.throwImageException();

  return pixel_indexes;
}
