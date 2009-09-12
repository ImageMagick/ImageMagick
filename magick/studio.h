/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private application programming interface declarations.
*/
#ifndef _MAGICKCORE_STUDIO_H
#define _MAGICKCORE_STUDIO_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__CYGWIN32__)
#  if !defined(__CYGWIN__)
#    define __CYGWIN__ __CYGWIN32__
#  endif
#endif

#if defined(_WIN32) || defined(WIN32)
#  if !defined(__WINDOWS__)
#    if defined(_WIN32)
#      define __WINDOWS__ _WIN32
#    else
#      if defined(WIN32)
#        define __WINDOWS__ WIN32
#      endif
#    endif
#  endif
#endif

#if defined(_WIN64) || defined(WIN64)
#  if !defined(__WINDOWS__)
#    if defined(_WIN64)
#      define __WINDOWS__ _WIN64
#    else
#      if defined(WIN64)
#        define __WINDOWS__ WIN64
#      endif
#    endif
#  endif
#endif

#define _BIND_TO_CURRENT_VCLIBS_VERSION 1

#if !defined(vms) && !defined(macintosh) && !defined(__WINDOWS__)
# define MAGICKCORE_POSIX_SUPPORT
#endif

#define MAGICKCORE_IMPLEMENTATION  1

#if !defined(_MAGICKCORE_CONFIG_H)
# define _MAGICKCORE_CONFIG_H
# if !defined(vms) && !defined(macintosh)
#  include "magick/magick-config.h"
# else
#  include "magick-config.h"
# endif
#if defined(MAGICKCORE__FILE_OFFSET_BITS) && !defined(_FILE_OFFSET_BITS)
# define _FILE_OFFSET_BITS MAGICKCORE__FILE_OFFSET_BITS
#endif
#if defined(_magickcore_const) && !defined(const)
# define const _magickcore_const
#endif
#if defined(_magickcore_inline) && !defined(inline)
# define inline _magickcore_inline
#endif
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
#if defined(_magickcore_restrict) && !defined(__restrict)
# define __restrict _magickcore_restrict
#endif
#endif

#if defined(MAGICKCORE_NAMESPACE_PREFIX)
# include "magick/methods.h"
#endif

#if !defined(const)
#  define STDC
#endif

#if defined(__BORLANDC__) && defined(_DLL)
#  pragma message("BCBMagick lib DLL export interface")
#  define _MAGICKDLL_
#  define _MAGICKLIB_
#  define MAGICKCORE_MODULES_SUPPORT
#  undef MAGICKCORE_BUILD_MODULES
#endif

#if defined(__WINDOWS__)
# if defined(_MT) && defined(_DLL) && !defined(_MAGICKDLL_) && !defined(_LIB)
#  define _MAGICKDLL_
# endif
# if defined(_MAGICKDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_MAGICKLIB_)
#   define MagickExport  __declspec(dllimport)
#   if defined(_VISUALC_)
#    pragma message( "MagickCore lib DLL import interface" )
#   endif
#  else
#   define MagickExport  __declspec(dllexport)
#   if defined(_VISUALC_)
#    pragma message( "MagickCore lib DLL export interface" )
#   endif
#  endif
# else
#  define MagickExport
#  if defined(_VISUALC_)
#   pragma message( "MagickCore lib static interface" )
#  endif
# endif

# if defined(_DLL) && !defined(_LIB)
#  define ModuleExport  __declspec(dllexport)
#  if defined(_VISUALC_)
#   pragma message( "MagickCore module DLL export interface" )
#  endif
# else
#  define ModuleExport
#  if defined(_VISUALC_)
#   pragma message( "MagickCore module static interface" )
#  endif

# endif
# define MagickGlobal __declspec(thread)
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4068)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
#  pragma warning(disable : 4996)
# endif
#else
# define MagickExport
# define ModuleExport
# define MagickGlobal
#endif

#define MagickSignature  0xabacadabUL
#if !defined(MaxTextExtent)
# define MaxTextExtent  4096
#endif

#include <stdarg.h>
#include <stdio.h>
#if defined(__WINDOWS__) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#if !defined(__WINDOWS__)
# include <unistd.h>
#else
# include <direct.h>
# if !defined(MAGICKCORE_HAVE_STRERROR)
#  define HAVE_STRERROR
# endif
#endif

#if defined(MAGICKCORE_HAVE_STRINGS_H)
# include <strings.h>
#endif
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <assert.h>

#if defined(MAGICKCORE_HAVE_PTHREAD)
# include <pthread.h>
#elif defined(__WINDOWS__)
#  define MAGICKORE_HAVE_WINTHREADS  1
#include <windows.h>
#endif
#if defined(MAGICKCORE_HAVE_SYS_SYSLIMITS_H)
# include <sys/syslimits.h>
#endif
#if defined(MAGICKCORE_HAVE_ARM_LIMITS_H)
# include <arm/limits.h>
#endif

