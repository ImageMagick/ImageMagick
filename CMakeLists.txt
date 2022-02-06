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

project(ImageMagick)

include(cmake/checks.cmake)

macro(magick_find_delegate delegate delegate_name default)
  if(DEFINED delegate OR ${default})
    find_package(${delegate_name} QUIET)
    set(${delegate} ${${delegate_name}_FOUND})
    if(${delegate})
      message("Delegate ${delegate_name} found.")
      set(IMAGEMAGICK_DELEGATES_INCLUDE_DIRS ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS} ${${delegate_name}_INCLUDE_DIR})
      set(IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${IMAGEMAGICK_DELEGATES_LIBRARIES_LINK} ${${delegate_name}_LIBRARIES})
      set(IMAGEMAGICK_DELEGATES_COMPILER_DEFINITIONS ${IMAGEMAGICK_DELEGATES_COMPILER_DEFINITIONS} ${${delegate_name}_DEFINITIONS})
    else()
      message("Delegate ${delegate_name} disabled. (Not found)")
    endif()
  else()
    set(${delegate} FALSE)
    message("Delegate ${delegate_name} disabled.")
  endif()
endmacro()

# Get the platform
if(WIN32)
  if(NOT WINDOWS)
    set(WINDOWS TRUE)
  endif()
  #TODO check for mingw or cygwin ???
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

magick_check_env()

# Don't compile coders and filters as modules
set(BUILD_MODULES FALSE)

# Permit enciphering and deciphering image pixels
set(CIPHER_SUPPORT TRUE)

# Not really sure what's this for
set(CODER_DIRNAME "")

# I think this is used if we wanted to install, since we only want a static library
set(CONFIGURE_PATH "")

# TODO should this be the path used????
set(CONFIGURE_RELATIVE_PATH "\"ImageMagick-7\"")

# I think this is used if we wanted to install, since we only want a static library
set(DOCUMENTATION_PATH "")

# Give user the option to compile with deprecated methods
option(EXCLUDE_DEPRECATED "Exclude deprecated methods" ON)

# TODO I think this is used if we wanted to install, since we only want a static library
set(EXECUTABLE_PATH "")

# TODO Not really sure what's this for
set(FILTER_DIRNAME "")

# This seems to be an ImageMagick flag to accurately represent the wide range of intensity levels found in real scenes 
set(HDRI_ENABLE_OBSOLETE_IN_H FALSE)

# Compile without JEMALLOC dependency
set(HasJEMALLOC FALSE)

# Compile without UMEM dependency
set(HasUMEM FALSE)

# I think this is used if we wanted to install, since we only want a static library
set(INCLUDEARCH_PATH "")

# I think this is used if we wanted to install, since we only want a static library
set(INCLUDE_PATH "")

# Not really sure what's this for
set(INSTALLED_SUPPORT "")

# TODO should this be the path used????
set(LIBRARY_PATH "\"ImageMagick-7\"")

# TODO should this be the path used????
set(LIBRARY_RELATIVE_PATH "\"ImageMagick-7\"")

# TODO I think this is used if we wanted to install, since we only want a static library
set(LIB_BIN_BASEDIRNAME "")

# TODO Compile without LQR dependency
set(LQR FALSE)

