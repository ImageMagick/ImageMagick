/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore log methods.
*/
#ifndef _MAGICKCORE_LOG_H
#define _MAGICKCORE_LOG_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "magick/exception.h"

#if !defined(GetMagickModule)
# define GetMagickModule()  __FILE__,__func__,(unsigned long) __LINE__
#endif

#define MagickLogFilename  "log.xml"

typedef enum
{
  UndefinedEvents,
  NoEvents = 0x00000,
  TraceEvent = 0x00001,
  AnnotateEvent = 0x00002,
  BlobEvent = 0x00004,
  CacheEvent = 0x00008,
  CoderEvent = 0x00010,
  ConfigureEvent = 0x00020,
  DeprecateEvent = 0x00040,
  DrawEvent = 0x00080,
  ExceptionEvent = 0x00100,
  LocaleEvent = 0x00200,
  ModuleEvent = 0x00400,
  PolicyEvent = 0x00800,
  ResourceEvent = 0x01000,
  TransformEvent = 0x02000,
  UserEvent = 0x04000,
  WandEvent = 0x08000,
  X11Event = 0x10000,
  AllEvents = 0x7fffffff
} LogEventType;

typedef struct _LogInfo
  LogInfo;

extern MagickExport char
  **GetLogList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport const char
  *GetLogName(void),
  *SetLogName(const char *);
                                                                                
extern MagickExport const LogInfo
  **GetLogInfoList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport LogEventType
  SetLogEventMask(const char *);

extern MagickExport MagickBooleanType
  IsEventLogging(void),
  ListLogInfo(FILE *,ExceptionInfo *),
  LogMagickEvent(const LogEventType,const char *,const char *,
    const unsigned long,const char *,...) 
    magick_attribute((format (printf,5,6))),
  LogMagickEventList(const LogEventType,const char *,const char *,
    const unsigned long,const char *,va_list)
    magick_attribute((format (printf,5,0)));

extern MagickExport void
  CloseMagickLog(void),
  DestroyLogList(void),
  SetLogFormat(const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
