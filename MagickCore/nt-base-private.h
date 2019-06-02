/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore Windows NT private methods.
*/
#ifndef MAGICKCORE_NT_BASE_PRIVATE_H
#define MAGICKCORE_NT_BASE_PRIVATE_H

#include "MagickCore/delegate.h"
#include "MagickCore/delegate-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/splay-tree.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT)

#if !defined(XS_VERSION)
struct dirent
{
  char
    d_name[2048];

  int
    d_namlen;
};

typedef struct _DIR
{
  HANDLE
    hSearch;

  WIN32_FIND_DATAW
    Win32FindData;

  BOOL
    firsttime;

  struct dirent
    file_info;
} DIR;

typedef struct _NTMEMORYSTATUSEX
{
  DWORD
    dwLength,
    dwMemoryLoad;

  DWORDLONG
    ullTotalPhys,
    ullAvailPhys,
    ullTotalPageFile,
    ullAvailPageFile,
    ullTotalVirtual,
    ullAvailVirtual,
    ullAvailExtendedVirtual;
} NTMEMORYSTATUSEX;

#if !defined(__MINGW32__)
struct timeval;

struct timezone
{
  int
    tz_minuteswest,
    tz_dsttime;
};
#endif

typedef UINT
  (CALLBACK *LPFNDLLFUNC1)(DWORD,UINT);

typedef UINT
  (CALLBACK *LPFNDLLFUNC2)(NTMEMORYSTATUSEX *);

#endif

#if defined(MAGICKCORE_BZLIB_DELEGATE)
#  if defined(_WIN32)
#    define BZ_IMPORT 1
#  endif
#endif

extern MagickPrivate char
  *NTGetLastError(void);

#if !defined(MAGICKCORE_LTDL_DELEGATE)
extern MagickPrivate const char
  *NTGetLibraryError(void);
#endif

#if !defined(XS_VERSION)
extern MagickPrivate const char
  *NTGetLibraryError(void);

extern MagickPrivate DIR
  *NTOpenDirectory(const char *);

extern MagickPrivate double
  NTElapsedTime(void),
  NTErf(double),
  NTUserTime(void);

extern MagickPrivate int
  Exit(int),
#if !defined(__MINGW32__)
  gettimeofday(struct timeval *,struct timezone *),
#endif
  IsWindows95(void),
  NTCloseDirectory(DIR *),
  NTCloseLibrary(void *),
  NTControlHandler(void),
  NTExitLibrary(void),
  NTTruncateFile(int,off_t),
  NTGhostscriptDLL(char *,int),
  NTGhostscriptEXE(char *,int),
  NTGhostscriptFonts(char *,int),
  NTGhostscriptLoadDLL(void),
  NTInitializeLibrary(void),
  NTSetSearchPath(const char *),
  NTSyncMemory(void *,size_t,int),
  NTUnmapMemory(void *,size_t),
  NTSystemCommand(const char *,char *);

extern MagickPrivate ssize_t
  NTGetPageSize(void),
  NTSystemConfiguration(int),
  NTTellDirectory(DIR *);

extern MagickPrivate MagickBooleanType
  NTGatherRandomData(const size_t,unsigned char *),
  NTGetExecutionPath(char *,const size_t),
  NTGetModulePath(const char *,char *),
  NTReportEvent(const char *,const MagickBooleanType);

extern MagickExport MagickBooleanType
  NTLongPathsEnabled();

extern MagickPrivate struct dirent
  *NTReadDirectory(DIR *);

extern MagickPrivate unsigned char
  *NTRegistryKeyLookup(const char *),
  *NTResourceToBlob(const char *);

extern MagickPrivate void
  *NTGetLibrarySymbol(void *,const char *),
  NTInitializeWinsock(MagickBooleanType),
  *NTMapMemory(char *,size_t,int,int,int,MagickOffsetType),
  *NTOpenLibrary(const char *),
  NTSeekDirectory(DIR *,ssize_t),
  NTWindowsGenesis(void),
  NTWindowsTerminus(void);

#endif /* !XS_VERSION */

#endif /* MAGICKCORE_WINDOWS_SUPPORT */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* !C++ */

#endif /* !MAGICKCORE_NT_BASE_H */
