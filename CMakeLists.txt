#   Copyright 2019 Marco Martins (marcomartins86@gmail.com)
#
#   Licensed under the ImageMagick License (the "License"); you may not use
#   this file except in compliance with the License.  You may obtain a copy
#   of the License at
#
#     https://imagemagick.org/script/license.php
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
#   License for the specific language governing permissions and limitations
#   under the License.
#
#   CMake script file to build ImageMagick
#   
cmake_minimum_required(VERSION 3.15.0)
cmake_policy(VERSION 3.15.0)

project (ImageMagick)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckPrototypeDefinition)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckTypeSize)
include(CheckLibraryExists)
include(CheckCXXSourceRuns)
include(CheckStructHasMember)
include(CheckCSourceCompiles)
include(TestBigEndian)

# Compiles the source code, runs the program and sets ${VAR} to 1 if the
# return value is equal to ${RESULT}.
macro(check_run_result SRC RESULT VAR)
  message(STATUS "Performing Test ${VAR}")
  set(SRC_FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.c)
  file(WRITE ${SRC_FILE} "${SRC}")
  try_run(RUN_RESULT COMPILE_RESULT ${CMAKE_BINARY_DIR} ${SRC_FILE}
          CMAKE_FLAGS -DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES})
  if (RUN_RESULT EQUAL ${RESULT})
    set(${VAR} 1)
	message(STATUS "Performing Test ${VAR} - Success")
  else()
    message(STATUS "Performing Test ${VAR} - Failed")
  endif ()
endmacro()

# Get the platform
IF(WIN32)
  IF(NOT WINDOWS)
    SET(WINDOWS TRUE)
  ENDIF()
  #TODO check for mingw or cygwin ???
ELSEIF(UNIX AND NOT APPLE)
  IF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    SET(LINUX TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD.*")
    SET(FREEBSD TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "kNetBSD.*|NetBSD.*")
    SET(NETBSD TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "kOpenBSD.*|OpenBSD.*")
    SET(OPENBSD TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES ".*GNU.*")
    SET(GNU TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES ".*BSDI.*")
    SET(BSDI TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "DragonFly.*|FreeBSD")
    SET(FREEBSD TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "SYSV5.*")
    SET(SYSV5 TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Solaris.*")
    SET(SOLARIS TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "HP-UX.*")
    SET(HPUX TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "AIX.*")
    SET(AIX TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Minix.*")
    SET(MINIX TRUE)
  ENDIF()
ELSEIF(APPLE)
  IF(CMAKE_SYSTEM_NAME MATCHES ".*Darwin.*")
    SET(DARWIN TRUE)
  ELSEIF(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*")
    SET(MACOSX TRUE)
  ENDIF()
  # TODO: iOS?
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "BeOS.*")
  SET(BEOS TRUE)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Haiku.*")
  SET(HAIKU TRUE)
ENDIF()

# if we're on MacOS X we enable the universal build
if(APPLE)
	SET(AC_APPLE_UNIVERSAL_BUILD TRUE)
endif()

# TODO Compile without AUTOTRACE dependency
SET(AUTOTRACE_DELEGATE FALSE)

# Don't compile coders and filters as modules
SET(BUILD_MODULES FALSE)

# Check for BZip2 
FIND_PACKAGE(BZip2)
SET(BZLIB_DELEGATE ${BZIP2_FOUND})

# TODO Compile without CAIRO dependency
SET(CAIRO_DELEGATE FALSE)

# Permit enciphering and deciphering image pixels
SET(CIPHER_SUPPORT TRUE)

# Check if `closedir' function returns void instead of `int'
CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
IF (HAVE_DIRENT_H)
    UNSET(HAVE_DIRENT_H)
    CHECK_SYMBOL_EXISTS(DIR dirent.h HAVE_DIRENT_H)
    IF (NOT HAVE_DIRENT_H)
        MESSAGE(STATUS "--- DIR not defined...")
    ELSE (NOT HAVE_DIRENT_H)
        CHECK_PROTOTYPE_DEFINITION(closedir "void closedir(DIR *dirp)" "NULL" "dirent.h" CLOSEDIR_VOID)
    ENDIF (NOT HAVE_DIRENT_H)
ENDIF (HAVE_DIRENT_H)

# Not really sure what's this for
SET(CODER_DIRNAME "")

# I think this is used if we wanted to install, since we only want a static library
SET(CONFIGURE_PATH "")

# TODO should this be the path used????
SET(CONFIGURE_RELATIVE_PATH "\"ImageMagick-7\"")

# TODO Compile without DJVU dependency
SET(DJVU_DELEGATE FALSE)

# I think this is used if we wanted to install, since we only want a static library
SET(DOCUMENTATION_PATH "")

# TODO Compile without Display Postscript dependency
SET(DPS_DELEGATE FALSE)

# Give user the option to compile with deprecated methods
OPTION(EXCLUDE_DEPRECATED "Exclude deprecated methods" ON)

# TODO I think this is used if we wanted to install, since we only want a static library
SET(EXECUTABLE_PATH "")

# TODO Compile without FFTW dependency
SET(FFTW_DELEGATE FALSE)

# TODO Not really sure what's this for
SET(FILTER_DIRNAME "")

# TODO Compile without FLIF dependency
SET(FLIF_DELEGATE FALSE)

# TODO Compile without FONTCONFIG dependency
SET(FONTCONFIG_DELEGATE FALSE)

# TODO Compile without FlashPIX dependency
SET(FPX_DELEGATE FALSE)

# Check for FreeType 
FIND_PACKAGE(Freetype)
SET(FREETYPE_DELEGATE ${FREETYPE_FOUND})

# TODO Compile without Ghostscript dependency
SET(GS_DELEGATE FALSE)

# TODO Compile without GVC dependency
SET(GVC_DELEGATE FALSE)

# Check if `acosh' exists
CHECK_FUNCTION_EXISTS(acosh HAVE_ACOSH)

# Check if <arm/limits.h> exists
CHECK_INCLUDE_FILE(arm/limits.h HAVE_ARM_LIMITS_H)

# Check if <arpa/inet.h> exists
CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARM_LIMITS_H)

# Check if `asinh' exists
CHECK_FUNCTION_EXISTS(asinh HAVE_ASINH)

# Check if `atanh' exists
CHECK_FUNCTION_EXISTS(atanh HAVE_ATANH)

# Check if `atexit' exists
CHECK_FUNCTION_EXISTS(atexit HAVE_ATEXIT)

# Check if `atoll' exists
CHECK_FUNCTION_EXISTS(atoll HAVE_ATOLL)

# Check if `bool' exists (check_type_size is not working at least on windows)
CHECK_CXX_SOURCE_COMPILES ("void main () {bool b = false;}" HAVE_BOOL)

# Check if `carg' exists
CHECK_FUNCTION_EXISTS(carg HAVE_CARG)

# Check if `cabs' exists
CHECK_FUNCTION_EXISTS(cabs HAVE_CABS)

# Check if `cimag' exists
CHECK_FUNCTION_EXISTS(cimag HAVE_CIMAG)

# Check if `clock' exists
CHECK_FUNCTION_EXISTS(clock HAVE_CLOCK)

# Check if `clock_getres' exists
CHECK_FUNCTION_EXISTS(clock_getres HAVE_CLOCK_GETRES)

# Check if `clock_gettime' exists
CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)

# Check if `CLOCK_REALTIME' exists
CHECK_SYMBOL_EXISTS(CLOCK_REALTIME time.h HAVE_CLOCK_REALTIME)

# Check if <CL/cl.h> exists
CHECK_INCLUDE_FILE(CL/cl.h HAVE_CL_CL_H)

# Check if <complex.h> exists
CHECK_INCLUDE_FILE(complex.h HAVE_COMPLEX_H)

# Check if `creal' exists
CHECK_FUNCTION_EXISTS(creal HAVE_CREAL)

# Check if `ctime_r' exists
CHECK_FUNCTION_EXISTS(ctime_r HAVE_CTIME_R)

# Check if `pread' exists
CHECK_FUNCTION_EXISTS(pread HAVE_DECL_PREAD)

# Check if `pwrite' exists
CHECK_FUNCTION_EXISTS(pwrite HAVE_DECL_PWRITE)

# Check if `strerror_r' exists
CHECK_SYMBOL_EXISTS(strerror_r string.h HAVE_DECL_STRERROR_R)

# Check if `strlcpy' exists
CHECK_FUNCTION_EXISTS(strlcpy HAVE_DECL_STRLCPY)

# Check if `tzname' exists
CHECK_SYMBOL_EXISTS (tzname time.h HAVE_DECL_TZNAME)

# Check if `vsnprintf' exists
CHECK_FUNCTION_EXISTS(vsnprintf HAVE_DECL_VSNPRINTF)

# Check if `directio' exists
CHECK_FUNCTION_EXISTS (directio HAVE_DIRECTIO)

# Check if <dlfcn.h> exists
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)

