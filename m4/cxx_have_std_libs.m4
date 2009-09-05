dnl @synopsis AC_CXX_HAVE_STD_LIBS
dnl
dnl If the compiler supports ISO C++ standard library (i.e., can
dnl include the files iostream, map, iomanip and cmath}), define
dnl HAVE_STD_LIBS.
dnl
dnl @category Cxx
dnl @author Todd Veldhuizen
dnl @author Luc Maisonobe <luc@spaceroots.org>
dnl @version 2004-02-04
dnl @license AllPermissive

AC_DEFUN([AC_CXX_HAVE_STD_LIBS],
[AC_CACHE_CHECK(whether the compiler supports ISO C++ standard library,
ac_cv_cxx_have_std_libs,
[AC_REQUIRE([AC_CXX_HAVE_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <iostream>
#include <map>
#include <iomanip>
#include <cmath>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[return 0;],
 ac_cv_cxx_have_std_libs=yes, ac_cv_cxx_have_std_libs=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std_libs" = yes; then
  AC_DEFINE(HAVE_STD_LIBS,,[define if the compiler supports ISO C++ standard library])
fi
])
