#ifndef _MAGICK_MAGICK_CONFIG_H
#define _MAGICK_MAGICK_CONFIG_H 1
 
/* magick/magick-config.h. Generated automatically at end of configure. */
/* config/config.h.  Generated from config.h.in by configure.  */
/* config/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define if you have AUTOTRACE library */
/* #undef AUTOTRACE_DELEGATE */

/* Define if coders and filters are to be built as modules. */
/* #undef BUILD_MODULES */

/* Define if you have the bzip2 library */
#ifndef MAGICKCORE_BZLIB_DELEGATE
#define MAGICKCORE_BZLIB_DELEGATE 1
#endif

/* Define if you have CAIRO library */
#ifndef MAGICKCORE_CAIRO_DELEGATE
#define MAGICKCORE_CAIRO_DELEGATE 1
#endif

/* permit enciphering and deciphering image pixels */
#ifndef MAGICKCORE_CIPHER_SUPPORT
#define MAGICKCORE_CIPHER_SUPPORT 1
#endif

/* Define to 1 if the `closedir' function returns void instead of `int'. */
/* #undef CLOSEDIR_VOID */

/* Location of coder modules */
#ifndef MAGICKCORE_CODER_PATH
#define MAGICKCORE_CODER_PATH "/usr/local/lib/ImageMagick-6.6.2/modules-Q16/coders/"
#endif

/* Subdirectory of lib where coder modules are installed */
#ifndef MAGICKCORE_CODER_RELATIVE_PATH
#define MAGICKCORE_CODER_RELATIVE_PATH "ImageMagick-6.6.2/modules-Q16/coders"
#endif

/* Directory where architecture-dependent configuration files live. */
#ifndef MAGICKCORE_CONFIGURE_PATH
#define MAGICKCORE_CONFIGURE_PATH "/usr/local/lib/ImageMagick-6.6.2/config/"
#endif

/* Subdirectory of lib where architecture-dependent configuration files live.
   */
#ifndef MAGICKCORE_CONFIGURE_RELATIVE_PATH
#define MAGICKCORE_CONFIGURE_RELATIVE_PATH "ImageMagick-6.6.2/config"
#endif

/* Define if you have DJVU library */
#ifndef MAGICKCORE_DJVU_DELEGATE
#define MAGICKCORE_DJVU_DELEGATE 1
#endif

/* Directory where ImageMagick documents live. */
#ifndef MAGICKCORE_DOCUMENTATION_PATH
#define MAGICKCORE_DOCUMENTATION_PATH "/usr/local/share/doc/ImageMagick-6.6.2/"
#endif

/* Define if you have Display Postscript */
/* #undef DPS_DELEGATE */

/* Build self-contained, embeddable, zero-configuration ImageMagick */
/* #undef EMBEDDABLE_SUPPORT */

/* exclude deprecated methods in MagickCore API */
/* #undef EXCLUDE_DEPRECATED */

/* Directory where executables are installed. */
#ifndef MAGICKCORE_EXECUTABLE_PATH
#define MAGICKCORE_EXECUTABLE_PATH "/usr/local/bin/"
#endif

/* Define if you have FFTW library */
#ifndef MAGICKCORE_FFTW_DELEGATE
#define MAGICKCORE_FFTW_DELEGATE 1
#endif

/* Location of filter modules */
#ifndef MAGICKCORE_FILTER_PATH
#define MAGICKCORE_FILTER_PATH "/usr/local/lib/ImageMagick-6.6.2/modules-Q16/filters/"
#endif

/* Subdirectory of lib where filter modules are installed */
#ifndef MAGICKCORE_FILTER_RELATIVE_PATH
#define MAGICKCORE_FILTER_RELATIVE_PATH "ImageMagick-6.6.2/modules-Q16/filters"
#endif

/* Define if you have FONTCONFIG library */
#ifndef MAGICKCORE_FONTCONFIG_DELEGATE
#define MAGICKCORE_FONTCONFIG_DELEGATE 1
#endif

/* Define if you have FlashPIX library */
/* #undef FPX_DELEGATE */

/* Define if you have FreeType (TrueType font) library */
#ifndef MAGICKCORE_FREETYPE_DELEGATE
#define MAGICKCORE_FREETYPE_DELEGATE 1
#endif

/* Define if you have Ghostscript library or framework */
/* #undef GS_DELEGATE */

/* Define if you have GVC library */
/* #undef GVC_DELEGATE */

/* Define to 1 if you have the `argz_add' function. */
#ifndef MAGICKCORE_HAVE_ARGZ_ADD
#define MAGICKCORE_HAVE_ARGZ_ADD 1
#endif

/* Define to 1 if you have the `argz_append' function. */
#ifndef MAGICKCORE_HAVE_ARGZ_APPEND
#define MAGICKCORE_HAVE_ARGZ_APPEND 1
#endif

/* Define to 1 if you have the `argz_count' function. */
#ifndef MAGICKCORE_HAVE_ARGZ_COUNT
#define MAGICKCORE_HAVE_ARGZ_COUNT 1
#endif

/* Define to 1 if you have the `argz_create_sep' function. */
#ifndef MAGICKCORE_HAVE_ARGZ_CREATE_SEP
#define MAGICKCORE_HAVE_ARGZ_CREATE_SEP 1
#endif

/* Define to 1 if you have the <argz.h> header file. */
#ifndef MAGICKCORE_HAVE_ARGZ_H
#define MAGICKCORE_HAVE_ARGZ_H 1
#endif

/* Define to 1 if you have the `argz_insert' function. */
#ifndef MAGICKCORE_HAVE_ARGZ_INSERT
#define MAGICKCORE_HAVE_ARGZ_INSERT 1
#endif

/* Define to 1 if you have the `argz_next' function. */
#ifndef MAGICKCORE_HAVE_ARGZ_NEXT
#define MAGICKCORE_HAVE_ARGZ_NEXT 1
#endif

/* Define to 1 if you have the `argz_stringify' function. */
#ifndef MAGICKCORE_HAVE_ARGZ_STRINGIFY
#define MAGICKCORE_HAVE_ARGZ_STRINGIFY 1
#endif

