# ===========================================================================
#       http://www.nongnu.org/autoconf-archive/ac_compile_warnings.html
# ===========================================================================
#
# OBSOLETE MACRO
#
#   Deprecated in favor of AX_CFLAGS_WARN_ALL.
#
# SYNOPSIS
#
#   AC_COMPILE_WARNINGS
#
# DESCRIPTION
#
#   Set the maximum warning verbosity according to C and C++ compiler used.
#   Currently supports g++ and gcc.
#
#   The compiler options are always added CFLAGS and CXXFLAGS even if these
#   are overidden at configure time. Removing the maximum warning flags can
#   be removed with --without-maximum-compile-warnings. For example:
#
#     ./configure --without-maximum-compile-warnings CFLAGS= CXXFLAGS=
#
# LICENSE
#
#   Copyright (c) 2008 Loic Dachary <loic@senga.org>
#   Copyright (c) 2008 William Fulton <wsf@fultondesigns.co.uk>
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

AC_DEFUN([AC_COMPILE_WARNINGS], [
AC_MSG_CHECKING([maximum warning verbosity option])
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH([maximum-compile-warnings],
              AS_HELP_STRING([--without-maximum-compile-warnings],
                             [Disable maximum warning verbosity]),
              [ac_compile_warnings_on="$withval"],
              [ac_compile_warnings_on=""])

  if test x"$ac_compile_warnings_on" = xno
  then
    ac_compile_warnings_msg=no
  else
    if test -n "$CXX"
    then
      if test "$GXX" = "yes"
      then
        ac_compile_warnings_opt='-Wall -W'
      fi
      CXXFLAGS="$CXXFLAGS $ac_compile_warnings_opt"
      ac_compile_warnings_msg="$ac_compile_warnings_opt for C++"
    fi

  if test -n "$CC"
  then
    if test "$GCC" = "yes"
    then
      ac_compile_warnings_opt='-Wall -W'
    fi
    CFLAGS="$CFLAGS $ac_compile_warnings_opt"
    ac_compile_warnings_msg="$ac_compile_warnings_msg $ac_compile_warnings_opt for C"
  fi
  fi
  AC_MSG_RESULT([$ac_compile_warnings_msg])
  unset ac_compile_warnings_msg
  unset ac_compile_warnings_opt
])
