# ===========================================================================
#   https://www.gnu.org/software/autoconf-archive/ax_cxx_namespace_std.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CXX_NAMESPACE_STD
#
# DESCRIPTION
#
#   If the compiler supports namespace std, define HAVE_NAMESPACE_STD.
#
# LICENSE
#
#   Copyright (c) 2009 Todd Veldhuizen
#   Copyright (c) 2009 Luc Maisonobe <luc@spaceroots.org>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 8

AU_ALIAS([AC_CXX_NAMESPACE_STD], [AX_CXX_NAMESPACE_STD])
AC_DEFUN([AX_CXX_NAMESPACE_STD], [
  AC_CACHE_CHECK(if g++ supports namespace std,
  ax_cv_cxx_have_std_namespace,
  [AC_LANG_PUSH([C++])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <iostream>
                  std::istream& is = std::cin;]], [])],
  [ax_cv_cxx_have_std_namespace=yes], [ax_cv_cxx_have_std_namespace=no])
  AC_LANG_POP([C++])
  ])
  if test "$ax_cv_cxx_have_std_namespace" = yes; then
    AC_DEFINE(HAVE_NAMESPACE_STD,,[Define if g++ supports namespace std. ])
  fi
])