/* Define to 1 if you have the <arm/limits.h> header file. */
/* #undef HAVE_ARM_LIMITS_H */

/* Define to 1 if you have the `atexit' function. */
#ifndef MAGICKCORE_HAVE_ATEXIT
#define MAGICKCORE_HAVE_ATEXIT 1
#endif

/* define if bool is a built-in type */
#ifndef MAGICKCORE_HAVE_BOOL
#define MAGICKCORE_HAVE_BOOL /**/
#endif

/* Define to 1 if you have the `cimag' function. */
#ifndef MAGICKCORE_HAVE_CIMAG
#define MAGICKCORE_HAVE_CIMAG 1
#endif

/* Define to 1 if you have the `clock' function. */
#ifndef MAGICKCORE_HAVE_CLOCK
#define MAGICKCORE_HAVE_CLOCK 1
#endif

/* Define to 1 if you have clock_gettime. */
#ifndef MAGICKCORE_HAVE_CLOCK_GETTIME
#define MAGICKCORE_HAVE_CLOCK_GETTIME 1
#endif

/* Define to 1 if clock_gettime supports CLOCK_REALTIME. */
#ifndef MAGICKCORE_HAVE_CLOCK_REALTIME
#define MAGICKCORE_HAVE_CLOCK_REALTIME 1
#endif

/* Define to 1 if you have the `closedir' function. */
#ifndef MAGICKCORE_HAVE_CLOSEDIR
#define MAGICKCORE_HAVE_CLOSEDIR 1
#endif

/* Define to 1 if you have the <CL/cl.h> header file. */
/* #undef HAVE_CL_CL_H */

/* Define to 1 if you have the <complex.h> header file. */
#ifndef MAGICKCORE_HAVE_COMPLEX_H
#define MAGICKCORE_HAVE_COMPLEX_H 1
#endif

/* Define to 1 if you have the declaration of `cygwin_conv_path', and to 0 if
   you don't. */
/* #undef HAVE_DECL_CYGWIN_CONV_PATH */

/* Define to 1 if you have the declaration of `pread', and to 0 if you don't.
   */
#ifndef MAGICKCORE_HAVE_DECL_PREAD
#define MAGICKCORE_HAVE_DECL_PREAD 1
#endif

/* Define to 1 if you have the declaration of `pwrite', and to 0 if you don't.
   */
#ifndef MAGICKCORE_HAVE_DECL_PWRITE
#define MAGICKCORE_HAVE_DECL_PWRITE 1
#endif

/* Define to 1 if you have the declaration of `strlcpy', and to 0 if you
   don't. */
#ifndef MAGICKCORE_HAVE_DECL_STRLCPY
#define MAGICKCORE_HAVE_DECL_STRLCPY 0
#endif

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
/* #undef HAVE_DECL_TZNAME */

/* Define to 1 if you have the declaration of `vsnprintf', and to 0 if you
   don't. */
#ifndef MAGICKCORE_HAVE_DECL_VSNPRINTF
#define MAGICKCORE_HAVE_DECL_VSNPRINTF 1
#endif

/* Define to 1 if you have the `directio' function. */
/* #undef HAVE_DIRECTIO */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#ifndef MAGICKCORE_HAVE_DIRENT_H
#define MAGICKCORE_HAVE_DIRENT_H 1
#endif

/* Define if you have the GNU dld library. */
/* #undef HAVE_DLD */

/* Define to 1 if you have the <dld.h> header file. */
/* #undef HAVE_DLD_H */

/* Define to 1 if you have the `dlerror' function. */
#ifndef MAGICKCORE_HAVE_DLERROR
#define MAGICKCORE_HAVE_DLERROR 1
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef MAGICKCORE_HAVE_DLFCN_H
#define MAGICKCORE_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you have the <dl.h> header file. */
/* #undef HAVE_DL_H */

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define if you have the _dyld_func_lookup function. */
/* #undef HAVE_DYLD */

/* Define to 1 if you have the <errno.h> header file. */
#ifndef MAGICKCORE_HAVE_ERRNO_H
#define MAGICKCORE_HAVE_ERRNO_H 1
#endif

/* Define to 1 if the system has the type `error_t'. */
#ifndef MAGICKCORE_HAVE_ERROR_T
#define MAGICKCORE_HAVE_ERROR_T 1
#endif

/* Define to 1 if you have the `execvp' function. */
#ifndef MAGICKCORE_HAVE_EXECVP
#define MAGICKCORE_HAVE_EXECVP 1
#endif

/* Define to 1 if you have the `fchmod' function. */
#ifndef MAGICKCORE_HAVE_FCHMOD
#define MAGICKCORE_HAVE_FCHMOD 1
#endif

/* Define to 1 if you have the <fcntl.h> header file. */
#ifndef MAGICKCORE_HAVE_FCNTL_H
#define MAGICKCORE_HAVE_FCNTL_H 1
#endif

/* Define to 1 if you have the `floor' function. */
#ifndef MAGICKCORE_HAVE_FLOOR
#define MAGICKCORE_HAVE_FLOOR 1
#endif

/* Define to 1 if you have the `fork' function. */
#ifndef MAGICKCORE_HAVE_FORK
#define MAGICKCORE_HAVE_FORK 1
#endif

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#ifndef MAGICKCORE_HAVE_FSEEKO
#define MAGICKCORE_HAVE_FSEEKO 1
#endif

/* Define to 1 if you have the <ft2build.h> header file. */
#ifndef MAGICKCORE_HAVE_FT2BUILD_H
#define MAGICKCORE_HAVE_FT2BUILD_H 1
#endif

/* Define to 1 if you have the `ftime' function. */
#ifndef MAGICKCORE_HAVE_FTIME
#define MAGICKCORE_HAVE_FTIME 1
#endif

/* Define to 1 if you have the `ftruncate' function. */
#ifndef MAGICKCORE_HAVE_FTRUNCATE
#define MAGICKCORE_HAVE_FTRUNCATE 1
#endif

