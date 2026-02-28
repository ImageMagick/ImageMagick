#!/bin/bash -x

function title {
  echo "=========================="
  echo $1
  echo "=========================="
}

export HOME=/build

PREFIX=/${HOME}/install

title "Generating CMake build"
mkdir -p ${HOME}
cd ${HOME}
cmake /src -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${PREFIX}

title "Building ImageMagick"
cmake --build . --config Release -j 4
ls -al
cmake --install . --config Release 

title "Build test_delegates"
export ImageMagick_ROOT=${PREFIX}/lib/cmake
mkdir -p ${HOME}/build_test
cd ${HOME}/build_test
cmake /src/tests -DCMAKE_BUILD_TYPE=Release 
cmake --build . --config Release
./delegates ${EXPECTED_DELEGATES}
