#  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Copyright (C) 2003 - 2008 GraphicsMagick Group
#
#  Copying and distribution of this file, with or without modification,
#  are permitted in any medium without royalty provided the copyright
#  notice and this notice are preserved.

# Process this file with autoconf to create configure script.
AC_PREREQ(2.69)

# ==============================================================================
# ImageMagick release.
# ==============================================================================
m4_define([magick_major_version], [7])
m4_define([magick_minor_version], [0])
m4_define([magick_micro_version], [8])
m4_define([magick_patchlevel_version], [55])
m4_define([magick_version],
          [magick_major_version.magick_minor_version.magick_micro_version-magick_patchlevel_version])
m4_define([magick_git_revision], esyscmd([sh -c "(gitversion.sh .) | awk '{ print \$1 }' | tr -d '\n'"]))
m4_define([magick_tar_name],[ImageMagick])

# ==============================================================================
# Initalize Automake
# ==============================================================================
AC_INIT([ImageMagick],
        [magick_version],
        [https://github.com/ImageMagick/ImageMagick/issues],
        [magick_tar_name],
        [https://imagemagick.org])
AC_CONFIG_SRCDIR([MagickCore/MagickCore.h])
AC_CONFIG_AUX_DIR([config])
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config/config.h])
AX_PREFIX_CONFIG_H([MagickCore/magick-baseconfig.h],[MagickCore])
AC_CANONICAL_SYSTEM
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign color-tests -Wall -Wno-portability subdir-objects dist-bzip2 dist-lzip dist-xz dist-zip tar-ustar])
AM_SILENT_RULES([yes])

#
# Save initial user-tunable values
#
USER_LIBS=$LIBS
for var in CC CFLAGS CPPFLAGS CXX CXXCPP LDFLAGS LIBS ; do
  eval isset=\${$var+set}
    if test "$isset" = 'set'; then
        eval val=$`echo $var`
        DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS}'${var}=${val}' "
    fi
done
AC_SUBST(DISTCHECK_CONFIG_FLAGS)

CONFIGURE_ARGS="$0 ${ac_configure_args}"
AC_SUBST(CONFIGURE_ARGS)

AC_ARG_ENABLE([reproducible-build],
    [AC_HELP_STRING([--enable-reproducible-build],
                    [enable reproducible build])],
    [enable_reproducible_build=$enableval],
    [enable_reproducible_build='no'])

# Source file containing package/library versioning information.
. ${srcdir}/version.sh

echo "configuring ${PACKAGE_NAME} ${PACKAGE_VERSION}${PACKAGE_VERSION_ADDENDUM}"

MAGICK_TARGET_CPU=$host_cpu
AC_SUBST(MAGICK_TARGET_CPU)
AC_DEFINE_UNQUOTED(MAGICK_TARGET_CPU,$MAGICK_TARGET_CPU,[Target Host CPU])

MAGICK_TARGET_VENDOR=$host_vendor
AC_SUBST(MAGICK_TARGET_VENDOR)
AC_DEFINE_UNQUOTED(MAGICK_TARGET_VENDOR,$MAGICK_TARGET_VENDOR,[Target Host Vendor])

MAGICK_TARGET_OS=$host_os
AC_SUBST(MAGICK_TARGET_OS)
AC_DEFINE_UNQUOTED(MAGICK_TARGET_OS,$MAGICK_TARGET_OS,[Target Host OS])


# Substitute versioning
AC_SUBST([MAGICK_MAJOR_VERSION],[magick_major_version])
AC_SUBST([MAGICK_MINOR_VERSION],[magick_minor_version])
AC_SUBST([MAGICK_MICRO_VERSION],[magick_micro_version])
AC_SUBST([MAGICK_PATCHLEVEL_VERSION],[magick_patchlevel_version])
AC_SUBST([MAGICK_VERSION],[magick_version])
AC_SUBST([MAGICK_GIT_REVISION],[magick_git_revision])

# Substitute library versioning
AC_SUBST(MAGICK_LIBRARY_CURRENT)dnl
AC_SUBST(MAGICK_LIBRARY_REVISION)dnl
AC_SUBST(MAGICK_LIBRARY_AGE)dnl
AC_SUBST([MAGICK_LIBRARY_CURRENT_MIN],
         [`expr $MAGICK_LIBRARY_CURRENT - $MAGICK_LIBRARY_AGE`])
AC_SUBST([MAGICK_LIBRARY_VERSION_INFO],
         [$MAGICK_LIBRARY_CURRENT:$MAGICK_LIBRARY_REVISION:$MAGICK_LIBRARY_AGE])

AC_SUBST(MAGICKPP_LIBRARY_CURRENT)dnl
AC_SUBST(MAGICKPP_LIBRARY_REVISION)dnl
AC_SUBST(MAGICKPP_LIBRARY_AGE)dnl
AC_SUBST([MAGICKPP_LIBRARY_CURRENT_MIN],
         [`expr $MAGICKPP_LIBRARY_CURRENT - $MAGICKPP_LIBRARY_AGE`])
AC_SUBST([MAGICKPP_LIBRARY_VERSION_INFO],
         [$MAGICKPP_LIBRARY_CURRENT:$MAGICKPP_LIBRARY_REVISION:$MAGICKPP_LIBRARY_AGE])

AC_SUBST(PACKAGE_NAME)dnl
AC_SUBST(PACKAGE_VERSION)dnl
AC_SUBST(PACKAGE_PERL_VERSION)dnl
AC_SUBST(PACKAGE_RELEASE)dnl
AC_SUBST(PACKAGE_CHANGE_DATE)dnl
AC_SUBST(PACKAGE_LIB_VERSION)dnl
AC_SUBST(PACKAGE_LIB_VERSION_NUMBER)dnl
AS_IF([test X$enable_reproducible_build = Xyes],
 [PACKAGE_RELEASE_DATE=$PACKAGE_RELEASE_DATE_REPRODUCIBLE],
 [PACKAGE_RELEASE_DATE=$PACKAGE_RELEASE_DATE_RAW],
)
AC_SUBST(PACKAGE_RELEASE_DATE)dnl

AC_SUBST(PACKAGE_VERSION_ADDENDUM)dnl

# Ensure that make can run correctly
AM_SANITY_CHECK

# versioning of library
MAGICK_LIB_VERSION="0x"
if test ${MAGICK_LIBRARY_CURRENT} -lt 10 ; then
  MAGICK_LIB_VERSION=${MAGICK_LIB_VERSION}0
fi
MAGICK_LIB_VERSION=${MAGICK_LIB_VERSION}${MAGICK_LIBRARY_CURRENT}
if test ${MAGICK_LIBRARY_AGE} -lt 10 ; then
  MAGICK_LIB_VERSION=${MAGICK_LIB_VERSION}0
fi
MAGICK_LIB_VERSION=${MAGICK_LIB_VERSION}${MAGICK_LIBRARY_AGE}
if test ${MAGICK_LIBRARY_REVISION} -lt 10 ; then
  MAGICK_LIB_VERSION=${MAGICK_LIB_VERSION}0
fi
MAGICK_LIB_VERSION=${MAGICK_LIB_VERSION}${MAGICK_LIBRARY_REVISION}
AC_SUBST(MAGICK_LIB_VERSION)

# Definition used to define MagickLibVersionText in version.h
MAGICK_LIB_VERSION_TEXT="${PACKAGE_VERSION}"
AC_SUBST(MAGICK_LIB_VERSION_TEXT)

# Definition used to define MagickLibVersionNumber in version.h
MAGICK_LIB_VERSION_NUMBER="${MAGICK_LIBRARY_CURRENT},${MAGICK_LIBRARY_AGE},${MAGICK_LIBRARY_REVISION}"
AC_SUBST(MAGICK_LIB_VERSION_NUMBER)

MAGICKPP_LIB_VERSION="0x"
if test ${MAGICKPP_LIBRARY_CURRENT} -lt 10 ; then
  MAGICKPP_LIB_VERSION=${MAGICKPP_LIB_VERSION}0
fi
MAGICKPP_LIB_VERSION=${MAGICKPP_LIB_VERSION}${MAGICKPP_LIBRARY_CURRENT}
if test ${MAGICKPP_LIBRARY_AGE} -lt 10 ; then
  MAGICKPP_LIB_VERSION=${MAGICKPP_LIB_VERSION}0
fi
MAGICKPP_LIB_VERSION=${MAGICKPP_LIB_VERSION}${MAGICKPP_LIBRARY_AGE}
if test ${MAGICKPP_LIBRARY_REVISION} -lt 10 ; then
  MAGICKPP_LIB_VERSION=${MAGICKPP_LIB_VERSION}0
fi
MAGICKPP_LIB_VERSION=${MAGICKPP_LIB_VERSION}${MAGICKPP_LIBRARY_REVISION}
AC_SUBST(MAGICKPP_LIB_VERSION)

# Definition used to define MagickLibVersionText in version.h
MAGICKPP_LIB_VERSION_TEXT="${PACKAGE_VERSION}"
AC_SUBST(MAGICKPP_LIB_VERSION_TEXT)

# Definition used to define MagickLibVersionNumber in version.h
MAGICK_LIB_VERSION_NUMBER="${MAGICK_LIBRARY_CURRENT},${MAGICK_LIBRARY_AGE},${MAGICK_LIBRARY_REVISION}"
AC_SUBST(MAGICK_LIB_VERSION_NUMBER)



# Regenerate config.status if ChangeLog or version.sh is updated.
AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/version.sh'])

PERLMAINCC=$CC

MAGICK_CFLAGS=''
MAGICK_CPPFLAGS=$CPPFLAGS_USER
MAGICK_PCFLAGS=$CPPFLAGS_USER
MAGICK_LDFLAGS=''
MAGICK_LIBS=''
MAGICK_FEATURES=''

#
# Evaluate shell variable equivalents to Makefile directory variables
#
if test "x$prefix" = xNONE; then
    prefix=$ac_default_prefix
fi
# Let make expand exec_prefix.
if test "x$exec_prefix" = xNONE; then
    exec_prefix='${prefix}'
fi

#
eval "eval PREFIX_DIR=${prefix}"
AC_SUBST(PREFIX_DIR)
eval "eval EXEC_PREFIX_DIR=${exec_prefix}"
AC_SUBST(EXEC_PREFIX_DIR)
eval "eval BIN_DIR=$bindir"
AC_SUBST(BIN_DIR)
eval "eval SBIN_DIR=$sbindir"
AC_SUBST(SBIN_DIR)
eval "eval LIBEXEC_DIR=$libexecdir"
AC_SUBST(LIBEXEC_DIR)
eval "eval DATA_DIR=$datadir"
AC_SUBST(DATA_DIR)
eval "eval DOC_DIR=$datadir/doc"
AC_SUBST(DOC_DIR)
eval "eval SYSCONF_DIR=$sysconfdir"
AC_SUBST(SYSCONF_DIR)
eval "eval SHAREDSTATE_DIR=$sharedstatedir"
AC_SUBST(SHAREDSTATE_DIR)
eval "eval LOCALSTATE_DIR=$localstatedir"
AC_SUBST(LOCALSTATE_DIR)
eval "eval LIB_DIR=$libdir"
AC_SUBST(LIB_DIR)
eval "eval INCLUDE_DIR=$includedir"
AC_SUBST(INCLUDE_DIR)
eval "eval PERSISTINCLUDE_DIR=$oldincludedir"
AC_SUBST(PERSISTINCLUDE_DIR)
eval "eval INFO_DIR=$infodir"
AC_SUBST(INFO_DIR)
eval "eval MAN_DIR=$mandir"
AC_SUBST(MAN_DIR)

# Get full paths to source and build directories
srcdirfull="`cd $srcdir && pwd`"
builddir="`pwd`"

#
# Compute variables useful for running uninstalled software.
#
MAGICK_CODER_MODULE_PATH="${builddir}/coders"
MAGICK_CONFIGURE_SRC_PATH="${srcdirfull}/config"
MAGICK_CONFIGURE_BUILD_PATH="${builddir}/config"
MAGICK_FILTER_MODULE_PATH="${builddir}/filters"
DIRSEP=':'
case "${build_os}" in
    mingw* )
        MAGICK_CODER_MODULE_PATH=`$WinPathScript "${MAGICK_CODER_MODULE_PATH}" 0`
        MAGICK_CONFIGURE_SRC_PATH=`$WinPathScript "${MAGICK_CONFIGURE_SRC_PATH}" 0`
        MAGICK_CONFIGURE_BUILD_PATH=`$WinPathScript "${MAGICK_CONFIGURE_BUILD_PATH}" 0`
        MAGICK_FILTER_MODULE_PATH=`$WinPathScript "${MAGICK_FILTER_MODULE_PATH}" 0`
        DIRSEP=';'
    ;;
esac
case "${host_os}" in
  mingw* )
    DIRSEP=';'
    ;;
esac
AC_SUBST(MAGICK_CODER_MODULE_PATH)
AC_SUBST(MAGICK_CONFIGURE_SRC_PATH)
AC_SUBST(MAGICK_CONFIGURE_BUILD_PATH)
AC_SUBST(MAGICK_FILTER_MODULE_PATH)
AC_SUBST(DIRSEP)


#
# Enable OS features.
#
AC_USE_SYSTEM_EXTENSIONS

# ==============================================================================
# Find required base packages
# ==============================================================================
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_STDC
AC_PROG_CPP
AC_PROG_LD
AC_SUBST(LD)
AC_PROG_CC_C99
AM_PROG_CC_C_O
AX_CFLAGS_WARN_ALL
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_SED
AC_PROG_AWK
AC_PROG_MKDIR_P
AM_WITH_DMALLOC
AX_C___ATTRIBUTE__
AX_GCC_ARCHFLAG([yes])
PKG_PROG_PKG_CONFIG([0.20])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Test for 64-bit build.
AC_CHECK_SIZEOF([size_t])

AX_COMPILER_VENDOR

if test "x$GCC" = "xyes"; then
  CFLAGS="$CFLAGS -fexceptions"
  touch local.exp
else
  cat > local.exp <<EOF
set CC_FOR_TARGET "$CC"
EOF
fi

# Check for linker script support
gl_LD_VERSION_SCRIPT

#
# Enable run-time checking.
#
AC_ARG_ENABLE([bounds-checking],
    [AC_HELP_STRING([--enable-bounds-checking],
                    [enable run-time bounds-checking])],
    [enable_bounds_checking=$enableval],
    [enable_bounds_checking='no'])

if test "$enable_bounds_checking" = yes; then
  AC_DEFINE([_FORTIFY_SOURCE], [2],
    [enable run-time bounds-checking])
fi

#
# Tests for Windows
#
AC_EXEEXT
AC_OBJEXT

GDI32_LIBS=''
WS2_32_LIBS=''
native_win32_build='no'
cygwin_build='no'
case "${host_os}" in
    cygwin* )
        cygwin_build='yes'
        GDI32_LIBS='-lgdi32'
    ;;
    mingw* )
        native_win32_build='yes'
        GDI32_LIBS='-lgdi32'
        OLE_LIBS='-loleaut32'
        WS2_32_LIBS="-lws2_32"
    ;;
esac
if test "${GDI32_LIBS}x" != 'x'; then
    AC_DEFINE(WINGDI32_DELEGATE,1,Define to use the Windows GDI32 library)
fi
AC_SUBST(GDI32_LIBS)
AC_SUBST(OLE_LIBS)
AC_SUBST(WS2_32_LIBS)
AM_CONDITIONAL(WINGDI32_DELEGATE, test "${GDI32_LIBS}x" != 'x' )
AM_CONDITIONAL(WIN32_NATIVE_BUILD, test "${native_win32_build}" = 'yes' )
AM_CONDITIONAL(CYGWIN_BUILD, test "${cygwin_build}" = 'yes' )
AM_CONDITIONAL(USING_CL, test "x${CC}" = 'xcl.exe' )

WinPathScript="${srcdirfull}/winpath.sh"
AC_SUBST(WinPathScript)

#
# Compiler flags tweaks
#
if test "${GCC}" != "yes"; then
    case "${host}" in
        *-*-hpux* )
            # aCC: HP ANSI C++ B3910B A.03.34
            CFLAGS="${CFLAGS} -Wp,-H30000"
            if test -n "${CXXFLAGS}"; then
                CXXFLAGS='-AA'
            else
                CXXFLAGS="${CXXFLAGS} -AA"
            fi
        ;;
        *-dec-osf5.* )
            # Compaq alphaev68-dec-osf5.1 compiler
            if test -n "${CXXFLAGS}"; then
                CXXFLAGS='-std strict_ansi -noimplicit_include'
            else
                CXXFLAGS="${CXXFLAGS} -std strict_ansi -noimplicit_include"
            fi
  esac
fi

# Check for lazy-loading.
AC_CACHE_CHECK([for linker lazyload option],[im_cv_ld_lazyload],
[
im_cv_ld_lazyload='none'
case "${host}" in
  *-*-solaris2.8 | *-*-solaris2.9 | *-*-solaris2.1? )
    if test "$lt_cv_prog_gnu_ld" != 'yes' ; then
       im_cv_ld_lazyload='-Wl,-zlazyload'
    fi
    ;;
esac
])
if test "${im_cv_ld_lazyload}" != 'none' ; then
  if test -z "${LDFLAGS}" ; then
    LDFLAGS="${im_cv_ld_lazyload}"
  else
    LDFLAGS="${im_cv_ld_lazyload} ${LDFLAGS}"
  fi
fi

dnl Platform-specific stuff
case "$host" in
  *darwin* | *-macos10*)
    dnl Disable FORTIFY_SOURCE to fix a bug in LVVM / OpenMP support
    CFLAGS="${CFLAGS} -D_FORTIFY_SOURCE=0"
    dnl OS X universal binary support, requires --disable-dependency-tracking
    AC_ARG_ENABLE([osx-universal-binary],
        AC_HELP_STRING([--enable-osx-universal-binary],
           [build universal binary on OS X [[default=no]]]),
        [build_osxuniversal="${enableval}"], [build_osxuniversal=no])

    if test "${build_osxuniversal}" != no ; then
      if test "$enable_dependency_tracking" != no ; then
        AC_MSG_ERROR([--enable-osx-universal-binary requires --disable-dependency-tracking.
Please re-run configure with these options:
  --disable-dependency-tracking --enable-osx-universal-binary
        ])
      fi
      CFLAGS="$CFLAGS -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc -arch i386"
      CXXFLAGS="$CXXFLAGS -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc -arch i386"
      LDFLAGS="$LDFLAGS -Wl,-syslibroot,/Developer/SDKs/MacOSX10.5.sdk -arch ppc -arch i386"
  fi
  ;;
esac

#
# ARCH specific include directory
#
AC_ARG_WITH([includearch-dir],
    [AC_HELP_STRING([--includearch-dir=DIR],
                    [ARCH specific include directory])],
                    [includearch_dir=$withval],
                    [includearch_dir=$INCLUDE_DIR])

eval "eval INCLUDEARCH_DIR=$includearch_dir"
AC_SUBST(INCLUDEARCH_DIR)

#
# ARCH specific configuration directory
#
AC_ARG_WITH([sharearch-dir],
    [AC_HELP_STRING([--sharearch-dir=DIR],
                    [ARCH specific config directory])],
                    [sharearch_dir=$withval],
                    [sharearch_dir="${LIB_DIR}"])

eval "eval SHAREARCH_DIR=$sharearch_dir"
AC_SUBST(SHAREARCH_DIR)

