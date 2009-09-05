dnl @synopsis AC_CXX_HAVE_LSTRING
dnl
dnl If the compiler can prevent names clashes using namespaces, define
dnl HAVE_LSTRING.
dnl
dnl @category Cxx
dnl @author James Berry
dnl @version 2005-02-21
dnl @license AllPermissive

AC_DEFUN([AC_CXX_HAVE_LSTRING],
[AC_CACHE_CHECK([whether the compiler implements L"widestring"],
ac_cv_cxx_have_lstring,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_COMPILE_IFELSE(
	AC_LANG_SOURCE(
		[[const wchar_t* s=L"wide string";]]),
 	ac_cv_cxx_have_lstring=yes, ac_cv_cxx_have_lstring=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_lstring" = yes; then
  AC_DEFINE(HAVE_LSTRING,,[define if the compiler implements L"widestring"])
fi
])
