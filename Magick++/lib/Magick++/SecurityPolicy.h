// This may look like C code, but it is really -*- C++ -*-
//
// Copyright @ 2018 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Definition of the security policy.
//

#if !defined(Magick_SecurityPolicy_header)
#define Magick_SecurityPolicy_header

#include "Magick++/Include.h"
#include <string>

namespace Magick
{
  class MagickPPExport SecurityPolicy
  {
  public:

    // The maximum number of significant digits to be printed.
    static bool precision(const int precision_);

    // Enables anonymous mapping for pixel cache.
    static bool anonymousCacheMemoryMap();

    // Enables anonymous virtual memory.
    static bool anonymousSystemMemoryMap();

    // The memory request limit in bytes.
    static bool maxMemoryRequest(const MagickSizeType limit_);

    // The maximum size of a profile in bytes.
    static bool maxProfileSize(const MagickSizeType limit_);

    // The number of passes to use when shredding files.
    static bool shred(const int passes_);

  private:
    SecurityPolicy(void);

    static bool setValue(const PolicyDomain domain_, const std::string name_,
      const std::string value_);

    template <typename T>
    static std::string toString(const T& value);

  }; // class SecurityPolicy

} // Magick namespace

#endif // Magick_SecurityPolicy_header
