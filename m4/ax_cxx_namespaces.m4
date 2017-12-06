# ===========================================================================
#    https://www.gnu.org/software/autoconf-archive/ax_cxx_namespaces.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CXX_NAMESPACES
#
# DESCRIPTION
#
#   If the compiler can prevent names clashes using namespaces, define
#   HAVE_NAMESPACES.
#
# LICENSE
#
#   Copyright (c) 2008 Todd Veldhuizen
#   Copyright (c) 2008 Luc Maisonobe <luc@spaceroots.org>
#   Copyright (c) 2013 Bastien Roucaries <roucaries.bastien+autoconf@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 9

AU_ALIAS([AC_CXX_NAMESPACES], [AX_CXX_NAMESPACES])
AC_DEFUN([AX_CXX_NAMESPACES],
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ax_cv_cxx_namespaces,
[AC_LANG_PUSH([C++])
 AC_COMPILE_IFELSE([AC_LANG_SOURCE([namespace Outer { namespace Inner { int i = 0; }}
                                   using namespace Outer::Inner; int foo(void) { return i;} ])],
                   ax_cv_cxx_namespaces=yes, ax_cv_cxx_namespaces=no)
 AC_LANG_POP
])
if test "$ax_cv_cxx_namespaces" = yes; then
  AC_DEFINE(HAVE_NAMESPACES,,[define if the compiler implements namespaces])
fi
])
