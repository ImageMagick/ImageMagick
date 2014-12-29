// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra 2014
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

    // The maximum number of open pixel cache files. When this limit is
    // exceeded, any subsequent pixels cached to disk are closed and reopened
    // on demand. This behavior permits a large number of images to be accessed
    // simultaneously on disk, but with a speed penalty due to repeated
    // open/close calls.
    static void file(const MagickSizeType limit_);
    static MagickSizeType file(void);

    // The maximum height of an image.
    static void height(const MagickSizeType limit_);
    static MagickSizeType height(void);

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

    // Periodically yield the CPU for at least the time specified in
    // milliseconds.
    static void throttle(const MagickSizeType limit_);
    static MagickSizeType throttle(void);

    // The maximum width of an image.
    static void width(const MagickSizeType limit_);
    static MagickSizeType width(void);

  private:
    ResourceLimits(void);

  }; // class ResourceLimits

} // Magick namespace

#endif // Magick_ResourceLimits_header
