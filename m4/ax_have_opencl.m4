# -*- mode: autoconf -*-
#
# AX_HAVE_OPENCL
#
# Check for an OpenCL implementation.  If CL is found, HAVE_OPENCL is defined
# and the required CPP and linker flags are included in the output variables
# "CL_CPPFLAGS" and "CL_LIBS", respectively.  If no usable CL implementation is
# found then "no_cl" is set to "yes" (otherwise it is set to "no").
#
# If the header <CL/cl.h> is found, "HAVE_CL_CL_H" is defined.  If the header 
# <OpenCL/cl.h> is found, HAVE_OPENCL_CL_H is defined.  These preprocessor
# definitions may not be mutually exclusive.
#
# This macro first checks /usr/include and /usr/lib for OpenCL support; if it is not
# found in those locations, then it checks in the standard installation locations
# for AMD's (/opt/AMDAPP) and NVIDIA's (/usr/local/cuda) SDKs.
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
AC_DEFUN([AX_HAVE_OPENCL],
[dnl
  AC_REQUIRE([AC_CANONICAL_HOST])dnl
  AC_ARG_ENABLE([opencl],
  [AS_HELP_STRING([--enable-opencl],
                  [use OpenCL])],
  [enable_opencl=$enableval],
  [enable_opencl='no'])
  if test x"$enable_opencl" != xno ; then
    CL_CPPFLAGS=""
    AC_CHECK_HEADERS([CL/cl.h OpenCL/cl.h], [HAVE_CL_H="yes"; break], [HAVE_CL_H="no"])
    if test x"$HAVE_CL_H" = xno ; then
      # check for AMD's SDK
      AC_MSG_CHECKING([for AMD's SDK cl.h])
      if test -d /opt/AMDAPP/include/CL ; then
        HAVE_CL_H="yes"
        AC_DEFINE([HAVE_CL_CL_H])
        CL_CPPFLAGS="-I/opt/AMDAPP/include"
      fi
      AC_MSG_RESULT([$HAVE_CL_H])
    fi
    if test x"$HAVE_CL_H" = xno ; then
      # check for NVIDIA's SDK
      AC_MSG_CHECKING([for NVIDIA's SDK cl.h])
      if test -d /usr/local/cuda/include/CL ; then
        HAVE_CL_H="yes"
        AC_DEFINE([HAVE_CL_CL_H])
        CL_CPPFLAGS="-I/usr/local/cuda/include"
      elif test -d /usr/local/cuda-6.5/include/CL ; then
        HAVE_CL_H="yes"
        AC_DEFINE([HAVE_CL_CL_H])
        CL_CPPFLAGS="-I/usr/local/cuda-6.5/include"
      elif test -d /usr/local/cuda-6.0/include/CL ; then
        HAVE_CL_H="yes"
        AC_DEFINE([HAVE_CL_CL_H])
        CL_CPPFLAGS="-I/usr/local/cuda-6.0/include"
      elif test -d /usr/local/cuda-5.5/include/CL ; then
        HAVE_CL_H="yes"
        AC_DEFINE([HAVE_CL_CL_H])
        CL_CPPFLAGS="-I/usr/local/cuda-5.5/include"
      fi
      AC_MSG_RESULT([$HAVE_CL_H])
    fi
    if test x"$HAVE_CL_H" = xno ; then
      no_cl=yes
      AC_MSG_WARN([no OpenCL headers found])
      CL_ENABLED=false
      CL_VERSION=0
    else
      #
      # First we check for Mac OS X, since OpenCL is standard there
      #
      CL_LIBS=""
      AC_MSG_CHECKING([for OpenCL library])
      case "$host_os" in
        darwin*) # On Mac OS X we check for installed frameworks
        AX_CHECK_FRAMEWORK([OpenCL], [
          CL_LIBS="-framework OpenCL"
          no_cl=no
          AC_MSG_RESULT(yes)
          CL_ENABLED=true
          CL_VERSION=1
        ]
        ,
        [
          no_cl=yes
          AC_MSG_RESULT(no)
          CL_ENABLED=false
          CL_VERSION=0
        ])
      ;;
      *)
        save_LIBS=$LIBS
        save_CFLAGS=$CFLAGS
        CFLAGS=$CL_CPPFLAGS
        LIBS="$save_LIBS -L/usr/lib64/nvidia -L/usr/lib/nvidia -lOpenCL"
        AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([
          #ifdef HAVE_OPENCL_CL_H
            #include <OpenCL/cl.h>
          #elif defined(HAVE_CL_CL_H)
            #include <CL/cl.h>
          #endif
          #include <stddef.h>
        ],[
          clGetPlatformIDs(0, NULL, NULL);
        ])],[
          no_cl=no
          AC_MSG_RESULT(yes)
          CL_ENABLED=true
          CL_VERSION=1
          CL_LIBS="-L/usr/lib64/nvidia -L/usr/lib/nvidia -lOpenCL"
        ],[
          no_cl=yes
          AC_MSG_RESULT(no)
          CL_ENABLED=false
          CL_VERSION=0
        ])
    
        LIBS=$save_LIBS
        CFLAGS=$save_CFLAGS
      ;;
      esac
    fi
  else
    no_cl=yes
    AC_MSG_CHECKING([for OpenCL library])
    AC_MSG_RESULT(disabled)
    CL_ENABLED=false
    CL_VERSION=0
  fi
])
