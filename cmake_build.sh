#!/bin/bash

# Build and test the cmake build
# Should work out of the box on Linux and macOS
# Windows requires a bash shell, e.g. Git Bash
# conan requires that conan is installed
#
# Before:
# rm -rf build && cd build
#
# Then:
# bash ../cmake_build.sh <lib-source> <build-type> <Release|Debug>
#
#
# Usage examples:
#
# Retrieve all dependencies from conan and build a static library in Release mode
# bash ../cmake_build.sh conan static Release
#
# Use conan w/ pkg-config to find dependencies and build a static library in Debug mode
# (this is mostly for testing pkg-config support and is not supported on Windows)
# bash ../cmake_build.sh pkg static Debug
#
# Use system-installed libraries available to CMake and build a shared library in Release mode
# bash ../cmake_build.sh system shared Release
#
#
# Exports a CMake config file to ${CMAKE_INSTALL_PREFIX} (or ${HOME}/tmp/im by default)
# and builds a small test program using the exported config file and the ImageMagick library

CONAN=$1
STATIC=$2
RELEASE=$3
if [ -z "${CMAKE_INSTALL_PREFIX}" ]; then
  CMAKE_INSTALL_PREFIX=${HOME}/tmp/im
fi

case ${STATIC} in
  static)
    CMAKE_STATIC=-DBUILD_SHARED_LIBS=OFF
    ;;
  shared)
    CMAKE_STATIC=-DBUILD_SHARED_LIBS=ON
    ;;
  *)
    echo "invalid lib type"
    exit 1
    ;;
esac

case ${RELEASE} in
  Debug | Release) ;;
  *)
    echo "invalid build type"
    exit 1
    ;;
esac

shift
shift
shift

case ${CONAN} in
  conan)
    conan install .. -of . --build=missing -o libtype=${STATIC} --settings=build_type=${RELEASE} --settings compiler.cppstd=20 $@
    CMAKE_CONAN=-DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
    CMAKE_CONAN_TESTS=-DCMAKE_TOOLCHAIN_FILE="../conan_toolchain.cmake"
    case ${STATIC} in
      static)
        # This option is usable only with conan, the built-in find modules
        # behave horribly with static modules
        # Many of them call internally pkg-config and do not pass --static
        CMAKE_STATIC="${CMAKE_STATIC} -DMAGICK_PREFER_STATIC_LIBS=ON"
      ;;
    esac
    ;;
  pkg)
    CONAN_GENERATOR=PkgConfigDeps conan install .. -of . --build=missing -o libtype=${STATIC} --settings=build_type=${RELEASE} --settings compiler.cppstd=20 ${PROFILE}
    export PKG_CONFIG_PATH=$(pwd)
    ;;
  system)
    ;;
  *)
    echo "invalid build type"
    exit 1
    ;;
esac

# Main ImageMagick build
cmake .. -DCMAKE_BUILD_TYPE=${RELEASE} ${CMAKE_CONAN} ${CMAKE_STATIC} ${CMAKE_NOSYS} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DZERO_CONFIGURATION_SUPPORT=ON
cmake --build . -j 4
cmake --install .

# Build a client program using the exported CMake config file
mkdir -p delegates
cd delegates
ImageMagick_ROOT=${CMAKE_INSTALL_PREFIX}/lib/cmake cmake ../../tests -DCMAKE_BUILD_TYPE=Release ${CMAKE_CONAN_TESTS}
cmake --build .
./delegates
case `uname` in
	Darwin) otool -L delegates ;;
	Linux) ldd delegates ;;
esac
