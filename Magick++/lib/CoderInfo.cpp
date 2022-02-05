// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// CoderInfo implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION  1

#include "Magick++/Include.h"
#include "Magick++/CoderInfo.h"
#include "Magick++/Exception.h"

using namespace std;

Magick::CoderInfo::CoderInfo(void)
  : _decoderThreadSupport(false),
    _description(),
    _encoderThreadSupport(false),
    _isMultiFrame(false),
    _isReadable(false),
    _isWritable(false),
    _mimeType(),
    _module(),
    _name()
{
}

Magick::CoderInfo::CoderInfo(const Magick::CoderInfo &coder_)
  : _decoderThreadSupport(coder_._decoderThreadSupport),
    _description(coder_._description),
    _encoderThreadSupport(coder_._encoderThreadSupport),
    _isMultiFrame(coder_._isMultiFrame),
    _isReadable(coder_._isReadable),
    _isWritable(coder_._isWritable),
    _mimeType(coder_._mimeType),
    _module(coder_._module),
    _name(coder_._name)
{
}

Magick::CoderInfo::CoderInfo(const std::string &name_)
  : _decoderThreadSupport(false),
    _description(),
    _encoderThreadSupport(false),
    _isMultiFrame(false),
    _isReadable(false),
    _isWritable(false),
    _mimeType(),
    _module(),
    _name()
{
  const Magick::MagickInfo
    *magickInfo;

  GetPPException;
  magickInfo=GetMagickInfo(name_.c_str(),exceptionInfo);
  ThrowPPException(false);
  if (magickInfo == 0)
    throwExceptionExplicit(MagickCore::OptionError,"Coder not found",
      name_.c_str());
  else
    {
      _decoderThreadSupport=(GetMagickDecoderThreadSupport(magickInfo) ==
        MagickTrue) ? true : false;
      _description=std::string(magickInfo->description);
      _encoderThreadSupport=(GetMagickEncoderThreadSupport(magickInfo) ==
        MagickTrue) ? true : false;
      _isMultiFrame=(GetMagickAdjoin(magickInfo) == MagickTrue) ? true : false;
      _isReadable=((magickInfo->decoder == (MagickCore::DecodeImageHandler *)
        NULL) ? false : true);
      _isWritable=((magickInfo->encoder == (MagickCore::EncodeImageHandler *)
        NULL) ? false : true);
      _mimeType=std::string(magickInfo->mime_type != (char *) NULL ?
        magickInfo->mime_type : "");
      _module=std::string(magickInfo->magick_module);
      _name=std::string(magickInfo->name);
    }
}

Magick::CoderInfo::~CoderInfo(void)
{
}

Magick::CoderInfo& Magick::CoderInfo::operator=(const CoderInfo &coder_)
{
  // If not being set to ourself
  if (this != &coder_)
    {
      _decoderThreadSupport=coder_._decoderThreadSupport;
      _description=coder_._description;
      _encoderThreadSupport=coder_._encoderThreadSupport;
      _isMultiFrame=coder_._isMultiFrame;
      _isReadable=coder_._isReadable;
      _isWritable=coder_._isWritable;
      _mimeType=coder_._mimeType;
      _module=coder_._module;
      _name=coder_._name;
    }
  return(*this);
}

bool Magick::CoderInfo::canReadMultithreaded(void) const
{
  return(_decoderThreadSupport);
}

bool Magick::CoderInfo::canWriteMultithreaded(void) const
{
  return(_encoderThreadSupport);
}

std::string Magick::CoderInfo::description(void) const
{
  return(_description);
}

bool Magick::CoderInfo::isReadable(void) const
{
  return(_isReadable);
}

bool Magick::CoderInfo::isWritable(void) const
{
  return(_isWritable);
}

bool Magick::CoderInfo::isMultiFrame(void) const
{
  return(_isMultiFrame);
}

std::string Magick::CoderInfo::mimeType(void) const
{
  return(_mimeType);
}

std::string Magick::CoderInfo::module(void) const
{
  return(_module);
}

std::string Magick::CoderInfo::name(void) const
{
  return(_name);
}

bool Magick::CoderInfo::unregister(void) const
{
  return(UnregisterMagickInfo(_name.c_str()) != MagickFalse);
}
