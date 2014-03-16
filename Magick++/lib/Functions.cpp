// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2002, 2003
//
// Simple C++ function wrappers for ImageMagick equivalents
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string>

using namespace std;

#include "Magick++/Functions.h"
#include "Magick++/Exception.h"

static bool magick_initialized=false;

// Clone C++ string as allocated C string, de-allocating any existing string
void Magick::CloneString(char **destination_, const std::string &source_)
{
  MagickCore::CloneString(destination_,source_.c_str());
}

MagickPPExport void Magick::DisableOpenCL(void)
{
  GetPPException;
  MagickCore::InitImageMagickOpenCL(MagickCore::MAGICK_OPENCL_OFF,NULL,NULL,
    &exceptionInfo);
  ThrowPPException;
}

MagickPPExport void Magick::EnableOpenCL(const bool useCache_)
{
  GetPPException;
  if (useCache_)
    MagickCore::InitImageMagickOpenCL(
      MagickCore::MAGICK_OPENCL_DEVICE_SELECT_AUTO,NULL,NULL,&exceptionInfo);
  else
    MagickCore::InitImageMagickOpenCL(
      MagickCore::MAGICK_OPENCL_DEVICE_SELECT_AUTO_CLEAR_CACHE,NULL,NULL,
      &exceptionInfo);
  ThrowPPException;
}

MagickPPExport void Magick::InitializeMagick(const char *path_)
{
  MagickCore::MagickCoreGenesis(path_,MagickFalse);
  if (!magick_initialized)
    magick_initialized=true;
}

MagickPPExport void Magick::TerminateMagick(void)
{
  if (magick_initialized)
    {
      magick_initialized=false;
      MagickCore::MagickCoreTerminus();
    }
}