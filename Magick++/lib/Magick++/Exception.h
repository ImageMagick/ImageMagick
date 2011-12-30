// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Definition of Magick::Exception and derived classes
// Magick::Warning* and Magick::Error*.  Derived from C++ STD
// 'exception' class for convenience.
//
// These classes form part of the Magick++ user interface.
//

#if !defined(Magick_Exception_header)
#define Magick_Exception_header

#include "Magick++/Include.h"
#include <string>
#include <exception>

namespace Magick
{
  class MagickPPExport Exception : public std::exception
  {
  public:
    Exception( const std::string& what_ );
    Exception( const Exception& original_ );
    Exception& operator= (const Exception& original_ );
    virtual const char* what () const throw();
    virtual ~Exception ( ) throw ();

  private:
    std::string _what;
  };

  //
  // Warnings
  //

  class MagickPPExport Warning : public Exception
  {
  public:
    explicit Warning ( const std::string& what_ );
    ~Warning() throw ();
  };
  
  class MagickPPExport WarningUndefined : public Warning
  {
  public:
    explicit WarningUndefined ( const std::string& what_ );
    ~WarningUndefined() throw ();
  };

  class MagickPPExport WarningBlob: public Warning
  {
  public:
    explicit WarningBlob ( const std::string& what_ );
    ~WarningBlob() throw ();
  };

  class MagickPPExport WarningCache: public Warning
  {
  public:
    explicit WarningCache ( const std::string& what_ );
    ~WarningCache() throw ();
  };

  class MagickPPExport WarningCoder: public Warning
  {
  public:
    explicit WarningCoder ( const std::string& what_ );
    ~WarningCoder() throw ();
  };

  class MagickPPExport WarningConfigure: public Warning
  {
  public:
    explicit WarningConfigure ( const std::string& what_ );
    ~WarningConfigure() throw ();
  };

  class MagickPPExport WarningCorruptImage: public Warning
  {
  public:
    explicit WarningCorruptImage ( const std::string& what_ );
    ~WarningCorruptImage() throw ();
  };

  class MagickPPExport WarningDelegate : public Warning
  {
  public:
    explicit WarningDelegate ( const std::string& what_ );
    ~WarningDelegate() throw ();
  };

  class MagickPPExport WarningDraw : public Warning
  {
  public:
    explicit WarningDraw ( const std::string& what_ );
    ~WarningDraw() throw ();
  };

  class MagickPPExport WarningFileOpen: public Warning
  {
  public:
    explicit WarningFileOpen ( const std::string& what_ );
    ~WarningFileOpen() throw ();
  };

  class MagickPPExport WarningImage: public Warning
  {
  public:
    explicit WarningImage ( const std::string& what_ );
    ~WarningImage() throw ();
  };

  class MagickPPExport WarningMissingDelegate : public Warning
  {
  public:
    explicit WarningMissingDelegate ( const std::string& what_ );
    ~WarningMissingDelegate() throw ();
  };

  class MagickPPExport WarningModule : public Warning
  {
  public:
    explicit WarningModule ( const std::string& what_ );
    ~WarningModule() throw ();
  };

  class MagickPPExport WarningMonitor : public Warning
  {
  public:
    explicit WarningMonitor ( const std::string& what_ );
    ~WarningMonitor() throw ();
  };

  class MagickPPExport WarningOption : public Warning
  {
  public:
    explicit WarningOption ( const std::string& what_ );
    ~WarningOption() throw ();
  };

  class MagickPPExport WarningRegistry : public Warning
  {
  public:
    explicit WarningRegistry ( const std::string& what_ );
    ~WarningRegistry() throw ();
  };

  class MagickPPExport WarningResourceLimit : public Warning
  {
  public:
    explicit WarningResourceLimit ( const std::string& what_ );
    ~WarningResourceLimit() throw ();
  };

  class MagickPPExport WarningStream : public Warning
  {
  public:
    explicit WarningStream ( const std::string& what_ );
    ~WarningStream() throw ();
  };

  class MagickPPExport WarningType : public Warning
  {
  public:
    explicit WarningType ( const std::string& what_ );
    ~WarningType() throw ();
  };

