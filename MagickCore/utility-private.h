/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/license/

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
#if defined(__MINGW32__)
#include <share.h>
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

static inline int access_utf8(const char *path,int mode)
{
  if (path == (const char *) NULL)
    return(-1);
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(access(path,mode));
#else
  return(NTAccessWide(path,mode));
#endif
}

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
#define close_utf8 _close
#else
#define close_utf8 close
#endif

static inline FILE *fopen_utf8(const char *path,const char *mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(fopen(path,mode));
#else
  return(NTOpenFileWide(path,mode));
#endif
}

static inline int open_utf8(const char *path,int flags,mode_t mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(open(path,flags,mode));
#else
  return(NTOpenWide(path,flags,mode));
#endif
}

static inline FILE *popen_utf8(const char *command,const char *type)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(popen(command,type));
#else
  return(NTOpenPipeWide(command,type));
#endif
}

static inline char *realpath_utf8(const char *path)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#if defined(MAGICKCORE_HAVE_REALPATH)
  /*
    This does not work for non-existing files so we should fine another way
    to do this in the future. This is only a possible issue when writing files.
  */
  return(realpath(path,(char *) NULL));
#else
  return(AcquireString(path));
#endif
#else
  return(NTRealPathWide(path));
#endif
}

static inline int remove_utf8(const char *path)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(unlink(path));
#else
  return(NTRemoveWide(path));
#endif
}

static inline int rename_utf8(const char *source,const char *destination)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(rename(source,destination));
#else
  return(NTRenameWide(source,destination));
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

  timestamp[0].tv_sec=attributes->st_atim.tv_sec;
  timestamp[0].tv_nsec=attributes->st_atim.tv_nsec;
  timestamp[1].tv_sec=attributes->st_mtim.tv_sec;
  timestamp[1].tv_nsec=attributes->st_mtim.tv_nsec;
  status=utimensat(AT_FDCWD,path,timestamp,0);
#else
  struct utimbuf
    timestamp;

  timestamp.actime=attributes->st_atime;
  timestamp.modtime=attributes->st_mtime;
  status=utime(path,&timestamp);
#endif
#else
  status=NTSetFileTimestamp(path,attributes);
#endif
  return(status);
}

static inline int stat_utf8(const char *path,struct stat *attributes)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(stat(path,attributes));
#else
  return(NTStatWide(path,attributes));
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
