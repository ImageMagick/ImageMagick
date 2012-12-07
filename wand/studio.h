/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand private application programming interface declarations.
*/
#ifndef _MAGICKWAND_STUDIO_H
#define _MAGICKWAND_STUDIO_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(WIN32) || defined(WIN64)
#  define MAGICKCORE_WINDOWS_SUPPORT
#else
#  define MAGICKCORE_POSIX_SUPPORT
#endif 

#define MAGICKWAND_IMPLEMENTATION  1

#if !defined(_MAGICKWAND_CONFIG_H)
# define _MAGICKWAND_CONFIG_H
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
#if defined(_magickcore_restrict) && !defined(restrict)
# define restrict  _magickcore_restrict
#endif
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
#endif

#if !defined(const)
#  define STDC
#endif

#if defined(__cplusplus) || defined(c_plusplus)
# define storage_class  c_class
#else
# define storage_class  class
#endif

#include <stdarg.h>
#include <stdio.h>
#if defined(MAGICKCORE_HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif
#if defined(MAGICKCORE_STDC_HEADERS)
# include <stdlib.h>
# include <stddef.h>
#else
# if defined(MAGICKCORE_HAVE_STDLIB_H)
#  include <stdlib.h>
# endif
#endif
#if defined(MAGICKCORE_HAVE_STRING_H)
# if !defined(STDC_HEADERS) && defined(MAGICKCORE_HAVE_MEMORY_H)
#  include <memory.h>
# endif
# include <string.h>
#endif
#if defined(MAGICKCORE_HAVE_STRINGS_H)
# include <strings.h>
#endif
#if defined(MAGICKCORE_HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#if defined(MAGICKCORE_HAVE_STDINT_H)
# include <stdint.h>
#endif
#if defined(MAGICKCORE_HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
# include <direct.h>
# if !defined(MAGICKCORE_HAVE_STRERROR)
#  define HAVE_STRERROR
# endif
#endif

#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <assert.h>

#if defined(MAGICKCORE_HAVE_XLOCALE_H)
# include <xlocale.h>
#endif
#if defined(MAGICKCORE_THREAD_SUPPORT)
# include <pthread.h>
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
#  define MAGICKCORE_HAVE_WINTHREADS  1
#include <windows.h>
#endif
#if defined(MAGICKCORE_HAVE_SYS_SYSLIMITS_H)
# include <sys/syslimits.h>
#endif
#if defined(MAGICKCORE_HAVE_ARM_LIMITS_H)
# include <arm/limits.h>
#endif

#if defined(MAGICKCORE__OPENCL)
#if defined(MAGICKCORE_HAVE_CL_CL_H)
#  include <CL/cl.h>
#endif
#if defined(MAGICKCORE_HAVE_OPENCL_CL_H)
#  include <OpenCL/cl.h>
#endif
#  define MAGICKCORE_OPENCL_SUPPORT  1
#endif

#if defined(_OPENMP) && ((_OPENMP >= 200203) || defined(__OPENCC__))
#  include <omp.h>
#  define MAGICKCORE_OPENMP_SUPPORT  1
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

#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(MAGICKCORE_POSIX_SUPPORT)
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
# include "wand/MagickWand.h"
# if !defined(MAGICKCORE_WINDOWS_SUPPORT)
#  include <sys/time.h>
# if defined(MAGICKCORE_HAVE_SYS_TIMES_H)
#  include <sys/times.h>
# endif
# if defined(MAGICKCORE_HAVE_SYS_RESOURCE_H)
#  include <sys/resource.h>
# endif
# if defined(MAGICKCORE_HAVE_SYS_MMAN_H)
#  include <sys/mman.h>
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
#endif

#if defined(S_IRUSR) && defined(S_IWUSR)
# define S_MODE (S_IRUSR | S_IWUSR)
#elif defined (MAGICKCORE_WINDOWS_SUPPORT)
# define S_MODE (_S_IREAD | _S_IWRITE)
#else
# define S_MODE  0600
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
# include "magick/nt-base.h"
#endif
#if defined(macintosh)
# include "magick/mac.h"
#endif
#if defined(vms)
# include "magick/vms.h"
#endif

#include "wand/MagickWand.h"

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
# if defined(MAGICKCORE_WINDOWS_SUPPORT)
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

#if !defined(PATH_MAX)
#define PATH_MAX  4096
#endif

/*
  Exception defines.
*/
#define ThrowWandFatalException(severity,tag,context) \
{ \
  ExceptionInfo \
    *exception; \
 \
  exception=AcquireExceptionInfo(); \
  (void) ThrowMagickException(exception,GetMagickModule(),severity,tag, \
    "`%s'",context); \
  CatchException(exception); \
  exception=DestroyExceptionInfo(exception); \
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
