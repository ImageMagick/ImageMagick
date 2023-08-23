// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
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
  ThrowPPException(false);
}

Magick::ImageRef::ImageRef(MagickCore::Image *image_)
  : _image(image_),
    _mutexLock(),
    _options(new Options),
    _refCount(1)
{
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
      throwExceptionExplicit(MagickCore::OptionError,
        "Invalid call to decrease");
      return(0);
    }
  count=(size_t) (--_refCount);
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

bool Magick::ImageRef::isShared()
{
  bool
    isShared;

  _mutexLock.lock();
  isShared=(_refCount > 1);
  _mutexLock.unlock();
  return(isShared);
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

Magick::ImageRef *Magick::ImageRef::replaceImage(ImageRef *imgRef,
  MagickCore::Image *replacement_)
{
  Magick::ImageRef
    *instance;

  imgRef->_mutexLock.lock();
  if (imgRef->_refCount == 1)
    {
      // We can replace the image if we own it.
      instance=imgRef;
      if (imgRef->_image != (MagickCore::Image*) NULL)
        (void) DestroyImageList(imgRef->_image);
      imgRef->_image=replacement_;
      imgRef->_mutexLock.unlock();
    }
  else
    {
      // We don't own the image, create a new ImageRef instance.
      instance=new ImageRef(replacement_,imgRef->_options);
      imgRef->_refCount--;
      imgRef->_mutexLock.unlock();
    }
  return(instance);
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
  ThrowPPException(true);

  return(std::string(property));
}

Magick::ImageRef::ImageRef(MagickCore::Image *image_,const Options *options_)
  : _image(image_),
    _mutexLock(),
    _options(0),
    _refCount(1)
{
  _options=new Options(*options_);
}
