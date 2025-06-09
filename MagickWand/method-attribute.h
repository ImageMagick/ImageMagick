/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand method attributes.
*/
#ifndef MAGICKWAND_METHOD_ATTRIBUTE_H
#define MAGICKWAND_METHOD_ATTRIBUTE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKWAND_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
#  define WandPrivate
#  if defined(_MT) && defined(_DLL) && !defined(_MAGICKDLL_) && !defined(_LIB)
#    define _MAGICKDLL_
#  endif
#  if defined(_MAGICKDLL_)
#    if !defined(_MAGICKLIB_)
#      if defined(__clang__) || defined(__GNUC__)
#        define WandExport __attribute__ ((dllimport))
#      else
#        define WandExport __declspec(dllimport)
#      endif
#    else
#      if defined(__clang__) || defined(__GNUC__)
#        define WandExport __attribute__ ((dllexport))
#      else
#        define WandExport __declspec(dllexport)
#      endif
#    endif
#  else
#    define WandExport
#  endif
#  if defined(_VISUALC_)
#    pragma warning(disable : 4018)
#    pragma warning(disable : 4068)
#    pragma warning(disable : 4244)
#    pragma warning(disable : 4142)
#    pragma warning(disable : 4800)
#    pragma warning(disable : 4786)
#    pragma warning(disable : 4996)
#  endif
#else
#  if defined(__clang__) || (__GNUC__ >= 4)
#    define WandExport __attribute__ ((visibility ("default")))
#    define WandPrivate  __attribute__ ((visibility ("hidden")))
#  else
#    define WandExport
#    define WandPrivate
#  endif
#endif

#define MagickWandSignature  0xabacadabUL
#if !defined(MagickPathExtent)
#  define MagickPathExtent  4096
#endif

#if defined(MAGICKCORE_HAVE___ATTRIBUTE__)
#  define wand_aligned(x)  __attribute__((aligned(x)))
#  define wand_attribute  __attribute__
#  define wand_unused(x)  wand_unused_ ## x __attribute__((unused))
#  define wand_unreferenced(x)  /* nothing */
#elif defined(MAGICKWAND_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
#  define wand_aligned(x)  __declspec(align(x))
#  define wand_attribute(x)  /* nothing */
#  define wand_unused(x) x
#  define wand_unreferenced(x) (x)
#else
#  define wand_aligned(x)  /* nothing */
#  define wand_attribute(x)  /* nothing */
#  define wand_unused(x) x
#  define wand_unreferenced(x)  /* nothing */
#endif

#if !defined(__clang__) && (defined(__GNUC__) && (__GNUC__) > 4)
#  define wand_alloc_size(x)  __attribute__((__alloc_size__(x)))
#  define wand_alloc_sizes(x,y)  __attribute__((__alloc_size__(x,y)))
#else
#  define wand_alloc_size(x)  /* nothing */
#  define wand_alloc_sizes(x,y)  /* nothing */
#endif

#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__) > 4)
#  define wand_cold_spot  __attribute__((__cold__))
#  define wand_hot_spot  __attribute__((__hot__))
#else
#  define wand_cold_spot
#  define wand_hot_spot
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
