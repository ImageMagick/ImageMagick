#  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
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

MagickWandincdir = $(INCLUDE_PATH)/MagickWand

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
	MagickWand/magick-image.c \
	MagickWand/magick-image.h \
	MagickWand/magick-property.c \
	MagickWand/magick-property.h \
	MagickWand/magick-cli.c \
	MagickWand/magick-cli.h \
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
	MagickWand/stream.c \
	MagickWand/stream.h \
	MagickWand/studio.h \
	MagickWand/wand.c \
	MagickWand/wand.h \
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
	MagickWand/wand-view.h

MAGICKWAND_NOINST_HDRS = \
	MagickWand/mogrify-private.h \
	MagickWand/magick-wand-private.h \
	MagickWand/operation-private.h \
	MagickWand/pixel-wand-private.h \
	MagickWand/studio.h \
	MagickWand/wand.h

# Headers which are installed
MagickWandinc_HEADERS = \
	$(MAGICKWAND_INCLUDE_HDRS)

MAGICKWAND_BIN_SCRPTS = \
	MagickWand/MagickWand-config

MAGICKWAND_PKGCONFIG = \
	MagickWand/MagickWand.pc

MAGICKWAND_MANS = \
	MagickWand/MagickWand-config.1

MAGICKWAND_BUILT_SRCS = 

MAGICKWAND_LIBS = MagickWand/libMagickWand.la

MagickWand_libMagickWand_la_SOURCES = $(MAGICKWAND_SOURCES)
MagickWand_libMagickWand_la_CPPFLAGS = $(LIBRARY_EXTRA_CPPFLAGS)
MagickWand_libMagickWand_la_LDFLAGS = -no-undefined -export-symbols-regex ".*" \
	-version-info \
	$(MAGICK_LIBRARY_CURRENT):$(MAGICK_LIBRARY_REVISION):$(MAGICK_LIBRARY_AGE)
MagickWand_libMagickWand_la_LIBADD = $(MAGICKCORE_LIBS) $(X11_LIBS) $(GOMP_LIBS) $(MATH_LIBS)

MAGICKWAND_EXTRA_DIST = \
	MagickWand/ChangeLog \
	MagickWand/input_256c.miff \
	MagickWand/input_bilevel.miff \
	MagickWand/input_gray.miff \
	MagickWand/input_truecolor.miff \
	MagickWand/MagickWand-config.1 \
	MagickWand/MagickWand.pc.in \
	MagickWand/sequence.miff \
	MagickWand/common.sh \
	$(MAGICKWAND_TESTS)

MagickWand_drawtest_SOURCES      = MagickWand/drawtest.c
MagickWand_drawtest_LDFLAGS      = $(LDFLAGS)
MagickWand_drawtest_LDADD        = $(MAGICKWAND_LIBS)

MagickWand_wandtest_SOURCES      = MagickWand/wandtest.c
MagickWand_wandtest_LDFLAGS      = $(LDFLAGS)
MagickWand_wandtest_LDADD        = $(MAGICKWAND_LIBS)

MAGICKWAND_CHECK_PGRMS = \
	MagickWand/drawtest \
	MagickWand/wandtest

MAGICKWAND_TESTS = \
	MagickWand/drawtest.sh \
	MagickWand/wandtest.sh

MAGICKWAND_CLEANFILES = \
	MagickWand/*_out.*
