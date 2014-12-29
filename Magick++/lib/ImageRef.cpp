// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
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
    _mutexLock(),
    _options(new Options),
    _refCount(1)
{
  GetPPException;
  _image=AcquireImage(_options->imageInfo(),exceptionInfo);
  ThrowPPException;
}

Magick::ImageRef::ImageRef(MagickCore::Image *image_)
  : _image(image_),
    _mutexLock(),
    _options(new Options),
    _refCount(1)
{
}

Magick::ImageRef::ImageRef(MagickCore::Image *image_,const Options *options_)
  : _image(image_),
    _mutexLock(),
    _options(0),
    _refCount(1)
{
  _options=new Options(*options_);
}

Magick::ImageRef::~ImageRef(void)
{
  // Deallocate image
  if (_image != (MagickCore::Image*) NULL)
    _image=DestroyImageList(_image);

  // Deallocate image options
  delete _options;
  _options=(Options *) NULL;
}

size_t Magick::ImageRef::decrease()
{
  size_t
    count;

  _mutexLock.lock();
  if (_refCount == 0)
    {
      _mutexLock.unlock();
      throwExceptionExplicit(OptionError,"Invalid call to decrease");
    }
  count=--_refCount;
  _mutexLock.unlock();
  return(count);
}

MagickCore::Image *&Magick::ImageRef::image(void)
{
  return(_image);
}

void Magick::ImageRef::increase()
{
  _mutexLock.lock();
  _refCount++;
  _mutexLock.unlock();
}

bool Magick::ImageRef::isOwner()
{
  bool
    isOwner;

  _mutexLock.lock();
  isOwner=(_refCount == 1);
  _mutexLock.unlock();
  return(isOwner);
}

void  Magick::ImageRef::options(Magick::Options *options_)
{
  delete _options;
  _options=options_;
}

Magick::Options *Magick::ImageRef::options(void)
{
  return(_options);
}

bool Magick::ImageRef::replaceImage(MagickCore::Image * replacement_)
{
  bool
    replaced;

  replaced=false;
  _mutexLock.lock();
  if (_refCount == 1)
    {
      if (_image != (MagickCore::Image*) NULL)
        (void) DestroyImageList(_image);
      _image=replacement_;
      replaced=true;
    }
  _mutexLock.unlock();
  return(replaced);
}

std::string Magick::ImageRef::signature(const bool force_)
{
  const char
    *property;

  // Re-calculate image signature if necessary
  GetPPException;
  _mutexLock.lock();
  property=(const char *) NULL;
  if (!force_ && (_image->taint == MagickFalse))
    property=GetImageProperty(_image,"Signature",exceptionInfo);
  if (property == (const char *) NULL)
    {
      (void) SignatureImage(_image,exceptionInfo);
      property=GetImageProperty(_image,"Signature",exceptionInfo);
    }
  _mutexLock.unlock();
  ThrowPPException;

  return(std::string(property));
}
