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
cmake_minimum_required(VERSION 3.20.0)
cmake_policy(VERSION 3.20.0)

project(ImageMagick)

include(cmake/checks.cmake)
find_package(PkgConfig)
set(CMAKE_TRY_COMPILE_CONFIGURATION ${CMAKE_BUILD_TYPE})

# Prefer static pkgconfig settings when building a static library
macro(get_pkgconfig_setting TARGET_LIST PKG SETTING)
  if(NOT ${BUILD_SHARED_LIBS} AND ${PKG}_STATIC_${SETTING})
    list(APPEND ${TARGET_LIST} ${${PKG}_STATIC_${SETTING}})
  else()
    list(APPEND ${TARGET_LIST} ${${PKG}_${SETTING}})
  endif()
endmacro()

#
# This is the centralized dependency resolution procedure
#
macro(magick_find_delegate)
  cmake_parse_arguments(MAGICK_FIND "" "DELEGATE;NAME;DEFAULT;PKGCONFIG" "TARGETS" ${ARGN})

  unset(MAGICK_FIND_SEARCH_TARGET)
  if((NOT DEFINED ${MAGICK_FIND_DELEGATE} AND ${MAGICK_FIND_DEFAULT}) OR ${MAGICK_FIND_DELEGATE})
    # package is enabled by default or explicitly enabled
    set(${MAGICK_FIND_NAME}_FOUND FALSE)
    find_package(${MAGICK_FIND_NAME} QUIET)
    if(${${MAGICK_FIND_NAME}_FOUND})
      # CMake config
      message("-- Found CMake package ${MAGICK_FIND_NAME} ${${MAGICK_FIND_NAME}_VERSION}")
      if(MAGICK_FIND_TARGETS)
        # Custom target
        set(MAGICK_FIND_SEARCH_TARGET ${MAGICK_FIND_TARGETS})
      else()
        # Default target
        set(MAGICK_FIND_SEARCH_TARGET ${MAGICK_FIND_NAME}::${MAGICK_FIND_NAME})
      endif()
      if(TARGET ${MAGICK_FIND_SEARCH_TARGET})
        set(IMAGEMAGICK_DELEGATES_LIBRARIES ${IMAGEMAGICK_DELEGATES_LIBRARIES} ${MAGICK_FIND_SEARCH_TARGET})
        # Add the include directories to CMAKE_REQUIRED_INCLUDES to check for header files
        get_target_property(TARGET_INCLUDES ${MAGICK_FIND_SEARCH_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
        if(TARGET_INCLUDES)
          list(APPEND CMAKE_REQUIRED_INCLUDES ${TARGET_INCLUDES})
        endif()
        # This goes into the FindImageMagick.cmake dependencies
        list(APPEND IMAGEMAGICK_DEPENDENCIES ${MAGICK_FIND_NAME})
        set(${MAGICK_FIND_DELEGATE} TRUE)
      else()
        message("-- Did not find expected target ${MAGICK_FIND_SEARCH_TARGET}, not using it")
        set(${MAGICK_FIND_NAME}_FOUND FALSE)
      endif()
    endif()

    if(NOT ${${MAGICK_FIND_NAME}_FOUND} AND DEFINED PkgConfig_FOUND)
      # pkg-config
      if(MAGICK_FIND_PKGCONFIG)
        # Custom pkgconfig name
        set(MAGICK_FIND_SEARCH_TARGET ${MAGICK_FIND_PKGCONFIG})
      else()
        # Default name
        set(MAGICK_FIND_SEARCH_TARGET ${MAGICK_FIND_NAME})
      endif()
      pkg_check_modules(${MAGICK_FIND_NAME} ${MAGICK_FIND_SEARCH_TARGET})
      if(${${MAGICK_FIND_NAME}_FOUND})
        get_pkgconfig_setting(CMAKE_REQUIRED_INCLUDES ${MAGICK_FIND_NAME} INCLUDE_DIRS)
        get_pkgconfig_setting(IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${MAGICK_FIND_NAME} INCLUDE_DIRS)
        get_pkgconfig_setting(IMAGEMAGICK_DELEGATES_LIBRARIES ${MAGICK_FIND_NAME} LIBRARIES)
        get_pkgconfig_setting(IMAGEMAGICK_DELEGATES_LIBRARY_DIRS ${MAGICK_FIND_NAME} LIBRARY_DIRS)
        get_pkgconfig_setting(IMAGEMAGICK_DELEGATES_CFLAGS ${MAGICK_FIND_NAME} CFLAGS)
        get_pkgconfig_setting(IMAGEMAGICK_DELEGATES_LDFLAGS ${MAGICK_FIND_NAME} LDFLAGS_OTHER)
        set(${MAGICK_FIND_DELEGATE} TRUE)
      endif()
    endif()
  endif()

  if(${MAGICK_FIND_DELEGATE})
    message("Delegate ${MAGICK_FIND_NAME} enabled")
  else()
    message("Delegate ${MAGICK_FIND_NAME} not found or disabled")
  endif()
endmacro()

# Get the platform
if(WIN32)
  # CMake being a portable build system, all OS use the POSIX file system structure
  set(POSIX_ON_WINDOWS_SUPPORT TRUE)
  if(NOT WINDOWS)
    set(WINDOWS TRUE)
  endif()
  # TODO check for mingw or cygwin ???
elseif(UNIX AND NOT APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    set(LINUX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD.*")
    set(FREEBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kNetBSD.*|NetBSD.*")
    set(NETBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kOpenBSD.*|OpenBSD.*")
    set(OPENBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES ".*GNU.*")
    set(GNU TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES ".*BSDI.*")
    set(BSDI TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "DragonFly.*|FreeBSD")
    set(FREEBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "SYSV5.*")
    set(SYSV5 TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "Solaris.*")
    set(SOLARIS TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "HP-UX.*")
    set(HPUX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "AIX.*")
    set(AIX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "Minix.*")
    set(MINIX TRUE)
  endif()
elseif(APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Darwin.*")
    set(DARWIN TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*")
    set(MACOSX TRUE)
  endif()
  # TODO: iOS?
elseif(CMAKE_SYSTEM_NAME MATCHES "BeOS.*")
  set(BEOS TRUE)
elseif(CMAKE_SYSTEM_NAME MATCHES "Haiku.*")
  set(HAIKU TRUE)
endif()

# if we're on MacOS X we enable the universal build
if(APPLE)
	set(AC_APPLE_UNIVERSAL_BUILD TRUE)
endif()

# emscripten?
if(CMAKE_CXX_COMPILER MATCHES "/em\\+\\+(-[a-zA-Z0-9.])?$")
  message(" * C++ compiler: Emscripten")
  set(EMSCRIPTEN TRUE)
  add_link_options("-s MODULARIZE" "-s 'EXPORT_NAME=ImageMagick'" "-lembind"
    "-sAUTO_NATIVE_LIBRARIES=0" "-sAUTO_JS_LIBRARIES=0"
  )
endif()

magick_check_env()

####### OPTIONS #######
set(MAGICKCORE_QUANTUM_DEPTH "16" CACHE STRING "Number of bits in a pixel Quantum")
set_property(CACHE MAGICKCORE_QUANTUM_DEPTH PROPERTY STRINGS "8;16;32;64")
set(CHANNEL_MASK_DEPTH 32)
set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} MAGICKCORE_CHANNEL_MASK_DEPTH=${CHANNEL_MASK_DEPTH} MAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH})
set(MAGICK_PCFLAGS ${MAGICK_PCFLAGS} -DMAGICKCORE_CHANNEL_MASK_DEPTH=${CHANNEL_MASK_DEPTH} -DMAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH})

option(BUILD_SHARED_LIBS "Build ImageMagick as a shared library" TRUE)
if(${BUILD_SHARED_LIBS})
  set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} _DLL)
else()
  set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} _LIB STATIC_MAGICK NOAUTOLINK_MAGICK)