# Check if `_doprnt' exists only if `vprintf' do not exists
CHECK_FUNCTION_EXISTS(vprintf HAVE_VPRINTF)
IF(NOT HAVE_VPRINTF)
	CHECK_FUNCTION_EXISTS(_doprnt HAVE_DOPRNT)
ENDIF()

# Check if `double_t' exists
CHECK_CXX_SOURCE_COMPILES (
"
  #include <math.h>
  void main () {double_t d = 0;}
" 
HAVE_DOUBLE_T)

# Check if `erf' exists
# Only check if not windows since <nt-base.h> explicit defines the macro and it will cause a lot of warnings
IF(NOT WINDOWS)
  CHECK_FUNCTION_EXISTS(erf HAVE_ERF)
ENDIF()

# Check if <errno.h> exists
CHECK_INCLUDE_FILE(errno.h HAVE_ERRNO_H)

# Check if `execvp' exists
CHECK_FUNCTION_EXISTS(execvp HAVE_EXECVP)

# Check if `fchmod' exists
CHECK_FUNCTION_EXISTS(fchmod HAVE_FCHMOD)

# Check if <fcntl.h> exists
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)

# Check if `float_t' exists
CHECK_CXX_SOURCE_COMPILES (
"
  #include <math.h> 
  void main () {float_t f = 0;}
" 
HAVE_FLOAT_T)

# Check if `floor' exists
CHECK_FUNCTION_EXISTS(floor HAVE_FLOOR)

# Check if `fork' exists
CHECK_FUNCTION_EXISTS(fork HAVE_FORK)

# Check if `fseeko' exists
CHECK_FUNCTION_EXISTS(fseeko HAVE_FSEEKO)

# Check if `ftime' exists
CHECK_FUNCTION_EXISTS(ftime HAVE_FTIME)

# Check if `ftruncate' exists
CHECK_FUNCTION_EXISTS(ftruncate HAVE_FTRUNCATE)

# Check if `getcwd' exists
CHECK_FUNCTION_EXISTS(getcwd HAVE_GETCWD)

# Check if `getc_unlocked' exists
CHECK_FUNCTION_EXISTS(getc_unlocked HAVE_GETC_UNLOCKED)

# Check if `getdtablesize' exists
CHECK_FUNCTION_EXISTS(getdtablesize HAVE_GETDTABLESIZE)

# Check if `getexecname' exists
CHECK_FUNCTION_EXISTS(getexecname HAVE_GETEXECNAME)

# Check if `getpagesize' exists
CHECK_FUNCTION_EXISTS(getpagesize HAVE_GETPAGESIZE)

# Check if `getpid' exists
CHECK_FUNCTION_EXISTS(getpid HAVE_GETPID)

# Check if `getrlimit' exists
CHECK_FUNCTION_EXISTS(getrlimit HAVE_GETRLIMIT)

# Check if `getrusage' exists
CHECK_FUNCTION_EXISTS(getrusage HAVE_GETRUSAGE)

# Check if `gettimeofday' exists
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)

# Check if `gmtime_r' exists
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GMTIME_R)

# I don't think our program will have a big memory impact
SET(HAVE_HUGEPAGES FALSE)

# Check if `intmax_t' exists
CHECK_TYPE_SIZE (intmax_t INTMAX_T)
IF(HAVE_INTMAX_T) # it was TRUE and we need it to be 1
  SET(HAVE_INTMAX_T 1)
ENDIF()

# Check if `intptr_t' exists
CHECK_TYPE_SIZE (intptr_t INTPTR_T)
IF(HAVE_INTPTR_T ) # it was TRUE and we need it to be 1
  SET(HAVE_INTPTR_T 1)
ENDIF()

# Check if <inttypes.h> exists
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)

# Check if `isnan' exists
CHECK_FUNCTION_EXISTS(isnan HAVE_ISNAN)

# Check if `j0' exists
CHECK_FUNCTION_EXISTS(j0 HAVE_J0)

# Check if `j1' exists
CHECK_FUNCTION_EXISTS(j1 HAVE_J1)

# Check if <lcms2.h> exists
CHECK_INCLUDE_FILE(lcms2.h HAVE_LCMS2_H)

# Check if <lcms2/lcms2.h> exists
CHECK_INCLUDE_FILE(lcms2/lcms2.h HAVE_LCMS2_LCMS2_H)

# Check if `gcov' exists
CHECK_LIBRARY_EXISTS(gcov "" "" HAVE_LIBGCOV)

# Check if <limits.h> exists
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)

# Check if <linux/unistd.h> exists
CHECK_INCLUDE_FILE(linux/unistd.h HAVE_LINUX_UNISTD_H)

# Check if `lltostr' exists
CHECK_FUNCTION_EXISTS(lltostr HAVE_LLTOSTR)

# Check if <locale.h> exists
CHECK_INCLUDE_FILE(locale.h HAVE_LOCALE_H)

# Check if `locale_t' exists
CHECK_TYPE_SIZE(locale_t LOCALE_T)
IF(HAVE_LOCALE_T ) # it was TRUE and we need it to be 1
  SET(HAVE_LOCALE_T 1)
ENDIF()

# Check if `localtime_r' exists
CHECK_FUNCTION_EXISTS(localtime_r HAVE_LOCALTIME_R)

# Check if `long double' exists
CHECK_TYPE_SIZE("long double" LONG_DOUBLE)
IF(HAVE_LONG_DOUBLE) # it was TRUE and we need it to be 1
  SET(HAVE_LONG_DOUBLE 1)
ENDIF()

# Check if `long double' have more precision than `double'
IF(HAVE_LONG_DOUBLE)
  CHECK_TYPE_SIZE(double DOUBLE)
  IF (${LONG_DOUBLE} GREATER ${DOUBLE})
    SET(HAVE_LONG_DOUBLE_WIDER 1)
  ENDIF()
ENDIF()

# Check if `long long int' exists
CHECK_TYPE_SIZE("long long int" LONG_LONG_INT)
IF(HAVE_LONG_LONG_INT) # it was TRUE and we need it to be 1
  SET(HAVE_LONG_LONG_INT 1)
ENDIF()

# Check if <machine/param.h> exists
CHECK_INCLUDE_FILE(machine/param.h HAVE_MACHINE_PARAM_H)

# Check if <mach-o/dyld.h.h> exists
CHECK_INCLUDE_FILE(mach-o/dyld.h HAVE_MACH_O_DYLD_H)

# Check if `mbstate_t' exists in <wchar.h>
CHECK_SYMBOL_EXISTS(mbstate_t wchar.h HAVE_MBSTATE_T)

# Check if `memmove' exists
CHECK_FUNCTION_EXISTS(memmove HAVE_MEMMOVE)

# Check if <memory.h> exists
CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)

# Check if `memset' exists
CHECK_FUNCTION_EXISTS(memset HAVE_MEMSET)

# Check if `mkstemp' exists
CHECK_FUNCTION_EXISTS(mkstemp HAVE_MKSTEMP)

# Check if `mmap' exists
CHECK_FUNCTION_EXISTS(mmap HAVE_MMAP)

# Check if `munmap' exists
CHECK_FUNCTION_EXISTS(munmap HAVE_MUNMAP)

# Check if `namespace' exists
CHECK_CXX_SOURCE_COMPILES ("namespace test {} void main() {using namespace ::test;}" HAVE_NAMESPACES)

# Check if `std::' exists
CHECK_CXX_SOURCE_COMPILES (
"
  #include <iostream> 
  void main() {std::istream& is = std::cin;}
"
HAVE_NAMESPACE_STD)

# Check if `nanosleep' exists
CHECK_FUNCTION_EXISTS(nanosleep HAVE_NANOSLEEP)

# Check if <ndir.h> exists
CHECK_INCLUDE_FILE(ndir.h HAVE_NDIR_H)

# Check if <netinet/in.h> exists
CHECK_INCLUDE_FILE(netinet/in.h HAVE_NETINET_IN_H)

