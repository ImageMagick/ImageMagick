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

project(MagickCore C)

set(MAGICKCORE_SRC
  MagickCore.h
  accelerate.c
  accelerate-private.h
  accelerate-kernels-private.h
  animate.c
  animate.h
  animate-private.h
  annotate.c
  annotate.h
  artifact.c
  artifact.h
  attribute.c
  attribute.h
  blob.c
  blob.h
  blob-private.h
  cache.c
  cache.h
  cache-private.h
  cache-view.c
  cache-view.h
  channel.c
  channel.h
  cipher.c
  cipher.h
  client.c
  client.h
  coder.c
  coder.h
  color.c
  color.h
  color-private.h
  colormap.c
  colormap.h
  colormap-private.h
  colorspace.c
  colorspace.h
  colorspace-private.h
  compare.c
  compare.h
  composite.c
  composite.h
  composite-private.h
  compress.c
  compress.h
  configure.c
  configure.h
  constitute.c
  constitute.h
  decorate.c
  decorate.h
  delegate.c
  delegate.h
  delegate-private.h
  deprecate.c
  deprecate.h
  display.c
  display.h
  display-private.h
  distort.c
  distort.h
  distribute-cache.c
  distribute-cache.h
  distribute-cache-private.h
  draw.c
  draw.h
  draw-private.h
  effect.c
  effect.h
  enhance.c
  enhance.h
  exception.c
  exception.h
  exception-private.h
  feature.c
  feature.h
  fourier.c
  fourier.h
  fx.c
  fx.h
  fx-private.h
  gem.c
  gem.h
  gem-private.h
  geometry.c
  geometry.h
  geometry-private.h
  histogram.c
  histogram.h
  identify.c
  identify.h
  image.c
  image.h
  image-private.h
  image-view.c
  image-view.h
  layer.c
  layer.h
  linked-list.c
  linked-list.h
  linked-list-private.h
  list.c
  list.h
  locale.c
  locale_.h
  log.c
  log.h
  magic.c
  magic.h
  magick.c
  magick-baseconfig.h
  magick-config.h
  magick-type.h
  magick.h
  matrix.c
  matrix.h
  matrix-private.h
  memory.c
  memory_.h
  memory-private.h
  method-attribute.h
  methods.h
  mime.c
  mime.h
  module.c
  module.h
  monitor.c
  monitor.h
  monitor-private.h
  montage.c
  montage.h
  morphology.c
  morphology.h
  morphology-private.h
  mutex.h
  nt-base.h
  nt-base-private.h
  nt-feature.h
  opencl.c
  opencl.h
  opencl-private.h
  option.c
  option.h
  option-private.h
  paint.c
  paint.h
  pixel.c
  pixel.h
  pixel-accessor.h
  pixel-private.h
  policy.c
  policy.h
  policy-private.h
  prepress.c
  prepress.h
  property.c
  property.h
  profile.c
  profile.h
  profile-private.h
  quantize.c
  quantize.h
  quantum.c
  quantum.h
  quantum-export.c
  quantum-import.c
  quantum-private.h
  random.c
  random_.h
  random-private.h
  registry.c
  registry.h
  resample.c
  resample.h
  resample-private.h
  resize.c
  resize.h
  resize-private.h
  resource.c
  resource_.h
  resource-private.h
  segment.c
  segment.h
  semaphore.c
  semaphore.h
  semaphore-private.h
  shear.c
  shear.h
  signature.c
  signature.h
  signature-private.h
  splay-tree.c
  splay-tree.h
  static.c
  static.h
  statistic.c
  statistic.h
  statistic-private.h
  stream.c
  stream.h
  stream-private.h
  string.c
  string_.h
  string-private.h
  studio.h
  thread.c
  thread_.h
  thread-private.h
  timer.c
  timer.h
  timer-private.h
  token.c
  token.h
  token-private.h
  transform.c
  transform.h
  threshold.c
  threshold.h
  type.c
  type.h
  utility.c
  utility.h
  utility-private.h
  version.c
  version.h
  version-private.h
  visual-effects.c
  visual-effects.h
  vision.c
  vision.h
  widget.c
  widget.h
  widget-private.h
  xml-tree.c
  xml-tree.h
  xml-tree-private.h
  xwindow.c
  xwindow.h
)

if(WIN32)
  set(MAGICKCORE_SRC ${MAGICKCORE_SRC}
    nt-base.c
    nt-base.h
    nt-base-private.h
    nt-feature.c
    nt-feature.h
  )
