#
# Top Makefile for Magick++
#
# Copyright Bob Friesenhahn, 1999, 2000, 2002, 2004
#

if  WITH_MAGICK_PLUS_PLUS
magickppincdir = $(INCLUDE_PATH)/Magick++

MAGICKPP_LIBS = Magick++/lib/libMagick++.la

MAGICKPP_SCRPTS = \
	Magick++/bin/Magick++-config

MAGICKPP_MANS = \
	Magick++/bin/Magick++-config.1

MAGICKPP_PKGCONFIG = \
	Magick++/lib/ImageMagick++.pc

MAGICKPP_TESTS = \
	Magick++/tests/exceptions.sh \
	Magick++/tests/appendImages.sh \
	Magick++/tests/attributes.sh \
	Magick++/tests/averageImages.sh \
	Magick++/tests/coalesceImages.sh \
	Magick++/tests/coderInfo.sh \
	Magick++/tests/colorHistogram.sh \
	Magick++/tests/color.sh \
	Magick++/tests/montageImages.sh \
	Magick++/tests/morphImages.sh \
	Magick++/tests/readWriteBlob.sh \
	Magick++/tests/readWriteImages.sh \
	Magick++/demo/analyze.sh \
	Magick++/demo/button.sh \
	Magick++/demo/demo.sh \
	Magick++/demo/flip.sh \
	Magick++/demo/gravity.sh \
	Magick++/demo/piddle.sh \
	Magick++/demo/shapes.sh \
	Magick++/demo/zoom_bessel.sh \
	Magick++/demo/zoom_blackman.sh \
	Magick++/demo/zoom_box.sh \
	Magick++/demo/zoom_catrom.sh \
	Magick++/demo/zoom_cubic.sh \
	Magick++/demo/zoom_gaussian.sh \
	Magick++/demo/zoom_hamming.sh \
	Magick++/demo/zoom_hanning.sh \
	Magick++/demo/zoom_hermite.sh \
	Magick++/demo/zoom_lanczos.sh \
	Magick++/demo/zoom_mitchell.sh \
	Magick++/demo/zoom_point.sh \
	Magick++/demo/zoom_quadratic.sh \
	Magick++/demo/zoom_sample.sh \
	Magick++/demo/zoom_scale.sh \
	Magick++/demo/zoom_sinc.sh \
	Magick++/demo/zoom_triangle.sh

MAGICKPP_EXTRA_DIST = \
	Magick++/AUTHORS \
	Magick++/COPYING \
	Magick++/ChangeLog \
	Magick++/INSTALL \
	Magick++/NEWS \
	Magick++/README \
	Magick++/bin/Magick++-config.1 \
	Magick++/bin/Magick++-config.in \
	Magick++/lib/ImageMagick++.pc.in \
	Magick++/demo/model.miff \
	Magick++/demo/smile.miff \
	Magick++/demo/smile_anim.miff \
	Magick++/demo/tile.miff \
	$(MAGICKPP_TESTS) \
	Magick++/tests/test_image.miff \
	Magick++/tests/test_image_anim.miff

