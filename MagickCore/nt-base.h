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
#  if defined(_MSC_VER)
#    if !defined(MAGICKCORE_SIZEOF_SSIZE_T)
#      define MAGICKCORE_SIZEOF_SSIZE_T 8
#    endif
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
