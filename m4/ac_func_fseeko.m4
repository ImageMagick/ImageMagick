# This file is part of Autoconf.                        -*- Autoconf -*-
# Checking for functions.
# Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Free Software
# Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#
# As a special exception, the Free Software Foundation gives unlimited
# permission to copy, distribute and modify the configure scripts that
# are the output of Autoconf.  You need not follow the terms of the GNU
# General Public License when using or distributing such scripts, even
# though portions of the text of Autoconf appear in them.  The GNU
# General Public License (GPL) does govern all other use of the material
# that constitutes the Autoconf program.
#
# Certain portions of the Autoconf source text are designed to be copied
# (in certain cases, depending on the input) into the output of
# Autoconf.  We call these the "data" portions.  The rest of the Autoconf
# source text consists of comments plus executable code that decides which
# of the data portions to output in any given case.  We call these
# comments and executable code the "non-data" portions.  Autoconf never
# copies any of the non-data portions into its output.
#
# This special exception to the GPL applies to versions of Autoconf
# released by the Free Software Foundation.  When you make and
# distribute a modified version of Autoconf, you may extend this special
# exception to the GPL to apply to your modified version as well, *unless*
# your modified version has the potential to copy into its output some
# of the text that was the non-data portion of the version that you started
# with.  (In other words, unless your change moves or copies text from
# the non-data portions to the data portions.)  If your modification has
# such potential, you must delete any notice of this special exception
# to the GPL from your modified version.
#
# Written by David MacKenzie, with help from
# Franc,ois Pinard, Karl Berry, Richard Pixley, Ian Lance Taylor,
# Roland McGrath, Noah Friedman, david d zuhn, and many others.

# AC_FUNC_FSEEKO
# --------------
AN_FUNCTION([ftello], [AC_FUNC_FSEEKO])
AN_FUNCTION([fseeko], [AC_FUNC_FSEEKO])
AC_DEFUN([AC_FUNC_FSEEKO],
[_AC_SYS_LARGEFILE_MACRO_VALUE(_LARGEFILE_SOURCE, 1,
   [ac_cv_sys_largefile_source],
   [Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2).],
   [[#include <sys/types.h> /* for off_t */
     #include <stdio.h>]],
   [[int (*fp) (FILE *, off_t, int) = fseeko;
     return fseeko (stdin, 0, 0) && fp (stdin, 0, 0);]])

# We used to try defining _XOPEN_SOURCE=500 too, to work around a bug
# in glibc 2.1.3, but that breaks too many other things.
# If you want fseeko and ftello with glibc, upgrade to a fixed glibc.
if test $ac_cv_sys_largefile_source != unknown; then
  AC_DEFINE(HAVE_FSEEKO, 1,
    [Define to 1 if fseeko (and presumably ftello) exists and is declared.])
fi
])# AC_FUNC_FSEEKO
