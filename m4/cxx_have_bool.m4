dnl @synopsis AC_CXX_HAVE_BOOL
dnl
dnl If the compiler recognizes bool as a separate built-in type, define
dnl HAVE_BOOL. Note that a typedef is not a separate type since you
dnl cannot overload a function such that it accepts either the basic
dnl type or the typedef.
dnl
dnl @category Cxx
dnl @author Todd Veldhuizen
dnl @author Luc Maisonobe <luc@spaceroots.org>
dnl @version 2004-02-04
dnl @license AllPermissive

AC_DEFUN([AC_CXX_HAVE_BOOL],
[AC_CACHE_CHECK(whether the compiler recognizes bool as a built-in type,
ac_cv_cxx_have_bool,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
int f(int  x){return 1;}
int f(char x){return 1;}
int f(bool x){return 1;}
],[bool b = true; return f(b);],
 ac_cv_cxx_have_bool=yes, ac_cv_cxx_have_bool=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_bool" = yes; then
  AC_DEFINE(HAVE_BOOL,,[define if bool is a built-in type])
fi
])
