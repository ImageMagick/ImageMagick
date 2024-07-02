/* config/config.h.cmake.  Hand crafted from config/config.h.in.  */
/* config/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
#cmakedefine AC_APPLE_UNIVERSAL_BUILD

/* Define if you have AUTOTRACE library */
#cmakedefine AUTOTRACE_DELEGATE

/* Define if coders and filters are to be built as modules. */
#cmakedefine BUILD_MODULES

/* Define if you have the bzip2 library */
#cmakedefine BZLIB_DELEGATE

/* Define if you have the zip library */
#cmakedefine ZIP_DELEGATE

/* Define if you have CAIRO library */
#cmakedefine CAIRO_DELEGATE

/* permit enciphering and deciphering image pixels */
#cmakedefine CIPHER_SUPPORT

/* Define to 1 if the `closedir' function returns void instead of `int'. */
#cmakedefine CLOSEDIR_VOID @CLOSEDIR_VOID@

/* coders subdirectory. */
#cmakedefine CODER_DIRNAME "@CODER_DIRNAME@"

/* Directory where architecture-dependent configuration files live. */
#cmakedefine CONFIGURE_PATH "@CONFIGURE_PATH@"

/* Subdirectory of lib where architecture-dependent configuration files live.
   */
#cmakedefine CONFIGURE_RELATIVE_PATH "@CONFIGURE_RELATIVE_PATH@"

/* Define if you have DJVU library */
#cmakedefine DJVU_DELEGATE

/* Directory where ImageMagick documents live. */
#cmakedefine DOCUMENTATION_PATH "@DOCUMENTATION_PATH@"

/* Define if you have Display Postscript */
#cmakedefine DPS_DELEGATE

/* exclude deprecated methods in MagickCore API */
#cmakedefine EXCLUDE_DEPRECATED

/* Directory where executables are installed. */
#cmakedefine EXECUTABLE_PATH "@EXECUTABLE_PATH@"

/* Define if you have FFTW library */
#cmakedefine FFTW_DELEGATE

/* filter subdirectory. */
#cmakedefine FILTER_DIRNAME "@FILTER_DIRNAME@"

/* Define if you have FLIF library */
#cmakedefine FLIF_DELEGATE

/* Define if you have FONTCONFIG library */
#cmakedefine FONTCONFIG_DELEGATE

/* Define if you have FlashPIX library */
#cmakedefine FPX_DELEGATE

/* Define if you have FREETYPE library */
#cmakedefine FREETYPE_DELEGATE

/* Define if you have Ghostscript library or framework */
#cmakedefine GS_DELEGATE

/* Define if you have GVC library */
#cmakedefine GVC_DELEGATE

/* Define to 1 if you have the `acosh' function. */
#cmakedefine HAVE_ACOSH @HAVE_ACOSH@

/* Define to 1 if you have the <arm/limits.h> header file. */
#cmakedefine HAVE_ARM_LIMITS_H @HAVE_ARM_LIMITS_H@

/* Define to 1 if you have the <arpa/inet.h> header file. */
#cmakedefine HAVE_ARPA_INET_H @HAVE_ARPA_INET_H@

/* Define to 1 if you have the `asinh' function. */
#cmakedefine HAVE_ASINH @HAVE_ASINH@

/* Define to 1 if you have the `atanh' function. */
#cmakedefine HAVE_ATANH @HAVE_ATANH@

/* Define to 1 if you have the `atexit' function. */
#cmakedefine HAVE_ATEXIT @HAVE_ATEXIT@

/* Define to 1 if you have the `atoll' function. */
#cmakedefine HAVE_ATOLL @HAVE_ATOLL@

/* define if bool is a built-in type */
#cmakedefine HAVE_BOOL

/* Define to 1 if you have the `cabs' function. */
#cmakedefine HAVE_CABS @HAVE_CABS@

/* Define to 1 if you have the `carg' function. */
#cmakedefine HAVE_CARG @HAVE_CARG@

/* Define to 1 if you have the `cimag' function. */
#cmakedefine HAVE_CIMAG @HAVE_CIMAG@

/* Define to 1 if you have the `clock' function. */
#cmakedefine HAVE_CLOCK @HAVE_CLOCK@

/* Define to 1 if you have the `clock_getres' function. */
#cmakedefine HAVE_CLOCK_GETRES @HAVE_CLOCK_GETRES@

/* Define to 1 if you have clock_gettime. */
#cmakedefine HAVE_CLOCK_GETTIME @HAVE_CLOCK_GETTIME@

/* Define to 1 if clock_gettime supports CLOCK_REALTIME. */
#cmakedefine HAVE_CLOCK_REALTIME @HAVE_CLOCK_REALTIME@

/* Define to 1 if you have the <CL/cl.h> header file. */
#cmakedefine HAVE_CL_CL_H @HAVE_CL_CL_H@

/* Define to 1 if you have the <complex.h> header file. */
#cmakedefine HAVE_COMPLEX_H @HAVE_COMPLEX_H@

/* Define to 1 if you have the `creal' function. */
#cmakedefine HAVE_CREAL @HAVE_CREAL@

/* Define to 1 if you have the `ctime_r' function. */
#cmakedefine HAVE_CTIME_R @HAVE_CTIME_R@

/* Define to 1 if you have the declaration of `pread', and to 0 if you don't.
   */
#cmakedefine01 HAVE_DECL_PREAD

/* Define to 1 if you have the declaration of `pwrite', and to 0 if you don't.
   */
#cmakedefine01 HAVE_DECL_PWRITE

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#cmakedefine01 HAVE_DECL_STRERROR_R

