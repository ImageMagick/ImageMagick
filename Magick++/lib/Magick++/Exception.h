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
  class MagickDLLDecl Exception : public std::exception
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

  class MagickDLLDecl Warning : public Exception
  {
  public:
    explicit Warning ( const std::string& what_ );
  };
  
  class MagickDLLDecl WarningUndefined : public Warning
  {
  public:
    explicit WarningUndefined ( const std::string& what_ );
  };

  class MagickDLLDecl WarningBlob: public Warning
  {
  public:
    explicit WarningBlob ( const std::string& what_ );
  };

  class MagickDLLDecl WarningCache: public Warning
  {
  public:
    explicit WarningCache ( const std::string& what_ );
  };

  class MagickDLLDecl WarningCoder: public Warning
  {
  public:
    explicit WarningCoder ( const std::string& what_ );
  };

  class MagickDLLDecl WarningConfigure: public Warning
  {
  public:
    explicit WarningConfigure ( const std::string& what_ );
  };

  class MagickDLLDecl WarningCorruptImage: public Warning
  {
  public:
    explicit WarningCorruptImage ( const std::string& what_ );
  };

  class MagickDLLDecl WarningDelegate : public Warning
  {
  public:
    explicit WarningDelegate ( const std::string& what_ );
  };

  class MagickDLLDecl WarningDraw : public Warning
  {
  public:
    explicit WarningDraw ( const std::string& what_ );
  };

  class MagickDLLDecl WarningFileOpen: public Warning
  {
  public:
    explicit WarningFileOpen ( const std::string& what_ );
  };

  class MagickDLLDecl WarningImage: public Warning
  {
  public:
    explicit WarningImage ( const std::string& what_ );
  };

  class MagickDLLDecl WarningMissingDelegate : public Warning
  {
  public:
    explicit WarningMissingDelegate ( const std::string& what_ );
  };

  class MagickDLLDecl WarningModule : public Warning
  {
  public:
    explicit WarningModule ( const std::string& what_ );
  };

  class MagickDLLDecl WarningMonitor : public Warning
  {
  public:
    explicit WarningMonitor ( const std::string& what_ );
  };

  class MagickDLLDecl WarningOption : public Warning
  {
  public:
    explicit WarningOption ( const std::string& what_ );
  };

  class MagickDLLDecl WarningRegistry : public Warning
  {
  public:
    explicit WarningRegistry ( const std::string& what_ );
  };

  class MagickDLLDecl WarningResourceLimit : public Warning
  {
  public:
    explicit WarningResourceLimit ( const std::string& what_ );
  };

  class MagickDLLDecl WarningStream : public Warning
  {
  public:
    explicit WarningStream ( const std::string& what_ );
  };

  class MagickDLLDecl WarningType : public Warning
  {
  public:
    explicit WarningType ( const std::string& what_ );
  };

  class MagickDLLDecl WarningXServer : public Warning
  {
  public:
   explicit WarningXServer ( const std::string& what_ );
  };

  //
  // Error exceptions
  //

  class MagickDLLDecl Error : public Exception
  {
  public:
    explicit Error ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorUndefined : public Error
  {
  public:
    explicit ErrorUndefined ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorBlob: public Error
  {
  public:
    explicit ErrorBlob ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorCache: public Error
  {
  public:
    explicit ErrorCache ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorCoder: public Error
  {
  public:
    explicit ErrorCoder ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorConfigure: public Error
  {
  public:
    explicit ErrorConfigure ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorCorruptImage: public Error
  {
  public:
    explicit ErrorCorruptImage ( const std::string& what_ );
  };
  
  class MagickDLLDecl ErrorDelegate : public Error
  {
  public:
    explicit ErrorDelegate ( const std::string& what_ );
  };
  
  class MagickDLLDecl ErrorDraw : public Error
  {
  public:
    explicit ErrorDraw ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorFileOpen: public Error
  {
  public:
    explicit ErrorFileOpen ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorImage: public Error
  {
  public:
    explicit ErrorImage ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorMissingDelegate : public Error
  {
  public:
    explicit ErrorMissingDelegate ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorModule : public Error
  {
  public:
    explicit ErrorModule ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorMonitor : public Error
  {
  public:
    explicit ErrorMonitor ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorOption : public Error
  {
  public:
    explicit ErrorOption ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorRegistry : public Error
  {
  public:
    explicit ErrorRegistry ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorResourceLimit : public Error
  {
  public:
    explicit ErrorResourceLimit ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorStream : public Error
  {
  public:
    explicit ErrorStream ( const std::string& what_ );
  };

  class MagickDLLDecl ErrorType : public Error
  {
  public:
    explicit ErrorType ( const std::string& what_ );
  };
  
  class MagickDLLDecl ErrorXServer : public Error
  {
  public:
    explicit ErrorXServer ( const std::string& what_ );
  };

  //
  // No user-serviceable components beyond this point.
  //

  // Throw exception based on raw data
  MagickDLLDeclExtern void throwExceptionExplicit( const MagickCore::ExceptionType severity_,
                                                   const char* reason_,
                                                   const char* description_ = 0 );

  // Thow exception based on ImageMagick's ExceptionInfo
  MagickDLLDeclExtern void throwException( MagickCore::ExceptionInfo &exception_ );

} // namespace Magick

#endif // Magick_Exception_header
