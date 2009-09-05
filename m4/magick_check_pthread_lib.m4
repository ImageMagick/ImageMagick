#
# Macro to test for pthread library
# Written by Bob Friesenhahn based on test in ACX_PTHREAD
# MAGICK_CHECK_PTHREAD_LIB(LIBRARY,
#              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
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

