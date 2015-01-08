// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002
// Copyright Dirk Lemstra 2013-2015
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
  : _description(),
    _isMultiFrame(false),
    _isReadable(false),
    _isWritable(false),
    _mimeType(),
    _name(),
    _threadSupport(NoThreadSupport)
{
}

Magick::CoderInfo::CoderInfo(const Magick::CoderInfo &coder_)
  : _description(coder_._description),
    _isMultiFrame(coder_._isMultiFrame),
    _isReadable(coder_._isReadable),
    _isWritable(coder_._isWritable),
    _mimeType(coder_._mimeType),
    _name(coder_._name),
    _threadSupport(coder_._threadSupport)
{
}

Magick::CoderInfo::CoderInfo(const std::string &name_)
  : _description(),
    _isMultiFrame(false),
    _isReadable(false),
    _isWritable(false),
    _mimeType(),
    _name(),
    _threadSupport(NoThreadSupport)
{
  const Magick::MagickInfo
    *magickInfo;

  GetPPException;
  magickInfo=GetMagickInfo(name_.c_str(),exceptionInfo);
  ThrowPPException;
  if (magickInfo == 0)
    throwExceptionExplicit(OptionError,"Coder not found",name_.c_str());
  else
    {
      _description=string(magickInfo->description);
      _isMultiFrame=((magickInfo->adjoin == MagickFalse) ? false : true);
      _isReadable=((magickInfo->decoder == (MagickCore::DecodeImageHandler *)
        NULL) ? false : true);
      _isWritable=((magickInfo->encoder == (MagickCore::EncodeImageHandler *)
        NULL) ? false : true);
      _mimeType=string(magickInfo->mime_type ? magickInfo->mime_type : "");
      _name=string(magickInfo->name);
      _threadSupport=magickInfo->thread_support;
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
      _description=coder_._description;
      _isMultiFrame=coder_._isMultiFrame;
      _isReadable=coder_._isReadable;
      _isWritable=coder_._isWritable;
      _mimeType=coder_._mimeType;
      _name=coder_._name;
      _threadSupport=coder_._threadSupport;
    }
  return(*this);
}

bool Magick::CoderInfo::canReadMultiThreaded(void) const
{
  return((_threadSupport & DecoderThreadSupport) == DecoderThreadSupport);
}

bool Magick::CoderInfo::canWriteMultiThreaded(void) const
{
  return((_threadSupport & EncoderThreadSupport) == EncoderThreadSupport);
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

std::string Magick::CoderInfo::name(void) const
{
  return(_name);
}

bool Magick::CoderInfo::unregister(void) const
{
  return(UnregisterMagickInfo(_name.c_str()) != MagickFalse);
}