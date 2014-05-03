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

Magick::Exception::Exception(const std::string& what_)
  : std::exception(),
    _what(what_)
{
}

Magick::Exception::Exception(const Magick::Exception& original_)
  : exception(original_),
    _what(original_._what)
{
}

Magick::Exception::~Exception() throw()
{
}

Magick::Exception& Magick::Exception::operator=(
  const Magick::Exception& original_)
{
  if(this != &original_)
    this->_what=original_._what;
  return(*this);
}

const char* Magick::Exception::what() const throw()
{
  return(_what.c_str());
}

Magick::Error::Error(const std::string& what_)
  : Exception(what_)
{
}

Magick::Error::~Error() throw()
{
}

Magick::ErrorBlob::ErrorBlob(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorBlob::~ErrorBlob() throw()
{
}

Magick::ErrorCache::ErrorCache(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorCache::~ErrorCache() throw()
{
}

Magick::ErrorCoder::ErrorCoder(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorCoder::~ErrorCoder() throw()
{
}

Magick::ErrorConfigure::ErrorConfigure(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorConfigure::~ErrorConfigure() throw()
{
}

Magick::ErrorCorruptImage::ErrorCorruptImage(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorCorruptImage::~ErrorCorruptImage() throw()
{
}

Magick::ErrorDelegate::ErrorDelegate(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorDelegate::~ErrorDelegate()throw()
{
}

Magick::ErrorDraw::ErrorDraw(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorDraw::~ErrorDraw() throw()
{
}

Magick::ErrorFileOpen::ErrorFileOpen(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorFileOpen::~ErrorFileOpen() throw()
{
}

Magick::ErrorImage::ErrorImage(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorImage::~ErrorImage() throw()
{
}

Magick::ErrorMissingDelegate::ErrorMissingDelegate(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorMissingDelegate::~ErrorMissingDelegate() throw ()
{
}

Magick::ErrorModule::ErrorModule(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorModule::~ErrorModule() throw()
{
}

Magick::ErrorMonitor::ErrorMonitor(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorMonitor::~ErrorMonitor() throw()
{
}

Magick::ErrorOption::ErrorOption(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorOption::~ErrorOption() throw()
{
}

Magick::ErrorPolicy::~ErrorPolicy() throw()
{
}

Magick::ErrorPolicy::ErrorPolicy(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorRegistry::ErrorRegistry(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorRegistry::~ErrorRegistry() throw()
{
}

Magick::ErrorResourceLimit::ErrorResourceLimit(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorResourceLimit::~ErrorResourceLimit() throw()
{
}

Magick::ErrorStream::ErrorStream(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorStream::~ErrorStream() throw()
{
}

Magick::ErrorType::ErrorType(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorType::~ErrorType() throw()
{
}

Magick::ErrorUndefined::ErrorUndefined(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorUndefined::~ErrorUndefined() throw()
{
}

Magick::ErrorXServer::ErrorXServer(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorXServer::~ErrorXServer() throw ()
{
}

Magick::Warning::Warning(const std::string& what_)
  : Exception(what_)
{
}

Magick::Warning::~Warning() throw()
{
}

Magick::WarningBlob::WarningBlob(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningBlob::~WarningBlob() throw()
{
}

Magick::WarningCache::WarningCache(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningCache::~WarningCache() throw()
{
}

Magick::WarningCoder::WarningCoder(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningCoder::~WarningCoder() throw()
{
}

Magick::WarningConfigure::WarningConfigure(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningConfigure::~WarningConfigure() throw()
{
}

Magick::WarningCorruptImage::WarningCorruptImage(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningCorruptImage::~WarningCorruptImage() throw()
{
}

Magick::WarningDelegate::WarningDelegate(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningDelegate::~WarningDelegate() throw()
{
}

Magick::WarningDraw::WarningDraw(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningDraw::~WarningDraw() throw()
{
}

Magick::WarningFileOpen::WarningFileOpen(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningFileOpen::~WarningFileOpen() throw()
{
}

Magick::WarningImage::WarningImage(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningImage::~WarningImage() throw()
{
}

Magick::WarningMissingDelegate::WarningMissingDelegate(
  const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningMissingDelegate::~WarningMissingDelegate() throw()
{
}

Magick::WarningModule::WarningModule(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningModule::~WarningModule() throw()
{
}

Magick::WarningMonitor::WarningMonitor(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningMonitor::~WarningMonitor() throw()
{
}

Magick::WarningOption::WarningOption(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningPolicy::~WarningPolicy() throw()
{
}

Magick::WarningPolicy::WarningPolicy(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningOption::~WarningOption() throw()
{
}

Magick::WarningRegistry::WarningRegistry(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningRegistry::~WarningRegistry() throw()
{
}

Magick::WarningResourceLimit::WarningResourceLimit(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningResourceLimit::~WarningResourceLimit() throw()
{
}

Magick::WarningStream::WarningStream(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningStream::~WarningStream() throw()
{
}

Magick::WarningType::WarningType(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningType::~WarningType() throw()
{
}

Magick::WarningUndefined::WarningUndefined(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningUndefined::~WarningUndefined() throw()
{
}

Magick::WarningXServer::WarningXServer(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningXServer::~WarningXServer() throw()
{
}

MagickPPExport void Magick::throwExceptionExplicit(
  const ExceptionType severity_,const char* reason_,const char* description_)
{
  ExceptionInfo
    exception;

  // Just return if there is no reported error
  if (severity_ == UndefinedException)
    return;

  GetPPException;
  ThrowException(&exception,severity_,reason_, description_);
  ThrowPPException;
}

MagickPPExport void Magick::throwException(ExceptionInfo &exception_)
{
  ExceptionType
    severity;

  MagickBooleanType
    relinquish;

  // Just return if there is no reported error
  if (exception_.severity == UndefinedException)
    return;

  // Format error message ImageMagick-style
  std::string message = SetClientName(0);
  if (exception_.reason != (char *) NULL)
    {
      message+=std::string(": ");
      message+=std::string(exception_.reason);
    }

  if (exception_.description != (char *) NULL)
    message += " (" + std::string(exception_.description) + ")";

  severity=exception_.severity;
  relinquish=exception_.relinquish;
  DestroyExceptionInfo(&exception_);
  if (relinquish)
    GetExceptionInfo(&exception_);

  switch (severity)
  {
    case BlobError:
    case BlobFatalError:
      throw ErrorBlob(message);
    case BlobWarning:
      throw WarningBlob(message);
    case CacheError:
    case CacheFatalError:
      throw ErrorCache(message);
    case CacheWarning:
      throw WarningCache(message);
    case CoderError:
    case CoderFatalError:
      throw ErrorCoder(message);
    case CoderWarning:
      throw WarningCoder(message);
    case ConfigureError:
    case ConfigureFatalError:
      throw ErrorConfigure(message);
    case ConfigureWarning:
      throw WarningConfigure(message);
    case CorruptImageError:
    case CorruptImageFatalError:
      throw ErrorCorruptImage(message);
    case CorruptImageWarning:
      throw WarningCorruptImage(message);
    case DelegateError:
    case DelegateFatalError:
      throw ErrorDelegate(message);
    case DelegateWarning:
      throw WarningDelegate(message);
    case DrawError:
    case DrawFatalError:
      throw ErrorDraw(message);
    case DrawWarning:
      throw WarningDraw(message);
    case FileOpenError:
    case FileOpenFatalError:
      throw ErrorFileOpen(message);
    case FileOpenWarning:
      throw WarningFileOpen(message);
    case ImageError:
    case ImageFatalError:
      throw ErrorImage(message);
    case ImageWarning:
      throw WarningImage(message);
    case MissingDelegateError:
    case MissingDelegateFatalError:
      throw ErrorMissingDelegate(message);
    case MissingDelegateWarning:
      throw WarningMissingDelegate(message);
    case ModuleError:
    case ModuleFatalError:
      throw ErrorModule(message);
    case ModuleWarning:
      throw WarningModule(message);
    case MonitorError:
    case MonitorFatalError:
      throw ErrorMonitor(message);
    case MonitorWarning:
      throw WarningMonitor(message);
    case OptionError:
    case OptionFatalError:
      throw ErrorOption(message);
    case OptionWarning:
      throw WarningOption(message);
    case RegistryError:
    case RegistryFatalError:
      throw ErrorRegistry(message);
    case RegistryWarning:
      throw WarningRegistry(message);
    case ResourceLimitError:
    case ResourceLimitFatalError:
      throw ErrorResourceLimit(message);
    case ResourceLimitWarning:
      throw WarningResourceLimit(message);
    case StreamError:
    case StreamFatalError:
      throw ErrorStream(message);
    case StreamWarning:
      throw WarningStream(message);
    case TypeError:
    case TypeFatalError:
      throw ErrorType(message);
    case TypeWarning:
      throw WarningType(message);
    case UndefinedException:
    default:
      throw ErrorUndefined(message);
    case XServerError:
    case XServerFatalError:
      throw ErrorXServer(message);
    case XServerWarning:
      throw WarningXServer(message);
    }
}