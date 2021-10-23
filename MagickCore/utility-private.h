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

  MagickCore private utility methods.
*/
#ifndef MAGICKCORE_UTILITY_PRIVATE_H
#define MAGICKCORE_UTILITY_PRIVATE_H

#include "MagickCore/memory_.h"
#include "MagickCore/nt-base.h"
#include "MagickCore/nt-base-private.h"
#if defined(MAGICKCORE_HAVE_UTIME_H)
#include <utime.h>
#endif

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
  ExpandFilename(char *);

static inline int MagickReadDirectory(DIR *directory,struct dirent *entry,
  struct dirent **result)
{
  (void) entry;
  errno=0;
  *result=readdir(directory);
  return(errno);
}

/*
  Windows UTF8 compatibility methods.
*/

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
static inline wchar_t *create_wchar_path(const char *utf8)
{
  int
    count;

  wchar_t
    *wideChar;

  count=MultiByteToWideChar(CP_UTF8,0,utf8,-1,NULL,0);
  if ((count > MAX_PATH) && (strncmp(utf8,"\\\\?\\",4) != 0) &&
      (NTLongPathsEnabled() == MagickFalse))
    {
      char
        buffer[MagickPathExtent];

      wchar_t
        shortPath[MAX_PATH],
        *longPath;

      (void) FormatLocaleString(buffer,MagickPathExtent,"\\\\?\\%s",utf8);
      count+=4;
      longPath=(wchar_t *) AcquireQuantumMemory(count,sizeof(*longPath));
      if (longPath == (wchar_t *) NULL)
        return((wchar_t *) NULL);
      count=MultiByteToWideChar(CP_UTF8,0,buffer,-1,longPath,count);
      if (count != 0)
        count=GetShortPathNameW(longPath,shortPath,MAX_PATH);
      longPath=(wchar_t *) RelinquishMagickMemory(longPath);
      if ((count < 5) || (count >= MAX_PATH))
        return((wchar_t *) NULL);
      wideChar=(wchar_t *) AcquireQuantumMemory((size_t) count-3,
        sizeof(*wideChar));
      wcscpy(wideChar,shortPath+4);
      return(wideChar);
    }
  wideChar=(wchar_t *) AcquireQuantumMemory(count,sizeof(*wideChar));
  if (wideChar == (wchar_t *) NULL)
    return((wchar_t *) NULL);
  count=MultiByteToWideChar(CP_UTF8,0,utf8,-1,wideChar,count);
  if (count == 0)
    {
      wideChar=(wchar_t *) RelinquishMagickMemory(wideChar);
      return((wchar_t *) NULL);
    }
  return(wideChar);
}

static inline wchar_t *create_wchar_mode(const char *mode)
{
  int
    count;

  wchar_t
    *wideChar;

  count=MultiByteToWideChar(CP_UTF8,0,mode,-1,NULL,0);
  wideChar=(wchar_t *) AcquireQuantumMemory((size_t) count+1,
    sizeof(*wideChar));
  if (wideChar == (wchar_t *) NULL)
    return((wchar_t *) NULL);
  count=MultiByteToWideChar(CP_UTF8,0,mode,-1,wideChar,count);
  if (count == 0)
    {
      wideChar=(wchar_t *) RelinquishMagickMemory(wideChar);
      return((wchar_t *) NULL);
    }
  /* Specifies that the file is not inherited by child processes */
  wideChar[count] = L'\0';
  wideChar[count-1] = L'N';
  return(wideChar);
}
#endif

static inline int access_utf8(const char *path,int mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(access(path,mode));
#else
   int
     status;

   wchar_t
     *path_wide;

   path_wide=create_wchar_path(path);
   if (path_wide == (wchar_t *) NULL)
     return(-1);
   status=_waccess(path_wide,mode);
   path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

static inline FILE *fopen_utf8(const char *path,const char *mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(fopen(path,mode));
#else
   FILE
     *file;

   wchar_t
     *mode_wide,
     *path_wide;

   path_wide=create_wchar_path(path);
   if (path_wide == (wchar_t *) NULL)
     return((FILE *) NULL);
   mode_wide=create_wchar_mode(mode);
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

static inline void getcwd_utf8(char *path,size_t extent)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  char
    *directory;

   directory=getcwd(path,extent);
   (void) directory;
#else
  wchar_t
    wide_path[MagickPathExtent];

  (void) _wgetcwd(wide_path,MagickPathExtent-1);
  (void) WideCharToMultiByte(CP_UTF8,0,wide_path,-1,path,(int) extent,NULL,NULL);
#endif
}

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__) && !defined(__MINGW32__)
typedef int
  mode_t;
