// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002
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
  : _name(),
    _description(),
    _mimeType(),
    _isReadable(false),
    _isWritable(false),
    _isMultiFrame(false)
{
}

Magick::CoderInfo::CoderInfo(const Magick::CoderInfo &coder_)
{
  _name=coder_._name;
  _description=coder_._description;
  _mimeType=coder_._mimeType;
  _isReadable=coder_._isReadable;
  _isWritable=coder_._isWritable;
  _isMultiFrame=coder_._isMultiFrame;
}

Magick::CoderInfo::CoderInfo(const std::string &name_)
  : _name(),
    _description(),
    _mimeType(),
    _isReadable(false),
    _isWritable(false),
    _isMultiFrame(false)
{
  const Magick::MagickInfo
    *magickInfo;

  GetPPException;
  magickInfo=GetMagickInfo(name_.c_str(),exceptionInfo);
  ThrowPPException;
  if (magickInfo == 0)
    {
      throwExceptionExplicit(OptionError,"Coder not found",name_.c_str());
    }
  else
    {
      _name=string(magickInfo->name);
      _description=string(magickInfo->description);
      _mimeType=string(magickInfo->mime_type ? magickInfo->mime_type : "");
      _isReadable=((magickInfo->decoder == 0) ? false : true);
      _isWritable=((magickInfo->encoder == 0) ? false : true);
      _isMultiFrame=((magickInfo->adjoin == 0) ? false : true);
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
      _name=coder_._name;
      _description=coder_._description;
      _mimeType=coder_._mimeType;
      _isReadable=coder_._isReadable;
      _isWritable=coder_._isWritable;
      _isMultiFrame=coder_._isMultiFrame;
    }
  return(*this);
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