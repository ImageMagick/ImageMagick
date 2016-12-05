#  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for building the MagickCore API.
# 

MagickCoreincdir = $(INCLUDE_PATH)/MagickCore
MagickCoreincarchdir = $(INCLUDEARCH_PATH)/MagickCore

# Headers which are installed
MagickCoreinc_HEADERS = \
	$(MAGICKCORE_INCLUDE_HDRS)
MagickCoreincarch_HEADERS = \
	$(MAGICKCORE_INCLUDEARCH_HDRS)

MAGICKCORE_BIN_SCRPTS = \
  MagickCore/MagickCore-config

MAGICKCORE_PKGCONFIG = \
  MagickCore/ImageMagick.pc \
	MagickCore/MagickCore.pc \
	MagickCore/ImageMagick-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.pc \
	MagickCore/MagickCore-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.pc 

OSX_GCOV_LDFLAG = @OSX_GCOV_LDFLAG@

MAGICKCORE_MANS = \
  MagickCore/MagickCore-config.1

MAGICKCORE_LIBS = MagickCore/libMagickCore-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.la

if WITH_MODULES
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_SOURCES = $(MAGICKCORE_BASE_SRCS) $(MAGICKCORE_PLATFORM_SRCS)
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LIBADD = $(MAGICK_DEP_LIBS)
else
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_SOURCES = $(MAGICKCORE_BASE_SRCS) $(MAGICKCORE_PLATFORM_SRCS) $(MAGICKCORE_CODER_SRCS) $(MAGICKCORE_FILTER_SRCS)
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LIBADD = $(MAGICK_DEP_LIBS)
endif # WITH_MODULES
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_CPPFLAGS = $(AM_CPPFLAGS) $(LIBRARY_EXTRA_CPPFLAGS)

if HAVE_LD_VERSION_SCRIPT
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION =  -Wl,--version-script=$(top_srcdir)/MagickCore/libMagickCore.map
else
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION = -export-symbols-regex ".*"
endif

MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS = -no-undefined \
  $(MagickCore_libMagickCore_la_LDFLAGS_VERSION) \
  $(OSX_GCOV_LDFLAG) $(MAGICK_LT_RELEASE_OPTS) -version-info \
  $(MAGICK_LIBRARY_CURRENT):$(MAGICK_LIBRARY_REVISION):$(MAGICK_LIBRARY_AGE)
MagickCore_libMagickCore_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_DEPENDENCIES =