# Path to the pkgconfig folder
AC_ARG_WITH([pkgconfigdir], AS_HELP_STRING([--with-pkgconfigdir=DIR],
            [Path to the pkgconfig directory @<:@LIBDIR/pkgconfig@:>@]),
            [pkgconfigdir="$withval"], [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST([pkgconfigdir])

#
# Enable support for POSIX thread APIs
#
AC_ARG_WITH([threads],
    [AC_HELP_STRING([--without-threads], [disable POSIX threads API support])],
    [with_threads=$withval],
    [with_threads='yes'])

THREAD_LIBS=''
have_threads=no
if test "$with_threads" != 'no'; then
    AX_PTHREAD()
    if test "$ax_pthread_ok" = yes; then
        have_threads=yes
        DEF_THREAD="$PTHREAD_CFLAGS"
        CFLAGS="$CFLAGS $DEF_THREAD"
        CXXFLAGS="$CXXFLAGS $DEF_THREAD"
        THREAD_LIBS="$PTHREAD_LIBS"
        if test "$CC" != "$PTHREAD_CC"; then
            AC_MSG_WARN([Replacing compiler $CC with compiler $PTHREAD_CC to support pthreads.])
            CC="$PTHREAD_CC"
        fi
        AC_DEFINE(THREAD_SUPPORT,1,[Define if you have POSIX threads libraries and header files.])
    fi
fi
LIBS="$LIBS $THREAD_LIBS"
AC_SUBST(THREAD_LIBS)

# Enable support for OpenMP
if test "$have_threads" != 'yes'; then
  ac_cv_prog_c_openmp=unsupported
fi
AC_OPENMP([C])
CFLAGS="$OPENMP_CFLAGS $CFLAGS"
MAGICK_PCFLAGS="$MAGICK_PCFLAGS $OPENMP_CFLAGS"
AC_SUBST(OPENMP_CFLAGS)
if test "$enable_openmp" != no; then
  if test "$ac_cv_prog_c_openmp" != 'unsupported'; then
    MAGICK_FEATURES="OpenMP $MAGICK_FEATURES"
  fi
fi

# Enable support for OpenCL
AX_OPENCL([C])
CFLAGS="$CL_CFLAGS $CFLAGS"
CPPFLAGS="$CL_CFLAGS $CPPFLAGS"
LIBS="$CL_LIBS $LIBS"
AC_SUBST(CL_CFLAGS)
if test "$enable_opencl" != no; then
  if test "X$ax_cv_check_cl_libcl" != Xno; then :
    MAGICK_FEATURES="OpenCL $MAGICK_FEATURES"
  fi
fi

########
#
# Check for large file support
#
########
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
LFS_CPPFLAGS=''
if test "$enable_largefile" != no; then
    case $ac_cv_sys_file_offset_bits in
    no)
        # nothing to do here as the host supports LFS fine
        ;;
    unknown)
        AC_MSG_CHECKING([for native large file support])
        AC_RUN_IFELSE([AC_LANG_PROGRAM([#include <unistd.h>
          main () {
          exit(!(sizeof(off_t) == 8));
        }])],
        [ac_cv_sys_file_offset_bits=64; AC_DEFINE(_FILE_OFFSET_BITS,64)
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])
        ;;
    *)
        LFS_CPPFLAGS="$LFS_CPPFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
        ;;
    esac
    if test "$ac_cv_sys_large_files" != 'no'; then
        LFS_CPPFLAGS="$LFS_CPPFLAGS -D_LARGE_FILES=1"
    fi
    if test "$ac_cv_sys_largefile_source" != 'no'; then
        LFS_CPPFLAGS="$LFS_CPPFLAGS -D_LARGEFILE_SOURCE=1"
    fi
fi
AC_SUBST(LFS_CPPFLAGS)

# Configure libtool
AC_LIBTOOL_DLOPEN
LT_INIT([win32-dll])
LT_LANG([C++])
AC_SUBST(LIBTOOL_DEPS)

# Check to see if building shared libraries
libtool_build_shared_libs='no'
if test "$enable_shared" = 'yes'; then
    libtool_build_shared_libs='yes'
fi

# Check to see if building static libraries
libtool_build_static_libs='no'
if test "$enable_static" = 'yes'; then
    libtool_build_static_libs='yes'
fi

AM_CONDITIONAL(WITH_SHARED_LIBS, test "${libtool_build_shared_libs}" = 'yes')
#
# Enable support for building loadable modules
#
build_modules='no'
AC_ARG_WITH([modules],
    [AC_HELP_STRING([--with-modules],
                    [enable building dynamically loadable modules])],
    [with_modules=$withval],
    [with_modules='no'])

# Only allow building loadable modules if we are building shared libraries
if test "$with_modules" != 'no' ; then
    if test "$libtool_build_shared_libs" = 'no'; then
        AC_MSG_WARN([Modules may only be built if building shared libraries is enabled.])
    build_modules='no'
  else
    build_modules='yes'
  fi
fi
if test "$build_modules" != 'no' ; then
    AC_DEFINE(BUILD_MODULES,1,Define if coders and filters are to be built as modules.)
    MAGICK_FEATURES="$MAGICK_FEATURES Modules"
fi
AM_CONDITIONAL(WITH_MODULES, test "$build_modules" != 'no')

# Enable build using delegate libraries built in subdirectories rather than installed
# delegate libraries (bzlib fftw fpx gslib jp2 jbig jpeg lcms lzma png tiff ttf wmf xml zlib)
AC_ARG_ENABLE([delegate-build],
    [AC_HELP_STRING([--enable-delegate-build],
                    [look for delegate libraries in build directory])],
    [enable_delegate_build=$enableval],
    [enable_delegate_build='no'])

AC_ARG_ENABLE([deprecated],
    [AC_HELP_STRING([--disable-deprecated],
                    [exclude deprecated methods in MagickCore and MagickWand APIs])],
    [enable_deprecated=$enableval],
    [enable_deprecated='no'])

if test "$enable_deprecated" = 'yes'; then
  AC_DEFINE(EXCLUDE_DEPRECATED,1,[exclude deprecated methods in MagickCore API])
else
  DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --disable-deprecated "
fi

# Build a version of ImageMagick which operates uninstalled.
# Used to build distributions located via MAGICK_HOME / executable path
AC_ARG_ENABLE([installed],
    [AC_HELP_STRING([--disable-installed],
                    [Formally install ImageMagick under PREFIX])],
    [enable_installed=$enableval],
    [enable_installed='yes'])

if test "$enable_installed" = 'yes'; then
    AC_DEFINE(INSTALLED_SUPPORT,1,[ImageMagick is formally installed under prefix])
else
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --disable-installed "
fi

# Permit enciphering and deciphering image pixels.
AC_ARG_ENABLE([cipher],
    [AC_HELP_STRING([--disable-cipher],
                    [disable enciphering and deciphering image pixels])],
    [enable_cipher=$enableval],
    [enable_cipher='yes'])

if test "$enable_cipher" = 'yes'; then
    AC_DEFINE(CIPHER_SUPPORT,1,[permit enciphering and deciphering image pixels])
    MAGICK_FEATURES="Cipher $MAGICK_FEATURES"
fi

# Build a zero-configuration version of ImageMagick.
AC_ARG_ENABLE([zero-configuration],
    [AC_HELP_STRING([--enable-zero-configuration],
                    [enable self-contained, embeddable, zero-configuration ImageMagick])],
    [enable_zero_configuration=$enableval],
    [enable_zero_configuration='no'])

if test "$enable_zero_configuration" = 'yes'; then
    AC_DEFINE(ZERO_CONFIGURATION_SUPPORT,1,[Build self-contained, embeddable, zero-configuration ImageMagick])
    MAGICK_FEATURES="Zero-configuration $MAGICK_FEATURES"
fi

# Build a high dynamic range version of ImageMagick.
AC_ARG_ENABLE([hdri],
    [AC_HELP_STRING([--enable-hdri],
                    [accurately represent the wide range of intensity levels found in real scenes])],
    [enable_hdri=$enableval],
    [enable_hdri='yes'])

MAGICK_HDRI=""
if test "$enable_hdri" = 'yes'; then
    MAGICK_HDRI="HDRI"
    magick_hdri_enable='1';
    MAGICK_FEATURES="HDRI $MAGICK_FEATURES"
else
    magick_hdri_enable='0'
fi
AC_DEFINE_UNQUOTED(HDRI_ENABLE_OBSOLETE_IN_H,$magick_hdri_enable,[Whether hdri is enabled or not])
AC_SUBST(MAGICK_HDRI)dnl
MAGICK_PCFLAGS="$MAGICK_PCFLAGS -DMAGICKCORE_HDRI_ENABLE=$magick_hdri_enable"
CFLAGS="$CFLAGS -DMAGICKCORE_HDRI_ENABLE=$magick_hdri_enable"
CPPFLAGS="$CPPFLAGS -DMAGICKCORE_HDRI_ENABLE=$magick_hdri_enable"

# Enable pipes (|) in filenames.
AC_ARG_ENABLE([pipes],
    [AC_HELP_STRING([--enable-pipes],
                    [enable pipes (|) in filenames])],
    [enable_pipes=$enableval],
    [enable_pipes='no'])

if test "$enable_pipes" = 'yes'; then
    AC_DEFINE(PIPES_SUPPORT,1,[enable pipes (|) in filenames])
    MAGICK_FEATURES="Pipes $MAGICK_FEATURES"
fi

# Build a version of ImageMagick with assert statements.
AC_ARG_ENABLE([assert],
    [AC_HELP_STRING([--disable-assert],
                    [disable assert() statements in build])],
    [enable_assert=$enableval],
    [enable_assert='yes'])

if test "$enable_assert" = 'no'; then
    AC_DEFINE(NDEBUG,1,[Turn off assert statements])
fi

# Don't emit "rebuild rules" for configure, Makefile.ins, etc.
AM_MAINTAINER_MODE

# Enable hugepages support
AC_ARG_ENABLE([hugepages],
    [AC_HELP_STRING([--enable-hugepages],
                    [enable 'huge pages' support])],
    [enable_hugepages=$enableval],
    [enable_hugepages='no'])

# Enable ccmalloc memory debugging support
AC_ARG_ENABLE([ccmalloc],
    [AC_HELP_STRING([--enable-ccmalloc],
                    [enable 'ccmalloc' memory debug support])],
    [enable_ccmalloc=$enableval],
    [enable_ccmalloc='no'])

# Enable Electric Fence memory debugging support
AC_ARG_ENABLE([efence],
    [AC_HELP_STRING([--enable-efence],
                    [enable 'efence' memory debug support])],
    [enable_efence=$enableval],
    [enable_efence='no'])

# Enable prof-based profiling support
AC_ARG_ENABLE([prof],
    [AC_HELP_STRING([--enable-prof],
                    [enable 'prof' profiling support])],
    [enable_prof=$enableval],
    [enable_prof='no'])

# Enable gprof-based profiling support
AC_ARG_ENABLE([gprof],
    [AC_HELP_STRING([--enable-gprof],
                    [enable 'gprof' profiling support])],
    [enable_gprof=$enableval],
    [enable_gprof='no'])

# Enable gcov-based profiling support
AC_ARG_ENABLE([gcov],
    [AC_HELP_STRING([--enable-gcov],
                    [enable 'gcov' profiling support])],
    [enable_gcov=$enableval],
    [enable_gcov='no'])

enable_profiling='no'
if test "$enable_prof" = 'yes' || test "$enable_gprof" = 'yes' || test "$enable_gcov" = 'yes'; then
    enable_profiling='yes'
    if test "$libtool_build_shared_libs" = 'yes'; then
        echo "Warning: Can not profile code using shared libraries"
    fi
fi

# Magick API method prefix
AC_ARG_WITH([method-prefix],
    [AC_HELP_STRING([--with-method-prefix=PREFIX],
                    [prefix MagickCore API methods])],
    [with_method_prefix=$withval],
    [with_method_prefix='no'])

if test "$with_method_prefix" != 'no'; then
    AC_DEFINE_UNQUOTED(NAMESPACE_PREFIX,$with_method_prefix,[Magick API method prefix])
    AC_DEFINE_UNQUOTED(NAMESPACE_PREFIX_TAG,["$with_method_prefix"],[Magick API method prefix tag])
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-method-prefix "
fi

# Enable legacy support (default no)
AC_ARG_ENABLE(legacy-support,
              [  --enable-legacy-support install legacy command-line utilities (default disabled)],
              [with_legacy_support=$enableval],
              [with_legacy_support='no'])
AM_CONDITIONAL(LEGACY_SUPPORT, test "$with_legacy_support" != 'no')

# Enable building command line utilities (default yes)
AC_ARG_WITH(utilities,
            [  --with-utilities enable building command-line utilities (default yes)],
            [with_utilities=$withval], [with_utilities='yes'])
AM_CONDITIONAL(WITH_UTILITIES, test "$with_utilities" = 'yes')

# Number of bits in a Quantum
AC_ARG_WITH([quantum-depth],
    [AC_HELP_STRING([--with-quantum-depth=DEPTH],
                    [number of bits in a pixel quantum (default 16)])],
    [with_quantum_depth=$withval],
    [with_quantum_depth=16])

if test "$with_quantum_depth" != '8'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-quantum-depth=$with_quantum_depth "
fi

case "${with_quantum_depth}" in
    8 ) ;;
    16 ) ;;
    32 ) ;;
    64 ) ;;
    * ) AC_MSG_ERROR("Pixel quantum depth must have value of 8, 16, 32, or 64") ;;
esac
QUANTUM_DEPTH="$with_quantum_depth"
AC_DEFINE_UNQUOTED(QUANTUM_DEPTH_OBSOLETE_IN_H,$QUANTUM_DEPTH,[Number of bits in a pixel Quantum (8/16/32/64)])
AC_SUBST(QUANTUM_DEPTH)dnl
MAGICK_PCFLAGS="$MAGICK_PCFLAGS -DMAGICKCORE_QUANTUM_DEPTH=$QUANTUM_DEPTH"
CFLAGS="$CFLAGS -DMAGICKCORE_QUANTUM_DEPTH=$QUANTUM_DEPTH"
CPPFLAGS="$CPPFLAGS -DMAGICKCORE_QUANTUM_DEPTH=$QUANTUM_DEPTH"

# define a lib suffix for abi purpose
MAGICK_ABI_SUFFIX="Q${QUANTUM_DEPTH}"
if test "$enable_hdri" = 'yes'; then
  MAGICK_ABI_SUFFIX="Q${QUANTUM_DEPTH}${MAGICK_HDRI}"
fi
AC_SUBST(MAGICK_ABI_SUFFIX)

# Set pixel cache threshold
AC_ARG_WITH([cache],
    [AC_HELP_STRING([--with-cache=THRESHOLD],
                    [set pixel cache threshhold in MB (default available memory)])],
    [with_cache=$withval],
    [with_cache=''])

if test "$with_cache" != ''; then
    AC_DEFINE_UNQUOTED(PixelCacheThreshold,$with_cache,[Pixel cache threshold in MB (defaults to available memory)])
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-cache=$with_cache "
fi

# Disable/Enable support for full delegate paths
AC_ARG_WITH([frozenpaths],
    [AC_HELP_STRING([--with-frozenpaths],
                    [freeze delegate paths])],
    [with_frozenpaths=$withval],
    [with_frozenpaths='no'])

# Enable build/install of Magick++
AC_ARG_WITH([magick-plus-plus],
    [AC_HELP_STRING([--without-magick-plus-plus],
                    [disable build/install of Magick++])],
    [with_magick_plus_plus=$withval],
    [with_magick_plus_plus='yes'])

# Encode the this name into the shared library.
AC_ARG_WITH([package-release-name],
    [AC_HELP_STRING([--with-package-release-name=NAME],
                    [encode this name into the shared library])],
[MAGICK_LT_RELEASE_OPTS="-release $withval"])
AC_SUBST(MAGICK_LT_RELEASE_OPTS)

# Disable build/install of PerlMagick.
AC_ARG_WITH([perl],
    [AC_HELP_STRING([--with-perl],
                    [enable build/install of PerlMagick])],
    [with_perl=$withval],
    [with_perl='no'])

# Options to pass when configuring PerlMagick
AC_ARG_WITH([perl-options],
    [AC_HELP_STRING([--with-perl-options=OPTIONS],
                    [options to pass on command-line when generating PerlMagick build file])],
PERL_MAKE_OPTIONS=$withval)
AC_SUBST(PERL_MAKE_OPTIONS)

# Enable jemalloc, object-caching memory allocation library.
AC_ARG_WITH(jemalloc,
        [  --with-jemalloc         enable jemalloc memory allocation library support],
        [with_jemalloc=$withval],
        [with_jemalloc='no'])
if test "$with_jemalloc" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-jemalloc=$with_jemalloc "
fi

# Enable umem, object-caching memory allocation library.
AC_ARG_WITH(umem,
        [  --with-umem             enable umem memory allocation library support],
        [with_umem=$withval],
        [with_umem='no'])
if test "$with_umem" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-umem=$with_umem "
fi

#
# Specify path to shared libstdc++ if not in normal location
#
AC_ARG_WITH([libstdc],
    [AC_HELP_STRING([--with-libstdc=DIR],
                    [ use libstdc++ in DIR (for GNU C++)])],
    [with_libstdc=$withval],
    [with_libstdc=''])

if test "$with_libstdc" != ''; then
    if test -d "$with_libstdc"; then
        LIBSTDCLDFLAGS="-L$with_libstdc"
    fi
fi
AC_SUBST(LIBSTDCLDFLAGS)

# Does gcc required -traditional?
AC_PROG_GCC_TRADITIONAL

########
#
# Set defines required to build DLLs and modules using MinGW
#
########
# These options are set for multi-thread DLL module build
#  libMagickCore:           _DLL _MAGICKMOD_ _MAGICKLIB_
#  module:              _DLL
#  executable/Magick++: _DLL _MAGICKMOD_
MODULE_EXTRA_CPPFLAGS=''
LIBRARY_EXTRA_CPPFLAGS=''
if test "${native_win32_build}" = 'yes'; then
    if test "${libtool_build_shared_libs}" = 'yes'; then
        CPPFLAGS="$CPPFLAGS -D_DLL"
        MAGICK_CPPFLAGS="$MAGICK_CPPFLAGS -D_DLL"
        MAGICK_PCFLAGS="$MAGICK_PCFLAGS -D_DLL"
        LIBRARY_EXTRA_CPPFLAGS="$LIBRARY_EXTRA_CPPFLAGS -D_MAGICKLIB_"
        if test "$build_modules" = 'yes'; then
            LIBRARY_EXTRA_CPPFLAGS="$LIBRARY_EXTRA_CPPFLAGS -D_MAGICKMOD_"
        else
            MODULE_EXTRA_CPPFLAGS="$MODULE_EXTRA_CPPFLAGS -D_MAGICKLIB_"
        fi
    else
        CPPFLAGS="$CPPFLAGS -D_LIB"
        MAGICK_CPPFLAGS="$MAGICK_CPPFLAGS -D_LIB"
        MAGICK_PCFLAGS="$MAGICK_PCFLAGS -D_LIB"
    fi
    if test "$with_threads" = 'yes'; then
         CPPFLAGS="$CPPFLAGS -D_MT"
         MAGICK_CPPFLAGS="$MAGICK_CPPFLAGS -D_MT"
         MAGICK_PCFLAGS="$MAGICK_PCFLAGS -D_MT"
    fi
fi
AC_SUBST(MODULE_EXTRA_CPPFLAGS)
AC_SUBST(LIBRARY_EXTRA_CPPFLAGS)

# Check standard headers
AC_HEADER_STDC
if ! test x"$ac_cv_header_stdc" = x"yes"; then
    AC_MSG_WARN([configure has detected that you do not have the ANSI standard C
    header files.  Compilation cannot proceed.  Please install the ANSI C
    headers and rerun this script.]);
fi
AC_HEADER_ASSERT
AC_HEADER_DIRENT

# Check additional headers
AC_CHECK_HEADERS(arm/limits.h arpa/inet.h complex.h errno.h fcntl.h limits.h linux/unistd.h locale.h machine/param.h mach-o/dyld.h netinet/in.h OS.h process.h sun_prefetch.h stdarg.h sys/ipc.h sys/mman.h sys/resource.h sys/sendfile.h sys/socket.h sys/syslimits.h sys/time.h sys/timeb.h sys/times.h sys/wait.h utime.h wchar.h xlocale.h)

########
#
# Checks for typedefs, structures, and compiler characteristics.
#
########

AC_HEADER_STDBOOL
AC_C_VOLATILE
AC_C_STRINGIZE
AC_HEADER_STAT
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE
AC_SYS_INTERPRETER

#
# Checks for language qualifiers and semantics.
#
AC_C_CHAR_UNSIGNED
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_C_VOLATILE

