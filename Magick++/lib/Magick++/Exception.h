// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2014-2015
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
  class MagickPPExport Exception: public std::exception
  {
  public:

    // Construct with message string
    Exception(const std::string& what_);

    // Construct with message string and nested exception
    Exception(const std::string& what_, Exception* nested_);

    // Copy constructor
    Exception(const Exception& original_);

    // Destructor
    virtual ~Exception() throw();

    // Assignment operator
    Exception& operator=(const Exception& original_);

    // Get string identifying exception
    virtual const char* what() const throw();

    // Get nested exception
    const Exception* nested() const throw();

    //////////////////////////////////////////////////////////////////////
    //
    // No user-serviceable parts beyond this point
    //
    //////////////////////////////////////////////////////////////////////

    void nested(Exception* nested_) throw();

  private:
    std::string _what;
    Exception* _nested;
  };

  //
  // Error exceptions
  //

  class MagickPPExport Error: public Exception
  {
  public:
    explicit Error(const std::string& what_);
    explicit Error(const std::string& what_,Exception *nested_);
    ~Error() throw();
  };

  class MagickPPExport ErrorBlob: public Error
  {
  public:
    explicit ErrorBlob(const std::string& what_);
    explicit ErrorBlob(const std::string& what_,Exception *nested_);
    ~ErrorBlob() throw();
  };

  class MagickPPExport ErrorCache: public Error
  {
  public:
    explicit ErrorCache(const std::string& what_);
    explicit ErrorCache(const std::string& what_,Exception *nested_);
    ~ErrorCache() throw();
  };

  class MagickPPExport ErrorCoder: public Error
  {
  public:
    explicit ErrorCoder(const std::string& what_);
    explicit ErrorCoder(const std::string& what_,Exception *nested_);
    ~ErrorCoder() throw();
  };

  class MagickPPExport ErrorConfigure: public Error
  {
  public:
    explicit ErrorConfigure(const std::string& what_);
    explicit ErrorConfigure(const std::string& what_,Exception *nested_);
    ~ErrorConfigure() throw();
  };

  class MagickPPExport ErrorCorruptImage: public Error
  {
  public:
    explicit ErrorCorruptImage(const std::string& what_);
    explicit ErrorCorruptImage(const std::string& what_,Exception *nested_);
    ~ErrorCorruptImage() throw();
  };
  
  class MagickPPExport ErrorDelegate: public Error
  {
  public:
    explicit ErrorDelegate(const std::string& what_);
    explicit ErrorDelegate(const std::string& what_,Exception *nested_);
    ~ErrorDelegate() throw();
  };
  
  class MagickPPExport ErrorDraw: public Error
  {
  public:
    explicit ErrorDraw(const std::string& what_);
    explicit ErrorDraw(const std::string& what_,Exception *nested_);
    ~ErrorDraw() throw();
  };

  class MagickPPExport ErrorFileOpen: public Error
  {
  public:
    explicit ErrorFileOpen(const std::string& what_);
    explicit ErrorFileOpen(const std::string& what_,Exception *nested_);
    ~ErrorFileOpen() throw();
  };

  class MagickPPExport ErrorImage: public Error
  {
  public:
    explicit ErrorImage(const std::string& what_);
    explicit ErrorImage(const std::string& what_,Exception *nested_);
    ~ErrorImage() throw();
  };

  class MagickPPExport ErrorMissingDelegate: public Error
  {
  public:
    explicit ErrorMissingDelegate(const std::string& what_);
    explicit ErrorMissingDelegate(const std::string& what_,Exception *nested_);
    ~ErrorMissingDelegate() throw();
  };

  class MagickPPExport ErrorModule: public Error
  {
  public:
    explicit ErrorModule(const std::string& what_);
    explicit ErrorModule(const std::string& what_,Exception *nested_);
    ~ErrorModule() throw();
  };

  class MagickPPExport ErrorMonitor: public Error
  {
  public:
    explicit ErrorMonitor(const std::string& what_);
    explicit ErrorMonitor(const std::string& what_,Exception *nested_);
    ~ErrorMonitor() throw();
  };

  class MagickPPExport ErrorOption: public Error
  {
  public:
    explicit ErrorOption(const std::string& what_);
    explicit ErrorOption(const std::string& what_,Exception *nested_);
    ~ErrorOption() throw();
  };

  class MagickPPExport ErrorPolicy: public Error
  {
  public:
    explicit ErrorPolicy(const std::string& what_);
    explicit ErrorPolicy(const std::string& what_,Exception *nested_);
    ~ErrorPolicy() throw();
  };

  class MagickPPExport ErrorRegistry: public Error
  {
  public:
    explicit ErrorRegistry(const std::string& what_);
    explicit ErrorRegistry(const std::string& what_,Exception *nested_);
    ~ErrorRegistry() throw();
  };

  class MagickPPExport ErrorResourceLimit: public Error
  {
  public:
    explicit ErrorResourceLimit(const std::string& what_);
    explicit ErrorResourceLimit(const std::string& what_,Exception *nested_);
    ~ErrorResourceLimit() throw();
  };

  class MagickPPExport ErrorStream: public Error
  {
  public:
    explicit ErrorStream(const std::string& what_);
    explicit ErrorStream(const std::string& what_,Exception *nested_);
    ~ErrorStream() throw();
  };

  class MagickPPExport ErrorType: public Error
  {
  public:
    explicit ErrorType(const std::string& what_);
    explicit ErrorType(const std::string& what_,Exception *nested_);
    ~ErrorType() throw();
  };

  class MagickPPExport ErrorUndefined: public Error
  {
  public:
    explicit ErrorUndefined(const std::string& what_);
    explicit ErrorUndefined(const std::string& what_,Exception *nested_);
    ~ErrorUndefined() throw();
  };
  
  class MagickPPExport ErrorXServer: public Error
  {
  public:
    explicit ErrorXServer(const std::string& what_);
    explicit ErrorXServer(const std::string& what_,Exception *nested_);
    ~ErrorXServer() throw();
  };

  //
  // Warnings
  //

  class MagickPPExport Warning: public Exception
  {
  public:
    explicit Warning(const std::string& what_);
    explicit Warning(const std::string& what_,Exception *nested_);
    ~Warning() throw();
  };

  class MagickPPExport WarningBlob: public Warning
  {
  public:
    explicit WarningBlob(const std::string& what_);
    explicit WarningBlob(const std::string& what_,Exception *nested_);
    ~WarningBlob() throw();
  };

  class MagickPPExport WarningCache: public Warning
  {
  public:
    explicit WarningCache(const std::string& what_);
    explicit WarningCache(const std::string& what_,Exception *nested_);
    ~WarningCache() throw();
  };

  class MagickPPExport WarningCoder: public Warning
  {
  public:
    explicit WarningCoder(const std::string& what_);
    explicit WarningCoder(const std::string& what_,Exception *nested_);
    ~WarningCoder() throw();
  };

  class MagickPPExport WarningConfigure: public Warning
  {
  public:
    explicit WarningConfigure(const std::string& what_);
    explicit WarningConfigure(const std::string& what_,Exception *nested_);
    ~WarningConfigure() throw();
  };

  class MagickPPExport WarningCorruptImage: public Warning
  {
  public:
    explicit WarningCorruptImage(const std::string& what_);
    explicit WarningCorruptImage(const std::string& what_,Exception *nested_);
    ~WarningCorruptImage() throw();
  };

  class MagickPPExport WarningDelegate: public Warning
  {
  public:
    explicit WarningDelegate(const std::string& what_);
    explicit WarningDelegate(const std::string& what_,Exception *nested_);
    ~WarningDelegate() throw();
  };

  class MagickPPExport WarningDraw : public Warning
  {
  public:
    explicit WarningDraw(const std::string& what_);
    explicit WarningDraw(const std::string& what_,Exception *nested_);
    ~WarningDraw() throw();
  };

  class MagickPPExport WarningFileOpen: public Warning
  {
  public:
    explicit WarningFileOpen(const std::string& what_);
    explicit WarningFileOpen(const std::string& what_,Exception *nested_);
    ~WarningFileOpen() throw();
  };

  class MagickPPExport WarningImage: public Warning
  {
  public:
    explicit WarningImage(const std::string& what_);
    explicit WarningImage(const std::string& what_,Exception *nested_);
    ~WarningImage() throw();
  };

  class MagickPPExport WarningMissingDelegate: public Warning
  {
  public:
    explicit WarningMissingDelegate(const std::string& what_);
    explicit WarningMissingDelegate(const std::string& what_,
      Exception *nested_);
    ~WarningMissingDelegate() throw();
  };

  class MagickPPExport WarningModule: public Warning
  {
  public:
    explicit WarningModule(const std::string& what_);
    explicit WarningModule(const std::string& what_,Exception *nested_);
    ~WarningModule() throw();
  };

  class MagickPPExport WarningMonitor: public Warning
  {
  public:
    explicit WarningMonitor(const std::string& what_);
    explicit WarningMonitor(const std::string& what_,Exception *nested_);
    ~WarningMonitor() throw();
  };

  class MagickPPExport WarningOption: public Warning
  {
  public:
    explicit WarningOption(const std::string& what_);
    explicit WarningOption(const std::string& what_,Exception *nested_);
    ~WarningOption() throw();
  };

  class MagickPPExport WarningPolicy: public Warning
  {
  public:
    explicit WarningPolicy(const std::string& what_);
    explicit WarningPolicy(const std::string& what_,Exception *nested_);
    ~WarningPolicy() throw();
  };

  class MagickPPExport WarningRegistry: public Warning
  {
  public:
    explicit WarningRegistry(const std::string& what_);
    explicit WarningRegistry(const std::string& what_,Exception *nested_);
    ~WarningRegistry() throw();
  };

  class MagickPPExport WarningResourceLimit: public Warning
  {
  public:
    explicit WarningResourceLimit(const std::string& what_);
    explicit WarningResourceLimit(const std::string& what_,Exception *nested_);
    ~WarningResourceLimit() throw();
  };

  class MagickPPExport WarningStream: public Warning
  {
  public:
    explicit WarningStream(const std::string& what_);
    explicit WarningStream(const std::string& what_,Exception *nested_);
    ~WarningStream() throw();
  };

  class MagickPPExport WarningType: public Warning
  {
  public:
    explicit WarningType(const std::string& what_);
    explicit WarningType(const std::string& what_,Exception *nested_);
    ~WarningType() throw();
  };

  class MagickPPExport WarningUndefined: public Warning
  {
  public:
    explicit WarningUndefined(const std::string& what_);
    explicit WarningUndefined(const std::string& what_,Exception *nested_);
    ~WarningUndefined() throw();
  };

  class MagickPPExport WarningXServer: public Warning
  {
  public:
    explicit WarningXServer(const std::string& what_);
    explicit WarningXServer(const std::string& what_,Exception *nested_);
    ~WarningXServer() throw();
  };

  //
  // No user-serviceable components beyond this point.
  //

  std::string formatExceptionMessage(
    const MagickCore::ExceptionInfo *exception_);

  Exception* createException(const MagickCore::ExceptionInfo *exception_);

  // Throw exception based on raw data
  extern MagickPPExport void throwExceptionExplicit(
    const MagickCore::ExceptionType severity_,const char* reason_,
    const char* description_=(char *) NULL);

  // Thow exception based on ImageMagick's ExceptionInfo
  extern MagickPPExport void throwException(
    MagickCore::ExceptionInfo *exception_,const bool quiet_=false);

} // namespace Magick

#endif // Magick_Exception_header
