/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        EEEEE  X   X   CCCC  EEEEE  PPPP  TTTTT  IIIII   OOO   N   N         %
%        E       X X   C      E      P   P   T      I    O   O  NN  N         %
%        EEE      X    C      EEE    PPPP    T      I    O   O  N N N         %
%        E       X X   C      E      P       T      I    O   O  N  NN         %
%        EEEEE   X  X   CCCC  EEEEE  P       T    IIIII   OOO   N   N         %
%                                                                             %
%                                                                             %
%                        MagickCore Exception Methods                         %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                                July 1993                                    %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/client.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

/*
  Define declarations.
*/
#define MaxExceptionList  64

/*
  Forward declarations.
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static void
  DefaultErrorHandler(const ExceptionType,const char *,const char *),
  DefaultFatalErrorHandler(const ExceptionType,const char *,const char *)
    magick_attribute((__noreturn__)),
  DefaultWarningHandler(const ExceptionType,const char *,const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*
  Global declarations.
*/
static ErrorHandler
  error_handler = DefaultErrorHandler;

static FatalErrorHandler
  fatal_error_handler = DefaultFatalErrorHandler;

static WarningHandler
  warning_handler = DefaultWarningHandler;

/*
  Static declarations.
*/
static SemaphoreInfo
  *exception_semaphore = (SemaphoreInfo *) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e E x c e p t i o n I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireExceptionInfo() allocates the ExceptionInfo structure.