#if defined(_OPENMP) && (_OPENMP >= 200203)
#  include <omp.h>
#  define MAGICKCORE_OPENMP_SUPPORT 1
#endif

#if defined(MAGICKCORE_HAVE_PREAD) && defined(MAGICKCORE_HAVE_DECL_PREAD) && !MAGICKCORE_HAVE_DECL_PREAD
ssize_t pread(int,void *,size_t,off_t);
#endif

#if defined(MAGICKCORE_HAVE_PWRITE) && defined(MAGICKCORE_HAVE_DECL_PWRITE) && !MAGICKCORE_HAVE_DECL_PWRITE
ssize_t pwrite(int,const void *,size_t,off_t);
#endif

#if defined(MAGICKCORE_HAVE_STRLCPY) && defined(MAGICKCORE_HAVE_DECL_STRLCPY) && !MAGICKCORE_HAVE_DECL_STRLCPY
extern size_t strlcpy(char *,const char *,size_t);
#endif

#if defined(MAGICKCORE_HAVE_VSNPRINTF) && defined(MAGICKCORE_HAVE_DECL_VSNPRINTF) && !MAGICKCORE_HAVE_DECL_VSNPRINTF
extern int vsnprintf(char *,size_t,const char *,va_list);
#endif

#if !defined(magick_attribute)
#  if !defined(__GNUC__)
#    define magick_attribute(x)  /* nothing */
#  else
#    define magick_attribute  __attribute__
#  endif
#endif

#if !defined(magick_unused)
#  if defined(__GNUC__)
#     define magick_unused(x)  magick_unused_ ## x __attribute__((unused))
#  elif defined(__LCLINT__)
#    define magick_unused(x) /*@unused@*/ x
#  else
#    define magick_unused(x) x
#  endif
#endif

#if defined(__WINDOWS__) || defined(MAGICKCORE_POSIX_SUPPORT)
# include <sys/types.h>
# include <sys/stat.h>
# if defined(MAGICKCORE_HAVE_FTIME)
# include <sys/timeb.h>
# endif
# if defined(MAGICKCORE_POSIX_SUPPORT)
#  if defined(MAGICKCORE_HAVE_SYS_NDIR_H) || defined(MAGICKCORE_HAVE_SYS_DIR_H) || defined(MAGICKCORE_HAVE_NDIR_H)
#   define dirent direct
#   define NAMLEN(dirent) (dirent)->d_namlen
#   if defined(MAGICKCORE_HAVE_SYS_NDIR_H)
#    include <sys/ndir.h>
#   endif
#   if defined(MAGICKCORE_HAVE_SYS_DIR_H)
#    include <sys/dir.h>
#   endif
#   if defined(MAGICKCORE_HAVE_NDIR_H)
#    include <ndir.h>
#   endif
#  else
#   include <dirent.h>
#   define NAMLEN(dirent) strlen((dirent)->d_name)
#  endif
#  include <sys/wait.h>
#  include <pwd.h>
# endif
# if !defined(S_ISDIR)
#  define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
# endif
# if !defined(S_ISREG)
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
# include "magick/magick-type.h"
# if !defined(__WINDOWS__)
#  include <sys/time.h>
# if defined(MAGICKCORE_HAVE_SYS_TIMES_H)
#  include <sys/times.h>
# endif
# if defined(MAGICKCORE_HAVE_SYS_RESOURCE_H)
#  include <sys/resource.h>
# endif
#endif
#else
# include <types.h>
# include <stat.h>
# if defined(macintosh)
#  if !defined(DISABLE_SIOUX)
#   include <SIOUX.h>
#   include <console.h>
#  endif
#  include <unix.h>
# endif
# include "magick/magick-type.h"
#endif

#if defined(S_IRUSR) && defined(S_IWUSR)
# define S_MODE (S_IRUSR | S_IWUSR)
#elif defined (__WINDOWS__)
# define S_MODE (_S_IREAD | _S_IWRITE)
#else
# define S_MODE  0600
#endif

#if defined(__WINDOWS__)
# include "magick/nt-base.h"
#endif
#if defined(macintosh)
# include "magick/mac.h"
#endif
#if defined(vms)
# include "magick/vms.h"
#endif

#undef HAVE_CONFIG_H
#undef gamma
#undef index
#undef pipe
#undef y1

