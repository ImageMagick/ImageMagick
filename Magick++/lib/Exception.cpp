// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Implementation of Exception and derived classes
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string>
#include <errno.h>
#include <string.h>

using namespace std;

#include "Magick++/Exception.h"

// Construct with message string
Magick::Exception::Exception( const std::string& what_ )
  : std::exception(),
    _what(what_)
{
}

// Copy constructor
Magick::Exception::Exception( const Magick::Exception& original_ )
  : exception(original_),
    _what(original_._what)
{
}

// Assignment operator
Magick::Exception& Magick::Exception::operator= (const Magick::Exception& original_ )
{
  if(this != &original_)
    {
      this->_what = original_._what;
    }
  return *this;
}

// Return message string
/*virtual*/ const char* Magick::Exception::what( ) const throw()
{
  return _what.c_str();
}

/* Destructor */
/*virtual*/ Magick::Exception::~Exception ( ) throw ()
{
}

//
// Warnings
//

Magick::Warning::Warning ( const std::string& what_ )
  : Exception(what_)
{
}

/*virtual*/ Magick::Warning::~Warning ( ) throw ()
{
}

Magick::WarningUndefined::WarningUndefined ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningUndefined::~WarningUndefined ( ) throw ()
{
}

Magick::WarningBlob::WarningBlob ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningBlob::~WarningBlob ( ) throw ()
{
}

Magick::WarningCache::WarningCache ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningCache::~WarningCache ( ) throw ()
{
}

Magick::WarningCoder::WarningCoder ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningCoder::~WarningCoder ( ) throw ()
{
}

Magick::WarningConfigure::WarningConfigure ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningConfigure::~WarningConfigure ( ) throw ()
{
}

Magick::WarningCorruptImage::WarningCorruptImage ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningCorruptImage::~WarningCorruptImage ( ) throw ()
{
}

Magick::WarningDelegate::WarningDelegate ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningDelegate::~WarningDelegate ( ) throw ()
{
}

Magick::WarningDraw::WarningDraw ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningDraw::~WarningDraw ( ) throw ()
{
}

Magick::WarningFileOpen::WarningFileOpen ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningFileOpen::~WarningFileOpen ( ) throw ()
{
}

Magick::WarningImage::WarningImage ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningImage::~WarningImage ( ) throw ()
{
}

Magick::WarningMissingDelegate::WarningMissingDelegate ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningMissingDelegate::~WarningMissingDelegate ( ) throw ()
{
}

Magick::WarningModule::WarningModule ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningModule::~WarningModule ( ) throw ()
{
}

Magick::WarningMonitor::WarningMonitor ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningMonitor::~WarningMonitor ( ) throw ()
{
}

Magick::WarningOption::WarningOption ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningOption::~WarningOption ( ) throw ()
{
}

Magick::WarningRegistry::WarningRegistry ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningRegistry::~WarningRegistry ( ) throw ()
{
}

Magick::WarningResourceLimit::WarningResourceLimit ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningResourceLimit::~WarningResourceLimit ( ) throw ()
{
}

Magick::WarningStream::WarningStream ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningStream::~WarningStream ( ) throw ()
{
}

Magick::WarningType::WarningType ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningType::~WarningType ( ) throw ()
{
}

Magick::WarningXServer::WarningXServer ( const std::string& what_ )
  : Warning(what_)
{
}

/*virtual*/ Magick::WarningXServer::~WarningXServer ( ) throw ()
{
}

//
// Errors
//

Magick::Error::Error ( const std::string& what_ )
  : Exception(what_)
{
}

/*virtual*/ Magick::Error::~Error ( ) throw ()
{
}

Magick::ErrorUndefined::ErrorUndefined ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorUndefined::~ErrorUndefined ( ) throw ()
{
}

Magick::ErrorBlob::ErrorBlob ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorBlob::~ErrorBlob ( ) throw ()
{
}

Magick::ErrorCache::ErrorCache ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorCache::~ErrorCache ( ) throw ()
{
}

Magick::ErrorCoder::ErrorCoder ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorCoder::~ErrorCoder ( ) throw ()
{
}

Magick::ErrorConfigure::ErrorConfigure ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorConfigure::~ErrorConfigure ( ) throw ()
{
}

Magick::ErrorCorruptImage::ErrorCorruptImage ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorCorruptImage::~ErrorCorruptImage ( ) throw ()
{
}

Magick::ErrorDelegate::ErrorDelegate ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorDelegate::~ErrorDelegate ( ) throw ()
{
}

Magick::ErrorDraw::ErrorDraw ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorDraw::~ErrorDraw ( ) throw ()
{
}

Magick::ErrorFileOpen::ErrorFileOpen ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorFileOpen::~ErrorFileOpen ( ) throw ()
{
}

Magick::ErrorImage::ErrorImage ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorImage::~ErrorImage ( ) throw ()
{
}

Magick::ErrorMissingDelegate::ErrorMissingDelegate ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorMissingDelegate::~ErrorMissingDelegate ( ) throw ()
{
}

