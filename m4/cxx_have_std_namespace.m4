dnl @synopsis AC_CXX_HAVE_STD_NAMESPACE
dnl
dnl If the compiler supports the std namespace, define
dnl HAVE_STD_NAMESPACE.
dnl
dnl @category Cxx
dnl @author Todd Veldhuizen
dnl @author Luc Maisonobe <luc@spaceroots.org>
dnl @version 2004-02-04
dnl @license AllPermissive

AC_DEFUN([AC_CXX_HAVE_STD_NAMESPACE],
[AC_CACHE_CHECK(whether the compiler supports the std namespace,
ac_cv_cxx_have_std_namespace,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <iostream>
	std::istream& is = std::cin;
	],[return 0;],
 ac_cv_cxx_have_std_namespace=yes, ac_cv_cxx_have_std_namespace=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std_namespace" = yes; then
  AC_DEFINE(HAVE_STD_NAMESPACE,,[define if the compiler supports the std namespace])
fi
])