#endif

static inline int open_utf8(const char *path,int flags,mode_t mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(open(path,flags,mode));
#else
  int
    status;

  wchar_t
    *path_wide;

  path_wide=create_wchar_path(path);
  if (path_wide == (wchar_t *) NULL)
    return(-1);
  /* O_NOINHERIT specifies that the file is not inherited by child processes */
  status=_wopen(path_wide,flags | O_NOINHERIT,mode);
  path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
  return(status);
#endif
}

static inline FILE *popen_utf8(const char *command,const char *type)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(popen(command,type));
#else
  FILE
    *file;

  int
    length;

  wchar_t
    *command_wide,
    type_wide[5];

  file=(FILE *) NULL;
  length=MultiByteToWideChar(CP_UTF8,0,type,-1,type_wide,5);
  if (length == 0)
    return(file);
  length=MultiByteToWideChar(CP_UTF8,0,command,-1,NULL,0);
  if (length == 0)
    return(file);
  command_wide=(wchar_t *) AcquireQuantumMemory(length,sizeof(*command_wide));
  if (command_wide == (wchar_t *) NULL)
    return(file);
  length=MultiByteToWideChar(CP_UTF8,0,command,-1,command_wide,length);
  if (length != 0)
    file=_wpopen(command_wide,type_wide);
  command_wide=(wchar_t *) RelinquishMagickMemory(command_wide);
  return(file);
#endif
}

static inline int remove_utf8(const char *path)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(unlink(path));
#else
   int
     status;

   wchar_t
     *path_wide;

   path_wide=create_wchar_path(path);
   if (path_wide == (wchar_t *) NULL)
     return(-1);
   status=_wremove(path_wide);
   path_wide=(wchar_t *) RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

static inline int rename_utf8(const char *source,const char *destination)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(rename(source,destination));
#else
   int
     status;

   wchar_t
     *destination_wide,
     *source_wide;

   source_wide=create_wchar_path(source);
   if (source_wide == (wchar_t *) NULL)
     return(-1);
   destination_wide=create_wchar_path(destination);
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

static inline int set_file_timestamp(const char *path,struct stat *attributes)
{
  int
    status;

#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#if defined(MAGICKCORE_HAVE_UTIMENSAT)
#if defined(__APPLE__) || defined(__NetBSD__) 
#define st_atim st_atimespec
#define st_ctim st_ctimespec
#define st_mtim st_mtimespec
#endif

  struct timespec
    timestamp[2];

  timestamp[0]=attributes->st_atim;
  timestamp[1]=attributes->st_mtim;
  status=utimensat(AT_FDCWD,path,timestamp,0);
#else
  struct utimbuf
    timestamp;

  timestamp.actime=attributes->st_atime;
  timestamp.modtime=attributes->st_mtime;
  status=utime(path,&timestamp);
#endif
#else
  HANDLE
    handle;

  wchar_t
    *path_wide;

  status=(-1);
  path_wide=create_wchar_path(path);
  if (path_wide == (WCHAR *) NULL)
    return(status);
  handle=CreateFileW(path_wide,FILE_WRITE_ATTRIBUTES,FILE_SHARE_WRITE |
    FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if (handle != (HANDLE) NULL)
    {
      FILETIME
        creationTime,
        lastAccessTime,
        lastWriteTime;

      LONGLONG
        dateTime;

      dateTime=Int32x32To64(attributes->st_ctime,10000000)+116444736000000000;
      creationTime.dwLowDateTime=(DWORD) dateTime;
      creationTime.dwHighDateTime=dateTime>>32;
      dateTime=Int32x32To64(attributes->st_atime,10000000)+116444736000000000;
      lastAccessTime.dwLowDateTime=(DWORD) dateTime;
      lastAccessTime.dwHighDateTime=dateTime>>32;
      dateTime=Int32x32To64(attributes->st_mtime,10000000)+116444736000000000;
      lastWriteTime.dwLowDateTime=(DWORD) dateTime;
      lastWriteTime.dwHighDateTime=dateTime>>32;
      status=SetFileTime(handle,&creationTime,&lastAccessTime,&lastWriteTime);
      CloseHandle(handle);
      status=0;
    }
  path_wide=(WCHAR *) RelinquishMagickMemory(path_wide);
#endif
  return(status);
}

static inline int stat_utf8(const char *path,struct stat *attributes)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(stat(path,attributes));
#else
   int
     status;

   wchar_t
     *path_wide;

   path_wide=create_wchar_path(path);
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
