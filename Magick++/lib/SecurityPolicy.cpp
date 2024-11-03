// This may look like C code, but it is really -*- C++ -*-
//
// Copyright @ 2018 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Implementation of the security policy.
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/SecurityPolicy.h"
#include "Magick++/Exception.h"
#include <string>
#include <sstream>


using namespace std;

bool Magick::SecurityPolicy::anonymousCacheMemoryMap()
{
  return(setValue(CachePolicyDomain,"memory-map","anonymous"));
}

bool Magick::SecurityPolicy::anonymousSystemMemoryMap()
{
  return(setValue(SystemPolicyDomain,"memory-map","anonymous"));
}

bool Magick::SecurityPolicy::precision(const int precision_)
{
  string
    value;

  value=toString(precision_);
  return(setValue(SystemPolicyDomain,"precision",value));
}

bool Magick::SecurityPolicy::maxMemoryRequest(const MagickSizeType limit_)
{
  string
    value;

  value=toString(limit_);
  return(setValue(SystemPolicyDomain,"max-memory-request",value));
}

bool Magick::SecurityPolicy::maxProfileSize(const MagickSizeType limit_)
{
  string
    value;

  value=toString(limit_);
  return(setValue(SystemPolicyDomain,"max-profile-size",value));
}

bool Magick::SecurityPolicy::shred(const int passes_)
{
  string
    value;

  value=toString(passes_);
  return(setValue(SystemPolicyDomain,"shred",value));
}

Magick::SecurityPolicy::SecurityPolicy()
{
}

bool Magick::SecurityPolicy::setValue(const PolicyDomain domain_,
  const std::string name_,const std::string value_)
{
  MagickBooleanType
    status;

  GetPPException;
  status=MagickCore::SetMagickSecurityPolicyValue(domain_,name_.c_str(),
    value_.c_str(),exceptionInfo);
  ThrowPPException(false);
  return(status != MagickFalse);
}

template <typename T>
std::string Magick::SecurityPolicy::toString(const T& value)
{
  stringstream ss;
  ss << value;
  return ss.str();
}