endif()

option(MAGICK_HDRI_ENABLE "Build ImageMagick with High dynamic-range imaging" TRUE)
if(MAGICK_HDRI_ENABLE)
  set(HDRI_SUFFIX HDRI)
  set(MAGICK_PCFLAGS ${MAGICK_PCFLAGS} -DMAGICKCORE_HDRI_ENABLE=1)
  set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} MAGICKCORE_HDRI_ENABLE=1)
else()
  set(MAGICK_PCFLAGS ${MAGICK_PCFLAGS} -DMAGICKCORE_HDRI_ENABLE=0)
  set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} MAGICKCORE_HDRI_ENABLE=1)
endif()

option(BUILD_MAGICKPP "Enable build/install of Magick++" TRUE)
option(BUILD_UTILITIES "Enable build/install of command-line utilities" TRUE)
option(BUILD_MODULES "Compile coders and filters as modules" FALSE) # TODO
option(CIPHER_SUPPORT "Permit enciphering and deciphering image pixels" TRUE)
option(EXCLUDE_DEPRECATED "Exclude deprecated methods" TRUE)
set(NAMESPACE_PREFIX "" CACHE STRING "Prefix MagickCore API methods")
set(NAMESPACE_PREFIX_TAG ${NAMESPACE_PREFIX})
set(PixelCacheThreshold "" CACHE STRING "Set pixel cache threshhold in MB (default available memory)")
option(WITH_DMALLOC "Use dmalloc for memory profiling" FALSE)
option(ZERO_CONFIGURATION_SUPPORT "Build self-contained, embeddable, zero-configuration ImageMagick" FALSE)
option(INSTALLED_SUPPORT "Expect the support files to be installed in prefix (TRUE) or to be found using MAGICK_HOME / executable path (FALSE)" TRUE)

