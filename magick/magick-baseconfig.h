#ifndef _MAGICK_MAGICK_BASECONFIG_H
#define _MAGICK_MAGICK_BASECONFIG_H 1
 
/* magick/magick-baseconfig.h. Generated automatically at end of configure. */
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
/* #undef CAIRO_DELEGATE */

/* permit enciphering and deciphering image pixels */
#ifndef MAGICKCORE_CIPHER_SUPPORT
#define MAGICKCORE_CIPHER_SUPPORT 1
#endif

/* Define to 1 if the `closedir' function returns void instead of `int'. */
/* #undef CLOSEDIR_VOID */

/* coders subdirectory. */
#ifndef MAGICKCORE_CODER_DIRNAME
#define MAGICKCORE_CODER_DIRNAME "coders"
#endif

/* Directory where architecture-dependent configuration files live. */
#ifndef MAGICKCORE_CONFIGURE_PATH
#define MAGICKCORE_CONFIGURE_PATH "/usr/local/etc/ImageMagick-6/"
#endif

/* Subdirectory of lib where architecture-dependent configuration files live.
   */
#ifndef MAGICKCORE_CONFIGURE_RELATIVE_PATH
#define MAGICKCORE_CONFIGURE_RELATIVE_PATH "ImageMagick-6"
#endif

/* Define if you have DJVU library */
#ifndef MAGICKCORE_DJVU_DELEGATE
#define MAGICKCORE_DJVU_DELEGATE 1
#endif

/* Directory where ImageMagick documents live. */
#ifndef MAGICKCORE_DOCUMENTATION_PATH
#define MAGICKCORE_DOCUMENTATION_PATH "/usr/local/share/doc/ImageMagick-6/"
#endif

/* Define if you have Display Postscript */
/* #undef DPS_DELEGATE */

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

/* filter subdirectory. */
#ifndef MAGICKCORE_FILTER_DIRNAME
#define MAGICKCORE_FILTER_DIRNAME "filters"
#endif

/* Define if you have FONTCONFIG library */
#ifndef MAGICKCORE_FONTCONFIG_DELEGATE
#define MAGICKCORE_FONTCONFIG_DELEGATE 1
#endif

/* Define if you have FlashPIX library */
#ifndef MAGICKCORE_FPX_DELEGATE
#define MAGICKCORE_FPX_DELEGATE 1
#endif

/* Define if you have FREETYPE library */
#ifndef MAGICKCORE_FREETYPE_DELEGATE
#define MAGICKCORE_FREETYPE_DELEGATE 1
#endif

/* Define if you have Ghostscript library or framework */
/* #undef GS_DELEGATE */

/* Define if you have GVC library */
/* #undef GVC_DELEGATE */

/* Define to 1 if you have the `acosh' function. */
#ifndef MAGICKCORE_HAVE_ACOSH
#define MAGICKCORE_HAVE_ACOSH 1
#endif

/* Define to 1 if you have the <arm/limits.h> header file. */
/* #undef HAVE_ARM_LIMITS_H */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#ifndef MAGICKCORE_HAVE_ARPA_INET_H
#define MAGICKCORE_HAVE_ARPA_INET_H 1
#endif

/* Define to 1 if you have the `asinh' function. */
#ifndef MAGICKCORE_HAVE_ASINH
#define MAGICKCORE_HAVE_ASINH 1
#endif

/* Define to 1 if you have the `atanh' function. */
#ifndef MAGICKCORE_HAVE_ATANH
#define MAGICKCORE_HAVE_ATANH 1
#endif

/* Define to 1 if you have the `atexit' function. */
#ifndef MAGICKCORE_HAVE_ATEXIT
#define MAGICKCORE_HAVE_ATEXIT 1
#endif

/* Define to 1 if you have the `atoll' function. */
#ifndef MAGICKCORE_HAVE_ATOLL
#define MAGICKCORE_HAVE_ATOLL 1
#endif

/* define if bool is a built-in type */
#ifndef MAGICKCORE_HAVE_BOOL
#define MAGICKCORE_HAVE_BOOL /**/
#endif

/* Define to 1 if you have the `cabs' function. */
#ifndef MAGICKCORE_HAVE_CABS
#define MAGICKCORE_HAVE_CABS 1
#endif

