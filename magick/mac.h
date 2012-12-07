/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore utility methods.
*/
#ifndef _MAGICKCORE_MAC_H
#define _MAGICKCORE_MAC_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <locale.h>
#include <Errors.h>
#include <Files.h>
#include <errno.h>

#if defined(_MAGICKCORE_POSIX_SUPPORT_VERSION)
# include <dirent.h>
# if !defined(DISABLE_SIOUX)
#  include <SIOUX.h>
# endif
#else
# include <stat.h>

#define S_IREAD  00400
#define S_IWRITE  00200

typedef struct _DIR
{
  int
    d_VRefNum;

  long
    d_DirID;

  int
    d_index;
} DIR;

struct dirent
{
  char
    d_name[255];

  int
    d_namlen;
};
#endif

MagickExport Image
  *ReadPICTImage(const ImageInfo *,ExceptionInfo *);

extern MagickExport int
  Exit(int),
  MACSystemCommand(const char *);

extern MagickExport MagickBooleanType
  MACIsMagickConflict(const char *);

extern MagickExport void
  MACErrorHandler(const ExceptionType,const char *,const char *),
  MACWarningHandler(const ExceptionType,const char *,const char *),
  ProcessPendingEvents(const char *),
  SetApplicationType(const char *,const char *,OSType);

#if defined(DISABLE_SIOUX)
typedef void
  (*MACEventHookPtr)(const char *);

typedef void
  (*MACErrorHookPtr)(const short,const char *text);

extern MagickExport void
  MACSetErrorHook(MACErrorHookPtr),
  MACSetEventHook(MACEventHookPtr),
  MACFatalErrorHandler(const ExceptionType,const char *,const char *);
#endif

#if !defined(_MAGICKCORE_POSIX_SUPPORT_VERSION)
extern MagickExport DIR
  *opendir(const char *);

extern MagickExport long
  telldir(DIR *);

extern MagickExport struct dirent
  *readdir(DIR *);

extern MagickExport void
  seekdir(DIR *,long
  closedir(DIR *);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