# Fetch some needed values from version.sh and fill the correspondent variables
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/configure CONTENT_CONFIGURE)

# Try to find the date in the format yyyy-mm-dd
string(REGEX MATCH "\nPACKAGE_RELEASE_DATE=([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9])\n" FOUND_LAST_CHANGE_DATE "${CONTENT_CONFIGURE}")
if(FOUND_LAST_CHANGE_DATE)
  set(PACKAGE_RELEASE_DATE ${CMAKE_MATCH_1})
endif()

# Try to find `PACKAGE_NAME'
string(REGEX MATCH "\nPACKAGE_NAME='([A-Za-z0-9]+)'\n" FOUND_PACKAGE_NAME ${CONTENT_CONFIGURE})
if(FOUND_PACKAGE_NAME)
  set(PACKAGE_NAME ${CMAKE_MATCH_1})
endif()

# Try to find `PACKAGE_LIB_VERSION'
string(REGEX MATCH "\nPACKAGE_LIB_VERSION=([A-Fa-f0-9x]+)\n" FOUND_PACKAGE_LIB_VERSION ${CONTENT_CONFIGURE})
if(FOUND_PACKAGE_LIB_VERSION)
  set(PACKAGE_LIB_VERSION ${CMAKE_MATCH_1})
endif()

# Try to find `PACKAGE_VERSION'
string(REGEX MATCH "\nPACKAGE_BASE_VERSION=([0-9.]+)\n" FOUND_PACKAGE_VERSION ${CONTENT_CONFIGURE})
if(FOUND_PACKAGE_VERSION)
  set(MAGICK_LIB_VERSION_TEXT ${CMAKE_MATCH_1})
  set(MAGICKPP_LIB_VERSION_TEXT ${CMAKE_MATCH_1})
  set(PACKAGE_BASE_VERSION ${CMAKE_MATCH_1})
endif()

# Try parse just the major version number from MAGICK_LIB_VERSION_TEXT
string(REGEX MATCH "([0-9])\\..*" FOUND_VERSION_MAJOR "${MAGICK_LIB_VERSION_TEXT}")
if(FOUND_VERSION_MAJOR)
  set(MAGICK_MAJOR_VERSION ${CMAKE_MATCH_1})
endif()

# Try to find `MAGICK_LIBRARY_CURRENT', `MAGICK_LIBRARY_REVISION' and `MAGICK_LIBRARY_AGE'
string(REGEX MATCH "\nMAGICK_LIBRARY_CURRENT=([0-9]+)\nMAGICK_LIBRARY_REVISION=([0-9]+)\nMAGICK_LIBRARY_AGE=([0-9]+)\n" FOUND_MAGICK_LIB_VERSION_NUMBER ${CONTENT_CONFIGURE})
if(FOUND_MAGICK_LIB_VERSION_NUMBER)
  set(MAGICK_LIB_VERSION_NUMBER "${CMAKE_MATCH_1},${CMAKE_MATCH_2},${CMAKE_MATCH_3}")
  set(MAGICK_LIBRARY_CURRENT ${CMAKE_MATCH_1})
  set(MAGICK_LIBRARY_CURRENT_MIN ${CMAKE_MATCH_1})
endif()

# Try to find `PACKAGE_RELEASE'
string(REGEX MATCH "\nPACKAGE_VERSION_ADDENDUM=-([0-9]+)\n" FOUND_PACKAGE_RELEASE ${CONTENT_CONFIGURE})
if(FOUND_PACKAGE_RELEASE)
  set(PACKAGE_VERSION_ADDENDUM "-${CMAKE_MATCH_1}")
endif()

