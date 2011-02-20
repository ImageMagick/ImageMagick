/* lt_system.h -- system portability abstraction layer

   Copyright (C) 2004, 2007, 2010 Free Software Foundation, Inc.
   Written by Gary V. Vaughan, 2004

   NOTE: The canonical source of this file is maintained with the
   GNU Libtool package.  Report bugs to bug-libtool@gnu.org.

GNU Libltdl is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

As a special exception to the GNU Lesser General Public License,
if you distribute this file as part of a program or library that
is built using GNU Libtool, you may include this file under the
same distribution terms that you use for the rest of that program.

GNU Libltdl is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with GNU Libltdl; see the file COPYING.LIB.  If not, a
copy can be downloaded from  http://www.gnu.org/licenses/lgpl.html,
or obtained by writing to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#if !defined(LT_SYSTEM_H)
#define LT_SYSTEM_H 1

#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>

/* Some systems do not define EXIT_*, even with STDC_HEADERS.  */
#if !defined(EXIT_SUCCESS)
# define EXIT_SUCCESS 0
#endif
#if !defined(EXIT_FAILURE)
# define EXIT_FAILURE 1
#endif

/* Just pick a big number... */
#define LT_FILENAME_MAX 2048


/* Saves on those hard to debug '\0' typos....  */
#define LT_EOS_CHAR	'\0'

/* LTDL_BEGIN_C_DECLS should be used at the beginning of your declarations,
   so that C++ compilers don't mangle their names.  Use LTDL_END_C_DECLS at
   the end of C declarations. */
#if defined(__cplusplus)
# define LT_BEGIN_C_DECLS	extern "C" {
# define LT_END_C_DECLS		}
#else
# define LT_BEGIN_C_DECLS	/* empty */
# define LT_END_C_DECLS		/* empty */
#endif

/* LT_STMT_START/END are used to create macros which expand to a
   a single compound statement in a portable way.  */
#if defined (__GNUC__) && !defined (__STRICT_ANSI__) && !defined (__cplusplus)
#  define LT_STMT_START        (void)(
#  define LT_STMT_END          )
#else
#  if (defined (sun) || defined (__sun__))
#    define LT_STMT_START      if (1)
#    define LT_STMT_END        else (void)0
#  else
#    define LT_STMT_START      do
#    define LT_STMT_END        while (0)
#  endif
#endif

/* Keep this code in sync between libtool.m4, ltmain, lt_system.h, and tests.  */
#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
/* DATA imports from DLLs on WIN32 con't be const, because runtime
   relocations are performed -- see ld's documentation on pseudo-relocs.  */
# define LT_DLSYM_CONST
#elif defined(__osf__)
/* This system does not cope well with relocations in const data.  */
# define LT_DLSYM_CONST
#else
# define LT_DLSYM_CONST const
#endif

/* Canonicalise Windows and Cygwin recognition macros.
   To match the values set by recent Cygwin compilers, make sure that if
   __CYGWIN__ is defined (after canonicalisation), __WINDOWS__ is NOT!  */
#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
# define __CYGWIN__ __CYGWIN32__
#endif
#if defined(__CYGWIN__)
# if defined(__WINDOWS__)
#   undef __WINDOWS__
# endif
#elif defined(_WIN32)
# define __WINDOWS__ _WIN32
#elif defined(WIN32)
# define __WINDOWS__ WIN32
#endif
#if defined(__CYGWIN__) && defined(__WINDOWS__)
# undef __WINDOWS__
#endif


/* DLL building support on win32 hosts;  mostly to workaround their
   ridiculous implementation of data symbol exporting. */
#if !defined(LT_SCOPE)
#  if defined(__WINDOWS__) || defined(__CYGWIN__)
#    if defined(DLL_EXPORT)		/* defined by libtool (if required) */
#      define LT_SCOPE	extern __declspec(dllexport)
#    endif
#    if defined(LIBLTDL_DLL_IMPORT)	/* define if linking with this dll */
       /* note: cygwin/mingw compilers can rely instead on auto-import */
#      define LT_SCOPE	extern __declspec(dllimport)
#    endif
#  endif
#  if !defined(LT_SCOPE)		/* static linking or !__WINDOWS__ */
#    define LT_SCOPE	extern
#  endif
#endif

#if defined(__WINDOWS__)
/* LT_DIRSEP_CHAR is accepted *in addition* to '/' as a directory
   separator when it is set. */
# define LT_DIRSEP_CHAR		'\\'
# define LT_PATHSEP_CHAR	';'
#else
# define LT_PATHSEP_CHAR	':'
#endif

#if defined(_MSC_VER) /* Visual Studio */
#  define R_OK 4
#endif

/* fopen() mode flags for reading a text file */
#undef	LT_READTEXT_MODE
#if defined(__WINDOWS__) || defined(__CYGWIN__)
#  define LT_READTEXT_MODE "rt"
#else
#  define LT_READTEXT_MODE "r"
#endif

/* The extra indirection to the LT__STR and LT__CONC macros is required so
   that if the arguments to LT_STR() (or LT_CONC()) are themselves macros,
   they will be expanded before being quoted.   */
#ifndef LT_STR
#  define LT__STR(arg)		#arg
#  define LT_STR(arg)		LT__STR(arg)
#endif

#ifndef LT_CONC
#  define LT__CONC(a, b)	a##b
#  define LT_CONC(a, b)		LT__CONC(a, b)
#endif
#ifndef LT_CONC3
#  define LT__CONC3(a, b, c)	a##b##c
#  define LT_CONC3(a, b, c)	LT__CONC3(a, b, c)
#endif

#endif /*!defined(LT_SYSTEM_H)*/