/*
  Review these platform specific definitions.
*/
#if defined(MAGICKCORE_POSIX_SUPPORT) && !defined(__OS2__)
# define DirectorySeparator  "/"
# define DirectoryListSeparator  ':'
# define EditorOptions  " -title \"Edit Image Comment\" -e vi"
# define Exit  exit
# define IsBasenameSeparator(c)  ((c) == '/' ? MagickTrue : MagickFalse)
# define X11_PREFERENCES_PATH  "~/."
# define ProcessPendingEvents(text)
# define ReadCommandlLine(argc,argv)
# define SetNotifyHandlers
#else
# if defined(vms)
#  define X11_APPLICATION_PATH  "decw$system_defaults:"
#  define DirectorySeparator  ""
#  define DirectoryListSeparator  ';'
#  define EditorOptions  ""
#  define Exit  exit
#  define IsBasenameSeparator(c) \
  (((c) == ']') || ((c) == ':') || ((c) == '/') ? MagickTrue : MagickFalse)
#  define MAGICKCORE_LIBRARY_PATH  "sys$login:"
#  define MAGICKCORE_CODER_PATH  "sys$login:"
#  define MAGICKCORE_FILTER_PATH  "sys$login:"
#  define MAGICKCORE_SHARE_PATH  "sys$login:"
#  define X11_PREFERENCES_PATH  "decw$user_defaults:"
#  define ProcessPendingEvents(text)
#  define ReadCommandlLine(argc,argv)
#  define SetNotifyHandlers
# endif
# if defined(__OS2__)
#   define DirectorySeparator  "\\"
#   define DirectoryListSeparator  ';'
# define EditorOptions  " -title \"Edit Image Comment\" -e vi"
# define Exit  exit
#  define IsBasenameSeparator(c) \
  (((c) == '/') || ((c) == '\\') ? MagickTrue : MagickFalse)
# define PreferencesDefaults  "~\."
# define ProcessPendingEvents(text)
# define ReadCommandlLine(argc,argv)
# define SetNotifyHandlers
#endif
# if defined(macintosh)
#  define X11_APPLICATION_PATH  "/usr/lib/X11/app-defaults/"
#  define DirectorySeparator  ":"
#  define DirectoryListSeparator  ';'
#  define EditorOptions ""
#  define IsBasenameSeparator(c)  ((c) == ':' ? MagickTrue : MagickFalse)
#  define MAGICKCORE_LIBRARY_PATH  ""
#  define MAGICKCORE_CODER_PATH  ""
#  define MAGICKCORE_FILTER_PATH  ""
#  define MAGICKCORE_SHARE_PATH  ""
#  define X11_PREFERENCES_PATH  "~/."
#  if defined(DISABLE_SIOUX)
#   define ReadCommandlLine(argc,argv)
#   define SetNotifyHandlers \
     SetFatalErrorHandler(MacFatalErrorHandler); \
     SetErrorHandler(MACErrorHandler); \
     SetWarningHandler(MACWarningHandler)
#  else
#   define ReadCommandlLine(argc,argv) argc=ccommand(argv); puts(MagickVersion);
#   define SetNotifyHandlers \
     SetErrorHandler(MACErrorHandler); \
     SetWarningHandler(MACWarningHandler)
#  endif
# endif
# if defined(__WINDOWS__)
#  define DirectorySeparator  "\\"
#  define DirectoryListSeparator  ';'
#  define EditorOptions ""
#  define IsBasenameSeparator(c) \
  (((c) == '/') || ((c) == '\\') ? MagickTrue : MagickFalse)
#  define ProcessPendingEvents(text)
#  if !defined(X11_PREFERENCES_PATH)
#    define X11_PREFERENCES_PATH  "~\\."
#  endif
#  define ReadCommandlLine(argc,argv)
#  define SetNotifyHandlers \
    SetErrorHandler(NTErrorHandler); \
    SetWarningHandler(NTWarningHandler)
#  undef sleep
#  define sleep(seconds)  Sleep(seconds*1000)
#  if !defined(MAGICKCORE_HAVE_TIFFCONF_H)
#    define HAVE_TIFFCONF_H
#  endif
# endif

#endif

/*
  Define system symbols if not already defined.
*/
#if !defined(STDIN_FILENO)
#define STDIN_FILENO  0x00
#endif

#if !defined(O_BINARY)
#define O_BINARY  0x00
#endif

#if defined(MAGICKCORE_LTDL_DELEGATE) || (defined(__WINDOWS__) && defined(_DLL) && !defined(_LIB))
#  define MAGICKCORE_MODULES_SUPPORT
#endif

#if defined(_MAGICKMOD_)
# undef MAGICKCORE_BUILD_MODULES
# define MAGICKCORE_BUILD_MODULES
#endif

/*
  I/O defines.
*/
#if defined(__WINDOWS__) && !defined(Windows95) && !defined(__BORLANDC__)
#define MagickSeek(file,offset,whence)  _lseeki64(file,offset,whence)
#define MagickTell(file)  _telli64(file)
#else
#define MagickSeek(file,offset,whence)  lseek(file,offset,whence)
#define MagickTell(file) tell(file)
#endif

/*
  Magick defines.
*/
#define Swap(x,y) ((x)^=(y), (y)^=(x), (x)^=(y))

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
