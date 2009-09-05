## Process this file with automake to produce Makefile.in
##
## Copyright (C) 2004 Free Software Foundation
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; see the file COPYING.  If not, write to
## the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
## Boston, MA 02111-1307, USA.

BUILT_SOURCES		=
MOSTLYCLEANFILES	=
EXTRA_DIST		=

AUTOMAKE_OPTIONS	= foreign

DEFS			= -DHAVE_CONFIG_H="<$(CONFIG_H)>" -DLTDL
AM_CPPFLAGS		= -I$(top_builddir) -I$(top_srcdir)  \
			  -I.. -I$(srcdir)/.. -I$(srcdir)/../libltdl
AM_LDFLAGS		= -no-undefined -module -avoid-version -export-dynamic

pkgincludedir		= $(includedir)/libltdl


## The loaders are preopened by libltdl, itself always built from
## pic-objects (either as a shared library, or a convenience library),
## so the loaders themselves must be made from pic-objects too.  We
## use convenience libraries for that purpose:
noinst_LTLIBRARIES	= $(LT_DLLOADERS)
EXTRA_LTLIBRARIES	= dlopen.la dld_link.la dyld.la load_add_on.la \
			  loadlibrary.la shl_load.la

## Build loaders (other than preopen) as modules:
dlopen_la_LIBADD 	= ../libdlloader.la $(LIBADD_DLOPEN)
shl_load_la_LIBADD	= ../libdlloader.la $(LIBADD_SHL_LOAD)
dyld_la_LIBADD		= ../libdlloader.la
load_add_on_la_LIBADD	= ../libdlloader.la
loadlibrary_la_LIBADD	= ../libdlloader.la
dld_link_la_LIBADD	= ../libdlloader.la -ldld


## These are installed as a subdirectory of pkgdatadir so that
## libtoolize --ltdl can find them later:
ltdldatadir		= $(pkgdatadir)/libltdl/loaders
ltdldata_DATA		= Makefile.am dld_link.c dlopen.c dyld.c \
			  load_add_on.c loadlibrary.c shl_load.c

../libdlloader.la:
	cd ..; $(MAKE) $(AM_MAKEFLAGS) libdlloader.la
