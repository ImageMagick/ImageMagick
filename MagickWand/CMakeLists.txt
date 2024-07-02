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

project(MagickWand C)

set(MAGICKWAND_SRC
  MagickWand.h
  animate.c
  animate.h
  compare.c
  compare.h
  composite.c
  composite.h
  conjure.c
  conjure.h
  convert.c
  convert.h
  deprecate.h
  deprecate.c
  display.c
  display.h
  drawing-wand.c
  drawing-wand.h
  identify.c
  identify.h
  import.c
  import.h
  magick-cli.c
  magick-cli.h
  magick-image.c
  magick-image.h
  magick-property.c
  magick-property.h
  magick-wand.c
  magick-wand-private.h
  method-attribute.h
  mogrify.c
  mogrify.h
  mogrify-private.h
  montage.c
  montage.h
  operation.c
  operation.h
  operation-private.h
  pixel-iterator.c
  pixel-iterator.h
  pixel-wand.c
  pixel-wand.h
  pixel-wand-private.h
  script-token.c
  script-token.h
  stream.c
  stream.h
  studio.h
  wand.c
  wand.h
  wandcli.c
  wandcli.h
  wandcli-private.h
  wand-view.c
  wand-view.h
)

set(MAGICKWAND_PUBLIC_HEADERS
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
  method-attribute.h
  mogrify.h
  montage.h
  operation.h
  pixel-iterator.h
  pixel-wand.h
  stream.h
  wandcli.h
  wand-view.h
)

if(${BUILD_SHARED_LIBS})
  add_library(${MAGICKWAND_BINARY_NAME} SHARED ${MAGICKWAND_SRC} ${MAGICK_RESOURCE_FILE_PATH})
else()
  add_library(${MAGICKWAND_BINARY_NAME} STATIC ${MAGICKWAND_SRC} ${MAGICK_RESOURCE_FILE_PATH})
endif()

set_target_properties(${MAGICKWAND_BINARY_NAME} PROPERTIES PUBLIC_HEADER "${MAGICKWAND_PUBLIC_HEADERS}")
target_include_directories(${MAGICKWAND_BINARY_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/ImageMagick-${MAGICK_MAJOR_VERSION}>
)
target_include_directories(${MAGICKWAND_BINARY_NAME} PRIVATE ${IMAGEMAGICK_BASE_PATH})
target_include_directories(${MAGICKWAND_BINARY_NAME} PRIVATE  ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS})
target_compile_options(${MAGICKWAND_BINARY_NAME} PRIVATE  ${IMAGEMAGICK_DELEGATES_CFLAGS})
# Set the MAGICKCORE_SHAREARCH_BASEDIRNAME to "config", not sure if I should add it to the <config.h>
target_compile_definitions(${MAGICKWAND_BINARY_NAME} PUBLIC ${MAGICK_BUILD_DEFINES} -D_MAGICKLIB_ -DMAGICKCORE_SHAREARCH_BASEDIRNAME=\"config\")
target_compile_options(${MAGICKWAND_BINARY_NAME} PUBLIC ${MAGICK_COMPILE_OPTIONS})
target_link_libraries(${MAGICKWAND_BINARY_NAME} PUBLIC ${MAGICKCORE_BINARY_NAME})
# If any shared libararies are in non-standard, code this paths into the binaries
set_target_properties(${MAGICKWAND_BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
# Allow the different ImageMagick libraries to find each other after installation
set_target_properties(${MAGICKWAND_BINARY_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")

install(TARGETS ${MAGICKWAND_BINARY_NAME}
  EXPORT MAGICKWAND_TARGETS
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION ${IMAGEMAGICK_INC_DIR}/MagickWand
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MagickWand.pc.in ${CMAKE_CURRENT_BINARY_DIR}/MagickWand.pc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MagickWand.pc.in ${CMAKE_CURRENT_BINARY_DIR}/MagickWand-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}.pc @ONLY)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/MagickWand.pc
  ${CMAKE_CURRENT_BINARY_DIR}/MagickWand-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}.pc
  DESTINATION lib
)

# Create the CMake config files
install(EXPORT MAGICKWAND_TARGETS FILE MagickWandTargets.cmake NAMESPACE ImageMagick:: DESTINATION lib/cmake)
include(CMakePackageConfigHelpers)
write_basic_package_version_file("MagickWand.cmake" COMPATIBILITY AnyNewerVersion VERSION ${MAGICK_MAJOR_VERSION})