elseif(CYGWIN)
  set(MAGICKCORE_SRC ${MAGICKCORE_SRC}
    nt-feature.c
    nt-feature.h
  )
endif()

set(MAGICKCORE_PUBLIC_HEADERS
  MagickCore.h
  animate.h
  annotate.h
  artifact.h
  attribute.h
  blob.h
  cache.h
  cache-view.h
  channel.h
  cipher.h
  client.h
  coder.h
  color.h
  colormap.h
  colorspace.h
  compare.h
  composite.h
  compress.h
  configure.h
  constitute.h
  decorate.h
  delegate.h
  deprecate.h
  display.h
  distort.h
  distribute-cache.h
  draw.h
  effect.h
  enhance.h
  exception.h
  feature.h
  fourier.h
  fx.h
  gem.h
  geometry.h
  histogram.h
  identify.h
  image.h
  image-view.h
  layer.h
  linked-list.h
  list.h
  locale_.h
  log.h
  magic.h
  magick.h
  magick-config.h
  magick-type.h
  matrix.h
  memory_.h
  method-attribute.h
  methods.h
  mime.h
  module.h
  monitor.h
  montage.h
  morphology.h
  nt-base.h
  opencl.h
  option.h
  paint.h
  pixel.h
  pixel-accessor.h
  policy.h
  prepress.h
  profile.h
  property.h
  quantize.h
  quantum.h
  random_.h
  registry.h
  resample.h
  resize.h
  resource_.h
  segment.h
  semaphore.h
  shear.h
  signature.h
  splay-tree.h
  static.h
  statistic.h
  stream.h
  string_.h
  studio.h
  timer.h
  token.h
  transform.h
  threshold.h
  type.h
  utility.h
  vision.h
  visual-effects.h
  widget.h
  xml-tree.h
  xwindow.h
)

set(MAGICKCORE_FILTERS_SRCS
  ${FILTERS_PATH}/analyze.c
)

