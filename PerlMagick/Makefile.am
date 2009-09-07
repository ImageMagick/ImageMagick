#  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
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
#  Makefile for building PerlMagick.

# If source files missing, see if they can be obtained via VPATH
perl-sources:
	@if test -n "$(VPATH)" ; then \
	  echo "Linking PerlMagick Sources ..." ; \
	  imagemagick=`(cd $(VPATH) ; pwd)` && \
	  ( cd $(PERLMAGICK) && \
	    sh $$imagemagick/config/lndir.sh $$imagemagick/$(PERLMAGICK) ) \
	fi ; \
	touch perl-sources

if WITH_PERL

PERLMAGICK=PerlMagick
PERLMAKEMAKER=$(PERLMAGICK)/Makefile.PL
PERLMAKEFILE=$(PERLMAGICK)/Makefile

PERLMAGICK_ALL_LOCAL_TARGETS = all-perl
PERLMAGICK_INSTALL_EXEC_LOCAL_TARGETS = install-exec-perl
PERLMAGICK_INSTALL_DATA_LOCAL_TARGETS = 
PERLMAGICK_UNINSTALL_LOCAL_TARGETS = uninstall-exec-perl
PERLMAGICK_CLEAN_LOCAL_TARGETS = clean-perl
PERLMAGICK_DISTCLEAN_LOCAL_TARGETS = clean-perl
PERLMAGICK_MAINTAINER_CLEAN_LOCAL_TARGETS = distclean-local
PERLMAGICK_TESTS = PerlMagick/check.sh

if WITH_PERL_DYNAMIC

$(PERLMAKEFILE): perl-sources $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(PERLMAKEMAKER)
	cd $(PERLMAGICK) && @PERL@ Makefile.PL $(PERL_MAKE_OPTIONS)

install-exec-perl: $(PERLMAKEFILE)
	( cd $(PERLMAGICK) && $(MAKE) CC='@CC@' && \
	$(MAKE) CC='@CC@' install && \
	$(MAKE) clean && rm -f  Makefile.old )

all-perl: perl-sources

uninstall-exec-perl: $(PERLMAKEFILE)
	echo "Uninstall not supported for PerlMagick"

check-perl: $(PERLMAKEFILE)
	cd $(PERLMAGICK) && $(MAKE) CC='@CC@' test

perl-build: $(PERLMAKEFILE)
	( cd $(PERLMAGICK) && $(MAKE) CC='@CC@' )

else
if WITH_PERL_STATIC

PERLSTATICNAME=PerlMagick

PERLMAGICK_CHECKSCRIPTS = perl-build

$(PERLMAKEFILE): perl-sources $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(PERLMAKEMAKER)
	cd $(PERLMAGICK) && @PERL@ Makefile.PL MAP_TARGET=$(PERLSTATICNAME) $(PERL_MAKE_OPTIONS) && $(MAKE) Makefile ; $(MAKE) Makefile

$(PERLMAGICK)/$(PERLSTATICNAME): $(MAGICKCORE_LIBS) $(MAGICKWAND_LIBS) $(PERLMAKEFILE)
	( rm -f $(PERLMAGICK)/$(PERLSTATICNAME) ; cd $(PERLMAGICK) && $(MAKE) CC='@CC@' $(PERLSTATICNAME) ; $(MAKE) CC='@CC@' $(PERLSTATICNAME) )

all-perl: $(PERLMAGICK)/$(PERLSTATICNAME)

install-exec-perl: $(PERLMAGICK)/$(PERLSTATICNAME)
	rm -f "$(DESTDIR)$(BIN_DIR)/$(PERLSTATICNAME)"
	if test "x$(DESTDIR)" = "x" -o "$(PERL_SUPPORTS_DESTDIR)" = 'yes' ; then \
	  ( cd $(PERLMAGICK) && \
	    $(MAKE) -f Makefile.aperl CC='@CC@' inst_perl MAP_TARGET=$(PERLSTATICNAME) \
	    INSTALLBIN="$(BIN_DIR)" \
	  ) ; \
	else \
	  ( cd $(PERLMAGICK) && \
	    $(MAKE) -f Makefile.aperl CC='@CC@' inst_perl MAP_TARGET=$(PERLSTATICNAME) \
	    INSTALLBIN="$(DESTDIR)$(BIN_DIR)" PREFIX="$(DESTDIR)$(prefix)" \
	  ) ; \
	fi

uninstall-exec-perl:
	rm -f '$(DESTDIR)$(BIN_DIR)/$(PERLSTATICNAME)'

check-perl: $(PERLMAGICK)/$(PERLSTATICNAME)
	cd $(PERLMAGICK) && $(MAKE) -f Makefile.aperl CC='@CC@' test

perl-build: $(PERLMAGICK)/$(PERLSTATICNAME)

endif # WITH_PERL_STATIC
endif # WTIH_PERL_DYNAMIC


clean-perl:
	(cd $(PERLMAGICK) && \
	( if test -f Makefile.old ; then $(MAKE) -f Makefile.old CC='@CC@' clean ; fi ) ; \
	( if test -f Makefile ; then $(MAKE) CC='@CC@' clean ; fi ) ; \
	( if test -f Makefile ; then $(MAKE) CC='@CC@' clean ; fi ) ; \
	rm -f Makefile.old PerlMagick ; \
	rm -f t/output* t/jng/*_tmp.jng t/*/output* )
	rm -f perl-sources

distclean-perl: clean-perl

else
# Satisfy makefile requirements if not building PERL
all-perl:
install-exec-perl:
uninstall-exec-perl:
check-perl:
clean-perl:
distclean-perl:
endif # WITH_PERL