/* Define to 1 if you have the declaration of `strlcpy', and to 0 if you
   don't. */
#cmakedefine01 HAVE_DECL_STRLCPY

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
#cmakedefine01 HAVE_DECL_TZNAME

/* Define to 1 if you have the declaration of `vsnprintf', and to 0 if you
   don't. */
#cmakedefine01 HAVE_DECL_VSNPRINTF

/* Define to 1 if you have the `directio' function. */
#cmakedefine HAVE_DIRECTIO @HAVE_DIRECTIO@

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H @HAVE_DIRENT_H@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H @HAVE_DLFCN_H@

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#cmakedefine HAVE_DOPRNT @HAVE_DOPRNT@

/* Define to 1 if the system has the type `double_t'. */
#cmakedefine HAVE_DOUBLE_T @HAVE_DOUBLE_T@

/* Define to 1 if you have the `erf' function. */
#cmakedefine HAVE_ERF @HAVE_ERF@

/* Define to 1 if you have the <errno.h> header file. */
#cmakedefine HAVE_ERRNO_H @HAVE_ERRNO_H@

/* Define to 1 if you have the `execvp' function. */
#cmakedefine HAVE_EXECVP @HAVE_EXECVP@

/* Define to 1 if you have the `fchmod' function. */
#cmakedefine HAVE_FCHMOD @HAVE_FCHMOD@

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H @HAVE_FCNTL_H@

/* Define to 1 if the system has the type `float_t'. */
#cmakedefine HAVE_FLOAT_T @HAVE_FLOAT_T@

/* Define to 1 if you have the `floor' function. */
#cmakedefine HAVE_FLOOR @HAVE_FLOOR@

/* Define to 1 if you have the `fork' function. */
#cmakedefine HAVE_FORK @HAVE_FORK@

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#cmakedefine HAVE_FSEEKO @HAVE_FSEEKO@

/* Define to 1 if you have the `ftime' function. */
#cmakedefine HAVE_FTIME @HAVE_FTIME@

/* Define to 1 if you have the `ftruncate' function. */
#cmakedefine HAVE_FTRUNCATE @HAVE_FTRUNCATE@

/* Define to 1 if you have the `getcwd' function. */
#cmakedefine HAVE_GETCWD @HAVE_GETCWD@

/* Define to 1 if you have the `getc_unlocked' function. */
#cmakedefine HAVE_GETC_UNLOCKED @HAVE_GETC_UNLOCKED@

/* Define to 1 if you have the `getdtablesize' function. */
#cmakedefine HAVE_GETDTABLESIZE @HAVE_GETDTABLESIZE@

/* Define to 1 if you have the `getexecname' function. */
#cmakedefine HAVE_GETEXECNAME @HAVE_GETEXECNAME@

/* Define to 1 if you have the `getpagesize' function. */
#cmakedefine HAVE_GETPAGESIZE @HAVE_GETPAGESIZE@

/* Define to 1 if you have the `getpid' function. */
#cmakedefine HAVE_GETPID @HAVE_GETPID@

/* Define to 1 if you have the `getrlimit' function. */
#cmakedefine HAVE_GETRLIMIT @HAVE_GETRLIMIT@

/* Define to 1 if you have the `getrusage' function. */
#cmakedefine HAVE_GETRUSAGE @HAVE_GETRUSAGE@

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine HAVE_GETTIMEOFDAY @HAVE_GETTIMEOFDAY@

/* Define to 1 if you have the `gmtime_r' function. */
#cmakedefine HAVE_GMTIME_R @HAVE_GMTIME_R@

/* Compile with hugepage support */
#cmakedefine HAVE_HUGEPAGES

/* Define to 1 if the system has the type `intmax_t'. */
#cmakedefine HAVE_INTMAX_T @HAVE_INTMAX_T@

/* Define to 1 if the system has the type `intptr_t'. */
#cmakedefine HAVE_INTPTR_T @HAVE_INTPTR_T@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H @HAVE_INTTYPES_H@

/* Define to 1 if you have the `isnan' function. */
#cmakedefine HAVE_ISNAN @HAVE_ISNAN@

/* Define to 1 if you have the `j0' function. */
#cmakedefine HAVE_J0 @HAVE_J0@

/* Define to 1 if you have the `j1' function. */
#cmakedefine HAVE_J1 @HAVE_J1@

/* Define if you have the <lcms2.h> header file. */
#cmakedefine HAVE_LCMS2_H

/* Define if you have the <lcms2/lcms2.h> header file. */
#cmakedefine HAVE_LCMS2_LCMS2_H

/* Define if you have the <libraw/libraw.h> header file. */
#cmakedefine HAVE_LIBRAW_LIBRAW_H

/* Define if you have the <libheif/heif.h> header file. */
#cmakedefine HAVE_LIBHEIF_HEIF_H

/* Define to 1 if you have the `gcov' library (-lgcov). */
#cmakedefine HAVE_LIBGCOV @HAVE_LIBGCOV@

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H @HAVE_LIMITS_H@

/* Define to 1 if you have the <linux/unistd.h> header file. */
#cmakedefine HAVE_LINUX_UNISTD_H @HAVE_LINUX_UNISTD_H@

/* Define to 1 if you have the `lltostr' function. */
#cmakedefine HAVE_LLTOSTR @HAVE_LLTOSTR@

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine HAVE_LOCALE_H @HAVE_LOCALE_H@

/* Define to 1 if the system has the type `locale_t'. */
#cmakedefine HAVE_LOCALE_T @HAVE_LOCALE_T@

