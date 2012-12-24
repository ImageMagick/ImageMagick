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
#  Copyright (C) 2003 - 2008 GraphicsMagick Group
#
#  Top-Level Makefile for building ImageMagick.
#

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

ACLOCAL_AMFLAGS = -I m4

MODULECOMMONFLAGS = -no-undefined -export-symbols-regex ".*" -shared -module -avoid-version

SUBDIRS = .

# Options to pass when running configure in the distcheck target.
#
# We want to preserve user-provided option variables so the same
# compiler, headers, and libraries are used as for a normal build.
DISTCHECK_CONFIGURE_FLAGS=$(DISTCHECK_CONFIG_FLAGS)

DISTCLEANFILES = _configs.sed magick/magick-config.h

## Make sure these will be cleaned even when they're not built by default.
CLEANFILES = \
	$(WAND_CLEANFILES) \
	$(MAGICKPP_CLEANFILES) \
	$(UTILITIES_CLEANFILES) \
	$(TESTS_CLEANFILES)

bin_PROGRAMS = \
	$(UTILITIES_PGMS)

# Binary scripts
bin_SCRIPTS = \
	$(MAGICK_BIN_SCRPTS) \
	$(WAND_BIN_SCRPTS) \
	$(MAGICKPP_SCRPTS)

include_HEADERS =

# Headers which are not installed but which are distributed
noinst_HEADERS = \
	$(MAGICK_NOINST_HDRS)

if WIN32_NATIVE_BUILD
SRCDIR='$(shell @WinPathScript@ $(srcdir)/)'
else
SRCDIR="$(srcdir)/"
endif

# Tests with .tap extensions use the TAP protocol and TAP driver
LOG_COMPILER = $(SHELL)

SH_LOG_COMPILER = $(LOG_COMPILER)
TAP_LOG_COMPILER = $(LOG_COMPILER)

# Test extensions
TEST_EXTENSIONS = .sh .tap

TAP_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/config/tap-driver.sh

# Environment parameters to be used during tests
TESTS_ENVIRONMENT = \
	MAKE="$(MAKE)" \
	MAKEFLAGS="$(MAKEFLAGS)" \
	MEMCHECK="$(MEMCHECK)"

# Tests to run
TESTS = \
	$(TESTS_TESTS) \
	$(MAGICKPP_TESTS) \
	$(WAND_TESTS) \
	$(UTILITIES_TESTS)

# Tests which are expected to fail
XFAIL_TESTS = \
	$(TESTS_XFAIL_TESTS) \
	$(UTILITIES_XFAIL_TESTS)

TOP_EXTRA_DIST = \
  AUTHORS.txt \
	ChangeLog \
	LICENSE \
	QuickStart.txt \
	NOTICE \
	Install-mac.txt \
	Install-unix.txt \
	Install-vms.txt \
	Install-windows.txt \
	magick.sh.in \
	Magickshr.opt \
	NEWS.txt \
	Platforms.txt \
	README.txt \
	index.html \
	version.sh \
	winpath.sh

# Additional files to distribute
EXTRA_DIST = \
	$(TOP_EXTRA_DIST) \
	$(CONFIG_EXTRA_DIST) \
	$(MAGICK_EXTRA_DIST) \
	$(WAND_EXTRA_DIST) \
	$(MAGICKPP_EXTRA_DIST) \
	$(UTILITIES_EXTRA_DIST) \
	$(TESTS_EXTRA_DIST)

lib_LTLIBRARIES = $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(MAGICKPP_LIBS)
AM_LDFLAGS =
noinst_LTLIBRARIES =
EXTRA_LTLIBRARIES =
BUILT_SOURCES =
MOSTLYCLEANFILES =

check_PROGRAMS = \
	$(TESTS_CHECK_PGRMS) \
	$(MAGICKPP_CHECK_PGRMS) \
	$(WAND_CHECK_PGRMS)

include m4/Makefile.am
include config/Makefile.am
include coders/Makefile.am
include magick/Makefile.am
include filters/Makefile.am
include wand/Makefile.am
include Magick++/Makefile.am
include utilities/Makefile.am
include tests/Makefile.am
include PerlMagick/Makefile.am

# Pkgconfig directory
pkgconfigdir = $(libdir)/pkgconfig

# Files to install in Pkgconfig directory
pkgconfig_DATA = \
	$(MAGICK_PKGCONFIG) \
	$(WAND_PKGCONFIG) \
	$(MAGICKPP_PKGCONFIG)

# create a copy for pc file (ideally the non abi should be symlinked)
%-@MAGICK_ABI_SUFFIX@.pc: %.pc
	cp -f $^ $@

# Manual pages to install
man_MANS = \
	$(MAGICK_MANS) \
	$(WAND_MANS) \
	$(MAGICKPP_MANS) \
	$(UTILITIES_MANS)

