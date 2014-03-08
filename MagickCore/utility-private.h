/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private utility methods.
*/
#ifndef _MAGICKCORE_UTILITY_PRIVATE_H
#define _MAGICKCORE_UTILITY_PRIVATE_H

#include "MagickCore/memory_.h"
#include "MagickCore/nt-base.h"
#include "MagickCore/nt-base-private.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate char
  **GetPathComponents(const char *,size_t *),
  **ListFiles(const char *,const char *,size_t *);

extern MagickPrivate MagickBooleanType
  GetExecutionPath(char *,const size_t),
  ShredFile(const char *);

extern MagickPrivate ssize_t
  GetMagickPageSize(void);

extern MagickPrivate void
  ChopPathComponents(char *,const size_t),
  ExpandFilename(char *),
  MagickDelay(const MagickSizeType);

/*
  Windows UTF8 compatibility methods.
*/

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
static inline wchar_t *create_wchar_string(const char *utf8)
{
  int
    count;
 
  wchar_t
    *wideChar;
 
  count=MultiByteToWideChar(CP_UTF8,0,utf8,-1,NULL,0);
  wideChar=(WCHAR *) AcquireQuantumMemory(count,sizeof(*wideChar));
  if (wideChar == (WCHAR *) NULL)
    return((WCHAR *) NULL);
  count=MultiByteToWideChar(CP_UTF8,0,utf8,-1,wideChar,count);
  if (count == 0)
    {
      wideChar=(WCHAR *) RelinquishMagickMemory(wideChar);
      return((WCHAR *) NULL);
    }
  return(wideChar);
}

static inline char *create_utf8_string(const wchar_t *wideChar)
{
  char
    *utf8;

  int
    count;

  count=WideCharToMultiByte(CP_UTF8,0,wideChar,-1,NULL,0,NULL,NULL);
  if (count < 0)
    return((char *) NULL);
  utf8=(char *) AcquireQuantumMemory(count+1,sizeof(*utf8));
  if (utf8 == (char *) NULL)
    return((char *) NULL);
  count=WideCharToMultiByte(CP_UTF8,0,wideChar,-1,utf8,count,NULL,NULL);
  if (count == 0)
    {
      utf8=DestroyString(utf8);
      return((char *) NULL);
    }
  utf8[count]=0;
  return(utf8);
}
#endif

static inline int access_utf8(const char *path,int mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
  return(access(path,mode));
#else
   int
     status;

   wchar_t
     *path_wide;

   path_wide=create_wchar_string(path);
   if (path_wide == (wchar_t *) NULL)
     return(-1);
   status=_waccess(path_wide,mode);
   path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

static inline FILE *fopen_utf8(const char *path,const char *mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
  return(fopen(path,mode));
#else
   FILE
     *file;

   wchar_t
     *mode_wide,
     *path_wide;

   path_wide=create_wchar_string(path);
   if (path_wide == (wchar_t *) NULL)
     return((FILE *) NULL);
   mode_wide=create_wchar_string(mode);
   if (mode_wide == (wchar_t *) NULL)
     {
       path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
       return((FILE *) NULL);
     }
   file=_wfopen(path_wide,mode_wide);
   mode_wide=(wchar_t *) RelinquishMagickMemory(mode_wide);
   path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
   return(file);
#endif
}

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__) && !defined(__MINGW32__) && !defined(__MINGW64__)
typedef int
  mode_t;
#endif

static inline int open_utf8(const char *path,int flags,mode_t mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
  return(open(path,flags,mode));
#else
   int
     status;

   wchar_t
     *path_wide;

   path_wide=create_wchar_string(path);
   if (path_wide == (wchar_t *) NULL)
     return(-1);
   status=_wopen(path_wide,flags,mode);
   path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

static inline FILE *popen_utf8(const char *command,const char *type)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
  return(popen(command,type));
#else
   FILE
     *file;

   wchar_t
     *type_wide,
     *command_wide;

   command_wide=create_wchar_string(command);
   if (command_wide == (wchar_t *) NULL)
     return((FILE *) NULL);
   type_wide=create_wchar_string(type);
   if (type_wide == (wchar_t *) NULL)
     {
       command_wide=(wchar_t *) RelinquishMagickMemory(command_wide);
       return((FILE *) NULL);
     }
   file=_wpopen(command_wide,type_wide);
   type_wide=(wchar_t *) RelinquishMagickMemory(type_wide);
   command_wide=(wchar_t *) RelinquishMagickMemory(command_wide);
   return(file);
#endif
}

static inline int remove_utf8(const char *path)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
  return(unlink(path));
#else
   int
     status;

   wchar_t
     *path_wide;

   path_wide=create_wchar_string(path);
   if (path_wide == (wchar_t *) NULL)
     return(-1);
   status=_wremove(path_wide);
   path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

static inline int rename_utf8(const char *source,const char *destination)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
  return(rename(source,destination));
#else
   int
     status;

   wchar_t
     *destination_wide,
     *source_wide;

   source_wide=create_wchar_string(source);
   if (source_wide == (wchar_t *) NULL)
     return(-1);
   destination_wide=create_wchar_string(destination);
   if (destination_wide == (wchar_t *) NULL)
     {
       source_wide=(wchar_t *) RelinquishMagickMemory(source_wide);
       return(-1);
     }
   status=_wrename(source_wide,destination_wide);
   destination_wide=(wchar_t *) RelinquishMagickMemory(destination_wide);
   source_wide=(wchar_t *) RelinquishMagickMemory(source_wide);
   return(status);
#endif
}

static inline int stat_utf8(const char *path,struct stat *attributes)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
  return(stat(path,attributes));
#else
   int
     status;

   wchar_t
     *path_wide;

   path_wide=create_wchar_string(path);
   if (path_wide == (WCHAR *) NULL)
     return(-1);
   status=wstat(path_wide,attributes);
   path_wide=(WCHAR *) RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