/* Define to 1 if you have the `localtime_r' function. */
#cmakedefine HAVE_LOCALTIME_R @HAVE_LOCALTIME_R@

/* Define to 1 if the system has the type `long double'. */
#cmakedefine HAVE_LONG_DOUBLE @HAVE_LONG_DOUBLE@

/* Define to 1 if the type `long double' works and has more range or precision
   than `double'. */
#cmakedefine HAVE_LONG_DOUBLE_WIDER @HAVE_LONG_DOUBLE_WIDER@

/* Define to 1 if the system has the type `long long int'. */
#cmakedefine HAVE_LONG_LONG_INT @HAVE_LONG_LONG_INT@

/* Define to 1 if you have the `lstat' function. */
#cmakedefine HAVE_LSTAT @HAVE_LSTAT@

/* Define to 1 if you have the <machine/param.h> header file. */
#cmakedefine HAVE_MACHINE_PARAM_H @HAVE_MACHINE_PARAM_H@

/* Define to 1 if you have the <mach-o/dyld.h> header file. */
#cmakedefine HAVE_MACH_O_DYLD_H @HAVE_MACH_O_DYLD_H@

/* Define to 1 if <wchar.h> declares mbstate_t. */
#cmakedefine HAVE_MBSTATE_T @HAVE_MBSTATE_T@

/* Define to 1 if you have the `memmove' function. */
#cmakedefine HAVE_MEMMOVE @HAVE_MEMMOVE@

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H @HAVE_MEMORY_H@

/* Define to 1 if you have the `memset' function. */
#cmakedefine HAVE_MEMSET @HAVE_MEMSET@

/* Define to 1 if you have the `mkstemp' function. */
#cmakedefine HAVE_MKSTEMP @HAVE_MKSTEMP@

/* Define to 1 if you have a working `mmap' system call. */
#cmakedefine HAVE_MMAP @HAVE_MMAP@

/* Define to 1 if you have the `munmap' function. */
#cmakedefine HAVE_MUNMAP @HAVE_MUNMAP@

/* define if the compiler implements namespaces */
#cmakedefine HAVE_NAMESPACES

/* Define if g++ supports namespace std. */
#cmakedefine HAVE_NAMESPACE_STD

/* Define to 1 if you have the `nanosleep' function. */
#cmakedefine HAVE_NANOSLEEP @HAVE_NANOSLEEP@

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H @HAVE_NDIR_H@

/* Define to 1 if you have the <netinet/in.h> header file. */
#cmakedefine HAVE_NETINET_IN_H @HAVE_NETINET_IN_H@

/* Define to 1 if you have the `newlocale' function. */
#cmakedefine HAVE_NEWLOCALE @HAVE_NEWLOCALE@

/* Define to 1 if you have the <OpenCL/cl.h> header file. */
#cmakedefine HAVE_OPENCL_CL_H @HAVE_OPENCL_CL_H@

/* Define to 1 if you have the <OS.h> header file. */
#cmakedefine HAVE_OS_H @HAVE_OS_H@

/* Define to 1 if you have the `pclose' function. */
#cmakedefine HAVE_PCLOSE @HAVE_PCLOSE@

/* Define to 1 if you have the `poll' function. */
#cmakedefine HAVE_POLL @HAVE_POLL@

/* Define to 1 if you have the `popen' function. */
#cmakedefine HAVE_POPEN @HAVE_POPEN@

/* Define to 1 if you have the `posix_fadvise' function. */
#cmakedefine HAVE_POSIX_FADVISE @HAVE_POSIX_FADVISE@

/* Define to 1 if you have the `posix_fallocate' function. */
#cmakedefine HAVE_POSIX_FALLOCATE @HAVE_POSIX_FALLOCATE@

/* Define to 1 if you have the `posix_madvise' function. */
#cmakedefine HAVE_POSIX_MADVISE @HAVE_POSIX_MADVISE@

/* Define to 1 if you have the `posix_memalign' function. */
#cmakedefine HAVE_POSIX_MEMALIGN @HAVE_POSIX_MEMALIGN@

/* Define to 1 if you have the `posix_spawnp' function. */
#cmakedefine HAVE_POSIX_SPAWNP @HAVE_POSIX_SPAWNP@

/* Define to 1 if you have the `pow' function. */
#cmakedefine HAVE_POW @HAVE_POW@

/* Define to 1 if you have the `pread' function. */
#cmakedefine HAVE_PREAD @HAVE_PREAD@

/* Define to 1 if you have the <process.h> header file. */
#cmakedefine HAVE_PROCESS_H @HAVE_PROCESS_H@

/* Define if you have POSIX threads libraries and header files. */
#cmakedefine HAVE_PTHREAD

/* Have PTHREAD_PRIO_INHERIT. */
#cmakedefine HAVE_PTHREAD_PRIO_INHERIT

/* Define to 1 if you have the `pwrite' function. */
#cmakedefine HAVE_PWRITE @HAVE_PWRITE@

/* Define to 1 if you have the `qsort_r' function. */
#cmakedefine HAVE_QSORT_R @HAVE_QSORT_R@

/* Define to 1 if you have the `raise' function. */
#cmakedefine HAVE_RAISE @HAVE_RAISE@

/* Define to 1 if you have the `rand_r' function. */
#cmakedefine HAVE_RAND_R @HAVE_RAND_R@

/* Define to 1 if you have the `readlink' function. */
#cmakedefine HAVE_READLINK @HAVE_READLINK@

/* Define to 1 if you have the `realpath' function. */
#cmakedefine HAVE_REALPATH @HAVE_REALPATH@