# If words are stored with the most significant byte first (like
# Motorola and SPARC CPUs), define `WORDS_BIGENDIAN'.
AC_C_BIGENDIAN

# Define to a suitable type, if standard headers do not define it.
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INTMAX_T
AC_TYPE_INTPTR_T
AC_TYPE_LONG_DOUBLE
AC_TYPE_LONG_DOUBLE_WIDER
AC_TYPE_LONG_LONG_INT
AC_TYPE_MBSTATE_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINTMAX_T
AC_TYPE_UINTPTR_T
AC_TYPE_UNSIGNED_LONG_LONG_INT

AC_CHECK_TYPES([locale_t], [], [], [[#include <xlocale.h>]])

# Float_t and double_t are intended to be the the most efficient type.
AC_CHECK_TYPES([float_t], [], [], [[#include <math.h>]])
AC_CHECK_TYPES([double_t], [], [], [[#include <math.h>]])
AC_CHECK_SIZEOF([float_t], [], [[#include <math.h>]])
AC_CHECK_SIZEOF([double_t], [], [[#include <math.h>]])

# Get size of float, double and long double for comparaison.
AC_CHECK_SIZEOF([float])
AC_CHECK_SIZEOF([double])
AC_CHECK_SIZEOF([long double])

# Obtain size of an 'signed short' and define as SIZEOF_SIGNED_SHORT
AC_CHECK_SIZEOF(signed short)

# Obtain size of an 'unsigned short' and define as SIZEOF_UNSIGNED_SHORT
AC_CHECK_SIZEOF(unsigned short)

# Obtain size of an 'signed int' and define as SIZEOF_SIGNED_INT
AC_CHECK_SIZEOF(signed int)

# Obtain size of an 'unsigned int' and define as SIZEOF_UNSIGNED_INT
AC_CHECK_SIZEOF(unsigned int)

# Obtain size of a 'signed long' and define as SIZEOF_SIGNED_LONG
AC_CHECK_SIZEOF(signed long)

# Obtain size of a 'unsigned long' and define as SIZEOF_UNSIGNED_LONG
AC_CHECK_SIZEOF(unsigned long)

# Obtain size of a 'long long' and define as SIZEOF_SIGNED_LONG_LONG.  If
# 'signed long long' is not supported then the value defined is zero.
AC_CHECK_SIZEOF(signed long long)

# Obtain size of a 'unsigned long long' and define as
# SIZEOF_UNSIGNED_LONG_LONG.  If 'unsigned long long' is not
# supported then the value defined is zero.
AC_CHECK_SIZEOF(unsigned long long)

# Obtain size of off_t and define as SIZEOF_OFF_T
AC_CHECK_SIZEOF(off_t)

# Obtain size of size_t and define as SIZEOF_SIZE_T
AC_CHECK_SIZEOF(size_t)

# Obtain size of ssize_t and define as SIZEOF_SSIZE_T
AC_CHECK_SIZEOF(ssize_t)

# Obtain size of an unsigned int pointer and define as SIZEOF_UNSIGNED_INTP
AC_CHECK_SIZEOF(unsigned int*)

#
# Compute sized types for current CPU and compiler options.
#

AC_MSG_CHECKING(for signed 8-bit type)
INT8_T='signed char'
AC_MSG_RESULT($INT8_T)
AC_SUBST(INT8_T)

AC_MSG_CHECKING(for unsigned 8-bit type)
UINT8_T='unsigned char'
AC_MSG_RESULT($UINT8_T)
AC_SUBST(UINT8_T)

AC_MSG_CHECKING(for signed 16-bit type)
INT16_T='signed short'
AC_MSG_RESULT($INT16_T)
AC_SUBST(INT16_T)

AC_MSG_CHECKING(for unsigned 16-bit type)
UINT16_T='unsigned short'
AC_MSG_RESULT($UINT16_T)
AC_SUBST(UINT16_T)

AC_MSG_CHECKING(for signed 32-bit type)
INT32_T='none'
INT32_F='none'
if test $ac_cv_sizeof_signed_int -eq 4; then
  INT32_T='signed int'
  INT32_F='""'
elif test $ac_cv_sizeof_signed_long -eq 4; then
  INT32_T='signed long'
  INT32_F='"l"'
fi
AC_MSG_RESULT($INT32_T)
AC_SUBST(INT32_T)
AC_SUBST(INT32_F)

AC_MSG_CHECKING(for unsigned 32-bit type)
UINT32_T='none'
UINT32_F='none'
if test $ac_cv_sizeof_unsigned_int -eq 4; then
  UINT32_T='unsigned int'
  UINT32_F='""'
elif test $ac_cv_sizeof_unsigned_long -eq 4; then
  UINT32_T='unsigned long'
  UINT32_F='"l"'
fi
AC_MSG_RESULT($UINT32_T)
AC_SUBST(UINT32_T)
AC_SUBST(UINT32_F)

AC_MSG_CHECKING(for signed 64-bit type)
INT64_T='none'
INT64_F='none'
if test $ac_cv_sizeof_signed_long -eq 8; then
  INT64_T='signed long'
  INT64_F='"l"'
elif test $ac_cv_sizeof_signed_long_long -eq 8; then
  INT64_T='signed long long'
  INT64_F='"ll"'
fi
case "${host_os}" in
  mingw* )
    INT64_F='"I64"'
    ;;
esac
AC_MSG_RESULT($INT64_T)
AC_SUBST(INT64_T)
AC_SUBST(INT64_F)

AC_MSG_CHECKING(for unsigned 64-bit type)
UINT64_T='none'
UINT64_F='none'
if test $ac_cv_sizeof_unsigned_long -eq 8; then
  UINT64_T='unsigned long'
  UINT64_F='"l"'
elif test $ac_cv_sizeof_unsigned_long_long -eq 8; then
  UINT64_T='unsigned long long'
  UINT64_F='"ll"'
fi
case "${host_os}" in
  mingw* )
    UINT64_F='"I64"'
    ;;
esac
AC_MSG_RESULT($UINT64_T)
AC_SUBST(UINT64_T)
AC_SUBST(UINT64_F)

AC_MSG_CHECKING(for unsigned maximum type)
UINTMAX_T='none'
UINTMAX_F='none'
if test "$UINT64_T" != 'none'; then
  UINTMAX_T=$UINT64_T
  UINTMAX_F=$UINT64_F
elif test "$UINT32_T" != 'none'; then
  UINTMAX_T=$UINT32_T
  UINTMAX_F=$UINT32_F
fi
AC_MSG_RESULT($UINTMAX_T)
AC_SUBST(UINTMAX_T)
AC_SUBST(UINTMAX_F)

AC_MSG_CHECKING(for pointer difference type)
UINTPTR_T='none'
UINTPTR_F='none'
if test $ac_cv_sizeof_unsigned_long -eq $ac_cv_sizeof_unsigned_intp; then
  UINTPTR_T='unsigned long'
  UINTPTR_F='"l"'
elif test $ac_cv_sizeof_unsigned_long_long -eq $ac_cv_sizeof_unsigned_intp; then
  UINTPTR_T='unsigned long long'
  UINTPTR_F='"ll"'
fi
AC_MSG_RESULT($UINTPTR_T)
AC_SUBST(UINTPTR_T)
AC_SUBST(UINTPTR_F)

AC_MSG_CHECKING([whether our compiler supports __func__])
AC_TRY_COMPILE([],
 [{ const char *func = __func__; return(func != 0 ? 0 : 1); }],
 AC_MSG_RESULT([yes]),
 AC_MSG_RESULT([no])
 AC_MSG_CHECKING([whether our compiler supports __FUNCTION__])
 AC_TRY_COMPILE([],
   [{ const char *func = __FUNCTION__; return(func != 0 ? 0 : 1); }],
   AC_MSG_RESULT([yes])
   AC_DEFINE(__func__, __FUNCTION__,
     [Define to appropriate substitute if compiler does not have __func__]),
   AC_MSG_RESULT([no])
   AC_DEFINE(__func__, __FILE__,
     [Define to appropriate substitute if compiler does not have __func__])))

########
#
# Check for functions
#
########
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MMAP
AC_FUNC_FORK
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STRTOD
AC_FUNC_STRERROR_R
AC_FUNC_VPRINTF

#
# Find math library
#
MATH_LIBS=''
AC_CHECK_LIB(m,sqrt,MATH_LIBS="-lm",,)
LIBS="$MATH_LIBS $LIBS"
AC_SUBST(MATH_LIBS)

#
# Find socket library
#
AC_SEARCH_LIBS(gethostbyname, resolv nsl)
AC_SEARCH_LIBS(socket, socket,
[
  AC_DEFINE([HAVE_SOCKET],[1],[Define to 1 if you have socket support.])
  MAGICK_FEATURES="DPC $MAGICK_FEATURES"
])

#
# Check for clock_gettime().
#
AC_SEARCH_LIBS(clock_gettime, rt,
[
  AC_DEFINE([HAVE_CLOCK_GETTIME],[1],[Define to 1 if you have clock_gettime.])
  AC_MSG_CHECKING([whether clock_gettime supports CLOCK_REALTIME])
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM(
      [[#include <time.h>]],
       [[clockid_t clockType = CLOCK_REALTIME;]])],
      [
        AC_MSG_RESULT(yes)
        AC_DEFINE([HAVE_CLOCK_REALTIME],[1],
          [Define to 1 if clock_gettime supports CLOCK_REALTIME.])
      ],
      AC_MSG_RESULT(no)
    )
  ],
  [
    AC_CHECK_FUNCS([gettimeofday ftime], [break])
  ]
)

########
#
# Check for function prototypes
#
########

AC_CHECK_DECLS([pread, pwrite],[],[],[
#include <unistd.h>])

AC_CHECK_DECLS([strlcpy],[],[],[
#include <strings.h>])

AC_CHECK_DECLS([vsnprintf],[],[],[
#include <stdio.h>
#include <stdarg.h>])

########
#
# C++ Support Tests (For Magick++)
#
########
have_magick_plus_plus='no'
if test "$with_magick_plus_plus" = 'yes'; then
    OLIBS="$LIBS"
    LIBS=''
    AC_LANG_PUSH(C++)

    # Full set of headers used...
    # algorithm cctype cerrno cmath cstdio cstdlib cstring ctime exception
    # functional iomanip iosfwd iostream iterator list string strstream utility
    AC_LANG([C++])
    AC_PROG_CXX
    AX_CXX_BOOL
    AX_CXX_NAMESPACES
    AX_CXX_NAMESPACE_STD
    AC_CXX_HAVE_STD_LIBS
    AC_OPENMP([C++])
    AC_LANG_POP

    AC_MSG_CHECKING([whether C++ compiler is sufficient for Magick++])
    if \
        test $ax_cv_cxx_bool = 'yes' && \
        test $ax_cv_cxx_namespaces = 'yes' && \
        test $ac_cv_cxx_have_std_libs = 'yes' && \
        test $ax_cv_cxx_have_std_namespace = 'yes'; then
        have_magick_plus_plus='yes'
    else
        have_magick_plus_plus='no (failed tests)'
    fi
    AC_MSG_RESULT([$have_magick_plus_plus])
    LIBS="$OLIBS"
fi
AM_CONDITIONAL(WITH_MAGICK_PLUS_PLUS, test "$have_magick_plus_plus" = 'yes')

# Only check for delegate libraries in subdirectories if requested.
if test "$enable_delegate_build" != 'no'; then
    # Check for delegate sub-directories and add -I & -L options as required.
    # This presumes that delegates are installed as detailed in the ImageMagick
    # README. If delegates are installed in a standard location where the
    # compiler will automatically find them then these options should not be
    # required.

    #
    # Most delegates have includes in the same directory as the library, but not all...
    #
    # Includes
    for dir in bzlib fftw fpx gslib/src jp2 jbig/libjbig jpeg lcms/include lzma magick openjp2/include png tiff/libtiff ttf/include wand webp/src wmf/include xml/include zlib; do
        if test -d "$builddir/$dir"; then
            CPPFLAGS="$CPPFLAGS -I$builddir/$dir"
        else
            if test -d "$srcdirfull/$dir"; then
                CPPFLAGS="$CPPFLAGS -I$srcdirfull/$dir"
            fi
        fi
    done

    # Libraries
    for dir in bzlib fftw fpx gslib/src jbig/libjbig jpeg lcms/src lzma magick openjp2 png tiff/libtiff ttf/objs wand webp wmf/src xml zlib; do
    if test -d "$builddir/$dir/.libs"; then
        LDFLAGS="$LDFLAGS -L$builddir/$dir/.libs"
    else
        if test -d "$srcdirfull/$dir/.libs"; then
            LDFLAGS="$LDFLAGS -L$srcdirfull/$dir/.libs"
        fi
    fi
    if test -d "$builddir/$dir"; then
        LDFLAGS="$LDFLAGS -L$builddir/$dir"
    else
        if test -d "$srcdirfull/$dir"; then
            LDFLAGS="$LDFLAGS -L$srcdirfull/$dir"
        fi
    fi
  done
fi

# Assume that delegate headers reside under same directory as ImageMagick
# installation prefix.
MAGICK_CPPFLAGS="-I$INCLUDE_DIR/${PACKAGE_NAME}-$MAGICK_MAJOR_VERSION $MAGICK_CPPFLAGS"

#
# Find the X11 RGB database
#
AC_CACHE_CHECK(for X11 configure files,im_cv_x_configure,
[# Look for the header file in a standard set of common directories.
# Check X11 before X11Rn because it is often a symlink to the current release.
    for ac_dir in          \
    /lib/usr/lib/X11       \
    /usr/X11/lib           \
    /usr/X11R4/lib         \
    /usr/X11R5/lib         \
    /usr/X11R6/lib         \
    /usr/X11R7/lib         \
    /usr/X386/lib          \
    /usr/XFree86/lib/X11   \
    /usr/athena/lib        \
    /usr/lib               \
    /usr/lib/X11           \
    /usr/lib/X11R4         \
    /usr/lib/X11R5         \
    /usr/lib/X11R6         \
    /usr/lib/X11R7         \
    /usr/local/X11/lib     \
    /usr/local/X11R4/lib   \
    /usr/local/X11R5/lib   \
    /usr/local/X11R6/lib   \
    /usr/local/lib         \
    /usr/local/lib/X11     \
    /usr/local/lib/X11R4   \
    /usr/local/lib/X11R5   \
    /usr/local/lib/X11R6   \
    /usr/local/lib/X11R7   \
    /usr/local/x11r5/lib   \
    /usr/lpp/Xamples/lib   \
    /usr/openwin/lib       \
    /usr/openwin/share/lib \
    /usr/unsupported/lib   \
    /usr/x386/lib          \
    ; do
    if test -f "$ac_dir/X11/rgb.txt"; then
      im_cv_x_configure="$ac_dir/X11/"
      break
    elif test -f "$ac_dir/rgb.txt"; then
      im_cv_x_configure="$ac_dir/"
      break
    fi

  done])
X11_CONFIGURE_PATH="$im_cv_x_configure"
case "${build_os}" in
  mingw* )
    X11ConfigurePath=`$WinPathScript "$X11ConfigurePath=" 1`
  ;;
esac
AC_DEFINE_UNQUOTED(X11_CONFIGURE_PATH,"$X11ConfigurePath",[Location of X11 configure files])

#
# Find OpenMP library
#
GOMP_LIBS=''
if test "$enable_openmp" != 'no'; then
  if test "${GCC}" = "yes"; then
    # Open64 (passes for GCC but uses different OpenMP implementation)
    if test "x$GOMP_LIBS" = x ; then
      if $CC --version 2>&1 | grep Open64 > /dev/null ; then
        AC_CHECK_LIB(openmp,omp_get_num_procs,GOMP_LIBS="-lopenmp",,)
      fi
    fi
    # Clang (passes for GCC but uses different OpenMP implementation)
    if test "x$LIB_OMP" = x ; then
      if $CC --version 2>&1 | grep clang > /dev/null ; then
        AC_CHECK_LIB(omp,GOMP_parallel_start,LIB_OMP="-lomp",,)
      fi
    fi
    # GCC
    if test "x$GOMP_LIBS" = x ; then
      AC_CHECK_LIB(gomp,GOMP_parallel_start,GOMP_LIBS="-lgomp",,)
    fi
  else
    # Sun CC
    if test "x$GOMP_LIBS" = x ; then
      AC_CHECK_LIB(mtsk,sunw_mp_register_warn,GOMP_LIBS="-lmtsk",,)
    fi
    # AIX xlc
    if test "x$GOMP_LIBS" = x ; then
    AC_CHECK_LIB(xlsmp,_xlsmpFlush,GOMP_LIBS="-lxlsmp",,)
    fi
    # SGI IRIX 6.5 MIPSpro C/C++
    if test "x$GOMP_LIBS" = x ; then
    AC_CHECK_LIB(mp,mp_destroy,GOMP_LIBS="-lmp",,)
    fi
  fi
  LIBS="$GOMP_LIBS $LIBS"
fi
AC_SUBST(GOMP_LIBS)

#
# Check for jemalloc.
#
have_jemalloc='no'
JEMALLOC_LIBS=''
if test "$with_jemalloc" != 'no'; then
  AC_MSG_CHECKING(for JEMALLOC support )
  AC_MSG_RESULT()
  failed=0
  passed=0
  AC_CHECK_HEADER([jemalloc/jemalloc.h],[passed=`expr $passed + 1`],[failed=`expr $failed + 1`])
  AC_CHECK_LIB(jemalloc,[malloc_stats_print],[passed=`expr $passed + 1`],[failed=`expr $failed + 1`],)
  AC_MSG_CHECKING(if jemalloc memory allocation library is complete)
  if test $passed -gt 0; then
    if test $failed -gt 0; then
      AC_MSG_RESULT(no -- some components failed test)
      have_jemalloc='no (failed tests)'
    else
      JEMALLOC_LIBS='-ljemalloc'
      LIBS="$JEMALLOC_LIBS $LIBS"
      AC_DEFINE(HasJEMALLOC,1,Define if you have jemalloc memory allocation library)
      AC_MSG_RESULT(yes)
      have_jemalloc='yes'
    fi
  else
    AC_MSG_RESULT(no)
  fi
fi
AM_CONDITIONAL(HasJEMALLOC, test "$have_jemalloc" = 'yes')
AC_SUBST(JEMALLOC_LIBS)

#
# Check for umem.
#
have_umem='no'
UMEM_LIBS=''
if test "$with_umem" != 'no'; then
  AC_MSG_CHECKING(for UMEM support )
  AC_MSG_RESULT()
  failed=0
  passed=0
  AC_CHECK_HEADER(umem.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
  AC_CHECK_LIB(umem,umem_alloc,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_CHECK_LIB(umem,umem_free,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
  AC_MSG_CHECKING(if umem memory allocation library is complete)
  if test $passed -gt 0; then
    if test $failed -gt 0; then
      AC_MSG_RESULT(no -- some components failed test)
      have_umem='no (failed tests)'
    else
      UMEM_LIBS='-lumem'
      LIBS="$UMEM_LIBS $LIBS"
      AC_DEFINE(HasUMEM,1,Define if you have umem memory allocation library)
      AC_MSG_RESULT(yes)
      have_umem='yes'
    fi
  else
    AC_MSG_RESULT(no)
  fi
fi
AM_CONDITIONAL(HasUMEM, test "$have_umem" = 'yes')
AC_SUBST(UMEM_LIBS)

#
# Add support for ccmalloc memory debugging library if requested
#
have_ccmalloc='no'
CCMALLOC_LIBS=''
if test "$enable_ccmalloc" = 'yes'; then
    AC_PATH_PROG(CCMALLOCDelegate,ccmalloc,)
    if test -n "$CCMALLOCDelegate"; then
        eval `grep PREFIX= $CCMALLOCDelegate | sed -e 's/PREFIX/CCMALLOC_PREFIX/'`
        OLIBS="$LIBS"
        # Assume that gcc is used with ccmalloc.
        LIBS="$LIBS $CCMALLOC_PREFIX/lib/ccmalloc-gcc.o"
       AC_CHECK_LIB(ccmalloc,ccmalloc_malloc,CCMALLOC_LIBS="$CCMALLOC_PREFIX/lib/ccmalloc-gcc.o -lccmalloc -ldl",,-ldl)
        if test -n "$CCMALLOC_LIBS"; then
            LIBS="$OLIBS"
            LIBS="$LIBS $CCMALLOC_LIBS"
            have_ccmalloc='yes'
        else
            LIBS="$OLIBS"
        fi
    fi
fi

#
# Add support for 'huge pages' if requested
#
AS_IF([test "x$enable_hugepages" != "xno"],[
  AC_DEFINE(HAVE_HUGEPAGES, 1, [Compile with hugepage support])
])

#
# Add support for efence memory debugging library if requested
#
if test "$enable_efence" = 'yes'; then
    EFENCE_LIBS='-lefence'
    LIBS="$EFENCE_LIBS $LIBS"
fi

dnl ===========================================================================

#
# Check for BZLIB
#

AC_ARG_WITH([bzlib],
    [AC_HELP_STRING([--without-bzlib],
                    [disable BZLIB support])],
    [with_bzlib=$withval],
    [with_bzlib='yes'])

if test "$with_bzlib" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-bzlib=$with_bzlib "
fi

have_bzlib='no'
if test "$with_bzlib" != 'no'; then
    BZLIB_LIBS=''
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for BZLIB])
    AC_MSG_RESULT([])
    failed=0
    passed=0
    found_libbz=0
    AC_CHECK_HEADER(bzlib.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_LIB(bz2,BZ2_bzDecompress,found_libbz=`expr $found_libbz + 1`,,)
    if test "$native_win32_build" = 'yes'; then
      AC_CHECK_LIB(bz2,_imp__BZ2_decompress,found_libbz=`expr $found_libbz + 1`,,)
    fi
    if test $found_libbz -gt 0; then
      passed=`expr $passed + 1`
    else
      failed=`expr $failed + 1`
    fi
    AC_MSG_CHECKING(if BZLIB package is complete)
    if test $passed -gt 0; then
      if test $failed -gt 0; then
        AC_MSG_RESULT(no -- some components failed test)
        have_bzlib='no (failed tests)'
      else
        BZLIB_LIBS='-lbz2'
        LIBS="$BZLIB_LIBS $LIBS"
        AC_DEFINE(BZLIB_DELEGATE,1,Define if you have the bzip2 library)
        AC_MSG_RESULT(yes)
        have_bzlib='yes'
      fi
    else
      AC_MSG_RESULT(no)
    fi
fi
AM_CONDITIONAL(BZLIB_DELEGATE, test "$have_bzlib" = 'yes')
AC_SUBST(BZLIB_LIBS)

#
# Find the X11 include and library directories.
#
IPC_LIBS=''
X11_LIBS=''
XEXT_LIBS=''
XT_LIBS=''
AC_PATH_XTRA
if test "$no_x" != 'yes'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for X11])
    AC_MSG_RESULT([])
    LDFLAGS="$LDFLAGS $X_LIBS"
    X11_LIBS="$X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
    LIBS="$X11_LIBS $LIBS"
    CPPFLAGS="$CPPFLAGS $X_CFLAGS"

    AC_DEFINE(X11_DELEGATE,1,Define if you have X11 library)dnl

    #
    # Check for X11 shared memory extension
    #
    # shmctl is required to support the shared memory extension
    AC_CHECK_FUNC([shmctl],[have_shmctl='yes'],[])
    if test "$have_shmctl" != 'yes'; then
        PERSIST_LIBS=$LIBS
        LIBS="$LIBS -lcygipc"
        AC_TRY_LINK_FUNC([shmctl],[have_shmctl='yes'; IPC_LIBS='-lcygipc'],[])
        LIBS=$PERSIST_LIBS
    fi

    if test "$have_shmctl" = 'yes'; then
        AC_CHECK_LIB([Xext],[XShmAttach],[XEXT_LIBS='-lXext' ; AC_DEFINE(HAVE_SHARED_MEMORY,1,X11 server supports shared memory extension)],[],[])
    fi

    #
    # Check for X11 shape extension
    #
    AC_CHECK_LIB([Xext],[XShapeCombineMask],[XEXT_LIBS='-lXext' ; AC_DEFINE(HAVE_SHAPE,1,X11 server supports shape extension)],[],[])
    AC_CHECK_LIB(Xt,XtSetEventDispatcher,XT_LIBS='-lXt',,)
    LIBS="$XEXT_LIBS $XT_LIBS $LIBS"
fi
if test "$no_x" != 'yes'; then
  have_x='yes'
else
  have_x='no'
fi
AM_CONDITIONAL(X11_DELEGATE, test "$have_x" = 'yes')
AC_SUBST(X11_LIBS)
AC_SUBST(XEXT_LIBS)

dnl ===========================================================================

#
# Check for ZLIB
#
AC_ARG_WITH([zlib],
    [AC_HELP_STRING([--without-zlib],
                    [disable ZLIB support])],
    [with_zlib=$withval],
    [with_zlib='yes'])

if test "$with_zlib" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-zlib=$with_zlib "
fi

have_zlib='no'
ZLIB_CFLAGS=""
ZLIB_LIBS=""
ZLIB_PKG=""
if test "x$with_zlib" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(ZLIB,[zlib >= 1.0.0], have_zlib=yes, have_zlib=no)
  AC_MSG_RESULT([])
fi

if test "$have_zlib" = 'yes'; then
  AC_DEFINE(ZLIB_DELEGATE,1,Define if you have ZLIB library)
  CFLAGS="$ZLIB_CFLAGS $CFLAGS"
  LIBS="$ZLIB_LIBS $LIBS"
fi

AM_CONDITIONAL(ZLIB_DELEGATE, test "$have_zlib" = 'yes')
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)

dnl ===========================================================================

#
# Check for ZSTD
#
AC_ARG_WITH([zstd],
    [AC_HELP_STRING([--without-zstd],
                    [disable ZSTD support])],
    [with_zstd=$withval],
    [with_zstd='yes'])

if test "$with_zstd" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-zstd=$with_zstd "
fi

have_zstd='no'
ZSTD_CFLAGS=""
ZSTD_LIBS=""
ZSTD_PKG=""
if test "x$with_zstd" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(LIBZSTD,[libzstd >= 1.0.0], have_zstd=yes, have_zstd=no)
  AC_MSG_RESULT([])
fi

if test "$have_zstd" = 'yes'; then
  AC_DEFINE(ZSTD_DELEGATE,1,Define if you have ZSTD library)
  CFLAGS="$ZSTD_CFLAGS $CFLAGS"
  LIBS="$ZSTD_LIBS $LIBS"
fi

AM_CONDITIONAL(ZSTD_DELEGATE, test "$have_zstd" = 'yes')
AC_SUBST(ZSTD_CFLAGS)
AC_SUBST(ZSTD_LIBS)

dnl ===========================================================================

# whether modules are built or not.
with_ltdl='yes'
have_ltdl='no'
LTDL_LIBS=''
if test "$build_modules" != 'no' || test "X$ax_cv_check_cl_libcl" != Xno; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  AC_MSG_CHECKING([for libltdl])
  AC_MSG_RESULT([])
  failed=0
  passed=0
  AC_CHECK_HEADER([ltdl.h],[passed=`expr $passed + 1`],[failed=`expr $failed + 1`])
  AC_CHECK_LIB([ltdl],[lt_dlinit],[passed=`expr $passed + 1`],[failed=`expr $failed + 1`],)
  AC_MSG_CHECKING([if libltdl package is complete])
  if test $passed -gt 0 ; then
    if test $failed -gt 0 ; then
      AC_MSG_RESULT([no -- some components failed test])
      have_ltdl='no (failed tests)'
    else
      LTDL_LIBS='-lltdl'
      LIBS="$LTDL_LIBS $LIBS"
      AC_DEFINE(LTDL_DELEGATE,1,[Define if using libltdl to support dynamically loadable modules and OpenCL])
      AC_DEFINE_UNQUOTED([LTDL_MODULE_EXT],["${shrext_cmds}"],[Native module suffix])
      AC_MSG_RESULT([yes])
      have_ltdl='yes'
    fi
  else
    AC_MSG_RESULT([no])
  fi
  if test "$have_ltdl" != 'yes' ; then
    AC_MSG_FAILURE([libltdl is required for modules and OpenCL builds],[1])
  fi
fi
AM_CONDITIONAL(WITH_LTDL, test "$have_ltdl" != 'no')

#
# If profiling, then check for -ldl and dlopen (required for Solaris & gcc)
#
LIB_DL=''
if test "$enable_profiling" = 'yes'; then
    AC_CHECK_LIB(dl,dlopen,LIB_DL='-ldl',,)
    LIBS="$LIB_DL $LIBS"
fi
AC_SUBST(LIB_DL)

dnl ===========================================================================

#
# Set Apple font directory.
#
AC_ARG_WITH([apple-font-dir],
    [AC_HELP_STRING([--with-apple-font-dir=DIR],
                    [Apple font directory])],
    [with_apple_font_dir=$withval],
    [with_apple_font_dir='default'])

if test "$with_apple_font_dir" != 'default'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-apple-font-dir=$with_apple_font_dir "
fi

dnl ===========================================================================

#
# Check for Autotrace delegate library.
#
AC_ARG_WITH([autotrace],
    [AC_HELP_STRING([--with-autotrace],
                    [enable autotrace support])],
        [with_autotrace=$withval],
        [with_autotrace='no'])

if test "$with_autotrace" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-autotrace=$with_autotrace "
fi

have_autotrace='no'
AUTOTRACE_CFLAGS=""
AUTOTRACE_LIBS=""
AUTOTRACE_PKG=""
if test "x$with_autotrace" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(AUTOTRACE,[autotrace >= 0.31.1], have_autotrace=yes, have_autotrace=no)
  AC_MSG_RESULT([])
fi

if test "$have_autotrace" = 'yes'; then
  AC_DEFINE(AUTOTRACE_DELEGATE,1,Define if you have AUTOTRACE library)
  CFLAGS="$AUTOTRACE_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(AUTOTRACE_DELEGATE,test "$have_autotrace" = 'yes')
AC_SUBST(AUTOTRACE_CFLAGS)
AC_SUBST(AUTOTRACE_LIBS)

dnl ===========================================================================

#
# Check for Display Postscript delegate library.
#
AC_ARG_WITH([dps],
    [AC_HELP_STRING([--without-dps],
                    [disable Display Postscript support])],
    [with_dps=$withval],
    [with_dps='yes'])

if test "$with_dps" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-dps=$with_dps "
fi

have_dps='no'
DPS_LIBS=''
if test "$with_dps" != 'no' && test "$with_x" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for DPS])
    AC_MSG_RESULT([])
    failed=0
    passed=0
    PERSIST_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -I${ac_x_includes}/X11"
    AC_CHECK_HEADER(DPS/dpsXclient.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    # DPS issues:
    # XFree86-4.x needs -lXt to provide XtMalloc for -ldps.
    # Cygwin doesn't deliver -lXt as a DLL, which prevents a DLL build.
    # Adobe DPS (as delivered on Solaris) doesn't require -lXt.
    # ImageMagick itself doesn't use -lXt.
    have_libdps='no'
    LIBDPS_XT=''
    AC_CHECK_LIB(dps,DPSInitialize,have_libdps='yes',have_libdps='no',)
    if test "$have_libdps" != 'yes'; then
        # Unset cache variable so we can try again.
        unset ac_cv_lib_dps_DPSInitialize
        AC_CHECK_LIB(dps,DPSInitialize,have_libdps='yes',have_libdps='no',-lXt)
        if test "$have_libdps" = 'yes'; then
            LIBDPS_XT='-lXt'
        fi
    fi
    if test "$have_libdps" = 'yes'; then
        passed=`expr $passed + 1`
    else
        failed=`expr $failed + 1`
    fi
    AC_CHECK_LIB(dpstk,XDPSPixelsPerPoint,passed=`expr $passed + 1`,failed=`expr $failed + 1`,-ldps $LIBDPS_XT)
    AC_MSG_CHECKING([if DPS package is complete])
    if test $passed -gt 0; then
        if test $failed -gt 0; then
            AC_MSG_RESULT([no -- some components failed test])
            have_dps='no (failed tests)'
            CPPFLAGS="$PERSIST_CPPFLAGS"
        else
            DPS_LIBS="-ldpstk -ldps ${LIBDPS_XT}"
            LIBS="$DPS_LIBS $LIBS"
            AC_DEFINE(DPS_DELEGATE,1,Define if you have Display Postscript)
            AC_MSG_RESULT([yes])
            have_dps='yes'
        fi
    else
        AC_MSG_RESULT([no])
        CPPFLAGS=$PERSIST_CPPFLAGS
    fi
fi
AM_CONDITIONAL(DPS_DELEGATE, test "$have_dps" = 'yes')
AC_SUBST(DPS_LIBS)

dnl ===========================================================================

#
# Set DejaVu font directory.
#
AC_ARG_WITH([dejavu-font-dir],
    [AC_HELP_STRING([--with-dejavu-font-dir=DIR],
                    [DejaVu font directory])],
    [with_dejavu_font_dir=$withval],
    [with_dejavu_font_dir='default'])

if test "$with_dejavu_font_dir" != 'default'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-dejavu-font-dir=$with_dejavu_font_dir "
fi

dnl ===========================================================================

#
# Check for FFTW delegate library.
#
AC_ARG_WITH([fftw],
    [AC_HELP_STRING([--without-fftw],
                    [disable FFTW support])],
    [with_fftw=$withval],
    [with_fftw='yes'])

if test "$with_fftw" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-fftw=$with_fftw "
fi

have_fftw='no'
FFTW_CFLAGS=""
FFTW_LIBS=""
FFTW_PKG=""
if test "x$with_fftw" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(fftw3,[fftw3 >= 3.0.0], have_fftw=yes, have_fftw=no)
  AC_MSG_RESULT([])
fi

if test "$have_fftw" = 'yes'; then
  AC_DEFINE(FFTW_DELEGATE,1,Define if you have FFTW library)
  FFTW_CFLAGS="$fftw3_CFLAGS"
  FFTW_LIBS="$fftw3_LIBS"
  CFLAGS="$fftw3_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(FFTW_DELEGATE, test "$have_fftw" = 'yes')
AC_SUBST(FFTW_CFLAGS)
AC_SUBST(FFTW_LIBS)

dnl ===========================================================================

#
# Check for FLIF delegate library.
#
AC_ARG_WITH(flif,
    [AC_HELP_STRING([--without-flif],
                    [disable FLIF support])],
    [with_flif=$withval],
    [with_flif='yes'])

if test "$with_flif" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-flif=$with_flif "
fi

have_flif='no'
FLIF_LIBS=''
if test "$with_flif" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for FLIF])
    AC_MSG_RESULT([])
    failed=0
    passed=0
    AC_CHECK_HEADER(flif.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_CHECK_LIB(flif,flif_create_decoder,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_MSG_CHECKING([if FLIF package is complete])
    if test $passed -gt 0; then
        if test $failed -gt 0; then
            AC_MSG_RESULT([no -- some components failed test])
            have_flif='no (failed tests)'
        else
            FLIF_LIBS='-lflif'
            LIBS="$FLIF_LIBS $LIBS"
            AC_DEFINE(FLIF_DELEGATE,1,Define if you have FLIF library)
            AC_MSG_RESULT([yes])
            have_flif='yes'
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AM_CONDITIONAL(FLIF_DELEGATE,test "$have_flif" = 'yes')
AC_SUBST(FLIF_LIBS)

dnl ===========================================================================

#
# Check for FlashPIX delegate library.
#
AC_ARG_WITH([fpx],
    [AC_HELP_STRING([--without-fpx],
                    [disable FlashPIX support])],
    [with_fpx=$withval],
    [with_fpx='yes'])

if test "$with_fpx" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-fpx=$with_fpx "
fi

have_fpx='no'
FPX_LIBS=''
if test "$with_fpx" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for FlashPIX])
    AC_MSG_RESULT([])
    failed=0
    passed=0
    AC_LANG_PUSH(C++)
    AC_CHECK_HEADER(fpxlib.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_LIB(fpx,FPX_OpenImageByFilename,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_LANG_POP
    AC_MSG_CHECKING([if FlashPIX package is complete])
    if test $passed -gt 0; then
        if test $failed -gt 0; then
            AC_MSG_RESULT([no -- some components failed test])
            have_fpx='no (failed tests)'
        else
            FPX_LIBS='-lfpx'
            AC_DEFINE(FPX_DELEGATE,1,Define if you have FlashPIX library)
            AC_MSG_RESULT([yes])
            have_fpx='yes'
            PERLMAINCC="$CXX"
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AM_CONDITIONAL(FPX_DELEGATE, test "$have_fpx" = 'yes')
AC_SUBST(FPX_LIBS)

dnl ===========================================================================

#
# Check for DJVU delegate library.
#
AC_ARG_WITH([djvu],
    [AC_HELP_STRING([--without-djvu],
                    [disable DjVu support])],
    [with_djvu=$withval],
    [with_djvu='yes'])

if test "$with_djvu" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-djvu=$with_djvu "
fi

have_djvu='no'
DJVU_CFLAGS=""
DJVU_LIBS=""
DJVU_PKG=""
if test "x$with_djvu" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(ddjvuapi,[ddjvuapi >= 3.5.0], have_djvu=yes, have_djvu=no)
  AC_MSG_RESULT([])
fi

if test "$have_djvu" = 'yes'; then
  AC_DEFINE(DJVU_DELEGATE,1,Define if you have DJVU library)
  DJVU_CFLAGS="$ddjvuapi_CFLAGS"
  DJVU_LIBS="$ddjvuapi_LIBS"
  CFLAGS="$ddjvuapi_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(DJVU_DELEGATE, test "$have_djvu" = 'yes')
AC_SUBST(DJVU_CFLAGS)
AC_SUBST(DJVU_LIBS)

dnl ===========================================================================

#
# Check for fontconfig delegate library.
#
AC_ARG_WITH([fontconfig],
    [AC_HELP_STRING([--without-fontconfig],
                    [disable fontconfig support])],
        [with_fontconfig=$withval],
        [with_fontconfig='yes'])

if test "$with_fontconfig" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-fontconfig=$with_fontconfig "
fi

have_fontconfig='no'
FONTCONFIG_CFLAGS=""
FONTCONFIG_LIBS=""
FONTCONFIG_PKG=""
if test "x$with_fontconfig" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(FONTCONFIG,[fontconfig >= 2.1.0], have_fontconfig=yes, have_fontconfig=no)
  AC_MSG_RESULT([])
fi

if test "$have_fontconfig" = 'yes'; then
  AC_DEFINE(FONTCONFIG_DELEGATE,1,Define if you have FONTCONFIG library)
  CFLAGS="$FONTCONFIG_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(FONTCONFIG_DELEGATE,test "$have_fontconfig" = 'yes')
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)

dnl ===========================================================================

#
# Check for the Freetype delegate library.
#
AC_ARG_WITH([freetype],
    [AC_HELP_STRING([--without-freetype],
                    [disable Freetype support])],
    [with_freetype=$withval],
    [with_freetype='yes'])

if test "$with_freetype" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-freetype=$with_freetype "
fi

have_freetype='no'
FREETYPE_CFLAGS=""
FREETYPE_LIBS=""
FREETYPE_PKG=""
if test "x$with_freetype" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(FREETYPE,[freetype2], have_freetype=yes, have_freetype=no)
  AC_MSG_RESULT([])
fi

if test "$have_freetype" = 'yes'; then
  AC_DEFINE(FREETYPE_DELEGATE,1,Define if you have FREETYPE library)
  CFLAGS="$FREETYPE_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(FREETYPE_DELEGATE, test "$have_freetype" = 'yes')
AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

dnl ===========================================================================

#
# Check for the raqm delegate library.
#
AC_ARG_WITH([raqm],
    [AC_HELP_STRING([--without-raqm],
                    [disable Raqm support])],
    [with_raqm=$withval],
    [with_raqm='yes'])

if test "$with_raqm" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-raqm=$with_raqm "
fi

have_raqm='no'
RAQM_CFLAGS=""
RAQM_LIBS=""
RAQM_PKG=""
if test "x$with_raqm" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(RAQM,[raqm], have_raqm=yes, have_raqm=no)
  AC_MSG_RESULT([])
fi

if test "$have_raqm" = 'yes'; then
  AC_DEFINE(RAQM_DELEGATE,1,Define if you have RAQM library)
  CFLAGS="$RAQM_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(RAQM_DELEGATE, test "$have_raqm" = 'yes')
AC_SUBST(RAQM_CFLAGS)
AC_SUBST(RAQM_LIBS)

dnl ===========================================================================

#
# Check for Ghostscript library or framework.
#
# Test for iapi.h & test for gsapi_new_instance in -lgs
# or -framework Ghostscript

AC_ARG_WITH([gslib],
    [AC_HELP_STRING([--with-gslib],
                    [enable Ghostscript library support])],
    [with_gslib=$withval],
    [with_gslib='no'])

gslib_framework='no'
if test "$with_gslib" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-gslib=$with_gslib "
fi

have_gslib='no'
GS_LIBS=''
if test "$with_gslib" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for Ghostscript])
    AC_MSG_RESULT([])
    framework=0
    failed=0
    passed=0
    AC_CHECK_HEADER(ghostscript/iapi.h,passed=`expr $passed + 1`,
        failed=`expr $failed + 1`,)
    AC_CHECK_HEADER(ghostscript/ierrors.h,passed=`expr $passed + 1`,
        failed=`expr $failed + 1`,)
    AC_CHECK_FRAMEWORK(Ghostscript,gsapi_new_instance,framework=`expr $framework + 1`,
    AC_CHECK_LIB(gs,gsapi_new_instance,passed=`expr $passed + 1`,failed=`expr $failed + 1`,),)
    AC_MSG_CHECKING([if Ghostscript package is complete])
    if test $passed -gt 0; then
        if test $failed -gt 0; then
            AC_MSG_RESULT([no -- some components failed test])
            have_gslib='no (failed tests)'
        else
            if test $framework -gt 0; then
                GS_LIBS='-framework Ghostscript'
                gslib_framework='yes'
                AC_MSG_RESULT([yes, using framework.])
            else
                AC_MSG_RESULT([yes, using library.])
                GS_LIBS='-lgs'
            fi
            LIBS="$GS_LIBS $LIBS"
            AC_DEFINE(GS_DELEGATE,1,Define if you have Ghostscript library or framework)
            have_gslib='yes'
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AM_CONDITIONAL(GS_DELEGATE, test "$have_gslib" = 'yes')
AC_SUBST(GS_LIBS)

# Set default font search path
AC_ARG_WITH([fontpath],
    [AC_HELP_STRING([--with-fontpath=DIR],
                    [prepend to default font search path])],
    [with_fontpath=$withval],
    [with_fontpath=''])

if test "$with_fontpath" != "yes" && test -z "$with_fontpath"; then
    with_fontpath=''
else
    AC_DEFINE_UNQUOTED(MAGICK_FONT_PATH,"$with_fontpath",[Define to prepend to default font search path.])
fi
if test "$with_fontpath=" != ''; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-fontpath=$with_fontpath "
fi

# Set Ghostscript font directory
AC_ARG_WITH([gs-font-dir],
    [AC_HELP_STRING([--with-gs-font-dir=DIR],
                    [Ghostscript font directory])],
    [with_gs_font_dir=$withval],
    [with_gs_font_dir='default'])

if test "$with_gs_font_dir" != 'default'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-gs-font-dir=$with_gs_font_dir "
fi

dnl ===========================================================================

#
# Check for GVC delegate library.
#
AC_ARG_WITH(gvc,
    [AC_HELP_STRING([--with-gvc],
                    [enable GVC support])],
    [with_gvc=$withval],
    [with_gvc='yes'])

if test "$with_gvc" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-gvc=$with_gvc "
fi

GVC_PKG=""
if test "x$with_gvc" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(GVC,[libgvc >= 2.9.0], have_gvc=yes, have_gvc=no)
  AC_MSG_RESULT([])
fi

if test "$have_gvc" = 'yes'; then
  AC_DEFINE(GVC_DELEGATE,1,Define if you have GVC library)
  CFLAGS="$GVC_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(GVC_DELEGATE, test "$have_gvc" = 'yes')
AC_SUBST(GVC_CFLAGS)
AC_SUBST(GVC_LIBS)

dnl ===========================================================================

#
# Check for the HEIC delegate library.
#
AC_ARG_WITH([heic],
    [AC_HELP_STRING([--without-heic],
                    [disable HEIC support])],
    [with_heic=$withval],
    [with_heic='yes'])

if test "$with_heic" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-heic=$with_heic "
fi

have_heic='no'
HEIF_CFLAGS=""
HEIF_LIBS=""
HEIF_PKG=""
if test "x$with_heic" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(HEIF,[libheif], have_heic=yes, have_heic=no)
  AC_MSG_RESULT([])
fi

if test "$have_heic" = 'yes'; then
  AC_DEFINE(HEIC_DELEGATE,1,Define if you have libheif library)
  CFLAGS="$HEIF_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(HEIC_DELEGATE, test "$have_heic" = 'yes')
AC_SUBST(HEIF_CFLAGS)
AC_SUBST(HEIF_LIBS)

dnl ===========================================================================

#
# Check for JBIG delegate library.
#
AC_ARG_WITH([jbig],
    [AC_HELP_STRING([--without-jbig],
                    [disable JBIG support])],
    [with_jbig=$withval],
    [with_jbig='yes'])

have_jbig='no'
JBIG_LIBS=''
if test "$with_jbig" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for JBIG])
    AC_MSG_RESULT([])
    failed=0
    passed=0
    AC_CHECK_HEADER(jbig.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_LIB(jbig,jbg_dec_init,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_MSG_CHECKING([if JBIG package is complete])
    if test $passed -gt 0; then
        if test $failed -gt 0; then
            AC_MSG_RESULT([no -- some components failed test])
            have_jbig='no (failed tests)'
        else
            JBIG_LIBS='-ljbig'
            LIBS="$JBIG_LIBS $LIBS"
            AC_DEFINE(JBIG_DELEGATE,1,Define if you have JBIG library)
            AC_MSG_RESULT([yes])
            have_jbig='yes'
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AM_CONDITIONAL(JBIG_DELEGATE, test "$have_jbig" = 'yes')
AC_SUBST(JBIG_LIBS)

dnl ===========================================================================

#
# Check for JPEG delegate library.
#
AC_ARG_WITH([jpeg],
    [AC_HELP_STRING([--without-jpeg],
                    [disable JPEG support])],
    [with_jpeg=$withval],
    [with_jpeg='yes'])

if test "$with_jpeg" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-jpeg=$with_jpeg "
fi

have_jpeg='no'
JPEG_LIBS=''
if test "$with_jpeg" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for JPEG])
    AC_MSG_RESULT([])
    failed=0
    passed=0
    AC_CHECK_HEADER(jconfig.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_HEADER(jerror.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_HEADER(jmorecfg.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_HEADER(jpeglib.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_LIB(jpeg,jpeg_read_header,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)

# Test for compatible JPEG library
if test "$ac_cv_jpeg_version_ok" != 'yes'; then
AC_CACHE_CHECK(for JPEG library is version 6b or later, ac_cv_jpeg_version_ok,
[AC_TRY_COMPILE(
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
,
changequote(<<, >>)dnl
<<
#if JPEG_LIB_VERSION < 62
#error IJG JPEG library must be version 6b or newer!
#endif
return 0;
>>,
changequote([, ])dnl
ac_cv_jpeg_version_ok='yes'; passed=`expr $passed + 1`,
ac_cv_jpeg_version_ok='no'; failed=`expr $failed + 1`)])
fi
    AC_MSG_CHECKING([if JPEG package is complete])
    if test $passed -gt 0; then
        if test $failed -gt 0; then
            AC_MSG_RESULT([no -- some components failed test])
            have_jpeg='no (failed tests)'
        else
            JPEG_LIBS='-ljpeg'
            LIBS="$JPEG_LIBS $LIBS"
            AC_DEFINE(JPEG_DELEGATE,1,Define if you have JPEG library)
            AC_MSG_RESULT([yes])
            have_jpeg='yes'
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AM_CONDITIONAL(JPEG_DELEGATE, test "$have_jpeg" = 'yes')
AC_SUBST(JPEG_LIBS)

dnl ===========================================================================

#
# Check for LCMS delegate library.
#
AC_ARG_WITH(lcms,
        [  --without-lcms          disable lcms (v1.1X) support],
        [with_lcms=$withval],
        [with_lcms='yes'])
if test "$with_lcms" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-lcms=$with_lcms "
fi

have_lcms='no'
LCMS_CFLAGS=""
LCMS_LIBS=""
LCMS_PKG=""
if test "x$with_lcms" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(LCMS2,[lcms2 >= 2.0.0], have_lcms=yes, have_lcms=no)
  AC_MSG_RESULT([])
fi

if test "$have_lcms" = 'yes'; then
  AC_DEFINE(LCMS_DELEGATE,1,Define if you have LCMS library)
  LCMS_CFLAGS="$LCMS2_CFLAGS"
  LCMS_LIBS="$LCMS2_LIBS"
  CFLAGS="$LCMS2_CFLAGS $CFLAGS"
  AC_CHECK_HEADER(lcms2/lcms2.h,have_lcms_header='yes',,)
  if test "$have_lcms_header" = 'yes'; then
    AC_DEFINE(HAVE_LCMS2_LCMS2_H,1,Define if you have the <lcms2/lcms2.h> header file.)
  else
    AC_DEFINE(HAVE_LCMS2_H,1,Define if you have the <lcms2.h> header file.)
  fi
fi

AM_CONDITIONAL(LCMS_DELEGATE, test "$have_lcms" = 'yes')
AC_SUBST(LCMS_CFLAGS)
AC_SUBST(LCMS_LIBS)

dnl ===========================================================================

#
# Check for the OpenJP2 delegate library.
#
AC_ARG_WITH([openjp2],
    [AC_HELP_STRING([--without-openjp2],
                    [disable OpenJP2 support])],
    [with_openjp2=$withval],
    [with_openjp2='yes'])

if test "$with_openjp2" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-openjp2=$with_openjp2 "
fi

have_openjp2='no'
LIBOPENJP2_CFLAGS=""
LIBOPENJP2_LIBS=""
LIBOPENJP2_PKG=""
if test "x$with_openjp2" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(LIBOPENJP2,[libopenjp2 >= 2.1.0], have_openjp2=yes, have_openjp2=no)
  AC_MSG_RESULT([])
fi

if test "$have_openjp2" = 'yes'; then
  AC_DEFINE(LIBOPENJP2_DELEGATE,1,Define if you have OPENJP2 library)
  CFLAGS="$LIBOPENJP2_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(LIBOPENJP2_DELEGATE, test "$have_openjp2" = 'yes')
AC_SUBST(LIBOPENJP2_CFLAGS)
AC_SUBST(LIBOPENJP2_LIBS)


dnl ===========================================================================

#
# Check for the LQR (Liquid Rescale) delegate library.
#
AC_ARG_WITH([lqr],
    [AC_HELP_STRING([--without-lqr],
                    [disable Liquid Rescale support])],
    [with_lqr=$withval],
    [with_lqr='yes'])

if test "$with_lqr" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-lqr=$with_lqr "
fi

have_lqr='no'
LQR_CFLAGS=""
LQR_LIBS=""
LQR_PKG=""
if test "x$with_lqr" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(LQR,[lqr-1 >= 0.1.0], have_lqr=yes, have_lqr=no)
  AC_MSG_RESULT([])
fi

if test "$have_lqr" = 'yes'; then
  AC_DEFINE(LQR_DELEGATE,1,Define if you have LQR library)
  CFLAGS="$LQR_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(LQR_DELEGATE, test "$have_lqr" = 'yes')
AC_SUBST(LQR_CFLAGS)
AC_SUBST(LQR_LIBS)

dnl ===========================================================================

# Disable LZMA (lzma library)
AC_ARG_WITH(lzma,
            [  --without-lzma          disable LZMA support],
            [with_lzma=$withval],
            [with_lzma='yes'])
if test "$with_lzma" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-lzma=$with_lzma "
fi

LZMA_PKG=""
if test "x$with_lzma" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(LZMA,[liblzma >= 2.9.0], have_lzma=yes, have_lzma=no)
  AC_MSG_RESULT([])
fi

if test "$have_lzma" = 'yes'; then
  AC_DEFINE(LZMA_DELEGATE,1,Define if you have LZMA library)
  CFLAGS="$LZMA_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(LZMA_DELEGATE, test "$have_lzma" = 'yes')
AC_SUBST(LZMA_CFLAGS)
AC_SUBST(LZMA_LIBS)

dnl ===========================================================================

#
# Check for the OpenEXR delegate library.
#
AC_ARG_WITH([openexr],
    [AC_HELP_STRING([--without-openexr],
                    [disable OpenEXR support])],
    [with_openexr=$withval],
    [with_openexr='yes'])

if test "$with_openexr" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-openexr=$with_openexr "
fi

have_openexr='no'
OPENEXR_CFLAGS=""
OPENEXR_LIBS=""
OPENEXR_PKG=""
if test "x$with_openexr" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(OPENEXR,[OpenEXR >= 1.0.6], have_openexr=yes, have_openexr=no)
  AC_MSG_RESULT([])
fi

if test "$have_openexr" = 'yes'; then
  AC_DEFINE(OPENEXR_DELEGATE,1,Define if you have OPENEXR library)
  CFLAGS="$OPENEXR_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(OPENEXR_DELEGATE, test "$have_openexr" = 'yes')
AC_SUBST(OPENEXR_CFLAGS)
AC_SUBST(OPENEXR_LIBS)

dnl ===========================================================================

#
# Check for PANGO delegate library.
#
AC_ARG_WITH([pango],
    [AC_HELP_STRING([--without-pango],
                    [disable PANGO support])],
    [with_pango=$withval],
    [with_pango='yes'])

if test "$with_pango" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-pango=$with_pango "
fi

have_pango='no'
have_pangocairo='no'
PANGO_CFLAGS=""
PANGO_LIBS=""
PANGO_PKG=""
if test "x$with_pango" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(PANGO, [pangocairo >= 1.28.1], have_pangocairo=yes, have_pangocairo=no)
  AC_MSG_RESULT([])
  PKG_CHECK_MODULES(PANGO, [pango >= 1.28.1], have_pango=yes, have_pango=no)
  AC_MSG_RESULT([])
fi

if test "$have_pango" = 'yes'; then
  AC_DEFINE(PANGO_DELEGATE,1,Define if you have PANGO library)
  CFLAGS="$PANGO_CFLAGS $CFLAGS"
fi

if test "$have_pangocairo" = 'yes'; then
  AC_DEFINE(PANGOCAIRO_DELEGATE,1,Define if you have PANGOCAIRO library)
  CFLAGS="$PANGOCAIRO_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(PANGO_DELEGATE, test "$have_pango" = 'yes')
AM_CONDITIONAL(PANGOCAIRO_DELEGATE, test "$have_pangocairo" = 'yes')
AC_SUBST(PANGO_CFLAGS)
AC_SUBST(PANGO_LIBS)

dnl ===========================================================================

#
# Check for PNG delegate library.
#
AC_ARG_WITH(png,
    [AC_HELP_STRING([--without-png],
                    [disable PNG support])],
    [with_png=$withval],
    [with_png='yes'])

if test "$with_png" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-png=$with_png "
fi

have_png='no'
PNG_CFLAGS=""
PNG_LIBS=""
PNG_PKG=""
if test "x$with_png" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(PNG,[libpng >= 1.0.0], have_png=yes, have_png=no)
  AC_MSG_RESULT([])
fi

if test "$have_png" = 'yes'; then
  AC_DEFINE(PNG_DELEGATE,1,Define if you have PNG library)
  CFLAGS="$PNG_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(PNG_DELEGATE, test "$have_png" = 'yes')
AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)

dnl ===========================================================================

#
# Check for the Raw delegate library.
#
AC_ARG_WITH([raw],
    [AC_HELP_STRING([--without-raw],
                    [disable Raw support])],
    [with_raw=$withval],
    [with_raw='yes'])

if test "$with_raw" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-libraw=$with_raw "
fi

have_raw='no'
RAW_R_CFLAGS=""
RAW_R_LIBS=""
RAW_R_PKG=""
if test "x$with_raw" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(RAW_R,[libraw_r >= 0.14.8], have_raw=yes, have_raw=no)
  AC_MSG_RESULT([])
fi

if test "$have_raw" = 'yes'; then
  AC_DEFINE(RAW_R_DELEGATE,1,Define if you have LIBRAW library)
  CFLAGS="$RAW_R_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(RAW_R_DELEGATE, test "$have_raw" = 'yes')
AC_SUBST(RAW_R_CFLAGS)
AC_SUBST(RAW_R_LIBS)

dnl ===========================================================================

#
# Check for RSVG delegate library.
#
AC_ARG_WITH([rsvg],
    [AC_HELP_STRING([--with-rsvg],
                    [enable RSVG support])],
    [with_rsvg=$withval],
    [with_rsvg='no'])

if test "$with_rsvg" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-rsvg=$with_rsvg "
fi

have_rsvg='no'
have_cairo='no'
RSVG_CFLAGS=""
RSVG_LIBS=""
RSVG_PKG=""
if test "x$with_rsvg" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(RSVG,[librsvg-2.0 >= 2.9.0], have_rsvg=yes, have_rsvg=no)
  AC_MSG_RESULT([])
  PKG_CHECK_MODULES(CAIRO_SVG, cairo-svg, have_cairo=yes, have_cairo=no)
  AC_MSG_RESULT([])
fi

if test "$have_rsvg" = 'yes'; then
  AC_DEFINE(RSVG_DELEGATE,1,Define if you have RSVG library)
  CFLAGS="$RSVG_CFLAGS $CFLAGS"
fi

if test "$have_cairo" = 'yes'; then
  AC_DEFINE(CAIRO_DELEGATE,1,Define if you have CAIRO library)
  CFLAGS="$CAIRO_SVG_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(RSVG_DELEGATE, test "$have_rsvg" = 'yes')
AM_CONDITIONAL(CAIRO_DELEGATE, test "$have_cairo" = 'yes')
AC_SUBST(RSVG_CFLAGS)
AC_SUBST(RSVG_LIBS)

dnl ===========================================================================

#
# Check for TIFF delegate library.
#
AC_ARG_WITH([tiff],
    [AC_HELP_STRING([--without-tiff],
                    [disable TIFF support])],
    [with_tiff=$withval],
    [with_tiff='yes'])

if test "$with_tiff" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-tiff=$with_tiff "
fi

have_tiff='no'
TIFF_LIBS=''
if test "$with_tiff" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for TIFF])
    AC_MSG_RESULT([])
    failed=0
    passed=0
    AC_CHECK_HEADER(tiff.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_HEADER(tiffio.h,passed=`expr $passed + 1`,failed=`expr $failed + 1`)
    AC_CHECK_LIB(tiff,TIFFOpen,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_CHECK_LIB(tiff,TIFFClientOpen,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_CHECK_LIB(tiff,TIFFIsByteSwapped,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_CHECK_LIB(tiff,TIFFReadRGBATile,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_CHECK_LIB(tiff,TIFFReadRGBAStrip,passed=`expr $passed + 1`,failed=`expr $failed + 1`,)
    AC_MSG_CHECKING([if TIFF package is complete])
    if test $passed -gt 0; then
        if test $failed -gt 0; then
            AC_MSG_RESULT([no -- some components failed test])
            have_tiff='no (failed tests)'
        else
            TIFF_LIBS='-ltiff'
            LIBS="$TIFF_LIBS $LIBS"
            AC_DEFINE(TIFF_DELEGATE,1,Define if you have TIFF library)
            AC_MSG_RESULT([yes])
            have_tiff='yes'
            AC_CHECK_HEADERS(tiffconf.h)
            AC_CHECK_FUNCS([TIFFIsCODECConfigured TIFFMergeFieldInfo \
              TIFFIsBigEndian TIFFReadEXIFDirectory TIFFSetErrorHandlerExt \
              TIFFSetTagExtender TIFFSetWarningHandlerExt \
              TIFFSwabArrayOfTriples])
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AM_CONDITIONAL(TIFF_DELEGATE, test "$have_tiff" = 'yes')
AC_SUBST(TIFF_LIBS)

#
# Set URW Base35 font directory.
#
AC_ARG_WITH([urw-base35-font-dir],
    [AC_HELP_STRING([--with-urw-base35-font-dir=DIR],
                    [URW-base35 font directory])],
    [with_urw_base35_font_dir=$withval],
    [with_urw_base35_font_dir='default'])

if test "$with_urw_base35_font_dir" != 'default'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-urw-base35-font-dir=$with_urw_base35_font_dir "
fi

dnl ===========================================================================

#
# Check for the WEBP delegate library.
#
AC_ARG_WITH([webp],
    [AC_HELP_STRING([--without-webp],
                    [disable WEBP support])],
    [with_webp=$withval],
    [with_webp='yes'])

if test "$with_webp" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-webp=$with_webp "
fi

have_webp='no'
have_webpmux='no'
WEBP_CFLAGS=""
WEBP_LIBS=""
WEBP_PKG=""
WEBPMUX_CFLAGS=""
WEBPMUX_LIBS=""
WEBPMUX_PKG=""
if test "x$with_webp" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(WEBP,[libwebp], have_webp=yes, have_webp=no)
  PKG_CHECK_MODULES(WEBPMUX,[libwebpmux >= 0.4.4], have_webpmux=yes, have_webpmux=no)
  AC_MSG_RESULT([])
fi

if test "$have_webp" = 'yes'; then
  AC_DEFINE(WEBP_DELEGATE,1,Define if you have WEBP library)
  CFLAGS="$WEBP_CFLAGS $CFLAGS"
fi

if test "$have_webpmux" = 'yes'; then
  AC_DEFINE(WEBPMUX_DELEGATE,1,Define if you have WEBPMUX library)
  CFLAGS="$WEBPMUX_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(WEBP_DELEGATE, test "$have_webp" = 'yes')
AM_CONDITIONAL(WEBPMUX_DELEGATE, test "$have_webpmux" = 'yes')
AC_SUBST(WEBPMUX_CFLAGS)
AC_SUBST(WEBPMUX_LIBS)
AC_SUBST(WEBP_CFLAGS)
AC_SUBST(WEBP_LIBS)

dnl ===========================================================================

#
# Set Windows font directory.
#
AC_ARG_WITH([windows-font-dir],
    [AC_HELP_STRING([--with-windows-font-dir=DIR],
                    [Windows font directory])],
    [with_windows_font_dir=$withval],
    [with_windows_font_dir='default'])

if test "$with_windows_font_dir" != 'default'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-windows-font-dir=$with_windows_font_dir "
fi

dnl ===========================================================================

#
# Check for WMF delegate library.
#
AC_ARG_WITH([wmf],
    [AC_HELP_STRING([--with-wmf],
                    [enable WMF support])],
    [with_wmf=$withval],
    [with_wmf='yes'])

if test "$with_wmf" != 'yes'; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-wmf=$with_wmf "
fi

have_wmf='no'
WMF_LIBS=''
WMF_CFLAGS=''
OLIBS="$LIBS"
if test "$with_wmf" != 'no'; then
  AC_MSG_CHECKING([for WMF support ])
  AC_MSG_RESULT([])

  have_libwmflite='no'
  have_libwmf_ipa_h='no'

  AC_CHECK_HEADER([libwmf/ipa.h],[have_libwmf_ipa_h='yes'],[],[$FT2BUILD_H])
  if test "$have_libwmf_ipa_h" = 'yes'; then
    AC_CHECK_LIB([wmflite],[wmf_lite_create],[have_libwmflite='yes'],[],[])
    if test "$have_libwmflite" = 'yes'; then
      AC_DEFINE(WMF_DELEGATE,1,Define if you have WMF library)
      WMF_LIBS='-lwmflite'
      LIBS="$WMF_LIBS $LIBS"
      have_wmf='yes'
    else
      AC_MSG_RESULT([no -- some components failed test])
      have_wmf='no (failed tests)'
      have_wmflite='no (failed tests)'
      LIBS="$OLIBS"
      WMF_LIBS=''
    fi
  fi
fi
AC_MSG_CHECKING([if WMF package is complete ])
if test "$have_wmf" = 'yes'; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([WMF_DELEGATE], [test "$have_wmf" = 'yes'])
AC_SUBST([WMF_CFLAGS])
AC_SUBST([WMF_LIBS])

dnl ===========================================================================

#
# Check for XML delegate library.
#
AC_ARG_WITH([xml],
    [AC_HELP_STRING([--without-xml],
                    [disable XML support])],
    [with_xml=$withval],
    [with_xml='yes'])

if test "$with_xml" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-xml=$with_xml "
fi

have_xml='no'
XML_CFLAGS=""
XML_LIBS=""
XML_PKG=""
if test "x$with_xml" = "xyes"; then
  AC_MSG_RESULT([-------------------------------------------------------------])
  PKG_CHECK_MODULES(XML,[libxml-2.0 >= 2.0.0], have_xml=yes, have_xml=no)
  AC_MSG_RESULT([])
fi

if test "$have_xml" = 'yes'; then
  AC_DEFINE(XML_DELEGATE,1,Define if you have XML library)
  CFLAGS="$XML_CFLAGS $CFLAGS"
fi

AM_CONDITIONAL(XML_DELEGATE, test "$have_xml" = 'yes')
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

dnl ===========================================================================

# Check for functions
#
AC_CHECK_FUNCS([acosh _aligned_malloc asinh atanh atoll atexit cabs carg cimag creal clock clock_getres clock_gettime ctime_r directio erf _exit execvp fchmod floor fork ftime ftruncate getc_unlocked getcwd getpid getexecname getdtablesize getpagesize getrlimit getrusage gettimeofday gmtime_r isnan j0 j1 lltostr localtime_r lstat memmove memset mkstemp munmap nanosleep newlocale _NSGetExecutablePath pclose _pclose poll popen _popen posix_fadvise posix_fallocate posix_madvise posix_memalign posix_spawnp pow pread pwrite qsort_r raise rand_r readlink realpath select seekdir sendfile setlocale socket sqrt setvbuf stat strchr strrchr strcspn strdup strpbrk strspn strstr strtod strtod_l strtol strtoul symlink sysconf sigemptyset sigaction spawnvp strerror strlcat strlcpy strcasecmp strncasecmp telldir tempnam times ulltostr uselocale usleep utime vfprintf vfprintf_l vsprintf vsnprintf vsnprintf_l waitpid _wfopen _wstat])

# Substitute compiler name to build/link PerlMagick
#
AC_SUBST([PERLMAINCC])

#
# Configure install Paths
#

# Path to ImageMagick header files
INCLUDE_RELATIVE_PATH="${PACKAGE_NAME}-${MAGICK_MAJOR_VERSION}"
INCLUDE_PATH="${INCLUDE_DIR}/${INCLUDE_RELATIVE_PATH}"
DEFINE_INCLUDE_PATH="${INCLUDE_DIR}/${INCLUDE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_INCLUDE_PATH=`$WinPathScript "$DEFINE_INCLUDE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(INCLUDE_PATH,"$DEFINE_INCLUDE_PATH",[Directory where ImageMagick headers live.])
AC_SUBST(INCLUDE_PATH)

# Path to ImageMagick header files (arch part)
INCLUDEARCH_PATH="${INCLUDEARCH_DIR}/${INCLUDE_RELATIVE_PATH}"
DEFINE_INCLUDEARCH_PATH="${INCLUDEARCH_DIR}/${INCLUDE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_INCLUDEARCH_PATH=`$WinPathScript "$DEFINE_INCLUDEARCH_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(INCLUDEARCH_PATH,"$DEFINE_INCLUDE_PATH",[Directory where ImageMagick architecture headers live.])
AC_SUBST(INCLUDEARCH_PATH)

# Subdirectory under lib to place ImageMagick lib files
LIBRARY_RELATIVE_PATH="${PACKAGE_NAME}-${PACKAGE_VERSION}"
AC_DEFINE_UNQUOTED(LIBRARY_RELATIVE_PATH,"$LIBRARY_RELATIVE_PATH",[Subdirectory of lib where ImageMagick architecture dependent files are installed.])

# Path to ImageMagick bin directory
EXECUTABLE_PATH="${BIN_DIR}"
DEFINE_EXECUTABLE_PATH="${BIN_DIR}/"
case "${build_os}" in
  mingw* )
    DEFINE_EXECUTABLE_PATH=`$WinPathScript "$DEFINE_EXECUTABLE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(EXECUTABLE_PATH,"$DEFINE_EXECUTABLE_PATH",[Directory where executables are installed.])
AC_SUBST(EXECUTABLE_PATH)

# Path to ImageMagick lib
LIBRARY_PATH="${LIB_DIR}/${LIBRARY_RELATIVE_PATH}"
DEFINE_LIBRARY_PATH="${LIB_DIR}/${LIBRARY_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_LIBRARY_PATH=`$WinPathScript "$DEFINE_LIBRARY_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(LIBRARY_PATH,"$DEFINE_LIBRARY_PATH",[Directory where architecture-dependent files live.])
AC_SUBST(LIBRARY_PATH)

#
# path to local binaries
if test "x$LIB_BIN_BASEDIRNAME" = "x"; then
   LIB_BIN_BASEDIRNAME="bin"
fi
AC_DEFINE_UNQUOTED(LIB_BIN_BASEDIRNAME,"$LIB_BIN_BASEDIRNAME",[Binaries in libraries path base name (will be during install linked to bin)])
AC_SUBST(LIB_BIN_BASEDIRNAME)

if test "x$LIB_BIN_DIR" = "x"; then
   LIB_BIN_DIR=${LIB_BIN_BASEDIRNAME}-${MAGICK_ABI_SUFFIX}
fi
AC_SUBST(LIB_BIN_DIR)

LIB_BIN_DIR_RELATIVE_PATH=$LIB_BIN_DIR
AC_SUBST(LIB_BIN_DIR_RELATIVE_PATH)
LIB_BIN_DIR_PATH="${LIBRARY_PATH}/${LIB_BIN_DIR_RELATIVE_PATH}"
AC_SUBST(LIB_BIN_DIR_PATH)

#
# path to modules lib
if test "x$MODULES_BASEDIRNAME" = "x"; then
   MODULES_BASEDIRNAME="modules"
fi
AC_DEFINE_UNQUOTED(MODULES_BASEDIRNAME,"$MODULES_BASEDIRNAME",[Module directory name without ABI part.])
AC_SUBST(MODULES_BASEDIRNAME)

if test "x$MODULES_DIRNAME" = "x"; then
  MODULES_DIRNAME=${MODULES_BASEDIRNAME}-${MAGICK_ABI_SUFFIX}
else
  AC_DEFINE_UNQUOTED(MODULES_DIRNAME,"$MODULES_DIRNAME",[Module directory dirname])
fi
AC_SUBST(MODULES_DIRNAME)

MODULES_RELATIVE_PATH="${MODULES_DIRNAME}"
AC_SUBST(MODULES_RELATIVE_PATH)
MODULES_PATH="${LIBRARY_PATH}/${MODULES_RELATIVE_PATH}"
AC_SUBST(MODULES_PATH)

#
# path to coders lib
if test "x$CODER_DIRNAME" = "x"; then
   CODER_DIRNAME="coders"
fi
AC_DEFINE_UNQUOTED(CODER_DIRNAME,"$CODER_DIRNAME",[coders subdirectory.])
AC_SUBST(CODER_DIRNAME)

CODER_RELATIVE_PATH="${CODER_DIRNAME}"
CODER_PATH="${MODULES_PATH}/${CODER_DIRNAME}"
AC_SUBST(CODER_PATH)

#
# Subdirectory under lib to place ImageMagick filter module files
#
# path to coders lib
if test "x$FILTER_DIRNAME" = "x"; then
   FILTER_DIRNAME="filters"
fi
AC_DEFINE_UNQUOTED(FILTER_DIRNAME,"$FILTER_DIRNAME",[filter subdirectory.])
AC_SUBST(FILTER_DIRNAME)

FILTER_RELATIVE_PATH="${FILTER_DIRNAME}"
FILTER_PATH="${MODULES_PATH}/${FILTER_DIRNAME}"
AC_SUBST(FILTER_PATH)

#
# Path to ImageMagick documentation files
DOCUMENTATION_RELATIVE_PATH="${PACKAGE_NAME}-${MAGICK_MAJOR_VERSION}"
DOCUMENTATION_PATH="${DOC_DIR}/${DOCUMENTATION_RELATIVE_PATH}"
DEFINE_DOCUMENTATION_PATH="${DOC_DIR}/${DOCUMENTATION_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_DOCUMENTATION_PATH=`$WinPathScript "$DEFINE_DOCUMENTATION_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(DOCUMENTATION_PATH,"$DEFINE_DOCUMENTATION_PATH",[Directory where ImageMagick documents live.])
AC_SUBST(DOCUMENTATION_PATH)

#
# Enable/Disable documentation
AC_ARG_ENABLE([docs],
  [AS_HELP_STRING([--disable-docs],
    [disable building of documentation])],
  wantdocs=$enableval, wantdocs=yes)
AM_CONDITIONAL([INSTALL_DOC], [test "$wantdocs" = "yes"])

# Subdirectory to place architecture-dependent configuration files
CONFIGURE_RELATIVE_PATH="${PACKAGE_NAME}-${MAGICK_MAJOR_VERSION}"
AC_DEFINE_UNQUOTED(CONFIGURE_RELATIVE_PATH,"$CONFIGURE_RELATIVE_PATH",[Subdirectory of lib where architecture-dependent configuration files live.])
CONFIGURE_PATH="${SYSCONF_DIR}/${CONFIGURE_RELATIVE_PATH}/"
DEFINE_CONFIGURE_PATH="${SYSCONF_DIR}/${CONFIGURE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_CONFIGURE_PATH=`$WinPathScript "$DEFINE_CONFIGURE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(CONFIGURE_PATH,"$DEFINE_CONFIGURE_PATH",[Directory where architecture-dependent configuration files live.])
AC_SUBST(CONFIGURE_PATH)

# Subdirectory to place architecture-independent configuration files
SHARE_RELATIVE_PATH="${PACKAGE_NAME}-${MAGICK_MAJOR_VERSION}"
AC_DEFINE_UNQUOTED(SHARE_RELATIVE_PATH,"$SHARE_RELATIVE_PATH",[Subdirectory of lib where architecture-independent configuration files live.])
SHARE_PATH="${DATA_DIR}/${SHARE_RELATIVE_PATH}"
DEFINE_SHARE_PATH="${DATA_DIR}/${SHARE_RELATIVE_PATH}/"
case "${build_os}" in
  mingw* )
    DEFINE_SHARE_PATH=`$WinPathScript "$DEFINE_SHARE_PATH" 1`
    ;;
esac
AC_DEFINE_UNQUOTED(SHARE_PATH,"$DEFINE_SHARE_PATH",[Directory where architecture-independent configuration files live.])
AC_SUBST(SHARE_PATH)

#
# Subdirectory to place architecture-dependent configuration files.
if test "x$SHAREARCH_BASEDIRNAME" = "x"; then
   SHAREARCH_BASEDIRNAME="config"
fi
AC_DEFINE_UNQUOTED(SHAREARCH_BASEDIRNAME,"$SHAREARCH_BASEDIRNAME",[Sharearch directory name without ABI part.])
AC_SUBST(SHAREARCH_BASEDIRNAME)

if test "x$SHAREARCH_DIRNAME" = "x"; then
  SHAREARCH_DIRNAME=${SHAREARCH_BASEDIRNAME}-${MAGICK_ABI_SUFFIX}
else
  AC_DEFINE_UNQUOTED(SHAREARCH_DIRNAME,"$SHAREARCH_DIRNAME",[Sharearch directory dirname])
fi
AC_SUBST(SHAREARCH_DIRNAME)

SHAREARCH_RELATIVE_PATH="${SHAREARCH_DIRNAME}"
AC_SUBST(SHAREARCH_RELATIVE_PATH)
SHAREARCH_PATH="${LIBRARY_PATH}/${SHAREARCH_RELATIVE_PATH}"
AC_SUBST(SHAREARCH_PATH)

#
# program_transform_name is formed for use in a Makefile, so create a
# modified version for use in a shell script.
configure_transform_name=`echo ${program_transform_name} | sed 's,\\$\\$,$,'`

# Default delegate definitions
AC_MSG_RESULT([-------------------------------------------------------------])
AC_MSG_CHECKING([for ImageMagick delegate programs])
AC_MSG_RESULT([])
BPGDecodeDelegateDefault='bpgdec'
BPGEncodeDelegateDefault='bpgenc'
BlenderDecodeDelegateDefault='blender'
BrowseDelegateDefault='xdg-open'
DNGDecodeDelegateDefault='ufraw-batch'
DOCDecodeDelegateDefault='soffice'
GVCDecodeDelegateDefault='dot'
DVIDecodeDelegateDefault='dvips'
EditorDelegateDefault='xterm'
ConvertDelegateDefault=`echo magick | sed ${configure_transform_name}`
DisplayDelegateDefault=`echo magick | sed ${configure_transform_name}`
MogrifyDelegateDefault=`echo magick | sed ${configure_transform_name}`
HPGLDecodeDelegateDefault='hp2xx'
HTMLDecodeDelegateDefault='html2ps'
ILBMDecodeDelegateDefault='ilbmtoppm'
ILBMEncodeDelegateDefault='ppmtoilbm'
JXRDecodeDelegateDefault='JxrDecApp'
JXREncodeDelegateDefault='JxrEncApp'
LEPDelegateDefault='lepton'
LPDelegateDefault='lp'
LPRDelegateDefault='lpr'
LaunchDelegateDefault='gimp'
MPEGDecodeDelegateDefault='avconv'
MPEGEncodeDelegateDefault='avconv'
MrSIDDecodeDelegateDefault='mrsidgeodecode'
MVDelegateDefault='mv'
PCLDelegateDefault='pcl6'
if test "$native_win32_build" = 'yes'; then
    PSDelegateDefault='gswin32c'
elif test "$gslib_framework" = 'yes'; then
    PSDelegateDefault='gsc'
else
    PSDelegateDefault='gs'
fi
RMDelegateDefault='rm'
RSVGDecodeDelegateDefault='rsvg-convert'
SVGDecodeDelegateDefault='inkscape'
UniconvertorDelegateDefault='uniconvertor'
WebPDecodeDelegateDefault='dwebp'
WebPEncodeDelegateDefault='cwebp'
WWWDecodeDelegateDefault='curl'
XPSDelegateDefault='gxps'

# Search for delegates
AC_PATH_PROG(BPGDecodeDelegate, "$BPGDecodeDelegateDefault", "$BPGDecodeDelegateDefault")
AC_PATH_PROG(BPGEncodeDelegate, "$BPGEncodeDelegateDefault", "$BPGEncodeDelegateDefault")
AC_PATH_PROG(BlenderDecodeDelegate, "$BlenderDecodeDelegateDefault", "$BlenderDecodeDelegateDefault")
AC_PATH_PROGS(BrowseDelegate, "$BrowseDelegateDefault" google-chrome firefox konqueror mozilla lynx, "$BrowseDelegateDefault")
AC_PATH_PROG(DNGDecodeDelegate, "$DNGDecodeDelegateDefault", "$DNGDecodeDelegateDefault")
AC_PATH_PROG(DOCDecodeDelegate, "$DOCDecodeDelegateDefault", "$DOCDecodeDelegateDefault")
AC_PATH_PROG(DVIDecodeDelegate, "$DVIDecodeDelegateDefault", "$DVIDecodeDelegateDefault")
AC_PATH_PROG(ConvertDelegate, "$ConvertDelegateDefault", "$ConvertDelegateDefault")
AC_PATH_PROG(DisplayDelegate, "$DisplayDelegateDefault", "$DisplayDelegateDefault")
AC_PATH_PROG(EditorDelegate, "$EditorDelegateDefault", "$EditorDelegateDefault")
AC_PATH_PROG(GVCDecodeDelegate, "$GVCDecodeDelegateDefault", "$GVCDecodeDelegateDefault")
AC_PATH_PROG(HPGLDecodeDelegate, "$HPGLDecodeDelegateDefault", "$HPGLDecodeDelegateDefault")
AC_PATH_PROG(HTMLDecodeDelegate, "$HTMLDecodeDelegateDefault", "$HTMLDecodeDelegateDefault")
AC_PATH_PROG(ILBMDecodeDelegate, "$ILBMDecodeDelegateDefault", "$ILBMDecodeDelegateDefault")
AC_PATH_PROG(ILBMEncodeDelegate, "$ILBMEncodeDelegateDefault", "$ILBMEncodeDelegateDefault")
AC_PATH_PROG(JXRDecodeDelegate, "$JXRDecodeDelegateDefault", "$JXRDecodeDelegateDefault")
AC_PATH_PROG(JXREncodeDelegate, "$JXREncodeDelegateDefault", "$JXREncodeDelegateDefault")
AC_PATH_PROG(LEPDelegate, "$LEPDelegateDefault", "$LEPDelegateDefault")
AC_PATH_PROG(LPDelegate, "$LPDelegateDefault", no)
AC_PATH_PROG(LPRDelegate, "$LPRDelegateDefault", "$LPRDelegateDefault")
AC_PATH_PROG(LaunchDelegate, "$LaunchDelegateDefault", "$LaunchDelegateDefault")
AC_PATH_PROG(MogrifyDelegate, "$MogrifyDelegateDefault", "$MogrifyDelegateDefault")
AC_PATH_PROG(MPEGDecodeDelegate, "$MPEGDecodeDelegateDefault", "$MPEGDecodeDelegateDefault")
if test "$MPEGDecodeDelegate" = "$MPEGDecodeDelegateDefault" ; then
  MPEGDecodeDelegateDefault='ffmpeg'
  AC_PATH_PROG(MPEGDecodeDelegate, "$MPEGDecodeDelegateDefault", "$MPEGDecodeDelegateDefault")
fi;
AC_PATH_PROG(MPEGEncodeDelegate, "$MPEGEncodeDelegateDefault", "$MPEGEncodeDelegateDefault")
if test "$MPEGEncodeDelegate" = "$MPEGEncodeDelegateDefault" ; then
  MPEGEncodeDelegateDefault='ffmpeg'
  AC_PATH_PROG(MPEGEncodeDelegate, "$MPEGEncodeDelegateDefault", "$MPEGEncodeDelegateDefault")
fi;
AC_PATH_PROG(MrSIDDecodeDelegate, "$MrSIDDecodeDelegateDefault", "$MrSIDDecodeDelegateDefault")
AC_PATH_PROG(MVDelegate, "$MVDelegateDefault", "$MVDelegateDefault")
AC_PATH_PROG(PCLDelegate, "$PCLDelegateDefault", "$PCLDelegateDefault")
AC_PATH_PROGS(PSDelegate, gsx gsc "$PSDelegateDefault", "$PSDelegateDefault")
AC_PATH_PROG(RMDelegate, "$RMDelegateDefault", "$RMDelegateDefault")
AC_PATH_PROG(RSVGDecodeDelegate, "$RSVGDecodeDelegateDefault", "$RSVGDecodeDelegateDefault")
AC_PATH_PROG(SVGDecodeDelegate, "$SVGDecodeDelegateDefault", "$SVGDecodeDelegateDefault")
AC_PATH_PROG(UniconvertorDelegate, "$UniconvertorDelegateDefault", "$UniconvertorDelegateDefault")
AC_PATH_PROG(WebPDecodeDelegate, "$WebPDecodeDelegateDefault", "$WebPDecodeDelegateDefault")
AC_PATH_PROG(WebPEncodeDelegate, "$WebPEncodeDelegateDefault", "$WebPEncodeDelegateDefault")
AC_PATH_PROG(WWWDecodeDelegate, "$WWWDecodeDelegateDefault", "$WWWDecodeDelegateDelegateDefault")
AC_PATH_PROG(XPSDelegate, "$XPSDelegateDefault", "$XPSDelegateDefault")

# Prefer lpr to lp; lp needs options tacked on.
if test "$LPRDelegate" != no; then
    PrintDelegate="$LPRDelegate"
else
    PrintDelegate="$LPDelegate -c -s"
fi
AC_SUBST(PrintDelegate)

# Installed ImageMagick utiltity paths
ConvertDelegate="${BIN_DIR}/${ConvertDelegateDefault}"
DisplayDelegate="${BIN_DIR}/${DisplayDelegateDefault}"
MogrifyDelegate="${BIN_DIR}/${MogrifyDelegateDefault}"

# Set delegate booleans
have_avconv='no'; if test "$MPEGDecodeDelegate" != "$MPEGDecodeDelegateDefault" ; then have_avconv='yes'; fi
have_gs='no'         ; if test "$PSDelegate" != "$PSDelegateDefault"; then have_gs='yes'; fi
have_hp2xx='no'      ; if test "$HPGLDecodeDelegate" !=  "$HPGLDecodeDelegateDefault" ; then have_hp2xx='yes'; fi
have_ilbmtoppm='no'  ; if test "$ILBMDecodeDelegate" != "$ILBMDecodeDelegateDefault" ; then have_ilbmtoppm='yes'; fi
have_mrsid='no'; if test "$MrSIDDecodeDelegate" != "$MrSIDDecodeDelegateDefault" ; then have_mrsid='yes'; fi
have_pcl='no'        ; if test "$PCLDelegate" != "$PCLDelegateDefault"; then have_pcl='yes'; fi
have_ppmtoilbm='no'  ; if test "$ILBMEncodeDelegate" != "$ILBMEncodeDelegateDefault" ; then have_ppmtoilbm='yes'; fi
have_xps='no'        ; if test "$XPSDelegate" != "$XPSDelegateDefault"; then have_xps='yes'; fi

#
# Test for font directories
#
type_include_files=''

# Apple fonts.
AC_MSG_CHECKING(for Apple fonts directory)
apple_font_dir=''
if test "${with_apple_font_dir}" != 'default'; then
  apple_font_dir="${with_apple_font_dir}/"
else
  for font_dir in '/Library/Fonts/'; do
    if test -f "${font_dir}Arial.ttf"; then
      apple_font_dir="${font_dir}"
      break 1
    fi
  done
fi
if test "${apple_font_dir}x" != 'x'; then
  type_include_files="${type_include_files} "'<include file="type-apple.xml" />'
  AC_MSG_RESULT([$apple_font_dir])
else
  AC_MSG_RESULT([not found!]);
fi
AC_SUBST(apple_font_dir)

# Dejavu fonts.
AC_MSG_CHECKING(for Dejavu fonts directory)
dejavu_font_dir=''
if test "${with_dejavu_font_dir}" != 'default'; then
  dejavu_font_dir="${with_dejavu_font_dir}/"
else
  for font_dir in "${prefix}/share/dejavu/fonts/" '/usr/share/fonts/dejavu/'; do
    if test -f "${font_dir}DejaVuSerif.ttf"; then
      dejavu_font_dir="${font_dir}"
      break 1
    fi
  done
fi
if test "${dejavu_font_dir}x" != 'x'; then
  type_include_files="${type_include_files} "'<include file="type-dejavu.xml" />'
  AC_MSG_RESULT([$dejavu_font_dir])
else
  AC_MSG_RESULT([not found!]);
fi
AC_SUBST(dejavu_font_dir)

# Ghostscript
AC_MSG_CHECKING(for Ghostscript fonts directory)
ghostscript_font_dir=''
if test "${with_gs_font_dir}" != 'default'; then
  ghostscript_font_dir="${with_gs_font_dir}/"
else
  if test "${native_win32_build}" = 'yes'; then
    # Native Windows Build
    for font_dir in "c:\\Program Files\\gs\\fonts\\" "c:\\Program Files \(x86\)\\gs\\fonts\\" "c:\\gs\\fonts\\"; do
      if test -f "${font_dir}a010013l.pfb"; then
        ghostscript_font_dir="$font_dir"
        break 1
      fi
    done
    if test "${PSDelegate}" != 'gswin32c'; then
      ghostscript_font_dir=`echo "${PSDelegate}" | sed -e 's:/gs/.*:/gs:;s:^/::;s/./&:/;s:/:\\\\:g'`"\\fonts\\"
    fi
  else
    # Linux / Mac OS X / Unix Build
    for font_dir in "${prefix}/share/ghostscript/fonts/" '/usr/share/fonts/default/Type1/' '/usr/share/ghostscript/fonts/' '/usr/share/fonts/ghostscript/' '/usr/share/fonts/type1/gsfonts/' '/opt/local/share/ghostscript/fonts/' '/sw/share/ghostscript/fonts/' '/System/Library/Frameworks/Ghostscript.framework/Resources/fonts/'; do
      if test -f "${font_dir}a010013l.pfb"; then
        ghostscript_font_dir="${font_dir}"
        break 1
      fi
    done
    if test "${ghostscript_font_dir}x" = 'x'; then
      if test "$PSDelegate" != 'gs'; then
        ghostscript_font_dir=`echo "$PSDelegate" | sed -e 's:/bin/gs:/share/ghostscript/fonts:'`"/"
      fi
    fi
  fi
fi
if test "${ghostscript_font_dir}x" != 'x'; then
  type_include_files="${type_include_files} "'<include file="type-ghostscript.xml" />'
  AC_MSG_RESULT([$ghostscript_font_dir])
else
  AC_MSG_RESULT([not found!]);
fi
AC_SUBST(ghostscript_font_dir)
case "${build_os}" in
  mingw* )
    PSDelegate=`$WinPathScript "$PSDelegate" 1`
    ;;
esac

# URW-base35 fonts.
AC_MSG_CHECKING(for URW-base35 fonts directory)
urw_base35_font_dir=''
if test "${with_urw_base35_font_dir}" != 'default'; then
  urw_base35_font_dir="${with_urw_base35_font_dir}/"
else
  for font_dir in "${prefix}/share/urw-base35/fonts/" '/usr/share/fonts/urw-base35/'; do
    if test -f "${font_dir}StandardSymbolsPS.t1"; then
      urw_base35_font_dir="${font_dir}"
      break 1
    fi
  done
fi
if test "${urw_base35_font_dir}x" != 'x'; then
  type_include_files="${type_include_files} "'<include file="type-urw-base35.xml" />'
  AC_MSG_RESULT([$urw_base35_font_dir])
else
  AC_MSG_RESULT([not found!]);
fi
AC_SUBST(urw_base35_font_dir)

# Windows fonts.
AC_MSG_CHECKING(for Windows fonts directory)
windows_font_dir=''
if test "${with_windows_font_dir}" != 'default'; then
  windows_font_dir="${with_windows_font_dir}/"
else
  for font_dir in '/usr/X11R6/lib/X11/fonts/truetype/' '/usr/X11R7/lib/X11/fonts/truetype/' '/usr/share/fonts/msttcore/' '/usr/share/fonts/microsoft/' '/usr/share/fonts/truetype/msttcorefonts/'; do
    if test -f "${font_dir}arial.ttf"; then
      windows_font_dir="${font_dir}"
      break 1
    fi
  done
fi
if test "${windows_font_dir}x" != 'x'; then
  type_include_files="${type_include_files} "'<include file="type-windows.xml" />'
  AC_MSG_RESULT([$windows_font_dir])
else
  AC_MSG_RESULT([not found!]);
fi
AC_SUBST(windows_font_dir)

AC_SUBST(type_include_files)

#
# Handle case where user doesn't want frozen paths
#
if test "$with_frozenpaths" != 'yes'; then
  # Re-set delegate definitions to default (no paths)
  BPGDecodeDelegate="$BPGDecodeDelegateDefault"
  BPGEncodeDelegate="$BPGEncodeDelegateDefault"
  BlenderDecodeDelegate="$BlenderDecodeDelegateDefault"
  BrowseDelegate="$BrowseDelegateDefault"
  ConvertDelegate="$ConvertDelegateDefault"
  DisplayDelegate="$DisplayDelegateDefault"
  DNGDecodeDelegate="$DNGDecodeDelegateDefault"
  DOCDecodeDelegate="$DOCDecodeDelegateDefault"
  DVIDecodeDelegate="$DVIDecodeDelegateDefault"
  EditorDelegate="$EditorDelegateDefault"
  GVCDecodeDelegate="$GVCDecodeDelegateDefault"
  HPGLDecodeDelegate="$HPGLDecodeDelegateDefault"
  HTMLDecodeDelegate="$HTMLDecodeDelegateDefault"
  ILBMDecodeDelegate="$ILBMDecodeDelegateDefault"
  ILBMEncodeDelegate="$ILBMEncodeDelegateDefault"
  JXRDecodeDelegate="$JXRDecodeDelegateDefault"
  JXREncodeDelegate="$JXREncodeDelegateDefault"
  LEPDelegate="$LEPDelegateDefault"
  LPDelegate="$LPDelegateDefault"
  LaunchDelegate="$LaunchDelegateDefault"
  MPEGDecodeDelegate="$MPEGDecodeDelegateDefault"
  MPEGEncodeDelegate="$MPEGEncodeDelegateDefault"
  MogrifyDelegate="$MogrifyDelegateDefault"
  MrSIDDecodeDelegate="$MrSIDDecodeDelegateDefault"
  PCLDelegate="$PCLDelegateDefault"
  PSDelegate="$PSDelegateDefault"
  RSVGDecodeDelegate="$RSVGDecodeDelegateDefault"
  SVGDecodeDelegate="$SVGDecodeDelegateDefault"
  ShowImageDelegate="$ShowImageDelegateDefault"
  UniconvertorDelegate="$UniconvertorDelegateDefault"
  WebPDecodeDelegate="$WebPDecodeDelegateDefault"
  WebPEncodeDelegate="$WebPEncodeDelegateDefault"
  WWWDecodeDelegate="$WWWDecodeDelegateDefault"
  XPSDelegate="$XPSDelegateDefault"
fi

# Delegate substitutions
AC_SUBST(BPGDecodeDelegate)
AC_SUBST(BPGEncodeDelegate)
AC_SUBST(BlenderDecodeDelegate)
AC_SUBST(BrowseDelegate)
AC_SUBST(ConvertDelegate)
AC_SUBST(GVCDecodeDelegate)
AC_SUBST(DVIDecodeDelegate)
AC_SUBST(EditorDelegate)
AC_SUBST(HPGLDecodeDelegate)
AC_SUBST(HTMLDecodeDelegate)
AC_SUBST(ILBMDecodeDelegate)
AC_SUBST(ILBMEncodeDelegate)
AC_SUBST(JXRDecodeDelegate)
AC_SUBST(JXREncodeDelegate)
AC_SUBST(LEPDelegate)
AC_SUBST(LPDelegate)
AC_SUBST(LaunchDelegate)
AC_SUBST(MPEGDecodeDelegate)
AC_SUBST(MPEGEncodeDelegate)
AC_SUBST(MogrifyDelegate)
AC_SUBST(MrSIDDecodeDelegate)
AC_SUBST(PCLDelegate)
AC_SUBST(PSDelegate)
AC_SUBST(ShowImageDelegate)
AC_SUBST(UniconvertorDelegate)
AC_SUBST(WebPDecodeDelegate)
AC_SUBST(WebPEncodeDelegate)
AC_SUBST(WWWDecodeDelegate)
AC_SUBST(XPSDelegate)

#
# RPM support.
#
RPM=''
AC_CHECK_PROGS(TAR,gnutar gtar tar)
AC_CHECK_PROGS(PERL,perl)
AC_CHECK_PROGS(RPM,rpmbuild rpm)
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AC_SUBST(RPM)
AM_CONDITIONAL(RPM_DELEGATE, test "x$RPM" != "x" )

#
# 7ZIP support (http://p7zip.sourceforge.net/)
#
P7ZIP=''
AC_CHECK_PROGS(P7ZIP,[7za])
AC_SUBST(P7ZIP)
AM_CONDITIONAL(P7ZIP_DELEGATE, test "x$P7ZIP" != "x" )

#
# ZIP support (http://www.info-zip.org/Zip.html)
#
ZIP=''
AC_CHECK_PROGS(ZIP,[zip])
AC_SUBST(ZIP)
AM_CONDITIONAL(ZIP_DELEGATE, test "x$ZIP" != "x" )

#
# GhostPCL related configuration.
#
PCLColorDevice=ppmraw
PCLCMYKDevice=pamcmyk32
PCLMonoDevice=pbmraw
if test -z "$PCLVersion"; then
    PCLVersion='unknown'
fi
if test $have_pcl = 'yes'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for PCL])
    AC_MSG_RESULT([])
    # PCLColorDevice
    AC_MSG_CHECKING([for pcl color device])
    if $PCLDelegate -dBATCH -sDEVICE=$PCLColorDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        PCLColorDevice=ppmraw
    fi
    AC_MSG_RESULT([$PCLColorDevice])

    # PCLCMYKDevice
    AC_MSG_CHECKING([for pcl CMYK device])
    if $PCLDelegate -dBATCH -sDEVICE=$PCLColorDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        PCLCMYKDevice=$PCLColorDevice
    fi
    AC_MSG_RESULT([$PCLCMYKDevice])

    # PCLMonoDevice
    AC_MSG_CHECKING([for pcl mono device])
    if $PCLDelegate -dBATCH -sDEVICE=$PCLMonoDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        PCLMonoDevice=$PCLColorDevice
    fi
    AC_MSG_RESULT([$PCLMonoDevice])
fi

AC_SUBST(PCLMonoDevice)
AC_SUBST(PCLColorDevice)
AC_SUBST(PCLCMYKDevice)
AC_SUBST(PCLVersion)

#
# GhostXPS related configuration.
#
XPSColorDevice=ppmraw
XPSCMYKDevice=bmpsep8
XPSMonoDevice=pbmraw
if test -z "$XPSVersion"; then
    XPSVersion='unknown'
fi
if test $have_xps = 'yes'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for XPS])
    AC_MSG_RESULT([])
    # XPSColorDevice
    AC_MSG_CHECKING([for xps color device])
    if $XPSDelegate -dBATCH -sDEVICE=$XPSColorDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        XPSColorDevice=ppmraw
    fi
    AC_MSG_RESULT([$XPSColorDevice])

    # XPSCMYKDevice
    AC_MSG_CHECKING([for xps CMYK device])
    if $XPSDelegate -dBATCH -sDEVICE=$XPSColorDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        XPSCMYKDevice=$XPSColorDevice
    fi
    AC_MSG_RESULT([$XPSCMYKDevice])

    # XPSMonoDevice
    AC_MSG_CHECKING([for xps mono device])
    if $XPSDelegate -dBATCH -sDEVICE=$XPSMonoDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        XPSMonoDevice=$XPSColorDevice
    fi
    AC_MSG_RESULT([$XPSMonoDevice])
fi

AC_SUBST(XPSMonoDevice)
AC_SUBST(XPSColorDevice)
AC_SUBST(XPSCMYKDevice)
AC_SUBST(XPSVersion)

#
# Ghostscript related configuration.
#
GSAlphaDevice=pngalpha
GSColorDevice=pnmraw
GSCMYKDevice=pamcmyk32
GSMonoDevice=pbmraw
GSPDFDevice=pdfwrite
GSPSDevice=ps2write
GSEPSDevice=eps2write
GSVersion='unknown'
if test $have_gs = 'yes'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for Ghostscript])
    AC_MSG_RESULT([])
    AC_MSG_CHECKING([for Ghostscript version])
    if GSVersion=`$PSDelegate --version`; then
        :
    else
        GSVersion=`$PSDelegate --help | sed -e '1q' | awk '{ print $3 }'`
    fi
    AC_MSG_RESULT([$GSVersion])

    # GSColorDevice
    AC_MSG_CHECKING([for gs color device])
    if $PSDelegate -q -dBATCH -sDEVICE=$GSColorDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        GSColorDevice=ppmraw
    fi
    AC_MSG_RESULT([$GSColorDevice])

    # GSAlphaDevice
    AC_MSG_CHECKING([for gs alpha device])
    if $PSDelegate -q -dBATCH -sDEVICE=$GSAlphaDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        GSAlphaDevice=$GSColorDevice
    fi
    AC_MSG_RESULT([$GSAlphaDevice])

    # GSCMYKDevice
    AC_MSG_CHECKING([for gs CMYK device])
    if $PSDelegate -q -dBATCH -sDEVICE=$GSCMYKDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        GSCMYKDevice=pam
    fi
    AC_MSG_RESULT([$GSCMYKDevice])

    # GSMonoDevice
    AC_MSG_CHECKING([for gs mono device])
    if $PSDelegate -q -dBATCH -sDEVICE=$GSMonoDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        GSMonoDevice=$GSColorDevice
    fi
    AC_MSG_RESULT([$GSMonoDevice])

    # GSPDFDevice
    AC_MSG_CHECKING([for gs PDF writing device])
    if $PSDelegate -q -dBATCH -sDEVICE=$GSPDFDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        GSPDFDevice=nodevice
    fi
    AC_MSG_RESULT([$GSPDFDevice])

    # GSPSDevice
    AC_MSG_CHECKING([for gs PS writing device])
    if $PSDelegate -q -dBATCH -sDEVICE=$GSPSDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        GSPSDevice=pswrite
    fi
    AC_MSG_RESULT([$GSPSDevice])

    # GSEPSDevice
    AC_MSG_CHECKING([for gs EPS writing device])
    if $PSDelegate -q -dBATCH -sDEVICE=$GSEPSDevice -sOutputFile=/dev/null < /dev/null 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD; then
        :
    else
        GSEPSDevice=epswrite
    fi
    AC_MSG_RESULT([$GSEPSDevice])
fi

AC_SUBST(GSAlphaDevice)
AC_SUBST(GSCMYKDevice)
AC_SUBST(GSColorDevice)
AC_SUBST(GSEPSDevice)
AC_SUBST(GSMonoDevice)
AC_SUBST(GSPDFDevice)
AC_SUBST(GSPSDevice)
AC_SUBST(GSVersion)

#
# PerlMagick-related configuration
#

# Look for PERL if PerlMagick requested
# If name/path of desired PERL interpreter is specified, look for that one first
have_perl='no'
if test "$with_perl" != 'no'; then
    AC_MSG_RESULT([-------------------------------------------------------------])
    AC_MSG_CHECKING([for Perl])
    AC_MSG_RESULT([])
    if test "$with_perl" != 'yes'; then
        AC_CACHE_CHECK(for perl,ac_cv_path_PERL,ac_cv_path_PERL="$with_perl");
        PERL=$ac_cv_path_PERL
        AC_SUBST(PERL)dnl
        have_perl="$ac_cv_path_PERL"
    else
        AC_PATH_PROGS(PERL,perl perl5,)dnl
        if test "$ac_cv_path_PERL"; then
            have_perl="$ac_cv_path_PERL"
        fi
    fi
fi

if test "$with_perl" != 'yes' ; then
    DISTCHECK_CONFIG_FLAGS="${DISTCHECK_CONFIG_FLAGS} --with-perl=$with_perl "
fi

PERL_SUPPORTS_DESTDIR='no'

with_perl_static='no'
with_perl_dynamic='no'
if test "$have_perl" != 'no'; then
    if test "$with_perl" != 'no' && test "$libtool_build_shared_libs" = 'no'; then
        with_perl_static='yes'
    fi
    if test "$with_perl" != 'no' && test "$libtool_build_shared_libs" = 'yes'; then
        with_perl_dynamic='yes'
    fi
    # Is PERL's MakeMaker new enough to support DESTDIR?
    AX_PROG_PERL_VERSION(5.8.1,[PERL_SUPPORTS_DESTDIR='yes'],[PERL_SUPPORTS_DESTDIR='no'])
fi
AM_CONDITIONAL(WITH_PERL, test "$have_perl" != 'no')
AM_CONDITIONAL(WITH_PERL_STATIC, test $with_perl_static = 'yes')
AM_CONDITIONAL(WITH_PERL_DYNAMIC, test $with_perl_dynamic = 'yes')
AC_SUBST(PERL_SUPPORTS_DESTDIR)

# Determine path to pick up MagickCore library from for use with building PerlMagick
MAGICKCORE_PATH="${LIB_DIR}"
if test $with_perl_static = 'yes'; then
    # Find out where libtool hides its uninstalled libraries (as libtool_objdir)
    libtool_objdir=$objdir

    # Linker search path to library, followed by -lMagickCore
    MAGICKCORE_PATH="${builddir}/MagickCore/${libtool_objdir}"
fi
AC_SUBST(MAGICKCORE_PATH)

# Create a simple string containing format names for all delegate libraries.
MAGICK_DELEGATES=''
if test "$have_autotrace"  = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES autotrace"
fi
if test "$have_bzlib"  = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES bzlib"
fi
if test "$have_dps"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES dps"
fi
if test "$have_djvu"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES djvu"
fi
if test "$have_avconv"   = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES mpeg"
fi
if test "$have_fftw"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES fftw"
fi
if test "$have_flif"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES flif"
fi
if test "$have_fpx"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES fpx"
fi
if test "$have_fontconfig"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES fontconfig"
fi
if test "$have_freetype"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES freetype"
fi
if test "$have_gslib"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES gslib"
fi
if test "$have_heic"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES heic"
fi
if test "$have_jbig"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES jbig"
fi
if test "$have_png$have_jpeg" = 'yesyes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES jng"
fi
if test "$have_jpeg"   = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES jpeg"
fi
if test "$have_lcms" = 'yes' || test "$have_lcms2" = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES lcms"
fi
if test "$have_lqr"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES lqr"
fi
if test "$have_lzma"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES lzma"
fi
if test "$have_openexr"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES openexr"
fi
if test "$have_openjp2"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES openjp2"
fi
if test "$have_pango"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES pango"
fi
if test "$have_png"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES png"
fi
have_ps='no'
if test "$have_dps"    = 'yes' || \
   test "$have_gs" = 'yes' || \
   test "${native_win32_build}" = 'yes' ; then
   have_ps='yes'
fi
if test "$have_ps"     = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES ps"
fi
if test "$have_raw"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES raw"
fi
if test "$have_raqm"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES raqm"
fi
if test "$have_ra_ppm" = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES rad"
fi
if test "$have_rsvg"   = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES rsvg"
fi
if test "$have_tiff"   = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES tiff"
fi
if test "$have_ttf"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES ttf"
fi
if test "$have_webp"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES webp"
fi
if test "$have_wmf"    = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES wmf"
fi
if test "$have_x"      = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES x"
fi
if test "$have_xml"      = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES xml"
fi
if test "$have_zlib"   = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES zlib"
fi
if test "$have_zstd"   = 'yes' ; then
   MAGICK_DELEGATES="$MAGICK_DELEGATES zstd"
fi

# Remove extraneous spaces from output variables (asthetic)
MAGICK_DELEGATES=`echo $MAGICK_DELEGATES | sed -e 's/  */ /g'`
MAGICK_FEATURES=`echo $MAGICK_FEATURES | sed -e 's/  */ /g'`
AC_SUBST(MAGICK_DELEGATES)
AC_SUBST(MAGICK_FEATURES)

#
# Handle special compiler flags
#

# Add '-p' if prof source profiling support enabled
if test "$enable_prof" = 'yes'; then
    CFLAGS="-p $CFLAGS"
    CXXFLAGS="-p $CXXFLAGS"
    LDFLAGS="-p $LDFLAGS"
fi

# Add '-pg' if gprof source profiling support enabled
if test "$enable_gprof" = 'yes'; then
    CFLAGS="-pg $CFLAGS"
    CXXFLAGS="-pg $CXXFLAGS"
    LDFLAGS="-pg $LDFLAGS"
fi

# Add '-ftest-coverage -fprofile-arcs' if gcov source profiling support enabled
# This is a gcc-specific feature
if test "$enable_gcov" = 'yes'; then
    AC_CHECK_LIB(gcov,_gcov_init)
    AC_CHECK_LIB(gcov,__gcov_init)
    case "$target_os" in
        darwin*)
            OSX_GCOV_LDFLAG="-Wl,-single_module"
        ;;
        *)
            OSX_GCOV_LDFLAG=""
        ;;
    esac
    AC_SUBST(OSX_GCOV_LDFLAG)
    CFLAGS="-ftest-coverage -fprofile-arcs  $CFLAGS"
    CXXFLAGS="-ftest-coverage -fprofile-arcs  $CXXFLAGS"
    LDFLAGS="-ftest-coverage -fprofile-arcs $LDFLAGS"
fi

#
# Build library dependency list for libMagickCore
#

if test "$build_modules" != 'no'; then
    MAGICK_DEP_LIBS="$USER_LIBS $LCMS_LIBS $FREETYPE_LIBS $RAQM_LIBS $LQR_LIBS $FFTW_LIBS $XML_LIBS $FLIF_LIBS $FONTCONFIG_LIBS $XEXT_LIBS $IPC_LIBS $X11_LIBS $XT_LIBS $LZMA_LIBS $BZLIB_LIBS $ZLIB_LIBS $ZSTD_LIBS $LTDL_LIBS $GDI32_LIBS $WS2_32_LIBS $MATH_LIBS $CL_LIBS $UMEM_LIBS $JEMALLOC_LIBS $THREAD_LIBS"
else
    MAGICK_DEP_LIBS="$USER_LIBS $JBIG_LIBS $LCMS_LIBS $TIFF_LIBS $FREETYPE_LIBS $RAQM_LIBS $JPEG_LIBS $GS_LIBS $LQR_LIBS $PNG_LIBS $AUTOTRACE_LIBS $DJVU_LIBS $FFTW_LIBS $FLIF_LIBS $FPX_LIBS $FONTCONFIG_LIBS $HEIF_LIBS $WEBPMUX_LIBS $WEBP_LIBS $WMF_LIBS $DPS_LIBS $XEXT_LIBS $XT_LIBS $IPC_LIBS $X11_LIBS $LZMA_LIBS $BZLIB_LIBS $OPENEXR_LIBS $LIBOPENJP2_LIBS $PANGO_LIBS $RAW_R_LIBS $RSVG_LIBS $XML_LIBS $GVC_LIBS $ZLIB_LIBS $ZSTD_LIBS $LTDL_LIBS $GDI32_LIBS $WS2_32_LIBS $MATH_LIBS $CL_LIBS $UMEM_LIBS $JEMALLOC_LIBS $THREAD_LIBS"
fi
MAGICK_EXTRA_DEP_LIBS="$GOMP_LIBS"
AC_SUBST(MAGICK_DEP_LIBS)
AC_SUBST(MAGICK_EXTRA_DEP_LIBS)

# Pass only user-provided LIBS as "global" libraries
LIBS=$USER_LIBS

#AC_SUBST(CPPFLAGS)
AC_SUBST(X_CFLAGS)
#AC_SUBST(LDFLAGS)
#AC_SUBST(X_PRE_LIBS)
#AC_SUBST(X_LIBS)
#AC_SUBST(X_EXTRA_LIBS)

MAGICK_CFLAGS=$CFLAGS
MAGICK_CXXFLAGS="$CXXFLAGS"
MAGICK_CPPFLAGS=`echo $MAGICK_CPPFLAGS | sed -e 's/  */ /g'`
MAGICK_PCFLAGS=`echo $MAGICK_PCFLAGS | sed -e 's/  */ /g'`
MAGICK_LDFLAGS="-L$LIB_DIR $LDFLAGS"
MAGICK_LIBS="$MAGICK_DEP_LIBS $MAGICK_EXTRA_DEP_LIBS"

AC_SUBST(MAGICK_CFLAGS)
AC_SUBST(MAGICK_CXXFLAGS)
AC_SUBST(MAGICK_CPPFLAGS)
AC_SUBST(MAGICK_PCFLAGS)
AC_SUBST(MAGICK_LDFLAGS)
AC_SUBST(MAGICK_LIBS)

# Set configured scripts to executable.
AC_CONFIG_COMMANDS([default],[],[])
AC_CONFIG_COMMANDS([magick.sh.in],[chmod +x magick.sh])
AC_CONFIG_COMMANDS([MagickCore-config.in],[chmod +x MagickCore/MagickCore-config])
AC_CONFIG_COMMANDS([MagickWand-config.in],[chmod +x MagickWand/MagickWand-config])
AC_CONFIG_COMMANDS([Magick++-config.in],[chmod +x Magick++/bin/Magick++-config])
AC_CONFIG_COMMANDS([PerlMagick/check.sh.in],[chmod +x PerlMagick/check.sh])

AC_MSG_RESULT([-------------------------------------------------------------])
AC_MSG_RESULT([Update ImageMagick configuration])

rm -f magick-version

result_dejavu_font_dir='none'
if test "${dejavu_font_dir}x" != 'x'; then
    result_dejavu_font_dir=$dejavu_font_dir
fi

result_ghostscript_font_dir='none'
if test "${ghostscript_font_dir}x" != 'x'; then
    result_ghostscript_font_dir=$ghostscript_font_dir
fi

result_urw_base35_font_dir='none'
if test "${urw_base35_font_dir}x" != 'x'; then
    result_urw_base35_font_dir=$urw_base35_font_dir
fi

result_windows_font_dir='none'
if test "${windows_font_dir}x" != 'x'; then
    result_windows_font_dir=${windows_font_dir}
fi

# ==============================================================================
# Generate build environment
# ==============================================================================
AC_CONFIG_FILES([\
    common.shi \
    config/configure.xml \
    config/delegates.xml \
    config/ImageMagick.rdf \
    config/MagickCore.dox \
    config/MagickWand.dox \
    config/Magick++.dox \
    config/type-apple.xml \
    config/type-dejavu.xml \
    config/type-ghostscript.xml \
    config/type-urw-base35.xml \
    config/type-windows.xml \
    config/type.xml \
    ImageMagick.spec \
    Magick++/bin/Magick++-config \
    MagickCore/ImageMagick.pc \
    Magick++/lib/Magick++.pc \
    MagickCore/MagickCore-config \
    MagickCore/MagickCore.pc \
    MagickCore/version.h \
    Makefile \
    magick.sh \
    PerlMagick/check.sh \
    PerlMagick/default/Magick.pm \
    PerlMagick/Makefile.PL \
    PerlMagick/default/Makefile.PL \
    PerlMagick/quantum/Makefile.PL \
    PerlMagick/quantum/quantum.pm \
    PerlMagick/quantum/quantum.xs \
    PerlMagick/quantum/typemap \
    utilities/animate.1 \
    utilities/compare.1 \
    utilities/composite.1 \
    utilities/conjure.1 \
    utilities/convert.1 \
    utilities/display.1 \
    utilities/identify.1 \
    utilities/ImageMagick.1 \
    utilities/import.1 \
    utilities/magick.1 \
    utilities/magick-script.1 \
    utilities/mogrify.1 \
    utilities/montage.1 \
    utilities/stream.1 \
    MagickWand/MagickWand-config \
    MagickWand/MagickWand.pc ])
AC_OUTPUT

# ==============================================================================
# ImageMagick Configuration
# ==============================================================================
AC_MSG_NOTICE([
==============================================================================
ImageMagick is configured as follows. Please verify that this configuration
matches your expectations.

  Host system type: $host
  Build system type: $build

                 Option                        Value
  ------------------------------------------------------------------------------
  Shared libraries  --enable-shared=$enable_shared		$libtool_build_shared_libs
  Static libraries  --enable-static=$enable_static		$libtool_build_static_libs
  Build utilities   --with-utilities=$with_utilities        $with_utilities
  Module support    --with-modules=$build_modules		$build_modules
  GNU ld            --with-gnu-ld=$with_gnu_ld		$lt_cv_prog_gnu_ld
  Quantum depth     --with-quantum-depth=$with_quantum_depth	$with_quantum_depth
  High Dynamic Range Imagery
                    --enable-hdri=$enable_hdri		$enable_hdri

  Install documentation:			$wantdocs

  Delegate Library Configuration:
  BZLIB             --with-bzlib=$with_bzlib		$have_bzlib
  Autotrace         --with-autotrace=$with_autotrace		$have_autotrace
  DJVU              --with-djvu=$with_djvu		$have_djvu
  DPS               --with-dps=$with_dps		$have_dps
  FFTW              --with-fftw=$with_fftw		$have_fftw
  FLIF              --with-flif=$with_flif		$have_flif
  FlashPIX          --with-fpx=$with_fpx		$have_fpx
  FontConfig        --with-fontconfig=$with_fontconfig	$have_fontconfig
  FreeType          --with-freetype=$with_freetype		$have_freetype
  Ghostscript lib   --with-gslib=$with_gslib		$have_gslib
  Graphviz          --with-gvc=$with_gvc		$have_gvc
  HEIC              --with-heic=$with_heic             $have_heic
  JBIG              --with-jbig=$with_jbig		$have_jbig
  JPEG v1           --with-jpeg=$with_jpeg		$have_jpeg
  LCMS              --with-lcms=$with_lcms		$have_lcms
  LQR               --with-lqr=$with_lqr		$have_lqr
  LTDL              --with-ltdl=$with_ltdl		$have_ltdl
  LZMA              --with-lzma=$with_lzma		$have_lzma
  Magick++          --with-magick-plus-plus=$with_magick_plus_plus	$have_magick_plus_plus
  OpenEXR           --with-openexr=$with_openexr		$have_openexr
  OpenJP2           --with-openjp2=$with_openjp2		$have_openjp2
  PANGO             --with-pango=$with_pango		$have_pango
  PERL              --with-perl=$with_perl		$have_perl
  PNG               --with-png=$with_png		$have_png
  RAQM              --with-raqm=$with_raqm		$have_raqm
  RAW               --with-raw=$with_raw 	   	$have_raw
  RSVG              --with-rsvg=$with_rsvg		$have_rsvg
  TIFF              --with-tiff=$with_tiff		$have_tiff
  WEBP              --with-webp=$with_webp		$have_webp
  WMF               --with-wmf=$with_wmf		$have_wmf
  X11               --with-x=$with_x			$have_x
  XML               --with-xml=$with_xml		$have_xml
  ZLIB              --with-zlib=$with_zlib		$have_zlib
  ZSTD              --with-zstd=$with_zstd		$have_zstd

  Delegate Program Configuration:
  GhostPCL          None				$PCLDelegate ($PCLVersion)
  GhostXPS          None				$XPSDelegate ($XPSVersion)
  Ghostscript       None				$PSDelegate ($GSVersion)

  Font Configuration:
  Apple fonts       --with-apple-font-dir=$with_apple_font_dir	$result_apple_font_dir
  Dejavu fonts      --with-dejavu-font-dir=$with_dejavu_font_dir	$result_dejavu_font_dir
  Ghostscript fonts --with-gs-font-dir=$with_gs_font_dir	$result_ghostscript_font_dir
  URW-base35 fonts  --with-urw-base35-font-dir=$with_urw_base35_font_dir  $result_urw_base35_font_dir
  Windows fonts     --with-windows-font-dir=$with_windows_font_dir	$result_windows_font_dir

  X11 Configuration:
        X_CFLAGS        = $X_CFLAGS
        X_PRE_LIBS      = $X_PRE_LIBS
        X_LIBS          = $X_LIBS
        X_EXTRA_LIBS    = $X_EXTRA_LIBS

  Options used to compile and link:
    PREFIX          = $PREFIX_DIR
    EXEC-PREFIX     = $EXEC_PREFIX_DIR
    VERSION         = $PACKAGE_VERSION
    CC              = $CC
    CFLAGS          = $CFLAGS
    CPPFLAGS        = $CPPFLAGS
    PCFLAGS         = $PCFLAGS
    DEFS            = $DEFS
    LDFLAGS         = $LDFLAGS
    LIBS            = $MAGICK_DEP_LIBS
    CXX             = $CXX
    CXXFLAGS        = $CXXFLAGS
    FEATURES        = $MAGICK_FEATURES
    DELEGATES       = $MAGICK_DELEGATES
==============================================================================
])
