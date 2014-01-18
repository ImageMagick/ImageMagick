// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra, 2014
//
// Definition of resource limits.
//

#if !defined(Magick_ResourceLimits_header)
#define Magick_ResourceLimits_header

#include "Magick++/Include.h"

namespace Magick
{
  class MagickPPExport ResourceLimits
  {
  public:

    // Pixel cache limit in bytes. Requests for memory above this limit
    // are automagically allocated on disk.
    static void area(const MagickSizeType limit_);
    static MagickSizeType area(void);

    // Pixel cache limit in bytes. Requests for memory above this limit
    // will fail.
    static void disk(const MagickSizeType limit_);
    static MagickSizeType disk(void);

    // Pixel cache limit in bytes.  Once this memory limit is exceeded,
    // all subsequent pixels cache operations are to/from disk.
    static void map(const MagickSizeType limit_);
    static MagickSizeType map(void);

    // Pixel cache limit in bytes. Once this memory limit is exceeded,
    // all subsequent pixels cache operations are to/from disk or to/from
    // memory mapped files.
    static void memory(const MagickSizeType limit_);
    static MagickSizeType memory(void);

    // Limits the number of threads used in multithreaded operations.
    static void thread(const MagickSizeType limit_);
    static MagickSizeType thread(void);

  private:
    ResourceLimits(void);

  }; // class ResourceLimits

} // Magick namespace

#endif // Magick_ResourceLimits_header
