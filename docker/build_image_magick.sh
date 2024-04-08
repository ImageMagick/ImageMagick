#!/bin/bash -x

function title {
  echo "=========================="
  echo $1
  echo "=========================="
}

export HOME=/build
cd ${HOME}


title "Copying sources"
rsync -lr /src/ ${HOME}
rm -rf ${HOME}/build

PREFIX=/build/install

title "Generating CMake build"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${PREFIX}

title "Building ImageMagick"
cmake --build . --config Release -j 4
ls -al
cmake --install . --config Release 

title "Build test_delegates"
cd ${HOME}
export ImageMagick_ROOT=${PREFIX}/lib/cmake
mkdir -p build_test
cd build_test
cmake ../tests -DCMAKE_BUILD_TYPE=Release 
cmake --build . --config Release
./delegates ${EXPECTED_DELEGATES}
