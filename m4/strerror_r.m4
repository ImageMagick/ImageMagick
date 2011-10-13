#serial 1003
# Experimental replacement for the function in the latest CVS autoconf.
# Use with the error.c file in ../lib.

# Copyright 2001 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

undefine([AC_FUNC_STRERROR_R])

# AC_FUNC_STRERROR_R
# ------------------
AC_DEFUN([AC_FUNC_STRERROR_R],
[AC_CHECK_DECLS([strerror_r])
AC_CHECK_FUNCS([strerror_r])
AC_CACHE_CHECK([whether strerror_r returns char *],
               ac_cv_func_strerror_r_char_p,
   [
    ac_cv_func_strerror_r_char_p=no
    if test $ac_cv_have_decl_strerror_r = yes; then
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT],
	[[
	  char buf[100];
	  char x = *strerror_r (0, buf, sizeof buf);
	  char *p = strerror_r (0, buf, sizeof buf);
	]])],
			ac_cv_func_strerror_r_char_p=yes)
    else
      # strerror_r is not declared.  Choose between
      # systems that have relatively inaccessible declarations for the
      # function.  BeOS and DEC UNIX 4.0 fall in this category, but the
      # former has a strerror_r that returns char*, while the latter
      # has a strerror_r that returns `int'.
      # This test should segfault on the DEC system.
      AC_RUN_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT
	extern char *strerror_r ();],
	[[char buf[100];
	  char x = *strerror_r (0, buf, sizeof buf);
	  exit (!isalpha (x));]])],
                    ac_cv_func_strerror_r_char_p=yes, , :)
    fi
  ])
if test $ac_cv_func_strerror_r_char_p = yes; then
  AC_DEFINE([STRERROR_R_CHAR_P], 1,
	    [Define to 1 if strerror_r returns char *.])
fi
])# AC_FUNC_STRERROR_R