# Try to find `MAGICKPP_LIBRARY_CURRENT', `MAGICKPP_LIBRARY_REVISION' and `MAGICKPP_LIBRARY_AGE'
string(REGEX MATCH "\nMAGICKPP_LIBRARY_CURRENT=([0-9]+)\nMAGICKPP_LIBRARY_REVISION=([0-9]+)\nMAGICKPP_LIBRARY_AGE=([0-9]+)\n" FOUND_MAGICKPP_LIBRARY_VERSION_INFO ${CONTENT_CONFIGURE})
if(FOUND_MAGICKPP_LIBRARY_VERSION_INFO)
  set(MAGICKPP_LIBRARY_VERSION_INFO "${CMAKE_MATCH_1}:${CMAKE_MATCH_2}:${CMAKE_MATCH_3}")
  set(MAGICKPP_LIBRARY_CURRENT ${CMAKE_MATCH_1})
  set(MAGICKPP_LIBRARY_CURRENT_MIN ${CMAKE_MATCH_1})
endif()

# TODO This hasn't been tested
set(VERSION "\"${PACKAGE_BASE_VERSION}\"")
set(MAGICK_TARGET_CPU "${CMAKE_SYSTEM_PROCESSOR}")
set(MAGICK_TARGET_OS "\"${CMAKE_SYSTEM_NAME}\"")
set(MAGICK_TARGET_VENDOR "\"${CMAKE_SYSTEM_NAME}\"")
set(PACKAGE "\"ImageMagick\"")
set(PACKAGE_BUGREPORT "\"https://github.com/ImageMagick/ImageMagick/issues\"")
set(PACKAGE_STRING "${PACKAGE}-${PACKAGE_BASE_VERSION}")
set(PACKAGE_TARNAME "\"ImageMagick\"")
set(PACKAGE_URL "\"https://imagemagick.org\"")
set(PACKAGE_VERSION "\"${PACKAGE_BASE_VERSION}\"")

# Set if pipe (|) is enabled on filenames
# TODO Add linux and mac os X
# TODO maybe this would be better with a test somehow
if(WIN32)
  set(PIPES_SUPPORT FALSE)
endif()

# Check if we are on AIX, do the _ALL_SOURCE magic
if(AIX)
  set(_ALL_SOURCE 1)
  set(_LARGE_FILES 1)
endif()

# Check if we're on Solaris
IF (SOLARIS)
  set(_POSIX_PTHREAD_SEMANTICS TRUE)
  set(__EXTENSIONS__ TRUE)
  # TODO restrict the system version and check if the define should be these
  set(_UINT32_T uint32_t)
  set(_UINT64_T uint64_t)
  set(_UINT8_T uint8_t)
endif()

# TODO Not sure how to test this, so for now it's disabled
set(_TANDEM_SOURCE FALSE)

# TODO check if this is supposed to be any of the ones defined https://cmake.org/cmake/help/v3.0/module/FindX11.html
set(X11_CONFIGURE_PATH "")

# TODO should this be an option ??
set(X_DISPLAY_MISSING 1)

if(NOT MSVC)
  set(_FILE_OFFSET_BITS 64)
  set(_LARGEFILE_SOURCE 1)
else()
  # These days, the only compiler which requires it is MSVC
  set(_FILE_OFFSET_BITS 32)
endif()

# Disable run-time bounds-checking
set(_FORTIFY_SOURCE FALSE)