set(MAGICKCORE_CODERS_SRCS
  ${CODERS_PATH}/aai.c
  ${CODERS_PATH}/aai.h
  ${CODERS_PATH}/art.c
  ${CODERS_PATH}/art.h
  ${CODERS_PATH}/ashlar.c
  ${CODERS_PATH}/ashlar.h
  ${CODERS_PATH}/avs.c
  ${CODERS_PATH}/avs.h
  ${CODERS_PATH}/bayer.c
  ${CODERS_PATH}/bayer.h
  ${CODERS_PATH}/bgr.c
  ${CODERS_PATH}/bgr.h
  ${CODERS_PATH}/bmp.c
  ${CODERS_PATH}/bmp.h
  ${CODERS_PATH}/braille.c
  ${CODERS_PATH}/braille.h
  ${CODERS_PATH}/bytebuffer-private.h
  ${CODERS_PATH}/cals.c
  ${CODERS_PATH}/cals.h
  ${CODERS_PATH}/caption.c
  ${CODERS_PATH}/caption.h
  ${CODERS_PATH}/cin.c
  ${CODERS_PATH}/cin.h
  ${CODERS_PATH}/cip.c
  ${CODERS_PATH}/cip.h
  ${CODERS_PATH}/clipboard.h
  ${CODERS_PATH}/clip.c
  ${CODERS_PATH}/clip.h
  ${CODERS_PATH}/cmyk.c
  ${CODERS_PATH}/cmyk.h
  ${CODERS_PATH}/coders.h
  ${CODERS_PATH}/coders-list.h
  ${CODERS_PATH}/coders-private.h
  ${CODERS_PATH}/cube.c
  ${CODERS_PATH}/cube.h
  ${CODERS_PATH}/cut.c
  ${CODERS_PATH}/cut.h
  ${CODERS_PATH}/dcm.c
  ${CODERS_PATH}/dcm.h
  ${CODERS_PATH}/dds.c
  ${CODERS_PATH}/dds.h
  ${CODERS_PATH}/debug.c
  ${CODERS_PATH}/debug.h
  ${CODERS_PATH}/dib.c
  ${CODERS_PATH}/dib.h
  ${CODERS_PATH}/djvu.h
  ${CODERS_PATH}/dng.c
  ${CODERS_PATH}/dng.h
  ${CODERS_PATH}/dot.c
  ${CODERS_PATH}/dot.h
  ${CODERS_PATH}/dps.h
  ${CODERS_PATH}/dpx.c
  ${CODERS_PATH}/dpx.h
  ${CODERS_PATH}/emf.h
  ${CODERS_PATH}/ept.h
  ${CODERS_PATH}/exr.h
  ${CODERS_PATH}/farbfeld.c
  ${CODERS_PATH}/farbfeld.h
  ${CODERS_PATH}/fax.c
  ${CODERS_PATH}/fax.h
  ${CODERS_PATH}/fits.c
  ${CODERS_PATH}/fits.h
  ${CODERS_PATH}/fl32.c
  ${CODERS_PATH}/fl32.h
  ${CODERS_PATH}/flif.h
  ${CODERS_PATH}/fpx.h
  ${CODERS_PATH}/ftxt.h
  ${CODERS_PATH}/ftxt.c
  ${CODERS_PATH}/ghostscript-private.h
  ${CODERS_PATH}/gif.c
  ${CODERS_PATH}/gif.h
  ${CODERS_PATH}/gradient.c
  ${CODERS_PATH}/gradient.h
  ${CODERS_PATH}/gray.c
  ${CODERS_PATH}/gray.h
  ${CODERS_PATH}/hald.c
  ${CODERS_PATH}/hald.h
  ${CODERS_PATH}/hdr.c
  ${CODERS_PATH}/hdr.h
  ${CODERS_PATH}/heic.h
  ${CODERS_PATH}/histogram.c
  ${CODERS_PATH}/histogram.h
  ${CODERS_PATH}/hrz.c
  ${CODERS_PATH}/hrz.h
  ${CODERS_PATH}/html.c
  ${CODERS_PATH}/html.h
  ${CODERS_PATH}/icon.c
  ${CODERS_PATH}/icon.h
  ${CODERS_PATH}/info.c
  ${CODERS_PATH}/info.h
  ${CODERS_PATH}/inline.c
  ${CODERS_PATH}/inline.h
  ${CODERS_PATH}/ipl.c
  ${CODERS_PATH}/ipl.h
  ${CODERS_PATH}/jbig.h
  ${CODERS_PATH}/jnx.c
  ${CODERS_PATH}/jnx.h
  ${CODERS_PATH}/jp2.h
  ${CODERS_PATH}/jpeg.h
  ${CODERS_PATH}/json.c
  ${CODERS_PATH}/json.h
  ${CODERS_PATH}/jxl.h
  ${CODERS_PATH}/kernel.c
  ${CODERS_PATH}/kernel.h
  ${CODERS_PATH}/label.c
  ${CODERS_PATH}/label.h
  ${CODERS_PATH}/mac.c
  ${CODERS_PATH}/mac.h
  ${CODERS_PATH}/magick.c
  ${CODERS_PATH}/magick.h
  ${CODERS_PATH}/map.c
  ${CODERS_PATH}/map.h
  ${CODERS_PATH}/mask.c
  ${CODERS_PATH}/mask.h
  ${CODERS_PATH}/mat.c
  ${CODERS_PATH}/mat.h
  ${CODERS_PATH}/matte.c
  ${CODERS_PATH}/matte.h
  ${CODERS_PATH}/meta.c
  ${CODERS_PATH}/meta.h
  ${CODERS_PATH}/miff.c
  ${CODERS_PATH}/miff.h
  ${CODERS_PATH}/mono.c
  ${CODERS_PATH}/mono.h
  ${CODERS_PATH}/mpc.c
  ${CODERS_PATH}/mpc.h
  ${CODERS_PATH}/mpr.c
  ${CODERS_PATH}/mpr.h
  ${CODERS_PATH}/msl.c
  ${CODERS_PATH}/msl.h
  ${CODERS_PATH}/mtv.c
  ${CODERS_PATH}/mtv.h
  ${CODERS_PATH}/mvg.c
  ${CODERS_PATH}/mvg.h
  ${CODERS_PATH}/null.c
  ${CODERS_PATH}/null.h
  ${CODERS_PATH}/ora.c
  ${CODERS_PATH}/ora.h
  ${CODERS_PATH}/otb.c
  ${CODERS_PATH}/otb.h
  ${CODERS_PATH}/palm.c
  ${CODERS_PATH}/palm.h
  ${CODERS_PATH}/pango.c
  ${CODERS_PATH}/pango.h
  ${CODERS_PATH}/pattern.c
  ${CODERS_PATH}/pattern.h
  ${CODERS_PATH}/pcd.c
  ${CODERS_PATH}/pcd.h
  ${CODERS_PATH}/pcl.c
  ${CODERS_PATH}/pcl.h
  ${CODERS_PATH}/pcx.c
  ${CODERS_PATH}/pcx.h
  ${CODERS_PATH}/pdb.c
  ${CODERS_PATH}/pdb.h
  ${CODERS_PATH}/pdf.c
  ${CODERS_PATH}/pdf.h
  ${CODERS_PATH}/pes.c
  ${CODERS_PATH}/pes.h
  ${CODERS_PATH}/pgx.c
  ${CODERS_PATH}/pgx.h
  ${CODERS_PATH}/pict.c
  ${CODERS_PATH}/pict.h
  ${CODERS_PATH}/pix.c
  ${CODERS_PATH}/pix.h
  ${CODERS_PATH}/plasma.c
  ${CODERS_PATH}/plasma.h
  ${CODERS_PATH}/png.h
  ${CODERS_PATH}/pnm.c
  ${CODERS_PATH}/pnm.h
  ${CODERS_PATH}/ps2.c
  ${CODERS_PATH}/ps2.h
  ${CODERS_PATH}/ps3.c
  ${CODERS_PATH}/ps3.h
  ${CODERS_PATH}/ps.c
  ${CODERS_PATH}/psd.c
  ${CODERS_PATH}/psd.h
  ${CODERS_PATH}/psd-private.h
  ${CODERS_PATH}/ps.h
  ${CODERS_PATH}/pwp.c
  ${CODERS_PATH}/pwp.h
  ${CODERS_PATH}/qoi.c
  ${CODERS_PATH}/qoi.h
  ${CODERS_PATH}/raw.c
  ${CODERS_PATH}/raw.h
  ${CODERS_PATH}/rgb.c
  ${CODERS_PATH}/rgb.h
  ${CODERS_PATH}/rgf.c
  ${CODERS_PATH}/rgf.h
  ${CODERS_PATH}/rla.c
  ${CODERS_PATH}/rla.h
  ${CODERS_PATH}/rle.c
  ${CODERS_PATH}/rle.h
  ${CODERS_PATH}/scr.c
  ${CODERS_PATH}/screenshot.c
  ${CODERS_PATH}/screenshot.h
  ${CODERS_PATH}/scr.h
  ${CODERS_PATH}/sct.c
  ${CODERS_PATH}/sct.h
  ${CODERS_PATH}/sfw.c
  ${CODERS_PATH}/sfw.h
  ${CODERS_PATH}/sgi.c
  ${CODERS_PATH}/sgi.h
  ${CODERS_PATH}/sixel.c
  ${CODERS_PATH}/sixel.h
  ${CODERS_PATH}/stegano.c
  ${CODERS_PATH}/stegano.h
  ${CODERS_PATH}/strimg.c
  ${CODERS_PATH}/strimg.h
  ${CODERS_PATH}/sun.c
  ${CODERS_PATH}/sun.h
  ${CODERS_PATH}/svg.c
  ${CODERS_PATH}/svg.h
  ${CODERS_PATH}/tga.c
  ${CODERS_PATH}/tga.h
  ${CODERS_PATH}/thumbnail.c
  ${CODERS_PATH}/thumbnail.h
  ${CODERS_PATH}/tiff.h
  ${CODERS_PATH}/tile.c
  ${CODERS_PATH}/tile.h
  ${CODERS_PATH}/tim2.c
  ${CODERS_PATH}/tim2.h
  ${CODERS_PATH}/tim.c
  ${CODERS_PATH}/tim.h
  ${CODERS_PATH}/ttf.c
  ${CODERS_PATH}/ttf.h
  ${CODERS_PATH}/txt.c
  ${CODERS_PATH}/txt.h
  ${CODERS_PATH}/uil.c
  ${CODERS_PATH}/uil.h
  ${CODERS_PATH}/url.c
  ${CODERS_PATH}/url.h
  ${CODERS_PATH}/uyvy.c
  ${CODERS_PATH}/uyvy.h
  ${CODERS_PATH}/vicar.c
  ${CODERS_PATH}/vicar.h
  ${CODERS_PATH}/vid.c
  ${CODERS_PATH}/vid.h
  ${CODERS_PATH}/video.c
  ${CODERS_PATH}/video.h
  ${CODERS_PATH}/viff.c
  ${CODERS_PATH}/viff.h
  ${CODERS_PATH}/vips.c
  ${CODERS_PATH}/vips.h
  ${CODERS_PATH}/wbmp.c
  ${CODERS_PATH}/wbmp.h
  ${CODERS_PATH}/webp.h
  ${CODERS_PATH}/wmf.h
  ${CODERS_PATH}/wpg.c
  ${CODERS_PATH}/wpg.h
  ${CODERS_PATH}/xbm.c
  ${CODERS_PATH}/xbm.h
  ${CODERS_PATH}/xc.c
  ${CODERS_PATH}/xcf.c
  ${CODERS_PATH}/xcf.h
  ${CODERS_PATH}/xc.h
  ${CODERS_PATH}/x.h
  ${CODERS_PATH}/xpm.c
  ${CODERS_PATH}/xpm.h
  ${CODERS_PATH}/xps.c
  ${CODERS_PATH}/xps.h
  ${CODERS_PATH}/xwd.h
  ${CODERS_PATH}/yaml.c
  ${CODERS_PATH}/yaml.h
  ${CODERS_PATH}/ycbcr.c
  ${CODERS_PATH}/ycbcr.h
  ${CODERS_PATH}/yuv.c
  ${CODERS_PATH}/yuv.h
)

