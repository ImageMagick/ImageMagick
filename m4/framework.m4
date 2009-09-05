# AC_CHECK_FRAMEWORK(FRAMEWORK, FUNCTION,
#              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#              [OTHER-LIBRARIES])
# ------------------------------------------------------
#
# Use a cache variable name containing both the framework and function name,
# because the test really is for framework $1 defining function $2, not
# just for framework $1.  Separate tests with the same $1 and different $2s
# may have different results.
#
# Note that using directly AS_VAR_PUSHDEF([ac_Framework], [ac_cv_framework_$1_$2])
# is asking for troubles, since AC_CHECK_FRAMEWORK($framework, fun) would give
# ac_cv_framework_$framework_fun, which is definitely not what was meant.  Hence
# the AS_LITERAL_IF indirection.
#
# FIXME: This macro is extremely suspicious.  It DEFINEs unconditionally,
# whatever the FUNCTION, in addition to not being a *S macro.  Note
# that the cache does depend upon the function we are looking for.
#
# It is on purpose we used `ac_check_framework_save_LIBS' and not just
# `ac_save_LIBS': there are many macros which don't want to see `LIBS'
# changed but still want to use AC_CHECK_FRAMEWORK, so they save `LIBS'.
# And ``ac_save_LIBS' is too tempting a name, so let's leave them some
# freedom.
AC_DEFUN([AC_CHECK_FRAMEWORK],
[m4_ifval([$3], , [AH_CHECK_FRAMEWORK([$1])])dnl
AS_LITERAL_IF([$1],
         [AS_VAR_PUSHDEF([ac_Framework], [ac_cv_framework_$1_$2])],
         [AS_VAR_PUSHDEF([ac_Framework], [ac_cv_framework_$1''_$2])])dnl
AC_CACHE_CHECK([for $2 in $1 framework], ac_Framework,
[ac_check_framework_save_LIBS=$LIBS
LIBS="-framework $1 $5 $LIBS"
AC_LINK_IFELSE([AC_LANG_CALL([], [$2])],
          [AS_VAR_SET(ac_Framework, yes)],
          [AS_VAR_SET(ac_Framework, no)])
LIBS=$ac_check_framework_save_LIBS])
AS_IF([test AS_VAR_GET(ac_Framework) = yes],
      [m4_default([$3], [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_FRAMEWORK_$1))
  LIBS="-framework $1 $LIBS"
])],
      [$4])dnl
AS_VAR_POPDEF([ac_Framework])dnl
])# AC_CHECK_FRAMEWORK

# AH_CHECK_FRAMEWORK(FRAMEWORK)
# ---------------------
m4_define([AH_CHECK_FRAMEWORK],
[AH_TEMPLATE(AS_TR_CPP(HAVE_FRAMEWORK_$1),
        [Define to 1 if you have the `]$1[' framework (-framework ]$1[).])])