# TODO We are not compiling against LTDL
set(LTDL_MODULE_EXT "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(LT_OBJDIR "")

# Set the default font search path
# TODO add dir for linux and mac os x
if(WIN32)
  set(MAGICK_FONT_PATH "\"$ENV{WINDIR}\\FONTS\\\"")
  # Escape the path C way where '\' must be '\\' (regex and cmake also escapes '\' so to find one we need '\\\\')
  string(REGEX REPLACE "\\\\" "\\\\\\\\" MAGICK_FONT_PATH ${MAGICK_FONT_PATH})
elseif(UNIX AND NOT APPLE)
  set(MAGICK_FONT_PATH "/usr/share/fonts/")
elseif(APPLE)
  set(MAGICK_FONT_PATH "/System/Library/Fonts/")
endif()

#TODO should we use the host cpu or the build bitness???
set(MAGICK_TARGET_CPU "")

#TODO what should go on the OS???
set(MAGICK_TARGET_OS "")

#TODO what should go on the vendor???
set(MAGICK_TARGET_VENDOR "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(MODULES_BASEDIRNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(MODULES_DIRNAME "")

# TODO if a namespace should be better define it here
set(NAMESPACE_PREFIX "")

# TODO same as above
set(NAMESPACE_PREFIX_TAG "")

# TODO Not sure why this isn't an ifdef inside the code, so it would check if it was on Debug or Release
set(NDEBUG 1)

# TODO I think this is used if we wanted to install, since we only want a static library
set(PACKAGE "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(PACKAGE_BUGREPORT "")

# TODO Will be correctly filled near the end of file
set(PACKAGE_NAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(PACKAGE_STRING "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(PACKAGE_TARNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(PACKAGE_URL "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(PACKAGE_VERSION "")

# Set if pipe (|) is enabled on filenames 
# TODO Add linux and mac os X
# TODO maybe this would be better with a test somehow
if(WIN32)
  set(PIPES_SUPPORT FALSE)
endif()

# TODO Not sure what to do here
set(PTHREAD_CREATE_JOINABLE "")

# TODO Maybe this should be an option
set(PixelCacheThreshold "")

# TODO Not sure what value to give here
set(QUANTUM_DEPTH_OBSOLETE_IN_H 16)

# TODO I think this is used if we wanted to install, since we only want a static library
set(SHAREARCH_BASEDIRNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(SHAREARCH_DIRNAME "")

# TODO I think this is used if we wanted to install, since we only want a static library
set(SHARE_PATH "")

# TODO should this be the path used????
set(SHARE_RELATIVE_PATH "\"ImageMagick-7\"")

# Check POSIX threads libraries and header files exists
find_package(Threads)
if(CMAKE_USE_PTHREADS_INIT)
  set(THREAD_SUPPORT TRUE)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
endif()

# Check if we are on AIX, do the _ALL_SOURCE magic
if(AIX)
  set(_ALL_SOURCE 1)
endif()

# Check if we're on Solaris
IF (SOLARIS)
  set(_POSIX_PTHREAD_SEMANTICS TRUE)
  set(__EXTENSIONS__ TRUE)
endif()

# TODO Not sure how to test this, so for now it's disabled
set(_TANDEM_SOURCE FALSE)

# TODO Define the version
set(VERSION "")

option(WITH_DMALLOC "Use dmalloc for memory profiling" OFF)

# TODO check if this is supposed to be any of the ones defined https://cmake.org/cmake/help/v3.0/module/FindX11.html
set(X11_CONFIGURE_PATH "")

# TODO should this be an option ??
set(X_DISPLAY_MISSING 1)

# TODO Should we try this ???? also option??
set(ZERO_CONFIGURATION_SUPPORT 0)

# TODO does this suffice?
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
  set(_FILE_OFFSET_BITS 64)
  set(_LARGEFILE_SOURCE 1)
else()
# Not sure on Windows if code is made to work with 64 bits 
  set(_FILE_OFFSET_BITS 32)
endif()

# Disable run-time bounds-checking
set(_FORTIFY_SOURCE FALSE)

# Check if we are on AIX, activate _LARGE_FILES
if(AIX)
  set(_LARGE_FILES 1)
endif()

#TODO restrict the system version and check if the define should be these
if(SOLARIS)
  set(_UINT32_T uint32_t)
  set(_UINT64_T uint64_t)
  set(_UINT8_T uint8_t)
endif()

# Give user the choice for MAGICKCORE_QUANTUM_DEPTH bits
set(MAGICKCORE_QUANTUM_DEPTH "16" CACHE STRING "Number of bits in a pixel Quantum")
set_property(CACHE MAGICKCORE_QUANTUM_DEPTH PROPERTY STRINGS "8;16;32;64")
set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} -DMAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH})

# Give user the choice for MAGICK_STATIC_BUILD bits
option(MAGICK_BUILD_STATIC "Build ImageMagick statically" OFF)
if(MAGICK_BUILD_STATIC)
  set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} -D_LIB)
else()
  set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} -D_DLL)
