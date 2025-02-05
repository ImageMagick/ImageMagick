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

project(ImageMagick)

set(MAGICK_CONFIG_FILES
  configure.xml
  delegates.xml
  ImageMagick.rdf
  MagickCore.dox
  MagickWand.dox
  Magick++.dox
  type.xml
)

set(MAGICK_INSTALLED_CONFIG_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/delegates.xml
  ${CMAKE_CURRENT_BINARY_DIR}/type.xml
  ${CMAKE_CURRENT_BINARY_DIR}/policy.xml
  colors.xml
  log.xml
  mime.xml
  quantization-table.xml
  thresholds.xml
)

set(MAGICK_APPLE_FONT_DIR "/Library/Fonts;/System/Library/Templates/Data/Library/Fonts;/System/Library/Fonts/Supplemental;/System/Library/Fonts/" CACHE STRING "Apple fonts directory")
foreach(DIR ${MAGICK_APPLE_FONT_DIR})
  if(EXISTS ${DIR}/Arial.ttf)
    message("Apple fonts in ${DIR}")
    list(APPEND TYPE_INCLUDE_FILES "<include file=\"type-apple.xml\" />\n")
    list(APPEND MAGICK_CONFIG_FILES type-apple.xml)
    list(APPEND MAGICK_INSTALLED_CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/type-apple.xml)
    set(apple_font_dir ${DIR})
    break()
  endif()
endforeach()

set(MAGICK_DEJAVU_FONT_DIR "/usr/share/fonts/dejavu;/usr/share/dejavu/fonts;/usr/share/fonts/truetype/dejavu/" CACHE STRING "Deja Vu fonts directory")
foreach(DIR ${MAGICK_DEJAVU_FONT_DIR})
  if(EXISTS ${DIR}/DejaVuSerif.ttf)
    message("DejaVu fonts in ${DIR}")
    list(APPEND TYPE_INCLUDE_FILES "<include file=\"type-dejavu.xml\" />\n")
    list(APPEND MAGICK_CONFIG_FILES type-dejavu.xml)
    list(APPEND MAGICK_INSTALLED_CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/type-dejavu.xml)
    set(dejavu_font_dir ${DIR})
    break()
  endif()
endforeach()

# TODO configure does a sed of the ghostscripts names
if(WIN32)
  set(GHOSTSCRIPT_FONT_DIR "C:/Program Files/gs/fonts;C:/Program Files(x86\)/gs/fonts;C:/gs/fonts" CACHE STRING "Ghostscript Fonts directory")
else()
  set(GHOSTSCRIPT_FONT_DIR "/usr/share/ghostscript/fonts;/usr/share/fonts/default/Type1;/usr/share/ghostscript/fonts;/usr/share/fonts/ghostscript;/usr/share/fonts/type1/gsfonts;/opt/local/share/ghostscript/fonts;/sw/share/ghostscript/fonts;/System/Library/Frameworks/Ghostscript.framework/Resources/fonts" CACHE STRING "Ghostscript Fonts directory")
endif()
foreach(DIR ${GHOSTSCRIPT_FONT_DIR})
  if(EXISTS ${DIR}/a010013l.pfb)
    message("Ghostscript fonts in ${DIR}")
    list(APPEND TYPE_INCLUDE_FILES "<include file=\"type-ghostscript.xml\" />\n")
    list(APPEND MAGICK_CONFIG_FILES type-ghostscript.xml)
    list(APPEND MAGICK_INSTALLED_CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/type-ghostscript.xml)
    set(ghostscript_font_dir ${DIR})
    break()
  endif()
endforeach(DIR)

# URW-base35 fonts
set(MAGICK_URW_FONT_DIR "/usr/share/fonts/urw-base35/" CACHE STRING "URW-base35 fonts directory")
foreach(DIR ${MAGICK_URW_FONT_DIR})
  if(EXISTS ${DIR})
    message("URW-base35 fonts in ${DIR}")
    list(APPEND TYPE_INCLUDE_FILES "<include file=\"type-urw-base35.xml\" />\n")
    list(APPEND MAGICK_CONFIG_FILES type-urw-base35.xml)
    list(APPEND MAGICK_INSTALLED_CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/type-urw-base35.xml)
    set(urw_base35_font_dir ${DIR})
    break()
  endif()