/* Define to 1 if you have the `getcwd' function. */
#ifndef MAGICKCORE_HAVE_GETCWD
#define MAGICKCORE_HAVE_GETCWD 1
#endif

/* Define to 1 if you have the `getdtablesize' function. */
#ifndef MAGICKCORE_HAVE_GETDTABLESIZE
#define MAGICKCORE_HAVE_GETDTABLESIZE 1
#endif

/* Define to 1 if you have the `getexecname' function. */
/* #undef HAVE_GETEXECNAME */

/* Define to 1 if you have the `getpagesize' function. */
#ifndef MAGICKCORE_HAVE_GETPAGESIZE
#define MAGICKCORE_HAVE_GETPAGESIZE 1
#endif

/* Define to 1 if you have the `getpid' function. */
#ifndef MAGICKCORE_HAVE_GETPID
#define MAGICKCORE_HAVE_GETPID 1
#endif

/* Define to 1 if you have the `getrlimit' function. */
#ifndef MAGICKCORE_HAVE_GETRLIMIT
#define MAGICKCORE_HAVE_GETRLIMIT 1
#endif

/* Define to 1 if you have the `getrusage' function. */
#ifndef MAGICKCORE_HAVE_GETRUSAGE
#define MAGICKCORE_HAVE_GETRUSAGE 1
#endif

/* Define to 1 if you have the `gettimeofday' function. */
#ifndef MAGICKCORE_HAVE_GETTIMEOFDAY
#define MAGICKCORE_HAVE_GETTIMEOFDAY 1
#endif

/* Define to 1 if you have the `gmtime_r' function. */
#ifndef MAGICKCORE_HAVE_GMTIME_R
#define MAGICKCORE_HAVE_GMTIME_R 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef MAGICKCORE_HAVE_INTTYPES_H
#define MAGICKCORE_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `j0' function. */
#ifndef MAGICKCORE_HAVE_J0
#define MAGICKCORE_HAVE_J0 1
#endif

/* Define to 1 if you have the `j1' function. */
#ifndef MAGICKCORE_HAVE_J1
#define MAGICKCORE_HAVE_J1 1
#endif

/* Define if you have the <lcms2.h> header file. */
/* #undef HAVE_LCMS2_H */

/* Define if you have the <lcms.h> header file. */
#ifndef MAGICKCORE_HAVE_LCMS_H
#define MAGICKCORE_HAVE_LCMS_H 1
#endif

/* Define if you have the <lcms/lcms2.h> header file. */
/* #undef HAVE_LCMS_LCMS2_H */

/* Define if you have the <lcms/lcms.h> header file. */
/* #undef HAVE_LCMS_LCMS_H */

/* Define if you have the libdl library or equivalent. */
#ifndef MAGICKCORE_HAVE_LIBDL
#define MAGICKCORE_HAVE_LIBDL 1
#endif

/* Define if libdlloader will be built on this platform */
#ifndef MAGICKCORE_HAVE_LIBDLLOADER
#define MAGICKCORE_HAVE_LIBDLLOADER 1
#endif

/* Define to 1 if you have the `gcov' library (-lgcov). */
/* #undef HAVE_LIBGCOV */

/* Define to 1 if you have the <limits.h> header file. */
#ifndef MAGICKCORE_HAVE_LIMITS_H
#define MAGICKCORE_HAVE_LIMITS_H 1
#endif

/* Define to 1 if you have the <linux/unistd.h> header file. */
#ifndef MAGICKCORE_HAVE_LINUX_UNISTD_H
#define MAGICKCORE_HAVE_LINUX_UNISTD_H 1
#endif

/* Define to 1 if you have the <locale.h> header file. */
#ifndef MAGICKCORE_HAVE_LOCALE_H
#define MAGICKCORE_HAVE_LOCALE_H 1
#endif

/* Define to 1 if you have the `localtime_r' function. */
#ifndef MAGICKCORE_HAVE_LOCALTIME_R
#define MAGICKCORE_HAVE_LOCALTIME_R 1
#endif

/* Define to 1 if the type `long double' works and has more range or precision
   than `double'. */
#ifndef MAGICKCORE_HAVE_LONG_DOUBLE_WIDER
#define MAGICKCORE_HAVE_LONG_DOUBLE_WIDER 1
#endif

/* Define to 1 if you have the `lstat' function. */
#ifndef MAGICKCORE_HAVE_LSTAT
#define MAGICKCORE_HAVE_LSTAT 1
#endif

/* define if the compiler implements L"widestring" */
#ifndef MAGICKCORE_HAVE_LSTRING
#define MAGICKCORE_HAVE_LSTRING /**/
#endif

/* Define this if a modern libltdl is already installed */
#ifndef MAGICKCORE_HAVE_LTDL
#define MAGICKCORE_HAVE_LTDL 1
#endif

/* Define to 1 if you have the <machine/param.h> header file. */
/* #undef HAVE_MACHINE_PARAM_H */

/* Define to 1 if you have the <mach-o/dyld.h> header file. */
/* #undef HAVE_MACH_O_DYLD_H */

/* Define to 1 if you have the `memmove' function. */
#ifndef MAGICKCORE_HAVE_MEMMOVE
#define MAGICKCORE_HAVE_MEMMOVE 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifndef MAGICKCORE_HAVE_MEMORY_H
#define MAGICKCORE_HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the `memset' function. */
#ifndef MAGICKCORE_HAVE_MEMSET
#define MAGICKCORE_HAVE_MEMSET 1
#endif

/* Define to 1 if you have the `mkstemp' function. */
#ifndef MAGICKCORE_HAVE_MKSTEMP
#define MAGICKCORE_HAVE_MKSTEMP 1
#endif

/* Define to 1 if you have a working `mmap' system call. */
#ifndef MAGICKCORE_HAVE_MMAP
#define MAGICKCORE_HAVE_MMAP 1
#endif

/* Define to 1 if you have a working `mmap' system call. */
#ifndef MAGICKCORE_HAVE_MMAP_FILEIO
#define MAGICKCORE_HAVE_MMAP_FILEIO 1
#endif