/* Define to 1 if you have the `carg' function. */
#ifndef MAGICKCORE_HAVE_CARG
#define MAGICKCORE_HAVE_CARG 1
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

/* Define to 1 if you have the <CL/cl.h> header file. */
/* #undef HAVE_CL_CL_H */

/* Define to 1 if you have the <complex.h> header file. */
#ifndef MAGICKCORE_HAVE_COMPLEX_H
#define MAGICKCORE_HAVE_COMPLEX_H 1
#endif

/* Define to 1 if you have the `creal' function. */
#ifndef MAGICKCORE_HAVE_CREAL
#define MAGICKCORE_HAVE_CREAL 1
#endif

/* Define to 1 if you have the `ctime_r' function. */
#ifndef MAGICKCORE_HAVE_CTIME_R
#define MAGICKCORE_HAVE_CTIME_R 1
#endif

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

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#ifndef MAGICKCORE_HAVE_DECL_STRERROR_R
#define MAGICKCORE_HAVE_DECL_STRERROR_R 1
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

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef MAGICKCORE_HAVE_DLFCN_H
#define MAGICKCORE_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if the system has the type `double_t'. */
#ifndef MAGICKCORE_HAVE_DOUBLE_T
#define MAGICKCORE_HAVE_DOUBLE_T 1
#endif

/* Define to 1 if you have the <errno.h> header file. */
#ifndef MAGICKCORE_HAVE_ERRNO_H
#define MAGICKCORE_HAVE_ERRNO_H 1
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

/* Define to 1 if the system has the type `float_t'. */
#ifndef MAGICKCORE_HAVE_FLOAT_T
#define MAGICKCORE_HAVE_FLOAT_T 1
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

/* Define to 1 if you have the `getc_unlocked' function. */
#ifndef MAGICKCORE_HAVE_GETC_UNLOCKED
#define MAGICKCORE_HAVE_GETC_UNLOCKED 1
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

/* Compile with hugepage support */
/* #undef HAVE_HUGEPAGES */

/* Define to 1 if the system has the type `intmax_t'. */
#ifndef MAGICKCORE_HAVE_INTMAX_T
#define MAGICKCORE_HAVE_INTMAX_T 1
#endif

/* Define to 1 if the system has the type `intptr_t'. */
#ifndef MAGICKCORE_HAVE_INTPTR_T
#define MAGICKCORE_HAVE_INTPTR_T 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef MAGICKCORE_HAVE_INTTYPES_H
#define MAGICKCORE_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `isnan' function. */
#ifndef MAGICKCORE_HAVE_ISNAN
#define MAGICKCORE_HAVE_ISNAN 1
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
#ifndef MAGICKCORE_HAVE_LCMS2_H
#define MAGICKCORE_HAVE_LCMS2_H 1
#endif

/* Define if you have the <lcms2/lcms2.h> header file. */
/* #undef HAVE_LCMS2_LCMS2_H */

/* Define if you have the <lcms.h> header file. */
/* #undef HAVE_LCMS_H */

/* Define if you have the <lcms/lcms.h> header file. */
/* #undef HAVE_LCMS_LCMS_H */

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

/* Define to 1 if you have the `lltostr' function. */
/* #undef HAVE_LLTOSTR */

/* Define to 1 if you have the <locale.h> header file. */
#ifndef MAGICKCORE_HAVE_LOCALE_H
#define MAGICKCORE_HAVE_LOCALE_H 1
#endif

/* Define to 1 if you have the `localtime_r' function. */
#ifndef MAGICKCORE_HAVE_LOCALTIME_R
#define MAGICKCORE_HAVE_LOCALTIME_R 1
#endif

/* Define to 1 if the system has the type `long double'. */
#ifndef MAGICKCORE_HAVE_LONG_DOUBLE
#define MAGICKCORE_HAVE_LONG_DOUBLE 1
#endif

/* Define to 1 if the type `long double' works and has more range or precision
   than `double'. */
#ifndef MAGICKCORE_HAVE_LONG_DOUBLE_WIDER
#define MAGICKCORE_HAVE_LONG_DOUBLE_WIDER 1
#endif