%
%  The format of the AcquireExceptionInfo method is:
%
%      ExceptionInfo *AcquireExceptionInfo(void)
%
*/
MagickExport ExceptionInfo *AcquireExceptionInfo(void)
{
  ExceptionInfo
    *exception;

  exception=(ExceptionInfo *) AcquireCriticalMemory(sizeof(*exception));
  InitializeExceptionInfo(exception);
  exception->relinquish=MagickTrue;
  return(exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l e a r M a g i c k E x c e p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearMagickException() clears any exception that may not have been caught
%  yet.
%
%  The format of the ClearMagickException method is:
%
%      ClearMagickException(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
*/

static void *DestroyExceptionElement(void *exception)
{
  ExceptionInfo
    *p;

  p=(ExceptionInfo *) exception;
  if (p->reason != (char *) NULL)
    p->reason=DestroyString(p->reason);
  if (p->description != (char *) NULL)
    p->description=DestroyString(p->description);
  p=(ExceptionInfo *) RelinquishMagickMemory(p);
  return((void *) NULL);
}

MagickExport void ClearMagickException(ExceptionInfo *exception)
{
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (exception->exceptions == (void *) NULL)
    return;
  LockSemaphoreInfo(exception->semaphore);
  ClearLinkedList((LinkedListInfo *) exception->exceptions,
    DestroyExceptionElement);
  exception->severity=UndefinedException;
  exception->reason=(char *) NULL;
  exception->description=(char *) NULL;
  UnlockSemaphoreInfo(exception->semaphore);
  errno=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C a t c h E x c e p t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CatchException() returns if no exceptions is found otherwise it reports
%  the exception as a warning, error, or fatal depending on the severity.
%
%  The format of the CatchException method is:
%
%      CatchException(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
*/
MagickExport void CatchException(ExceptionInfo *exception)
{
  LinkedListInfo
    *exceptions;

  const ExceptionInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (exception->exceptions  == (void *) NULL)
    return;
  LockSemaphoreInfo(exception->semaphore);
  exceptions=(LinkedListInfo *) exception->exceptions;
  ResetLinkedListIterator(exceptions);
  p=(const ExceptionInfo *) GetNextValueInLinkedList(exceptions);
  while (p != (const ExceptionInfo *) NULL)
  {
    if ((p->severity >= WarningException) && (p->severity < ErrorException))
      MagickWarning(p->severity,p->reason,p->description);
    if ((p->severity >= ErrorException) && (p->severity < FatalErrorException))
      MagickError(p->severity,p->reason,p->description);
    if (p->severity >= FatalErrorException)
      MagickFatalError(p->severity,p->reason,p->description);
    p=(const ExceptionInfo *) GetNextValueInLinkedList(exceptions);
  }
  UnlockSemaphoreInfo(exception->semaphore);
  ClearMagickException(exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e E x c e p t i o n I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneExceptionInfo() clones the ExceptionInfo structure.
%
%  The format of the CloneExceptionInfo method is:
%
%      ExceptionInfo *CloneException(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
*/
MagickExport ExceptionInfo *CloneExceptionInfo(ExceptionInfo *exception)
{
  ExceptionInfo
    *clone_exception;

  clone_exception=(ExceptionInfo *) AcquireCriticalMemory(sizeof(*exception));
  InitializeExceptionInfo(clone_exception);
  InheritException(clone_exception,exception);
  clone_exception->relinquish=MagickTrue;
  return(clone_exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f a u l t E r r o r H a n d l e r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefaultErrorHandler() displays an error reason.
%
%  The format of the DefaultErrorHandler method is:
%
%      void MagickError(const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
static void DefaultErrorHandler(const ExceptionType magick_unused(severity),
  const char *reason,const char *description)
{
  magick_unreferenced(severity);

  if (reason == (char *) NULL)
    return;
  (void) FormatLocaleFile(stderr,"%s: %s",GetClientName(),reason);
  if (description != (char *) NULL)
    (void) FormatLocaleFile(stderr," (%s)",description);
  (void) FormatLocaleFile(stderr,".\n");
  (void) fflush(stderr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f a u l t F a t a l E r r o r H a n d l e r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefaultFatalErrorHandler() displays an error reason and then terminates the
%  program.
%
%  The format of the DefaultFatalErrorHandler method is:
%
%      void MagickFatalError(const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the program.
%
%    o description: Specifies any description to the reason.
%
*/
static void DefaultFatalErrorHandler(const ExceptionType severity,
  const char *reason,const char *description)
{
  (void) FormatLocaleFile(stderr,"%s:",GetClientName());
  if (reason != (char *) NULL)
    (void) FormatLocaleFile(stderr," %s",reason);
  if (description != (char *) NULL)
    (void) FormatLocaleFile(stderr," (%s)",description);
  (void) FormatLocaleFile(stderr,".\n");
  (void) fflush(stderr);
  MagickCoreTerminus();
  exit((int) (severity-FatalErrorException)+1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f a u l t W a r n i n g H a n d l e r                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefaultWarningHandler() displays a warning reason.
%
%  The format of the DefaultWarningHandler method is:
%
%      void DefaultWarningHandler(const ExceptionType severity,
%        const char *reason,const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric warning category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
static void DefaultWarningHandler(const ExceptionType magick_unused(severity),
  const char *reason,const char *description)
{
  magick_unreferenced(severity);

  if (reason == (char *) NULL)
    return;
  (void) FormatLocaleFile(stderr,"%s: %s",GetClientName(),reason);
  if (description != (char *) NULL)
    (void) FormatLocaleFile(stderr," (%s)",description);
  (void) FormatLocaleFile(stderr,".\n");
  (void) fflush(stderr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y E x c e p t i o n I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyExceptionInfo() deallocates memory associated with an exception.
%
%  The format of the DestroyExceptionInfo method is:
%
%      ExceptionInfo *DestroyExceptionInfo(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
*/
MagickExport ExceptionInfo *DestroyExceptionInfo(ExceptionInfo *exception)
{
  MagickBooleanType
    relinquish;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (exception->semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&exception->semaphore);
  LockSemaphoreInfo(exception->semaphore);
  exception->severity=UndefinedException;
  if (exception->relinquish != MagickFalse)
    {
      exception->signature=(~MagickCoreSignature);
      if (exception->exceptions != (void *) NULL)
        exception->exceptions=(void *) DestroyLinkedList((LinkedListInfo *)
          exception->exceptions,DestroyExceptionElement);
    }
  else
    if (exception->exceptions != (void *) NULL)
      ClearLinkedList((LinkedListInfo *) exception->exceptions,
        DestroyExceptionElement);
  relinquish=exception->relinquish;
  UnlockSemaphoreInfo(exception->semaphore);
  if (relinquish != MagickFalse)
    {
      RelinquishSemaphoreInfo(&exception->semaphore);
      exception=(ExceptionInfo *) RelinquishMagickMemory(exception);
    }
  return(exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x e c e p t i o n C o m p o n e n t G e n e s i s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExceptionComponentGenesis() instantiates the exception component.
%
%  The format of the ExceptionComponentGenesis method is:
%
%      MagickBooleanType ExceptionComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType ExceptionComponentGenesis(void)
{
  if (exception_semaphore == (SemaphoreInfo *) NULL)
    exception_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x c e p t i o n C o m p o n e n t T e r m i n u s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExceptionComponentTerminus() destroys the exception component.
%
%  The format of the ExceptionComponentTerminus method is:
%
%      void ExceptionComponentTerminus(void)
%
*/
MagickPrivate void ExceptionComponentTerminus(void)
{
  if (exception_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&exception_semaphore);
  LockSemaphoreInfo(exception_semaphore);
  UnlockSemaphoreInfo(exception_semaphore);
  RelinquishSemaphoreInfo(&exception_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E x c e p t i o n M e s s a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetExceptionMessage() returns the error message defined by the specified
%  error code.
%
%  The format of the GetExceptionMessage method is:
%
%      char *GetExceptionMessage(const int error)
%
%  A description of each parameter follows:
%
%    o error: the error code.
%
*/
MagickExport char *GetExceptionMessage(const int error)
{
  char
    exception[MagickPathExtent];

  *exception='\0';
#if defined(MAGICKCORE_HAVE_STRERROR_R)
#if !defined(MAGICKCORE_STRERROR_R_CHAR_P)
  (void) strerror_r(error,exception,sizeof(exception));
#else
  (void) CopyMagickString(exception,strerror_r(error,exception,
    sizeof(exception)),sizeof(exception));
#endif
#else
  (void) CopyMagickString(exception,strerror(error),sizeof(exception));
#endif
  return(ConstantString(exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e E x c e p t i o n M e s s a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleExceptionMessage() converts a enumerated exception severity and tag
%  to a message in the current locale.
%
%  The format of the GetLocaleExceptionMessage method is:
%
%      const char *GetLocaleExceptionMessage(const ExceptionType severity,
%        const char *tag)
%
%  A description of each parameter follows:
%
%    o severity: the severity of the exception.
%
%    o tag: the message tag.
%
*/

static const char *ExceptionSeverityToTag(const ExceptionType severity)
{
  switch (severity)
  {
    case ResourceLimitWarning: return("Resource/Limit/Warning/");
    case TypeWarning: return("Type/Warning/");
    case OptionWarning: return("Option/Warning/");
    case DelegateWarning: return("Delegate/Warning/");
    case MissingDelegateWarning: return("Missing/Delegate/Warning/");
    case CorruptImageWarning: return("Corrupt/Image/Warning/");
    case FileOpenWarning: return("File/Open/Warning/");
    case BlobWarning: return("Blob/Warning/");
    case StreamWarning: return("Stream/Warning/");
    case CacheWarning: return("Cache/Warning/");
    case CoderWarning: return("Coder/Warning/");
    case FilterWarning: return("Filter/Warning/");
    case ModuleWarning: return("Module/Warning/");
    case DrawWarning: return("Draw/Warning/");
    case ImageWarning: return("Image/Warning/");
    case WandWarning: return("Wand/Warning/");
    case XServerWarning: return("XServer/Warning/");
    case MonitorWarning: return("Monitor/Warning/");
    case RegistryWarning: return("Registry/Warning/");
    case ConfigureWarning: return("Configure/Warning/");
    case PolicyWarning: return("Policy/Warning/");
    case ResourceLimitError: return("Resource/Limit/Error/");
    case TypeError: return("Type/Error/");
    case OptionError: return("Option/Error/");
    case DelegateError: return("Delegate/Error/");
    case MissingDelegateError: return("Missing/Delegate/Error/");
    case CorruptImageError: return("Corrupt/Image/Error/");
    case FileOpenError: return("File/Open/Error/");
    case BlobError: return("Blob/Error/");
    case StreamError: return("Stream/Error/");
    case CacheError: return("Cache/Error/");
    case CoderError: return("Coder/Error/");
    case FilterError: return("Filter/Error/");
    case ModuleError: return("Module/Error/");
    case DrawError: return("Draw/Error/");
    case ImageError: return("Image/Error/");
    case WandError: return("Wand/Error/");
    case XServerError: return("XServer/Error/");
    case MonitorError: return("Monitor/Error/");
    case RegistryError: return("Registry/Error/");
    case ConfigureError: return("Configure/Error/");
    case PolicyError: return("Policy/Error/");
    case ResourceLimitFatalError: return("Resource/Limit/FatalError/");
    case TypeFatalError: return("Type/FatalError/");
    case OptionFatalError: return("Option/FatalError/");
    case DelegateFatalError: return("Delegate/FatalError/");
    case MissingDelegateFatalError: return("Missing/Delegate/FatalError/");
    case CorruptImageFatalError: return("Corrupt/Image/FatalError/");
    case FileOpenFatalError: return("File/Open/FatalError/");
    case BlobFatalError: return("Blob/FatalError/");
    case StreamFatalError: return("Stream/FatalError/");
    case CacheFatalError: return("Cache/FatalError/");
    case CoderFatalError: return("Coder/FatalError/");
    case FilterFatalError: return("Filter/FatalError/");
    case ModuleFatalError: return("Module/FatalError/");
    case DrawFatalError: return("Draw/FatalError/");
    case ImageFatalError: return("Image/FatalError/");
    case WandFatalError: return("Wand/FatalError/");
    case XServerFatalError: return("XServer/FatalError/");
    case MonitorFatalError: return("Monitor/FatalError/");
    case RegistryFatalError: return("Registry/FatalError/");
    case ConfigureFatalError: return("Configure/FatalError/");
    case PolicyFatalError: return("Policy/FatalError/");
    default: break;
  }
  return("");
}

MagickExport const char *GetLocaleExceptionMessage(const ExceptionType severity,
  const char *tag)
{
  char
    message[MagickPathExtent];

  const char
    *locale_message;

  assert(tag != (const char *) NULL);
  (void) FormatLocaleString(message,MagickPathExtent,"Exception/%s%s",
    ExceptionSeverityToTag(severity),tag);
  locale_message=GetLocaleMessage(message);
  if (locale_message == (const char *) NULL)
    return(tag);
  if (locale_message == message)
    return(tag);
  return(locale_message);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n h e r i t E x c e p t i o n                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InheritException() inherits an exception from a related exception.
%
%  The format of the InheritException method is:
%
%      InheritException(ExceptionInfo *exception,const ExceptionInfo *relative)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
%    o relative: the related exception info.
%
*/
MagickExport void InheritException(ExceptionInfo *exception,
  const ExceptionInfo *relative)
{
  const ExceptionInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  assert(relative != (ExceptionInfo *) NULL);
  assert(relative->signature == MagickCoreSignature);
  assert(exception != relative);
  if (relative->exceptions == (void *) NULL)
    return;
  LockSemaphoreInfo(relative->semaphore);
  ResetLinkedListIterator((LinkedListInfo *) relative->exceptions);
  p=(const ExceptionInfo *) GetNextValueInLinkedList((LinkedListInfo *)
    relative->exceptions);
  while (p != (const ExceptionInfo *) NULL)
  {
    (void) ThrowException(exception,p->severity,p->reason,p->description);
    p=(const ExceptionInfo *) GetNextValueInLinkedList((LinkedListInfo *)
      relative->exceptions);
  }
  UnlockSemaphoreInfo(relative->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e t E x c e p t i o n I n f o                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeExceptionInfo() initializes an exception to default values.
%
%  The format of the InitializeExceptionInfo method is:
%
%      InitializeExceptionInfo(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
*/
MagickPrivate void InitializeExceptionInfo(ExceptionInfo *exception)
{
  assert(exception != (ExceptionInfo *) NULL);
  (void) memset(exception,0,sizeof(*exception));
  exception->severity=UndefinedException;
  exception->exceptions=(void *) NewLinkedList(0);
  exception->semaphore=AcquireSemaphoreInfo();
  exception->signature=MagickCoreSignature;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E r r o r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickError() calls the exception handler methods with an error reason.
%
%  The format of the MagickError method is:
%
%      void MagickError(const ExceptionType error,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o exception: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
MagickExport void MagickError(const ExceptionType error,const char *reason,
  const char *description)
{
  if (error_handler != (ErrorHandler) NULL)
    (*error_handler)(error,reason,description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F a t al E r r o r                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFatalError() calls the fatal exception handler methods with an error
%  reason.
%
%  The format of the MagickError method is:
%
%      void MagickFatalError(const ExceptionType error,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o exception: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
MagickExport void MagickFatalError(const ExceptionType error,const char *reason,
  const char *description)
{
  if (fatal_error_handler != (FatalErrorHandler) NULL)
    (*fatal_error_handler)(error,reason,description);
  MagickCoreTerminus();
  exit(1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W a r n i n g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWarning() calls the warning handler methods with a warning reason.
%
%  The format of the MagickWarning method is:
%
%      void MagickWarning(const ExceptionType warning,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o warning: the warning severity.
%
%    o reason: Define the reason for the warning.
%
%    o description: Describe the warning.
%
*/
MagickExport void MagickWarning(const ExceptionType warning,const char *reason,
  const char *description)
{
  if (warning_handler != (WarningHandler) NULL)
    (*warning_handler)(warning,reason,description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t E r r o r H a n d l e r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetErrorHandler() sets the exception handler to the specified method
%  and returns the previous exception handler.
%
%  The format of the SetErrorHandler method is:
%
%      ErrorHandler SetErrorHandler(ErrorHandler handler)
%
%  A description of each parameter follows:
%
%    o handler: the method to handle errors.
%
*/
MagickExport ErrorHandler SetErrorHandler(ErrorHandler handler)
{
  ErrorHandler
    previous_handler;

  if (exception_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&exception_semaphore);
  LockSemaphoreInfo(exception_semaphore);
  previous_handler=error_handler;
  error_handler=handler;
  UnlockSemaphoreInfo(exception_semaphore);
  return(previous_handler);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t F a t a l E r r o r H a n d l e r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetFatalErrorHandler() sets the fatal exception handler to the specified
%  method and returns the previous fatal exception handler.
%
%  The format of the SetErrorHandler method is:
%
%      ErrorHandler SetErrorHandler(ErrorHandler handler)
%
%  A description of each parameter follows:
%
%    o handler: the method to handle errors.
%
*/
MagickExport FatalErrorHandler SetFatalErrorHandler(FatalErrorHandler handler)
{
  FatalErrorHandler
    previous_handler;

  if (exception_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&exception_semaphore);
  LockSemaphoreInfo(exception_semaphore);
  previous_handler=fatal_error_handler;
  fatal_error_handler=handler;
  UnlockSemaphoreInfo(exception_semaphore);
  return(previous_handler);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t W a r n i n g H a n d l e r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetWarningHandler() sets the warning handler to the specified method
%  and returns the previous warning handler.
%
%  The format of the SetWarningHandler method is:
%
%      ErrorHandler SetWarningHandler(ErrorHandler handler)
%
%  A description of each parameter follows:
%
%    o handler: the method to handle warnings.
%
*/
MagickExport WarningHandler SetWarningHandler(WarningHandler handler)
{
  WarningHandler
    previous_handler;

  if (exception_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&exception_semaphore);
  LockSemaphoreInfo(exception_semaphore);
  previous_handler=warning_handler;
  warning_handler=handler;
  UnlockSemaphoreInfo(exception_semaphore);
  return(previous_handler);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T h r o w E x c e p t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ThrowException() throws an exception with the specified severity code,
%  reason, and optional description.
%
%  The format of the ThrowException method is:
%
%      MagickBooleanType ThrowException(ExceptionInfo *exception,
%        const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
%    o severity: the severity of the exception.
%
%    o reason: the reason for the exception.
%
%    o description: the exception description.
%
*/
MagickExport MagickBooleanType ThrowException(ExceptionInfo *exception,
  const ExceptionType severity,const char *reason,const char *description)
{
  LinkedListInfo
    *exceptions;

  ExceptionInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  LockSemaphoreInfo(exception->semaphore);
  exceptions=(LinkedListInfo *) exception->exceptions;
  if (GetNumberOfElementsInLinkedList(exceptions) > MaxExceptionList)
    {
      if (severity < ErrorException)
        {
          UnlockSemaphoreInfo(exception->semaphore);
          return(MagickTrue);
        }
      p=(ExceptionInfo *) GetLastValueInLinkedList(exceptions);
      if (p->severity >= ErrorException)
        {
          UnlockSemaphoreInfo(exception->semaphore);
          return(MagickTrue);
        }
    }
  p=(ExceptionInfo *) GetLastValueInLinkedList(exceptions);
  if ((p != (ExceptionInfo *) NULL) && (p->severity == severity) &&
      (LocaleCompare(exception->reason,reason) == 0) &&
      (LocaleCompare(exception->description,description) == 0))
    {
      UnlockSemaphoreInfo(exception->semaphore);
      return(MagickTrue);
    }
  p=(ExceptionInfo *) AcquireMagickMemory(sizeof(*p));
  if (p == (ExceptionInfo *) NULL)
    {
      UnlockSemaphoreInfo(exception->semaphore);
      ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
    }
  (void) memset(p,0,sizeof(*p));
  p->severity=severity;
  if (reason != (const char *) NULL)
    p->reason=ConstantString(reason);
  if (description != (const char *) NULL)
    p->description=ConstantString(description);
  p->signature=MagickCoreSignature;
  (void) AppendValueToLinkedList(exceptions,p);
  if (p->severity > exception->severity)
    {
      exception->severity=p->severity;
      exception->reason=p->reason;
      exception->description=p->description;
    }
  UnlockSemaphoreInfo(exception->semaphore);
  if (GetNumberOfElementsInLinkedList(exceptions) == MaxExceptionList)
    (void) ThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"TooManyExceptions",
      "(exception processing is suspended)");
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T h r o w M a g i c k E x c e p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ThrowMagickException logs an exception as determined by the log
%  configuration file.  If an error occurs, MagickFalse is returned
%  otherwise MagickTrue.
%
%  The format of the ThrowMagickException method is:
%
%      MagickBooleanType ThrowFileException(ExceptionInfo *exception,
%        const char *module,const char *function,const size_t line,
%        const ExceptionType severity,const char *tag,const char *format,...)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
%    o filename: the source module filename.
%
%    o function: the function name.
%
%    o line: the line number of the source module.
%
%    o severity: Specifies the numeric error category.
%
%    o tag: the locale tag.
%
%    o format: the output format.
%
*/

MagickExport MagickBooleanType ThrowMagickExceptionList(
  ExceptionInfo *exception,const char *module,const char *function,
  const size_t line,const ExceptionType severity,const char *tag,
  const char *format,va_list operands)
{
  char
    message[MagickPathExtent],
    path[MagickPathExtent],
    reason[MagickPathExtent];

  const char
    *locale,
    *type;

  int
    n;

  MagickBooleanType
    status;

  size_t
    length;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  locale=GetLocaleExceptionMessage(severity,tag);
  (void) CopyMagickString(reason,locale,MagickPathExtent);
  (void) ConcatenateMagickString(reason," ",MagickPathExtent);
  length=strlen(reason);
#if defined(MAGICKCORE_HAVE_VSNPRINTF)
  n=vsnprintf(reason+length,MagickPathExtent-length,format,operands);
#else
  n=vsprintf(reason+length,format,operands);
#endif
  if (n < 0)
    reason[MagickPathExtent-1]='\0';
  status=LogMagickEvent(ExceptionEvent,module,function,line,"%s",reason);
  GetPathComponent(module,TailPath,path);
  type="undefined";
  if ((severity >= WarningException) && (severity < ErrorException))
    type="warning";
  if ((severity >= ErrorException) && (severity < FatalErrorException))
    type="error";
  if (severity >= FatalErrorException)
    type="fatal";
  (void) FormatLocaleString(message,MagickPathExtent,"%s @ %s/%s/%s/%.20g",
    reason,type,path,function,(double) line);
  (void) ThrowException(exception,severity,message,(char *) NULL);
  return(status);
}

MagickExport MagickBooleanType ThrowMagickException(ExceptionInfo *exception,
  const char *module,const char *function,const size_t line,
  const ExceptionType severity,const char *tag,const char *format,...)
{
  MagickBooleanType
    status;

  va_list
    operands;

  va_start(operands,format);
  status=ThrowMagickExceptionList(exception,module,function,line,severity,tag,
    format,operands);
  va_end(operands);
  return(status);
}