# Library base sources
MAGICKCORE_BASE_SRCS = \
	MagickCore/MagickCore.h \
	MagickCore/accelerate.c \
	MagickCore/accelerate-private.h \
	MagickCore/accelerate-kernels-private.h \
	MagickCore/animate.c \
	MagickCore/animate.h \
	MagickCore/animate-private.h \
	MagickCore/annotate.c \
	MagickCore/annotate.h \
	MagickCore/artifact.c \
	MagickCore/artifact.h \
	MagickCore/attribute.c \
	MagickCore/attribute.h \
	MagickCore/blob.c \
	MagickCore/blob.h \
	MagickCore/blob-private.h \
	MagickCore/cache.c \
	MagickCore/cache.h \
	MagickCore/cache-private.h \
	MagickCore/cache-view.c \
	MagickCore/cache-view.h \
	MagickCore/channel.c \
	MagickCore/channel.h \
	MagickCore/cipher.c \
	MagickCore/cipher.h \
	MagickCore/client.c \
	MagickCore/client.h \
	MagickCore/coder.c \
	MagickCore/coder.h \
	MagickCore/color.c \
	MagickCore/color.h \
	MagickCore/color-private.h \
	MagickCore/colormap.c \
	MagickCore/colormap.h \
	MagickCore/colormap-private.h \
	MagickCore/colorspace.c \
	MagickCore/colorspace.h \
	MagickCore/colorspace-private.h \
	MagickCore/compare.c \
	MagickCore/compare.h \
	MagickCore/composite.c \
	MagickCore/composite.h \
	MagickCore/composite-private.h \
	MagickCore/compress.c \
	MagickCore/compress.h \
	MagickCore/configure.c \
	MagickCore/configure.h \
	MagickCore/constitute.c \
	MagickCore/constitute.h \
	MagickCore/decorate.c \
	MagickCore/decorate.h \
	MagickCore/delegate.c \
	MagickCore/delegate.h \
	MagickCore/delegate-private.h \
	MagickCore/deprecate.c \
	MagickCore/deprecate.h \
	MagickCore/display.c \
	MagickCore/display.h \
	MagickCore/display-private.h \
	MagickCore/distort.c \
	MagickCore/distort.h \
	MagickCore/distribute-cache.c \
	MagickCore/distribute-cache.h \
	MagickCore/distribute-cache-private.h \
	MagickCore/draw.c \
	MagickCore/draw.h \
	MagickCore/draw-private.h \
	MagickCore/effect.c \
	MagickCore/effect.h \
	MagickCore/enhance.c \
	MagickCore/enhance.h \
	MagickCore/exception.c \
	MagickCore/exception.h \
	MagickCore/exception-private.h \
	MagickCore/feature.c \
	MagickCore/feature.h \
	MagickCore/fourier.c \
	MagickCore/fourier.h \
	MagickCore/fx.c \
	MagickCore/fx.h \
	MagickCore/fx-private.h \
	MagickCore/gem.c \
	MagickCore/gem.h \
	MagickCore/gem-private.h \
	MagickCore/geometry.c \
	MagickCore/geometry.h \
	MagickCore/histogram.c \
	MagickCore/histogram.h \
	MagickCore/identify.c \
	MagickCore/identify.h \
	MagickCore/image.c \
	MagickCore/image.h \
	MagickCore/image-private.h \
	MagickCore/image-view.c \
	MagickCore/image-view.h \
	MagickCore/layer.c \
	MagickCore/layer.h \
	MagickCore/linked-list.c \
	MagickCore/linked-list.h \
	MagickCore/list.c \
	MagickCore/list.h \
	MagickCore/locale.c \
	MagickCore/locale_.h \
	MagickCore/log.c \
	MagickCore/log.h \
	MagickCore/magic.c \
	MagickCore/magic.h \
	MagickCore/magick.c \
	MagickCore/magick-baseconfig.h \
	MagickCore/magick-config.h \
	MagickCore/magick-type.h \
	MagickCore/magick.h \
	MagickCore/matrix.c \
	MagickCore/matrix.h \
	MagickCore/matrix-private.h \
	MagickCore/memory.c \
	MagickCore/memory_.h \
	MagickCore/memory-private.h \
	MagickCore/method-attribute.h \
	MagickCore/methods.h \
	MagickCore/mime.c \
	MagickCore/mime.h \
	MagickCore/module.c \
	MagickCore/module.h \
	MagickCore/monitor.c \
	MagickCore/monitor.h \
	MagickCore/monitor-private.h \
	MagickCore/montage.c \
	MagickCore/montage.h \
	MagickCore/morphology.c \
	MagickCore/morphology.h \
	MagickCore/morphology-private.h \
	MagickCore/nt-base.h \
  MagickCore/nt-base-private.h \
	MagickCore/nt-feature.h \
	MagickCore/opencl.c \
	MagickCore/opencl.h \
	MagickCore/opencl-private.h \
	MagickCore/option.c \
	MagickCore/option.h \
	MagickCore/option-private.h \
	MagickCore/paint.c \
	MagickCore/paint.h \
	MagickCore/pixel.c \
	MagickCore/pixel.h \
	MagickCore/pixel-accessor.h \
	MagickCore/pixel-private.h \
	MagickCore/policy.c \
	MagickCore/policy.h \
	MagickCore/prepress.c \
	MagickCore/prepress.h \
	MagickCore/property.c \
	MagickCore/property.h \
	MagickCore/profile.c \
	MagickCore/profile.h \
	MagickCore/profile-private.h \
	MagickCore/quantize.c \
	MagickCore/quantize.h \
	MagickCore/quantum.c \
	MagickCore/quantum.h \
	MagickCore/quantum-export.c \
	MagickCore/quantum-import.c \
	MagickCore/quantum-private.h \
	MagickCore/random.c \
	MagickCore/random_.h \
	MagickCore/random-private.h \
	MagickCore/registry.c \
	MagickCore/registry.h \
	MagickCore/resample.c \
	MagickCore/resample.h \
	MagickCore/resample-private.h \
	MagickCore/resize.c \
	MagickCore/resize.h \
	MagickCore/resize-private.h \
	MagickCore/resource.c \
	MagickCore/resource_.h \
	MagickCore/resource-private.h \
	MagickCore/segment.c \
	MagickCore/segment.h \
	MagickCore/semaphore.c \
	MagickCore/semaphore.h \
	MagickCore/semaphore-private.h \
	MagickCore/shear.c \
	MagickCore/shear.h \
	MagickCore/signature.c \
	MagickCore/signature.h \
	MagickCore/signature-private.h \
	MagickCore/splay-tree.c \
	MagickCore/splay-tree.h \
	MagickCore/static.c \
	MagickCore/static.h \
	MagickCore/statistic.c \
	MagickCore/statistic.h \
	MagickCore/stream.c \
	MagickCore/stream.h \
	MagickCore/stream-private.h \
	MagickCore/string.c \
	MagickCore/string_.h \
	MagickCore/string-private.h \
	MagickCore/studio.h \
	MagickCore/thread.c \
	MagickCore/thread_.h \
	MagickCore/thread-private.h \
	MagickCore/timer.c \
	MagickCore/timer.h \
	MagickCore/token.c \
	MagickCore/token.h \
	MagickCore/token-private.h \
	MagickCore/transform.c \
	MagickCore/transform.h \
	MagickCore/threshold.c \
	MagickCore/threshold.h \
	MagickCore/type.c \
	MagickCore/type.h \
	MagickCore/utility.c \
	MagickCore/utility.h \
	MagickCore/utility-private.h \
	MagickCore/version.c \
	MagickCore/version.h \
	MagickCore/version-private.h \
	MagickCore/vision.c \
	MagickCore/vision.h \
	MagickCore/widget.c \
	MagickCore/widget.h \
	MagickCore/widget-private.h \
	MagickCore/xml-tree.c \
	MagickCore/xml-tree.h \
	MagickCore/xml-tree-private.h \
	MagickCore/xwindow.c \
	MagickCore/xwindow.h 

