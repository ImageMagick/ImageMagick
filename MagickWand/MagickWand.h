/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand Application Programming Interface declarations.
*/

#ifndef _MAGICKWAND_MAGICKWAND_H
#define _MAGICKWAND_MAGICKWAND_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(_MAGICKWAND_CONFIG_H)
# define _MAGICKWAND_CONFIG_H
# if !defined(vms) && !defined(macintosh)
#  include "MagickCore/magick-config.h"
# else
#  include "magick-config.h"
# endif
#if defined(_magickcore_const) && !defined(const)
# define const _magickcore_const
#endif
#if defined(_magickcore_inline) && !defined(inline)
# define inline _magickcore_inline
#endif
#if defined(_magickcore_restrict) && !defined(restrict)
# define restrict  _magickcore_restrict
#endif
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(WIN32) || defined(WIN64)
#  define MAGICKCORE_WINDOWS_SUPPORT
#else
#  define MAGICKCORE_POSIX_SUPPORT
#endif 

#if defined(__BORLANDC__) && defined(_DLL)
#  pragma message("BCBMagick lib DLL export interface")
#  define _MAGICKDLL_
#  define _MAGICKLIB_
#endif

#include "MagickWand/method-attribute.h"

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
# define WandPrivate
# if defined(_MT) && defined(_DLL) && !defined(_MAGICKDLL_) && !defined(_LIB)
#  define _MAGICKDLL_
# endif
# if defined(_MAGICKDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_MAGICKLIB_)
#   if defined(__GNUC__)
#    define WandExport __attribute__ ((__dllimport__))
#   else
#    define WandExport __declspec(dllimport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "MagickWand lib DLL import interface" )
#   endif
#  else
#   if defined(__GNUC__)
#    define WandExport __attribute__ ((__dllexport__))
#   else
#    define WandExport __declspec(dllexport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "MagickWand lib DLL export interface" )
#   endif
#  endif
# else
#  define WandExport
#  if defined(_VISUALC_)
#   pragma message( "MagickWand lib static interface" )
#  endif
# endif

# define WandGlobal __declspec(thread)
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
#  pragma warning(disable : 4996)
# endif
#else
# if __GNUC__ >= 4
#  define WandExport __attribute__ ((__visibility__ ("default")))
#  define WandPrivate  __attribute__ ((__visibility__ ("hidden")))
# else
#   define WandExport
#   define WandPrivate
# endif
# define WandGlobal
#endif

#if defined(MAGICKCORE_HAVE___ATTRIBUTE__)
#  define wand_aligned(x)  __attribute__((__aligned__(x)))
#  define wand_attribute  __attribute__
#  define wand_unused(x)  wand_unused_ ## x __attribute__((__unused__))
#else
#  define wand_aligned(x)  /* nothing */
#  define wand_attribute(x)  /* nothing */
#  define wand_unused(x) x
#endif

#if defined(MAGICKCORE_HAVE___ALLOC_SIZE__)
#  define wand_alloc_size(x)  __attribute__((__alloc_size__(x)))
#  define wand_alloc_sizes(x,y)  __attribute__((__alloc_size__(x,y)))
#  define wand_cold  __attribute__((__cold__))
#  define wand_hot  __attribute__((__hot__))
#else
#  define wand_alloc_size(x)  /* nothing */
#  define wand_alloc_sizes(x,y)  /* nothing */
#  define wand_cold
#  define wand_hot
#endif

typedef struct _MagickWand
  MagickWand;

#include "MagickCore/MagickCore.h"
#include "MagickWand/animate.h"
#include "MagickWand/compare.h"
#include "MagickWand/composite.h"
#include "MagickWand/conjure.h"
#include "MagickWand/convert.h"
#include "MagickWand/deprecate.h"
#include "MagickWand/display.h"
#include "MagickWand/drawing-wand.h"
#include "MagickWand/identify.h"
#include "MagickWand/import.h"
#include "MagickWand/wandcli.h"
#include "MagickWand/operation.h"
#include "MagickWand/magick-cli.h"
#include "MagickWand/magick-property.h"
#include "MagickWand/magick-image.h"
#include "MagickWand/mogrify.h"
#include "MagickWand/montage.h"
#include "MagickWand/pixel-iterator.h"
#include "MagickWand/pixel-wand.h"
#include "MagickWand/stream.h"
#include "MagickWand/wand-view.h"

extern WandExport char
  *MagickGetException(const MagickWand *,ExceptionType *);

extern WandExport ExceptionType
  MagickGetExceptionType(const MagickWand *);

extern WandExport MagickBooleanType
  IsMagickWand(const MagickWand *),
  MagickClearException(MagickWand *),
  MagickSetIteratorIndex(MagickWand *,const ssize_t);

extern WandExport MagickWand
  *CloneMagickWand(const MagickWand *),
  *DestroyMagickWand(MagickWand *),
  *NewMagickWand(void),
  *NewMagickWandFromImage(const Image *);

extern WandExport ssize_t
  MagickGetIteratorIndex(MagickWand *);

extern WandExport void
  ClearMagickWand(MagickWand *),
  MagickWandGenesis(void),
  MagickWandTerminus(void),
  *MagickRelinquishMemory(void *),
  MagickResetIterator(MagickWand *),
  MagickSetFirstIterator(MagickWand *),
  MagickSetLastIterator(MagickWand *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
