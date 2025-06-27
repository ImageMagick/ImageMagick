dnl ax_check_framework.m4
dnl
dnl COPYRIGHT (c) 2008 John Reppy (http://cs.uchicago.edu/~jhr)
dnl All rights reserved.
dnl
dnl @synopsis AX_CHECK_FRAMEWORK(framework, [action-if-found], [action-if-not-found])
dnl
dnl This macro is similar to the AC_CHECK_LIB macro, except that it works
dnl for macOS frameworks.  It checks to see if the given framework exists
dnl on the host system and if so prepends "-framework framework" to the FRAMEWORKS
dnl variable.
dnl 
dnl @author John Reppy <http://cs.uchicago.edu/~jhr>
dnl
AC_DEFUN([AX_CHECK_FRAMEWORK],[
  AC_REQUIRE([AC_CANONICAL_HOST])dnl
  case x"$host_os" in
    xdarwin*)
      AS_VAR_PUSHDEF([ax_Check], [ax_cv_check_$1])
      AC_CACHE_CHECK([for framework $1], [ax_Check],[
	AC_LANG_PUSH([C])
	ac_check_framework_save_LIBS=$LIBS
	LIBS="-framework $1 $LIBS"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])], [ax_Check="yes"], [ax_Check="no"])
	AC_LANG_POP([C])dnl
      ])
      if test x"$ax_Check" = "xyes" ; then
	m4_ifvaln([$2],[$2],[:])dnl
      else
	m4_ifvaln([$3],[$3],[:])dnl
      fi
      LIBS=$ac_check_framework_save_LIBS
      AS_VAR_POPDEF([ax_Check])dnl
    ;;
  esac
])dnl