# ###### PATHS #######
set(CODER_DIRNAME "coders")
set(CODER_PATH "${CMAKE_INSTALL_PREFIX}/local/lib/ImageMagick-${MAGICK_MAJOR_VERSION}/modules-${MAGICK_ABI_SUFFIX}/${CODER_DIRNAME}/")
set(CONFIGURE_RELATIVE_PATH "ImageMagick-${MAGICK_MAJOR_VERSION}")
set(CONFIGURE_PATH "${CMAKE_INSTALL_PREFIX}/etc/${CONFIGURE_RELATIVE_PATH}/")
set(DOCUMENTATION_PATH "${CMAKE_INSTALL_PREFIX}/share/doc/ImageMagick-${MAGICK_MAJOR_VERSION}/")
set(EXECUTABLE_PATH "${CMAKE_INSTALL_PREFIX}/bin/")
set(FILTER_DIRNAME "filters")
set(INCLUDEARCH_PATH "${CMAKE_INSTALL_PREFIX}/include/")
set(INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/include/")
set(LIBRARY_RELATIVE_PATH "lib/ImageMagick-${MAGICK_MAJOR_VERSION}")
set(LIBRARY_PATH "${CMAKE_INSTALL_PREFIX}/${LIBRARY_RELATIVE_PATH}/")
set(LIB_BIN_BASEDIRNAME "bin")
set(SHAREARCH_BASEDIRNAME "share")
set(SHAREARCH_DIRNAME "share")
set(SHARE_RELATIVE_PATH "ImageMagick-7")
set(SHARE_PATH "${CMAKE_INSTALL_PREFIX}/${SHAREARCH_DIRNAME}/${SHARE_RELATIVE_PATH}/")
set(MODULES_BASEDIRNAME "modules")
set(MODULES_DIRNAME "modules")
set(LT_OBJDIR "")

# Set the default font search path
if(WIN32)
  set(MAGICK_FONT_PATH "$ENV{WINDIR}\\FONTS\\")
  # Escape the path C way where '\' must be '\\' (regex and cmake also escapes '\' so to find one we need '\\\\')
  string(REGEX REPLACE "\\\\" "\\\\\\\\" MAGICK_FONT_PATH ${MAGICK_FONT_PATH})
elseif(UNIX AND NOT APPLE)
  set(MAGICK_FONT_PATH "/usr/share/fonts/")
elseif(APPLE)
  set(MAGICK_FONT_PATH "/System/Library/Fonts/")
endif()

####### DELEGATES #######
magick_find_delegate(DELEGATE BZLIB_DELEGATE NAME BZip2 DEFAULT TRUE)
magick_find_delegate(DELEGATE LZMA_DELEGATE NAME LibLZMA DEFAULT TRUE)
magick_find_delegate(DELEGATE ZLIB_DELEGATE NAME ZLIB DEFAULT TRUE)
magick_find_delegate(DELEGATE ZIP_DELEGATE NAME ZIP DEFAULT TRUE TARGETS libzip::zip)
magick_find_delegate(DELEGATE ZSTD_DELEGATE NAME zstd DEFAULT TRUE
    TARGETS zstd::libzstd_static PKGCONFIG libzstd)
magick_find_delegate(DELEGATE BROTLI_DELEGATE NAME brotli DEFAULT TRUE)
magick_find_delegate(DELEGATE FREETYPE_DELEGATE NAME Freetype DEFAULT TRUE)

magick_find_delegate(DELEGATE XML_DELEGATE NAME LibXml2 DEFAULT TRUE)
# Alas, CMake has a very primitive built-in FindLibXml2.cmake that is cheeky enough
# to get in the way without automatically including all the dependencies
if(pkgcfg_lib_PC_LIBXML_iconv)
  list(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES ${pkgcfg_lib_PC_LIBXML_iconv})
endif()

magick_find_delegate(DELEGATE OPENMP_SUPPORT NAME OpenMP DEFAULT TRUE TARGETS OpenMP::OpenMP_C)

magick_find_delegate(DELEGATE THREADS_SUPPORT NAME Threads DEFAULT TRUE TARGETS)
if(CMAKE_USE_PTHREADS_INIT)
  set(THREAD_SUPPORT TRUE)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  set(IMAGEMAGICK_DELEGATES_LIBRARIES ${IMAGEMAGICK_DELEGATES_LIBRARIES} Threads::Threads)
endif()
CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
if(HAVE_PTHREAD_H)
  CHECK_LIBRARY_EXISTS(pthread pthread_create "" HAVE_PTHREAD)
  CHECK_SYMBOL_EXISTS(PTHREAD_PRIO_INHERIT pthread.h HAVE_PTHREAD_PRIO_INHERIT)
endif()
# TODO Not sure what to do here
set(PTHREAD_CREATE_JOINABLE "")

magick_find_delegate(DELEGATE TIFF_DELEGATE NAME TIFF DEFAULT TRUE)
if(TIFF_FOUND)
  set(HAVE_TIFFCONF_H 1)
  set(HAVE_TIFFISBIGENDIAN 1)
  set(HAVE_TIFFISCODECCONFIGURED 1)
  set(HAVE_TIFFMERGEFIELDINFO 1)
  set(HAVE_TIFFREADEXIFDIRECTORY 1)
  set(HAVE_TIFFSETERRORHANDLEREXT 1)
  set(HAVE_TIFFSETTAGEXTENDER 1)
  set(HAVE_TIFFSETWARNINGHANDLEREXT 1)
  set(HAVE_TIFFSWABARRAYOFTRIPLES 1)
endif()
magick_find_delegate(DELEGATE LCMS_DELEGATE NAME lcms DEFAULT TRUE PKGCONFIG lcms2)
if(lcms_FOUND)
  set(HAVE_LCMS2_H 1)
endif()
magick_find_delegate(DELEGATE RAW_R_DELEGATE NAME libraw DEFAULT TRUE)
if(libraw_FOUND)
  set(HAVE_LIBRAW_LIBRAW_H 1)
endif()
magick_find_delegate(DELEGATE HEIC_DELEGATE NAME libheif DEFAULT TRUE TARGETS libheif::heif)
if(libheif_FOUND)
  set(HAVE_LIBHEIF_HEIF_H 1)
endif()

magick_find_delegate(DELEGATE JBIG_DELEGATE NAME jbig DEFAULT TRUE)
magick_find_delegate(DELEGATE JPEG_DELEGATE NAME JPEG DEFAULT TRUE)
magick_find_delegate(DELEGATE LIBOPENJP2_DELEGATE NAME OpenJPEG DEFAULT TRUE
    TARGETS openjp2 PKGCONFIG libopenjp2)
# OpenJPEG has an ancient CMake that wasn't fixed until 2.5
if(OPENJPEG_INCLUDE_DIRS)
  list(APPEND IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${OPENJPEG_INCLUDE_DIRS})
endif()
magick_find_delegate(DELEGATE OPENEXR_DELEGATE NAME OpenEXR DEFAULT TRUE)
magick_find_delegate(DELEGATE PNG_DELEGATE NAME PNG DEFAULT TRUE)
magick_find_delegate(DELEGATE RSVG_DELEGATE NAME Rsvg DEFAULT TRUE)
magick_find_delegate(DELEGATE WEBP_DELEGATE NAME WebP DEFAULT TRUE
    TARGETS WebP::webp PKGCONFIG libwebp)
magick_find_delegate(DELEGATE WEBPMUX_DELEGATE NAME WebP DEFAULT TRUE
    TARGETS WebP::libwebpmux PKGCONFIG libwebpmux)
magick_find_delegate(DELEGATE WEBPDEMUX_DELEGATE NAME WebP DEFAULT TRUE
    TARGETS WebP::webpdemux PKGCONFIG libwebpdemux)
magick_find_delegate(DELEGATE JXL_DELEGATE NAME libjxl DEFAULT TRUE)
if(${JXL_DELEGATE})
  unset(JXL_DELEGATE)
  # ImageMagick requires the threaded version, othewise it cannot work
  magick_find_delegate(DELEGATE JXL_DELEGATE NAME libjxl_threads DEFAULT TRUE)
endif()
magick_find_delegate(DELEGATE FONTCONFIG_DELEGATE NAME Fontconfig DEFAULT TRUE)
magick_find_delegate(DELEGATE CAIRO_DELEGATE NAME Cairo DEFAULT TRUE
    TARGETS cairo::cairo PKGCONFIG cairo)

magick_find_delegate(DELEGATE FFTW_DELEGATE NAME FFTW3 DEFAULT TRUE
    TARGETS fftw::fftw PKGCONFIG fftw3)

magick_find_delegate(DELEGATE HASJEMALLOC NAME jemalloc DEFAULT FALSE)

magick_find_delegate(DELEGATE LTDL_DELEGATE NAME LTDL DEFAULT TRUE)
set(LTDL_MODULE_EXT "")
if (LTDL_FOUND)
  magick_find_delegate(DELEGATE OPENCLLIB_DELEGATE NAME OpenCL DEFAULT TRUE)
  if(NOT OpenCL_FOUND)
    unset(OPENCLLIB_DELEGATE)
    magick_find_delegate(DELEGATE OPENCLLIB_DELEGATE NAME OpenCLHeaders DEFAULT TRUE
      TARGETS OpenCL::Headers PKGCONFIG OpenCL-Headers)
  endif()
  if(${OPENCLLIB_DELEGATE})
    set(_OPENCL 1)
    list(APPEND IMAGEMAGICK_DELEGATES_CFLAGS -DCL_TARGET_OPENCL_VERSION=300)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DCL_TARGET_OPENCL_VERSION=300)
    check_c_source_compiles("#include <OpenCL/cl.h>\nint main(void) { return 0; }" HAVE_OPENCL_CL_H)
    check_c_source_compiles("#include <CL/cl.h>\nint main(void) { return 0; }" HAVE_CL_CL_H)
  endif()
