/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore exception methods.
*/
#ifndef _MAGICKCORE_EXCEPTION_H
#define _MAGICKCORE_EXCEPTION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "magick/semaphore.h"

typedef enum
{
  UndefinedException,
  WarningException = 300,
  ResourceLimitWarning = 300,
  TypeWarning = 305,
  OptionWarning = 310,
  DelegateWarning = 315,
  MissingDelegateWarning = 320,
  CorruptImageWarning = 325,
  FileOpenWarning = 330,
  BlobWarning = 335,
  StreamWarning = 340,
  CacheWarning = 345,
  CoderWarning = 350,
  FilterWarning = 352,
  ModuleWarning = 355,
  DrawWarning = 360,
  ImageWarning = 365,
  WandWarning = 370,
  RandomWarning = 375,
  XServerWarning = 380,
  MonitorWarning = 385,
  RegistryWarning = 390,
  ConfigureWarning = 395,
  PolicyWarning = 399,
  ErrorException = 400,
  ResourceLimitError = 400,
  TypeError = 405,
  OptionError = 410,
  DelegateError = 415,
  MissingDelegateError = 420,
  CorruptImageError = 425,
  FileOpenError = 430,
  BlobError = 435,
  StreamError = 440,
  CacheError = 445,
  CoderError = 450,
  FilterError = 452,
  ModuleError = 455,
  DrawError = 460,
  ImageError = 465,
  WandError = 470,
  RandomError = 475,
  XServerError = 480,
  MonitorError = 485,
  RegistryError = 490,
  ConfigureError = 495,
  PolicyError = 499,
  FatalErrorException = 700,
  ResourceLimitFatalError = 700,
  TypeFatalError = 705,
  OptionFatalError = 710,
  DelegateFatalError = 715,
  MissingDelegateFatalError = 720,
  CorruptImageFatalError = 725,
  FileOpenFatalError = 730,
  BlobFatalError = 735,
  StreamFatalError = 740,
  CacheFatalError = 745,
  CoderFatalError = 750,
  FilterFatalError = 752,
  ModuleFatalError = 755,
  DrawFatalError = 760,
  ImageFatalError = 765,
  WandFatalError = 770,
  RandomFatalError = 775,
  XServerFatalError = 780,
  MonitorFatalError = 785,
  RegistryFatalError = 790,
  ConfigureFatalError = 795,
  PolicyFatalError = 799
} ExceptionType;

struct _ExceptionInfo
{
  ExceptionType
    severity;

  int
    error_number;

  char
    *reason,
    *description;

  void
    *exceptions;

  MagickBooleanType
    relinquish;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

typedef void
  (*ErrorHandler)(const ExceptionType,const char *,const char *);

typedef void
  (*FatalErrorHandler)(const ExceptionType,const char *,const char *);

typedef void
  (*WarningHandler)(const ExceptionType,const char *,const char *);

extern MagickExport char
  *GetExceptionMessage(const int);

extern MagickExport const char
  *GetLocaleExceptionMessage(const ExceptionType,const char *);

extern MagickExport ErrorHandler
  SetErrorHandler(ErrorHandler);

extern MagickExport ExceptionInfo
  *AcquireExceptionInfo(void),
  *DestroyExceptionInfo(ExceptionInfo *);

extern MagickExport FatalErrorHandler
  SetFatalErrorHandler(FatalErrorHandler);

extern MagickExport MagickBooleanType
  ThrowException(ExceptionInfo *,const ExceptionType,const char *,
    const char *),
  ThrowMagickException(ExceptionInfo *,const char *,const char *,const size_t,
    const ExceptionType,const char *,const char *,...)
    magick_attribute((__format__ (__printf__,7,8))),
  ThrowMagickExceptionList(ExceptionInfo *,const char *,const char *,
    const size_t,const ExceptionType,const char *,const char *,va_list)
    magick_attribute((__format__ (__printf__,7,0)));

extern MagickExport void
  CatchException(ExceptionInfo *),
  ClearMagickException(ExceptionInfo *),
  GetExceptionInfo(ExceptionInfo *),
  InheritException(ExceptionInfo *,const ExceptionInfo *),
  MagickError(const ExceptionType,const char *,const char *),
  MagickFatalError(const ExceptionType,const char *,const char *),
  MagickWarning(const ExceptionType,const char *,const char *);

extern MagickExport WarningHandler
  SetWarningHandler(WarningHandler);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