# Let's fill a variable with the code for the available delegates
set(MAGICKCORE_CODERS_DELEGATES_SRCS "")
if(DPS_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/dps.c)
endif()
if(DJVU_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/djvu.c)
endif()
if(OPENEXR_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/exr.c)
endif()
if(FLIF_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/flif.c)
endif()
if(FPX_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/fpx.c)
endif()
if(HEIC_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/heic.c)
endif()
if(JBIG_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/jbig.c)
endif()
if(JPEG_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/jpeg.c)
endif()
if(JXL_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/jxl.c)
endif()
if(LIBOPENJP2_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/jp2.c)
endif()
if(PNG_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/png.c)
endif()
if(TIFF_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/ept.c ${CODERS_PATH}/tiff.c)
endif()
if(WEBP_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/webp.c)
endif()
if(WINGDI32_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/clipboard.c)
  if(MSVC)
    # This is hackish, but it is what allows to build ImageMagick with meson
    # While CMake does support compiling individual files as different language, meson does not
    # and it is probably a bad feature to have anyway, this should be fixed in ImageMagick
    # This creates a separate compile target, a single object file and then uses MSVC /TP to build it
    add_library(emf OBJECT ${CODERS_PATH}/emf.c)
    target_compile_options(emf PRIVATE "/TP")
    target_include_directories(emf PRIVATE ${IMAGEMAGICK_BASE_PATH})
    target_include_directories(emf PRIVATE ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS})
    target_compile_options(emf PRIVATE ${IMAGEMAGICK_DELEGATES_CFLAGS})
    target_compile_definitions(emf PRIVATE ${MAGICK_BUILD_DEFINES} -D_MAGICKLIB_ -DMAGICKCORE_SHAREARCH_BASEDIRNAME=\"config\")
    target_compile_options(emf PRIVATE ${MAGICK_COMPILE_OPTIONS})
    list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS $<TARGET_OBJECTS:emf>)
  else()
    list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/emf.c)
  endif()
  # This is the CMake solution: emf.c uses namespaces so we need to compile as c++
  set_source_files_properties(${CODERS_PATH}/emf.c PROPERTIES LANGUAGE CXX)
