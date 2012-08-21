#
# Makefile for Magick++ tests
#
# Copyright Bob Friesenhahn, 1999, 2000, 2002, 2004
#
AUTOMAKE_OPTIONS = 1.8 foreign nostdinc

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/Magick++/lib -I$(top_srcdir)

#
# Execute 'make check' to compile and run a test program
#
check_PROGRAMS =  \
	appendImages \
	attributes \
	averageImages \
	coalesceImages \
	coderInfo \
	color \
	colorHistogram \
	exceptions \
	montageImages \
	morphImages \
	readWriteBlob \
	readWriteImages

LDADD = ../lib/libMagick++.la

appendImages_SOURCES	= appendImages.cpp
attributes_SOURCES	= attributes.cpp
averageImages_SOURCES	= averageImages.cpp
coalesceImages_SOURCES	= coalesceImages.cpp
coderInfo_SOURCES	= coderInfo.cpp
color_SOURCES		= color.cpp
colorHistogram_SOURCES  = colorHistogram.cpp
exceptions_SOURCES	= exceptions.cpp
montageImages_SOURCES	= montageImages.cpp
morphImages_SOURCES	= morphImages.cpp
readWriteBlob_SOURCES	= readWriteBlob.cpp
readWriteImages_SOURCES = readWriteImages.cpp


if WIN32_NATIVE_BUILD
SRCDIR='$(shell @WinPathScript@ $(srcdir)/)'
else
SRCDIR="$(srcdir)/"
endif

MAGICK_ENV = "$(SHELL) $(top_builddir)/magick.sh"

# Environment parameters to be used during tests
TESTS_ENVIRONMENT = \
	MAGICK_ENV=$(MAGICK_ENV) \
	SRCDIR=$(SRCDIR)

# Tests to run
TESTS = \
	tests.tap

# Tests which are expected to fail
XFAIL_TESTS =

# Extra files to distribute
EXTRA_DIST = \
	$(TESTS) \
	test_image.miff \
	test_image_anim.miff

CLEANFILES = colorHistogram.txt testmagick_anim_out.miff ir.out # SunWS_cache