endforeach()

# URW-base35 Type1 fonts
set(MAGICK_URW_TYPE1_FONT_DIR "/usr/share/fonts/type1/urw-base35/" CACHE STRING "URW-base35 Type1 fonts directory")
foreach(DIR ${MAGICK_URW_TYPE1_FONT_DIR})
  if(EXISTS ${DIR})
    message("URW-base35 Type1 fonts in ${DIR}")
    list(APPEND TYPE_INCLUDE_FILES "<include file=\"type-urw-base35-type1.xml\" />\n")
    list(APPEND MAGICK_CONFIG_FILES type-urw-base35-type1.xml)
    list(APPEND MAGICK_INSTALLED_CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/type-urw-base35-type1.xml)
    set(urw_base35_type1_font_dir ${DIR})
    break()
  endif()
endforeach()

# Truetype fonts
set(MAGICK_TT_FONT_DIR "/usr/X11R6/lib/X11/fonts/truetype/;/usr/X11R7/lib/X11/fonts/truetype/;/usr/share/fonts/msttcore/;/usr/share/fonts/microsoft/;/usr/share/fonts/truetype/msttcorefonts/;/usr/share/fonts/truetype/" CACHE STRING "Truetype fonts directory")
foreach(DIR ${MAGICK_TT_FONT_DIR})
  if(IS_DIRECTORY ${DIR})
    message("Truetype fonts in ${DIR}")
    list(APPEND TYPE_INCLUDE_FILES "<include file=\"type-windows.xml\" />\n")
    list(APPEND MAGICK_CONFIG_FILES type-windows.xml)
    list(APPEND MAGICK_INSTALLED_CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/type-windows.xml)
    set(windows_font_dir ${DIR})
    break()
  endif()
endforeach()

set(CC ${CMAKE_C_COMPILER})
set(CXX ${CMAKE_CXX_COMPILER})
set(CFLAGS ${MAGICK_COMPILE_OPTIONS} ${IMAGEMAGICK_DELEGATES_CFLAGS})
set(CPPFLAGS ${MAGICK_COMPILE_OPTIONS} ${IMAGEMAGICK_DELEGATES_CFLAGS})
set(CXXFLAGS ${MAGICK_COMPILE_OPTIONS} ${IMAGEMAGICK_DELEGATES_CFLAGS})
set(LDFLAGS ${IMAGEMAGICK_DELEGATES_LDFLAGS})
set(QuantumDepth ${MAGICKCORE_QUANTUM_DEPTH})
set(PREFIX ${CMAKE_INSTALL_PREFIX})
string(REPLACE ";" "  " type_include_files "${TYPE_INCLUDE_FILES}")

foreach(CONF_FILE IN LISTS MAGICK_CONFIG_FILES)
  configure_file(${CONF_FILE}.in ${CONF_FILE} @ONLY)
endforeach()

set(MAGICK_SECURITY_POLICY "open" CACHE STRING "Default security policy")
set_property(CACHE MAGICK_SECURITY_POLICY PROPERTY STRINGS "open;limited;secure;websafe")
configure_file(policy-${MAGICK_SECURITY_POLICY}.xml policy.xml @ONLY)

install(FILES
  ${MAGICK_INSTALLED_CONFIG_FILES}
  DESTINATION etc/ImageMagick-${MAGICK_MAJOR_VERSION}
)

install(FILES
  english.xml
  francais.xml
  locale.xml
  DESTINATION share/ImageMagick-${MAGICK_MAJOR_VERSION}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/configure.xml
  DESTINATION lib/ImageMagick-${MAGICK_MAJOR_VERSION}/config-${MAGICK_ABI_SUFFIX}
)
