# ===========================================================================
#    http://www.gnu.org/software/autoconf-archive/ax_lang_compiler_ms.html
# ===========================================================================
#
# OBSOLETE MACRO
#
#   Deprecated in favor of AX_COMPILER_VENDOR. A call to this macro can be
#   replaced by:
#
#     AX_COMPILER_VENDOR
#     AS_IF([test $ax_cv_c_compiler_vendor = microsoft],
#         [ax_compiler_ms=yes],[ax_compiler_ms=no])
#
# SYNOPSIS
#
#   AX_LANG_COMPILER_MS
#
# DESCRIPTION
#
#   Check whether the compiler for the current language is Microsoft.
#
#   This macro is modeled after _AC_LANG_COMPILER_GNU in the GNU Autoconf
#   implementation.
#
# LICENSE
#
#   Copyright (c) 2009 Braden McDaniel <braden@endoframe.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 11

AC_DEFUN([AX_LANG_COMPILER_MS],
[AC_CACHE_CHECK([whether we are using the Microsoft _AC_LANG compiler],
                [ax_cv_[]_AC_LANG_ABBREV[]_compiler_ms],
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[#ifndef _MSC_VER
       choke me
#endif
]])],
                   [ax_compiler_ms=yes],
                   [ax_compiler_ms=no])
ax_cv_[]_AC_LANG_ABBREV[]_compiler_ms=$ax_compiler_ms
])])