endif()

# Give user the choice for MAGICK_HDRI_ENABLE
option(MAGICK_HDRI_ENABLE "Build ImageMagick with High dynamic-range imaging" ON)
if(MAGICK_HDRI_ENABLE)
  set(HDRI_SUFFIX HDRI)
  set(MAGICK_BUILD_DEFINES ${MAGICK_BUILD_DEFINES} -DMAGICKCORE_HDRI_ENABLE)
endif()

# Fetch some needed values from version.sh and fill the correspondent variables
FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/configure CONTENT_CONFIGURE)

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
endif()

# Try parse just the major version number from MAGICK_LIB_VERSION_TEXT
string(REGEX MATCH "([0-9])\\..*" FOUND_VERSION_MAJOR "${MAGICK_LIB_VERSION_TEXT}")
if(FOUND_VERSION_MAJOR)
  set(MAGICK_VERSION_MAJOR ${CMAKE_MATCH_1})
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
"\\*\\/\n#define[ ]+([A-Z0-9_]+)[ ]*([a-zA-Z0-9\":\\\-]*)\n" 
"*/\n#ifndef MAGICKCORE_\\1\n#define MAGICKCORE_\\1 \\2\n#endif\n"
CONTENT_CONFIG_H ${CONTENT_CONFIG_H})
# All uncaps defines will be prefixed with a macro _magickcore_
string(REGEX REPLACE 
"\\*\\/\n#define[ ]+([a-z0-9_]+)[ ]*([a-zA-Z0-9 \"_]*)\n" 
"*/\n#ifndef _magickcore_\\1\n#define _magickcore_\\1 \\2\n#endif\n"
CONTENT_CONFIG_H ${CONTENT_CONFIG_H})
# For some reason (I can't seem to find the logic) there's some defines we need to remove the prefix
string(REGEX REPLACE "_magickcore___func__" "__func__" CONTENT_CONFIG_H ${CONTENT_CONFIG_H})
# Also for ssize_t we need to typedef instead of define or else magick++ will not compile
string(REGEX REPLACE 
"#ifndef _magickcore_ssize_t\n#define _magickcore_ssize_t ([a-zA-Z ]*)\n#endif" 
"typedef \\1 ssize_t;"
CONTENT_CONFIG_H ${CONTENT_CONFIG_H})

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

####### DELEGATES #######
magick_find_delegate(BZLIB_DELEGATE BZip2 TRUE)
magick_find_delegate(LZMA_DELEGATE LibLZMA TRUE)
magick_find_delegate(ZLIB_DELEGATE ZLIB TRUE)
magick_find_delegate(ZSTD_DELEGATE Zstd TRUE)

magick_find_delegate(LCMS_DELEGATE LCMS TRUE)
magick_find_delegate(FREETYPE_DELEGATE Freetype TRUE)
magick_find_delegate(XML_DELEGATE LibXml2 TRUE)

# TODO Is this correct?
magick_find_delegate(TIFF_DELEGATE TIFF TRUE)
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

magick_find_delegate(HEIC_DELEGATE HEIF TRUE)
magick_find_delegate(JBIG_DELEGATE JBIG TRUE)
magick_find_delegate(JPEG_DELEGATE JPEG TRUE)
magick_find_delegate(LIBOPENJP2_DELEGATE OpenJpeg TRUE)
magick_find_delegate(OPENEXR_DELEGATE OpenEXR TRUE)
magick_find_delegate(PNG_DELEGATE PNG TRUE)
magick_find_delegate(RAW_R_DELEGATE LibRaw TRUE)
magick_find_delegate(RSVG_DELEGATE Rsvg TRUE)
magick_find_delegate(WEBP_DELEGATE WEBP TRUE)
magick_find_delegate(JXL_DELEGATE JXL TRUE)

