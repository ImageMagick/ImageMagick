// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2003
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Simple C++ function wrappers for often used or otherwise
// inconvenient ImageMagick equivalents
//

#if !defined(Magick_Functions_header)
#define Magick_Functions_header

#include "Magick++/Include.h"
#include <string>

namespace Magick
{
  // Clone C++ string as allocated C string, de-allocating any existing string
  MagickPPExport void CloneString(char **destination_,
    const std::string &source_);

  // Disable OpenCL acceleration (only works when build with OpenCL support)
  MagickPPExport void DisableOpenCL(void);

  // Enable OpenCL acceleration (only works when build with OpenCL support)
  MagickPPExport bool EnableOpenCL(void);

  // C library initialization routine
  MagickPPExport void InitializeMagick(const char *path_);

  // Seed a new sequence of pseudo-random numbers
  MagickPPExport void SetRandomSeed(const unsigned long seed);

  // Set the ImageMagick security policy.
  MagickPPExport bool SetSecurityPolicy(const std::string &policy_);

  // C library de-initialize routine
  MagickPPExport void TerminateMagick();

  // Constructor to initialize the Magick++ environment
  class MagickPPExport MagickPlusPlusGenesis
  {
  public:

    // Constructor to initialize Magick++
    MagickPlusPlusGenesis(const char *path_)
    {
      InitializeMagick( path_ );
    }

    // Destructor to de-initialize Magick++
    ~MagickPlusPlusGenesis()
    {
      TerminateMagick();
    }

  private:

  };
}
#endif // Magick_Functions_header