/* Define to 1 if you have the `seekdir' function. */
#cmakedefine HAVE_SEEKDIR @HAVE_SEEKDIR@

/* Define to 1 if you have the `select' function. */
#cmakedefine HAVE_SELECT @HAVE_SELECT@

/* Define to 1 if you have the `sendfile' function. */
#cmakedefine HAVE_SENDFILE @HAVE_SENDFILE@

/* Define to 1 if you have the `setlocale' function. */
#cmakedefine HAVE_SETLOCALE @HAVE_SETLOCALE@

/* Define to 1 if you have the `setvbuf' function. */
#cmakedefine HAVE_SETVBUF @HAVE_SETVBUF@

/* X11 server supports shape extension */
#cmakedefine HAVE_SHAPE

/* X11 server supports shared memory extension */
#cmakedefine HAVE_SHARED_MEMORY

/* Define to 1 if you have the `sigaction' function. */
#cmakedefine HAVE_SIGACTION @HAVE_SIGACTION@

/* Define to 1 if you have the `sigemptyset' function. */
#cmakedefine HAVE_SIGEMPTYSET @HAVE_SIGEMPTYSET@

/* Define to 1 if you have the `socket' function. */
#cmakedefine HAVE_SOCKET @HAVE_SOCKET@

/* Define to 1 if you have the `spawnvp' function. */
#cmakedefine HAVE_SPAWNVP @HAVE_SPAWNVP@

/* Define to 1 if you have the `sqrt' function. */
#cmakedefine HAVE_SQRT @HAVE_SQRT@

/* Define to 1 if you have the `stat' function. */
#cmakedefine HAVE_STAT @HAVE_STAT@

/* Define to 1 if you have the <stdarg.h> header file. */
#cmakedefine HAVE_STDARG_H @HAVE_STDARG_H@

/* Define to 1 if stdbool.h conforms to C99. */
#cmakedefine HAVE_STDBOOL_H @HAVE_STDBOOL_H@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H @HAVE_STDINT_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H @HAVE_STDLIB_H@

/* define if the compiler supports ISO C++ standard library */
#cmakedefine HAVE_STD_LIBS

/* Define to 1 if you have the `strcasecmp' function. */
#cmakedefine HAVE_STRCASECMP @HAVE_STRCASECMP@

/* Define to 1 if you have the `strchr' function. */
#cmakedefine HAVE_STRCHR @HAVE_STRCHR@

/* Define to 1 if you have the `strcspn' function. */
#cmakedefine HAVE_STRCSPN @HAVE_STRCSPN@

/* Define to 1 if you have the `strdup' function. */
#cmakedefine HAVE_STRDUP @HAVE_STRDUP@

/* Define to 1 if you have the `strerror' function. */
#cmakedefine HAVE_STRERROR @HAVE_STRERROR@

/* Define to 1 if you have the `strerror_r' function. */
#cmakedefine HAVE_STRERROR_R @HAVE_STRERROR_R@

/* Define to 1 if cpp supports the ANSI # stringizing operator. */
#cmakedefine HAVE_STRINGIZE @HAVE_STRINGIZE@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H @HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H @HAVE_STRING_H@

/* Define to 1 if you have the `strlcat' function. */
#cmakedefine HAVE_STRLCAT @HAVE_STRLCAT@

/* Define to 1 if you have the `strlcpy' function. */
#cmakedefine HAVE_STRLCPY @HAVE_STRLCPY@

/* Define to 1 if you have the `strncasecmp' function. */
#cmakedefine HAVE_STRNCASECMP @HAVE_STRNCASECMP@

/* Define to 1 if you have the `strpbrk' function. */
#cmakedefine HAVE_STRPBRK @HAVE_STRPBRK@

/* Define to 1 if you have the `strrchr' function. */
#cmakedefine HAVE_STRRCHR @HAVE_STRRCHR@

/* Define to 1 if you have the `strspn' function. */
#cmakedefine HAVE_STRSPN @HAVE_STRSPN@

/* Define to 1 if you have the `strstr' function. */
#cmakedefine HAVE_STRSTR @HAVE_STRSTR@

/* Define to 1 if you have the `strtod' function. */
#cmakedefine HAVE_STRTOD @HAVE_STRTOD@

/* Define to 1 if you have the `strtod_l' function. */
#cmakedefine HAVE_STRTOD_L @HAVE_STRTOD_L@

/* Define to 1 if you have the `strtol' function. */
#cmakedefine HAVE_STRTOL @HAVE_STRTOL@

/* Define to 1 if you have the `strtoul' function. */
#cmakedefine HAVE_STRTOUL @HAVE_STRTOUL@

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
#cmakedefine HAVE_STRUCT_TM_TM_ZONE @HAVE_STRUCT_TM_TM_ZONE@

/* Define to 1 if you have the <sun_prefetch.h> header file. */
#cmakedefine HAVE_SUN_PREFETCH_H @HAVE_SUN_PREFETCH_H@

/* Define to 1 if you have the `symlink' function. */
#cmakedefine HAVE_SYMLINK @HAVE_SYMLINK@

/* Define to 1 if you have the `sysconf' function. */
#cmakedefine HAVE_SYSCONF @HAVE_SYSCONF@

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H @HAVE_SYS_DIR_H@

/* Define to 1 if you have the <sys/ipc.h> header file. */
#cmakedefine HAVE_SYS_IPC_H @HAVE_SYS_IPC_H@