/* Define to 1 if the system has the type 'long long int'. */
#ifndef MAGICKCORE_HAVE_LONG_LONG_INT
#define MAGICKCORE_HAVE_LONG_LONG_INT 1
#endif

/* Define to 1 if you have the `lstat' function. */
#ifndef MAGICKCORE_HAVE_LSTAT
#define MAGICKCORE_HAVE_LSTAT 1
#endif

/* Define to 1 if you have the <machine/param.h> header file. */
/* #undef HAVE_MACHINE_PARAM_H */

/* Define to 1 if you have the <mach-o/dyld.h> header file. */
/* #undef HAVE_MACH_O_DYLD_H */

/* Define to 1 if <wchar.h> declares mbstate_t. */
#ifndef MAGICKCORE_HAVE_MBSTATE_T
#define MAGICKCORE_HAVE_MBSTATE_T 1
#endif

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

/* Define to 1 if you have the `nanosleep' function. */
#ifndef MAGICKCORE_HAVE_NANOSLEEP
#define MAGICKCORE_HAVE_NANOSLEEP 1
#endif

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
#ifndef MAGICKCORE_HAVE_NETINET_IN_H
#define MAGICKCORE_HAVE_NETINET_IN_H 1
#endif

/* Define to 1 if you have the `newlocale' function. */
#ifndef MAGICKCORE_HAVE_NEWLOCALE
#define MAGICKCORE_HAVE_NEWLOCALE 1
#endif

/* Define to 1 if you have the <OpenCL/cl.h> header file. */
/* #undef HAVE_OPENCL_CL_H */

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

/* Define to 1 if you have the `posix_spawnp' function. */
#ifndef MAGICKCORE_HAVE_POSIX_SPAWNP
#define MAGICKCORE_HAVE_POSIX_SPAWNP 1
#endif

/* Define to 1 if you have the `pow' function. */
#ifndef MAGICKCORE_HAVE_POW
#define MAGICKCORE_HAVE_POW 1
#endif

/* Define to 1 if you have the `pread' function. */
#ifndef MAGICKCORE_HAVE_PREAD
#define MAGICKCORE_HAVE_PREAD 1
#endif

/* Define to 1 if you have the <process.h> header file. */
/* #undef HAVE_PROCESS_H */

/* Define if you have POSIX threads libraries and header files. */
#ifndef MAGICKCORE_HAVE_PTHREAD
#define MAGICKCORE_HAVE_PTHREAD 1
#endif

/* Have PTHREAD_PRIO_INHERIT. */
#ifndef MAGICKCORE_HAVE_PTHREAD_PRIO_INHERIT
#define MAGICKCORE_HAVE_PTHREAD_PRIO_INHERIT 1
#endif

/* Define to 1 if you have the `pwrite' function. */
#ifndef MAGICKCORE_HAVE_PWRITE
#define MAGICKCORE_HAVE_PWRITE 1
#endif

/* Define to 1 if you have the `qsort_r' function. */
#ifndef MAGICKCORE_HAVE_QSORT_R
#define MAGICKCORE_HAVE_QSORT_R 1
#endif

/* Define to 1 if you have the `raise' function. */
#ifndef MAGICKCORE_HAVE_RAISE
#define MAGICKCORE_HAVE_RAISE 1
#endif

/* Define to 1 if you have the `rand_r' function. */
#ifndef MAGICKCORE_HAVE_RAND_R
#define MAGICKCORE_HAVE_RAND_R 1
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

/* Define to 1 if you have the `sigaction' function. */
#ifndef MAGICKCORE_HAVE_SIGACTION
#define MAGICKCORE_HAVE_SIGACTION 1
#endif

/* Define to 1 if you have the `sigemptyset' function. */
#ifndef MAGICKCORE_HAVE_SIGEMPTYSET
#define MAGICKCORE_HAVE_SIGEMPTYSET 1
#endif

/* Define to 1 if you have the `socket' function. */
#ifndef MAGICKCORE_HAVE_SOCKET
#define MAGICKCORE_HAVE_SOCKET 1
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

/* Define to 1 if you have the `strtod' function. */
#ifndef MAGICKCORE_HAVE_STRTOD
#define MAGICKCORE_HAVE_STRTOD 1
#endif

