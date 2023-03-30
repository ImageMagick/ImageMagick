#  Copyright 1999 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for building MagickWand API.
#

MagickWandincdir = $(INCLUDE_PATH)/MagickWand

MAGICKWAND_CPPFLAGS = $(AM_CPPFLAGS)

MAGICKWAND_SOURCES = \
  MagickWand/MagickWand.h \
  MagickWand/animate.c \
  MagickWand/animate.h \
  MagickWand/compare.c \
  MagickWand/compare.h \
  MagickWand/composite.c \
  MagickWand/composite.h \
  MagickWand/conjure.c \
  MagickWand/conjure.h \
  MagickWand/convert.c \
  MagickWand/convert.h \
  MagickWand/deprecate.h \
  MagickWand/deprecate.c \
  MagickWand/display.c \
  MagickWand/display.h \
  MagickWand/drawing-wand.c \
  MagickWand/drawing-wand.h \
  MagickWand/identify.c \
  MagickWand/identify.h \
  MagickWand/import.c \
  MagickWand/import.h \
  MagickWand/magick-cli.c \
  MagickWand/magick-cli.h \
  MagickWand/magick-image.c \
  MagickWand/magick-image.h \
  MagickWand/magick-property.c \
  MagickWand/magick-property.h \
  MagickWand/magick-wand.c \
  MagickWand/magick-wand-private.h \
  MagickWand/method-attribute.h \
  MagickWand/mogrify.c \
  MagickWand/mogrify.h \
  MagickWand/mogrify-private.h \
  MagickWand/montage.c \
  MagickWand/montage.h \
  MagickWand/operation.c \
  MagickWand/operation.h \
  MagickWand/operation-private.h \
  MagickWand/pixel-iterator.c \
  MagickWand/pixel-iterator.h \
  MagickWand/pixel-wand.c \
  MagickWand/pixel-wand.h \
  MagickWand/pixel-wand-private.h \
  MagickWand/script-token.c \
  MagickWand/script-token.h \
  MagickWand/stream.c \
  MagickWand/stream.h \
  MagickWand/studio.h \
  MagickWand/wand.c \
  MagickWand/wand.h \
  MagickWand/wandcli.c \
  MagickWand/wandcli.h \
  MagickWand/wandcli-private.h \
  MagickWand/wand-view.c \
  MagickWand/wand-view.h

MAGICKWAND_INCLUDE_HDRS = \
  MagickWand/MagickWand.h \
  MagickWand/animate.h \
  MagickWand/compare.h \
  MagickWand/composite.h \
  MagickWand/conjure.h \
  MagickWand/convert.h \
  MagickWand/deprecate.h \
  MagickWand/display.h \
  MagickWand/drawing-wand.h \
  MagickWand/identify.h \
  MagickWand/import.h \
  MagickWand/magick-cli.h \
  MagickWand/magick-image.h \
  MagickWand/magick-property.h \
  MagickWand/method-attribute.h \
  MagickWand/mogrify.h \
  MagickWand/montage.h \
  MagickWand/operation.h \
  MagickWand/pixel-iterator.h \
  MagickWand/pixel-wand.h \
  MagickWand/stream.h \
  MagickWand/wandcli.h \
  MagickWand/wand-view.h

MAGICKWAND_NOINST_HDRS = \
  MagickWand/mogrify-private.h \
  MagickWand/magick-wand-private.h \
  MagickWand/operation-private.h \
  MagickWand/pixel-wand-private.h \
  MagickWand/script-token.h \
  MagickWand/studio.h \
  MagickWand/wand.h \
  MagickWand/wandcli-private.h

# Headers which are installed
MagickWandinc_HEADERS = \
  $(MAGICKWAND_INCLUDE_HDRS)

MAGICKWAND_BIN_SCRPTS = \
  MagickWand/MagickWand-config

MAGICKWAND_PKGCONFIG = \
  MagickWand/MagickWand.pc \
  MagickWand/MagickWand-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.pc

MAGICKWAND_MANS = \
  MagickWand/MagickWand-config.1

MAGICKWAND_BUILT_SRCS =

MAGICKWAND_LIBS = MagickWand/libMagickWand-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.la

MagickWand_libMagickWand_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_SOURCES = $(MAGICKWAND_SOURCES)
MagickWand_libMagickWand_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_CPPFLAGS = $(MAGICKWAND_CPPFLAGS) $(LIBRARY_EXTRA_CPPFLAGS)

if HAVE_LD_VERSION_SCRIPT
MagickWand_libMagickWand_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION =  -Wl,--version-script=$(top_srcdir)/MagickWand/libMagickWand.map
else
MagickWand_libMagickWand_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION = -export-symbols-regex ".*"
endif

MagickWand_libMagickWand_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS = -no-undefined \
  $(MagickWand_libMagickWand_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION) $(MAGICK_LT_RELEASE_OPTS) \
  -version-info \
  $(MAGICK_LIBRARY_CURRENT):$(MAGICK_LIBRARY_REVISION):$(MAGICK_LIBRARY_AGE)
MagickWand_libMagickWand_@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LIBADD = $(MAGICKCORE_LIBS) $(X11_LIBS) $(GOMP_LIBS) $(MATH_LIBS)

MAGICKWAND_EXTRA_DIST = \
  MagickWand/ChangeLog \
  MagickWand/libMagickWand.map \
  MagickWand/MagickWand-config.1 \
  MagickWand/MagickWand.pc.in

MAGICKWAND_CLEANFILES =