endif()
if(WMF_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/wmf.c)
endif()
if(X11_DELEGATE)
  list(APPEND MAGICKCORE_CODERS_DELEGATES_SRCS ${CODERS_PATH}/x.c ${CODERS_PATH}/xwd.c)
endif()

if(${BUILD_SHARED_LIBS})
  add_library(${MAGICKCORE_BINARY_NAME} SHARED ${MAGICKCORE_SRC} ${MAGICKCORE_FILTERS_SRCS} ${MAGICKCORE_CODERS_SRCS} ${MAGICKCORE_CODERS_DELEGATES_SRCS} ${MAGICK_RESOURCE_FILE_PATH})
else()
  add_library(${MAGICKCORE_BINARY_NAME} STATIC ${MAGICKCORE_SRC} ${MAGICKCORE_FILTERS_SRCS} ${MAGICKCORE_CODERS_SRCS} ${MAGICKCORE_CODERS_DELEGATES_SRCS} ${MAGICK_RESOURCE_FILE_PATH})
endif()

# on macOS some pkgconfigs place the frameworks in LDFLAGS
# CMake has special handling for macOS frameworks but only in LIBRARIES
if(APPLE)
  set(I 0)
  list(LENGTH IMAGEMAGICK_DELEGATES_LDFLAGS LEN)
  while(${I} LESS ${LEN} )
    list(GET IMAGEMAGICK_DELEGATES_LDFLAGS ${I} EL)
    if(${EL} STREQUAL "-framework")
      MATH(EXPR I "${I}+1")
      list(GET IMAGEMAGICK_DELEGATES_LDFLAGS ${I} EL)
      target_link_libraries(${MAGICKCORE_BINARY_NAME} PUBLIC "-framework ${EL}")
    else()
      target_link_options(${MAGICKCORE_BINARY_NAME} PUBLIC ${EL})
    endif()
    MATH(EXPR I "${I}+1")
    unset(EL)
  endwhile()
