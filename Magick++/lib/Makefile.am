#
# Makefile for Magick++ library
#
# Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
#
AUTOMAKE_OPTIONS = 1.8 nostdinc

SUBDIRS = Magick++

AM_CPPFLAGS = -I$(top_builddir) -I$(srcdir) -I$(top_srcdir)

includedir = @includedir@
pkgdatadir = $(datadir)/Magick++
pkglibdir = $(libdir)/Magick++
pkgincludedir = $(includedir)/Magick++

lib_LTLIBRARIES    = libMagick++.la

LIBRARY_CURRENT    = @LIBRARY_CURRENT@
LIBRARY_REVISION   = @LIBRARY_REVISION@
LIBRARY_AGE        = @LIBRARY_AGE@

libMagick___la_SOURCES = \
	Blob.cpp BlobRef.cpp CoderInfo.cpp Color.cpp Drawable.cpp \
	Exception.cpp Functions.cpp Geometry.cpp Image.cpp ImageRef.cpp \
	Montage.cpp Options.cpp Pixels.cpp STL.cpp Thread.cpp TypeMetric.cpp

libMagick___la_LDFLAGS = \
	-version-info $(LIBRARY_CURRENT):$(LIBRARY_REVISION):$(LIBRARY_AGE)

libMagick___la_LIBADD = $(top_builddir)/wand/libMagickWand.la $(top_builddir)/magick/libMagickCore.la

include_HEADERS = Magick++.h

EXTRA_DIST = ImageMagick++.pc.in

AM_LDFLAGS = @LDFLAGS@ $(LIBSTDCLDFLAGS)

# Pkgconfig directory
pkgconfigdir = $(libdir)/pkgconfig

# Files to install in Pkgconfig directory
pkgconfig_DATA = ImageMagick++.pc
