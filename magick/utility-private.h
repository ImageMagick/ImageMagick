/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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

/*
  Windows UTF8 compatibility methods.
*/

static inline int access_utf8(const char *path,int mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(access(path,mode));
#else
   int
     count,
     status;

   WCHAR
     *path_wide;

   path_wide=(WCHAR *) NULL;
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,NULL,0);
   path_wide=(WCHAR *) AcquireQuantumMemory(count,sizeof(*path_wide));
   if (path_wide == (WCHAR *) NULL)
     return(-1);
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,path_wide,count);
   status=_waccess(path_wide,mode);
   path_wide=RelinquishMagickMemory(path_wide);
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

   WCHAR
     *mode_wide,
     *path_wide;

   path_wide=(WCHAR *) NULL;
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,NULL,0);
   path_wide=(WCHAR *) AcquireQuantumMemory(count,sizeof(*path_wide));
   if (path_wide == (WCHAR *) NULL)
     return(-1);
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,path_wide,count);
   count=MultiByteToWideChar(CP_UTF8,0,mode,-1,NULL,0);
   mode_wide=(WCHAR *) AcquireQuantumMemory(count,sizeof(*mode_wide));
   if (mode_wide == (WCHAR *) NULL)
     {
       path_wide=RelinquishMagickMemory(path_wide);
       return(-1);
     }
   count=MultiByteToWideChar(CP_UTF8,0,mode,-1,mode_wide,count);
   file=_wfopen(path_wide,mode_width);
   mode_wide=RelinquishMagickMemory(mode_wide);
   path_wide=RelinquishMagickMemory(path_wide);
   return(file);
#endif
}

static inline int open_utf8(const char *path,int flags,int mode)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(open(path,flags,mode));
#else
   int
     count,
     status;

   WCHAR
     *path_wide;

   path_wide=(WCHAR *) NULL;
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,NULL,0);
   path_wide=(WCHAR *) AcquireQuantumMemory(count,sizeof(*path_wide));
   if (path_wide == (WCHAR *) NULL)
     return(-1);
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,path_wide,count);
   status=_wopen(path_wide,flags,mode);
   path_wide=RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

static inline int remove_utf8(const char *path)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(unlink(path));
#else
   int
     count,
     status;

   WCHAR
     *path_wide;

   path_wide=(WCHAR *) NULL;
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,NULL,0);
   path_wide=(WCHAR *) AcquireQuantumMemory(count,sizeof(*path_wide));
   if (path_wide == (WCHAR *) NULL)
     return(-1);
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,path_wide,count);
   status=_wremove(path_wide);
   path_wide=RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

static inline int stat_utf8(const char *path,struct stat *attributes)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__)
  return(stat(path,attributes));
#else
   int
     count,
     status;

   WCHAR
     *path_wide;

   path_wide=(WCHAR *) NULL;
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,NULL,0);
   path_wide=(WCHAR *) AcquireQuantumMemory(count,sizeof(*path_wide));
   if (path_wide == (WCHAR *) NULL)
     return(-1);
   count=MultiByteToWideChar(CP_UTF8,0,path,-1,path_wide,count);
   status=_wstat(path_wide,attributes);
   path_wide=RelinquishMagickMemory(path_wide);
   return(status);
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
