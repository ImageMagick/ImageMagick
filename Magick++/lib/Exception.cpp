// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2014-2017
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
    _what(what_),
    _nested((Exception *) NULL)
{
}

Magick::Exception::Exception(const std::string& what_,
  Exception* nested_)
    : std::exception(),
    _what(what_),
    _nested(nested_)
{
}

Magick::Exception::Exception(const Magick::Exception& original_)
  : exception(original_),
    _what(original_._what),
    _nested((Exception *) NULL)
{
}

Magick::Exception::~Exception() throw()
{
  if (_nested != (Exception *) NULL)
    delete _nested;
}

Magick::Exception& Magick::Exception::operator=(
  const Magick::Exception& original_)
{
  if (this != &original_)
    this->_what=original_._what;
  return(*this);
}

const char* Magick::Exception::what() const throw()
{
  return(_what.c_str());
}

const Magick::Exception* Magick::Exception::nested() const throw()
{
  return(_nested);
}

void Magick::Exception::nested(Exception* nested_) throw()
{
  _nested=nested_;
}

Magick::Error::Error(const std::string& what_)
  : Exception(what_)
{
}

Magick::Error::Error(const std::string& what_,Exception *nested_)
  : Exception(what_,nested_)
{
}

Magick::Error::~Error() throw()
{
}

