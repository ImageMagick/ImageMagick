# -*- mode: autoconf -*-
#
# AX_OPENCL
#
# Check for an OpenCL implementation.  If CL is found, _OPENCL is defined and
# the required compiler and linker flags are included in the output variables
# "CL_CFLAGS" and "CL_LIBS", respectively.  If no usable CL implementation is
# found, "no_cl" is set to "yes".
#
# If the header "CL/OpenCL.h" is found, "HAVE_CL_OPENCL_H" is defined.  If the
# header "OpenCL/OpenCL.h" is found, HAVE_OPENCL_OPENCL_H is defined.  These
# preprocessor definitions may not be mutually exclusive.
#
# Based on AX_CHECK_GL, version: 2.4 author: Braden McDaniel
# <braden@endoframe.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#
# As a special exception, the you may copy, distribute and modify the
# configure scripts that are the output of Autoconf when processing
# the Macro.  You need not follow the terms of the GNU General Public
# License when using or distributing such scripts.
#
AC_DEFUN([AX_OPENCL],
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_PROG_SED])dnl
AC_REQUIRE([ACX_PTHREAD])dnl

AC_ARG_ENABLE([opencl],
    [AC_HELP_STRING([--enable-opencl],
                    [enable OpenCL support])],
    [enable_opencl=$enableval],
    [enable_opencl='no'])

if test "$enable_opencl" = 'yes'; then
  AC_LANG_PUSH([$1])
  AX_LANG_COMPILER_MS
  AS_IF([test X$ax_compiler_ms = Xno],
        [CL_CFLAGS="${PTHREAD_CFLAGS}"; CL_LIBS="${PTHREAD_LIBS} -lm"])
  
  ax_save_CPPFLAGS=$CPPFLAGS
  ax_save_CL_CFLAGS=$CL_CFLAGS

  found_opencl_header='no'
  CPPFLAGS="$CL_CFLAGS $CPPFLAGS"
  AC_CHECK_HEADERS([CL/cl.h OpenCL/cl.h],
                   [found_opencl_header='yes'
                    break;],
                   [found_opencl_header='no'])

  AS_IF([test X$found_opencl_header = Xno],
        [AS_UNSET([ac_cv_header_CL_cl_h])
         AS_UNSET([ac_cv_header_OpenCL_cl_h])
         CL_CFLAGS="-I$AMDAPPSDKROOT/include"
         CPPFLAGS="$ax_save_CPPFLAGS $CL_CFLAGS"
         AC_CHECK_HEADERS([CL/cl.h OpenCL/cl.h],
                          [found_opencl_header='yes'
                           break;],
                          [found_opencl_header='no'])
        ],
        [])

  CPPFLAGS="$ax_save_CPPFLAGS"
  
  AC_CHECK_HEADERS([windows.h])
  
  m4_define([AX_OPENCL_PROGRAM],
            [AC_LANG_PROGRAM([[
  # if defined(HAVE_WINDOWS_H) && defined(_WIN32)
  #   include <windows.h>
  # endif
  # ifdef HAVE_CL_CL_H
  #   include <CL/cl.h>
  # elif defined(HAVE_OPENCL_CL_H)
  #   include <OpenCL/cl.h>
  # else
  #   error no CL.h
  # endif]],
                             [[clCreateContextFromType(0,0,0,0,0)]])])
  
  AC_CACHE_CHECK([for OpenCL library], [ax_cv_check_cl_libcl],
  [ax_cv_check_cl_libcl=no
  case $host_cpu in
    x86_64) ax_check_cl_libdir=lib64
            ax_check_cl_amd_libdir=x86_64 
	    ;;
    *)      ax_check_cl_libdir=lib
            ax_check_cl_amd_libdir=x86 
            ;;
  esac
  ax_save_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$CL_CFLAGS $CPPFLAGS"
  ax_save_LIBS=$LIBS
  LIBS=""
  ax_check_libs="-lOpenCL -lCL -lclparser"
  for ax_lib in $ax_check_libs; do
    AS_IF([test X$ax_compiler_ms = Xyes],
          [ax_try_lib=`echo $ax_lib | $SED -e 's/^-l//' -e 's/$/.lib/'`],
          [ax_try_lib=$ax_lib])
    LIBS="$ax_try_lib $CL_LIBS $ax_save_LIBS"
  AC_LINK_IFELSE([AX_OPENCL_PROGRAM],
                 [ax_cv_check_cl_libcl=$ax_try_lib; break],
                 [ax_check_cl_nvidia_flags="-L/usr/$ax_check_cl_libdir/nvidia" LIBS="$ax_try_lib $ax_check_cl_nvidia_flags $CL_LIBS $ax_save_LIBS"
                 AC_LINK_IFELSE([AX_OPENCL_PROGRAM],
                                [ax_cv_check_cl_libcl="$ax_try_lib $ax_check_cl_nvidia_flags"; break],
                                [ax_check_cl_dylib_flag='-Wl,-framework,OpenCL -L/System/Library/Frameworks/OpenCL.framework/Versions/A/Libraries' LIBS="$ax_try_lib $ax_check_cl_dylib_flag $CL_LIBS $ax_save_LIBS"
                                AC_LINK_IFELSE([AX_OPENCL_PROGRAM],
                                               [ax_cv_check_cl_libcl="$ax_try_lib $ax_check_cl_dylib_flag"; break],
						                                   [ax_check_cl_amd_flags="-L$AMDAPPSDKROOT/lib/$ax_check_cl_amd_libdir" LIBS="$ax_try_lib $ax_check_cl_amd_flags $CL_LIBS $ax_save_LIBS"
						                                   AC_LINK_IFELSE([AX_OPENCL_PROGRAM],
							                                 [ax_cv_check_cl_libcl="$ax_try_lib $ax_check_cl_amd_flags"; break]
						                                                  )
						                                   ]
						                                   )])])
  done
  
  AS_IF([test "X$ax_cv_check_cl_libcl" = Xno],
        [LIBS='-Wl,-framework,OpenCL'
        AC_LINK_IFELSE([AX_OPENCL_PROGRAM],
                       [ax_cv_check_cl_libcl=$LIBS])])
  
  LIBS=$ax_save_LIBS
  CPPFLAGS=$ax_save_CPPFLAGS])
  
  AS_IF([test "X$ax_cv_check_cl_libcl" = Xno],
        [no_cl=yes; CL_CFLAGS=""; CL_LIBS=""],
        [CL_LIBS="$ax_cv_check_cl_libcl $CL_LIBS"; AC_DEFINE([_OPENCL], [1],
      [Define this for the OpenCL Accelerator])])
  AC_LANG_POP([$1])
fi
  
AC_SUBST([CL_CFLAGS])

#remove static link on Linux
CL_LIBS=`echo $CL_LIBS | $SED -e 's/-lOpenCL //'`
AC_SUBST([CL_LIBS])
])dnl
