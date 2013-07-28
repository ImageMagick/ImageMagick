# SYNOPSIS
#
#   MAGICK_CHECK_PTHREAD_LIB(LIBRARY,
#              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Macro to test for pthread library
#   Written by Bob Friesenhahn based on test in ACX_PTHREAD
#
# LICENSE
#
#   Copyright (c) 2008 Steven G. Johnson <stevenj@alum.mit.edu>
#
#   This program is free software: you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation, either version 3 of the License, or (at your
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

AC_DEFUN([MAGICK_CHECK_PTHREAD_LIB], [
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_SAVE
AC_LANG_C
magick_pthread_lib_ok=no

LIB=-l$1
save_LIBS="$LIBS"
LIBS="$LIBS $LIB"

AC_MSG_CHECKING([for the pthreads library $LIB])
AC_TRY_LINK([#include <pthread.h>],
  [  pthread_t th;
  pthread_join(th, 0);
  pthread_attr_init(0);
  pthread_cleanup_push(0, 0);
  pthread_create(0,0,0,0);
  pthread_cleanup_pop(0); ],
  [magick_pthread_lib_ok=yes])

AC_MSG_RESULT(${magick_pthread_lib_ok})
if test "$magick_pthread_lib_ok" = yes
then
  $2
  :
else
  $3
  :
fi

LIBS="$save_LIBS"

AC_LANG_RESTORE
])dnl MAGICK_CHECK_PTHREAD_LIB

