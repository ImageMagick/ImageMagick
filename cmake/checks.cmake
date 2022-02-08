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

macro(magick_check_env)
  # Check if `closedir' function returns void instead of `int'
  CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
  if(HAVE_DIRENT_H)
      unset(HAVE_DIRENT_H)
      CHECK_SYMBOL_EXISTS(DIR dirent.h HAVE_DIRENT_H)
      if(NOT HAVE_DIRENT_H)
          message(STATUS "--- DIR not defined...")
      else(NOT HAVE_DIRENT_H)
          CHECK_PROTOTYPE_DEFINITION(closedir "void closedir(DIR *dirp)" "NULL" "dirent.h" CLOSEDIR_VOID)
      endif(NOT HAVE_DIRENT_H)
  endif(HAVE_DIRENT_H)

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
  if(NOT HAVE_VPRINTF)
    CHECK_FUNCTION_EXISTS(_doprnt HAVE_DOPRNT)
  endif()

  # Check if `double_t' exists
  CHECK_CXX_SOURCE_COMPILES (
  "
    #include <math.h>
    void main () {double_t d = 0;}
  " 
  HAVE_DOUBLE_T)

  # Check if `erf' exists
  # Only check if not windows since <nt-base.h> explicit defines the macro and it will cause a lot of warnings
  if(NOT WINDOWS)
    CHECK_FUNCTION_EXISTS(erf HAVE_ERF)
  endif()

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
  set(HAVE_HUGEPAGES FALSE)

  # Check if `intmax_t' exists
  CHECK_TYPE_SIZE (intmax_t INTMAX_T)
  if(HAVE_INTMAX_T) # it was TRUE and we need it to be 1
    set(HAVE_INTMAX_T 1)
  endif()

  # Check if `intptr_t' exists
  CHECK_TYPE_SIZE (intptr_t INTPTR_T)
  if(HAVE_INTPTR_T ) # it was TRUE and we need it to be 1
    set(HAVE_INTPTR_T 1)
  endif()

  # Check if <inttypes.h> exists
  CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)

  # Check if `isnan' exists
  CHECK_FUNCTION_EXISTS(isnan HAVE_ISNAN)

  # Check if `j0' exists
  CHECK_FUNCTION_EXISTS(j0 HAVE_J0)

  # Check if `j1' exists
  CHECK_FUNCTION_EXISTS(j1 HAVE_J1)

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
  if(HAVE_LOCALE_T ) # it was TRUE and we need it to be 1
    set(HAVE_LOCALE_T 1)
  endif()

  # Check if `localtime_r' exists
  CHECK_FUNCTION_EXISTS(localtime_r HAVE_LOCALTIME_R)

  # Check if `long double' exists
  CHECK_TYPE_SIZE("long double" LONG_DOUBLE)
  if(HAVE_LONG_DOUBLE) # it was TRUE and we need it to be 1
    set(HAVE_LONG_DOUBLE 1)
  endif()

  # Check if `long double' have more precision than `double'
  if(HAVE_LONG_DOUBLE)
    CHECK_TYPE_SIZE(double DOUBLE)
    if(${LONG_DOUBLE} GREATER ${DOUBLE})
      set(HAVE_LONG_DOUBLE_WIDER 1)
    endif()
  endif()

  # Check if `long long int' exists
  CHECK_TYPE_SIZE("long long int" LONG_LONG_INT)
  if(HAVE_LONG_LONG_INT) # it was TRUE and we need it to be 1
    set(HAVE_LONG_LONG_INT 1)
  endif()

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
  find_package(X11)
  if(X11_Xshape_FOUND)
    set(HAVE_SHAPE 1)
  endif()
  if(X11_XShm_FOUND)
    set(HAVE_SHARED_MEMORY 1)
  endif()

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
  set(CMAKE_REQUIRED_DEFINITIONS_SAVE ${CMAKE_REQUIRED_DEFINITIONS})
  if(HAVE_NAMESPACES)
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -DHAVE_NAMESPACES)
  endif()
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
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS_SAVE})

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

  # Check if `times' exists
  CHECK_FUNCTION_EXISTS(times HAVE_TIMES)

  # Check if `tm_zone' is a member of `struct tm' (deprecated, use `HAVE_STRUCT_TM_TM_ZONE' instead)
  CHECK_STRUCT_HAS_MEMBER("struct tm" tm_zone time.h HAVE_TM_ZONE)

  # Check if `tzname' is a member of `struct tm' only if `tm_zone' isn't defined
  if(NOT HAVE_STRUCT_TM_TM_ZONE)
    CHECK_STRUCT_HAS_MEMBER("struct tm" tzname time.h HAVE_TZNAME)
  endif()

  # Check if `uintmax_t' exists
  CHECK_TYPE_SIZE(uintmax_t UINTMAX_T)
  if(HAVE_UINTMAX_T) # it was TRUE and we need it to be 1
    set(HAVE_UINTMAX_T 1)
  endif()

  # Check if `uintptr_t' exists
  CHECK_TYPE_SIZE(uintptr_t UINTPTR_T)
  if(HAVE_UINTPTR_T) # it was TRUE and we need it to be 1
    set(HAVE_UINTPTR_T 1)
  endif()

  # Check if `ulltostr' exists
  CHECK_FUNCTION_EXISTS(ulltostr HAVE_ULLTOSTR)

  # Check if <unistd.h> exists
  CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)

  # Check if `unsigned long long int' exists
  CHECK_TYPE_SIZE("unsigned long long int" UNSIGNED_LONG_LONG_INT)
  if(HAVE_UNSIGNED_LONG_LONG_INT) # it was TRUE and we need it to be 1
    set(HAVE_UNSIGNED_LONG_LONG_INT 1)
  endif()

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
  if(HAVE_FORK)
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
  endif()

  # Check if `vfork' works
  if(HAVE_VFORK)
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
  endif()

  # Check if <xlocale.h> exists
  CHECK_INCLUDE_FILE(xlocale.h HAVE_XLOCALE_H)

  # Check if `_aligned_malloc' exists
  CHECK_FUNCTION_EXISTS(_aligned_malloc HAVE__ALIGNED_MALLOC)

  # Check if `_Bool' exists
  CHECK_TYPE_SIZE(_Bool _BOOL)
  if(HAVE__BOOL) # it was TRUE and we need it to be 1
    set(HAVE__BOOL 1)
  endif()

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
  if(SIGNAL_RETURN_TYPE_IS_VOID)
    set(RETSIGTYPE void)
  else(SIGNAL_RETURN_TYPE_IS_VOID)
    set(RETSIGTYPE int)
  endif(SIGNAL_RETURN_TYPE_IS_VOID)

  #TODO These seems to be obsolet but should we check them ????
  set(SELECT_TYPE_ARG1 "")
  set(SELECT_TYPE_ARG234 "")
  set(SELECT_TYPE_ARG5 "")

  # Check `double' size
  CHECK_TYPE_SIZE(double SIZEOF_DOUBLE)

  # Check `double_t' size
  if(HAVE_DOUBLE_T)
    set(CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES})
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} "math.h")
    CHECK_TYPE_SIZE(double_t SIZEOF_DOUBLE_T)
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE})
  endif()

  # Check `float' size
  CHECK_TYPE_SIZE(float SIZEOF_FLOAT)

  # Check `float_t' size
  if(HAVE_FLOAT_T)
    set(CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES})
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} "math.h")
    CHECK_TYPE_SIZE(float_t SIZEOF_FLOAT_T)
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE})
  endif()

  # Check `long double' size
  if(HAVE_LONG_DOUBLE)
    CHECK_TYPE_SIZE("long double" SIZEOF_LONG_DOUBLE)
  endif()

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
  set(STAT_MACROS_BROKEN 0)

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

  # Check if we can safely include both <sys/time.h> and <time.h>
  CHECK_CXX_SOURCE_COMPILES(
  "
    #include <sys/time.h>
    #include <time.h>
    void main(void){}
  "
  TIME_WITH_SYS_TIME)

  # Check if `struct tm' exists in <sys/time.h>
  if(HAVE_SYS_TIME_H)
    CHECK_SYMBOL_EXISTS("struct tm" sys/time.h TM_IN_SYS_TIME)
  endif()

  # Check if _GNU_SOURCE is available
  CHECK_SYMBOL_EXISTS(__GNU_LIBRARY__ features.h _GNU_SOURCE)

  # Check if system is Big Endian
  TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

  # Check if we are on MINIX
  CHECK_SYMBOL_EXISTS(_MINIX "stdio.h" EVENT___MINIX)

  # Check if system does not provide POSIX.1 features except with this defined
  #TODO does this suffice ????
  CHECK_SYMBOL_EXISTS(_POSIX_1_SOURCE "stdio.h" EVENT___POSIX_1_SOURCE)
  if(NOT _POSIX_1_SOURCE)
    set(_POSIX_1_SOURCE 2)
  endif()

  # TODO Is this true or should it be 1 when not found???
  CHECK_SYMBOL_EXISTS(_POSIX_SOURCE "stdio.h" EVENT___POSIX_SOURCE)

  if(NOT CMAKE_COMPILER_IS_GNUCC)
    CHECK_RUN_RESULT(
    "
      #include <limits.h>
      int main (void) { return CHAR_MIN == 0; }
    "
    1
    __CHAR_UNSIGNED__)
  endif()

  # Check for compiler `__func__' compatibility
  CHECK_C_SOURCE_COMPILES("void main() {char *function_name = __func__;}" HAVE___FUNC__)
  CHECK_C_SOURCE_COMPILES("void main() {char *function_name = __FUNCTION__;}" HAVE___FUNCTION__)

  if(HAVE___FUNC__)
    set(__func__ __func__)
  elseif(HAVE___FUNCTION__) 
    set(__func__ __FUNCTION__)
  else()
    set(__func__ "")
  endif()

  # Check if `const' is supported by compiler 
  CHECK_C_SOURCE_COMPILES("void main() {const char *s = \"Test\";}" HAVE_CONST)
  # Only set const to empty if it doesn't exist otherwise magick++ will not compile
  if(NOT HAVE_CONST)
    set(const " ")
  endif()

  # Check if <sys/types.h> doesn't define `gid_t'
  if(HAVE_SYS_TYPES_H)
    CHECK_SYMBOL_EXISTS(gid_t sys/types.h HAVE_GID_T)
    if(NOT HAVE_GID_T)
      SET(gid_t int)
    endif()
  endif()

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

  if(HAVE_INLINE)
    set(inline inline)
  elseif(HAVE___INLINE)
    set(inline __inline)
  elseif(HAVE___INLINE__)
    set(inline __inline__)
  else()
    set(inline "")
  endif()

  #TODO these defines if system doesn't define them
  set(int16_t "")
  set(int32_t "")
  set(int64_t "")
  set(int8_t "")
  set(intmax_t "")
  set(intptr_t "")
  set(mbstate_t "")

  # Check if <sys/types.h> doesn't define `mode_t'
  if(HAVE_SYS_TYPES_H)
    CHECK_SYMBOL_EXISTS(mode_t sys/types.h HAVE_MODE_T)
    if(NOT HAVE_MODE_T)
      set(mode_t int)
    endif()
  endif()

  # Check if <sys/types.h> doesn't define `pid_t'
  if(HAVE_SYS_TYPES_H)
    CHECK_SYMBOL_EXISTS(pid_t sys/types.h HAVE_PID_T)
    if(NOT HAVE_PID_T)
      set(pid_t int)
    endif()
  endif()

  # Check for the compiler restrict compatible instruction
  CHECK_C_SOURCE_COMPILES(
    "int test (void *restrict x);\nint main (void) {return 0;}"
    HAVE_RESTRICT)

  CHECK_C_SOURCE_COMPILES(
  "typedef struct abc *d;\nint test (d __restrict x);\nint main (void) {return 0;}"
    HAVE___RESTRICT)

  if(HAVE___RESTRICT)
    set(restrict __restrict)
  elseif(NOT HAVE_RESTRICT)
    set(restrict " ")
  endif()

  # Check if <sys/types.h> doesn't define `size_t'
  if(HAVE_SYS_TYPES_H)
    if(SIZEOF_SIZE_T)
      set(HAVE_SIZE_T 1)
    else()
      CHECK_SYMBOL_EXISTS(size_t sys/types.h HAVE_SIZE_T)
      if(NOT HAVE_SIZE_T)
        if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
          set(size_t "unsigned int")
        else()
          set(size_t "unsigned long long")
        endif()
      set(SIZEOF_SIZE_T ${CMAKE_SIZEOF_VOID_P})
      endif()
    endif()
  endif()

  # Check if <sys/types.h> doesn't define `ssize_t'
  if(HAVE_SYS_TYPES_H)
    if(SIZEOF_SSIZE_T)
      set(HAVE_SSIZE_T 1)
    else()
      CHECK_SYMBOL_EXISTS(ssize_t sys/types.h HAVE_SSIZE_T)
      if(NOT HAVE_SSIZE_T)
        if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
          set(ssize_t int)
        else()
          set(ssize_t "long long")
        endif()
        set(SIZEOF_SSIZE_T ${CMAKE_SIZEOF_VOID_P})
      endif()
    endif()
  endif()

  # Check if <sys/types.h> doesn't define `uid_t'
  if(HAVE_SYS_TYPES_H)
    CHECK_SYMBOL_EXISTS(uid_t sys/types.h HAVE_UID_T)
    if(NOT HAVE_UID_T)
      set(uid_t int)
    endif()
  endif()

  #TODO these defines if system doesn't define them
  set(uint16_t "")
  set(uint32_t "")
  set(uint64_t "")
  set(uint8_t "")
  set(uintmax_t "")
  set(uintptr_t "")

  # Check if `vfork' is not working and define it as `fork'
  if(NOT HAVE_WORKING_VFORK)
    set(vfork fork)
  endif()

  # Check if `volatile' works
  CHECK_CXX_SOURCE_COMPILES(
  "
  void main() { volatile int i = 1; }
  "
  HAVE_VOLATILE)

  if(HAVE_VOLATILE)
    set(volatile volatile)
  else()
    set(volatile "")
  endif()
endmacro()