endif()

# TODO
# These are absent from conan
magick_find_delegate(DELEGATE AUTOTRACE_DELEGATE NAME AUTOTRACE DEFAULT TRUE)
magick_find_delegate(DELEGATE DJVU_DELEGATE NAME DJVU DEFAULT TRUE)
magick_find_delegate(DELEGATE DPS_DELEGATE NAME DPS DEFAULT TRUE)
magick_find_delegate(DELEGATE FLIF_DELEGATE NAME FLIF DEFAULT TRUE)
magick_find_delegate(DELEGATE FPX_DELEGATE NAME FlashPIX DEFAULT TRUE)
magick_find_delegate(DELEGATE GS_DELEGATE NAME Ghostscript DEFAULT TRUE)
magick_find_delegate(DELEGATE GVC_DELEGATE NAME GVC DEFAULT TRUE)
magick_find_delegate(DELEGATE PANGO_DELEGATE NAME Pango DEFAULT TRUE)
magick_find_delegate(DELEGATE PANGOCAIRO_DELEGATE NAME PangoCairo DEFAULT TRUE)
magick_find_delegate(DELEGATE RAQM_DELEGATE NAME RAQM DEFAULT TRUE)
magick_find_delegate(DELEGATE WMF_DELEGATE NAME WMF DEFAULT TRUE)
magick_find_delegate(DELEGATE LQR NAME Lqr DEFAULT TRUE)

