// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2002, 2003
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Simple C++ function wrappers for ImageMagick equivalents
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string>

#include "Magick++/Functions.h"
#include "Magick++/Exception.h"

using namespace std;

static bool magick_initialized=false;

// Clone C++ string as allocated C string, de-allocating any existing string
MagickPPExport void Magick::CloneString(char **destination_,
  const std::string &source_)
{
  MagickCore::CloneString(destination_,source_.c_str());
}

MagickPPExport void Magick::DisableOpenCL(void)
{
  MagickCore::SetOpenCLEnabled(MagickFalse);
}

MagickPPExport bool Magick::EnableOpenCL(void)
{
  bool
    status;

 status=MagickCore::SetOpenCLEnabled(MagickTrue) != MagickFalse;
 return(status);
}

MagickPPExport void Magick::InitializeMagick(const char *path_)
{
  MagickCore::MagickCoreGenesis(path_,MagickFalse);
  if (!magick_initialized)
    magick_initialized=true;
}

MagickPPExport void Magick::SetRandomSeed(const unsigned long seed)
{
  MagickCore::SetRandomSecretKey(seed);
}

MagickPPExport bool Magick::SetSecurityPolicy(const std::string &policy_)
{
  bool
    status;

  GetPPException;
  status=MagickCore::SetMagickSecurityPolicy(policy_.c_str(),
    exceptionInfo) != MagickFalse;
  ThrowPPException(false);
  return(status);
}

MagickPPExport void Magick::TerminateMagick(void)
{
  if (magick_initialized)
    {
      magick_initialized=false;
      MagickCore::MagickWandTerminus();
    }
}