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

project(Magick++ CXX)

SET(MAGICKXX_PUBLIC_HEADERS
  lib/Magick++/Blob.h
  lib/Magick++/CoderInfo.h
  lib/Magick++/Color.h
  lib/Magick++/Drawable.h
  lib/Magick++/Exception.h
  lib/Magick++/Functions.h
  lib/Magick++/Geometry.h
  lib/Magick++/Image.h
  lib/Magick++/Include.h
  lib/Magick++/Montage.h
  lib/Magick++/Pixels.h
  lib/Magick++/ResourceLimits.h
  lib/Magick++/SecurityPolicy.h
  lib/Magick++/Statistic.h
  lib/Magick++/STL.h
  lib/Magick++/TypeMetric.h
)

set(MAGICKXX_SRC
  lib/Blob.cpp
  lib/BlobRef.cpp
  lib/CoderInfo.cpp
  lib/Color.cpp
  lib/Drawable.cpp
  lib/Exception.cpp
  lib/Functions.cpp
  lib/Geometry.cpp
  lib/Image.cpp
  lib/ImageRef.cpp
  lib/Montage.cpp
  lib/Options.cpp
  lib/Pixels.cpp
  lib/ResourceLimits.cpp
  lib/SecurityPolicy.cpp
  lib/Statistic.cpp
  lib/STL.cpp
  lib/Thread.cpp
  lib/TypeMetric.cpp
  lib/Magick++.h
  lib/Magick++/Blob.h
  lib/Magick++/BlobRef.h
  lib/Magick++/CoderInfo.h
  lib/Magick++/Color.h
  lib/Magick++/Drawable.h
  lib/Magick++/Exception.h
  lib/Magick++/Functions.h
  lib/Magick++/Geometry.h
  lib/Magick++/Image.h
  lib/Magick++/ImageRef.h
  lib/Magick++/Include.h
  lib/Magick++/Montage.h
  lib/Magick++/Options.h
  lib/Magick++/Pixels.h
  lib/Magick++/ResourceLimits.h
  lib/Magick++/SecurityPolicy.h
  lib/Magick++/Statistic.h
  lib/Magick++/STL.h
  lib/Magick++/Thread.h
  lib/Magick++/TypeMetric.h
)

if(MAGICK_BUILD_STATIC)
  add_library(${MAGICKXX_BINARY_NAME} STATIC ${MAGICKXX_SRC} ${MAGICK_RESOURCE_FILE_PATH})
else()
  add_library(${MAGICKXX_BINARY_NAME} SHARED ${MAGICKXX_SRC} ${MAGICK_RESOURCE_FILE_PATH})
endif()

set_target_properties(${MAGICKXX_BINARY_NAME} PROPERTIES PUBLIC_HEADER "${MAGICKXX_PUBLIC_HEADERS}")
target_include_directories(${MAGICKXX_BINARY_NAME} PUBLIC ${IMAGEMAGICK_BASE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_compile_definitions(${MAGICKXX_BINARY_NAME} PRIVATE ${MAGICK_BUILD_DEFINES} -D_MAGICKLIB_)
target_link_libraries(${MAGICKXX_BINARY_NAME} PUBLIC ${MAGICKWAND_BINARY_NAME})

install(TARGETS ${MAGICKXX_BINARY_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION ${IMAGEMAGICK_INC_DIR}/Magick++
)

install(FILES
  lib/Magick++.h DESTINATION ${IMAGEMAGICK_INC_DIR}
)
