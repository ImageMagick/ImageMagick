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

  MagickCore private utility methods.
*/
#ifndef _MAGICKCORE_UTILITY_PRIVATE_H
#define _MAGICKCORE_UTILITY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/memory_.h"
#include "MagickCore/nt-base-private.h"

extern MagickPrivate char
  **GetPathComponents(const char *,size_t *),
  **ListFiles(const char *,const char *,size_t *);

extern MagickPrivate MagickBooleanType
  GetExecutionPath(char *,const size_t);

extern MagickPrivate ssize_t
  GetMagickPageSize(void);

extern MagickPrivate void
  ChopPathComponents(char *,const size_t),
  ExpandFilename(char *),
  MagickDelay(const MagickSizeType);

/*
  Windows UTF8 compatibility methods.
*/

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__) && !defined(__MINGW32__)
typedef int
  mode_t;

static inline int MultiByteToWideCharacter(const char *string,
  WCHAR **wide_string,size_t *extent)
{
  size_t
    length;

  *extent=0;
  if (wide_string == (WCHAR **) NULL)
    return(0);
  *wide_string=(WCHAR *) NULL;
  if (string == (const char *) NULL)
    return(0);
  length=strlen(string)+1;
  *wide_string=(WCHAR *) AcquireQuantumMemory(length,sizeof(*wide_string));
  if (*wide_string == (WCHAR *) NULL)
    return(-1);
  return(mbstowcs_s(extent,*wide_string,length,string,_TRUNCATE));
}
#endif

static inline int access_utf8(const char *path,int mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(access(path,mode));
#else
   int
     status;

   ssize_t
     extent;

   WCHAR
     *wide_path;

   status=MultiByteToWideCharacter(path,&wide_path,&extent);
   if (status != 0)
     return(status);
   status=_waccess(wide_path,mode);
   wide_path=(WCHAR *) RelinquishMagickMemory(wide_path);
   return(status);
#endif
}

static inline FILE *fopen_utf8(const char *path,const char *mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(fopen(path,mode));
#else
   FILE
     *file;

   int
     status;

   ssize_t
     extent;

   WCHAR
     *wide_mode,
     *wide_path;

   status=MultiByteToWideCharacter(path,&wide_path,&extent);
   if (status != 0)
     return((FILE *) NULL);
   status=MultiByteToWideCharacter(mode,&wide_mode,&extent);
   if (status != 0)
     {
       wide_path=(WCHAR *) RelinquishMagickMemory(wide_path);
       return((FILE *) NULL);
     }
   file=_wfopen(wide_path,wide_mode);
   wide_mode=(WCHAR *) RelinquishMagickMemory(wide_mode);
   wide_path=(WCHAR *) RelinquishMagickMemory(wide_path);
   return(file);
#endif
}

static inline int open_utf8(const char *path,int flags,mode_t mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(open(path,flags,mode));
#else
   int
     status;

   ssize_t
     extent;

   WCHAR
     *wide_path;

   status=MultiByteToWideCharacter(path,&wide_path,&extent);
   if (status != 0)
     return(status);
   status=_wopen(wide_path,flags,mode);
   wide_path=(WCHAR *) RelinquishMagickMemory(wide_path);
   return(status);
#endif
}

static inline FILE *popen_utf8(const char *command,const char *type)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(fopen(command,type));
#else
   FILE
     *file;

   int
     status;

   ssize_t
     extent;

   WCHAR
     *wide_type,
     *wide_command;

   status=MultiByteToWideCharacter(command,&wide_command,&extent);
   if (status != 0)
     return((FILE *) NULL);
   status=MultiByteToWideCharacter(type,&wide_type,&extent);
   if (status != 0)
     {
       wide_command=(WCHAR *) RelinquishMagickMemory(wide_command);
       return((FILE *) NULL);
     }
   file=_wpopen(wide_command,wide_type);
   wide_type=(WCHAR *) RelinquishMagickMemory(wide_type);
   wide_command=(WCHAR *) RelinquishMagickMemory(wide_command);
   return(file);
#endif
}

static inline int remove_utf8(const char *path)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(unlink(path));
#else
   int
     status;

   ssize_t
     extent;

   WCHAR
     *wide_path;

   status=MultiByteToWideCharacter(path,&wide_path,&extent);
   if (status != 0)
     return(status);
   status=_wremove(wide_path);
   wide_path=(WCHAR *) RelinquishMagickMemory(wide_path);
   return(status);
#endif
}

static inline int rename_utf8(const char *source,const char *destination)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(rename(source,destination));
#else
   int
     status;

   ssize_t
     extent;

   WCHAR
     *wide_destination,
     *wide_source;

   status=MultiByteToWideCharacter(source,&wide_source,&extent);
   if (status != 0)
     return(status);
   status=MultiByteToWideCharacter(destination,&wide_destination,&extent);
   if (status != 0)
     {
       wide_source=(WCHAR *) RelinquishMagickMemory(wide_source);
       return(status);
     }
   status=_wrename(wide_source,wide_destination);
   wide_destination=(WCHAR *) RelinquishMagickMemory(wide_destination);
   wide_source=(WCHAR *) RelinquishMagickMemory(wide_source);
   return(status);
#endif
}

static inline int stat_utf8(const char *path,struct stat *attributes)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(stat(path,attributes));
#else
   int
     status;

   ssize_t
     extent;

   WCHAR
     *wide_path;

   status=MultiByteToWideCharacter(path,&wide_path,&extent);
   if (status != 0)
     return(status);
   status=_wstat64(wide_path,attributes);
   wide_path=(WCHAR *) RelinquishMagickMemory(wide_path);
   return(status);
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
