// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2002, 2003
// Copyright Dirk Lemstra 2014
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

void Magick::CloneString(char **destination_,const std::string &source_)
{
  MagickCore::CloneString(destination_,source_.c_str());
}

MagickPPExport void Magick::DisableOpenCL(void)
{
  GetPPException;
  MagickCore::InitImageMagickOpenCL(MagickCore::MAGICK_OPENCL_OFF,NULL,NULL,
    exceptionInfo);
  ThrowPPException(false);
}

MagickPPExport bool Magick::EnableOpenCL(const bool useCache_)
{
  bool
    status;

  GetPPException;
  if (useCache_)
    status=MagickCore::InitImageMagickOpenCL(
      MagickCore::MAGICK_OPENCL_DEVICE_SELECT_AUTO,NULL,NULL,exceptionInfo) ==
      MagickTrue;
  else
    status=MagickCore::InitImageMagickOpenCL(
      MagickCore::MAGICK_OPENCL_DEVICE_SELECT_AUTO_CLEAR_CACHE,NULL,NULL,
      exceptionInfo) == MagickTrue;
  ThrowPPException(false);
  return(status);
}

MagickPPExport void Magick::InitializeMagick(const char *path_)
{
  MagickCore::MagickCoreGenesis(path_,MagickFalse);
  if (!magick_initialized)
    magick_initialized=true;
}

//
// Create a local wrapper around MagickCoreTerminus
//
namespace Magick
{
  extern "C" {
    void MagickPlusPlusDestroyMagick(void);
  }
}

void Magick::MagickPlusPlusDestroyMagick(void)
{
  if (magick_initialized)
    {
      magick_initialized=false;
      MagickCore::MagickCoreTerminus();
    }
}

//
// Cleanup class to ensure that ImageMagick singletons are destroyed
// so as to avoid any resemblence to a memory leak (which seems to
// confuse users)
//
namespace Magick
{
  class MagickCleanUp
  {
  public:

    MagickCleanUp(void);
    ~MagickCleanUp(void);
  };

  // The destructor for this object is invoked when the destructors for
  // static objects in this translation unit are invoked.
  static MagickCleanUp magickCleanUpGuard;
}

Magick::MagickCleanUp::MagickCleanUp(void)
{
  // Don't even think about invoking InitializeMagick here!
}

Magick::MagickCleanUp::~MagickCleanUp(void)
{
  MagickPlusPlusDestroyMagick();
}