# Architecture independent data files installed in the package's data directory
docdir = $(DOCUMENTATION_PATH)
doc_DATA = \
	LICENSE \
	ChangeLog \
	NEWS.txt

if MAINTAINER_MODE
MAINTAINER_TARGETS = \
	magick-version \
  ImageMagick.spec \
  $(PERLMAGICK_MAINTAINER_TARGETS)
endif

all-local: $(MAGICKPP_LOCAL_TARGETS) $(PERLMAGICK_ALL_LOCAL_TARGETS) $(MAINTAINER_TARGETS)

install-exec-local: $(PERLMAGICK_INSTALL_EXEC_LOCAL_TARGETS)

install-data-local: $(MAGICK_INSTALL_DATA_LOCAL_TARGETS) $(PERLMAGICK_INSTALL_DATA_LOCAL_TARGETS) $(HTML_INSTALL_DATA_TARGETS)

uninstall-local: $(MAGICK_UNINSTALL_LOCAL_TARGETS) $(PERLMAGICK_UNINSTALL_LOCAL_TARGETS) $(HTML_UNINSTALL_DATA_TARGETS)

clean-local: $(PERLMAGICK_CLEAN_LOCAL_TARGETS)

distclean-local: $(PERLMAGICK_DISTCLEAN_LOCAL_TARGETS)

maintainer-clean-local: $(PERLMAGICK_MAINTAINER_CLEAN_LOCAL_TARGETS)

check-local: $(PERLMAGICK_CHECK_LOCAL_TARGETS)

# drd: valgrind's newer thread error detector
drd:
	$(MAKE) MEMCHECK='valgrind --tool=drd --check-stack-var=yes --var-info=yes \
	  --quiet $(VALGRIND_EXTRA_OPTS)' check

# helgrind: valgrind's older thread error detector
helgrind:
	$(MAKE) MEMCHECK='valgrind --tool=helgrind --error-exitcode=2 --quiet \
	$(VALGRIND_EXTRA_OPTS)' check

# memcheck: valgrind's memory access checker
memcheck:
	$(MAKE) MEMCHECK='valgrind --tool=memcheck --leak-check=full --read-var-info=yes \
	--error-exitcode=2 --track-origins=yes --num-callers=12 \
	--quiet $(VALGRIND_EXTRA_OPTS)' check

# ptrcheck: valgrind's experimental pointer checking tool.
ptrcheck:
	$(MAKE) MEMCHECK='valgrind --tool=exp-ptrcheck --quiet $(VALGRIND_EXTRA_OPTS)' check

# Non-Automake subdirectories to distribute
DISTDIRS = images scenes scripts www PerlMagick
dist-hook:
	( \
	  builddir=`pwd` ; \
	  cd $(srcdir) && \
	  ( \
	    for dir in $(DISTDIRS) ; do \
	      find $$dir -depth -print | egrep -v '(~$$)|(/CVS)|(/\.#)|(/\.deps)|(/\.svn)' \
	        | cpio -pdum $$builddir/$(distdir) 2> /dev/null ; \
	    done \
	  ) \
	)

#
# Additional install rules
#