# Check if `newlocale' exists
CHECK_FUNCTION_EXISTS(newlocale HAVE_NEWLOCALE)

# Check if <OpenCL/cl.h> exists
CHECK_INCLUDE_FILE(OpenCL/cl.h HAVE_OPENCL_CL_H)

# Check if <OS.h> exists
CHECK_INCLUDE_FILE(OS.h HAVE_OS_H)

# Check if `pclose' exists
CHECK_FUNCTION_EXISTS(pclose HAVE_PCLOSE)

# Check if `poll' exists
CHECK_FUNCTION_EXISTS(poll HAVE_POLL)

# Check if `popen' exists
CHECK_FUNCTION_EXISTS(popen HAVE_POPEN)

# Check if `posix_fadvise' exists
CHECK_FUNCTION_EXISTS(posix_fadvise HAVE_POSIX_FADVISE)

# Check if `posix_fallocate' exists
CHECK_FUNCTION_EXISTS(posix_fallocate HAVE_POSIX_FALLOCATE)

# Check if `posix_madvise' exists
CHECK_FUNCTION_EXISTS(posix_madvise HAVE_POSIX_MADVISE)

# Check if `posix_memalign' exists
CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)

# Check if `posix_spawnp' exists
CHECK_FUNCTION_EXISTS(posix_spawnp HAVE_POSIX_SPAWNP)

# Check if `pow' exists
CHECK_FUNCTION_EXISTS(pow HAVE_POW)

# Check if `pread' exists
CHECK_FUNCTION_EXISTS(pread HAVE_PREAD)

# Check if <process.h> exists
CHECK_INCLUDE_FILE(process.h HAVE_PROCESS_H)

# Check if POSIX threads library, header and symbols exists
CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
IF(HAVE_PTHREAD_H)
  CHECK_LIBRARY_EXISTS(pthread pthread_create "" HAVE_PTHREAD)
  CHECK_SYMBOL_EXISTS(PTHREAD_PRIO_INHERIT pthread.h HAVE_PTHREAD_PRIO_INHERIT)
ENDIF()

# Check if `pwrite' exists
CHECK_FUNCTION_EXISTS(pwrite HAVE_PWRITE)

# Check if `qsort_r' exists
CHECK_FUNCTION_EXISTS(qsort_r HAVE_QSORT_R)

# Check if `raise' exists
CHECK_FUNCTION_EXISTS(raise HAVE_RAISE)

# Check if `rand_r' exists
CHECK_FUNCTION_EXISTS(rand_r HAVE_RAND_R)

# Check if `readlink' exists
CHECK_FUNCTION_EXISTS(readlink HAVE_READLINK)

# Check if `realpath' exists
CHECK_FUNCTION_EXISTS(realpath HAVE_REALPATH)

# Check if `seekdir' exists
CHECK_FUNCTION_EXISTS(seekdir HAVE_SEEKDIR)

# Check if `select' exists
CHECK_FUNCTION_EXISTS(select HAVE_SELECT)

# Check if `sendfile' exists
CHECK_FUNCTION_EXISTS(sendfile HAVE_SENDFILE)

# Check if `setlocale' exists
CHECK_FUNCTION_EXISTS(setlocale HAVE_SETLOCALE)

# Check if `setvbuf' exists
CHECK_FUNCTION_EXISTS(setvbuf HAVE_SETVBUF)

# Check supported X11 extensions  
FIND_PACKAGE(X11)
IF(X11_Xshape_FOUND)
  SET(HAVE_SHAPE 1)
ENDIF()
IF(X11_XShm_FOUND)
  SET(HAVE_SHARED_MEMORY 1)
ENDIF()

# Check if `sigaction' exists
CHECK_FUNCTION_EXISTS(sigaction HAVE_SIGACTION)

# Check if `sigemptyset' exists
CHECK_FUNCTION_EXISTS(sigemptyset HAVE_SIGEMPTYSET)

# Check if `socket' exists
CHECK_FUNCTION_EXISTS(socket HAVE_SOCKET)

# Check if `spawnvp' exists
CHECK_FUNCTION_EXISTS(spawnvp HAVE_SPAWNVP)

# Check if `sqrt' exists
CHECK_FUNCTION_EXISTS(sqrt HAVE_SQRT)

# Check if `stat' exists
CHECK_FUNCTION_EXISTS(stat HAVE_STAT)

# Check if `stdarg' exists
CHECK_FUNCTION_EXISTS(stdarg HAVE_STDARG_H)

# Check if <stdbool.h> exists and conforms to C99
CHECK_CXX_SOURCE_COMPILES (
"
  #include <stdbool.h> 
  void main() {bool b = __bool_true_false_are_defined;}
"
HAVE_STDBOOL_H)

# Check if <stdint.h> exists
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)

# Check if <stdlib.h> exists
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)

# Check if compiler supports ISO C++ standard library
SET(CMAKE_REQUIRED_DEFINITIONS_SAVE ${CMAKE_REQUIRED_DEFINITIONS})
IF(HAVE_NAMESPACES)
  SET(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -DHAVE_NAMESPACES)
ENDIF()
CHECK_CXX_SOURCE_COMPILES (
"
  #include <map>
  #include <iomanip>
  #include <cmath>
  #ifdef HAVE_NAMESPACES
    using namespace std;
  #endif
  
  void main() {}
"
HAVE_STD_LIBS)
SET(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS_SAVE})

# Check if `strcasecmp' exists
CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)

# Check if `strchr' exists
CHECK_FUNCTION_EXISTS(strchr HAVE_STRCHR)

# Check if `strcspn' exists
CHECK_FUNCTION_EXISTS(strcspn HAVE_STRCSPN)

# Check if `strdup' exists
CHECK_FUNCTION_EXISTS(strdup HAVE_STRDUP)

# Check if `strerror' exists
CHECK_FUNCTION_EXISTS(strerror HAVE_STRERROR)

# Check if `strerror_r' exists
CHECK_FUNCTION_EXISTS(strerror_r HAVE_STRERROR_R)

# Check if `#' stringizing operator is supported
CHECK_CXX_SOURCE_RUNS(
"
  #define x(y) #y
  int main() { char c[] = \"c\"; char* p = x(c); return (c[0] != p[0]) || (c[1] != p[1]); }
"
HAVE_STRINGIZE)

# Check if <strings.h> exists
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)

# Check if <string.h> exists
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)

# Check if <strlcat.h> exists
CHECK_INCLUDE_FILE(strlcat.h HAVE_STRLCAT)

# Check if <strlcpy.h> exists
CHECK_INCLUDE_FILE(strlcpy.h HAVE_STRLCPY)

# Check if `strncasecmp' exists
CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)

# Check if `strpbrk' exists
CHECK_FUNCTION_EXISTS(strpbrk HAVE_STRPBRK)

# Check if `strrchr' exists
CHECK_FUNCTION_EXISTS(strrchr HAVE_STRRCHR)

# Check if `strspn' exists
CHECK_FUNCTION_EXISTS(strspn HAVE_STRSPN)

# Check if `strstr' exists
CHECK_FUNCTION_EXISTS(strstr HAVE_STRSTR)

# Check if `strtod' exists
CHECK_FUNCTION_EXISTS(strtod HAVE_STRSTR)

# Check if `strtod_l' exists
CHECK_FUNCTION_EXISTS(strtod_l HAVE_STRTOD_L)

# Check if `strtol' exists
CHECK_FUNCTION_EXISTS(strtol HAVE_STRTOL)

# Check if `strtoul' exists
CHECK_FUNCTION_EXISTS(strtoul HAVE_STRTOUL)

# Check if `tm_zone' is a member of `struct tm'
CHECK_STRUCT_HAS_MEMBER("struct tm" tm_zone time.h HAVE_STRUCT_TM_TM_ZONE)

# Check if <sun_prefetch.h> exists
CHECK_INCLUDE_FILE(sun_prefetch.h HAVE_SUN_PREFETCH_H)

# Check if `symlink' exists
CHECK_FUNCTION_EXISTS(symlink HAVE_SYMLINK)

# Check if `sysconf' exists
CHECK_FUNCTION_EXISTS(sysconf HAVE_SYSCONF)

# Check if <sys/dir.h> exists and defines `DIR'
CHECK_SYMBOL_EXISTS(DIR sys/dir.h HAVE_SYS_DIR_H)