else()
  target_link_options(${MAGICKCORE_BINARY_NAME} PUBLIC ${IMAGEMAGICK_DELEGATES_LDFLAGS})
endif()

set_target_properties(${MAGICKCORE_BINARY_NAME} PROPERTIES PUBLIC_HEADER "${MAGICKCORE_PUBLIC_HEADERS}")
target_include_directories(${MAGICKCORE_BINARY_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/ImageMagick-${MAGICK_MAJOR_VERSION}>
)
target_include_directories(${MAGICKCORE_BINARY_NAME} PRIVATE ${IMAGEMAGICK_BASE_PATH})
target_include_directories(${MAGICKCORE_BINARY_NAME} PRIVATE ${IMAGEMAGICK_DELEGATES_INCLUDE_DIRS})
target_compile_options(${MAGICKCORE_BINARY_NAME} PRIVATE ${IMAGEMAGICK_DELEGATES_CFLAGS})
target_link_directories(${MAGICKCORE_BINARY_NAME} PUBLIC ${IMAGEMAGICK_DELEGATES_LIBRARY_DIRS})
target_link_libraries(${MAGICKCORE_BINARY_NAME} PUBLIC ${IMAGEMAGICK_DELEGATES_LIBRARIES})
#TODO should we make a test to see if we need to link -lm ??
if(UNIX)
  target_link_libraries(${MAGICKCORE_BINARY_NAME} PUBLIC -lm)
  set(MATH_LIBS "-lm")
endif()
# Set the MAGICKCORE_SHAREARCH_BASEDIRNAME to "config", not sure if I should add it to the <config.h>
target_compile_definitions(${MAGICKCORE_BINARY_NAME} PUBLIC ${MAGICK_BUILD_DEFINES} -D_MAGICKLIB_ -DMAGICKCORE_SHAREARCH_BASEDIRNAME=\"config\")
target_compile_options(${MAGICKCORE_BINARY_NAME} PUBLIC ${MAGICK_COMPILE_OPTIONS})
# If any shared libararies are in non-standard, code this paths into the binaries
set_target_properties(${MAGICKCORE_BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
# Allow the different ImageMagick libraries to find each other after installation
set_target_properties(${MAGICKCORE_BINARY_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")

install(TARGETS ${MAGICKCORE_BINARY_NAME}
  EXPORT MAGICKCORE_TARGETS
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION ${IMAGEMAGICK_INC_DIR}/MagickCore
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/magick-baseconfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
  DESTINATION ${IMAGEMAGICK_INC_DIR}/MagickCore
)

# Create the .pc files (alas these lack the dependencies)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MagickCore.pc.in ${CMAKE_CURRENT_BINARY_DIR}/MagickCore.pc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ImageMagick.pc.in ${CMAKE_CURRENT_BINARY_DIR}/ImageMagick.pc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MagickCore.pc.in ${CMAKE_CURRENT_BINARY_DIR}/MagickCore-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}.pc @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ImageMagick.pc.in ${CMAKE_CURRENT_BINARY_DIR}/ImageMagick-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}.pc @ONLY)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/MagickCore.pc
  ${CMAKE_CURRENT_BINARY_DIR}/MagickCore-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}.pc
  ${CMAKE_CURRENT_BINARY_DIR}/ImageMagick.pc
  ${CMAKE_CURRENT_BINARY_DIR}/ImageMagick-${MAGICK_MAJOR_VERSION}.${MAGICK_ABI_SUFFIX}.pc
  DESTINATION lib
)

# Create the CMake config files
install(EXPORT MAGICKCORE_TARGETS FILE MagickCoreTargets.cmake NAMESPACE ImageMagick:: DESTINATION lib/cmake)
include(CMakePackageConfigHelpers)
write_basic_package_version_file("MagickCore.cmake" COMPATIBILITY AnyNewerVersion VERSION ${MAGICK_MAJOR_VERSION})
