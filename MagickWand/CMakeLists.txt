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

PROJECT(MagickWand C)

SET(MAGICKWAND_BASE_SRCS
#headers
MagickWand.h 
animate.h 
compare.h 
composite.h 
conjure.h 
convert.h 
deprecate.h 
display.h 
drawing-wand.h 
identify.h 
import.h 
magick-cli.h 
magick-image.h 
magick-property.h 
magick-wand-private.h
method-attribute.h 
mogrify.h 
mogrify-private.h
montage.h 
operation.h 
operation-private.h
pixel-iterator.h 
pixel-wand.h 
pixel-wand-private.h
script-token.h
stream.h 
studio.h
wand.h
wandcli.h 
wandcli-private.h
wand-view.h

#sources
animate.c 
compare.c
composite.c
conjure.c
convert.c
deprecate.c
display.c
drawing-wand.c
identify.c
import.c
magick-cli.c
magick-image.c
magick-property.c
magick-wand.c
mogrify.c
montage.c
operation.c
pixel-iterator.c
pixel-wand.c
script-token.c
stream.c
wand.c
wandcli.c
wand-view.c
)

IF(MAGICK_BUILD_STATIC)
  ADD_LIBRARY(${MAGICKWAND_BINARY_NAME} STATIC ${MAGICKWAND_BASE_SRCS} ${MAGICK_RESOURCE_FILE_PATH})
ELSE()
  ADD_LIBRARY(${MAGICKWAND_BINARY_NAME} SHARED ${MAGICKWAND_BASE_SRCS} ${MAGICK_RESOURCE_FILE_PATH})
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${MAGICKWAND_BINARY_NAME} PUBLIC ${IMAGEMAGICK_BASE_PATH})
TARGET_INCLUDE_DIRECTORIES(${MAGICKWAND_BINARY_NAME} PRIVATE  ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS})
# Seems that this must always be set MAGICKCORE_HDRI_ENABLE now or it will give preprocessor compile error
# Set _MAGICKLIB_ since this is a library project
# Set the MAGICKCORE_SHAREARCH_BASEDIRNAME to "config", not sure if I should add it to the <config.h>
TARGET_COMPILE_DEFINITIONS(${MAGICKWAND_BINARY_NAME} PRIVATE -DMAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH} -D${MAGICK_BUILD_DEFINE} -DMAGICKCORE_HDRI_ENABLE -D_MAGICKLIB_ -DMAGICKCORE_SHAREARCH_BASEDIRNAME=\"config\")
TARGET_LINK_LIBRARIES(${MAGICKWAND_BINARY_NAME} PUBLIC ${MAGICKCORE_BINARY_NAME})