# Check if <sys/ipc.h> exists
CHECK_INCLUDE_FILE(sys/ipc.h HAVE_SYS_IPC_H)

# Check if <sys/mman.h> exists
CHECK_INCLUDE_FILE(sys/mman.h HAVE_SYS_MMAN_H)

# Check if <sys/ndir.h> exists and defines `DIR'
CHECK_SYMBOL_EXISTS(DIR sys/ndir.h HAVE_SYS_NDIR_H)

# Check if <sys/param.h> exists
CHECK_INCLUDE_FILE(sys/param.h HAVE_SYS_PARAM_H)

# Check if <sys/resource.h> exists
CHECK_INCLUDE_FILE(sys/resource.h HAVE_SYS_RESOURCE_H)

# Check if <sys/select.h> exists
CHECK_INCLUDE_FILE(sys/select.h HAVE_SYS_SELECT_H)

# Check if <sys/sendfile.h> exists
CHECK_INCLUDE_FILE(sys/sendfile.h HAVE_SYS_SENDFILE_H)

# Check if <sys/socket.h> exists
CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)

# Check if <sys/stat.h> exists
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)

# Check if <sys/syslimits.h> exists
CHECK_INCLUDE_FILE(sys/syslimits.h HAVE_SYS_SYSLIMITS_H)

# Check if <sys/timeb.h> exists
CHECK_INCLUDE_FILE(sys/timeb.h HAVE_SYS_TIMEB_H)

# Check if <sys/times.h> exists
CHECK_INCLUDE_FILE(sys/times.h HAVE_SYS_TIMES_H)

# Check if <sys/time.h> exists
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)

# Check if <sys/types.h> exists
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)

# Check if <sys/wait.h> exists
CHECK_INCLUDE_FILE(sys/wait.h HAVE_SYS_WAIT_H)

# Check if `telldir' exists
CHECK_FUNCTION_EXISTS(telldir HAVE_TELLDIR)

# Check if `tempnam' exists
CHECK_FUNCTION_EXISTS(tempnam HAVE_TEMPNAM)

# TODO Is this correct ???
FIND_PACKAGE(TIFF)
IF(TIFF_FOUND)
  SET(HAVE_TIFFCONF_H 1)
  SET(HAVE_TIFFISBIGENDIAN 1)
  SET(HAVE_TIFFISCODECCONFIGURED 1)
  SET(HAVE_TIFFMERGEFIELDINFO 1)
  SET(HAVE_TIFFREADEXIFDIRECTORY 1)
  SET(HAVE_TIFFSETERRORHANDLEREXT 1)
  SET(HAVE_TIFFSETTAGEXTENDER 1)
  SET(HAVE_TIFFSETWARNINGHANDLEREXT 1)
  SET(HAVE_TIFFSWABARRAYOFTRIPLES 1)
ENDIF()

# Check if `times' exists
CHECK_FUNCTION_EXISTS(times HAVE_TIMES)

# Check if `tm_zone' is a member of `struct tm' (deprecated, use `HAVE_STRUCT_TM_TM_ZONE' instead)
CHECK_STRUCT_HAS_MEMBER("struct tm" tm_zone time.h HAVE_TM_ZONE)

# Check if `tzname' is a member of `struct tm' only if `tm_zone' isn't defined
IF(NOT HAVE_STRUCT_TM_TM_ZONE)
  CHECK_STRUCT_HAS_MEMBER("struct tm" tzname time.h HAVE_TZNAME)
ENDIF()

# Check if `uintmax_t' exists
CHECK_TYPE_SIZE(uintmax_t UINTMAX_T)
IF(HAVE_UINTMAX_T) # it was TRUE and we need it to be 1
  SET(HAVE_UINTMAX_T 1)
ENDIF()

# Check if `uintptr_t' exists
CHECK_TYPE_SIZE(uintptr_t UINTPTR_T)
IF(HAVE_UINTPTR_T) # it was TRUE and we need it to be 1
  SET(HAVE_UINTPTR_T 1)
ENDIF()

# Check if `ulltostr' exists
CHECK_FUNCTION_EXISTS(ulltostr HAVE_ULLTOSTR)

# Check if <unistd.h> exists
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)

# Check if `unsigned long long int' exists
CHECK_TYPE_SIZE("unsigned long long int" UNSIGNED_LONG_LONG_INT)
IF(HAVE_UNSIGNED_LONG_LONG_INT) # it was TRUE and we need it to be 1
  SET(HAVE_UNSIGNED_LONG_LONG_INT 1)
ENDIF()

# Check if `uselocale' exists
CHECK_FUNCTION_EXISTS(uselocale HAVE_USELOCALE)

# Check if `usleep' exists
CHECK_FUNCTION_EXISTS(usleep HAVE_USLEEP)

# Check if `utime' exists
CHECK_FUNCTION_EXISTS(utime HAVE_UTIME)

# Check if <utime.h> exists
CHECK_INCLUDE_FILE(utime.h HAVE_UTIME_H)

# Check if `vfork' exists
CHECK_FUNCTION_EXISTS(vfork HAVE_VFORK)

# Check if <vfork.h> exists
CHECK_INCLUDE_FILE(vfork.h HAVE_VFORK_H)

# Check if `vfprintf' exists
CHECK_FUNCTION_EXISTS(vfprintf HAVE_VFPRINTF)

# Check if `vfprintf_l' exists
CHECK_FUNCTION_EXISTS(vfprintf_l HAVE_VFPRINTF_L)

# Check if `vprintf' exists
CHECK_FUNCTION_EXISTS(vprintf HAVE_VPRINTF)

# Check if `vsnprintf' exists
CHECK_FUNCTION_EXISTS(vsnprintf HAVE_VSNPRINTF)

# Check if `vsnprintf_l' exists
CHECK_FUNCTION_EXISTS(vsnprintf_l HAVE_VSNPRINTF_L)

# Check if `vsprintf' exists
CHECK_FUNCTION_EXISTS(vsprintf HAVE_VSPRINTF)

# Check if `waitpid' exists
CHECK_FUNCTION_EXISTS(waitpid HAVE_WAITPID)

# Check if <wchar.h> exists
CHECK_INCLUDE_FILE(wchar.h HAVE_WCHAR_H)

# Check if <windows.h> exists
CHECK_INCLUDE_FILE(windows.h HAVE_WINDOWS_H)

# Check if `fork' works
IF(HAVE_FORK)
  CHECK_CXX_SOURCE_RUNS(
  "
    #ifdef HAVE_SYS_TYPES_H
	  #include <sys/types.h>
	#endif
	#ifdef HAVE_UNISTD_H
	  #include <unistd.h>
	#endif
    int main() { if (fork() < 0) return(1); return(0); }
  "
  HAVE_WORKING_FORK)
ENDIF()

# Check if `vfork' works
IF(HAVE_VFORK)
  CHECK_CXX_SOURCE_RUNS(
  "
    #ifdef HAVE_SYS_TYPES_H
	  #include <sys/types.h>
	#endif
	#ifdef HAVE_UNISTD_H
	  #include <unistd.h>
	#endif
	#ifdef HAVE_VFORK_H
	  #include <vfork.h>
	#endif
    int main() { if (vfork() < 0) return(1); return(0); }
  "
  HAVE_WORKING_VFORK)
ENDIF()

# Check if <xlocale.h> exists
CHECK_INCLUDE_FILE(xlocale.h HAVE_XLOCALE_H)

# Check if `_aligned_malloc' exists
CHECK_FUNCTION_EXISTS(_aligned_malloc HAVE__ALIGNED_MALLOC)

# Check if `_Bool' exists
CHECK_TYPE_SIZE(_Bool _BOOL)
IF(HAVE__BOOL) # it was TRUE and we need it to be 1
  SET(HAVE__BOOL 1)
ENDIF()

# Check if `_exit' exists
CHECK_FUNCTION_EXISTS(_exit HAVE__EXIT)

# Check if `_NSGetExecutablePath' exists
CHECK_FUNCTION_EXISTS(_NSGetExecutablePath HAVE__NSGETEXECUTABLEPATH)

# Check if `_pclose' exists
CHECK_FUNCTION_EXISTS(_pclose HAVE__PCLOSE)

# Check if `_popen' exists
CHECK_FUNCTION_EXISTS(_popen HAVE__POPEN)

# Check if `_wfopen' exists
CHECK_FUNCTION_EXISTS(_wfopen HAVE__WFOPEN)

