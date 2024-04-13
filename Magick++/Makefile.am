#
# Top Makefile for Magick++
#
# Copyright Bob Friesenhahn, 1999, 2000, 2002, 2004, 2008
#

# AM_CPPFLAGS += -I$(top_srcdir)/Magick++/lib

MAGICKPP_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/Magick++/lib

if WITH_MAGICK_PLUS_PLUS
MAGICKPP_LIBS = Magick++/lib/libMagick++-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.la
MAGICKPP_LDADD = $(MAGICKPP_LIBS) $(top_builddir)/MagickCore/libMagickCore-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.la $(top_builddir)/MagickWand/libMagickWand-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.la
MAGICKPP_CHECK_PGRMS = $(MAGICKPP_CHECK_PGRMS_OPT)
MAGICKPP_MANS = $(MAGICKPP_MANS_OPT)
MAGICKPP_PKGCONFIG = $(MAGICKPP_PKGCONFIG_OPT)
MAGICKPP_SCRPTS = $(MAGICKPP_SCRPTS_OPT)
MAGICKPP_TESTS = $(MAGICKPP_TEST_SCRPTS_OPT)
MAGICKPP_TOP_INCHEADERS = $(MAGICKPP_TOP_INCHEADERS_OPT)
MAGICKPP_INCHEADERS = $(MAGICKPP_INCHEADERS_OPT)
else
MAGICKPP_LIBS = 
MAGICKPP_LDADD =
MAGICKPP_CHECK_PGRMS = 
MAGICKPP_MANS = 
MAGICKPP_PKGCONFIG = 
MAGICKPP_SCRPTS =
MAGICKPP_TESTS = 
MAGICKPP_TOP_INCHEADERS =
MAGICKPP_INCHEADERS =
endif

MAGICKPP_SCRPTS_OPT = \
  Magick++/bin/Magick++-config

MAGICKPP_MANS_OPT = \
  Magick++/bin/Magick++-config.1

MAGICKPP_PKGCONFIG_OPT = \
  Magick++/lib/Magick++.pc \
  Magick++/lib/Magick++-@MAGICK_MAJOR_VERSION@.@MAGICK_ABI_SUFFIX@.pc

MAGICKPP_TEST_SCRPTS_OPT = \
  Magick++/tests/tests.tap \
  Magick++/demo/demos.tap

MAGICKPP_EXTRA_DIST = \
  Magick++/AUTHORS \
  Magick++/LICENSE \
  Magick++/bin/Magick++-config.1 \
  Magick++/bin/Magick++-config.in \
  Magick++/lib/libMagick++.map \
  Magick++/lib/Magick++.pc.in \
  Magick++/demo/model.miff \
  Magick++/demo/smile.miff \
  Magick++/demo/smile_anim.miff \
  Magick++/demo/tile.miff \
  $(MAGICKPP_TEST_SCRPTS_OPT) \
  Magick++/tests/test_image.miff \
  Magick++/tests/test_image_anim.miff

