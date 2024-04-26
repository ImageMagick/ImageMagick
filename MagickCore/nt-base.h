/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore Windows NT utility methods.
*/
#ifndef MAGICKCORE_NT_BASE_H
#define MAGICKCORE_NT_BASE_H

#include "MagickCore/exception.h"
#include "MagickCore/geometry.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT)

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#  define _CRT_SECURE_NO_DEPRECATE  1
#endif
#include <windows.h>
#include <wchar.h>
#include <winuser.h>
#include <wingdi.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <malloc.h>
#include <sys/utime.h>
#if defined(_DEBUG) && !defined(__MINGW32__)
#include <crtdbg.h>
#endif

#define PROT_READ  0x01
#define PROT_WRITE  0x02
#define MAP_SHARED  0x01
#define MAP_PRIVATE  0x02
#define MAP_ANONYMOUS  0x20
#define F_OK 0
#define R_OK 4
#define W_OK 2
#define RW_OK 6
#define _SC_PAGE_SIZE 1
#define _SC_PHYS_PAGES 2
#define _SC_OPEN_MAX 3
#ifdef _WIN64
#  if !defined(SSIZE_MAX)
#    define SSIZE_MAX LLONG_MAX
#  endif
#else
#  if !defined(SSIZE_MAX)
#    define SSIZE_MAX LONG_MAX
#  endif
#endif
#ifndef S_ISCHR
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif

#if defined(_MSC_VER)
# if !defined(MAGICKCORE_MSC_VER)
#   if (_MSC_VER >= 1930)
#     define MAGICKCORE_MSC_VER 2022
#   elif (_MSC_VER >= 1920)
#     define MAGICKCORE_MSC_VER 2019
#   elif (_MSC_VER >= 1910)
#     define MAGICKCORE_MSC_VER 2017
#   endif
# endif
#endif

#if !defined(closedir)
#  define closedir(directory)  NTCloseDirectory(directory)
#endif
#if !defined(freelocale)
#  define freelocale  _free_locale
#endif
#if !defined(fsync)
#  define fsync  _commit
#endif
#if !defined(locale_t)
#define locale_t _locale_t
#endif
#if !defined(MAGICKCORE_LTDL_DELEGATE)
#if !defined(lt_dlclose)
#  define lt_dlclose(handle)  NTCloseLibrary(handle)
#endif
#if !defined(lt_dlerror)
#  define lt_dlerror()  NTGetLibraryError()
#endif
#if !defined(lt_dlopen)
#  define lt_dlopen(filename)  NTOpenLibrary(filename)
#endif
#if !defined(lt_dlsym)
#  define lt_dlsym(handle,name)  NTGetLibrarySymbol(handle,name)
#endif
#endif
#if !defined(mmap)
#  define MAGICKCORE_HAVE_MMAP 1
#  define mmap(address,length,protection,access,file,offset) \
  NTMapMemory(address,length,protection,access,file,offset)
#endif
#if !defined(munmap)
#  define munmap(address,length)  NTUnmapMemory(address,length)
#endif
#if !defined(opendir)
#  define opendir(directory)  NTOpenDirectory(directory)
#endif
#if !defined(pclose)
#  define pclose  _pclose
#endif
#if !defined(popen)
#  define popen  _popen
#endif
#if !defined(read)
#  define read(fd,buffer,count)  _read(fd,buffer,(unsigned int) count)
#endif
#if !defined(readdir)
#  define readdir(directory)  NTReadDirectory(directory)
#endif
#if !defined(strtod_l)
#define strtod_l  _strtod_l
#endif
#if !defined(strcasecmp)
#  define strcasecmp  _stricmp
#endif
#if !defined(strncasecmp)
#  define strncasecmp  _strnicmp
#endif
#if !defined(sysconf)
#  define sysconf(name)  NTSystemConfiguration(name)
#  define MAGICKCORE_HAVE_SYSCONF 1
#endif
#if !defined(vfprintf_l)
#define vfprintf_l  _vfprintf_l
#endif
#if !defined(vsnprintf_l)
#define vsnprintf_l  _vsnprintf_l
#endif
#if !defined(write)
#  define write(fd,buffer,count)  _write(fd,buffer,(unsigned int) count)
#endif
#if !defined(__MINGW32__)
#  if !defined(fstat)
#    define fstat  _fstati64
#  endif
#  if !defined(stat)
#    define stat  _stati64
#  endif
#endif

#if defined(__BORLANDC__)
#undef _O_RANDOM
#define _O_RANDOM 0
#undef _O_SEQUENTIAL
#define _O_SEQUENTIAL 0
#undef _O_SHORT_LIVED
#define _O_SHORT_LIVED 0
#undef _O_TEMPORARY
#define _O_TEMPORARY 0
#endif

typedef struct _GhostInfo
  GhostInfo_;

extern MagickExport char
  **NTArgvToUTF8(const int argc,wchar_t **);

extern MagickExport const GhostInfo_
  *NTGhostscriptDLLVectors(void);

extern MagickExport void
  NTErrorHandler(const ExceptionType,const char *,const char *),
  NTGhostscriptUnLoadDLL(void),
  NTWarningHandler(const ExceptionType,const char *,const char *);

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