# Check if `_wstat' exists
CHECK_FUNCTION_EXISTS(_wstat HAVE__WSTAT)

# Check if `__attribute__' exists
CHECK_C_SOURCE_COMPILES(
"
  #include <stdlib.h>
  static void foo(void) __attribute__ ((unused));
  void main() { }
"
HAVE___ATTRIBUTE__)

# This seems to be an ImageMagick flag to accurately represent the wide range of intensity levels found in real scenes 
SET(HDRI_ENABLE_OBSOLETE_IN_H FALSE)

# Check if `libheif' exists
CHECK_LIBRARY_EXISTS(libheif "" "" HEIC_DELEGATE)

# Compile without JEMALLOC dependency
SET(HasJEMALLOC FALSE)

# Compile without UMEM dependency
SET(HasUMEM FALSE)

# I think this is used if we wanted to install, since we only want a static library
SET(INCLUDEARCH_PATH "")

# I think this is used if we wanted to install, since we only want a static library
SET(INCLUDE_PATH "")

# Not really sure what's this for
SET(INSTALLED_SUPPORT "")

# TODO Compile without JBIG dependency
SET(JBIG_DELEGATE FALSE)

# Check for JPEG 
FIND_PACKAGE(JPEG)
SET(JPEG_DELEGATE ${JPEG_FOUND})

# TODO Compile without LCMS dependency
SET(LCMS_DELEGATE FALSE)

# TODO Compile without OPENJP2 dependency
SET(LIBOPENJP2_DELEGATE FALSE)

# TODO should this be the path used????
SET(LIBRARY_PATH "\"ImageMagick-7\"")

# TODO should this be the path used????
SET(LIBRARY_RELATIVE_PATH "\"ImageMagick-7\"")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(LIB_BIN_BASEDIRNAME "")

# TODO Compile without LQR dependency
SET(LQR FALSE)

# TODO Compile without libltdl dependency
SET(LTDL_DELEGATE FALSE)

# TODO We are not compiling against LTDL
SET(LTDL_MODULE_EXT "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(LT_OBJDIR "")

# Check for LibLZMA 
FIND_PACKAGE(LibLZMA)
SET(LZMA_DELEGATE ${LIBLZMA_FOUND}) 

# Set the default font search path
# TODO add dir for linux and mac os x
IF(WIN32)
  SET(MAGICK_FONT_PATH "\"$ENV{WINDIR}\\FONTS\\\"")
  # Escape the path C way where '\' must be '\\' (regex and cmake also escapes '\' so to find one we need '\\\\')
  STRING(REGEX REPLACE "\\\\" "\\\\\\\\" MAGICK_FONT_PATH ${MAGICK_FONT_PATH})
ELSEIF(UNIX AND NOT APPLE)
  SET(MAGICK_FONT_PATH "/usr/share/fonts/")
ELSEIF(APPLE)
  SET(MAGICK_FONT_PATH "/System/Library/Fonts/")
ENDIF()

#TODO should we use the host cpu or the build bitness???
SET(MAGICK_TARGET_CPU "")

#TODO what should go on the OS???
SET(MAGICK_TARGET_OS "")

#TODO what should go on the vendor???
SET(MAGICK_TARGET_VENDOR "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(MODULES_BASEDIRNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(MODULES_DIRNAME "")

# TODO if a namespace should be better define it here
SET(NAMESPACE_PREFIX "")

# TODO same as above
SET(NAMESPACE_PREFIX_TAG "")

# TODO Not sure why this isn't an ifdef inside the code, so it would check if it was on Debug or Release
SET(NDEBUG 1)

# TODO Compile without OPENEXR dependency
SET(OPENEXR_DELEGATE FALSE)

# TODO I think this is used if we wanted to install, since we only want a static library
SET(PACKAGE "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(PACKAGE_BUGREPORT "")

# TODO Will be correctly filled near the end of file
SET(PACKAGE_NAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(PACKAGE_STRING "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(PACKAGE_TARNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(PACKAGE_URL "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(PACKAGE_VERSION "")

# TODO Compile without PANGOCAIRO dependency
SET(PANGOCAIRO_DELEGATE FALSE)

# TODO Compile without PANGO dependency
SET(PANGO_DELEGATE FALSE)

# Set if pipe (|) is enabled on filenames 
# TODO Add linux and mac os X
# TODO maybe this would be better with a test somehow
IF(WIN32)
  SET(PIPES_SUPPORT FALSE)
ENDIF()

# Check for PNG 
FIND_PACKAGE(PNG)
SET(PNG_DELEGATE ${PNG_FOUND})

# TODO Not sure what to do here
SET(PTHREAD_CREATE_JOINABLE "")

# TODO Maybe this should be an option
SET(PixelCacheThreshold "")

# TODO Not sure what value to give here
SET(QUANTUM_DEPTH_OBSOLETE_IN_H 16)

# TODO Compile without RAQM dependency
SET(RAQM_DELEGATE FALSE)

# TODO Compile without LIBRAW dependency
SET(RAW_R_DELEGATE FALSE)

# Check return type of signal handlers
CHECK_C_SOURCE_COMPILES(
"
  #include <signal.h>
  #ifdef signal
    #undef signal
  #endif
  #ifdef __cplusplus
  extern \"C\" void (*signal (int, void (*)(int)))(int);
  #else
  void (*signal ()) ();
  #endif
  void main() {}
"
SIGNAL_RETURN_TYPE_IS_VOID)
IF(SIGNAL_RETURN_TYPE_IS_VOID)
  SET(RETSIGTYPE void)
ELSE(SIGNAL_RETURN_TYPE_IS_VOID)
  SET(RETSIGTYPE int)
ENDIF(SIGNAL_RETURN_TYPE_IS_VOID)

# TODO Compile without RSVG dependency
SET(RSVG_DELEGATE FALSE)

#TODO These seems to be obsolet but should we check them ????
SET(SELECT_TYPE_ARG1 "")
SET(SELECT_TYPE_ARG234 "")
SET(SELECT_TYPE_ARG5 "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(SHAREARCH_BASEDIRNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(SHAREARCH_DIRNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
SET(SHARE_PATH "")

# TODO should this be the path used????
SET(SHARE_RELATIVE_PATH "\"ImageMagick-7\"")

# Check `double' size
CHECK_TYPE_SIZE(double SIZEOF_DOUBLE)

# Check `double_t' size
IF(HAVE_DOUBLE_T)
  SET(CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES})
  SET(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} "math.h")
  CHECK_TYPE_SIZE(double_t SIZEOF_DOUBLE_T)
  SET(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE})
ENDIF()

# Check `float' size
CHECK_TYPE_SIZE(float SIZEOF_FLOAT)

# Check `float_t' size
IF(HAVE_FLOAT_T)
  SET(CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES})
  SET(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} "math.h")
  CHECK_TYPE_SIZE(float_t SIZEOF_FLOAT_T)
  SET(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE})
ENDIF()

# Check `long double' size
IF(HAVE_LONG_DOUBLE)
  CHECK_TYPE_SIZE("long double" SIZEOF_LONG_DOUBLE)
ENDIF()

# Check `off_t' size
CHECK_TYPE_SIZE(off_t SIZEOF_OFF_T)

# Check `signed int' size
CHECK_TYPE_SIZE("signed int" SIZEOF_SIGNED_INT)

# Check `signed long' size
CHECK_TYPE_SIZE("signed long" SIZEOF_SIGNED_LONG)

# Check `signed long long' size
CHECK_TYPE_SIZE("signed long long" SIZEOF_SIGNED_LONG_LONG)

# Check `signed short' size
CHECK_TYPE_SIZE("signed short" SIZEOF_SIGNED_SHORT)

# Check `size_t' size
CHECK_TYPE_SIZE("size_t" SIZEOF_SIZE_T)

# Check `ssize_t' size
CHECK_TYPE_SIZE("ssize_t" SIZEOF_SSIZE_T)

# Check `unsigned int' size
CHECK_TYPE_SIZE("unsigned int" SIZEOF_UNSIGNED_INT)

# Check `unsigned int*' size
CHECK_TYPE_SIZE("unsigned int*" SIZEOF_UNSIGNED_INTP)

# Check `unsigned long' size
CHECK_TYPE_SIZE("unsigned long" SIZEOF_UNSIGNED_LONG)

# Check `unsigned long long' size
CHECK_TYPE_SIZE("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)

# Check `unsigned long long' size
CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)