/* Define to 1 if you have the <sys/mman.h> header file. */
#cmakedefine HAVE_SYS_MMAN_H @HAVE_SYS_MMAN_H@

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H @HAVE_SYS_NDIR_H@

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H @HAVE_SYS_PARAM_H@

/* Define to 1 if you have the <sys/resource.h> header file. */
#cmakedefine HAVE_SYS_RESOURCE_H @HAVE_SYS_RESOURCE_H@

/* Define to 1 if you have the <sys/select.h> header file. */
#cmakedefine HAVE_SYS_SELECT_H @HAVE_SYS_SELECT_H@

/* Define to 1 if you have the <sys/sendfile.h> header file. */
#cmakedefine HAVE_SYS_SENDFILE_H @HAVE_SYS_SENDFILE_H@

/* Define to 1 if you have the <sys/socket.h> header file. */
#cmakedefine HAVE_SYS_SOCKET_H @HAVE_SYS_SOCKET_H@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H @HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/syslimits.h> header file. */
#cmakedefine HAVE_SYS_SYSLIMITS_H @HAVE_SYS_SYSLIMITS_H@

/* Define to 1 if you have the <sys/timeb.h> header file. */
#cmakedefine HAVE_SYS_TIMEB_H @HAVE_SYS_TIMEB_H@

/* Define to 1 if you have the <sys/times.h> header file. */
#cmakedefine HAVE_SYS_TIMES_H @HAVE_SYS_TIMES_H@

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H @HAVE_SYS_TIME_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H @HAVE_SYS_TYPES_H@

/* Define to 1 if you have the <sys/wait.h> header file. */
#cmakedefine HAVE_SYS_WAIT_H @HAVE_SYS_WAIT_H@

/* Define to 1 if you have the `telldir' function. */
#cmakedefine HAVE_TELLDIR @HAVE_TELLDIR@

/* Define to 1 if you have the `tempnam' function. */
#cmakedefine HAVE_TEMPNAM @HAVE_TEMPNAM@

/* Define to 1 if you have the <tiffconf.h> header file. */
#cmakedefine HAVE_TIFFCONF_H @HAVE_TIFFCONF_H@

/* Define to 1 if you have the `TIFFIsBigEndian' function. */
#cmakedefine HAVE_TIFFISBIGENDIAN @HAVE_TIFFISBIGENDIAN@

/* Define to 1 if you have the `TIFFIsCODECConfigured' function. */
#cmakedefine HAVE_TIFFISCODECCONFIGURED @HAVE_TIFFISCODECCONFIGURED@

/* Define to 1 if you have the `TIFFMergeFieldInfo' function. */
#cmakedefine HAVE_TIFFMERGEFIELDINFO @HAVE_TIFFMERGEFIELDINFO@

/* Define to 1 if you have the `TIFFReadEXIFDirectory' function. */
#cmakedefine HAVE_TIFFREADEXIFDIRECTORY @HAVE_TIFFREADEXIFDIRECTORY@

/* Define to 1 if you have the `TIFFSetErrorHandlerExt' function. */
#cmakedefine HAVE_TIFFSETERRORHANDLEREXT @HAVE_TIFFSETERRORHANDLEREXT@

/* Define to 1 if you have the `TIFFSetTagExtender' function. */
#cmakedefine HAVE_TIFFSETTAGEXTENDER @HAVE_TIFFSETTAGEXTENDER@

/* Define to 1 if you have the `TIFFSetWarningHandlerExt' function. */
#cmakedefine HAVE_TIFFSETWARNINGHANDLEREXT @HAVE_TIFFSETWARNINGHANDLEREXT@

/* Define to 1 if you have the `TIFFSwabArrayOfTriples' function. */
#cmakedefine HAVE_TIFFSWABARRAYOFTRIPLES @HAVE_TIFFSWABARRAYOFTRIPLES@

/* Define to 1 if you have the `times' function. */
#cmakedefine HAVE_TIMES @HAVE_TIMES@

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#cmakedefine HAVE_TM_ZONE @HAVE_TM_ZONE@

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
#cmakedefine HAVE_TZNAME @HAVE_TZNAME@

/* Define to 1 if the system has the type `uintmax_t'. */
#cmakedefine HAVE_UINTMAX_T @HAVE_UINTMAX_T@

/* Define to 1 if the system has the type `uintptr_t'. */
#cmakedefine HAVE_UINTPTR_T @HAVE_UINTPTR_T@

/* Define to 1 if you have the `ulltostr' function. */
#cmakedefine HAVE_ULLTOSTR @HAVE_ULLTOSTR@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H @HAVE_UNISTD_H@

/* Define to 1 if the system has the type `unsigned long long int'. */
#cmakedefine HAVE_UNSIGNED_LONG_LONG_INT @HAVE_UNSIGNED_LONG_LONG_INT@

/* Define to 1 if you have the `uselocale' function. */
#cmakedefine HAVE_USELOCALE @HAVE_USELOCALE@

/* Define to 1 if you have the `usleep' function. */
#cmakedefine HAVE_USLEEP @HAVE_USLEEP@

/* Define to 1 if you have the `utime' function. */
#cmakedefine HAVE_UTIME @HAVE_UTIME@

/* Define to 1 if you have the <utime.h> header file. */
#cmakedefine HAVE_UTIME_H @HAVE_UTIME_H@

/* Define to 1 if you have the `vfork' function. */
#cmakedefine HAVE_VFORK @HAVE_VFORK@

/* Define to 1 if you have the <vfork.h> header file. */
#cmakedefine HAVE_VFORK_H @HAVE_VFORK_H@

/* Define to 1 if you have the `vfprintf' function. */
#cmakedefine HAVE_VFPRINTF @HAVE_VFPRINTF@

