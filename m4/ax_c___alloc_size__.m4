# ===========================================================================
#    http://www.gnu.org/software/autoconf-archive/ax_c___alloc_size__.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_C___ALLOC_SIZE__
#
# DESCRIPTION
#
#   Provides a test for the compiler support of __alloc_size__ extensions.
#   Defines HAVE___ALLOC_SIZE__ if it is found.
#
# LICENSE
#
#   Copyright (c) 2008 Stepan Kasal <skasal@redhat.com>
#   Copyright (c) 2008 Christian Haggstrom
#   Copyright (c) 2008 Ryan McCabe <ryan@numb.org>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

#serial 8

AC_DEFUN([AX_C___ALLOC_SIZE__], [
  AC_CACHE_CHECK([for __alloc_size__], [ax_cv___alloc_size__],
    [AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM(
	[[#include <stdlib.h>
	  static void foo(const size_t) __attribute__((__alloc_size__(1)));
	  static void
	  foo(const size_t bar) {
	      exit(1);
	  }
        ]], [])],
      [ax_cv___alloc_size__=yes],
      [ax_cv___alloc_size__=no]
    )
  ])
  if test "$ax_cv___alloc_size__" = "yes"; then
    AC_DEFINE([HAVE___ALLOC_SIZE__], 1, [define if your compiler has __alloc_size__])
  fi
])