# Check `unsigned short' size
CHECK_TYPE_SIZE("unsigned short" SIZEOF_UNSIGNED_SHORT)

# TODO Not sure how to heck if the `S_IS*' macros in <sys/stat.h> are broken
# Should we test them all ???? 
SET(STAT_MACROS_BROKEN 0)

# Check ANSI C header files exists
CHECK_INCLUDE_FILES("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

# Check strerror_r returns `char *'
CHECK_CXX_SOURCE_COMPILES(
"
  void main() 
  {
    char buf[100];
    char x = *strerror_r(0, buf, sizeof buf);
    char *p = strerror_r(0, buf, sizeof buf);
  }
"
STRERROR_R_CHAR_P)

# Check POSIX threads libraries and header files exists
FIND_PACKAGE(Threads)
IF(CMAKE_USE_PTHREADS_INIT)
  SET(THREAD_SUPPORT TRUE)
  SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  SET(THREADS_PREFER_PTHREAD_FLAG TRUE)
ENDIF()

# Check for TIFF 
# FIND_PACKAGE(TIFF) already called above
SET(TIFF_DELEGATE ${TIFF_FOUND})

# Check if we can safely include both <sys/time.h> and <time.h>
CHECK_CXX_SOURCE_COMPILES(
"
  #include <sys/time.h>
  #include <time.h>
  void main(void){}
"
TIME_WITH_SYS_TIME)

# Check if `struct tm' exists in <sys/time.h>
IF(HAVE_SYS_TIME_H)
  CHECK_SYMBOL_EXISTS("struct tm" sys/time.h TM_IN_SYS_TIME)
ENDIF()

# Check if we are on AIX, do the _ALL_SOURCE magic
IF(AIX)
  SET(_ALL_SOURCE 1)
ENDIF()

# Check if _GNU_SOURCE is available
CHECK_SYMBOL_EXISTS(__GNU_LIBRARY__ features.h _GNU_SOURCE)

# Check if we're on Solaris
IF (SOLARIS)
  SET(_POSIX_PTHREAD_SEMANTICS TRUE)
  SET(__EXTENSIONS__ TRUE)
ENDIF()

# TODO Not sure how to test this, so for now it's disabled
SET(_TANDEM_SOURCE FALSE)

# TODO Define the version
SET(VERSION "")

# TODO Compile without WEBPMUX dependency
SET(WEBPMUX_DELEGATE FALSE)

# TODO Compile without WEBP dependency
SET(WEBP_DELEGATE FALSE)

# TODO Should we check if gdi32 exists if windows ????
IF(WIN32)
  SET(WINGDI32_DELEGATE TRUE)
ENDIF()

OPTION(WITH_DMALLOC "Use dmalloc for memory profiling" OFF)

# TODO Compile without WMF dependency
SET(WMF_DELEGATE FALSE)

# Check if system is Big Endian
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

# TODO check if this is supposed to be any of the ones defined https://cmake.org/cmake/help/v3.0/module/FindX11.html
SET(X11_CONFIGURE_PATH "")

# Compile with X11 if present
# FIND_PACKAGE(X11) was already called above
SET(X11_DELEGATE ${X11_FOUND})

# TODO Compile without XML dependency, which XML lib is?
SET(XML_DELEGATE FALSE)

# TODO should this be an option ??
SET(X_DISPLAY_MISSING 1)

# TODO Should we try this ???? also option??
SET(ZERO_CONFIGURATION_SUPPORT 0)

# Check for ZLIB
FIND_PACKAGE(ZLIB)
SET(ZLIB_DELEGATE ${ZLIB_FOUND})

# TODO Compile without ZSTD dependency
SET(ZSTD_DELEGATE FALSE)

# TODO does this suffice?
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
  SET(_FILE_OFFSET_BITS 64)
  SET(_LARGEFILE_SOURCE 1)
ELSE()
# Not sure on Windows if code is made to work with 64 bits 
  SET(_FILE_OFFSET_BITS 32)
ENDIF()

# Disable run-time bounds-checking
SET(_FORTIFY_SOURCE FALSE)

# Check if we are on AIX, activate _LARGE_FILES
IF(AIX)
  SET(_LARGE_FILES 1)
ENDIF()

# Check if we are on MINIX
CHECK_SYMBOL_EXISTS(_MINIX "stdio.h" EVENT___MINIX)

# Set OpenCL Accelerator
# TODO does it suffice to enable only if we have the headers ???
IF((HAVE_OPENCL_CL_H OR HAVE_CL_CL_H) AND DEFINED OPENCL_LIBRARY)
  SET(_OPENCL 1)
  SET(OPENCLLIB_DELEGATE TRUE)
ELSE()
  SET(_OPENCL "")
ENDIF()

# Check if system does not provide POSIX.1 features except with this defined
#TODO does this suffice ????
CHECK_SYMBOL_EXISTS(_POSIX_1_SOURCE "stdio.h" EVENT___POSIX_1_SOURCE)
IF(NOT _POSIX_1_SOURCE)
  SET(_POSIX_1_SOURCE 2)
ENDIF()

# TODO Is this true or should it be 1 when not found???
CHECK_SYMBOL_EXISTS(_POSIX_SOURCE "stdio.h" EVENT___POSIX_SOURCE)

#TODO restrict the system version and check if the define should be these
IF(SOLARIS)
  SET(_UINT32_T uint32_t)
  SET(_UINT64_T uint64_t)
  SET(_UINT8_T uint8_t)
ENDIF()

IF(NOT CMAKE_COMPILER_IS_GNUCC)
  CHECK_RUN_RESULT(
  "
    #include <limits.h>
    int main (void) { return CHAR_MIN == 0; }
  "
  1
  __CHAR_UNSIGNED__)
ENDIF()

# Check for compiler `__func__' compatibility
CHECK_C_SOURCE_COMPILES("void main() {char *function_name = __func__;}" HAVE___FUNC__)
CHECK_C_SOURCE_COMPILES("void main() {char *function_name = __FUNCTION__;}" HAVE___FUNCTION__)

IF(HAVE___FUNC__)
  SET(__func__ __func__)
ELSEIF(HAVE___FUNCTION__) 
  SET(__func__ __FUNCTION__)
ELSE()
  SET(__func__ "")
ENDIF()

# Check if `const' is supported by compiler 
CHECK_C_SOURCE_COMPILES("void main() {const char *s = \"Test\";}" HAVE_CONST)
# Only set const to empty if it doesn't exist otherwise magick++ will not compile
IF(NOT HAVE_CONST)
  SET(const " ")
ENDIF()

# Check if <sys/types.h> doesn't define `gid_t'
IF(HAVE_SYS_TYPES_H)
  CHECK_SYMBOL_EXISTS(gid_t sys/types.h HAVE_GID_T)
  IF(NOT HAVE_GID_T)
    SET(gid_t int)
  ENDIF()
ENDIF()

# Check for the compiler inline compatible instruction
CHECK_C_SOURCE_COMPILES(
  "static inline int test (void) {return 0;}\nint main (void) {return test();}"
  HAVE_INLINE)

CHECK_C_SOURCE_COMPILES (
  "static __inline int test (void) {return 0;}\nint main (void) {return test();}"
  HAVE___INLINE)

CHECK_C_SOURCE_COMPILES (
  "static __inline__ int test (void) {return 0;}\nint main (void) {return test();}"
  HAVE___INLINE__)

IF(HAVE_INLINE)
	SET(inline inline)
ELSEIF(HAVE___INLINE)
	SET(inline __inline)
ELSEIF(HAVE___INLINE__)
	SET(inline __inline__)
ELSE()
	SET(inline "")
ENDIF()

#TODO these defines if system doesn't define them
SET(int16_t "")
SET(int32_t "")
SET(int64_t "")
SET(int8_t "")
SET(intmax_t "")
SET(intptr_t "")
SET(mbstate_t "")

# Check if <sys/types.h> doesn't define `mode_t'
IF(HAVE_SYS_TYPES_H)
  CHECK_SYMBOL_EXISTS(mode_t sys/types.h HAVE_MODE_T)
  IF(NOT HAVE_MODE_T)
    SET(mode_t int)
  ENDIF()
ENDIF()

# Check if <sys/types.h> doesn't define `pid_t'
IF(HAVE_SYS_TYPES_H)
  CHECK_SYMBOL_EXISTS(pid_t sys/types.h HAVE_PID_T)
  IF(NOT HAVE_PID_T)
    SET(pid_t int)
  ENDIF()
ENDIF()

# Check for the compiler restrict compatible instruction
CHECK_C_SOURCE_COMPILES(
  "int test (void *restrict x);\nint main (void) {return 0;}"
  HAVE_RESTRICT)

CHECK_C_SOURCE_COMPILES(
"typedef struct abc *d;\nint test (d __restrict x);\nint main (void) {return 0;}"
  HAVE___RESTRICT)

IF(HAVE___RESTRICT)
  SET(restrict __restrict)
ELSEIF(NOT HAVE_RESTRICT)
  SET(restrict " ")
ENDIF()

# Check if <sys/types.h> doesn't define `size_t'
IF(HAVE_SYS_TYPES_H)
  IF(SIZEOF_SIZE_T)
    SET(HAVE_SIZE_T 1)
  ELSE()
    CHECK_SYMBOL_EXISTS(size_t sys/types.h HAVE_SIZE_T)
    IF(NOT HAVE_SIZE_T)
      IF("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        SET(size_t "unsigned int")
      ELSE()
        SET(size_t "unsigned long long")
      ENDIF()
	  SET(SIZEOF_SIZE_T ${CMAKE_SIZEOF_VOID_P})
    ENDIF()
  ENDIF()
ENDIF()

# Check if <sys/types.h> doesn't define `ssize_t'
IF(HAVE_SYS_TYPES_H)
  IF(SIZEOF_SSIZE_T)
    SET(HAVE_SSIZE_T 1)
  ELSE()
    CHECK_SYMBOL_EXISTS(ssize_t sys/types.h HAVE_SSIZE_T)
    IF(NOT HAVE_SSIZE_T)
      IF("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        SET(ssize_t int)
      ELSE()
        SET(ssize_t "long long")
      ENDIF()
	  SET(SIZEOF_SSIZE_T ${CMAKE_SIZEOF_VOID_P})
	ENDIF()
  ENDIF()
ENDIF()

# Check if <sys/types.h> doesn't define `uid_t'
IF(HAVE_SYS_TYPES_H)
  CHECK_SYMBOL_EXISTS(uid_t sys/types.h HAVE_UID_T)
  IF(NOT HAVE_UID_T)
    SET(uid_t int)
  ENDIF()
ENDIF()

#TODO these defines if system doesn't define them
SET(uint16_t "")
SET(uint32_t "")
SET(uint64_t "")
SET(uint8_t "")
SET(uintmax_t "")
SET(uintptr_t "")

# Check if `vfork' is not working and define it as `fork'
IF(NOT HAVE_WORKING_VFORK)
  SET(vfork fork)
ENDIF()

# Check if `volatile' works
CHECK_CXX_SOURCE_COMPILES(
"
void main() { volatile int i = 1; }
"
HAVE_VOLATILE)

IF(HAVE_VOLATILE)
  SET(volatile volatile)
ELSE()
  SET(volatile "")
ENDIF()

# Give user the choice for MAGICKCORE_QUANTUM_DEPTH bits
SET(MAGICKCORE_QUANTUM_DEPTH "16" CACHE STRING "Number of bits in a pixel Quantum")
SET_PROPERTY(CACHE MAGICKCORE_QUANTUM_DEPTH PROPERTY STRINGS "8;16;32;64")

# Give user the choice for MAGICK_STATIC_BUILD bits
OPTION(MAGICK_BUILD_STATIC "Build Image Magick statically" OFF)
IF(MAGICK_BUILD_STATIC)
  SET(MAGICK_BUILD_DEFINE "_LIB")
ELSE()
  SET(MAGICK_BUILD_DEFINE "_DLL")
ENDIF()

# Fetch some needed values from version.sh and fill the correspondent variables
FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/configure CONTENT_CONFIGURE)

# Try to find the date in the format yyyy-mm-dd
STRING(REGEX MATCH "\nPACKAGE_RELEASE_DATE=([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9])\n" FOUND_LAST_CHANGE_DATE "${CONTENT_CONFIGURE}")
IF(FOUND_LAST_CHANGE_DATE)
  SET(PACKAGE_RELEASE_DATE ${CMAKE_MATCH_1})
ENDIF()

# Try to find `PACKAGE_NAME'
STRING(REGEX MATCH "\nPACKAGE_NAME='([A-Za-z0-9]+)'\n" FOUND_PACKAGE_NAME ${CONTENT_CONFIGURE})
IF(FOUND_PACKAGE_NAME)
  SET(PACKAGE_NAME ${CMAKE_MATCH_1})
ENDIF()

# Try to find `PACKAGE_LIB_VERSION'
STRING(REGEX MATCH "\nPACKAGE_LIB_VERSION=([A-Fa-f0-9x]+)\n" FOUND_PACKAGE_LIB_VERSION ${CONTENT_CONFIGURE})
IF(FOUND_PACKAGE_LIB_VERSION)
  SET(PACKAGE_LIB_VERSION ${CMAKE_MATCH_1})
ENDIF()

# Try to find `PACKAGE_VERSION'
STRING(REGEX MATCH "\nPACKAGE_BASE_VERSION=([0-9.]+)\n" FOUND_PACKAGE_VERSION ${CONTENT_CONFIGURE})
IF(FOUND_PACKAGE_VERSION)
  SET(MAGICK_LIB_VERSION_TEXT ${CMAKE_MATCH_1})
  SET(MAGICKPP_LIB_VERSION_TEXT ${CMAKE_MATCH_1})
ENDIF()

# Try parse just the major version number from MAGICK_LIB_VERSION_TEXT
STRING(REGEX MATCH "([0-9])\\..*" FOUND_VERSION_MAJOR "${MAGICK_LIB_VERSION_TEXT}")
IF(FOUND_VERSION_MAJOR)
  SET(MAGICK_VERSION_MAJOR ${CMAKE_MATCH_1})
ENDIF()

# Try to find `MAGICK_LIBRARY_CURRENT', `MAGICK_LIBRARY_REVISION' and `MAGICK_LIBRARY_AGE'
STRING(REGEX MATCH "\nMAGICK_LIBRARY_CURRENT=([0-9]+)\nMAGICK_LIBRARY_REVISION=([0-9]+)\nMAGICK_LIBRARY_AGE=([0-9]+)\n" FOUND_MAGICK_LIB_VERSION_NUMBER ${CONTENT_CONFIGURE})
IF(FOUND_MAGICK_LIB_VERSION_NUMBER)
  SET(MAGICK_LIB_VERSION_NUMBER "${CMAKE_MATCH_1},${CMAKE_MATCH_2},${CMAKE_MATCH_3}")
  SET(MAGICK_LIBRARY_CURRENT ${CMAKE_MATCH_1})
  SET(MAGICK_LIBRARY_CURRENT_MIN ${CMAKE_MATCH_1})
ENDIF()

# Try to find `PACKAGE_RELEASE'
STRING(REGEX MATCH "\nPACKAGE_VERSION_ADDENDUM=-([0-9]+)\n" FOUND_PACKAGE_RELEASE ${CONTENT_CONFIGURE})
IF(FOUND_PACKAGE_RELEASE)
  SET(PACKAGE_VERSION_ADDENDUM "-${CMAKE_MATCH_1}")
ENDIF()

# Try to find `MAGICKPP_LIBRARY_CURRENT', `MAGICKPP_LIBRARY_REVISION' and `MAGICKPP_LIBRARY_AGE'
STRING(REGEX MATCH "\nMAGICKPP_LIBRARY_CURRENT=([0-9]+)\nMAGICKPP_LIBRARY_REVISION=([0-9]+)\nMAGICKPP_LIBRARY_AGE=([0-9]+)\n" FOUND_MAGICKPP_LIBRARY_VERSION_INFO ${CONTENT_CONFIGURE})
IF(FOUND_MAGICKPP_LIBRARY_VERSION_INFO)
  SET(MAGICKPP_LIBRARY_VERSION_INFO "${CMAKE_MATCH_1}:${CMAKE_MATCH_2}:${CMAKE_MATCH_3}")
  SET(MAGICKPP_LIBRARY_CURRENT ${CMAKE_MATCH_1})
  SET(MAGICKPP_LIBRARY_CURRENT_MIN ${CMAKE_MATCH_1})
ENDIF()

# Set the version.h file path to be autogenerated
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/MagickCore/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/MagickCore/version.h)

IF(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 8)
  SET(MagickQuantumDepth "Q8")
ELSEIF(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 16)
  SET(MagickQuantumDepth "Q16")
ELSEIF(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 32)
  SET(MagickQuantumDepth "Q32")
ELSEIF(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 64)
  SET(MagickQuantumDepth "Q64")
ELSE()
  SET(MagickQuantumDepth "Q?")
ENDIF()

# rc files are only used on Windows binaries
IF(WIN32)
  SET(MagickLibVersionNumber ${MAGICK_LIB_VERSION_NUMBER})
  SET(MagickLibVersionText \"${MAGICK_LIB_VERSION_TEXT}\")
  
  IF(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    SET(MagickPlatform "x86")
  ELSEIF(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    SET(MagickPlatform "x64")
  ELSE()
    SET(MagickPlatform ${MAGICK_TARGET_CPU})
  ENDIF()
  
  # Read the version.h file so we can get some needed values using regex
  FILE(READ ${CMAKE_CURRENT_BINARY_DIR}/MagickCore/version.h CONTENT_VERSION_H)
  
  # Try to find `MagickCopyright'
  STRING(REGEX MATCH "\n#define[ ]+MagickCopyright[ ]+\"([^\"]+)\"\n" FOUND_MAGICK_COPYRIGHT ${CONTENT_VERSION_H})
  IF(FOUND_MAGICK_COPYRIGHT)
    SET(MagickCopyright \"${CMAKE_MATCH_1}\")
  ENDIF()
  
  # Try to find `MagickAuthoritativeURL'
  STRING(REGEX MATCH "\n#define[ ]+MagickAuthoritativeURL[ ]+\"([^\"]+)\"\n" FOUND_MAGICK_AUTHORITATIVE_URL ${CONTENT_VERSION_H})
  IF(FOUND_MAGICK_AUTHORITATIVE_URL)
    SET(MagickAuthoritativeURL ${CMAKE_MATCH_1})
  ENDIF()

  SET(MagickVersion "\"${PACKAGE_NAME} ${MAGICK_LIB_VERSION_TEXT}${PACKAGE_VERSION_ADDENDUM} ${MagickQuantumDepth} ${MagickPlatform} ${PACKAGE_RELEASE_DATE} ${MagickAuthoritativeURL}\"")
  
  # Set the ImageMagick.rc file path to be autogenerated
  SET(MAGICK_RESOURCE_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/config/resources/ImageMagick.rc)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/ImageMagick.rc.cmake ${MAGICK_RESOURCE_FILE_PATH})
  # Copy only the needed resource files to output directory
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/images/ImageMagick.ico ${CMAKE_CURRENT_BINARY_DIR}/images/ImageMagick.ico COPYONLY)  
ENDIF()

# Before making the config.h we still need to do somethings
SET(PACKAGE_NAME \"${PACKAGE_NAME}\")

# Set the config.h file path to be autogenerated
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config/config.h)

# Now we need to read its content to generate the magick-baseconfig.h header
FILE(READ ${CMAKE_CURRENT_BINARY_DIR}/config/config.h CONTENT_CONFIG_H)

# Now we need to Regex Replace some things in the header file
# All caps defines will be prefixed with a macro MAGICKCORE_
STRING(REGEX REPLACE 
"\\*\\/\n#define[ ]+([A-Z0-9_]+)[ ]*([a-zA-Z0-9\":\\\-]*)\n" 
"*/\n#ifndef MAGICKCORE_\\1\n#define MAGICKCORE_\\1 \\2\n#endif\n"
CONTENT_CONFIG_H ${CONTENT_CONFIG_H})
# All uncaps defines will be prefixed with a macro _magickcore_
STRING(REGEX REPLACE 
"\\*\\/\n#define[ ]+([a-z0-9_]+)[ ]*([a-zA-Z0-9 \"_]*)\n" 
"*/\n#ifndef _magickcore_\\1\n#define _magickcore_\\1 \\2\n#endif\n"
CONTENT_CONFIG_H ${CONTENT_CONFIG_H})
# For some reason (I can't seem to find the logic) there's some defines we need to remove the prefix
STRING(REGEX REPLACE "_magickcore___func__" "__func__" CONTENT_CONFIG_H ${CONTENT_CONFIG_H})
# Also for ssize_t we need to typedef instead of define or else magick++ will not compile
STRING(REGEX REPLACE 
"#ifndef _magickcore_ssize_t\n#define _magickcore_ssize_t ([a-zA-Z ]*)\n#endif" 
"typedef \\1 ssize_t;"
CONTENT_CONFIG_H ${CONTENT_CONFIG_H})

# Now we need to append things to the previously read file content
SET(CONTENT_CONFIG_H 
"#ifndef _MAGICKCORE_MAGICK_BASECONFIG_H
#define _MAGICKCORE_MAGICK_BASECONFIG_H 1

/* MagickCore/magick-baseconfig.h. Generated automatically at end of configure. */
/* config/config.h.  Generated from config.h.cmake by CMake.  */
${CONTENT_CONFIG_H}

/* once: _MAGICKCORE_MAGICK_BASECONFIG_H */
#endif
"
)

# Create the magick-baseconfig.h file
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/MagickCore/magick-baseconfig.h "${CONTENT_CONFIG_H}")

# Set ImageMagick base dir to be used inside sub-projects
SET(IMAGEMAGICK_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Set coders andd filters path
SET(CODERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/coders)
SET(FILTERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/filters)

SET(IMAGEMAGICK_INC_DIR "include/ImageMagick-${MAGICK_VERSION_MAJOR}")

# Build delegates include directories and libraries to link
# Let's fill a variable with the include dirs for the available delegates
SET(IMAGEMAGICK_DELEGATES_INCLUDE_DIRS "")
# Let's fill a variable with the libraries to link for the available delegates
SET(IMAGEMAGICK_DELEGATES_LIBRARIES_LINK "")
# Let's fill a variable with the libraries compiler definitions for the available delegates
SET(IMAGEMAGICK_DELEGATES_COMPILER_DEFINITIONS "")
#TODO add libraries and include dirs for other delegates
IF(BZLIB_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${BZIP2_INCLUDE_DIR})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${BZIP2_LIBRARIES})
ENDIF()
IF(FREETYPE_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${FREETYPE_LIBRARIES})
ENDIF()
IF(JPEG_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${JPEG_LIBRARIES})
ENDIF()
IF(LZMA_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${LIBLZMA_INCLUDE_DIRS})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${LIBLZMA_LIBRARIES})
ENDIF()
IF(PNG_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${PNG_INCLUDE_DIRS})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${PNG_LIBRARIES})
  LIST(APPEND IMAGEMAGICK_DELEGATES_COMPILER_DEFINITIONS ${PNG_DEFINITIONS})
