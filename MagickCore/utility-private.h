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

#include <errno.h>
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

#define MagickPathTemplate "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  /* min 6 X's */

#define MagickMaxIOChunk  ((size_t) 0x7ffff000)

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

static inline int access_utf8(const char *path,int mode)
{
  if (path == (const char *) NULL)
    {
      errno=EINVAL;
      return(-1);
    }
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
  if ((path == (const char *) NULL) || (mode == (const char *) NULL))
    {
      errno=EINVAL;
      return((FILE *) NULL);
    }
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(fopen(path,mode));
#else
  return(NTOpenFileWide(path,mode));
#endif
}

static inline MagickBooleanType is_symlink_utf8(const char *path)
{
  if (path == (const char *) NULL)
    return(MagickFalse);
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#if defined(MAGICKCORE_POSIX_SUPPORT)
  {
    struct stat
      status;

    if (lstat(path,&status) == -1)
      return(MagickFalse);
    return(S_ISLNK(status.st_mode) != 0 ? MagickTrue : MagickFalse);
  }
#else
  return(MagickFalse);
#endif
#else
  return(NTIsSymlinkWide(path));
#endif
}

static inline ssize_t MagickRead(int fd,void *buffer,size_t extent)
{
  unsigned char
    *p;

  size_t
    offset;

  /*
    Read exactly `extent` bytes unless EOF or an error occurs.
  */
  if ((buffer == (void *) NULL) && (extent != 0))
    {
      errno=EINVAL;
      return(-1);
    }
  if (extent > (size_t) MAGICK_SSIZE_MAX)
    {
      errno=EINVAL;  /* return value could not be represented as ssize_t */
      return(-1);
    }
  p=(unsigned char *) buffer;
  offset=0;
  while (offset < extent)
  {
    size_t
      chunk;

    ssize_t
      count;

    chunk=extent-offset;
    if (chunk > MagickMaxIOChunk)
      chunk=MagickMaxIOChunk;
    count=read(fd,p+offset,chunk);
    if (count < 0)
      {
        if (errno == EINTR)
          continue;
        return(-1);
      }
    if (count == 0)
      break;
    offset+=(size_t) count;
  }
  return((ssize_t) offset);
}

static inline int MagickReadDirectory(DIR *directory,struct dirent *entry,
  struct dirent **result)
{
  /*
    Returns the next directory entry via *result.
  */
  (void) entry;
  if ((directory == (DIR *) NULL) || (result == (struct dirent **) NULL))
    return(EINVAL);
  errno=0;
  *result=readdir(directory);
  return(*result != (struct dirent *) NULL ? 0 : errno);
}

static inline ssize_t MagickWrite(int fd,const void *buffer,size_t extent)
{
  const unsigned char
    *p;

  size_t
    offset;

  /*
    Write exactly `extent` bytes unless an error occurs.
  */
  if ((buffer == (const void *) NULL) && (extent != 0))
    {
      errno=EINVAL;
      return(-1);
    }
  if (extent > (size_t) MAGICK_SSIZE_MAX)
    {
      errno=EINVAL;
      return(-1);
    }
  p=(const unsigned char *) buffer;
  offset=0;
  while (offset < extent)
  {
    size_t
      chunk;

    ssize_t
      count;

    chunk=extent-offset;
    if (chunk > MagickMaxIOChunk)
      chunk=MagickMaxIOChunk;
    count=write(fd,p+offset,chunk);
    if (count < 0)
      {
        if (errno == EINTR)
          continue;
        return(-1);
      }
    if (count == 0)
      {
        errno=EIO;  /* no progress and no error: avoid a silent short write */
        break;
      }
    offset+=(size_t) count;
  }
  return((ssize_t) offset);
}

static inline int open_utf8(const char *path,int flags,mode_t mode)
{
  if (path == (const char *) NULL)
    {
      errno=EINVAL;
      return(-1);
    }
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(open(path,flags,mode));
#else
  return(NTOpenWide(path,flags,mode));
#endif
}

static inline FILE *popen_utf8(const char *command,const char *type)
{
  /*
    Caution: `command` is interpreted by the shell.
  */
  if ((command == (const char *) NULL) || (type == (const char *) NULL))
    {
      errno=EINVAL;
      return((FILE *) NULL);
    }
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(popen(command,type));
#else
  return(NTOpenPipeWide(command,type));
#endif
}

static inline char *realpath_utf8(const char *path)
{
  /*
    The returned string is owned by the caller.
  */
  if (path == (const char *) NULL)
    {
      errno=EINVAL;
      return((char *) NULL);
    }
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#if defined(MAGICKCORE_HAVE_REALPATH)
  /*
    This does not work for non-existing files.
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
  if (path == (const char *) NULL)
    {
      errno=EINVAL;
      return(-1);
    }
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
  return(unlink(path));
#else
  return(NTRemoveWide(path));
#endif
}

static inline int rename_utf8(const char *source,const char *destination)
{
  if ((source == (const char *) NULL) || (destination == (const char *) NULL))
    {
      errno=EINVAL;
      return(-1);
    }
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

  if ((path == (const char *) NULL) || (attributes == (struct stat *) NULL))
    {
      errno=EINVAL;
      return(-1);
    }
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#if defined(MAGICKCORE_HAVE_UTIMENSAT)
  {
    struct timespec
      timestamp[2];

#if defined(__APPLE__) || defined(__NetBSD__)
    timestamp[0]=attributes->st_atimespec;
    timestamp[1]=attributes->st_mtimespec;
#else
    timestamp[0]=attributes->st_atim;
    timestamp[1]=attributes->st_mtim;
#endif
    status=utimensat(AT_FDCWD,path,timestamp,0);
  }
#else
  {
    struct utimbuf
      timestamp;

    timestamp.actime=attributes->st_atime;
    timestamp.modtime=attributes->st_mtime;
    status=utime(path,&timestamp);
  }
#endif
#else
  status=NTSetFileTimestamp(path,attributes);
#endif
  return(status);
}

static inline int stat_utf8(const char *path,struct stat *attributes)
{
  if ((path == (const char *) NULL) || (attributes == (struct stat *) NULL))
    {
      errno=EINVAL;
      return(-1);
    }
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
