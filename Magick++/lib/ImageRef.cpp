// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
// Copyright Dirk Lemstra 2015
//
// Implementation of ImageRef
//
// This is an internal implementation class.
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/ImageRef.h"
#include "Magick++/Exception.h"
#include "Magick++/Options.h"

Magick::ImageRef::ImageRef(void)
  : _image(0),
    _options(new Options),
    _refCount(1),
    _mutexLock()
{
  _image=AcquireImage(_options->imageInfo());
  throwException(&_image->exception);
}

Magick::ImageRef::ImageRef(MagickCore::Image *image_)
  : _image(image_),
    _options(new Options),
    _refCount(1),
    _mutexLock()
{
}

Magick::ImageRef::ImageRef(MagickCore::Image *image_,const Options *options_)
  : _image(image_),
    _options(0),
    _refCount(1),
    _mutexLock()
{
  _options=new Options(*options_);
}

Magick::ImageRef::~ImageRef(void)
{
  // Deallocate image
  if (_image != (MagickCore::Image*) NULL)
    {
      DestroyImageList(_image);
      _image=(MagickCore::Image*) NULL;
    }

  // Deallocate image options
  delete _options;
  _options=(Options *) NULL;
}

void Magick::ImageRef::image(MagickCore::Image * image_)
{
  if (_image != (MagickCore::Image*) NULL)
    DestroyImageList(_image);
  _image=image_;
}

void  Magick::ImageRef::options(Magick::Options *options_)
{
  delete _options;
  _options=options_;
}