/* Define to 1 if you have the `strtod_l' function. */
#ifndef MAGICKCORE_HAVE_STRTOD_L
#define MAGICKCORE_HAVE_STRTOD_L 1
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

/* Define to 1 if you have the <sun_prefetch.h> header file. */
/* #undef HAVE_SUN_PREFETCH_H */

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

/* Define to 1 if you have the <sys/ipc.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_IPC_H
#define MAGICKCORE_HAVE_SYS_IPC_H 1
#endif

/* Define to 1 if you have the <sys/mman.h> header file. */
#ifndef MAGICKCORE_HAVE_SYS_MMAN_H
#define MAGICKCORE_HAVE_SYS_MMAN_H 1
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

/* Define to 1 if you have the `TIFFIsBigEndian' function. */
#ifndef MAGICKCORE_HAVE_TIFFISBIGENDIAN
#define MAGICKCORE_HAVE_TIFFISBIGENDIAN 1
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

/* Define to 1 if the system has the type `uintmax_t'. */
#ifndef MAGICKCORE_HAVE_UINTMAX_T
#define MAGICKCORE_HAVE_UINTMAX_T 1
#endif

/* Define to 1 if the system has the type `uintptr_t'. */
#ifndef MAGICKCORE_HAVE_UINTPTR_T
#define MAGICKCORE_HAVE_UINTPTR_T 1
#endif

/* Define to 1 if you have the `ulltostr' function. */
/* #undef HAVE_ULLTOSTR */

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef MAGICKCORE_HAVE_UNISTD_H
#define MAGICKCORE_HAVE_UNISTD_H 1
#endif

/* Define to 1 if the system has the type 'unsigned long long int'. */
#ifndef MAGICKCORE_HAVE_UNSIGNED_LONG_LONG_INT
#define MAGICKCORE_HAVE_UNSIGNED_LONG_LONG_INT 1
#endif

/* Define to 1 if you have the `uselocale' function. */
#ifndef MAGICKCORE_HAVE_USELOCALE
#define MAGICKCORE_HAVE_USELOCALE 1
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

/* Define to 1 if you have the `vfprintf' function. */
#ifndef MAGICKCORE_HAVE_VFPRINTF
#define MAGICKCORE_HAVE_VFPRINTF 1
#endif

/* Define to 1 if you have the `vfprintf_l' function. */
/* #undef HAVE_VFPRINTF_L */

/* Define to 1 if you have the `vprintf' function. */
#ifndef MAGICKCORE_HAVE_VPRINTF
#define MAGICKCORE_HAVE_VPRINTF 1
#endif

/* Define to 1 if you have the `vsnprintf' function. */
#ifndef MAGICKCORE_HAVE_VSNPRINTF
#define MAGICKCORE_HAVE_VSNPRINTF 1
#endif

/* Define to 1 if you have the `vsnprintf_l' function. */
/* #undef HAVE_VSNPRINTF_L */

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

/* Define to 1 if `fork' works. */
#ifndef MAGICKCORE_HAVE_WORKING_FORK
#define MAGICKCORE_HAVE_WORKING_FORK 1
#endif

/* Define to 1 if `vfork' works. */
#ifndef MAGICKCORE_HAVE_WORKING_VFORK
#define MAGICKCORE_HAVE_WORKING_VFORK 1
#endif

/* Define to 1 if you have the <xlocale.h> header file. */
#ifndef MAGICKCORE_HAVE_XLOCALE_H
#define MAGICKCORE_HAVE_XLOCALE_H 1
#endif

/* Define to 1 if you have the `_aligned_malloc' function. */
/* #undef HAVE__ALIGNED_MALLOC */

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

/* Whether hdri is enabled or not */
#ifndef MAGICKCORE_HDRI_ENABLE_OBSOLETE_IN_H
#define MAGICKCORE_HDRI_ENABLE_OBSOLETE_IN_H 0
#endif

/* Define if you have umem memory allocation library */
/* #undef HasUMEM */

/* Directory where ImageMagick architecture headers live. */
#ifndef MAGICKCORE_INCLUDEARCH_PATH
#define MAGICKCORE_INCLUDEARCH_PATH "/usr/local/include/ImageMagick-6/"
#endif

/* Directory where ImageMagick headers live. */
#ifndef MAGICKCORE_INCLUDE_PATH
#define MAGICKCORE_INCLUDE_PATH "/usr/local/include/ImageMagick-6/"
#endif