/* Define to 1 if you have the `munmap' function. */
#ifndef MAGICKCORE_HAVE_MUNMAP
#define MAGICKCORE_HAVE_MUNMAP 1
#endif

/* define if the compiler implements namespaces */
#ifndef MAGICKCORE_HAVE_NAMESPACES
#define MAGICKCORE_HAVE_NAMESPACES /**/
#endif

/* Define if g++ supports namespace std. */
#ifndef MAGICKCORE_HAVE_NAMESPACE_STD
#define MAGICKCORE_HAVE_NAMESPACE_STD /**/
#endif

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <OpenCL/cl.h> header file. */
/* #undef HAVE_OPENCL_CL_H */

/* Define to 1 if you have the `opendir' function. */
#ifndef MAGICKCORE_HAVE_OPENDIR
#define MAGICKCORE_HAVE_OPENDIR 1
#endif

/* Define to 1 if you have the <OS.h> header file. */
/* #undef HAVE_OS_H */

/* Define to 1 if you have the `pclose' function. */
#ifndef MAGICKCORE_HAVE_PCLOSE
#define MAGICKCORE_HAVE_PCLOSE 1
#endif

/* Define to 1 if you have the `poll' function. */
#ifndef MAGICKCORE_HAVE_POLL
#define MAGICKCORE_HAVE_POLL 1
#endif

/* Define to 1 if you have the `popen' function. */
#ifndef MAGICKCORE_HAVE_POPEN
#define MAGICKCORE_HAVE_POPEN 1
#endif

/* Define to 1 if you have the `posix_fadvise' function. */
#ifndef MAGICKCORE_HAVE_POSIX_FADVISE
#define MAGICKCORE_HAVE_POSIX_FADVISE 1
#endif

/* Define to 1 if you have the `posix_fallocate' function. */
#ifndef MAGICKCORE_HAVE_POSIX_FALLOCATE
#define MAGICKCORE_HAVE_POSIX_FALLOCATE 1
#endif

/* Define to 1 if you have the `posix_madvise' function. */
#ifndef MAGICKCORE_HAVE_POSIX_MADVISE
#define MAGICKCORE_HAVE_POSIX_MADVISE 1
#endif

/* Define to 1 if you have the `posix_memalign' function. */
#ifndef MAGICKCORE_HAVE_POSIX_MEMALIGN
#define MAGICKCORE_HAVE_POSIX_MEMALIGN 1
#endif

/* Define to 1 if you have the `pow' function. */
#ifndef MAGICKCORE_HAVE_POW
#define MAGICKCORE_HAVE_POW 1
#endif

/* Define to 1 if you have the `pread' function. */
#ifndef MAGICKCORE_HAVE_PREAD
#define MAGICKCORE_HAVE_PREAD 1
#endif

/* Define if libtool can extract symbol lists from object files. */
#ifndef MAGICKCORE_HAVE_PRELOADED_SYMBOLS
#define MAGICKCORE_HAVE_PRELOADED_SYMBOLS 1
#endif

/* Define to 1 if you have the <process.h> header file. */
/* #undef HAVE_PROCESS_H */

/* Define if you have POSIX threads libraries and header files. */
#ifndef MAGICKCORE_HAVE_PTHREAD
#define MAGICKCORE_HAVE_PTHREAD 1
#endif

/* Define to 1 if you have the `pwrite' function. */
#ifndef MAGICKCORE_HAVE_PWRITE
#define MAGICKCORE_HAVE_PWRITE 1
#endif

/* Define to 1 if you have the `raise' function. */
#ifndef MAGICKCORE_HAVE_RAISE
#define MAGICKCORE_HAVE_RAISE 1
#endif

/* Define to 1 if you have the `rand_r' function. */
#ifndef MAGICKCORE_HAVE_RAND_R
#define MAGICKCORE_HAVE_RAND_R 1
#endif

/* Define to 1 if you have the `readdir' function. */
#ifndef MAGICKCORE_HAVE_READDIR
#define MAGICKCORE_HAVE_READDIR 1
#endif

/* Define to 1 if you have the `readdir_r' function. */
#ifndef MAGICKCORE_HAVE_READDIR_R
#define MAGICKCORE_HAVE_READDIR_R 1
#endif

/* Define to 1 if you have the `readlink' function. */
#ifndef MAGICKCORE_HAVE_READLINK
#define MAGICKCORE_HAVE_READLINK 1
#endif

/* Define to 1 if you have the `realpath' function. */
#ifndef MAGICKCORE_HAVE_REALPATH
#define MAGICKCORE_HAVE_REALPATH 1
#endif

/* Define to 1 if you have the `seekdir' function. */
#ifndef MAGICKCORE_HAVE_SEEKDIR
#define MAGICKCORE_HAVE_SEEKDIR 1
#endif

/* Define to 1 if you have the `select' function. */
#ifndef MAGICKCORE_HAVE_SELECT
#define MAGICKCORE_HAVE_SELECT 1
#endif

/* Define to 1 if you have the `setlocale' function. */
#ifndef MAGICKCORE_HAVE_SETLOCALE
#define MAGICKCORE_HAVE_SETLOCALE 1
#endif

/* Define to 1 if you have the `setvbuf' function. */
#ifndef MAGICKCORE_HAVE_SETVBUF
#define MAGICKCORE_HAVE_SETVBUF 1
#endif

/* X11 server supports shape extension */
#ifndef MAGICKCORE_HAVE_SHAPE
#define MAGICKCORE_HAVE_SHAPE 1
#endif

/* X11 server supports shared memory extension */
#ifndef MAGICKCORE_HAVE_SHARED_MEMORY
#define MAGICKCORE_HAVE_SHARED_MEMORY 1
#endif

/* Define if you have the shl_load function. */
/* #undef HAVE_SHL_LOAD */

/* Define to 1 if you have the `sigaction' function. */
#ifndef MAGICKCORE_HAVE_SIGACTION
#define MAGICKCORE_HAVE_SIGACTION 1
#endif