magick_find_delegate(DELEGATE HASUMEM NAME UMEM DEFAULT TRUE)

# TODO Should we check if gdi32 exists if windows?
if(WIN32)
  set(WINGDI32_DELEGATE TRUE)
endif()

# Compile with X11 if present
magick_find_delegate(DELEGATE X11_DELEGATE NAME X11 DEFAULT TRUE)
if(X11_DELEGATE)
  set(IMAGEMAGICK_DELEGATES_LIBRARIES ${IMAGEMAGICK_DELEGATES_LIBRARIES} X11::Xext)
endif()

# Set ImageMagick base dir to be used inside sub-projects
set(IMAGEMAGICK_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Set coders andd filters path
set(CODERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/coders)
set(FILTERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/filters)

set(IMAGEMAGICK_INC_DIR "include/ImageMagick-${MAGICK_MAJOR_VERSION}")

####### CONFIG #######
# Set the version.h file path to be autogenerated
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MagickCore/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/MagickCore/version.h)

if(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 8)
  set(MagickQuantumDepth "Q8")
elseif(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 16)
  set(MagickQuantumDepth "Q16")
elseif(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 32)
  set(MagickQuantumDepth "Q32")
elseif(${MAGICKCORE_QUANTUM_DEPTH} EQUAL 64)
  set(MagickQuantumDepth "Q64")
else()
  set(MagickQuantumDepth "Q?")
endif()

# rc files are only used on Windows binaries
if(WIN32)
  set(MagickLibVersionNumber ${MAGICK_LIB_VERSION_NUMBER})
  set(MagickLibVersionText \"${MAGICK_LIB_VERSION_TEXT}\")

  if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(MagickPlatform "x86")
  elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(MagickPlatform "x64")
  else()
    set(MagickPlatform ${MAGICK_TARGET_CPU})
  endif()

  # Read the version.h file so we can get some needed values using regex
  file(READ ${CMAKE_CURRENT_BINARY_DIR}/MagickCore/version.h CONTENT_VERSION_H)

  # Try to find `MagickCopyright'
  string(REGEX MATCH "\n#define[ ]+MagickCopyright[ ]+\"([^\"]+)\"\n" FOUND_MAGICK_COPYRIGHT ${CONTENT_VERSION_H})
  if(FOUND_MAGICK_COPYRIGHT)
    set(MagickCopyright \"${CMAKE_MATCH_1}\")
  endif()

  # Try to find `MagickAuthoritativeURL'
  string(REGEX MATCH "\n#define[ ]+MagickAuthoritativeURL[ ]+\"([^\"]+)\"\n" FOUND_MAGICK_AUTHORITATIVE_URL ${CONTENT_VERSION_H})
  if(FOUND_MAGICK_AUTHORITATIVE_URL)
    set(MagickAuthoritativeURL ${CMAKE_MATCH_1})
  endif()

  set(MagickVersion "\"${PACKAGE_NAME} ${MAGICK_LIB_VERSION_TEXT}${PACKAGE_VERSION_ADDENDUM} ${MagickQuantumDepth} ${MagickPlatform} ${PACKAGE_RELEASE_DATE} ${MagickAuthoritativeURL}\"")

  # Set the ImageMagick.rc file path to be autogenerated
  set(MAGICK_RESOURCE_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/config/resources/ImageMagick.rc)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/ImageMagick.rc.cmake ${MAGICK_RESOURCE_FILE_PATH})
  # Copy only the needed resource files to output directory
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/images/ImageMagick.ico ${CMAKE_CURRENT_BINARY_DIR}/images/ImageMagick.ico COPYONLY)
endif()

# Before making the config.h we still need to do somethings
set(PACKAGE_NAME \"${PACKAGE_NAME}\")

# Set the config.h file path to be autogenerated
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config/config.h)

# Now we need to read its content to generate the magick-baseconfig.h header
file(READ ${CMAKE_CURRENT_BINARY_DIR}/config/config.h CONTENT_CONFIG_H)

