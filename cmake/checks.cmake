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
include(CheckSourceRuns)
include(TestBigEndian)

macro(magick_check_env)
  # Check if `closedir' function returns void instead of `int'
  check_include_file(dirent.h HAVE_DIRENT_H)
  if(HAVE_DIRENT_H)
      unset(HAVE_DIRENT_H)
      check_symbol_exists(DIR dirent.h HAVE_DIRENT_H)
      if(NOT HAVE_DIRENT_H)
          message(STATUS "--- DIR not defined...")
      else(NOT HAVE_DIRENT_H)
          check_prototype_definition(closedir "void closedir(DIR *dirp)" "NULL" "dirent.h" CLOSEDIR_VOID)
      endif(NOT HAVE_DIRENT_H)
  endif(HAVE_DIRENT_H)

  # Check if `acosh' exists
  check_function_exists(acosh HAVE_ACOSH)

  # Check if <arm/limits.h> exists
  check_include_file(arm/limits.h HAVE_ARM_LIMITS_H)

  # Check if <arpa/inet.h> exists
  check_include_file(arpa/inet.h HAVE_ARM_LIMITS_H)

  # Check if `asinh' exists
  check_function_exists(asinh HAVE_ASINH)

  # Check if `atanh' exists
  check_function_exists(atanh HAVE_ATANH)

  # Check if `atexit' exists
  check_function_exists(atexit HAVE_ATEXIT)

  # Check if `atoll' exists
  check_function_exists(atoll HAVE_ATOLL)

  # Check if `bool' exists (check_type_size is not working at least on windows)
  check_cxx_source_compiles ("void main () {bool b = false;}" HAVE_BOOL)

  # Check if `clock' exists
  check_function_exists(clock HAVE_CLOCK)

  # Check if `clock_getres' exists
  check_function_exists(clock_getres HAVE_CLOCK_GETRES)

  # Check if `clock_gettime' exists
  check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)

  # Check if `CLOCK_REALTIME' exists
  check_symbol_exists(CLOCK_REALTIME time.h HAVE_CLOCK_REALTIME)

  # Check if <complex.h> exists
  check_include_file(complex.h HAVE_COMPLEX_H)

  if(WIN32)
    # FFTW is not compatible with complex.h in MSVC
    set(HAVE_COMPLEX_H FALSE)
  endif()
  if(HAVE_COMPLEX_H)
    check_c_source_compiles("#include <complex.h>\nint main(void) { cabs(1.0); return 0; }" HAVE_CABS)
    check_c_source_compiles("#include <complex.h>\nint main(void) { carg(1.0); return 0; }" HAVE_CARG)
    check_c_source_compiles("#include <complex.h>\nint main(void) { cimag(1.0); return 0; }" HAVE_CIMAG)
    check_c_source_compiles("#include <complex.h>\nint main(void) { creal(1.0); return 0; }" HAVE_CREAL)
  endif()

  # Check if `ctime_r' exists
  check_function_exists(ctime_r HAVE_CTIME_R)

  # Check if `pread' exists
  check_function_exists(pread HAVE_DECL_PREAD)

  # Check if `pwrite' exists
  check_function_exists(pwrite HAVE_DECL_PWRITE)

  # Check if `strerror_r' exists
  check_symbol_exists(strerror_r string.h HAVE_DECL_STRERROR_R)

  # Check if `strlcpy' exists
  check_function_exists(strlcpy HAVE_DECL_STRLCPY)

  # Check if `tzname' exists
  check_symbol_exists(tzname time.h HAVE_DECL_TZNAME)

  # Check if `directio' exists
  check_function_exists(directio HAVE_DIRECTIO)

  # Check if <dlfcn.h> exists
  check_include_file(dlfcn.h HAVE_DLFCN_H)

  # Check if `double_t' exists
  check_cxx_source_compiles (
  "
    #include <math.h>
    void main () {double_t d = 0;}
  " 
  HAVE_DOUBLE_T)

  # Check if `erf' exists
  # Only check if not windows since <nt-base.h> explicit defines the macro and it will cause a lot of warnings
  if(NOT WINDOWS)
    check_function_exists(erf HAVE_ERF)
  endif()

  # Check if <errno.h> exists
  check_include_file(errno.h HAVE_ERRNO_H)

  # Check if `execvp' exists
  check_function_exists(execvp HAVE_EXECVP)

  # Check if `fchmod' exists
  check_function_exists(fchmod HAVE_FCHMOD)

  # Check if <fcntl.h> exists
  check_include_file(fcntl.h HAVE_FCNTL_H)

  # Check if `float_t' exists
  check_cxx_source_compiles (
  "
    #include <math.h> 
    void main () {float_t f = 0;}
  " 
  HAVE_FLOAT_T)

  # Check if `floor' exists
  check_function_exists(floor HAVE_FLOOR)

  # Check if `fork' exists
  check_function_exists(fork HAVE_FORK)

  # Check if `fseeko' exists
  check_function_exists(fseeko HAVE_FSEEKO)

  # Check if `ftime' exists
  check_function_exists(ftime HAVE_FTIME)

  # Check if `ftruncate' exists
  check_function_exists(ftruncate HAVE_FTRUNCATE)

  # Check if `getcwd' exists
  check_function_exists(getcwd HAVE_GETCWD)

  # Check if `getc_unlocked' exists
  check_function_exists(getc_unlocked HAVE_GETC_UNLOCKED)

  # Check if `getdtablesize' exists
  check_function_exists(getdtablesize HAVE_GETDTABLESIZE)

  # Check if `getexecname' exists
  check_function_exists(getexecname HAVE_GETEXECNAME)

  # Check if `getpagesize' exists
  check_function_exists(getpagesize HAVE_GETPAGESIZE)

  # Check if `getpid' exists
  check_function_exists(getpid HAVE_GETPID)

  # Check if `getrlimit' exists
  check_function_exists(getrlimit HAVE_GETRLIMIT)

  # Check if `getrusage' exists
  check_function_exists(getrusage HAVE_GETRUSAGE)

  # Check if `gettimeofday' exists
  check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)

  # Check if `gmtime_r' exists
  check_function_exists(gettimeofday HAVE_GMTIME_R)

  # I don't think our program will have a big memory impact
  set(HAVE_HUGEPAGES FALSE)

  # Check if `intmax_t' exists
  check_type_size (intmax_t INTMAX_T)
  if(HAVE_INTMAX_T) # it was TRUE and we need it to be 1
    set(HAVE_INTMAX_T 1)
  endif()

  # Check if `intptr_t' exists
  check_type_size (intptr_t INTPTR_T)
  if(HAVE_INTPTR_T ) # it was TRUE and we need it to be 1
    set(HAVE_INTPTR_T 1)
  endif()

  # Check if <inttypes.h> exists
  check_include_file(inttypes.h HAVE_INTTYPES_H)

  # Check if `isnan' exists
  check_function_exists(isnan HAVE_ISNAN)

  # Check if `j0' exists
  check_function_exists(j0 HAVE_J0)

  # Check if `j1' exists
  check_function_exists(j1 HAVE_J1)

  # Check if `gcov' exists
  check_library_exists(gcov "" "" HAVE_LIBGCOV)

  # Check if <limits.h> exists
  check_include_file(limits.h HAVE_LIMITS_H)

  # Check if <linux/unistd.h> exists
  check_include_file(linux/unistd.h HAVE_LINUX_UNISTD_H)

  # Check if `lltostr' exists
  check_function_exists(lltostr HAVE_LLTOSTR)

  # Check if <locale.h> exists
  check_include_file(locale.h HAVE_LOCALE_H)

  # Check if `locale_t' exists
  check_type_size(locale_t LOCALE_T)
  if(HAVE_LOCALE_T ) # it was TRUE and we need it to be 1
    set(HAVE_LOCALE_T 1)
  endif()

  # Check if `localtime_r' exists
  check_function_exists(localtime_r HAVE_LOCALTIME_R)

  # Check if `long double' exists
  check_type_size("long double" LONG_DOUBLE)
  if(HAVE_LONG_DOUBLE) # it was TRUE and we need it to be 1
    set(HAVE_LONG_DOUBLE 1)
  endif()

  # Check if `long double' have more precision than `double'
  if(HAVE_LONG_DOUBLE)
    check_type_size(double DOUBLE)
    if(${LONG_DOUBLE} GREATER ${DOUBLE})
      set(HAVE_LONG_DOUBLE_WIDER 1)
    endif()
  endif()

  # Check if `long long int' exists
  check_type_size("long long int" LONG_LONG_INT)
  if(HAVE_LONG_LONG_INT) # it was TRUE and we need it to be 1
    set(HAVE_LONG_LONG_INT 1)
  endif()

  # Check if <machine/param.h> exists
  check_include_file(machine/param.h HAVE_MACHINE_PARAM_H)

  # Check if <mach-o/dyld.h.h> exists
  check_include_file(mach-o/dyld.h HAVE_MACH_O_DYLD_H)

  # Check if `mbstate_t' exists in <wchar.h>
  check_symbol_exists(mbstate_t wchar.h HAVE_MBSTATE_T)

  # Check if `memmove' exists
  check_function_exists(memmove HAVE_MEMMOVE)

  # Check if <memory.h> exists
  check_include_file(memory.h HAVE_MEMORY_H)

  # Check if `memset' exists
  check_function_exists(memset HAVE_MEMSET)

  # Check if `mkstemp' exists
  check_function_exists(mkstemp HAVE_MKSTEMP)

  # Check if `mmap' exists
  check_function_exists(mmap HAVE_MMAP)

  # Check if `munmap' exists
  check_function_exists(munmap HAVE_MUNMAP)

  # Check if `namespace' exists
  check_cxx_source_compiles ("namespace test {} void main() {using namespace ::test;}" HAVE_NAMESPACES)

  # Check if `std::' exists
  check_cxx_source_compiles (
  "
    #include <iostream> 
    void main() {std::istream& is = std::cin;}
  "
  HAVE_NAMESPACE_STD)

  # Check if `nanosleep' exists
  check_function_exists(nanosleep HAVE_NANOSLEEP)

  # Check if <ndir.h> exists
  check_include_file(ndir.h HAVE_NDIR_H)

  # Check if <netinet/in.h> exists
  check_include_file(netinet/in.h HAVE_NETINET_IN_H)

  # Check if `newlocale' exists
  check_function_exists(newlocale HAVE_NEWLOCALE)

  # Check if <OS.h> exists
  check_include_file(OS.h HAVE_OS_H)

  # Check if `pclose' exists
  check_function_exists(pclose HAVE_PCLOSE)

  # Check if `poll' exists
  check_function_exists(poll HAVE_POLL)

  # Check if `popen' exists
  check_function_exists(popen HAVE_POPEN)

  # Check if `posix_fadvise' exists
  check_function_exists(posix_fadvise HAVE_POSIX_FADVISE)

  # Check if `posix_fallocate' exists
  check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)

  # Check if `posix_madvise' exists
  check_function_exists(posix_madvise HAVE_POSIX_MADVISE)

  # Check if `posix_memalign' exists
  check_function_exists(posix_memalign HAVE_POSIX_MEMALIGN)

  # Check if `posix_spawnp' exists
  check_function_exists(posix_spawnp HAVE_POSIX_SPAWNP)

  # Check if `pow' exists
  check_function_exists(pow HAVE_POW)

  # Check if `pread' exists
  check_function_exists(pread HAVE_PREAD)

  # Check if <process.h> exists
  check_include_file(process.h HAVE_PROCESS_H)

  # Check if `pwrite' exists
  check_function_exists(pwrite HAVE_PWRITE)

  # Check if `qsort_r' exists
  check_function_exists(qsort_r HAVE_QSORT_R)

  # Check if `raise' exists
  check_function_exists(raise HAVE_RAISE)

  # Check if `rand_r' exists
  check_function_exists(rand_r HAVE_RAND_R)

  # Check if `readlink' exists
  check_function_exists(readlink HAVE_READLINK)

  # Check if `realpath' exists
  check_function_exists(realpath HAVE_REALPATH)

  # Check if `seekdir' exists
  check_function_exists(seekdir HAVE_SEEKDIR)

  # Check if `select' exists
  check_function_exists(select HAVE_SELECT)

  # Check if `sendfile' exists
  check_function_exists(sendfile HAVE_SENDFILE)

  # Check if `setlocale' exists
  check_function_exists(setlocale HAVE_SETLOCALE)

  # Check if `setvbuf' exists
  check_function_exists(setvbuf HAVE_SETVBUF)

  # Check supported X11 extensions  
  find_package(X11)
  if(X11_Xshape_FOUND)
    set(HAVE_SHAPE 1)
  endif()
  if(X11_XShm_FOUND)
    set(HAVE_SHARED_MEMORY 1)
  endif()

  # Check if `sigaction' exists
  check_function_exists(sigaction HAVE_SIGACTION)

  # Check if `sigemptyset' exists
  check_function_exists(sigemptyset HAVE_SIGEMPTYSET)

  # Check if `socket' exists
  check_function_exists(socket HAVE_SOCKET)

  # Check if `spawnvp' exists
  check_function_exists(spawnvp HAVE_SPAWNVP)

  # Check if `sqrt' exists
  check_function_exists(sqrt HAVE_SQRT)

  # Check if `stat' exists
  check_function_exists(stat HAVE_STAT)

  # Check if `stdarg' exists
  check_function_exists(stdarg HAVE_STDARG_H)

  # Check if <stdbool.h> exists and conforms to C99
  check_cxx_source_compiles (
  "
    #include <stdbool.h> 
    void main() {bool b = __bool_true_false_are_defined;}
  "
  HAVE_STDBOOL_H)

  # Check if <stdint.h> exists
  check_include_file(stdint.h HAVE_STDINT_H)

  # Check if <stdlib.h> exists
  check_include_file(stdlib.h HAVE_STDLIB_H)

  # Check if compiler supports ISO C++ standard library
  set(CMAKE_REQUIRED_DEFINITIONS_SAVE ${CMAKE_REQUIRED_DEFINITIONS})
  if(HAVE_NAMESPACES)
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -DHAVE_NAMESPACES)
  endif()
  check_cxx_source_compiles (
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
  check_function_exists(strcasecmp HAVE_STRCASECMP)

  # Check if `strchr' exists
  check_function_exists(strchr HAVE_STRCHR)

  # Check if `strcspn' exists
  check_function_exists(strcspn HAVE_STRCSPN)

  # Check if `strdup' exists
  check_function_exists(strdup HAVE_STRDUP)

  # Check if `strerror' exists
  check_function_exists(strerror HAVE_STRERROR)

  # Check if `strerror_r' exists
  check_function_exists(strerror_r HAVE_STRERROR_R)

  # Check if `#' stringizing operator is supported
  set(HAVE_STRINGIZE_EXITCODE 1)
  set(HAVE_STRINGIZE_EXITCODE__TRYRUN_OUTPUT 1)
  check_cxx_source_runs(
  "
    #define x(y) #y
    int main() { char c[] = \"c\"; char* p = x(c); return (c[0] != p[0]) || (c[1] != p[1]); }
  "
  HAVE_STRINGIZE)

  # Check if <strings.h> exists
  check_include_file(strings.h HAVE_STRINGS_H)

  # Check if <string.h> exists
  check_include_file(string.h HAVE_STRING_H)

  # Check if <strlcat.h> exists
  check_include_file(strlcat.h HAVE_STRLCAT)

  # Check if <strlcpy.h> exists
  check_include_file(strlcpy.h HAVE_STRLCPY)

  # Check if `strncasecmp' exists
  check_function_exists(strncasecmp HAVE_STRNCASECMP)

  # Check if `strpbrk' exists
  check_function_exists(strpbrk HAVE_STRPBRK)

  # Check if `strrchr' exists
  check_function_exists(strrchr HAVE_STRRCHR)

  # Check if `strspn' exists
  check_function_exists(strspn HAVE_STRSPN)

  # Check if `strstr' exists
  check_function_exists(strstr HAVE_STRSTR)

  # Check if `strtod' exists
  check_function_exists(strtod HAVE_STRSTR)

  # Check if `strtod_l' exists
  check_function_exists(strtod_l HAVE_STRTOD_L)

  # Check if `strtol' exists
  check_function_exists(strtol HAVE_STRTOL)

  # Check if `strtoul' exists
  check_function_exists(strtoul HAVE_STRTOUL)

  # Check if `tm_zone' is a member of `struct tm'
  check_struct_has_member("struct tm" tm_zone time.h HAVE_STRUCT_TM_TM_ZONE)

  # Check if <sun_prefetch.h> exists
  check_include_file(sun_prefetch.h HAVE_SUN_PREFETCH_H)

  # Check if `symlink' exists
  check_function_exists(symlink HAVE_SYMLINK)

  # Check if `sysconf' exists
  check_function_exists(sysconf HAVE_SYSCONF)

  # Check if <sys/dir.h> exists and defines `DIR'
  check_symbol_exists(DIR sys/dir.h HAVE_SYS_DIR_H)

  # Check if <sys/ipc.h> exists
  check_include_file(sys/ipc.h HAVE_SYS_IPC_H)

  # Check if <sys/mman.h> exists
  check_include_file(sys/mman.h HAVE_SYS_MMAN_H)

  # Check if <sys/ndir.h> exists and defines `DIR'
  check_symbol_exists(DIR sys/ndir.h HAVE_SYS_NDIR_H)

  # Check if <sys/param.h> exists
  check_include_file(sys/param.h HAVE_SYS_PARAM_H)

  # Check if <sys/resource.h> exists
  check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)

  # Check if <sys/select.h> exists
  check_include_file(sys/select.h HAVE_SYS_SELECT_H)

  # Check if <sys/sendfile.h> exists
  check_include_file(sys/sendfile.h HAVE_SYS_SENDFILE_H)

  # Check if <sys/socket.h> exists
  check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)

  # Check if <sys/stat.h> exists
  check_include_file(sys/stat.h HAVE_SYS_STAT_H)

  # Check if <sys/syslimits.h> exists
  check_include_file(sys/syslimits.h HAVE_SYS_SYSLIMITS_H)

  # Check if <sys/timeb.h> exists
  check_include_file(sys/timeb.h HAVE_SYS_TIMEB_H)

  # Check if <sys/times.h> exists
  check_include_file(sys/times.h HAVE_SYS_TIMES_H)

  # Check if <sys/time.h> exists
  check_include_file(sys/time.h HAVE_SYS_TIME_H)

  # Check if <sys/types.h> exists
  check_include_file(sys/types.h HAVE_SYS_TYPES_H)

  # Check if <sys/wait.h> exists
  check_include_file(sys/wait.h HAVE_SYS_WAIT_H)

  # Check if `telldir' exists
  check_function_exists(telldir HAVE_TELLDIR)

  # Check if `tempnam' exists
  check_function_exists(tempnam HAVE_TEMPNAM)

  # Check if `times' exists
  check_function_exists(times HAVE_TIMES)

  # Check if `tm_zone' is a member of `struct tm' (deprecated, use `HAVE_STRUCT_TM_TM_ZONE' instead)
  check_struct_has_member("struct tm" tm_zone time.h HAVE_TM_ZONE)

  # Check if `tzname' is a member of `struct tm' only if `tm_zone' isn't defined
  if(NOT HAVE_STRUCT_TM_TM_ZONE)
    check_struct_has_member("struct tm" tzname time.h HAVE_TZNAME)
  endif()

  # Check if `uintmax_t' exists
  check_type_size(uintmax_t UINTMAX_T)
  if(HAVE_UINTMAX_T) # it was TRUE and we need it to be 1
    set(HAVE_UINTMAX_T 1)
  endif()

  # Check if `uintptr_t' exists
  check_type_size(uintptr_t UINTPTR_T)
  if(HAVE_UINTPTR_T) # it was TRUE and we need it to be 1
    set(HAVE_UINTPTR_T 1)
  endif()

  # Check if `ulltostr' exists
  check_function_exists(ulltostr HAVE_ULLTOSTR)

  # Check if <unistd.h> exists
  check_include_file(unistd.h HAVE_UNISTD_H)

  # Check if `unsigned long long int' exists
  check_type_size("unsigned long long int" UNSIGNED_LONG_LONG_INT)
  if(HAVE_UNSIGNED_LONG_LONG_INT) # it was TRUE and we need it to be 1
    set(HAVE_UNSIGNED_LONG_LONG_INT 1)
  endif()

  # Check if `uselocale' exists
  check_function_exists(uselocale HAVE_USELOCALE)

  # Check if `usleep' exists
  check_function_exists(usleep HAVE_USLEEP)

  # Check if `utime' exists
  check_function_exists(utime HAVE_UTIME)

  # Check if <utime.h> exists
  check_include_file(utime.h HAVE_UTIME_H)

  # Check if `vfork' exists
  check_function_exists(vfork HAVE_VFORK)

  # Check if <vfork.h> exists
  check_include_file(vfork.h HAVE_VFORK_H)

  # Check if `vfprintf' exists
  check_symbol_exists(vfprintf stdio.h HAVE_VFPRINTF)

  # Check if `vfprintf_l' exists
  check_symbol_exists(vfprintf_l stdio.h HAVE_VFPRINTF_L)

  # Check if `vprintf' exists
  check_symbol_exists(vprintf stdio.h HAVE_VPRINTF)
  # Check if `_doprnt' exists only if `vprintf' do not exists
  if(NOT HAVE_VPRINTF)
    check_function_exists(_doprnt HAVE_DOPRNT)
  endif()

  # Check if `vsnprintf' exists
  check_symbol_exists(vsnprintf stdio.h HAVE_VSNPRINTF)
  check_function_exists(vsnprintf HAVE_DECL_VSNPRINTF)

  # Check if `vsnprintf_l' exists
  check_symbol_exists(vsnprintf_l stdio.h HAVE_VSNPRINTF_L)

  # Check if `vsprintf' exists
  check_symbol_exists(vsprintf stdio.h HAVE_VSPRINTF)

  # Check if `waitpid' exists
  check_function_exists(waitpid HAVE_WAITPID)

  # Check if <wchar.h> exists
  check_include_file(wchar.h HAVE_WCHAR_H)

  # Check if <windows.h> exists
  check_include_file(windows.h HAVE_WINDOWS_H)

  # Check if `fork' works
  if(HAVE_FORK)
    set(HAVE_WORKING_FORK_EXITCODE 1)
    set(HAVE_WORKING_FORK_EXITCODE__TRYRUN_OUTPUT 1)
    check_cxx_source_runs(
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
    set(HAVE_WORKING_VFORK_EXITCODE 1)
    set(HAVE_WORKING_VFORK_EXITCODE__TRYRUN_OUTPUT 1)
    check_cxx_source_runs(
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
  check_include_file(xlocale.h HAVE_XLOCALE_H)

  # Check if `_aligned_malloc' exists
  check_function_exists(_aligned_malloc HAVE__ALIGNED_MALLOC)

  # Check if `_Bool' exists
  check_type_size(_Bool _BOOL)
  if(HAVE__BOOL) # it was TRUE and we need it to be 1
    set(HAVE__BOOL 1)
  endif()

  # Check if `_exit' exists
  check_function_exists(_exit HAVE__EXIT)

  # Check if `_NSGetExecutablePath' exists
  check_function_exists(_NSGetExecutablePath HAVE__NSGETEXECUTABLEPATH)

  # Check if `_pclose' exists
  check_function_exists(_pclose HAVE__PCLOSE)

  # Check if `_popen' exists
  check_function_exists(_popen HAVE__POPEN)

  # Check if `_wfopen' exists
  check_function_exists(_wfopen HAVE__WFOPEN)

  # Check if `_wstat' exists
  check_function_exists(_wstat HAVE__WSTAT)

  # Check if `__attribute__' exists
  check_c_source_compiles(
  "
    #include <stdlib.h>
    static void foo(void) __attribute__ ((unused));
    void main() { }
  "
  HAVE___ATTRIBUTE__)

  # Check return type of signal handlers
  check_c_source_compiles(
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
  check_type_size(double SIZEOF_DOUBLE)

  # Check `double_t' size
  if(HAVE_DOUBLE_T)
    set(CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES})
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} "math.h")
    check_type_size(double_t SIZEOF_DOUBLE_T)
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE})
  endif()

  # Check `float' size
  check_type_size(float SIZEOF_FLOAT)

  # Check `float_t' size
  if(HAVE_FLOAT_T)
    set(CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES})
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} "math.h")
    check_type_size(float_t SIZEOF_FLOAT_T)
    set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE})
  endif()

  # Check `long double' size
  if(HAVE_LONG_DOUBLE)
    check_type_size("long double" SIZEOF_LONG_DOUBLE)
  endif()

  # Check `off_t' size
  check_type_size(off_t SIZEOF_OFF_T)

  # Check `signed int' size
  check_type_size("signed int" SIZEOF_SIGNED_INT)

  # Check `signed long' size
  check_type_size("signed long" SIZEOF_SIGNED_LONG)

  # Check `signed long long' size
  check_type_size("signed long long" SIZEOF_SIGNED_LONG_LONG)

  # Check `signed short' size
  check_type_size("signed short" SIZEOF_SIGNED_SHORT)

  # Check `size_t' size
  check_type_size("size_t" SIZEOF_SIZE_T)

  # Check `ssize_t' size
  check_type_size("ssize_t" SIZEOF_SSIZE_T)

  # Check `unsigned int' size
  check_type_size("unsigned int" SIZEOF_UNSIGNED_INT)

  # Check `unsigned int*' size
  check_type_size("unsigned int*" SIZEOF_UNSIGNED_INTP)

  # Check `unsigned long' size
  check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

  # Check `unsigned long long' size
  check_type_size("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)

  # Check `unsigned long long' size
  check_type_size("void*" SIZEOF_VOID_P)

  # Check `unsigned short' size
  check_type_size("unsigned short" SIZEOF_UNSIGNED_SHORT)

  # TODO Not sure how to heck if the `S_IS*' macros in <sys/stat.h> are broken
  # Should we test them all ???? 
  set(STAT_MACROS_BROKEN 0)

  # Check ANSI C header files exists
  check_include_fileS("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

  # Check strerror_r returns `char *'
  check_cxx_source_compiles(
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
  check_cxx_source_compiles(
  "
    #include <sys/time.h>
    #include <time.h>
    void main(void){}
  "
  TIME_WITH_SYS_TIME)

  # Check if `struct tm' exists in <sys/time.h>
  if(HAVE_SYS_TIME_H)
    check_symbol_exists("struct tm" sys/time.h TM_IN_SYS_TIME)
  endif()

  # Check if _GNU_SOURCE is available
  check_symbol_exists(__GNU_LIBRARY__ features.h _GNU_SOURCE)

  # Check if system is Big Endian
  TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

  # Check if we are on MINIX
  check_symbol_exists(_MINIX "stdio.h" EVENT___MINIX)

  # Check if system does not provide POSIX.1 features except with this defined
  #TODO does this suffice ????
  check_symbol_exists(_POSIX_1_SOURCE "stdio.h" EVENT___POSIX_1_SOURCE)
  if(NOT _POSIX_1_SOURCE)
    set(_POSIX_1_SOURCE 2)
  endif()

  # TODO Is this true or should it be 1 when not found???
  check_symbol_exists(_POSIX_SOURCE "stdio.h" EVENT___POSIX_SOURCE)

  if(NOT CMAKE_COMPILER_IS_GNUCC)
    set(__CHAR_UNSIGNED___EXITCODE 1)
    set(__CHAR_UNSIGNED___EXITCODE__TRYRUN_OUTPUT 1)
    check_source_runs(C
    "
      #include <limits.h>
      int main (void) { return CHAR_MIN == 0; }
    "
    __CHAR_UNSIGNED__)
  endif()

  # Check for compiler `__func__' compatibility
  check_c_source_compiles("void main() {char *function_name = __func__;}" HAVE___FUNC__)
  check_c_source_compiles("void main() {char *function_name = __FUNCTION__;}" HAVE___FUNCTION__)

  if(HAVE___FUNC__)
    set(__func__ __func__)
  elseif(HAVE___FUNCTION__) 
    set(__func__ __FUNCTION__)
  else()
    set(__func__ "")
  endif()

  # Check if `const' is supported by compiler 
  check_c_source_compiles("void main() {const char *s = \"Test\";}" HAVE_CONST)
  # Only set const to empty if it doesn't exist otherwise magick++ will not compile
  if(NOT HAVE_CONST)
    set(const " ")
  endif()

  # Check if <sys/types.h> doesn't define `gid_t'
  if(HAVE_SYS_TYPES_H)
    check_symbol_exists(gid_t sys/types.h HAVE_GID_T)
    if(NOT HAVE_GID_T)
      SET(gid_t int)
    endif()
  endif()

  # Check for the compiler inline compatible instruction
  check_c_source_compiles(
    "static inline int test (void) {return 0;}\nint main (void) {return test();}"
    HAVE_INLINE)

  check_c_source_compiles (
    "static __inline int test (void) {return 0;}\nint main (void) {return test();}"
    HAVE___INLINE)

  check_c_source_compiles (
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
    check_symbol_exists(mode_t sys/types.h HAVE_MODE_T)
    if(NOT HAVE_MODE_T)
      set(mode_t int)
    endif()
  endif()

  # Check if <sys/types.h> doesn't define `pid_t'
  if(HAVE_SYS_TYPES_H)
    check_symbol_exists(pid_t sys/types.h HAVE_PID_T)
    if(NOT HAVE_PID_T)
      set(pid_t int)
    endif()
  endif()

  # Check for the compiler restrict compatible instruction
  check_c_source_compiles(
    "int test (void *restrict x);\nint main (void) {return 0;}"
    HAVE_RESTRICT)

  check_c_source_compiles(
  "typedef struct abc *d;\nint test (d __restrict x);\nint main (void) {return 0;}"
    HAVE___RESTRICT)

  # We prefer __restrict to restrict unlike the AC_C_RESTRICT macro
  # because restrict requires C11 mode
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
      check_symbol_exists(size_t sys/types.h HAVE_SIZE_T)
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
      check_symbol_exists(ssize_t sys/types.h HAVE_SSIZE_T)
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
    check_symbol_exists(uid_t sys/types.h HAVE_UID_T)
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
  check_cxx_source_compiles(
  "
  void main() { volatile int i = 1; }
  "
  HAVE_VOLATILE)

  if(HAVE_VOLATILE)
    set(volatile volatile)
  else()
    set(volatile "")
  endif()

  #TODO check if this is actually true
  set(SETJMP_IS_THREAD_SAFE TRUE)
endmacro()
