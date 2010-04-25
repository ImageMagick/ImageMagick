# ===========================================================================
#   http://www.gnu.org/software/autoconf-archive/ax_prog_perl_version.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_PROG_PERL_VERSION([VERSION],[ACTION-IF-TRUE],[ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   Makes sure that perl supports the version indicated. If true the shell
#   commands in ACTION-IF-TRUE are executed. If not the shell commands in
#   ACTION-IF-FALSE are run. Note if $PERL is not set (for example by
#   running AC_CHECK_PROG or AC_PATH_PROG) the macro will fail.
#
#   Example:
#
#     AC_PATH_PROG([PERL],[perl])
#     AX_PROG_PERL_VERSION([5.8.0],[ ... ],[ ... ])
#
#   This will check to make sure that the perl you have supports at least
#   version 5.8.0.
#
#   NOTE: This macro uses the $PERL variable to perform the check.
#   AX_WITH_PERL can be used to set that variable prior to running this
#   macro. The $PERL_VERSION variable will be valorized with the detected
#   version.
#
# LICENSE
#
#   Copyright (c) 2009 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 11

AC_DEFUN([AX_PROG_PERL_VERSION],[
    AC_REQUIRE([AC_PROG_SED])
    AC_REQUIRE([AC_PROG_GREP])

    AS_IF([test -n "$PERL"],[
        ax_perl_version="$1"

        AC_MSG_CHECKING([for perl version])
        changequote(<<,>>)
        perl_version=`$PERL --version 2>&1 | $GREP "This is perl" | $SED -e 's/.* v\([0-9]*\.[0-9]*\.[0-9]*\) .*/\1/'`
        changequote([,])
        AC_MSG_RESULT($perl_version)

	AC_SUBST([PERL_VERSION],[$perl_version])

        AX_COMPARE_VERSION([$ax_perl_version],[le],[$perl_version],[
	    :
            $2
        ],[
	    :
            $3
        ])
    ],[
        AC_MSG_WARN([could not find the perl interpreter])
        $3
    ])
])
