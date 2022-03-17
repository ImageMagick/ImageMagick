// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// CoderInfo Definition
//
// Container for image format support information.
//

#if !defined (Magick_CoderInfo_header)
#define Magick_CoderInfo_header  1

#include "Magick++/Include.h"
#include <string>

namespace Magick
{
  class MagickPPExport CoderInfo
  {
  public:

    enum MatchType {
      AnyMatch,  // match any coder
      TrueMatch, // match coder if true
      FalseMatch // match coder if false
    };

    // Default constructor
    CoderInfo(void);

    // Copy constructor
    CoderInfo(const CoderInfo &coder_);

    // Construct with coder name
    CoderInfo(const std::string &name_);

    // Destructor
    ~CoderInfo(void);

    // Assignment operator
    CoderInfo& operator=(const CoderInfo &coder_);

    // Format can read multi-threaded
    bool canReadMultithreaded(void) const;

    // Format can write multi-threaded
    bool canWriteMultithreaded(void) const;

    // Format description
    std::string description(void) const;

    // Format supports multiple frames
    bool isMultiFrame(void) const;

    // Format is readable
    bool isReadable(void) const;

    // Format is writeable
    bool isWritable(void) const;

    // Format mime type
    std::string mimeType(void) const;

    // Name of the module
    std::string module(void) const;

    // Format name
    std::string name(void) const;

    // Unregisters this coder
    bool unregister(void) const;

  private:
    bool        _decoderThreadSupport;
    std::string _description;
    bool        _encoderThreadSupport;
    bool        _isMultiFrame;
    bool        _isReadable;
    bool        _isWritable;
    std::string _mimeType;
    std::string _module;
    std::string _name;
  };

} // namespace Magick

#endif // Magick_CoderInfo_header