if WIN32_NATIVE_BUILD
MAGICKCORE_PLATFORM_SRCS = \
  MagickCore/nt-base.c \
  MagickCore/nt-base.h \
  MagickCore/nt-base-private.h \
  MagickCore/nt-feature.c \
  MagickCore/nt-feature.h
else
if CYGWIN_BUILD
MAGICKCORE_PLATFORM_SRCS = \
  MagickCore/nt-feature.c \
  MagickCore/nt-feature.h
else
MAGICKCORE_PLATFORM_SRCS =
endif # if CYGWIN_BUILD
endif # if WIN32_NATIVE_BUILD

MAGICKCORE_INCLUDE_HDRS = \
	MagickCore/MagickCore.h \
	MagickCore/animate.h \
	MagickCore/annotate.h \
	MagickCore/artifact.h \
	MagickCore/attribute.h \
	MagickCore/blob.h \
	MagickCore/cache.h \
	MagickCore/cache-view.h \
	MagickCore/channel.h \
	MagickCore/cipher.h \
	MagickCore/client.h \
	MagickCore/coder.h \
	MagickCore/color.h \
	MagickCore/colormap.h \
	MagickCore/colorspace.h \
	MagickCore/compare.h \
	MagickCore/composite.h \
	MagickCore/compress.h \
	MagickCore/configure.h \
	MagickCore/constitute.h \
	MagickCore/decorate.h \
	MagickCore/delegate.h \
	MagickCore/deprecate.h \
	MagickCore/display.h \
	MagickCore/distort.h \
	MagickCore/distribute-cache.h \
	MagickCore/draw.h \
	MagickCore/effect.h \
	MagickCore/enhance.h \
	MagickCore/exception.h \
	MagickCore/feature.h \
	MagickCore/fourier.h \
	MagickCore/fx.h \
	MagickCore/gem.h \
	MagickCore/geometry.h \
	MagickCore/histogram.h \
	MagickCore/identify.h \
	MagickCore/image.h \
	MagickCore/image-view.h \
	MagickCore/layer.h \
	MagickCore/linked-list.h \
	MagickCore/list.h \
	MagickCore/locale_.h \
	MagickCore/log.h \
	MagickCore/magic.h \
	MagickCore/magick.h \
	MagickCore/magick-config.h \
	MagickCore/magick-type.h \
	MagickCore/matrix.h \
	MagickCore/memory_.h \
	MagickCore/method-attribute.h \
	MagickCore/methods.h \
	MagickCore/mime.h \
	MagickCore/module.h \
	MagickCore/monitor.h \
	MagickCore/montage.h \
	MagickCore/morphology.h \
	MagickCore/nt-base.h \
	MagickCore/opencl.h \
	MagickCore/option.h \
	MagickCore/paint.h \
	MagickCore/pixel.h \
	MagickCore/pixel-accessor.h \
	MagickCore/policy.h \
	MagickCore/prepress.h \
	MagickCore/profile.h \
	MagickCore/property.h \
	MagickCore/quantize.h \
	MagickCore/quantum.h \
	MagickCore/random_.h \
	MagickCore/registry.h \
	MagickCore/resample.h \
	MagickCore/resize.h \
	MagickCore/resource_.h \
	MagickCore/segment.h \
	MagickCore/semaphore.h \
	MagickCore/shear.h \
	MagickCore/signature.h \
	MagickCore/splay-tree.h \
	MagickCore/statistic.h \
	MagickCore/stream.h \
	MagickCore/string_.h \
	MagickCore/studio.h \
	MagickCore/timer.h \
	MagickCore/token.h \
	MagickCore/transform.h \
	MagickCore/threshold.h \
	MagickCore/type.h \
	MagickCore/utility.h \
	MagickCore/version.h \
	MagickCore/vision.h \
	MagickCore/widget.h \
	MagickCore/xml-tree.h \
	MagickCore/xwindow.h