/* Define to 1 if you have the `sigemptyset' function. */
#ifndef MAGICKCORE_HAVE_SIGEMPTYSET
#define MAGICKCORE_HAVE_SIGEMPTYSET 1
#endif

/* Define to 1 if you have the `spawnvp' function. */
/* #undef HAVE_SPAWNVP */

/* Define to 1 if you have the `sqrt' function. */
#ifndef MAGICKCORE_HAVE_SQRT
#define MAGICKCORE_HAVE_SQRT 1
#endif

/* Define to 1 if you have the `stat' function. */
#ifndef MAGICKCORE_HAVE_STAT
#define MAGICKCORE_HAVE_STAT 1
#endif

/* Define to 1 if you have the <stdarg.h> header file. */
#ifndef MAGICKCORE_HAVE_STDARG_H
#define MAGICKCORE_HAVE_STDARG_H 1
#endif

/* Define to 1 if stdbool.h conforms to C99. */
#ifndef MAGICKCORE_HAVE_STDBOOL_H
#define MAGICKCORE_HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef MAGICKCORE_HAVE_STDINT_H
#define MAGICKCORE_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef MAGICKCORE_HAVE_STDLIB_H
#define MAGICKCORE_HAVE_STDLIB_H 1
#endif

/* define if the compiler supports ISO C++ standard library */
#ifndef MAGICKCORE_HAVE_STD_LIBS
#define MAGICKCORE_HAVE_STD_LIBS /**/
#endif

/* Define to 1 if you have the `strcasecmp' function. */
#ifndef MAGICKCORE_HAVE_STRCASECMP
#define MAGICKCORE_HAVE_STRCASECMP 1
#endif

/* Define to 1 if you have the `strchr' function. */
#ifndef MAGICKCORE_HAVE_STRCHR
#define MAGICKCORE_HAVE_STRCHR 1
#endif

/* Define to 1 if you have the `strcspn' function. */
#ifndef MAGICKCORE_HAVE_STRCSPN
#define MAGICKCORE_HAVE_STRCSPN 1
#endif

/* Define to 1 if you have the `strdup' function. */
#ifndef MAGICKCORE_HAVE_STRDUP
#define MAGICKCORE_HAVE_STRDUP 1
#endif

/* Define to 1 if you have the `strerror' function. */
#ifndef MAGICKCORE_HAVE_STRERROR
#define MAGICKCORE_HAVE_STRERROR 1
#endif

/* Define to 1 if you have the `strerror_r' function. */
#ifndef MAGICKCORE_HAVE_STRERROR_R
#define MAGICKCORE_HAVE_STRERROR_R 1
#endif

/* Define to 1 if cpp supports the ANSI # stringizing operator. */
#ifndef MAGICKCORE_HAVE_STRINGIZE
#define MAGICKCORE_HAVE_STRINGIZE 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifndef MAGICKCORE_HAVE_STRINGS_H
#define MAGICKCORE_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef MAGICKCORE_HAVE_STRING_H
#define MAGICKCORE_HAVE_STRING_H 1
#endif

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the `strncasecmp' function. */
#ifndef MAGICKCORE_HAVE_STRNCASECMP
#define MAGICKCORE_HAVE_STRNCASECMP 1
#endif

/* Define to 1 if you have the `strpbrk' function. */
#ifndef MAGICKCORE_HAVE_STRPBRK
#define MAGICKCORE_HAVE_STRPBRK 1
#endif

/* Define to 1 if you have the `strrchr' function. */
#ifndef MAGICKCORE_HAVE_STRRCHR
#define MAGICKCORE_HAVE_STRRCHR 1
#endif

/* Define to 1 if you have the `strspn' function. */
#ifndef MAGICKCORE_HAVE_STRSPN
#define MAGICKCORE_HAVE_STRSPN 1
#endif

/* Define to 1 if you have the `strstr' function. */
#ifndef MAGICKCORE_HAVE_STRSTR
#define MAGICKCORE_HAVE_STRSTR 1
#endif

/* Define to 1 if you have the `strtol' function. */
#ifndef MAGICKCORE_HAVE_STRTOL
#define MAGICKCORE_HAVE_STRTOL 1
#endif

/* Define to 1 if you have the `strtoul' function. */
#ifndef MAGICKCORE_HAVE_STRTOUL
#define MAGICKCORE_HAVE_STRTOUL 1
#endif

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
#ifndef MAGICKCORE_HAVE_STRUCT_TM_TM_ZONE
#define MAGICKCORE_HAVE_STRUCT_TM_TM_ZONE 1
#endif

/* Define to 1 if you have the `symlink' function. */
#ifndef MAGICKCORE_HAVE_SYMLINK
#define MAGICKCORE_HAVE_SYMLINK 1
#endif

/* Define to 1 if you have the `sysconf' function. */
#ifndef MAGICKCORE_HAVE_SYSCONF
#define MAGICKCORE_HAVE_SYSCONF 1
#endif

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/dl.h> header file. */
/* #undef HAVE_SYS_DL_H */

/* Define to 1 if you have the <sys/ipc.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_IPC_H
#define MAGICKCORE_HAVE_SYS_IPC_H 1
#endif

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_PARAM_H
#define MAGICKCORE_HAVE_SYS_PARAM_H 1
#endif

/* Define to 1 if you have the <sys/resource.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_RESOURCE_H
#define MAGICKCORE_HAVE_SYS_RESOURCE_H 1
#endif

/* Define to 1 if you have the <sys/select.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_SELECT_H
#define MAGICKCORE_HAVE_SYS_SELECT_H 1
#endif

/* Define to 1 if you have the <sys/socket.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_SOCKET_H
#define MAGICKCORE_HAVE_SYS_SOCKET_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_STAT_H
#define MAGICKCORE_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/syslimits.h> header file. */
/* #undef HAVE_SYS_SYSLIMITS_H */

