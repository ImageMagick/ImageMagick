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

PROJECT(Magick C)

SET(MAGICK_SRC
magick.c
)

ADD_EXECUTABLE(${MAGICK_BINARY_NAME} ${MAGICK_SRC} ${MAGICK_RESOURCE_FILE_PATH})

TARGET_INCLUDE_DIRECTORIES(${MAGICK_BINARY_NAME} PRIVATE ${IMAGEMAGICK_BASE_PATH})
# Seems that this must always be set MAGICKCORE_HDRI_ENABLE now or it will give preprocessor compile error
TARGET_COMPILE_DEFINITIONS(${MAGICK_BINARY_NAME} PRIVATE -DMAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH} -D${MAGICK_BUILD_DEFINE} -DMAGICKCORE_HDRI_ENABLE)
TARGET_LINK_LIBRARIES(${MAGICK_BINARY_NAME} PRIVATE ${MAGICKWAND_BINARY_NAME})

INSTALL(TARGETS ${MAGICK_BINARY_NAME}
RUNTIME DESTINATION bin
)