  class MagickPPExport WarningXServer : public Warning
  {
  public:
    explicit WarningXServer ( const std::string& what_ );
    ~WarningXServer() throw ();
  };

  //
  // Error exceptions
  //

  class MagickPPExport Error : public Exception
  {
  public:
    explicit Error ( const std::string& what_ );
    ~Error() throw ();
  };

  class MagickPPExport ErrorUndefined : public Error
  {
  public:
    explicit ErrorUndefined ( const std::string& what_ );
    ~ErrorUndefined() throw ();
  };

  class MagickPPExport ErrorBlob: public Error
  {
  public:
    explicit ErrorBlob ( const std::string& what_ );
    ~ErrorBlob() throw ();
  };

  class MagickPPExport ErrorCache: public Error
  {
  public:
    explicit ErrorCache ( const std::string& what_ );
    ~ErrorCache() throw ();
  };

  class MagickPPExport ErrorCoder: public Error
  {
  public:
    explicit ErrorCoder ( const std::string& what_ );
    ~ErrorCoder() throw ();
  };

  class MagickPPExport ErrorConfigure: public Error
  {
  public:
    explicit ErrorConfigure ( const std::string& what_ );
    ~ErrorConfigure() throw ();
  };

  class MagickPPExport ErrorCorruptImage: public Error
  {
  public:
    explicit ErrorCorruptImage ( const std::string& what_ );
    ~ErrorCorruptImage() throw ();
  };
  
  class MagickPPExport ErrorDelegate : public Error
  {
  public:
    explicit ErrorDelegate ( const std::string& what_ );
    ~ErrorDelegate() throw ();
  };
  
  class MagickPPExport ErrorDraw : public Error
  {
  public:
    explicit ErrorDraw ( const std::string& what_ );
    ~ErrorDraw() throw ();
  };

  class MagickPPExport ErrorFileOpen: public Error
  {
  public:
    explicit ErrorFileOpen ( const std::string& what_ );
    ~ErrorFileOpen() throw ();
  };

  class MagickPPExport ErrorImage: public Error
  {
  public:
    explicit ErrorImage ( const std::string& what_ );
    ~ErrorImage() throw ();
  };

  class MagickPPExport ErrorMissingDelegate : public Error
  {
  public:
    explicit ErrorMissingDelegate ( const std::string& what_ );
    ~ErrorMissingDelegate() throw ();
  };

  class MagickPPExport ErrorModule : public Error
  {
  public:
    explicit ErrorModule ( const std::string& what_ );
    ~ErrorModule() throw ();
  };

  class MagickPPExport ErrorMonitor : public Error
  {
  public:
    explicit ErrorMonitor ( const std::string& what_ );
    ~ErrorMonitor() throw ();
  };

  class MagickPPExport ErrorOption : public Error
  {
  public:
    explicit ErrorOption ( const std::string& what_ );
    ~ErrorOption() throw ();
  };

  class MagickPPExport ErrorRegistry : public Error
  {
  public:
    explicit ErrorRegistry ( const std::string& what_ );
    ~ErrorRegistry() throw ();
  };

  class MagickPPExport ErrorResourceLimit : public Error
  {
  public:
    explicit ErrorResourceLimit ( const std::string& what_ );
    ~ErrorResourceLimit() throw ();
  };

  class MagickPPExport ErrorStream : public Error
  {
  public:
    explicit ErrorStream ( const std::string& what_ );
    ~ErrorStream() throw ();
  };

  class MagickPPExport ErrorType : public Error
  {
  public:
    explicit ErrorType ( const std::string& what_ );
    ~ErrorType() throw ();
  };
  
  class MagickPPExport ErrorXServer : public Error
  {
  public:
    explicit ErrorXServer ( const std::string& what_ );
    ~ErrorXServer() throw ();
  };

  //
  // No user-serviceable components beyond this point.
  //

  // Throw exception based on raw data
  extern MagickPPExport void throwExceptionExplicit( const MagickCore::ExceptionType severity_,
                                                   const char* reason_,
                                                   const char* description_ = 0 );

  // Thow exception based on ImageMagick's ExceptionInfo
  extern MagickPPExport void throwException( MagickCore::ExceptionInfo &exception_ );

} // namespace Magick

#endif // Magick_Exception_header