MAGICKPP_CLEANFILES = \
  Magick++/demo/*_out* \
  Magick++/demo/ir.out \
  Magick++/tests/colorHistogram.txt \
  Magick++/tests/testmagick_anim_out.miff \
  Magick++/tests/ir.out

Magick___lib_libMagick___@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_SOURCES = \
  Magick++/lib/Blob.cpp \
  Magick++/lib/BlobRef.cpp \
  Magick++/lib/CoderInfo.cpp \
  Magick++/lib/Color.cpp \
  Magick++/lib/Drawable.cpp \
  Magick++/lib/Exception.cpp \
  Magick++/lib/Functions.cpp \
  Magick++/lib/Geometry.cpp \
  Magick++/lib/Image.cpp \
  Magick++/lib/ImageRef.cpp \
  Magick++/lib/Montage.cpp \
  Magick++/lib/Options.cpp \
  Magick++/lib/Pixels.cpp \
  Magick++/lib/ResourceLimits.cpp \
  Magick++/lib/SecurityPolicy.cpp \
  Magick++/lib/Statistic.cpp \
  Magick++/lib/STL.cpp \
  Magick++/lib/Thread.cpp \
  Magick++/lib/TypeMetric.cpp \
  Magick++/lib/Magick++.h \
  Magick++/lib/Magick++/Blob.h \
  Magick++/lib/Magick++/BlobRef.h \
  Magick++/lib/Magick++/CoderInfo.h \
  Magick++/lib/Magick++/Color.h \
  Magick++/lib/Magick++/Drawable.h \
  Magick++/lib/Magick++/Exception.h \
  Magick++/lib/Magick++/Functions.h \
  Magick++/lib/Magick++/Geometry.h \
  Magick++/lib/Magick++/Image.h \
  Magick++/lib/Magick++/ImageRef.h \
  Magick++/lib/Magick++/Include.h \
  Magick++/lib/Magick++/Montage.h \
  Magick++/lib/Magick++/Options.h \
  Magick++/lib/Magick++/Pixels.h \
  Magick++/lib/Magick++/ResourceLimits.h \
  Magick++/lib/Magick++/SecurityPolicy.h \
  Magick++/lib/Magick++/Statistic.h \
  Magick++/lib/Magick++/STL.h \
  Magick++/lib/Magick++/Thread.h \
  Magick++/lib/Magick++/TypeMetric.h

Magick___lib_libMagick___@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_CPPFLAGS = \
  $(MAGICKPP_CPPFLAGS) $(LIBRARY_EXTRA_CPPFLAGS)

magickpptopincdir = $(INCLUDE_PATH)

magickpptopinc_HEADERS = $(MAGICKPP_TOP_INCHEADERS)

MAGICKPP_TOP_INCHEADERS_OPT = \
  Magick++/lib/Magick++.h

magickppincdir = $(INCLUDE_PATH)/Magick++

magickppinc_HEADERS = $(MAGICKPP_INCHEADERS)

MAGICKPP_INCHEADERS_OPT = \
  Magick++/lib/Magick++/Blob.h \
  Magick++/lib/Magick++/CoderInfo.h \
  Magick++/lib/Magick++/Color.h \
  Magick++/lib/Magick++/Drawable.h \
  Magick++/lib/Magick++/Exception.h \
  Magick++/lib/Magick++/Functions.h \
  Magick++/lib/Magick++/Geometry.h \
  Magick++/lib/Magick++/Image.h \
  Magick++/lib/Magick++/Include.h \
  Magick++/lib/Magick++/Montage.h \
  Magick++/lib/Magick++/Pixels.h \
  Magick++/lib/Magick++/ResourceLimits.h \
  Magick++/lib/Magick++/SecurityPolicy.h \
  Magick++/lib/Magick++/Statistic.h \
  Magick++/lib/Magick++/STL.h \
  Magick++/lib/Magick++/TypeMetric.h

if HAVE_LD_VERSION_SCRIPT
Magick___lib_libMagick___@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION =  -Wl,--version-script=$(top_srcdir)/Magick++/lib/libMagick++.map
else
Magick___lib_libMagick___@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS_VERSION = -export-symbols-regex ".*"
endif

Magick___lib_libMagick___@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LDFLAGS = -no-undefined \
  $(Magick___lib_libMagick___la_LDFLAGS_VERSION) $(MAGICK_LT_RELEASE_OPTS) \
  -version-info \
  $(MAGICKPP_LIBRARY_CURRENT):$(MAGICKPP_LIBRARY_REVISION):$(MAGICKPP_LIBRARY_AGE)
Magick___lib_libMagick___@MAGICK_MAJOR_VERSION@_@MAGICK_ABI_SUFFIX@_la_LIBADD = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)

MAGICKPP_CHECK_PGRMS_OPT = \
  Magick++/demo/analyze \
  Magick++/demo/button \
  Magick++/demo/demo \
  Magick++/demo/detrans \
  Magick++/demo/flip \
  Magick++/demo/gravity \
  Magick++/demo/piddle \
  Magick++/demo/shapes \
  Magick++/demo/zoom \
  Magick++/tests/appendImages \
  Magick++/tests/attributes \
  Magick++/tests/averageImages \
  Magick++/tests/coalesceImages \
  Magick++/tests/coderInfo \
  Magick++/tests/color \
  Magick++/tests/colorHistogram \
  Magick++/tests/exceptions \
  Magick++/tests/geometry \
  Magick++/tests/montageImages \
  Magick++/tests/morphImages \
  Magick++/tests/readWriteBlob \
  Magick++/tests/readWriteImages

Magick___demo_analyze_SOURCES   = Magick++/demo/analyze.cpp
Magick___demo_analyze_LDADD     = $(MAGICKPP_LDADD)
Magick___demo_analyze_CPPFLAGS  = $(MAGICKPP_CPPFLAGS)

Magick___demo_button_SOURCES    = Magick++/demo/button.cpp
Magick___demo_button_LDADD      = $(MAGICKPP_LDADD)
Magick___demo_button_CPPFLAGS   = $(MAGICKPP_CPPFLAGS)

Magick___demo_demo_SOURCES      = Magick++/demo/demo.cpp
Magick___demo_demo_LDADD        = $(MAGICKPP_LDADD)
Magick___demo_demo_CPPFLAGS     = $(MAGICKPP_CPPFLAGS)

Magick___demo_detrans_SOURCES   = Magick++/demo/detrans.cpp
Magick___demo_detrans_LDADD     = $(MAGICKPP_LDADD)
Magick___demo_detrans_CPPFLAGS  = $(MAGICKPP_CPPFLAGS)

Magick___demo_flip_SOURCES      = Magick++/demo/flip.cpp
Magick___demo_flip_LDADD        = $(MAGICKPP_LDADD)
Magick___demo_flip_CPPFLAGS     = $(MAGICKPP_CPPFLAGS)

Magick___demo_gravity_SOURCES   = Magick++/demo/gravity.cpp
Magick___demo_gravity_LDADD     = $(MAGICKPP_LDADD)
Magick___demo_gravity_CPPFLAGS  = $(MAGICKPP_CPPFLAGS)

Magick___demo_piddle_SOURCES    = Magick++/demo/piddle.cpp
Magick___demo_piddle_LDADD      = $(MAGICKPP_LDADD)
Magick___demo_piddle_CPPFLAGS   = $(MAGICKPP_CPPFLAGS)

Magick___demo_shapes_SOURCES    = Magick++/demo/shapes.cpp
Magick___demo_shapes_LDADD      = $(MAGICKPP_LDADD)
Magick___demo_shapes_CPPFLAGS   = $(MAGICKPP_CPPFLAGS)

Magick___demo_zoom_SOURCES    = Magick++/demo/zoom.cpp
Magick___demo_zoom_LDADD      = $(MAGICKPP_LDADD)
Magick___demo_zoom_CPPFLAGS   = $(MAGICKPP_CPPFLAGS)

Magick___tests_appendImages_SOURCES   = Magick++/tests/appendImages.cpp
Magick___tests_appendImages_LDADD     = $(MAGICKPP_LDADD)
Magick___tests_appendImages_CPPFLAGS  = $(MAGICKPP_CPPFLAGS)

Magick___tests_attributes_SOURCES     = Magick++/tests/attributes.cpp
Magick___tests_attributes_LDADD       = $(MAGICKPP_LDADD)
Magick___tests_attributes_CPPFLAGS    = $(MAGICKPP_CPPFLAGS)

Magick___tests_averageImages_SOURCES  = Magick++/tests/averageImages.cpp
Magick___tests_averageImages_LDADD    = $(MAGICKPP_LDADD)
Magick___tests_averageImages_CPPFLAGS = $(MAGICKPP_CPPFLAGS)

Magick___tests_coalesceImages_SOURCES = Magick++/tests/coalesceImages.cpp
Magick___tests_coalesceImages_LDADD   = $(MAGICKPP_LDADD)
Magick___tests_coalesceImages_CPPFLAGS= $(MAGICKPP_CPPFLAGS)

Magick___tests_coderInfo_SOURCES      = Magick++/tests/coderInfo.cpp
Magick___tests_coderInfo_LDADD        = $(MAGICKPP_LDADD)
Magick___tests_coderInfo_CPPFLAGS     = $(MAGICKPP_CPPFLAGS)

Magick___tests_color_SOURCES          = Magick++/tests/color.cpp
Magick___tests_color_LDADD            = $(MAGICKPP_LDADD)
Magick___tests_color_CPPFLAGS         = $(MAGICKPP_CPPFLAGS)

Magick___tests_colorHistogram_SOURCES = Magick++/tests/colorHistogram.cpp
Magick___tests_colorHistogram_LDADD   = $(MAGICKPP_LDADD)
Magick___tests_colorHistogram_CPPFLAGS= $(MAGICKPP_CPPFLAGS)

Magick___tests_exceptions_SOURCES     = Magick++/tests/exceptions.cpp
Magick___tests_exceptions_LDADD       = $(MAGICKPP_LDADD)
Magick___tests_exceptions_CPPFLAGS    = $(MAGICKPP_CPPFLAGS)

Magick___tests_geometry_SOURCES     = Magick++/tests/geometry.cpp
Magick___tests_geometry_LDADD       = $(MAGICKPP_LDADD)
Magick___tests_geometry_CPPFLAGS    = $(MAGICKPP_CPPFLAGS)

Magick___tests_montageImages_SOURCES  = Magick++/tests/montageImages.cpp
Magick___tests_montageImages_LDADD    = $(MAGICKPP_LDADD)
Magick___tests_montageImages_CPPFLAGS = $(MAGICKPP_CPPFLAGS)

Magick___tests_morphImages_SOURCES    = Magick++/tests/morphImages.cpp
Magick___tests_morphImages_LDADD      = $(MAGICKPP_LDADD)
Magick___tests_morphImages_CPPFLAGS   = $(MAGICKPP_CPPFLAGS)

Magick___tests_readWriteBlob_SOURCES  = Magick++/tests/readWriteBlob.cpp
Magick___tests_readWriteBlob_LDADD    = $(MAGICKPP_LDADD)
Magick___tests_readWriteBlob_CPPFLAGS = $(MAGICKPP_CPPFLAGS)

Magick___tests_readWriteImages_SOURCES  = Magick++/tests/readWriteImages.cpp
Magick___tests_readWriteImages_LDADD    = $(MAGICKPP_LDADD)
Magick___tests_readWriteImages_CPPFLAGS = $(MAGICKPP_CPPFLAGS)

MAGICKPP_LOCAL_TARGETS = www/Magick++/NEWS.html www/Magick++/ChangeLog.html