/* ImageMagick is formally installed under prefix */
#ifndef MAGICKCORE_INSTALLED_SUPPORT
#define MAGICKCORE_INSTALLED_SUPPORT 1
#endif

/* Define if you have JBIG library */
#ifndef MAGICKCORE_JBIG_DELEGATE
#define MAGICKCORE_JBIG_DELEGATE 1
#endif

/* Define if you have JPEG library */
#ifndef MAGICKCORE_JPEG_DELEGATE
#define MAGICKCORE_JPEG_DELEGATE 1
#endif

/* Define if you have LCMS (v1.11 or later) library */
#ifndef MAGICKCORE_LCMS_DELEGATE
#define MAGICKCORE_LCMS_DELEGATE 1
#endif

/* Define if you have OPENJP2 library */
/* #undef LIBOPENJP2_DELEGATE */

/* Directory where architecture-dependent files live. */
#ifndef MAGICKCORE_LIBRARY_PATH
#define MAGICKCORE_LIBRARY_PATH "/usr/local/lib/ImageMagick-6.8.9/"
#endif

/* Subdirectory of lib where ImageMagick architecture dependent files are
   installed */
#ifndef MAGICKCORE_LIBRARY_RELATIVE_PATH
#define MAGICKCORE_LIBRARY_RELATIVE_PATH "ImageMagick-6.8.9"
#endif

/* Binaries in libraries path base name (will be during install linked to bin)
   */
#ifndef MAGICKCORE_LIB_BIN_BASEDIRNAME
#define MAGICKCORE_LIB_BIN_BASEDIRNAME "bin"
#endif

/* Define if you have LQR library */
/* #undef LQR_DELEGATE */

/* Define if using libltdl to support dynamically loadable modules */
/* #undef LTDL_DELEGATE */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#ifndef MAGICKCORE_LT_OBJDIR
#define MAGICKCORE_LT_OBJDIR ".libs/"
#endif

/* Define if you have LZMA library */
#ifndef MAGICKCORE_LZMA_DELEGATE
#define MAGICKCORE_LZMA_DELEGATE 1
#endif

/* Define to prepend to default font search path. */
/* #undef MAGICK_FONT_PATH */

/* Target Host CPU */
#ifndef MAGICKCORE_MAGICK_TARGET_CPU
#define MAGICKCORE_MAGICK_TARGET_CPU x86_64
#endif

/* Target Host OS */
#ifndef MAGICKCORE_MAGICK_TARGET_OS
#define MAGICKCORE_MAGICK_TARGET_OS linux-gnu
#endif

/* Target Host Vendor */
#ifndef MAGICKCORE_MAGICK_TARGET_VENDOR
#define MAGICKCORE_MAGICK_TARGET_VENDOR unknown
#endif

/* Module directory name without ABI part. */
#ifndef MAGICKCORE_MODULES_BASEDIRNAME
#define MAGICKCORE_MODULES_BASEDIRNAME "modules"
#endif

/* Module directory dirname */
/* #undef MODULES_DIRNAME */

/* Magick API method prefix */
/* #undef NAMESPACE_PREFIX */

/* Define to 1 if assertions should be disabled. */
/* #undef NDEBUG */

/* Define if you have OPENEXR library */
#ifndef MAGICKCORE_OPENEXR_DELEGATE
#define MAGICKCORE_OPENEXR_DELEGATE 1
#endif

/* Name of package */
#ifndef MAGICKCORE_PACKAGE
#define MAGICKCORE_PACKAGE "ImageMagick"
#endif

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
#define MAGICKCORE_PACKAGE_STRING "ImageMagick 6.8.9-1"
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
#define MAGICKCORE_PACKAGE_VERSION "6.8.9-1"
#endif

/* Define if you have PANGOCAIRO library */
#ifndef MAGICKCORE_PANGOCAIRO_DELEGATE
#define MAGICKCORE_PANGOCAIRO_DELEGATE 1
#endif

/* Define if you have PANGO library */
#ifndef MAGICKCORE_PANGO_DELEGATE
#define MAGICKCORE_PANGO_DELEGATE 1
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
#ifndef MAGICKCORE_QUANTUM_DEPTH_OBSOLETE_IN_H
#define MAGICKCORE_QUANTUM_DEPTH_OBSOLETE_IN_H 16
#endif

