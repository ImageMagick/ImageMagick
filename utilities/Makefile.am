#  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for building ImageMagick utilities.

if WITH_UTILITIES
UTILITIES_PGMS = \
  utilities/magick

UTILITIES_XFAIL_TESTS = \
  $(UTILITIES_TTF_XFAIL_TESTS) \
  $(UTILITIES_XML_XFAIL_TESTS)

utilities_magick_LDADD    = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS)
utilities_magick_LDFLAGS  = $(LDFLAGS)
utilities_magick_SOURCES  = utilities/magick.c

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
  utilities/magick-script.1 \
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
  utilities/magick-script.1.in \
  utilities/mogrify.1.in \
  utilities/montage.1.in \
  utilities/stream.1.in

UTILITIES_EXTRA_DIST = \
  $(UTILITIES_MANS) \
  $(UTILITIES_TESTS)

UTILITIES_CLEANFILES =

# Link these utilities to 'magick'.
MAGICK_UTILITIES=animate compare composite conjure convert display identify import magick-script mogrify montage stream

else
UTILITIES_PGMS =
UTILITIES_MANS =
UTILITIES_CONFIGURE =
UTILITIES_EXTRA_DIST =
UTILITIES_CLEANFILES =
MAGICK_UTILITIES=
endif

UTILITIES_INSTALL_EXEC_LOCAL_TARGETS=install-exec-local-utilities
install-exec-local-utilities:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
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