# TODO are these correct and should they be off by default?
magick_find_delegate(AUTOTRACE_DELEGATE AUTOTRACE FALSE)
magick_find_delegate(CAIRO_DELEGATE Cairo FALSE)
magick_find_delegate(DJVU_DELEGATE DJVU FALSE)
magick_find_delegate(DPS_DELEGATE DPS FALSE)
magick_find_delegate(FFTW_DELEGATE FFTW FALSE)
magick_find_delegate(FLIF_DELEGATE FLIF FALSE)
magick_find_delegate(FONTCONFIG_DELEGATE Fontconfig FALSE)
magick_find_delegate(FPX_DELEGATE FlashPIX FALSE)
magick_find_delegate(GS_DELEGATE Ghostscript FALSE)
magick_find_delegate(GVC_DELEGATE GVC FALSE)
magick_find_delegate(LTDL_DELEGATE LTDL FALSE)
magick_find_delegate(PANGO_DELEGATE Pango FALSE)
magick_find_delegate(PANGOCAIRO_DELEGATE PangoCairo FALSE)
magick_find_delegate(RAQM_DELEGATE RAQM FALSE)
magick_find_delegate(WEBPMUX_DELEGATE WEBPMUX FALSE)
magick_find_delegate(WMF_DELEGATE WMF FALSE)

# Set OpenCL Accelerator
# TODO does it suffice to enable only if we have the headers ???
if((HAVE_OPENCL_CL_H OR HAVE_CL_CL_H) AND DEFINED OPENCL_LIBRARY)
  set(_OPENCL 1)
  set(OPENCLLIB_DELEGATE TRUE)
else()
  set(_OPENCL "")
endif()

# TODO Should we check if gdi32 exists if windows?
if(WIN32)
  set(WINGDI32_DELEGATE TRUE)
endif()

# Compile with X11 if present
magick_find_delegate(X11_DELEGATE X11 TRUE)

# Delegate special cases
if(ZLIB_DELEGATE)
  option(LZMA_API_STATIC "Use lib LZMA as static?" OFF)
  if(LZMA_API_STATIC)
    set(IMAGEMAGICK_DELEGATES_COMPILER_DEFINITIONS ${IMAGEMAGICK_DELEGATES_COMPILER_DEFINITIONS} "-DLZMA_API_STATIC")
  endif()
endif()

if(OPENCLLIB_DELEGATE)
  set(IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${IMAGEMAGICK_DELEGATES_LIBRARIES_LINK} ${OPENCL_LIBRARY})
endif()

# Set ImageMagick base dir to be used inside sub-projects
set(IMAGEMAGICK_BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Set coders andd filters path
set(CODERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/coders)
set(FILTERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/filters)

set(IMAGEMAGICK_INC_DIR "include/ImageMagick-${MAGICK_VERSION_MAJOR}")

#Although this is not a delegate we can add the pthread library dependency here
if(THREAD_SUPPORT)
  list(APPEND IMAGEMAGICK_DELEGATES_LIBRARIES_LINK ${CMAKE_THREAD_LIBS_INIT})
endif()

# Set the binaries name
set(MAGICK_BINARY_SUFFIX "${MAGICK_VERSION_MAJOR}.${MagickQuantumDepth}${HDRI_SUFFIX}")
set(MAGICKCORE_BINARY_NAME "MagickCore-${MAGICK_BINARY_SUFFIX}")
set(MAGICKWAND_BINARY_NAME "MagickWand-${MAGICK_BINARY_SUFFIX}")
set(MAGICKXX_BINARY_NAME "Magick++-${MAGICK_BINARY_SUFFIX}")
set(MAGICK_BINARY_NAME "Magick")

# Add the subdirectories to be compiled
add_subdirectory("MagickCore")
# TODO: Add options for these
add_subdirectory("MagickWand")
add_subdirectory("Magick++")
add_subdirectory("utilities")