/* Define as the return type of signal handlers (`int' or `void'). */
#ifndef MAGICKCORE_RETSIGTYPE
#define MAGICKCORE_RETSIGTYPE void
#endif

/* Define if you have RSVG library */
/* #undef RSVG_DELEGATE */

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

/* Sharearch directory name without ABI part. */
#ifndef MAGICKCORE_SHAREARCH_BASEDIRNAME
#define MAGICKCORE_SHAREARCH_BASEDIRNAME "config"
#endif

/* Sharearch directory dirname */
/* #undef SHAREARCH_DIRNAME */

/* Directory where architecture-independent configuration files live. */
#ifndef MAGICKCORE_SHARE_PATH
#define MAGICKCORE_SHARE_PATH "/usr/local/share/ImageMagick-6/"
#endif

/* Subdirectory of lib where architecture-independent configuration files
   live. */
#ifndef MAGICKCORE_SHARE_RELATIVE_PATH
#define MAGICKCORE_SHARE_RELATIVE_PATH "ImageMagick-6"
#endif

/* The size of `double', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_DOUBLE
#define MAGICKCORE_SIZEOF_DOUBLE 8
#endif

/* The size of `double_t', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_DOUBLE_T
#define MAGICKCORE_SIZEOF_DOUBLE_T 8
#endif

/* The size of `float', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_FLOAT
#define MAGICKCORE_SIZEOF_FLOAT 4
#endif

/* The size of `float_t', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_FLOAT_T
#define MAGICKCORE_SIZEOF_FLOAT_T 4
#endif

/* The size of `long double', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_LONG_DOUBLE
#define MAGICKCORE_SIZEOF_LONG_DOUBLE 16
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

/* The size of `ssize_t', as computed by sizeof. */
#ifndef MAGICKCORE_SIZEOF_SSIZE_T
#define MAGICKCORE_SIZEOF_SSIZE_T 8
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

/* Define to 1 if strerror_r returns char *. */
#ifndef MAGICKCORE_STRERROR_R_CHAR_P
#define MAGICKCORE_STRERROR_R_CHAR_P 1
#endif

/* Define if you have POSIX threads libraries and header files. */
#ifndef MAGICKCORE_THREAD_SUPPORT
#define MAGICKCORE_THREAD_SUPPORT 1
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


/* Version number of package */
#ifndef MAGICKCORE_VERSION
#define MAGICKCORE_VERSION "6.8.9-1"
#endif

/* Define if you have WEBP library */
#ifndef MAGICKCORE_WEBP_DELEGATE
#define MAGICKCORE_WEBP_DELEGATE 1
#endif

/* Define to use the Windows GDI32 library */
/* #undef WINGDI32_DELEGATE */

/* Define if using the dmalloc debugging malloc package */
/* #undef WITH_DMALLOC */

/* Define if you have wmflite library */
/* #undef WMFLITE_DELEGATE */

/* Define if you have wmf library */
/* #undef WMF_DELEGATE */

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

/* Build self-contained, embeddable, zero-configuration ImageMagick */
/* #undef ZERO_CONFIGURATION_SUPPORT */

/* Define if you have zlib compression library */
#ifndef MAGICKCORE_ZLIB_DELEGATE
#define MAGICKCORE_ZLIB_DELEGATE 1
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

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

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to appropriate substitue if compiler does not have __func__ */
/* #undef __func__ */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to the widest signed integer type if <stdint.h> and <inttypes.h> do
   not define. */
/* #undef intmax_t */

/* Define to the type of a signed integer type wide enough to hold a pointer,
   if such a type exists, and if the system does not define it. */
/* #undef intptr_t */

/* Define to a type if <wchar.h> does not define. */
/* #undef mbstate_t */

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

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */

/* Define to the widest unsigned integer type if <stdint.h> and <inttypes.h>
   do not define. */
/* #undef uintmax_t */

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
/* #undef uintptr_t */

/* Define as `fork' if `vfork' does not work. */
/* #undef vfork */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */
 
/* once: _MAGICK_MAGICK_BASECONFIG_H */
#endif