/* Define to 1 if you have the `vfprintf_l' function. */
#cmakedefine HAVE_VFPRINTF_L @HAVE_VFPRINTF_L@

/* Define to 1 if you have the `vprintf' function. */
#cmakedefine HAVE_VPRINTF @HAVE_VPRINTF@

/* Define to 1 if you have the `vsnprintf' function. */
#cmakedefine HAVE_VSNPRINTF @HAVE_VSNPRINTF@

/* Define to 1 if you have the `vsnprintf_l' function. */
#cmakedefine HAVE_VSNPRINTF_L @HAVE_VSNPRINTF_L@

/* Define to 1 if you have the `vsprintf' function. */
#cmakedefine HAVE_VSPRINTF @HAVE_VSPRINTF@

/* Define to 1 if you have the `waitpid' function. */
#cmakedefine HAVE_WAITPID @HAVE_WAITPID@

/* Define to 1 if you have the <wchar.h> header file. */
#cmakedefine HAVE_WCHAR_H @HAVE_WCHAR_H@

/* Define to 1 if you have the <windows.h> header file. */
#cmakedefine HAVE_WINDOWS_H @HAVE_WINDOWS_H@

/* Define to 1 if `fork' works. */
#cmakedefine HAVE_WORKING_FORK @HAVE_WORKING_FORK@

/* Define to 1 if `vfork' works. */
#cmakedefine HAVE_WORKING_VFORK @HAVE_WORKING_VFORK@

/* Define to 1 if you have the <xlocale.h> header file. */
#cmakedefine HAVE_XLOCALE_H @HAVE_XLOCALE_H@

/* Define to 1 if you have the `_aligned_malloc' function. */
#cmakedefine HAVE__ALIGNED_MALLOC @HAVE__ALIGNED_MALLOC@

/* Define to 1 if the system has the type `_Bool'. */
#cmakedefine HAVE__BOOL @HAVE__BOOL@

/* Define to 1 if you have the `_exit' function. */
#cmakedefine HAVE__EXIT @HAVE__EXIT@

/* Define to 1 if you have the `_NSGetExecutablePath' function. */
#cmakedefine HAVE__NSGETEXECUTABLEPATH @HAVE__NSGETEXECUTABLEPATH@

/* Define to 1 if you have the `_pclose' function. */
#cmakedefine HAVE__PCLOSE @HAVE__PCLOSE@

/* Define to 1 if you have the `_popen' function. */
#cmakedefine HAVE__POPEN @HAVE__POPEN@

/* Define to 1 if you have the `_wfopen' function. */
#cmakedefine HAVE__WFOPEN @HAVE__WFOPEN@

/* Define to 1 if you have the `_wstat' function. */
#cmakedefine HAVE__WSTAT @HAVE__WSTAT@

/* define if your compiler has __attribute__ */
#cmakedefine HAVE___ATTRIBUTE__

/* Whether hdri is enabled or not */
#cmakedefine HDRI_ENABLE_OBSOLETE_IN_H 

/* Define if you have libheif library */
#cmakedefine HEIC_DELEGATE

/* Define if you have jemalloc memory allocation library */
#cmakedefine HASJEMALLOC

/* Define if you have umem memory allocation library */
#cmakedefine HASUMEM

/* Directory where ImageMagick architecture headers live. */
#cmakedefine INCLUDEARCH_PATH "@INCLUDEARCH_PATH@"

/* Directory where ImageMagick headers live. */
#cmakedefine INCLUDE_PATH "@INCLUDE_PATH@"

/* ImageMagick is formally installed under prefix */
#cmakedefine INSTALLED_SUPPORT @INSTALLED_SUPPORT@

/* Define if you have JBIG library */
#cmakedefine JBIG_DELEGATE

/* Define if you have JPEG library */
#cmakedefine JPEG_DELEGATE

/* Define if you have JXL library */
#cmakedefine JXL_DELEGATE

/* Define if you have LCMS library */
#cmakedefine LCMS_DELEGATE

/* Define if you have OPENJP2 library */
#cmakedefine LIBOPENJP2_DELEGATE

/* Directory where architecture-dependent files live. */
#cmakedefine LIBRARY_PATH "@LIBRARY_PATH@"

/* Subdirectory of lib where ImageMagick architecture dependent files are
   installed. */
#cmakedefine LIBRARY_RELATIVE_PATH "@LIBRARY_RELATIVE_PATH@"

/* Binaries in libraries path base name (will be during install linked to bin)
   */
#cmakedefine LIB_BIN_BASEDIRNAME "@LIB_BIN_BASEDIRNAME@"

/* Define if you have LQR library */
#cmakedefine LQR_DELEGATE

/* Define if using libltdl to support dynamically loadable modules and OpenCL
   */
#cmakedefine LTDL_DELEGATE

/* Native module suffix */
#cmakedefine LTDL_MODULE_EXT @LTDL_MODULE_EXT@

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#cmakedefine LT_OBJDIR @LT_OBJDIR@

/* Define if you have LZMA library */
#cmakedefine LZMA_DELEGATE

/* Define to prepend to default font search path. */
#cmakedefine MAGICK_FONT_PATH "@MAGICK_FONT_PATH@"

/* Target Host CPU */
#cmakedefine MAGICK_TARGET_CPU @MAGICK_TARGET_CPU@

/* Target Host OS */
#cmakedefine MAGICK_TARGET_OS @MAGICK_TARGET_OS@

/* Target Host Vendor */
#cmakedefine MAGICK_TARGET_VENDOR @MAGICK_TARGET_VENDOR@

