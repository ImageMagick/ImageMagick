#  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
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
#  Makefile for building ImageMagick utilities.

UTILITIES_PGMS = \
	utilities/animate \
	utilities/compare \
	utilities/composite \
	utilities/conjure \
	utilities/display \
	utilities/identify \
	utilities/import \
	utilities/magick \
	utilities/mogrify \
	utilities/montage \
	utilities/stream

UTILITIES_XFAIL_TESTS = \
  $(UTILITIES_TTF_XFAIL_TESTS) \
  $(UTILITIES_XML_XFAIL_TESTS)

utilities_animate_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_animate_LDFLAGS	= $(LDFLAGS)
utilities_animate_SOURCES	= utilities/animate.c

utilities_compare_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_compare_LDFLAGS	= $(LDFLAGS)
utilities_compare_SOURCES	= utilities/compare.c

utilities_composite_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_composite_LDFLAGS	= $(LDFLAGS)
utilities_composite_SOURCES	= utilities/composite.c

utilities_conjure_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_conjure_LDFLAGS	= $(LDFLAGS)
utilities_conjure_SOURCES	= utilities/conjure.c

utilities_display_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_display_LDFLAGS	= $(LDFLAGS)
utilities_display_SOURCES	= utilities/display.c

utilities_identify_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_identify_LDFLAGS	= $(LDFLAGS)
utilities_identify_SOURCES	= utilities/identify.c

utilities_import_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_import_LDFLAGS	= $(LDFLAGS)
utilities_import_SOURCES	= utilities/import.c

utilities_magick_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_magick_LDFLAGS	= $(LDFLAGS)
utilities_magick_SOURCES	= utilities/magick.c

utilities_mogrify_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_mogrify_LDFLAGS	= $(LDFLAGS)
utilities_mogrify_SOURCES	= utilities/mogrify.c

utilities_montage_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_montage_LDFLAGS	= $(LDFLAGS)
utilities_montage_SOURCES	= utilities/montage.c

utilities_stream_LDADD	= $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_stream_LDFLAGS	= $(LDFLAGS)
utilities_stream_SOURCES	= utilities/stream.c

UTILITIES_MANS = \
	utilities/ImageMagick.1 \
	utilities/animate.1 \
	utilities/compare.1 \
	utilities/composite.1 \
	utilities/conjure.1 \
	utilities/convert.1 \
	utilities/display.1 \
	utilities/identify.1 \
	utilities/import.1 \
	utilities/magick.1 \
	utilities/mogrify.1 \
	utilities/montage.1 \
	utilities/stream.1

UTILITIES_CONFIGURE = \
	utilities/ImageMagick.1.in \
	utilities/animate.1.in \
	utilities/compare.1.in \
	utilities/composite.1.in \
	utilities/conjure.1.in \
	utilities/convert.1.in \
	utilities/display.1.in \
	utilities/identify.1.in \
	utilities/import.1.in \
	utilities/magick.1.in \
	utilities/mogrify.1.in \
	utilities/montage.1.in \
	utilities/stream.1.in

UTILITIES_EXTRA_DIST = \
	$(UTILITIES_MANS) \
	$(UTILITIES_TESTS) 

UTILITIES_CLEANFILES = 

# Link these utilities to 'magick'.
MAGICK_UTILITIES=convert magick-script
UTILITIES_INSTALL_EXEC_LOCAL_TARGETS=install-exec-local-utilities
install-exec-local-utilities:
	$(mkdir_p) $(DESTDIR)$(bindir)
	cd $(DESTDIR)$(bindir) ; \
	magick=`echo "magick" | sed 's,^.*/,,;s/$(EXEEXT)$$//;$(transform);s/$$/$(EXEEXT)/'`; \
	for name in $(MAGICK_UTILITIES) ; \
	do \
	  target=`echo "$$name" | sed 's,^.*/,,;s/$(EXEEXT)$$//;$(transform);s/$$/$(EXEEXT)/'`; \
	  rm -f $$target ; \
	  $(LN_S) $$magick $$target ; \
	done

UTILITIES_UNINSTALL_LOCAL_TARGETS=uninstall-local-utilities
uninstall-local-utilities:
	cd $(DESTDIR)$(bindir) ; \
	for name in $(MAGICK_UTILITIES) ; \
	do \
	  target=`echo "$$name" | sed 's,^.*/,,;s/$(EXEEXT)$$//;$(transform);s/$$/$(EXEEXT)/'`; \
	  rm -f $$target ; \
	done