Magick::ErrorBlob::ErrorBlob(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorBlob::ErrorBlob(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorBlob::~ErrorBlob() throw()
{
}

Magick::ErrorCache::ErrorCache(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorCache::ErrorCache(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorCache::~ErrorCache() throw()
{
}

Magick::ErrorCoder::ErrorCoder(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorCoder::ErrorCoder(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorCoder::~ErrorCoder() throw()
{
}

Magick::ErrorConfigure::ErrorConfigure(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorConfigure::ErrorConfigure(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorConfigure::~ErrorConfigure() throw()
{
}

Magick::ErrorCorruptImage::ErrorCorruptImage(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorCorruptImage::ErrorCorruptImage(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorCorruptImage::~ErrorCorruptImage() throw()
{
}

Magick::ErrorDelegate::ErrorDelegate(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorDelegate::ErrorDelegate(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorDelegate::~ErrorDelegate()throw()
{
}

Magick::ErrorDraw::ErrorDraw(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorDraw::ErrorDraw(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
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

Magick::ErrorFileOpen::ErrorFileOpen(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}


Magick::ErrorImage::ErrorImage(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorImage::ErrorImage(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorImage::~ErrorImage() throw()
{
}

Magick::ErrorMissingDelegate::ErrorMissingDelegate(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorMissingDelegate::ErrorMissingDelegate(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorMissingDelegate::~ErrorMissingDelegate() throw ()
{
}

Magick::ErrorModule::ErrorModule(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorModule::ErrorModule(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorModule::~ErrorModule() throw()
{
}

Magick::ErrorMonitor::ErrorMonitor(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorMonitor::ErrorMonitor(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorMonitor::~ErrorMonitor() throw()
{
}

Magick::ErrorOption::ErrorOption(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorOption::ErrorOption(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorOption::~ErrorOption() throw()
{
}

Magick::ErrorPolicy::ErrorPolicy(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorPolicy::ErrorPolicy(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorPolicy::~ErrorPolicy() throw()
{
}


Magick::ErrorRegistry::ErrorRegistry(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorRegistry::ErrorRegistry(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorRegistry::~ErrorRegistry() throw()
{
}

Magick::ErrorResourceLimit::ErrorResourceLimit(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorResourceLimit::ErrorResourceLimit(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorResourceLimit::~ErrorResourceLimit() throw()
{
}

Magick::ErrorStream::ErrorStream(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorStream::ErrorStream(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorStream::~ErrorStream() throw()
{
}

Magick::ErrorType::ErrorType(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorType::ErrorType(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorType::~ErrorType() throw()
{
}

Magick::ErrorUndefined::ErrorUndefined(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorUndefined::ErrorUndefined(const std::string& what_,
  Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorUndefined::~ErrorUndefined() throw()
{
}

Magick::ErrorXServer::ErrorXServer(const std::string& what_)
  : Error(what_)
{
}

Magick::ErrorXServer::ErrorXServer(const std::string& what_,Exception *nested_)
  : Error(what_,nested_)
{
}

Magick::ErrorXServer::~ErrorXServer() throw ()
{
}

Magick::Warning::Warning(const std::string& what_)
  : Exception(what_)
{
}

Magick::Warning::Warning(const std::string& what_,Exception *nested_)
  : Exception(what_,nested_)
{
}

Magick::Warning::~Warning() throw()
{
}

Magick::WarningBlob::WarningBlob(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningBlob::WarningBlob(const std::string& what_,Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningBlob::~WarningBlob() throw()
{
}

Magick::WarningCache::WarningCache(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningCache::WarningCache(const std::string& what_,Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningCache::~WarningCache() throw()
{
}

Magick::WarningCoder::WarningCoder(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningCoder::WarningCoder(const std::string& what_,Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningCoder::~WarningCoder() throw()
{
}

Magick::WarningConfigure::WarningConfigure(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningConfigure::WarningConfigure(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningConfigure::~WarningConfigure() throw()
{
}

Magick::WarningCorruptImage::WarningCorruptImage(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningCorruptImage::WarningCorruptImage(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningCorruptImage::~WarningCorruptImage() throw()
{
}

Magick::WarningDelegate::WarningDelegate(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningDelegate::WarningDelegate(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningDelegate::~WarningDelegate() throw()
{
}

Magick::WarningDraw::WarningDraw(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningDraw::WarningDraw(const std::string& what_,Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningDraw::~WarningDraw() throw()
{
}

Magick::WarningFileOpen::WarningFileOpen(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningFileOpen::WarningFileOpen(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningFileOpen::~WarningFileOpen() throw()
{
}

Magick::WarningImage::WarningImage(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningImage::WarningImage(const std::string& what_,Exception *nested_)
  : Warning(what_,nested_)
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

Magick::WarningMissingDelegate::WarningMissingDelegate(
  const std::string& what_,Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningMissingDelegate::~WarningMissingDelegate() throw()
{
}

Magick::WarningModule::WarningModule(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningModule::WarningModule(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}


Magick::WarningModule::~WarningModule() throw()
{
}

Magick::WarningMonitor::WarningMonitor(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningMonitor::WarningMonitor(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningMonitor::~WarningMonitor() throw()
{
}

Magick::WarningOption::WarningOption(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningOption::WarningOption(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningOption::~WarningOption() throw()
{
}

Magick::WarningRegistry::WarningRegistry(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningRegistry::WarningRegistry(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningRegistry::~WarningRegistry() throw()
{
}

Magick::WarningPolicy::WarningPolicy(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningPolicy::WarningPolicy(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningPolicy::~WarningPolicy() throw()
{
}

Magick::WarningResourceLimit::WarningResourceLimit(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningResourceLimit::WarningResourceLimit(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningResourceLimit::~WarningResourceLimit() throw()
{
}

Magick::WarningStream::WarningStream(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningStream::WarningStream(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningStream::~WarningStream() throw()
{
}

Magick::WarningType::WarningType(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningType::WarningType(const std::string& what_,Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningType::~WarningType() throw()
{
}

Magick::WarningUndefined::WarningUndefined(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningUndefined::WarningUndefined(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningUndefined::~WarningUndefined() throw()
{
}

Magick::WarningXServer::WarningXServer(const std::string& what_)
  : Warning(what_)
{
}

Magick::WarningXServer::WarningXServer(const std::string& what_,
  Exception *nested_)
  : Warning(what_,nested_)
{
}

Magick::WarningXServer::~WarningXServer() throw()
{
}

std::string Magick::formatExceptionMessage(const MagickCore::ExceptionInfo *exception_)
{
  // Format error message ImageMagick-style
  std::string message=GetClientName();
  if (exception_->reason != (char *) NULL)
    {
      message+=std::string(": ");
      message+=std::string(exception_->reason);
    }

  if (exception_->description != (char *) NULL)
    message += " (" + std::string(exception_->description) + ")";
  return(message);
}

Magick::Exception* Magick::createException(const MagickCore::ExceptionInfo *exception_)
{
  std::string message=formatExceptionMessage(exception_);
  switch (exception_->severity)
  {
    case BlobError:
    case BlobFatalError:
      return new ErrorBlob(message);
    case BlobWarning:
      return new WarningBlob(message);
    case CacheError:
    case CacheFatalError:
      return new ErrorCache(message);
    case CacheWarning:
      return new WarningCache(message);
    case CoderError:
    case CoderFatalError:
      return new ErrorCoder(message);
    case CoderWarning:
      return new WarningCoder(message);
    case ConfigureError:
    case ConfigureFatalError:
      return new ErrorConfigure(message);
    case ConfigureWarning:
      return new WarningConfigure(message);
    case CorruptImageError:
    case CorruptImageFatalError:
      return new ErrorCorruptImage(message);
    case CorruptImageWarning:
      return new WarningCorruptImage(message);
    case DelegateError:
    case DelegateFatalError:
      return new ErrorDelegate(message);
    case DelegateWarning:
      return new WarningDelegate(message);
    case DrawError:
    case DrawFatalError:
      return new ErrorDraw(message);
    case DrawWarning:
      return new WarningDraw(message);
    case FileOpenError:
    case FileOpenFatalError:
      return new ErrorFileOpen(message);
    case FileOpenWarning:
      return new WarningFileOpen(message);
    case ImageError:
    case ImageFatalError:
      return new ErrorImage(message);
    case ImageWarning:
      return new WarningImage(message);
    case MissingDelegateError:
    case MissingDelegateFatalError:
      return new ErrorMissingDelegate(message);
    case MissingDelegateWarning:
      return new WarningMissingDelegate(message);
    case ModuleError:
    case ModuleFatalError:
      return new ErrorModule(message);
    case ModuleWarning:
      return new WarningModule(message);
    case MonitorError:
    case MonitorFatalError:
      return new ErrorMonitor(message);
    case MonitorWarning:
      return new WarningMonitor(message);
    case OptionError:
    case OptionFatalError:
      return new ErrorOption(message);
    case OptionWarning:
      return new WarningOption(message);
    case PolicyWarning:
      return new WarningPolicy(message);
    case PolicyError:
    case PolicyFatalError:
      return new ErrorPolicy(message);
    case RegistryError:
    case RegistryFatalError:
      return new ErrorRegistry(message);
    case RegistryWarning:
      return new WarningRegistry(message);
    case ResourceLimitError:
    case ResourceLimitFatalError:
      return new ErrorResourceLimit(message);
    case ResourceLimitWarning:
      return new WarningResourceLimit(message);
    case StreamError:
    case StreamFatalError:
      return new ErrorStream(message);
    case StreamWarning:
      return new WarningStream(message);
    case TypeError:
    case TypeFatalError:
      return new ErrorType(message);
    case TypeWarning:
      return new WarningType(message);
    case UndefinedException:
    default:
      return new ErrorUndefined(message);
    case XServerError:
    case XServerFatalError:
      return new ErrorXServer(message);
    case XServerWarning:
      return new WarningXServer(message);
    }
}

MagickPPExport void Magick::throwExceptionExplicit(
  const ExceptionType severity_,const char* reason_,const char* description_)
{
  // Just return if there is no reported error
  if (severity_ == UndefinedException)
    return;

  GetPPException;
  ThrowException(exceptionInfo,severity_,reason_, description_);
  ThrowPPException(false);
}

MagickPPExport void Magick::throwException(ExceptionInfo *exception_,
  const bool quiet_)
{
  const ExceptionInfo
    *p;

  Exception
    *nestedException,
    *q;

  ExceptionType
    severity;

  size_t
    index;

  // Just return if there is no reported error
  if (exception_->severity == UndefinedException)
    return;

  std::string message=formatExceptionMessage(exception_);
  nestedException=(Exception *) NULL;
  q=(Exception *) NULL;
  LockSemaphoreInfo(exception_->semaphore);
  if (exception_->exceptions != (void *) NULL)
    {
      index=GetNumberOfElementsInLinkedList((LinkedListInfo *)
        exception_->exceptions);
      while(index > 0)
      {
        p=(const ExceptionInfo *) GetValueFromLinkedList((LinkedListInfo *)
          exception_->exceptions,--index);
        if ((p->severity != exception_->severity) || (LocaleCompare(p->reason,
            exception_->reason) != 0) || (LocaleCompare(p->description,
            exception_->description) != 0))
          {
            if (nestedException == (Exception *) NULL)
              {
                nestedException=createException(p);
                q=nestedException;
              }
            else
              {
                Exception
                  *r;

                r=createException(p);
                q->nested(r);
                q=r;
              }
          }
      }
    }
  severity=exception_->severity;
  UnlockSemaphoreInfo(exception_->semaphore);

  if ((quiet_) && (severity < MagickCore::ErrorException))
    {
      delete nestedException;
      return;
    }

  DestroyExceptionInfo(exception_);

  switch (severity)
  {
    case BlobError:
    case BlobFatalError:
      throw ErrorBlob(message,nestedException);
    case BlobWarning:
      throw WarningBlob(message,nestedException);
    case CacheError:
    case CacheFatalError:
      throw ErrorCache(message,nestedException);
    case CacheWarning:
      throw WarningCache(message,nestedException);
    case CoderError:
    case CoderFatalError:
      throw ErrorCoder(message,nestedException);
    case CoderWarning:
      throw WarningCoder(message,nestedException);
    case ConfigureError:
    case ConfigureFatalError:
      throw ErrorConfigure(message,nestedException);
    case ConfigureWarning:
      throw WarningConfigure(message,nestedException);
    case CorruptImageError:
    case CorruptImageFatalError:
      throw ErrorCorruptImage(message,nestedException);
    case CorruptImageWarning:
      throw WarningCorruptImage(message,nestedException);
    case DelegateError:
    case DelegateFatalError:
      throw ErrorDelegate(message,nestedException);
    case DelegateWarning:
      throw WarningDelegate(message,nestedException);
    case DrawError:
    case DrawFatalError:
      throw ErrorDraw(message,nestedException);
    case DrawWarning:
      throw WarningDraw(message,nestedException);
    case FileOpenError:
    case FileOpenFatalError:
      throw ErrorFileOpen(message,nestedException);
    case FileOpenWarning:
      throw WarningFileOpen(message,nestedException);
    case ImageError:
    case ImageFatalError:
      throw ErrorImage(message,nestedException);
    case ImageWarning:
      throw WarningImage(message,nestedException);
    case MissingDelegateError:
    case MissingDelegateFatalError:
      throw ErrorMissingDelegate(message,nestedException);
    case MissingDelegateWarning:
      throw WarningMissingDelegate(message,nestedException);
    case ModuleError:
    case ModuleFatalError:
      throw ErrorModule(message,nestedException);
    case ModuleWarning:
      throw WarningModule(message,nestedException);
    case MonitorError:
    case MonitorFatalError:
      throw ErrorMonitor(message,nestedException);
    case MonitorWarning:
      throw WarningMonitor(message,nestedException);
    case OptionError:
    case OptionFatalError:
      throw ErrorOption(message,nestedException);
    case OptionWarning:
      throw WarningOption(message,nestedException);
    case PolicyWarning:
      throw WarningPolicy(message,nestedException);
    case PolicyError:
    case PolicyFatalError:
      throw ErrorPolicy(message,nestedException);
    case RegistryError:
    case RegistryFatalError:
      throw ErrorRegistry(message,nestedException);
    case RegistryWarning:
      throw WarningRegistry(message,nestedException);
    case ResourceLimitError:
    case ResourceLimitFatalError:
      throw ErrorResourceLimit(message,nestedException);
    case ResourceLimitWarning:
      throw WarningResourceLimit(message,nestedException);
    case StreamError:
    case StreamFatalError:
      throw ErrorStream(message,nestedException);
    case StreamWarning:
      throw WarningStream(message,nestedException);
    case TypeError:
    case TypeFatalError:
      throw ErrorType(message,nestedException);
    case TypeWarning:
      throw WarningType(message,nestedException);
    case UndefinedException:
    default:
      throw ErrorUndefined(message,nestedException);
    case XServerError:
    case XServerFatalError:
      throw ErrorXServer(message,nestedException);
    case XServerWarning:
      throw WarningXServer(message,nestedException);
    }
}