/* Module directory name without ABI part. */
#cmakedefine MODULES_BASEDIRNAME @MODULES_BASEDIRNAME@

/* Module directory dirname */
#cmakedefine MODULES_DIRNAME "@MODULES_DIRNAME@"

/* Magick API method prefix */
#cmakedefine NAMESPACE_PREFIX @NAMESPACE_PREFIX@

/* Magick API method prefix tag */
#cmakedefine NAMESPACE_PREFIX_TAG @NAMESPACE_PREFIX_TAG@

/* Define if you have OPENEXR library */
#cmakedefine OPENEXR_DELEGATE

/* Name of package */
#cmakedefine PACKAGE @PACKAGE@

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT @PACKAGE_BUGREPORT@

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME @PACKAGE_NAME@

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING @PACKAGE_STRING@

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME @PACKAGE_TARNAME@

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL @PACKAGE_URL@

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION @PACKAGE_VERSION@

/* Define if you have PANGOCAIRO library */
#cmakedefine PANGOCAIRO_DELEGATE

/* Define if you have PANGO library */
#cmakedefine PANGO_DELEGATE

/* enable pipes (|) in filenames */
#cmakedefine PIPES_SUPPORT

/* enable POSIX support file tree on Windows */
#cmakedefine POSIX_ON_WINDOWS_SUPPORT

/* Define if you have PNG library */
#cmakedefine PNG_DELEGATE

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
#cmakedefine PTHREAD_CREATE_JOINABLE @PTHREAD_CREATE_JOINABLE@

/* Pixel cache threshold in MB (defaults to available memory) */
#cmakedefine PixelCacheThreshold @PixelCacheThreshold@

/* Number of bits in a pixel Quantum (8/16/32/64) */
#cmakedefine QUANTUM_DEPTH_OBSOLETE_IN_H @QUANTUM_DEPTH_OBSOLETE_IN_H@

/* Define if you have RAQM library */
#cmakedefine RAQM_DELEGATE @RAQM_DELEGATE@

/* Define if you have LIBRAW library */
#cmakedefine RAW_R_DELEGATE @RAW_R_DELEGATE@

/* Define as the return type of signal handlers (`int' or `void'). */
#cmakedefine RETSIGTYPE @RETSIGTYPE@

/* Define if you have RSVG library */
#cmakedefine RSVG_DELEGATE

/* Define to the type of arg 1 for `select'. */
#cmakedefine SELECT_TYPE_ARG1 @SELECT_TYPE_ARG1@

/* Define to the type of args 2, 3 and 4 for `select'. */
#cmakedefine SELECT_TYPE_ARG234 @SELECT_TYPE_ARG234@

/* Define to the type of arg 5 for `select'. */
#cmakedefine SELECT_TYPE_ARG5 @SELECT_TYPE_ARG5@

/* Setjmp/longjmp are thread safe */
#cmakedefine SETJMP_IS_THREAD_SAFE

/* Sharearch directory name without ABI part. */
#cmakedefine SHAREARCH_BASEDIRNAME @SHAREARCH_BASEDIRNAME@

/* Sharearch directory dirname */
#cmakedefine SHAREARCH_DIRNAME "@SHAREARCH_DIRNAME@"

/* Directory where architecture-independent configuration files live. */
#cmakedefine SHARE_PATH "@SHARE_PATH@"

/* Subdirectory of lib where architecture-independent configuration files
   live. */
#cmakedefine SHARE_RELATIVE_PATH "@SHARE_RELATIVE_PATH@"

/* The size of `double', as computed by sizeof. */
#cmakedefine SIZEOF_DOUBLE @SIZEOF_DOUBLE@

/* The size of `double_t', as computed by sizeof. */
#cmakedefine SIZEOF_DOUBLE_T @SIZEOF_DOUBLE_T@

/* The size of `float', as computed by sizeof. */
#cmakedefine SIZEOF_FLOAT @SIZEOF_FLOAT@

/* The size of `float_t', as computed by sizeof. */
#cmakedefine SIZEOF_FLOAT_T @SIZEOF_FLOAT_T@

/* The size of `long double', as computed by sizeof. */
#cmakedefine SIZEOF_LONG_DOUBLE @SIZEOF_LONG_DOUBLE@

/* The size of `off_t', as computed by sizeof. */
#cmakedefine SIZEOF_OFF_T @SIZEOF_OFF_T@

/* The size of `signed int', as computed by sizeof. */
#cmakedefine SIZEOF_SIGNED_INT @SIZEOF_SIGNED_INT@

/* The size of `signed long', as computed by sizeof. */
#cmakedefine SIZEOF_SIGNED_LONG @SIZEOF_SIGNED_LONG@

/* The size of `signed long long', as computed by sizeof. */
#cmakedefine SIZEOF_SIGNED_LONG_LONG @SIZEOF_SIGNED_LONG_LONG@

/* The size of `signed short', as computed by sizeof. */
#cmakedefine SIZEOF_SIGNED_SHORT @SIZEOF_SIGNED_SHORT@

/* The size of `size_t', as computed by sizeof. */
#cmakedefine SIZEOF_SIZE_T @SIZEOF_SIZE_T@

/* The size of `ssize_t', as computed by sizeof. */
#cmakedefine SIZEOF_SSIZE_T @SIZEOF_SSIZE_T@