Magick::ErrorModule::ErrorModule ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorModule::~ErrorModule ( ) throw ()
{
}

Magick::ErrorMonitor::ErrorMonitor ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorMonitor::~ErrorMonitor ( ) throw ()
{
}

Magick::ErrorOption::ErrorOption ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorOption::~ErrorOption ( ) throw ()
{
}

Magick::ErrorRegistry::ErrorRegistry ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorRegistry::~ErrorRegistry ( ) throw ()
{
}

Magick::ErrorResourceLimit::ErrorResourceLimit ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorResourceLimit::~ErrorResourceLimit ( ) throw ()
{
}

Magick::ErrorStream::ErrorStream ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorStream::~ErrorStream ( ) throw ()
{
}

Magick::ErrorType::ErrorType ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorType::~ErrorType ( ) throw ()
{
}

Magick::ErrorXServer::ErrorXServer ( const std::string& what_ )
  : Error(what_)
{
}

/*virtual*/ Magick::ErrorXServer::~ErrorXServer ( ) throw ()
{
}

// Format and throw exception
MagickPPExport void Magick::throwExceptionExplicit( const ExceptionType severity_,
                                                   const char* reason_,
                                                   const char* description_)
{
  // Just return if there is no reported error
  if ( severity_ == UndefinedException )
    return;

  ExceptionInfo exception;

  GetExceptionInfo( &exception );
  ThrowException( &exception, severity_, reason_, description_ );
  throwException( exception );
  (void) DestroyExceptionInfo( &exception );
}

// Throw C++ exception
MagickPPExport void Magick::throwException( ExceptionInfo &exception_ )
{
  // Just return if there is no reported error
  if ( exception_.severity == UndefinedException )
    return;

  // Format error message ImageMagick-style
  std::string message = SetClientName(0);
  if ( exception_.reason != 0 )
    {
      message += std::string(": ");
      message += std::string(exception_.reason);
    }

  if ( exception_.description != 0 )
    message += " (" + std::string(exception_.description) + ")";

  ExceptionType severity = exception_.severity;
  MagickBooleanType relinquish = exception_.relinquish;
  DestroyExceptionInfo( &exception_ );
  if (relinquish)
    GetExceptionInfo( &exception_ );

  switch ( severity )
    {
      // Warnings
    case ResourceLimitWarning :
      throw WarningResourceLimit( message );
    case TypeWarning :
      throw WarningType( message );
    case OptionWarning :
      throw WarningOption( message );
    case DelegateWarning :
      throw WarningDelegate( message );
    case MissingDelegateWarning :
      throw WarningMissingDelegate( message );
    case CorruptImageWarning :
      throw WarningCorruptImage( message );
    case FileOpenWarning :
      throw WarningFileOpen( message );
    case BlobWarning :
      throw WarningBlob ( message );
    case StreamWarning :
      throw WarningStream ( message );
    case CacheWarning :
      throw WarningCache ( message );
    case CoderWarning :
      throw WarningCoder ( message );
    case ModuleWarning :
      throw WarningModule( message );
    case DrawWarning :
      throw WarningDraw( message );
    case ImageWarning :
      throw WarningImage( message );
    case XServerWarning :
      throw WarningXServer( message );
    case MonitorWarning :
      throw WarningMonitor( message );
    case RegistryWarning :
      throw WarningRegistry( message );
    case ConfigureWarning :
      throw WarningConfigure( message );
      // Errors
    case ResourceLimitError :
    case ResourceLimitFatalError :
      throw ErrorResourceLimit( message );
    case TypeError :
    case TypeFatalError :
      throw ErrorType( message );
    case OptionError :
    case OptionFatalError :
      throw ErrorOption( message );
    case DelegateError :
    case DelegateFatalError :
      throw ErrorDelegate( message );
    case MissingDelegateError :
    case MissingDelegateFatalError :
      throw ErrorMissingDelegate( message );
    case CorruptImageError :
    case CorruptImageFatalError :
      throw ErrorCorruptImage( message );
    case FileOpenError :
    case FileOpenFatalError :
      throw ErrorFileOpen( message );
    case BlobError :
    case BlobFatalError :
      throw ErrorBlob ( message );
    case StreamError :
    case StreamFatalError :
      throw ErrorStream ( message );
    case CacheError :
    case CacheFatalError :
      throw ErrorCache ( message );
    case CoderError :
    case CoderFatalError :
      throw ErrorCoder ( message );
    case ModuleError :
    case ModuleFatalError :
      throw ErrorModule ( message );
    case DrawError :
    case DrawFatalError :
      throw ErrorDraw ( message );
    case ImageError :
    case ImageFatalError :
      throw ErrorImage ( message );
    case XServerError :
    case XServerFatalError :
      throw ErrorXServer ( message );
    case MonitorError :
    case MonitorFatalError :
      throw ErrorMonitor ( message );
    case RegistryError :
    case RegistryFatalError :
      throw ErrorRegistry ( message );
    case ConfigureError :
    case ConfigureFatalError :
      throw ErrorConfigure ( message );
    case UndefinedException :
    default :
      throw ErrorUndefined( message );
    }

}