# Install HTML files
pkgdocdir = $(DOCUMENTATION_PATH)
DOCDIRS = images images/patterns www www/api www/source www/api/MagickCore www/api/MagickWand www/Magick++
HTML_INSTALL_DATA_TARGETS = install-data-html
install-data-html:
	$(mkinstalldirs) $(DESTDIR)$(pkgdocdir)
	$(INSTALL_DATA) $(srcdir)/index.html $(DESTDIR)$(pkgdocdir)
	@for dir in $(DOCDIRS) ; do \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/$$dir && \
	  for file in $(srcdir)/$$dir/*.* ; do \
	    echo "$(INSTALL_DATA) $$file $(DESTDIR)$(pkgdocdir)/$$dir" ; \
	    $(INSTALL_DATA) "$$file" $(DESTDIR)$(pkgdocdir)/$$dir ; \
	  done ; \
	done

# Uninstall HTML files
HTML_UNINSTALL_DATA_TARGETS = uninstall-data-html
uninstall-data-html:
	rm -f $(DESTDIR)$(pkgdocdir)/index.html
	for dir in $(DOCDIRS) ; do \
	  rm -f -r $(DESTDIR)$(pkgdocdir)/$$dir ; \
	done

# Ensure that version.h at $(srcdir)/magick/version.h is kept up to date.
magick-version: magick/version.h
	@if test -f "$(srcdir)/VisualMagick/installer/inc/version.isx.in" ; then \
	  ./config.status --file="$(srcdir)/../VisualMagick/installer/inc/version.isx" 2> /dev/null ; \
	fi
	@if test -n "$(VPATH)" ; then \
	  cmp magick/version.h $(srcdir)/magick/version.h > /dev/null ; \
          if test $$? -eq 1 ; then \
	    echo "Updating $(srcdir)/magick/version.h ..."; \
	    cp magick/version.h $(srcdir)/magick/version.h ; \
	  fi ; \
	fi ; \
	touch magick-version

${srcdir}/configure: ${srcdir}/version.sh

# Automagically reconfigure libtool
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# Format C API documentation
html:
	doxygen config/MagickCore.dox
	doxygen config/MagickWand.dox
	doxygen config/Magick++.dox

#
# Build Windows source Zip and 7Zip balls
#
if ZIP_DELEGATE
DIST_WINDOWS_SRC_ZIP=$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-windows.zip
else
DIST_WINDOWS_SRC_ZIP=
endif
if P7ZIP_DELEGATE
DIST_WINDOWS_SRC_7ZIP=$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-windows.7z
else
DIST_WINDOWS_SRC_7ZIP=
endif
$(DIST_WINDOWS_SRC_ZIP) $(DIST_WINDOWS_SRC_7ZIP) windows-dist:
	if test -d $(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM) ; then \
	  chmod -R u+w $(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM) ; \
	  rm -rf $(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM) ; \
	fi
  svn export https://subversion.imagemagick.org/subversion/ImageMagick-Windows/trunk $(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)
if ZIP_DELEGATE
	rm -f $(DIST_WINDOWS_SRC_ZIP)
	$(ZIP) -r -9 -q  $(DIST_WINDOWS_SRC_ZIP) $(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)
endif # ZIP_DELEGATE
if P7ZIP_DELEGATE
	rm -f $(DIST_WINDOWS_SRC_7ZIP)
	$(P7ZIP) a -t7z -mx=9 $(DIST_WINDOWS_SRC_7ZIP) $(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)
	chmod 644 $(DIST_WINDOWS_SRC_7ZIP)
endif # P7ZIP_DELEGATE
	rm -rf $(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)

#
# RPM build support
#
if RPM_DELEGATE

DIST_ARCHIVE_SRPM=$(distdir)-1.src.rpm
.PHONY: srpm
$(DIST_ARCHIVE_SRPM) srpm: dist-bzip2
	rm -f $(DIST_ARCHIVE_SRPM)
	$(RPM) --define="_sourcedir `pwd`" --define="_srcrpmdir `pwd`" --nodeps --bs ImageMagick.spec
	@echo ==============================================================
	@echo $(DIST_ARCHIVE_SRPM) is ready for distribution.
	@echo ==============================================================

RPMDIR=rpmbuild
RPMARCH=$(MAGICK_TARGET_CPU)

DIST_ARCHIVE_RPM= \
	$(RPMDIR)/$(RPMARCH)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-$(PACKAGE_RELEASE).$(RPMARCH).rpm \
	$(RPMDIR)/$(RPMARCH)/$(PACKAGE_NAME)-c++-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-$(PACKAGE_RELEASE).$(RPMARCH).rpm \
	$(RPMDIR)/$(RPMARCH)/$(PACKAGE_NAME)-c++-devel-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-$(PACKAGE_RELEASE).$(RPMARCH).rpm \
	$(RPMDIR)/$(RPMARCH)/$(PACKAGE_NAME)-devel-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-$(PACKAGE_RELEASE).$(RPMARCH).rpm \
	$(RPMDIR)/$(RPMARCH)/$(PACKAGE_NAME)-perl-$(PACKAGE_VERSION)$(PACKAGE_VERSION_ADDENDUM)-$(PACKAGE_RELEASE).$(RPMARCH).rpm

.PHONY: rpm
rpm:	dist
	rm -rf $(RPMDIR)
	$(MKDIR_P) $(RPMDIR)
	$(MKDIR_P) $(RPMDIR)/BUILD
	$(MKDIR_P) $(RPMDIR)/RPMS
	$(RPM) --define="_sourcedir `pwd`" --define="_rpmdir `pwd`/$(RPMDIR)/RPMS" --define="_builddir `pwd`/$(RPMDIR)/BUILD" --nodeps -bb ImageMagick.spec
	@echo ==============================================================
	@echo $(DIST_ARCHIVE_RPM) is ready for distribution.
	@echo ==============================================================

else
DIST_ARCHIVE_RPM=
endif # RPM_DELEGATE

#
# Build a validated snapshot release and move to the snapshots directory.
#
snapshot: distcheck
	$(MAKE) $(DIST_ARCHIVE_SRPM)
	$(MAKE) $(DIST_WINDOWS_SRC_ZIP)
	$(MAKE) $(DIST_WINDOWS_SRC_7ZIP)
	mv $(DIST_ARCHIVES) $(DIST_WINDOWS_SRC_ZIP) $(DIST_WINDOWS_SRC_7ZIP) $(DIST_ARCHIVE_SRPM) /var/ftp/pub/ImageMagick/beta/