MAGICKPP_CLEANFILES = \
	Magick++/demo/*_out.* \
	Magick++/demo/ir.out \
	Magick++/tests/colorHistogram.txt \
	Magick++/tests/testmagick_anim_out.miff \
	Magick++/tests/ir.out

Magick___lib_libMagick___la_SOURCES = \
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
	Magick++/lib/Magick++/STL.h \
	Magick++/lib/Magick++/Thread.h \
	Magick++/lib/Magick++/TypeMetric.h

magickpptopincdir = $(INCLUDE_PATH)
magickpptopinc_HEADERS = \
	Magick++/lib/Magick++.h

magickppinc_HEADERS = \
	Magick++/lib/Magick++/Blob.h \
	Magick++/lib/Magick++/CoderInfo.h \
	Magick++/lib/Magick++/Color.h \
	Magick++/lib/Magick++/Drawable.h \
	Magick++/lib/Magick++/Exception.h \
	Magick++/lib/Magick++/Geometry.h \
	Magick++/lib/Magick++/Image.h \
	Magick++/lib/Magick++/Include.h \
	Magick++/lib/Magick++/Montage.h \
	Magick++/lib/Magick++/Pixels.h \
	Magick++/lib/Magick++/STL.h \
	Magick++/lib/Magick++/TypeMetric.h

Magick___lib_libMagick___la_LDFLAGS = $(MAGICK_LT_RELEASE_OPTS) \
	-version-info $(MAGICK_LIBRARY_CURRENT):$(MAGICK_LIBRARY_REVISION):$(MAGICK_LIBRARY_AGE)
Magick___lib_libMagick___la_LIBADD = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)

MAGICKPP_CHECK_PGRMS = \
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
	Magick++/tests/montageImages \
	Magick++/tests/morphImages \
	Magick++/tests/readWriteBlob \
	Magick++/tests/readWriteImages

Magick___demo_analyze_SOURCES		      = Magick++/demo/analyze.cpp
Magick___demo_analyze_LDADD		        = $(MAGICKPP_LIBS)

Magick___demo_button_SOURCES		      = Magick++/demo/button.cpp
Magick___demo_button_LDADD		        = $(MAGICKPP_LIBS)

Magick___demo_demo_SOURCES		        = Magick++/demo/demo.cpp
Magick___demo_demo_LDADD		          = $(MAGICKPP_LIBS)

Magick___demo_detrans_SOURCES		      = Magick++/demo/detrans.cpp
Magick___demo_detrans_LDADD		        = $(MAGICKPP_LIBS)

Magick___demo_flip_SOURCES		        = Magick++/demo/flip.cpp
Magick___demo_flip_LDADD		          = $(MAGICKPP_LIBS)

Magick___demo_gravity_SOURCES		      = Magick++/demo/gravity.cpp
Magick___demo_gravity_LDADD		        = $(MAGICKPP_LIBS)

Magick___demo_piddle_SOURCES		      = Magick++/demo/piddle.cpp
Magick___demo_piddle_LDADD		        = $(MAGICKPP_LIBS)

Magick___demo_shapes_SOURCES		      = Magick++/demo/shapes.cpp
Magick___demo_shapes_LDADD		        = $(MAGICKPP_LIBS)

Magick___demo_zoom_SOURCES		        = Magick++/demo/zoom.cpp
Magick___demo_zoom_LDADD		          = $(MAGICKPP_LIBS)

Magick___tests_appendImages_SOURCES	  = Magick++/tests/appendImages.cpp
Magick___tests_appendImages_LDADD	    = $(MAGICKPP_LIBS)

Magick___tests_attributes_SOURCES	    = Magick++/tests/attributes.cpp
Magick___tests_attributes_LDADD		    = $(MAGICKPP_LIBS)

Magick___tests_averageImages_SOURCES  = Magick++/tests/averageImages.cpp
Magick___tests_averageImages_LDADD 	  = $(MAGICKPP_LIBS)

Magick___tests_coalesceImages_SOURCES	= Magick++/tests/coalesceImages.cpp
Magick___tests_coalesceImages_LDADD	  = $(MAGICKPP_LIBS)

Magick___tests_coderInfo_SOURCES	    = Magick++/tests/coderInfo.cpp
Magick___tests_coderInfo_LDADD		    = $(MAGICKPP_LIBS)

Magick___tests_color_SOURCES		      = Magick++/tests/color.cpp
Magick___tests_color_LDADD		        = $(MAGICKPP_LIBS)

Magick___tests_colorHistogram_SOURCES	= Magick++/tests/colorHistogram.cpp
Magick___tests_colorHistogram_LDADD	  = $(MAGICKPP_LIBS)

Magick___tests_exceptions_SOURCES	    = Magick++/tests/exceptions.cpp
Magick___tests_exceptions_LDADD		    = $(MAGICKPP_LIBS)

Magick___tests_montageImages_SOURCES	= Magick++/tests/montageImages.cpp
Magick___tests_montageImages_LDADD	  = $(MAGICKPP_LIBS)

Magick___tests_morphImages_SOURCES	  = Magick++/tests/morphImages.cpp
Magick___tests_morphImages_LDADD	    = $(MAGICKPP_LIBS)

Magick___tests_readWriteBlob_SOURCES	= Magick++/tests/readWriteBlob.cpp
Magick___tests_readWriteBlob_LDADD	  = $(MAGICKPP_LIBS)

Magick___tests_readWriteImages_SOURCES= Magick++/tests/readWriteImages.cpp
Magick___tests_readWriteImages_LDADD  = $(MAGICKPP_LIBS)

MAGICKPP_LOCAL_TARGETS = www/Magick++/NEWS.html www/Magick++/ChangeLog.html

endif

if WITH_PERL

# Build HTML version of news
www/Magick++/NEWS.html: Magick++/NEWS
#	@PERL@ $(top_srcdir)/scripts/txt2html -t 'Magick++ News' < $(srcdir)/NEWS  > $(srcdir)/www/Magick++/NEWS.html
#	echo "Please do a 'cvs commit www/Magick++/NEWS.html' to submit updated HTML file"

# Build HTML version of ChangeLog
www/Magick++/ChangeLog.html: Magick++/ChangeLog
#	@PERL@ $(top_srcdir)/scripts/txt2html -t 'Magick++ ChangeLog' < $(srcdir)/ChangeLog > $(srcdir)/www/Magick++/ChangeLog.html
#	echo "Please do a 'cvs commit www/Magick++/ChangeLog.html' to submit updated HTML file"

else

#all-local:

endif


