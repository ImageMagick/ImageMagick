#  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
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
#  Makefile for building MagickWand API.
#

wandincdir = $(INCLUDE_PATH)/wand

WAND_SOURCES = \
	wand/MagickWand.h \
	wand/animate.c \
	wand/animate.h \
	wand/compare.c \
	wand/compare.h \
	wand/composite.c \
	wand/composite.h \
	wand/conjure.c \
	wand/conjure.h \
	wand/convert.c \
	wand/convert.h \
	wand/deprecate.h \
	wand/deprecate.c \
	wand/display.c \
	wand/display.h \
	wand/drawing-wand.c \
	wand/drawing-wand.h \
	wand/identify.c \
	wand/identify.h \
	wand/import.c \
	wand/import.h \
	wand/magick-image.c \
	wand/magick-image.h \
	wand/magick-property.c \
	wand/magick-property.h \
	wand/magick-wand.c \
	wand/magick-wand.h \
	wand/magick-wand-private.h \
	wand/method-attribute.h \
	wand/mogrify.c \
	wand/mogrify.h \
	wand/mogrify-private.h \
	wand/montage.c \
	wand/montage.h \
	wand/pixel-iterator.c \
	wand/pixel-iterator.h \
	wand/pixel-wand.c \
	wand/pixel-wand.h \
	wand/pixel-wand-private.h \
	wand/stream.c \
	wand/stream.h \
	wand/studio.h \
	wand/wand.c \
	wand/wand.h \
	wand/wand-view.c \
	wand/wand-view.h 

WAND_INCLUDE_HDRS = \
	wand/MagickWand.h \
	wand/animate.h \
	wand/compare.h \
	wand/composite.h \
	wand/conjure.h \
	wand/convert.h \
	wand/deprecate.h \
	wand/display.h \
	wand/drawing-wand.h \
	wand/identify.h \
	wand/import.h \
	wand/magick-image.h \
	wand/magick-property.h \
	wand/magick-wand.h \
	wand/magick_wand.h \
	wand/method-attribute.h \
	wand/mogrify.h \
	wand/montage.h \
	wand/pixel-iterator.h \
	wand/pixel-wand.h \
	wand/stream.h \
	wand/wand-view.h

WAND_NOINST_HDRS = \
	wand/mogrify-private.h \
	wand/magick-wand-private.h \
	wand/pixel-wand-private.h \
	wand/studio.h \
	wand/wand.h

# Headers which are installed
wandinc_HEADERS = \
	$(WAND_INCLUDE_HDRS)

WAND_BIN_SCRPTS = \
	wand/Wand-config \
	wand/MagickWand-config

WAND_PKGCONFIG = \
	wand/MagickWand.pc \
	wand/Wand.pc \
	wand/MagickWand-@MAGICK_ABI_SUFFIX@.pc \
	wand/Wand-@MAGICK_ABI_SUFFIX@.pc

WAND_MANS = \
	wand/MagickWand-config.1 \
	wand/Wand-config.1

WAND_BUILT_SRCS = 

MAGICKWAND_LIBS = wand/libMagickWand-@MAGICK_ABI_SUFFIX@.la

wand_libMagickWand_@MAGICK_ABI_SUFFIX@_la_SOURCES = $(WAND_SOURCES)
wand_libMagickWand_@MAGICK_ABI_SUFFIX@_la_CPPFLAGS = $(LIBRARY_EXTRA_CPPFLAGS)

if HAVE_LD_VERSION_SCRIPT
wand_libMagickWand_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION =  -Wl,--version-script=$(top_srcdir)/wand/libMagickWand.map
else
wand_libMagickWand_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION = -export-symbols-regex ".*"
endif

wand_libMagickWand_@MAGICK_ABI_SUFFIX@_la_LDFLAGS = -no-undefined \
  $(wand_libMagickWand_la_LDFLAGS_VERSION) $(MAGICK_LT_RELEASE_OPTS) \
	-version-info \
	$(MAGICK_LIBRARY_CURRENT):$(MAGICK_LIBRARY_REVISION):$(MAGICK_LIBRARY_AGE)
wand_libMagickWand_@MAGICK_ABI_SUFFIX@_la_LIBADD = $(MAGICKCORE_LIBS) $(X11_LIBS) $(GOMP_LIBS) $(MATH_LIBS)

WAND_EXTRA_DIST = \
	wand/ChangeLog \
	wand/libMagickWand.map \
	wand/MagickWand-config.1 \
	wand/MagickWand.pc.in \
	wand/Wand-config.1 \
	wand/Wand.pc.in

WAND_CLEANFILES =
