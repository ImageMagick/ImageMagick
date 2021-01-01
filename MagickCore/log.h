/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore log methods.
*/
#ifndef MAGICKCORE_LOG_H
#define MAGICKCORE_LOG_H

#include "MagickCore/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(GetMagickModule)
# define GetMagickModule()  __FILE__,__func__,(unsigned long) __LINE__
#endif

#define MagickLogFilename  "log.xml"

typedef enum
{
  UndefinedEvents = 0x000000,
  NoEvents = 0x00000,
  AccelerateEvent = 0x00001,
  AnnotateEvent = 0x00002,
  BlobEvent = 0x00004,
  CacheEvent = 0x00008,
  CoderEvent = 0x00010,
  ConfigureEvent = 0x00020,
  DeprecateEvent = 0x00040,
  DrawEvent = 0x00080,
  ExceptionEvent = 0x00100,   /* Log Errors and Warnings immediately */
  ImageEvent = 0x00200,
  LocaleEvent = 0x00400,
  ModuleEvent = 0x00800,      /* Log coder and filter modules */
  PixelEvent = 0x01000,
  PolicyEvent = 0x02000,
  ResourceEvent = 0x04000,
  TraceEvent = 0x08000,
  TransformEvent = 0x10000,
  UserEvent = 0x20000,
  WandEvent = 0x40000,        /* Log MagickWand */
  X11Event = 0x80000,
  CommandEvent = 0x100000,    /* Log Command Processing (CLI & Scripts) */
  AllEvents = 0x7fffffff
} LogEventType;

typedef struct _LogInfo
  LogInfo;

typedef void
  (*MagickLogMethod)(const LogEventType,const char *);

extern MagickExport char
  **GetLogList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const char
  *GetLogName(void),
  *SetLogName(const char *);

extern MagickExport const LogInfo
  **GetLogInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport LogEventType
  SetLogEventMask(const char *);

extern MagickExport MagickBooleanType
  IsEventLogging(void) magick_attribute((__pure__)),
  ListLogInfo(FILE *,ExceptionInfo *),
  LogMagickEvent(const LogEventType,const char *,const char *,const size_t,
    const char *,...) magick_attribute((__format__ (__printf__,5,6))),
  LogMagickEventList(const LogEventType,const char *,const char *,const size_t,
    const char *,va_list) magick_attribute((__format__ (__printf__,5,0)));

extern MagickExport void
  CloseMagickLog(void),
  SetLogFormat(const char *),
  SetLogMethod(MagickLogMethod);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