/* Define to 1 if you have the <sys/timeb.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_TIMEB_H
#define MAGICKCORE_HAVE_SYS_TIMEB_H 1
#endif

/* Define to 1 if you have the <sys/times.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_TIMES_H
#define MAGICKCORE_HAVE_SYS_TIMES_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_TIME_H
#define MAGICKCORE_HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_TYPES_H
#define MAGICKCORE_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the <sys/wait.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_WAIT_H
#define MAGICKCORE_HAVE_SYS_WAIT_H 1
#endif

/* Define to 1 if you have the `telldir' function. */
#ifndef MAGICKCORE_HAVE_TELLDIR
#define MAGICKCORE_HAVE_TELLDIR 1
#endif

/* Define to 1 if you have the `tempnam' function. */
#ifndef MAGICKCORE_HAVE_TEMPNAM
#define MAGICKCORE_HAVE_TEMPNAM 1
#endif

/* Define to 1 if you have the <tiffconf.h> header file. */
#ifndef MAGICKCORE_HAVE_TIFFCONF_H
#define MAGICKCORE_HAVE_TIFFCONF_H 1
#endif

/* Define to 1 if you have the `TIFFIsCODECConfigured' function. */
#ifndef MAGICKCORE_HAVE_TIFFISCODECCONFIGURED
#define MAGICKCORE_HAVE_TIFFISCODECCONFIGURED 1
#endif

/* Define to 1 if you have the `TIFFMergeFieldInfo' function. */
#ifndef MAGICKCORE_HAVE_TIFFMERGEFIELDINFO
#define MAGICKCORE_HAVE_TIFFMERGEFIELDINFO 1
#endif

/* Define to 1 if you have the `TIFFReadEXIFDirectory' function. */
#ifndef MAGICKCORE_HAVE_TIFFREADEXIFDIRECTORY
#define MAGICKCORE_HAVE_TIFFREADEXIFDIRECTORY 1
#endif

/* Define to 1 if you have the `TIFFSetErrorHandlerExt' function. */
#ifndef MAGICKCORE_HAVE_TIFFSETERRORHANDLEREXT
#define MAGICKCORE_HAVE_TIFFSETERRORHANDLEREXT 1
#endif

/* Define to 1 if you have the `TIFFSetTagExtender' function. */
#ifndef MAGICKCORE_HAVE_TIFFSETTAGEXTENDER
#define MAGICKCORE_HAVE_TIFFSETTAGEXTENDER 1
#endif

/* Define to 1 if you have the `TIFFSetWarningHandlerExt' function. */
#ifndef MAGICKCORE_HAVE_TIFFSETWARNINGHANDLEREXT
#define MAGICKCORE_HAVE_TIFFSETWARNINGHANDLEREXT 1
#endif

/* Define to 1 if you have the `TIFFSwabArrayOfTriples' function. */
#ifndef MAGICKCORE_HAVE_TIFFSWABARRAYOFTRIPLES
#define MAGICKCORE_HAVE_TIFFSWABARRAYOFTRIPLES 1
#endif

/* Define to 1 if you have the `times' function. */
#ifndef MAGICKCORE_HAVE_TIMES
#define MAGICKCORE_HAVE_TIMES 1
#endif

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#ifndef MAGICKCORE_HAVE_TM_ZONE
#define MAGICKCORE_HAVE_TM_ZONE 1
#endif

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
/* #undef HAVE_TZNAME */

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef MAGICKCORE_HAVE_UNISTD_H
#define MAGICKCORE_HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `usleep' function. */
#ifndef MAGICKCORE_HAVE_USLEEP
#define MAGICKCORE_HAVE_USLEEP 1
#endif

/* Define to 1 if you have the `utime' function. */
#ifndef MAGICKCORE_HAVE_UTIME
#define MAGICKCORE_HAVE_UTIME 1
#endif

/* Define to 1 if you have the `vfork' function. */
#ifndef MAGICKCORE_HAVE_VFORK
#define MAGICKCORE_HAVE_VFORK 1
#endif

/* Define to 1 if you have the <vfork.h> header file. */
/* #undef HAVE_VFORK_H */

/* Define to 1 if you have the `vprintf' function. */
#ifndef MAGICKCORE_HAVE_VPRINTF
#define MAGICKCORE_HAVE_VPRINTF 1
#endif

/* Define to 1 if you have the `vsnprintf' function. */
#ifndef MAGICKCORE_HAVE_VSNPRINTF
#define MAGICKCORE_HAVE_VSNPRINTF 1
#endif

/* Define to 1 if you have the `vsprintf' function. */
#ifndef MAGICKCORE_HAVE_VSPRINTF
#define MAGICKCORE_HAVE_VSPRINTF 1
#endif

/* Define to 1 if you have the `waitpid' function. */
#ifndef MAGICKCORE_HAVE_WAITPID
#define MAGICKCORE_HAVE_WAITPID 1
#endif

/* Define to 1 if you have the <wchar.h> header file. */
#ifndef MAGICKCORE_HAVE_WCHAR_H
#define MAGICKCORE_HAVE_WCHAR_H 1
#endif

/* Define to 1 if you have the <windows.h> header file. */
/* #undef HAVE_WINDOWS_H */

/* This value is set to 1 to indicate that the system argz facility works */
#ifndef MAGICKCORE_HAVE_WORKING_ARGZ
#define MAGICKCORE_HAVE_WORKING_ARGZ 1
#endif

/* Define to 1 if `fork' works. */
#ifndef MAGICKCORE_HAVE_WORKING_FORK
#define MAGICKCORE_HAVE_WORKING_FORK 1
#endif

/* Define to 1 if `vfork' works. */
#ifndef MAGICKCORE_HAVE_WORKING_VFORK
#define MAGICKCORE_HAVE_WORKING_VFORK 1
#endif

/* Define to 1 if the system has the type `_Bool'. */
#ifndef MAGICKCORE_HAVE__BOOL
#define MAGICKCORE_HAVE__BOOL 1
#endif

/* Define to 1 if you have the `_exit' function. */
#ifndef MAGICKCORE_HAVE__EXIT
#define MAGICKCORE_HAVE__EXIT 1
#endif