# Now we need to Regex Replace some things in the header file
# All caps defines will be prefixed with a macro MAGICKCORE_
string(REGEX REPLACE
  "\\*\\/\n#define[ ]+([A-Z0-9_]+)[ ]*([\./ a-zA-Z0-9\":_\\\-]*)\n"
  "*/\n#ifndef MAGICKCORE_\\1\n#define MAGICKCORE_\\1 \\2\n#endif\n"
  CONTENT_CONFIG_H ${CONTENT_CONFIG_H}
)
# All uncaps defines will be prefixed with a macro _magickcore_
string(REGEX REPLACE
  "\\*\\/\n#define[ ]+([a-z0-9_]+)[ ]*([\./ a-zA-Z0-9\":_\\\-]*)\n"
  "*/\n#ifndef _magickcore_\\1\n#define _magickcore_\\1 \\2\n#endif\n"
  CONTENT_CONFIG_H ${CONTENT_CONFIG_H}
)
# For some reason (I can't seem to find the logic) there's some defines we need to remove the prefix
string(REGEX REPLACE
  "_magickcore___func__"
  "__func__"
  CONTENT_CONFIG_H ${CONTENT_CONFIG_H}
)
# Also for ssize_t we need to typedef instead of define or else magick++ will not compile
string(REGEX REPLACE
  "#ifndef _magickcore_ssize_t\n#define _magickcore_ssize_t ([a-zA-Z ]*)\n#endif"
  "typedef \\1 ssize_t;"
  CONTENT_CONFIG_H ${CONTENT_CONFIG_H}
)

# Now we need to append things to the previously read file content
set(CONTENT_CONFIG_H
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
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/MagickCore/magick-baseconfig.h "${CONTENT_CONFIG_H}")

if(ZERO_CONFIGURATION_SUPPORT)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/config/thresholds.xml THRESHOLD_XML)
  # Convert xml file to header
  string(REPLACE "\"" "\\\"" THRESHOLD_XML "${THRESHOLD_XML}")
  string(REGEX REPLACE "\n" "\"\n    \"" THRESHOLD_XML "${THRESHOLD_XML}")
  set(THRESHOLD_H "static const char *const\n  BuiltinMap=\n    \"${THRESHOLD_XML}")
  string(LENGTH ${THRESHOLD_H} THRESHOLD_H_LEN)
  math(EXPR THRESHOLD_H_LEN "${THRESHOLD_H_LEN} - 6")
  string(SUBSTRING ${THRESHOLD_H} 0 ${THRESHOLD_H_LEN} THRESHOLD_H)
  set(THRESHOLD_H "${THRESHOLD_H};\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/MagickCore/threshold-map.h "${THRESHOLD_H}")
endif()

# Set the binaries name
set(MAGICK_ABI_SUFFIX "${MagickQuantumDepth}${HDRI_SUFFIX}")
set(MAGICKCORE_BINARY_NAME "MagickCore-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}")
set(MAGICKWAND_BINARY_NAME "MagickWand-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}")
set(MAGICKXX_BINARY_NAME "Magick++-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}")
set(MAGICK_BINARY_NAME "Magick")

# Allow direct linking to the libraries from the build tree
set(BUILD_RPATH ${CMAKE_LIBRARY_PATH} ${CMAKE_CURRENT_BINARY_DIR}/MagickCore ${CMAKE_CURRENT_BINARY_DIR}/MagickWand)

# Configure Magick++-config
if($ENV{PKG_CONFIG})
  set(PKG_CONFIG "PKG_CONFIG_PATH=${CMAKE_INSTALL_PREFIX}/lib $ENV{PKG_CONFIG}")
else()
  set(PKG_CONFIG "PKG_CONFIG_PATH=${CMAKE_INSTALL_PREFIX}/lib pkg-config")
endif()
set(prefix ${CMAKE_INSTALL_PREFIX})
if(NOT WIN32)
  set(exec_prefix "\${prefix}/bin")
else()
  set(exec_prefix "\${prefix}")
endif()
set(libdir "\${prefix}/lib")
set(includedir "\${prefix}/include")
if(NOT INCLUDEARCH_DIR)
  set(INCLUDEARCH_DIR "\${prefix}/include")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/ImageMagickConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ImageMagickConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ImageMagickConfig.cmake DESTINATION lib/cmake)

# Add the subdirectories to be compiled
add_subdirectory("MagickCore")
add_subdirectory("MagickWand")
if(BUILD_MAGICKPP)
  add_subdirectory("Magick++")
endif()
if(BUILD_UTILITIES)
  add_subdirectory("utilities")
endif()
if(NOT ZERO_CONFIGURATION_SUPPORT)
  add_subdirectory("config")
endif()

install(FILES
  LICENSE
  NOTICE
  DESTINATION licenses
)
