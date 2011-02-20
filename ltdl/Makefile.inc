## Makefile.inc -- includable Makefile.am snippet
##
##   Copyright (C) 2003, 2004, 2005, 2007 Free Software Foundation, Inc.
##   Written by Gary V. Vaughan, 2003
##
##   NOTE: The canonical source of this file is maintained with the
##   GNU Libtool package.  Report bugs to bug-libtool@gnu.org.
##
## GNU Libltdl is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## As a special exception to the GNU Lesser General Public License,
## if you distribute this file as part of a program or library that
## is built using GNU libtool, you may include this file under the
## same distribution terms that you use for the rest of that program.
##
## GNU Libltdl is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU LesserGeneral Public
## License along with GNU Libltdl; see the file README.  If not, a
## copy can be downloaded from http://www.gnu.org/licenses/lgpl.html,
## or obtained by writing to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#####

## DO NOT REMOVE THIS LINE -- make depends on it

# -I$(srcdir) is needed for user that built libltdl with a sub-Automake
# (not as a sub-package!) using 'nostdinc':
AM_CPPFLAGS	       += -DLT_CONFIG_H='<$(LT_CONFIG_H)>' \
			  -DLTDL -I. -I$(srcdir) -Iltdl \
			  -I$(srcdir)/ltdl -I$(srcdir)/ltdl/libltdl
AM_LDFLAGS	       += -no-undefined
LTDL_VERSION_INFO	= -version-info 10:0:3

noinst_LTLIBRARIES	+= $(LT_DLLOADERS)

if INSTALL_LTDL
ltdlincludedir		= $(includedir)/libltdl
ltdlinclude_HEADERS	= ltdl/libltdl/lt_system.h \
			  ltdl/libltdl/lt_error.h \
			  ltdl/libltdl/lt_dlloader.h
include_HEADERS	       += ltdl/ltdl.h
lib_LTLIBRARIES	       += ltdl/libltdl.la
endif

if CONVENIENCE_LTDL
noinst_LTLIBRARIES     += ltdl/libltdlc.la
endif

ltdl_libltdl_la_SOURCES = ltdl/libltdl/lt__alloc.h \
			  ltdl/libltdl/lt__dirent.h \
			  ltdl/libltdl/lt__glibc.h \
			  ltdl/libltdl/lt__private.h \
			  ltdl/libltdl/lt__strl.h \
			  ltdl/libltdl/lt_dlloader.h \
			  ltdl/libltdl/lt_error.h \
	                  ltdl/libltdl/lt_system.h \
			  ltdl/libltdl/slist.h \
			  ltdl/loaders/preopen.c \
			  ltdl/lt__alloc.c \
			  ltdl/lt_dlloader.c \
			  ltdl/lt_error.c \
			  ltdl/ltdl.c \
			  ltdl/ltdl.h \
			  ltdl/slist.c

EXTRA_DIST	       += ltdl/lt__dirent.c \
			  ltdl/lt__strl.c

ltdl_libltdl_la_CPPFLAGS	= -DLTDLOPEN=$(LTDLOPEN) $(AM_CPPFLAGS)
ltdl_libltdl_la_LDFLAGS	= $(AM_LDFLAGS) $(LTDL_VERSION_INFO) $(LT_DLPREOPEN)
ltdl_libltdl_la_LIBADD	= $(ltdl_LTLIBOBJS)
ltdl_libltdl_la_DEPENDENCIES	= $(LT_DLLOADERS) $(ltdl_LTLIBOBJS)

ltdl_libltdlc_la_SOURCES	= $(ltdl_libltdl_la_SOURCES)
ltdl_libltdlc_la_CPPFLAGS	= -DLTDLOPEN=$(LTDLOPEN)c $(AM_CPPFLAGS)
ltdl_libltdlc_la_LDFLAGS	= $(AM_LDFLAGS) $(LT_DLPREOPEN)
ltdl_libltdlc_la_LIBADD	= $(ltdl_libltdl_la_LIBADD)
ltdl_libltdlc_la_DEPENDENCIES= $(ltdl_libltdl_la_DEPENDENCIES)

## The loaders are preopened by libltdl, itself always built from
## pic-objects (either as a shared library, or a convenience library),
## so the loaders themselves must be made from pic-objects too.  We
## use convenience libraries for that purpose:
EXTRA_LTLIBRARIES	       += ltdl/dlopen.la \
				  ltdl/dld_link.la \
				  ltdl/dyld.la \
				  ltdl/load_add_on.la \
				  ltdl/loadlibrary.la \
				  ltdl/shl_load.la

ltdl_dlopen_la_SOURCES	= ltdl/loaders/dlopen.c
ltdl_dlopen_la_LDFLAGS	= -module -avoid-version
ltdl_dlopen_la_LIBADD 	= $(LIBADD_DLOPEN)

ltdl_dld_link_la_SOURCES	= ltdl/loaders/dld_link.c
ltdl_dld_link_la_LDFLAGS	= -module -avoid-version
ltdl_dld_link_la_LIBADD	= -ldld

ltdl_dyld_la_SOURCES		= ltdl/loaders/dyld.c
ltdl_dyld_la_LDFLAGS		= -module -avoid-version

ltdl_load_add_on_la_SOURCES	= ltdl/loaders/load_add_on.c
ltdl_load_add_on_la_LDFLAGS	= -module -avoid-version

ltdl_loadlibrary_la_SOURCES	= ltdl/loaders/loadlibrary.c
ltdl_loadlibrary_la_LDFLAGS	= -module -avoid-version

ltdl_shl_load_la_SOURCES	= ltdl/loaders/shl_load.c
ltdl_shl_load_la_LDFLAGS	= -module -avoid-version
ltdl_shl_load_la_LIBADD	= $(LIBADD_SHL_LOAD)

## Make sure these will be cleaned even when they're not built by default:
CLEANFILES		       += ltdl/libltdl.la \
				  ltdl/libltdlc.la \
				  ltdl/libdlloader.la

## Automake-1.9.6 doesn't clean subdir AC_LIBOBJ compiled objects
## automatically:
CLEANFILES	       += $(ltdl_LIBOBJS) $(ltdl_LTLIBOBJS)

EXTRA_DIST	       += ltdl/README \
			  ltdl/README

## --------------------------- ##
## Gnulib Makefile.am snippets ##
## --------------------------- ##

BUILT_SOURCES	+= ltdl/$(ARGZ_H)
EXTRA_DIST	+= ltdl/argz_.h \
		   ltdl/argz.c

# We need the following in order to create an <argz.h> when the system
# doesn't have one that works with the given compiler.
all-local $(lib_OBJECTS): ltdl/$(ARGZ_H)
ltdl/argz.h: ltdl/argz_.h
	$(mkinstalldirs) . ltdl/
	cp $(srcdir)/ltdl/argz_.h $@-t
	mv $@-t $@
MOSTLYCLEANFILES += ltdl/argz.h \
		    ltdl/argz.h-t