MAGICKCORE_NOINST_HDRS = \
	MagickCore/accelerate-private.h \
	MagickCore/accelerate-kernels-private.h \
	MagickCore/animate-private.h \
	MagickCore/annotate-private.h \
	MagickCore/blob-private.h \
 	MagickCore/cache-private.h \
	MagickCore/cache-private.h \
 	MagickCore/coder-private.h \
	MagickCore/colormap-private.h \
 	MagickCore/color-private.h \
	MagickCore/color-private.h \
	MagickCore/colorspace-private.h \
	MagickCore/composite-private.h \
 	MagickCore/configure-private.h \
 	MagickCore/constitute-private.h \
 	MagickCore/delegate-private.h \
	MagickCore/delegate-private.h \
	MagickCore/display-private.h \
	MagickCore/distribute-cache-private.h \
	MagickCore/draw-private.h \
	MagickCore/exception-private.h \
	MagickCore/fx-private.h \
	MagickCore/gem-private.h \
	MagickCore/image-private.h \
 	MagickCore/locale-private.h \
 	MagickCore/log-private.h \
 	MagickCore/magick-private.h \
 	MagickCore/magic-private.h \
	MagickCore/matrix-private.h \
	MagickCore/memory-private.h \
 	MagickCore/methods-private.h \
 	MagickCore/mime-private.h \
	MagickCore/mime-private.h \
 	MagickCore/module-private.h \
	MagickCore/monitor-private.h \
	MagickCore/morphology-private.h \
	MagickCore/nt-base.h \
	MagickCore/nt-feature.h \
	MagickCore/opencl-private.h \
	MagickCore/option-private.h \
 	MagickCore/pixel-private.h \
 	MagickCore/policy-private.h \
 	MagickCore/profile-private.h \
	MagickCore/quantum-private.h \
 	MagickCore/random_-private.h \
	MagickCore/random-private.h \
 	MagickCore/registry-private.h \
	MagickCore/resample-private.h \
	MagickCore/resize-private.h \
 	MagickCore/resource-private.h \
	MagickCore/resource-private.h \
 	MagickCore/semaphore-private.h \
	MagickCore/semaphore-private.h \
	MagickCore/signature-private.h \
	MagickCore/static.h \
	MagickCore/stream-private.h \
	MagickCore/string-private.h \
	MagickCore/thread_.h \
	MagickCore/thread-private.h \
	MagickCore/token-private.h \
	MagickCore/transform-private.h \
 	MagickCore/type-private.h \
	MagickCore/utility-private.h \
	MagickCore/version-private.h \
 	MagickCore/widget-private.h \
	MagickCore/xml-tree-private.h \
	MagickCore/xwindow-private.h

MAGICKCORE_INCLUDEARCH_HDRS = \
	MagickCore/magick-baseconfig.h

MAGICKCORE_EXTRA_DIST = \
	MagickCore/MagickCore-config.in \
	$(MAGICKCORE_MANS) \
	MagickCore/ImageMagick.pc.in \
	MagickCore/MagickCore.pc.in \
	MagickCore/libMagickCore.map \
	MagickCore/nt-base.c \
	MagickCore/nt-feature.c