ENDIF()
IF(TIFF_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${TIFF_INCLUDE_DIRS})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${TIFF_LIBRARIES})
ENDIF()
IF(X11_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${X11_DELEGATE})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${X11_LIBRARIES})
ENDIF()
IF(ZLIB_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${ZLIB_LIBRARIES})
  OPTION(LZMA_API_STATIC "Use lib LZMA as static?" OFF)
  IF(LZMA_API_STATIC)
    LIST(APPEND IMAGEMAGICK_DELEGATES_COMPILER_DEFINITIONS "-DLZMA_API_STATIC")
  ENDIF()
ENDIF()
IF(OPENCLLIB_DELEGATE)
  LIST(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${OPENCL_LIBRARY})
ENDIF()

#Although this is not a delegate we can add the pthread library dependency here
IF(THREAD_SUPPORT)
  list(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

# Set the binaries name
SET(MAGICKCORE_BINARY_NAME "MagickCore-${MAGICK_VERSION_MAJOR}.${MagickQuantumDepth}")
SET(MAGICKWAND_BINARY_NAME "MagickWand-${MAGICK_VERSION_MAJOR}.${MagickQuantumDepth}")
SET(MAGICKXX_BINARY_NAME "Magick++-${MAGICK_VERSION_MAJOR}.${MagickQuantumDepth}")
SET(MAGICK_BINARY_NAME "Magick-${MAGICK_VERSION_MAJOR}.${MagickQuantumDepth}")

# Add the subdirectories to be compiled
ADD_SUBDIRECTORY("MagickCore")
# TODO: Add options for these
ADD_SUBDIRECTORY("MagickWand")
ADD_SUBDIRECTORY("Magick++")
ADD_SUBDIRECTORY("utilities")