/* Define to 1 if you have the `_NSGetExecutablePath' function. */
/* #undef HAVE__NSGETEXECUTABLEPATH */

/* Define to 1 if you have the `_pclose' function. */
/* #undef HAVE__PCLOSE */

/* Define to 1 if you have the `_popen' function. */
/* #undef HAVE__POPEN */

/* Define to 1 if you have the `_wfopen' function. */
/* #undef HAVE__WFOPEN */

/* Define to 1 if you have the `_wstat' function. */
/* #undef HAVE__WSTAT */

/* define if your compiler has __attribute__ */
#ifndef MAGICKCORE_HAVE___ATTRIBUTE__
#define MAGICKCORE_HAVE___ATTRIBUTE__ 1
#endif

/* accurately represent the wide range of intensity levels in real scenes */
/* #undef HDRI_SUPPORT */

/* Define if you have umem memory allocation library */
/* #undef HasUMEM */

/* ImageMagick is formally installed under prefix */
#ifndef MAGICKCORE_INSTALLED_SUPPORT
#define MAGICKCORE_INSTALLED_SUPPORT 1
#endif

/* Define if you have JBIG library */
/* #undef JBIG_DELEGATE */

/* Define if you have JPEG version 2 "Jasper" library */
#ifndef MAGICKCORE_JP2_DELEGATE
#define MAGICKCORE_JP2_DELEGATE 1
#endif

/* Define if you have JPEG library */
#ifndef MAGICKCORE_JPEG_DELEGATE
#define MAGICKCORE_JPEG_DELEGATE 1
#endif

/* Define if you have LCMS library */
#ifndef MAGICKCORE_LCMS_DELEGATE
#define MAGICKCORE_LCMS_DELEGATE 1
#endif

/* Directory where architecture-dependent files live. */
#ifndef MAGICKCORE_LIBRARY_PATH
#define MAGICKCORE_LIBRARY_PATH "/usr/local/lib/ImageMagick-6.6.2/"
#endif

/* Subdirectory of lib where ImageMagick architecture dependent files are
   installed */
#ifndef MAGICKCORE_LIBRARY_RELATIVE_PATH
#define MAGICKCORE_LIBRARY_RELATIVE_PATH "ImageMagick-6.6.2"
#endif

/* Define if you have LQR library */
/* #undef LQR_DELEGATE */

/* Define if using libltdl to support dynamically loadable modules */
#ifndef MAGICKCORE_LTDL_DELEGATE
#define MAGICKCORE_LTDL_DELEGATE 1
#endif

/* Define if the OS needs help to load dependent libraries for dlopen(). */
/* #undef LTDL_DLOPEN_DEPLIBS */

/* Define to the system default library search path. */
#ifndef MAGICKCORE_LT_DLSEARCH_PATH
#define MAGICKCORE_LT_DLSEARCH_PATH "/lib64:/usr/lib64:/lib:/usr/lib:/usr/lib64/alliance/lib:/usr/lib64/atlas:/usr/lib64/kicad:/usr/lib64/llvm:/usr/lib64/mysql:/usr/lib64/octave-3.2.4:/usr/lib64/openmotif:/usr/lib64/qt-3.3/lib:/usr/lib64/tcl8.5/tclx8.4:/usr/lib64/tcl8.5:/usr/lib64/wine/:/usr/lib64/xulrunner-1.9.2"
#endif

/* The archive extension */
#ifndef MAGICKCORE_LT_LIBEXT
#define MAGICKCORE_LT_LIBEXT "a"
#endif

/* Define to the extension used for runtime loadable modules, say, ".so". */
#ifndef MAGICKCORE_LT_MODULE_EXT
#define MAGICKCORE_LT_MODULE_EXT ".so"
#endif

/* Define to the name of the environment variable that determines the run-time
   module search path. */
#ifndef MAGICKCORE_LT_MODULE_PATH_VAR
#define MAGICKCORE_LT_MODULE_PATH_VAR "LD_LIBRARY_PATH"
#endif

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#ifndef MAGICKCORE_LT_OBJDIR
#define MAGICKCORE_LT_OBJDIR ".libs/"
#endif

/* Define to prepend to default font search path. */
/* #undef MAGICK_FONT_PATH */

/* Magick API method prefix */
/* #undef NAMESPACE_PREFIX */

/* Define to 1 if assertions should be disabled. */
/* #undef NDEBUG */

/* Define if dlsym() requires a leading underscore in symbol names. */
/* #undef NEED_USCORE */

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Define if you have OPENEXR library */
/* #undef OPENEXR_DELEGATE */

/* Define to the address where bug reports for this package should be sent. */
#ifndef MAGICKCORE_PACKAGE_BUGREPORT
#define MAGICKCORE_PACKAGE_BUGREPORT "http://www.imagemagick.org"
#endif

/* Define to the full name of this package. */
#ifndef MAGICKCORE_PACKAGE_NAME
#define MAGICKCORE_PACKAGE_NAME "ImageMagick"
#endif

/* Define to the full name and version of this package. */
#ifndef MAGICKCORE_PACKAGE_STRING
#define MAGICKCORE_PACKAGE_STRING "ImageMagick 6.6.2"
#endif

/* Define to the one symbol short name of this package. */
#ifndef MAGICKCORE_PACKAGE_TARNAME
#define MAGICKCORE_PACKAGE_TARNAME "ImageMagick"
#endif

/* Define to the home page for this package. */
#ifndef MAGICKCORE_PACKAGE_URL
#define MAGICKCORE_PACKAGE_URL ""
#endif

/* Define to the version of this package. */
#ifndef MAGICKCORE_PACKAGE_VERSION
#define MAGICKCORE_PACKAGE_VERSION "6.6.2"
#endif

/* Define if you have PNG library */
#ifndef MAGICKCORE_PNG_DELEGATE
#define MAGICKCORE_PNG_DELEGATE 1
#endif

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Pixel cache threshold in MB (defaults to available memory) */
/* #undef PixelCacheThreshold */

