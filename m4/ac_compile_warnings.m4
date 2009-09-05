dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/ac_compile_warnings.html
dnl
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