/* The size of `unsigned int', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_INT @SIZEOF_UNSIGNED_INT@

/* The size of `unsigned int*', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_INTP @SIZEOF_UNSIGNED_INTP@

/* The size of `unsigned long', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_LONG @SIZEOF_UNSIGNED_LONG@

/* The size of `unsigned long long', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_LONG_LONG @SIZEOF_UNSIGNED_LONG_LONG@

/* The size of `void *', as computed by sizeof. */
#cmakedefine SIZEOF_VOID_P @SIZEOF_VOID_P@

/* The size of `unsigned short', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_SHORT @SIZEOF_UNSIGNED_SHORT@

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
#cmakedefine STAT_MACROS_BROKEN @STAT_MACROS_BROKEN@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS @STDC_HEADERS@

/* Define to 1 if strerror_r returns char *. */
#cmakedefine STRERROR_R_CHAR_P @STRERROR_R_CHAR_P@

/* Define if you have POSIX threads libraries and header files. */
#cmakedefine THREAD_SUPPORT

/* Define if you have TIFF library */
#cmakedefine TIFF_DELEGATE

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME @TIME_WITH_SYS_TIME@

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME @TM_IN_SYS_TIME@

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
#cmakedefine _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them (emscripten is an unassuming GNU).  */
#ifndef _GNU_SOURCE
   #ifdef __EMSCRIPTEN__
      #define _GNU_SOURCE
   #else
      #cmakedefine _GNU_SOURCE
   #endif
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
#cmakedefine _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
#cmakedefine _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
#cmakedefine __EXTENSIONS__
#endif


/* Version number of package */
#cmakedefine VERSION @VERSION@

/* Define if you have WEBPMUX library */
#cmakedefine WEBPMUX_DELEGATE

/* Define if you have WEBP library */
#cmakedefine WEBP_DELEGATE

/* Define to use the Windows GDI32 library */
#cmakedefine WINGDI32_DELEGATE

/* Define if using the dmalloc debugging malloc package */
#cmakedefine WITH_DMALLOC

/* Define if you have WMF library */
#cmakedefine WMF_DELEGATE

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#cmakedefine WORDS_BIGENDIAN
# endif
#endif

/* Location of X11 configure files */
#cmakedefine X11_CONFIGURE_PATH "@X11_CONFIGURE_PATH@"

/* Define if you have X11 library */
#cmakedefine X11_DELEGATE

/* Define if you have XML library */
#cmakedefine XML_DELEGATE

/* Define to 1 if the X Window System is missing or not being used. */
#cmakedefine X_DISPLAY_MISSING @X_DISPLAY_MISSING@

/* Build self-contained, embeddable, zero-configuration ImageMagick */
#cmakedefine01 ZERO_CONFIGURATION_SUPPORT

/* Define if you have ZLIB library */
#cmakedefine ZLIB_DELEGATE

/* Define if you have ZSTD library */
#cmakedefine ZSTD_DELEGATE

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#cmakedefine _FILE_OFFSET_BITS @_FILE_OFFSET_BITS@

/* enable run-time bounds-checking */
#cmakedefine _FORTIFY_SOURCE

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
#cmakedefine _LARGEFILE_SOURCE @_LARGEFILE_SOURCE@

/* Define for large files, on AIX-style hosts. */
#cmakedefine _LARGE_FILES

/* Define to 1 if on MINIX. */
#cmakedefine _MINIX @_MINIX@

/* Define this for the OpenCL Accelerator */
#cmakedefine _OPENCL

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#cmakedefine _POSIX_1_SOURCE @_POSIX_1_SOURCE@

/* Define to 1 if you need to in order for `stat' and other things to work. */
#cmakedefine _POSIX_SOURCE @_POSIX_SOURCE@

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#cmakedefine _UINT32_T @_UINT32_T@

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#cmakedefine _UINT64_T @_UINT64_T@

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
#cmakedefine _UINT8_T @_UINT8_T@

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
#cmakedefine __CHAR_UNSIGNED__
#endif

/* Define to appropriate substitue if compiler does not have __func__ */
#cmakedefine __func__ @__func__@

/* Define to empty if `const' does not conform to ANSI C. */
#cmakedefine const @const@

/* Define to `int' if <sys/types.h> doesn't define. */
#cmakedefine gid_t @gid_t@

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define inline @inline@
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine int16_t @int16_t@

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine int32_t @int32_t@

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine int64_t @int64_t@

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
#cmakedefine int8_t @int8_t@

/* Define to the widest signed integer type if <stdint.h> and <inttypes.h> do
   not define. */
#cmakedefine intmax_t @intmax_t@

/* Define to the type of a signed integer type wide enough to hold a pointer,
   if such a type exists, and if the system does not define it. */
#cmakedefine intptr_t @intptr_t@

/* Define to a type if <wchar.h> does not define. */
#cmakedefine mbstate_t @mbstate_t@

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine mode_t @mode_t@

/* Define to `long int' if <sys/types.h> does not define. */
#cmakedefine off_t @off_t@

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine pid_t @pid_t@

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#cmakedefine restrict @restrict@
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
#cmakedefine size_t @size_t@

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine ssize_t @ssize_t@

/* Define to `int' if <sys/types.h> doesn't define. */
#cmakedefine uid_t @uid_t@

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint16_t @uint16_t@

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint32_t @uint32_t@

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint64_t @uint64_t@

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine uint8_t @uint8_t@

/* Define to the widest unsigned integer type if <stdint.h> and <inttypes.h>
   do not define. */
#cmakedefine uintmax_t @uintmax_t@

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
#cmakedefine uintptr_t @uintptr_t@

/* Define as `fork' if `vfork' does not work. */
#cmakedefine vfork @vfork@

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
#define volatile @volatile@