/* Number of bits in a pixel Quantum (8/16/32/64) */
#ifndef MAGICKCORE_QUANTUM_DEPTH
#define MAGICKCORE_QUANTUM_DEPTH 16
#endif

/* Define as the return type of signal handlers (`int' or `void'). */
#ifndef MAGICKCORE_RETSIGTYPE
#define MAGICKCORE_RETSIGTYPE void
#endif

/* Define if you have RSVG library */
#ifndef MAGICKCORE_RSVG_DELEGATE
#define MAGICKCORE_RSVG_DELEGATE 1
#endif

/* Define to the type of arg 1 for `select'. */
#ifndef MAGICKCORE_SELECT_TYPE_ARG1
#define MAGICKCORE_SELECT_TYPE_ARG1 int
#endif

/* Define to the type of args 2, 3 and 4 for `select'. */
#ifndef MAGICKCORE_SELECT_TYPE_ARG234
#define MAGICKCORE_SELECT_TYPE_ARG234 (fd_set *)
#endif

/* Define to the type of arg 5 for `select'. */
#ifndef MAGICKCORE_SELECT_TYPE_ARG5
#define MAGICKCORE_SELECT_TYPE_ARG5 (struct timeval *)
#endif

/* Directory where architecture-independent configuration files live. */
#ifndef MAGICKCORE_SHARE_CONFIGURE_PATH
#define MAGICKCORE_SHARE_CONFIGURE_PATH "/usr/local/share/ImageMagick-6.6.2/config/"
#endif

/* Subdirectory of lib where architecture-independent configuration files
   live. */
#ifndef MAGICKCORE_SHARE_CONFIGURE_RELATIVE_PATH
#define MAGICKCORE_SHARE_CONFIGURE_RELATIVE_PATH "ImageMagick-6.6.2/config"
#endif

/* Directory where architecture-independent files live. */
#ifndef MAGICKCORE_SHARE_PATH
#define MAGICKCORE_SHARE_PATH "/usr/local/share/ImageMagick-6.6.2/"
#endif

/* The size of `off_t', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_OFF_T
#define MAGICKCORE_SIZEOF_OFF_T 8
#endif

/* The size of `signed int', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_SIGNED_INT
#define MAGICKCORE_SIZEOF_SIGNED_INT 4
#endif

/* The size of `signed long', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_SIGNED_LONG
#define MAGICKCORE_SIZEOF_SIGNED_LONG 8
#endif

/* The size of `signed long long', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_SIGNED_LONG_LONG
#define MAGICKCORE_SIZEOF_SIGNED_LONG_LONG 8
#endif

/* The size of `signed short', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_SIGNED_SHORT
#define MAGICKCORE_SIZEOF_SIGNED_SHORT 2
#endif

/* The size of `size_t', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_SIZE_T
#define MAGICKCORE_SIZEOF_SIZE_T 8
#endif

/* The size of `unsigned int', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_UNSIGNED_INT
#define MAGICKCORE_SIZEOF_UNSIGNED_INT 4
#endif

/* The size of `unsigned int*', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_UNSIGNED_INTP
#define MAGICKCORE_SIZEOF_UNSIGNED_INTP 8
#endif

/* The size of `unsigned long', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_UNSIGNED_LONG
#define MAGICKCORE_SIZEOF_UNSIGNED_LONG 8
#endif

/* The size of `unsigned long long', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_UNSIGNED_LONG_LONG
#define MAGICKCORE_SIZEOF_UNSIGNED_LONG_LONG 8
#endif

/* The size of `unsigned short', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_UNSIGNED_SHORT
#define MAGICKCORE_SIZEOF_UNSIGNED_SHORT 2
#endif

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
/* #undef STAT_MACROS_BROKEN */

/* Define to 1 if you have the ANSI C header files. */
#ifndef MAGICKCORE_STDC_HEADERS
#define MAGICKCORE_STDC_HEADERS 1
#endif

/* Define if you have TIFF library */
#ifndef MAGICKCORE_TIFF_DELEGATE
#define MAGICKCORE_TIFF_DELEGATE 1
#endif

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#ifndef MAGICKCORE_TIME_WITH_SYS_TIME
#define MAGICKCORE_TIME_WITH_SYS_TIME 1
#endif

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Define to use the Windows GDI32 library */
/* #undef WINGDI32_DELEGATE */

/* Define if using the dmalloc debugging malloc package */
/* #undef WITH_DMALLOC */

/* Define if you have WMF library */
#ifndef MAGICKCORE_WMF_DELEGATE
#define MAGICKCORE_WMF_DELEGATE 1
#endif

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Location of X11 configure files */
#ifndef MAGICKCORE_X11_CONFIGURE_PATH
#define MAGICKCORE_X11_CONFIGURE_PATH ""
#endif

/* Define if you have X11 library */
#ifndef MAGICKCORE_X11_DELEGATE
#define MAGICKCORE_X11_DELEGATE 1
#endif

/* Define if you have XML library */
#ifndef MAGICKCORE_XML_DELEGATE
#define MAGICKCORE_XML_DELEGATE 1
#endif

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Define if you have zlib compression library */
#ifndef MAGICKCORE_ZLIB_DELEGATE
#define MAGICKCORE_ZLIB_DELEGATE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#ifndef MAGICKCORE__FILE_OFFSET_BITS
#define MAGICKCORE__FILE_OFFSET_BITS 64
#endif

/* enable run-time bounds-checking */
/* #undef _FORTIFY_SOURCE */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define this for the OpenCL Accelerator */
/* #undef _OPENCL */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define so that glibc/gnulib argp.h does not typedef error_t. */
/* #undef __error_t_defined */

/* Define to appropriate substitue if compiler does not have __func__ */
/* #undef __func__ */

/* Define to a type to use for `error_t' if it is not otherwise available. */
/* #undef error_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#ifndef _magickcore_restrict
#define _magickcore_restrict __restrict
#endif
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef ssize_t */

/* Define as `fork' if `vfork' does not work. */
/* #undef vfork */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */
 
/* once: _MAGICK_MAGICK_CONFIG_H */
#endif
