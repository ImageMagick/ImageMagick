/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore method attributes.
*/
#ifndef MAGICKCORE_METHOD_ATTRIBUTE_H
#define MAGICKCORE_METHOD_ATTRIBUTE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__BORLANDC__) && defined(_DLL)
#  pragma message("BCBMagick lib DLL export interface")
#  define _MAGICKDLL_
#  define _MAGICKLIB_
#  define MAGICKCORE_MODULES_SUPPORT
#  undef MAGICKCORE_BUILD_MODULES
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
# define MagickPrivate
# if defined(_MT) && defined(_DLL) && !defined(_MAGICKDLL_) && !defined(_LIB)
#  define _MAGICKDLL_
# endif
# if defined(_MAGICKDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_MAGICKLIB_)
#   if defined(__clang__) || defined(__GNUC__)
#    define MagickExport __attribute__ ((dllimport))
#   else
#    define MagickExport __declspec(dllimport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "MagickCore lib DLL import interface" )
#   endif
#  else
#   if defined(__clang__) || defined(__GNUC__)
#    define MagickExport __attribute__ ((dllexport))
#   else
#    define MagickExport __declspec(dllexport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "MagickCore lib DLL export interface" )
#   endif
#  endif
# else
#  define MagickExport
#  if defined(_VISUALC_)
#   pragma message( "MagickCore lib static interface" )
#  endif
# endif

# if defined(_DLL) && !defined(_LIB)
#   if defined(__clang__) || defined(__GNUC__)
#    define ModuleExport __attribute__ ((dllexport))
#   else
#    define ModuleExport __declspec(dllexport)
#   endif
#  if defined(_VISUALC_)
#   pragma message( "MagickCore module DLL export interface" )
#  endif
# else
#  define ModuleExport
#  if defined(_VISUALC_)
#   pragma message( "MagickCore module static interface" )
#  endif

# endif
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4068)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
#  pragma warning(disable : 4996)
# endif
#else
# if defined(__clang__) || (__GNUC__ >= 4)
#  define MagickExport __attribute__ ((visibility ("default")))
#  define MagickPrivate  __attribute__ ((visibility ("hidden")))
# else
#   define MagickExport
#   define MagickPrivate
# endif
# define ModuleExport  MagickExport
#endif

#define MagickCoreSignature  0xabacadabUL
#if !defined(MagickPathExtent)
# define MagickPathExtent  4096  /* always >= 4096 */
#endif
# define MaxTextExtent  MagickPathExtent

#if defined(MAGICKCORE_HAVE___ATTRIBUTE__)
#  define magick_aligned(x,y)  x __attribute__((aligned(y)))
#  define magick_attribute  __attribute__
#  define magick_unused(x)  magick_unused_ ## x __attribute__((unused))
#  define magick_unreferenced(x)  /* nothing */
#elif defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
#  define magick_aligned(x,y)  __declspec(align(y)) x
#  define magick_attribute(x)  /* nothing */
#  define magick_unused(x) x
#  define magick_unreferenced(x) (x)
#else
#  define magick_aligned(x,y)  /* nothing */
#  define magick_attribute(x)  /* nothing */
#  define magick_unused(x) x
#  define magick_unreferenced(x)  /* nothing */
#endif

#if !defined(__clang__) && (((__GNUC__) > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)))
#  define magick_alloc_size(x)  __attribute__((__alloc_size__(x)))
#  define magick_alloc_sizes(x,y)  __attribute__((__alloc_size__(x,y)))
#else
#  define magick_alloc_size(x)  /* nothing */
#  define magick_alloc_sizes(x,y)  /* nothing */
#endif

#if defined(__clang__) || (((__GNUC__) > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)))
#  define magick_cold_spot  __attribute__((__cold__))
#  define magick_hot_spot  __attribute__((__hot__))
#else
#  define magick_cold_spot
#  define magick_hot_spot
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
