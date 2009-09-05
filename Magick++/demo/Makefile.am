#
# Makefile for Magick++ demo programs
#
# Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
#
AUTOMAKE_OPTIONS = 1.8 foreign nostdinc

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/Magick++/lib -I$(top_srcdir)

check_PROGRAMS = analyze button demo detrans flip gravity piddle shapes zoom

LDADD = ../lib/libMagick++.la

analyze_SOURCES = analyze.cpp
button_SOURCES  = button.cpp
demo_SOURCES    = demo.cpp
detrans_SOURCES = detrans.cpp
flip_SOURCES    = flip.cpp
gravity_SOURCES = gravity.cpp
piddle_SOURCES  = piddle.cpp
shapes_SOURCES  = shapes.cpp
zoom_SOURCES	= zoom.cpp

EXTRA_DIST = model.miff smile.miff smile_anim.miff tile.miff

CLEANFILES = *_out.* ir.out # SunWS_cache

ZOOM_FILTERS = bessel blackman box catrom cubic gaussian hamming hanning hermite \
 lanczos mitchell point quadratic sample scale sinc triangle
ZOOM_GEOMETRY = 600x600

if WIN32_NATIVE_BUILD
SRCDIR='$(shell @WinPathScript@ $(srcdir)/)'
else
SRCDIR='$(srcdir)/'
endif

MAGICK_ENV = $(SHELL) $(top_builddir)/magick.sh

# FIXME: MinGW needs Windows path form of srcdir here (or pass as command argument)
check:
	@$(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./analyze '$(srcdir)/model.miff'
	@$(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./button
	@$(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./demo
	@$(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./flip
	@$(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./gravity
	@$(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./piddle
	@$(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./shapes
	for filter in $(ZOOM_FILTERS) ; do  \
	  echo "$$filter ..." ; \
	  $(MAGICK_ENV) SRCDIR=$(SRCDIR) $(MEMCHECK) ./zoom -filter $$filter -geometry $(ZOOM_GEOMETRY) '$(srcdir)/model.miff' zoom_$${filter}_out.miff ; \
	done

